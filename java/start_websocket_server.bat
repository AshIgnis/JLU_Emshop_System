@echo off
REM Emshop WebSocket 服务器启动脚本

echo ========================================
echo    Emshop WebSocket Server Launcher
echo ========================================
echo.

REM 设置默认参数
set PORT=8081
set SSL=true

REM 检查是否传入了端口参数
if NOT "%1"=="" set PORT=%1
if NOT "%2"=="" set SSL=%2

echo 启动配置:
echo - 端口: %PORT%
echo - SSL: %SSL%
echo - URL: wss://localhost:%PORT%/ws
echo.

REM 检查 JNI 库文件
if not exist "emshop_native_oop.dll" (
    echo 警告: 未找到 JNI 库文件 emshop_native_oop.dll
    echo 请确保已编译 C++ JNI 库
    echo.
)

if not exist "libmysql.dll" (
    echo 警告: 未找到 MySQL 库文件 libmysql.dll
    echo 请确保 MySQL 客户端库已安装
    echo.
)

REM 启动服务器
echo 正在启动 WebSocket 服务器...
echo.
mvn exec:java -Dexec.mainClass="emshop.EmshopWebSocketServer" -Dexec.args="%PORT% %SSL%"

REM 如果 Maven 不可用，尝试直接使用编译后的类
if errorlevel 1 (
    echo.
    echo Maven 启动失败，尝试使用预编译的 JAR 文件...
    if exist "target\emshop-server.jar" (
        java -jar target\emshop-server.jar %PORT% %SSL%
    ) else (
        echo 错误: 未找到编译后的 JAR 文件
        echo 请先运行: mvn clean package
    )
)

echo.
echo 服务器已停止
pause