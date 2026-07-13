#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os

def main():
    print(">>> 当前 Python 解释器信息")
    print(sys.version)
    print(">>> sys.executable:", sys.executable)

    try:
        import win32clipboard, win32con
        print("✅ pywin32 已成功导入")
    except Exception as e:
        print("❌ pywin32 导入失败：", e)
        sys.exit(1)

    # 简单写入一个纯白 1×1 PNG 到剪贴板，检查 DIB 写入是否成功
    try:
        from PIL import Image
        from io import BytesIO
        img = Image.new("RGB", (1, 1), (255, 255, 255))
        output = BytesIO()
        img.save(output, format="BMP")
        dib = output.getvalue()[14:]            # 去掉 BMP Header
        output.close()

        win32clipboard.OpenClipboard()
        win32clipboard.EmptyClipboard()
        win32clipboard.SetClipboardData(win32con.CF_DIB, dib)
        win32clipboard.CloseClipboard()
        print("✅ 已把 1×1 白色位图写入剪贴板（未报错）")
    except Exception as e:
        print("❌ 写入剪贴板时出错：", e)
        sys.exit(1)

if __name__ == "__main__":
    main()