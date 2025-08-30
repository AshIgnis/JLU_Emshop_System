@echo off
:: JLU Emshop System - 启动脚本
:: 用于启动Netty服务器和客户端

cd /d "%~dp0"

echo ================================
echo JLU Emshop System Launcher
echo ================================
echo.
echo 请选择操作:
echo 1. 编译项目
echo 2. 启动服务器
echo 3. 启动客户端
echo 4. 清理项目
echo 5. 退出
echo.
set /p choice="请输入选择 (1-5): "

if "%choice%"=="1" goto compile
if "%choice%"=="2" goto server
if "%choice%"=="3" goto client  
if "%choice%"=="4" goto clean
if "%choice%"=="5" goto exit
echo 无效选择，请重新运行脚本
pause
goto end

:compile
echo.
echo 正在编译项目...
call mvn clean compile
if %errorlevel% neq 0 (
    echo 编译失败！请检查错误信息。
    pause
    goto end
)
echo 编译成功！
pause
goto end

:server
echo.
echo 正在启动Emshop服务器...
echo 服务器将在端口 8080 启动
echo 按 Ctrl+C 停止服务器
echo.
call mvn exec:java@server
goto end

:client
echo.
echo 正在启动Emshop客户端...
echo 将连接到 localhost:8080
echo.
call mvn exec:java@client
goto end

:clean
echo.
echo 正在清理项目...
call mvn clean
echo 清理完成！
pause
goto end

:exit
echo 退出程序
goto end

:end
