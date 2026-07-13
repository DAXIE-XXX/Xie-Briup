@echo off
:: ==============================================================
::  一键打包 MessageBombardm.py 为 exe（单文件）
::  适用于 Windows CMD（即命令提示符）
:: ==============================================================
:: 1. 检查 Python 是否可用
python --version >NUL 2>&1
if errorlevel 1 (
    echo 【错误】未检测到 Python，请先安装 Python 并把其加入 PATH。
    pause
    exit /b 1
)

:: 2. 检查 PyInstaller 是否已安装
python -c "import pyinstaller" >NUL 2>&1
if errorlevel 1 (
    echo 【提示】正在安装 PyInstaller...
    pip install -U pyinstaller
    if errorlevel 1 (
        echo 【错误】PyInstaller 安装失败，请检查网络或 pip 配置。
        pause
        exit /b 1
    )
)

:: 3. 项目路径（请根据实际情况修改，如果路径固定可直接写死）
set "PROJECT_ROOT=E:\XIEXIAOXUAN\PythonScript\MessageBombardm"
set "SCRIPT=%PROJECT_ROOT%\MessageBombardm.py"

:: 4. 检查脚本是否存在
if not exist "%SCRIPT%" (
    echo 【错误】找不到脚本文件：%SCRIPT%
    pause
    exit /b 1
)

:: 5. 创建输出目录（可选）
set "DIST_DIR=%PROJECT_ROOT%\dist"
if not exist "%DIST_DIR%" md "%DIST_DIR%"

:: 6. 运行 PyInstaller（单文件、无控制台、图标可自行替换）
echo 正在打包，请稍候...
pyinstaller ^
    --onefile ^
    --noconsole ^
    --distpath "%DIST_DIR%" ^
    --workpath "%PROJECT_ROOT%\build_tmp" ^
    --specpath "%PROJECT_ROOT%" ^
    "%SCRIPT%"

:: 7. 打包结果提示
if exist "%DIST_DIR%\MessageBombardm.exe" (
    echo -------------------------------------------------
    echo 【成功】MessageBombardm.exe 已生成 → %DIST_DIR%
    echo 直接在 CMD 下运行：  %DIST_DIR%\MessageBombardm.exe
) else (
    echo 【错误】打包失败，请查看上面的错误信息。
)

pause
exit /b