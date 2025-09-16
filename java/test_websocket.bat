@echo off
REM WebSocket 连接测试脚本

echo ========================================
echo    WebSocket Connection Test Tool
echo ========================================
echo.

REM 设置默认参数
set SERVER_URL=wss://localhost:8081/ws
set USERNAME=admin
set PASSWORD=admin123

REM 检查是否传入了参数
if NOT "%1"=="" set SERVER_URL=%1
if NOT "%2"=="" set USERNAME=%2
if NOT "%3"=="" set PASSWORD=%3

echo 测试配置:
echo - 服务器: %SERVER_URL%
echo - 用户名: %USERNAME%
echo - 密码: %PASSWORD%
echo.

echo 开始 WebSocket 连接测试...
echo.

mvn exec:java -Dexec.mainClass="emshop.WebSocketTestClient" -Dexec.args="%SERVER_URL% %USERNAME% %PASSWORD%"

echo.
echo 测试完成
pause