@echo off
chcp 65001
cls

echo ===============================================
echo  JLU Emshop 电商系统 - Windows启动脚本
echo  作者: JLU 55240425 屈熙宸  
echo  技术栈: Java 21 + WebSocket + JNI + MySQL
echo ===============================================
echo.

cd /d "d:\codehome\jlu\JLU_Emshop_System\java"

echo 正在编译Java项目...
call mvn compile -q
if %ERRORLEVEL% neq 0 (
    echo ❌ 编译失败，请检查代码
    pause
    exit /b 1
)
echo ✅ 编译成功

echo.
echo 正在启动WebSocket服务器...
echo 服务器地址: ws://localhost:8082/ws
echo 按 Ctrl+C 停止服务器
echo.

java -cp "target/classes;dependency/*" emshop.EmshopWebSocketServer