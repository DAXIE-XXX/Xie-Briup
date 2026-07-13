#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
hello_spammer.py
----------------
循环向指定坐标或窗口中心输入 "hello"。

* 支持窗口标题定位（需要 pygetwindow）
* 支持手动坐标输入
* 可自定义间隔、循环次数
* 可选 Ctrl+Alt+S 全局热键退出（需要 keyboard，必须以管理员运行）
"""

import sys, time, argparse, threading
import pyautogui

# ---------- 可选依赖的安全导入 ----------
try:
    import pygetwindow as gw
except Exception:
    gw = None
try:
    import keyboard
except Exception:
    keyboard = None

# ---------- 兼容打印函数 ----------
def safe_print(*args, **kwargs):
    """
    在有/无控制台的环境都能安全使用的打印函数。
    - 有控制台时使用 print(..., flush=True) 立即刷新；
    - 没有控制台（--noconsole）时 sys.stdout 为 None，直接跳过 flush。
    """
    print(*args, **kwargs, flush=True)
    if sys.stdout:
        try:
            sys.stdout.flush()
        except Exception:
            pass

# ---------- 工具函数 ----------
def find_window_center(title: str):
    """返回窗口中心坐标 (x, y)，若找不到返回 None"""
    if not gw:
        safe_print("[WARN] 未安装 pygetwindow，窗口定位不可用。")
        return None
    windows = gw.getWindowsWithTitle(title)
    if not windows:
        safe_print(f"[ERROR] 找不到标题包含 “{title}” 的窗口。")
        return None
    w = windows[0]
    return w.left + w.width // 2, w.top + w.height // 2

def wait_for_exit(stop_event):
    """后台线程：监听 Ctrl+Alt+S，收到后设置 stop_event"""
    if not keyboard:
        return
    safe_print("[INFO] 按 Ctrl+Alt+S 可随时停止自动输入")
    keyboard.wait('ctrl+alt+s')
    stop_event.set()
    safe_print("\n[INFO] 接收到停止指令，程序即将退出…")

# ---------- 主函数 ----------
def main():
    parser = argparse.ArgumentParser(description="在指定位置循环输入 'hello'")
    group = parser.add_mutually_exclusive_group()
    group.add_argument("-t", "--title", type=str,
                       help="窗口标题（部分匹配），脚本会在窗口中心输入")
    group.add_argument("-c", "--coords", type=str,
                       help="屏幕坐标，格式 x,y（左上角为原点）")
    parser.add_argument("-i", "--interval", type=float, default=1.0,
                        help="每次输入之间的间隔（秒），默认 15.0")
    parser.add_argument("-n", "--count", type=int, default=0,
                        help="输入次数，0 表示无限循环（默认 0）")
    args = parser.parse_args()

    # ---------- 1️⃣ 解析目标位置 ----------
    if args.title:
        pos = find_window_center(args.title)
        if not pos:
            sys.exit(1)
        safe_print(f"[INFO] 将在标题包含 “{args.title}” 的窗口中心 ({pos[0]}, {pos[1]}) 输入")
    elif args.coords:
        try:
            x_str, y_str = args.coords.split(",")
            pos = (int(x_str.strip()), int(y_str.strip()))
        except Exception:
            safe_print("[ERROR] 坐标格式错误，请使用 x,y 形式，例如 500,300")
            sys.exit(1)
        safe_print(f"[INFO] 将在坐标 ({pos[0]}, {pos[1]}) 输入")
    else:
        # 交互式选择
        safe_print("\n请选择输入方式：")
        safe_print("  1) 根据窗口标题定位")
        safe_print("  2) 手动输入屏幕坐标")
        choice = input("输入 1 或 2（默认 2）: ").strip()
        if choice == "1":
            title = input("请输入窗口标题关键字: ").strip()
            pos = find_window_center(title)
            if not pos:
                sys.exit(1)
            safe_print(f"[INFO] 将在窗口中心 ({pos[0]}, {pos[1]}) 输入")
        else:
            coord_str = input("请输入坐标 x,y（左上角为原点）: ").strip()
            try:
                x_str, y_str = coord_str.split(",")
                pos = (int(x_str.strip()), int(y_str.strip()))
            except Exception:
                safe_print("[ERROR] 坐标格式错误")
                sys.exit(1)

    # ---------- 2️⃣ 参数 ----------
    interval = max(0.05, args.interval)   # 防止间隔为 0 导致 CPU 飙升
    total = max(0, args.count)

    # ---------- 3️⃣ 启动热键监听（可选） ----------
    stop_event = threading.Event()
    if keyboard:
        threading.Thread(target=wait_for_exit,
                         args=(stop_event,),
                         daemon=True).start()

    # ---------- 4️⃣ 循环输入 ----------
    safe_print("[INFO] 开始自动输入（Ctrl+Alt+S 退出），Ctrl+C 亦可中断")
    count = 0
    try:
        while not stop_event.is_set():
            pyautogui.moveTo(pos[0], pos[1], duration=0.1)
            pyautogui.typewrite("Are you a waste?", interval=0.05)   # 如需其它文字自行改
            pyautogui.press('enter')   # 若想回车请取消注释

            count += 1
            if total and count >= total:
                safe_print(f"\n[INFO] 已完成 {total} 次，程序结束。")
                break
            time.sleep(interval)
    except KeyboardInterrupt:
        safe_print("\n[INFO] 手动 Ctrl+C 中断，程序退出。")
    except Exception as e:
        safe_print(f"\n[ERROR] 运行时异常: {e}")

if __name__ == "__main__":
    main()