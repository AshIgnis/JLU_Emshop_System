@echo off
chcp 65001 >nul
echo ============================================
echo   重启Java服务器并验证限购功能
echo ============================================
echo.

echo [1/3] 停止当前运行的Java服务器...
taskkill /F /IM java.exe >nul 2>&1
if %errorlevel%==0 (
    echo ✓ 已停止运行中的Java进程
) else (
    echo ! 未发现运行中的Java进程
)
timeout /t 2 >nul
echo.

echo [2/3] 清理并重新编译Java项目...
cd /d "%~dp0java"
call mvn clean compile
if %errorlevel% neq 0 (
    echo ✗ Maven编译失败
    pause
    exit /b 1
)
echo ✓ Maven编译成功
echo.

echo [3/3] 启动Java服务器...
echo.
echo ============================================
echo   服务器启动中...
echo   端口: 8080
echo   按Ctrl+C停止服务器
echo ============================================
echo.
call mvn exec:java@server

pause
