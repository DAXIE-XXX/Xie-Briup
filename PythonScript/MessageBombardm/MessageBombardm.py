#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
自定义文本/图片循环输出（支持自动发送）——美化版 UI
功能保持原来的全部特性，只是界面更整齐、配色更统一。
"""

# ==================== 标准库 ====================
import sys
import threading
import time
from pathlib import Path
from tkinter import (
    Tk, Frame, filedialog, messagebox, W, LEFT, BOTH, X, Y
)
from tkinter import StringVar, BooleanVar

# ==================== ttk（主题化控件） ====================
try:
    from tkinter import ttk
except Exception as e:
    messagebox.showerror("Tkinter 错误", f"ttk 加载失败: {e}")
    sys.exit(1)

# ==================== 第三方库 ====================
try:
    import pyautogui
except ImportError:
    messagebox.showerror("缺少依赖", "请先运行 `pip install pyautogui`")
    sys.exit(1)

try:
    from PIL import Image
except ImportError:
    messagebox.showerror("缺少依赖", "请先运行 `pip install pillow`")
    sys.exit(1)

if sys.platform.startswith("win"):
    try:
        import win32clipboard
        import win32con
    except Exception:
        messagebox.showerror(
            "缺少依赖",
            "Windows 需要 `pip install pywin32` 才能复制图片到剪贴板"
        )
        sys.exit(1)

# keyboard（可选） -------------------------------------------------
try:
    import keyboard
    HAVE_KEYBOARD = True
except Exception:
    HAVE_KEYBOARD = False

# ==================== 线程控制 ====================
stop_flag = threading.Event()

# ==================== 辅助函数 ====================
def _show_info(msg: str):
    """在任意线程中安全弹出信息框"""
    messagebox.showinfo("提示", msg)


def record_position(target_var: StringVar, tip: str):
    """让用户把鼠标移动到目标位置，3 秒后记录坐标"""
    _show_info(
        f"请把鼠标移动到 **{tip}** 的位置，\n"
        "3 秒后自动记录坐标。"
    )
    time.sleep(3)
    x, y = pyautogui.position()
    target_var.set(f"{x},{y}")
    _show_info(f"{tip} 坐标已记录: ({x}, {y})")


# ------------------- 剪贴板（跨平台） -------------------
def _image_to_clipboard(img_path: Path) -> bool:
    """把图片写入系统剪贴板，返回是否成功"""
    if not img_path.is_file():
        messagebox.showerror("错误", f"图片文件不存在: {img_path}")
        return False

    # ----- Windows -----
    if sys.platform.startswith("win"):
        try:
            img = Image.open(img_path).convert("RGB")
            import io
            with io.BytesIO() as buf:
                img.save(buf, "BMP")
                data = buf.getvalue()[14:]          # 去掉 BMP 文件头的 14 字节
            win32clipboard.OpenClipboard()
            win32clipboard.EmptyClipboard()
            win32clipboard.SetClipboardData(win32con.CF_DIB, data)
            win32clipboard.CloseClipboard()
            return True
        except Exception as e:
            messagebox.showerror("剪贴板错误", f"写入图片失败: {e}")
            return False

    # ----- macOS -----
    if sys.platform == "darwin":
        try:
            import subprocess, io
            img = Image.open(img_path).convert("RGBA")
            with io.BytesIO() as buf:
                img.save(buf, format="PNG")
                png = buf.getvalue()
            p = subprocess.Popen(
                ["pbcopy"], env={"LANG": "en_US.UTF-8"}, stdin=subprocess.PIPE
            )
            p.communicate(png)
            return p.returncode == 0
        except Exception as e:
            messagebox.showerror("剪贴板错误", f"macOS 复制图片失败: {e}")
            return False

    # ----- Linux -----
    if sys.platform.startswith("linux"):
        try:
            import subprocess, shlex, io
            img = Image.open(img_path).convert("RGB")
            with io.BytesIO() as buf:
                img.save(buf, format="PNG")
                png = buf.getvalue()
            for prog in ("xclip", "xsel"):
                try:
                    cmd = "xclip -selection clipboard -t image/png -i" \
                        if prog == "xclip" else \
                        "xsel --clipboard --input --mime-type image/png"
                    p = subprocess.Popen(shlex.split(cmd), stdin=subprocess.PIPE)
                    p.communicate(png)
                    if p.returncode == 0:
                        return True
                except FileNotFoundError:
                    continue
            messagebox.showerror(
                "剪贴板错误",
                "Linux 需要安装 xclip 或 xsel 才能复制图片到剪贴板。\n"
                "请运行: sudo apt install xclip   # 或 sudo apt install xsel"
            )
            return False
        except Exception as e:
            messagebox.showerror("剪贴板错误", f"Linux 复制图片失败: {e}")
            return False

    # ----- 其它平台 -----
    messagebox.showerror("不支持的系统", f"当前系统 ({sys.platform}) 不支持图片粘贴")
    return False


# ==== 改动点 ==== #
def _text_to_clipboard_and_paste(text: str) -> bool:
    """
    把 *text* 写入系统剪贴板并立刻粘贴（Ctrl+V / ⌘+V）。
    兼容 Windows、macOS、Linux。
    返回 True 表示粘贴成功，False 表示出现异常。
    """
    try:
        # 1️⃣ 把文字写入剪贴板（platform‑specific）
        if sys.platform.startswith("win"):
            import win32clipboard, win32con
            win32clipboard.OpenClipboard()
            win32clipboard.EmptyClipboard()
            win32clipboard.SetClipboardData(win32con.CF_UNICODETEXT, text)
            win32clipboard.CloseClipboard()
        elif sys.platform == "darwin":
            import subprocess
            p = subprocess.Popen(
                ["pbcopy"], env={"LANG": "en_US.UTF-8"}, stdin=subprocess.PIPE
            )
            p.communicate(text.encode("utf-8"))
        elif sys.platform.startswith("linux"):
            # 使用 xclip / xsel（若不存在则报错）
            import subprocess, shlex
            for prog in ("xclip", "xsel"):
                try:
                    if prog == "xclip":
                        cmd = "xclip -selection clipboard"
                    else:
                        cmd = "xsel --clipboard --input"
                    p = subprocess.Popen(
                        shlex.split(cmd), stdin=subprocess.PIPE
                    )
                    p.communicate(text.encode("utf-8"))
                    if p.returncode == 0:
                        break
                except FileNotFoundError:
                    continue
            else:
                messagebox.showerror(
                    "剪贴板错误",
                    "Linux 需要安装 xclip 或 xsel 才能复制文字到剪贴板。\n"
                    "请运行: sudo apt install xclip   # 或 sudo apt install xsel"
                )
                return False
        else:
            messagebox.showerror("不支持的系统", f"当前系统 ({sys.platform}) 不支持文字粘贴")
            return False

        # 2️⃣ 粘贴（Ctrl+V / ⌘+V）
        if sys.platform.startswith("darwin"):
            pyautogui.hotkey("command", "v")
        else:
            pyautogui.hotkey("ctrl", "v")
        return True
    except Exception as e:
        messagebox.showerror("剪贴板错误", f"文字粘贴失败: {e}")
        return False
# ==== 改动点结束 ==== #


# ==================== 主业务逻辑 ====================
def start_output():
    """读取 UI 参数，启动子线程执行循环输出"""
    txt = text_var.get()
    send_image = send_image_var.get()
    send_text_first = send_text_first_var.get()          # <-- 新增
    img_path = Path(image_path_var.get()) if send_image else None

    # 必要的校验
    if not send_image and not txt:
        messagebox.showwarning("输入错误", "请填写要输出的文本或勾选并选择图片")
        return

    edit_pos = pos_var.get()
    if not edit_pos:
        messagebox.showwarning("输入错误", "请先记录编辑框坐标")
        return
    try:
        ex, ey = map(int, edit_pos.split(","))
    except Exception:
        messagebox.showerror("坐标错误", "编辑框坐标格式错误，请重新记录")
        return

    # 发送按钮（可选）
    btn_pos = None
    if send_btn_var.get():
        try:
            bx, by = map(int, send_btn_var.get().split(","))
            btn_pos = (bx, by)
        except Exception:
            messagebox.showerror("坐标错误", "发送按钮坐标格式错误，请重新记录")
            return

    # 循环参数
    try:
        interval = float(interval_var.get())
        if interval < 0:
            raise ValueError
    except ValueError:
        messagebox.showerror("间隔错误", "间隔必须是非负数（秒）")
        return

    try:
        max_cnt = int(count_var.get())
        if max_cnt < 0:
            raise ValueError
    except ValueError:
        messagebox.showerror("次数错误", "次数必须是非负整数（0 表示无限）")
        return

    really_send = send_var.get()
    auto_send = autosend_var.get()

    if send_image and not img_path.is_file():
        messagebox.showerror("图片错误", "请选择一张有效的图片文件")
        return

    # UI 锁定
    start_btn.config(state="disabled")
    stop_btn.config(state="normal")
    stop_flag.clear()

    # 全局热键 Esc 停止
    if HAVE_KEYBOARD:
        keyboard.unhook_all()
        keyboard.add_hotkey('esc', stop_output, suppress=False)

    threading.Thread(
        target=output_worker,
        args=(
            txt, ex, ey, btn_pos,
            interval, max_cnt,
            really_send, auto_send,
            send_image, img_path,
            send_text_first                      # <-- 传递新参数
        ),
        daemon=True,
    ).start()


def stop_output():
    """停止循环并恢复 UI 状态"""
    stop_flag.set()
    start_btn.config(state="normal")
    stop_btn.config(state="disabled")
    if HAVE_KEYBOARD:
        keyboard.unhook_all()
    _show_info("循环已停止")


def output_worker(
    text: str, edit_x: int, edit_y: int,
    send_btn: tuple | None,
    interval: float, max_cnt: int,
    really_send: bool, auto_send: bool,
    send_image: bool, img_path: Path,
    send_text_first: bool                 # <-- 新增
):
    """子线程：实际执行循环（已加入文字‑优先逻辑）"""
    i = 0
    while not stop_flag.is_set():
        if max_cnt != 0 and i >= max_cnt:
            break

        # ① 获得焦点
        pyautogui.click(edit_x, edit_y)

        # ② 发送文字 / 图片（顺序由 “文字优先” 决定）
        if send_image and send_text_first:
            # 先发送文字，再发送图片
            if really_send:
                _text_to_clipboard_and_paste(text)
                time.sleep(0.1)                       # 给系统一点时间
                _image_to_clipboard(img_path)
                if sys.platform.startswith("darwin"):
                    pyautogui.hotkey("command", "v")
                else:
                    pyautogui.hotkey("ctrl", "v")
            else:
                print(f"[第 {i+1} 次] (文字) {text}")
                print(f"[第 {i+1} 次] (图片) {img_path.name}")

        elif send_image:          # 原来的“仅图片”路径（或文字不优先）
            if really_send:
                if _image_to_clipboard(img_path):
                    # 粘贴：Windows/Linux 用 Ctrl+V，macOS 用 ⌘+V
                    if sys.platform.startswith("darwin"):
                        pyautogui.hotkey("command", "v")
                    else:
                        pyautogui.hotkey("ctrl", "v")
                else:
                    print("[警告] 图片复制到剪贴板失败，跳过本次发送")
            else:
                print(f"[第 {i+1} 次] (图片) {img_path.name}")

        else:   # 只发送文字
            if really_send:
                _text_to_clipboard_and_paste(text)
            else:
                print(f"[第 {i+1} 次] {text}")

        # ③ 自动发送（Enter / Click）
        if auto_send:
            if send_btn:
                bx, by = send_btn
                pyautogui.click(bx, by)
            else:
                pyautogui.press('enter')

        i += 1

        # ④ 等待下一轮（细粒度，便于随时停止）
        for _ in range(int(interval * 10)):
            if stop_flag.is_set():
                break
            time.sleep(0.1)

    # 循环结束后恢复 UI
    stop_flag.set()
    start_btn.config(state="normal")
    stop_btn.config(state="disabled")
    if HAVE_KEYBOARD:
        keyboard.unhook_all()


# ==================== GUI ====================
app = Tk()
app.title("自定义文本/图片循环输出")
app.resizable(False, False)   # 固定窗口防止破坏布局

# ---------- ttk 主题 ----------
style = ttk.Style()
style.theme_use('clam')
style.configure('TLabel', font=('Segoe UI', 10))
style.configure('TEntry', font=('Segoe UI', 10))
style.configure('TButton', font=('Segoe UI', 10), padding=4)
style.configure('TCheckbutton', font=('Segoe UI', 10), padding=2)

# ---------- 主容器 ----------
PAD_X = 12
PAD_Y = 8

# **在这里声明所有需要的全局 StringVar / BooleanVar**（必须在创建控件之前）
text_var        = StringVar()
pos_var         = StringVar()
send_btn_var    = StringVar()
interval_var    = StringVar(value="1.0")
count_var       = StringVar(value="0")
send_var        = BooleanVar(value=True)          # 实际敲键盘/点击
autosend_var    = BooleanVar(value=False)         # 自动发送（Enter / Click）
send_image_var  = BooleanVar(value=False)         # **新增：是否发送图片**
image_path_var  = StringVar()                     # **新增：图片文件路径**
# ==== 改动点 ==== #
send_text_first_var = BooleanVar(value=False)    # “文字优先” 勾选框（默认关闭）
# ==== 改动点结束 ==== #

# 1️⃣ 内容区（文本 & 图片） -------------------------------------------------
txt_frame = ttk.LabelFrame(app, text="  内容区  ", padding=(PAD_X, PAD_Y))
txt_frame.grid(row=0, column=0, padx=PAD_X, pady=PAD_Y, sticky="ew")

ttk.Label(txt_frame, text="要输出的文本：").grid(row=0, column=0, sticky=W, padx=5, pady=3)
ttk.Entry(txt_frame, textvariable=text_var, width=50).grid(row=0, column=1, columnspan=3, sticky=W, padx=5, pady=3)

ttk.Checkbutton(txt_frame, text="发送图片（优先）", variable=send_image_var)\
    .grid(row=1, column=0, sticky=W, padx=5, pady=3)

# ==== 改动点 ==== #
ttk.Checkbutton(txt_frame, text="文字优先（先粘贴文字再粘贴图片）", variable=send_text_first_var)\
    .grid(row=1, column=1, sticky=W, padx=5, pady=3)
# ==== 改动点结束 ==== #

ttk.Entry(txt_frame, textvariable=image_path_var, width=35, state="readonly")\
    .grid(row=1, column=2, sticky=W, padx=5, pady=3)
ttk.Button(txt_frame, text="选择图片", command=lambda: image_path_var.set(
    filedialog.askopenfilename(
        title="请选择要发送的图片",
        filetypes=[("图片文件", "*.png;*.jpg;*.jpeg;*.bmp;*.gif"), ("全部文件", "*.*")]
    )
)).grid(row=1, column=3, sticky=W, padx=5, pady=3)

# 2️⃣ 坐标记录区 -------------------------------------------------
pos_frame = ttk.LabelFrame(app, text="  坐标记录  ", padding=(PAD_X, PAD_Y))
pos_frame.grid(row=1, column=0, padx=PAD_X, pady=PAD_Y, sticky="ew")

ttk.Label(pos_frame, text="编辑框坐标 (x, y)：").grid(row=0, column=0, sticky=W, padx=5, pady=3)
ttk.Entry(pos_frame, textvariable=pos_var, width=20, state="readonly")\
    .grid(row=0, column=1, sticky=W, padx=5, pady=3)
ttk.Button(pos_frame, text="记录编辑框坐标",
           command=lambda: record_position(pos_var, "编辑框"))\
    .grid(row=0, column=2, padx=5, pady=3)

ttk.Label(pos_frame, text="发送按钮坐标 (可选)：").grid(row=1, column=0, sticky=W, padx=5, pady=3)
ttk.Entry(pos_frame, textvariable=send_btn_var, width=20, state="readonly")\
    .grid(row=1, column=1, sticky=W, padx=5, pady=3)
ttk.Button(pos_frame, text="记录发送按钮",
           command=lambda: record_position(send_btn_var, "发送按钮"))\
    .grid(row=1, column=2, padx=5, pady=3)

# 3️⃣ 循环参数区 -------------------------------------------------
param_frame = ttk.LabelFrame(app, text="  循环参数  ", padding=(PAD_X, PAD_Y))
param_frame.grid(row=2, column=0, padx=PAD_X, pady=PAD_Y, sticky="ew")

ttk.Label(param_frame, text="间隔 (秒)：").grid(row=0, column=0, sticky=W, padx=5, pady=3)
ttk.Entry(param_frame, textvariable=interval_var, width=10).grid(row=0, column=1, sticky=W, padx=5, pady=3)

ttk.Label(param_frame, text="循环次数 (0=无限)：").grid(row=0, column=2, sticky=W, padx=5, pady=3)
ttk.Entry(param_frame, textvariable=count_var, width=10).grid(row=0, column=3, sticky=W, padx=5, pady=3)

# 4️⃣ 开关区 -------------------------------------------------
switch_frame = ttk.LabelFrame(app, text="  开关  ", padding=(PAD_X, PAD_Y))
switch_frame.grid(row=3, column=0, padx=PAD_X, pady=PAD_Y, sticky="ew")

ttk.Checkbutton(switch_frame, text="实际发送（键盘/鼠标）", variable=send_var)\
    .grid(row=0, column=0, sticky=W, padx=10, pady=3)
ttk.Checkbutton(switch_frame, text="自动发送 (Enter / 点击)", variable=autosend_var)\
    .grid(row=0, column=1, sticky=W, padx=10, pady=3)

# 5️⃣ 开始 / 停止 按钮 -------------------------------------------------
action_frame = Frame(app)
action_frame.grid(row=4, column=0, pady=12)

start_btn = ttk.Button(action_frame, text="开始循环", width=14, command=start_output)
start_btn.grid(row=0, column=0, padx=10)

stop_btn = ttk.Button(action_frame, text="停止", width=14, command=stop_output, state="disabled")
stop_btn.grid(row=0, column=1, padx=10)

# --------- 退出清理 ----------
def on_closing():
    if messagebox.askokcancel("退出", "确定要退出吗？"):
        stop_flag.set()
        if HAVE_KEYBOARD:
            keyboard.unhook_all()
        app.destroy()

app.protocol("WM_DELETE_WINDOW", on_closing)

# ---------- 运行 ----------
app.mainloop()