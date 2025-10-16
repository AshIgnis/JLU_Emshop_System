@echo off
chcp 65001 >nul
echo.
echo ================================================
echo    🎨 Emshop Qt客户端
echo ================================================
echo.
echo 正在启动客户端...
echo.

cd /d "%~dp0qtclient\build\src"

if exist "emshop_qtclient.exe" (
    echo ✅ 找到可执行文件
    echo 📍 路径: %CD%\emshop_qtclient.exe
    echo.
    
    REM 设置Qt运行时路径
    set PATH=D:\Qt\6.9.1\mingw_64\bin;%PATH%
    
    start "" "emshop_qtclient.exe"
    
    echo ✨ 客户端已启动！
    echo.
    echo UI改进包括:
    echo   • 渐变背景登录界面
    echo   • 现代化主窗口设计
    echo   • 深色菜单栏和状态栏
    echo   • 美化的表格和按钮
    echo   • 圆角输入框和卡片布局
    echo.
    timeout /t 3 >nul
) else (
    echo ❌ 错误: 未找到可执行文件
    echo 请先编译项目:
    echo   cd qtclient\build
    echo   mingw32-make -j8
    echo.
    pause
)
