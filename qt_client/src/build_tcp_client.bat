@echo off
echo ====================================
echo JLU 电商系统 - Qt TCP客户端构建脚本
echo 兼容你的原有 EmshopNettyServer.java
echo ====================================

REM 检查Qt环境
where qmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo [错误] 未找到qmake，请确保Qt已正确安装并在PATH中
    pause
    exit /b 1
)

REM 检查编译器
where g++ >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo [错误] 未找到g++编译器
    pause
    exit /b 1
)

echo [信息] Qt环境已配置
qmake --version

REM 创建构建目录
if not exist build mkdir build
cd build

echo [构建] 生成Makefile...
qmake ../CMakeLists.txt -o Makefile
if %ERRORLEVEL% neq 0 (
    echo [错误] qmake生成失败
    cd ..
    pause
    exit /b 1
)

echo [构建] 编译项目...
make
if %ERRORLEVEL% neq 0 (
    echo [错误] 编译失败
    cd ..
    pause
    exit /b 1
)

cd ..
echo.
echo ====================================
echo ✅ 构建完成！
echo 🔗 TCP客户端已准备就绪
echo 🌐 可以连接到你的EmshopNettyServer.java
echo ====================================
echo.
echo 使用方法：
echo 1. 先启动你的Java Netty服务器（EmshopNettyServer.java）
echo 2. 运行 build/emshop_client.exe
echo 3. 在登录界面输入服务器地址（如：localhost:8081）
echo 4. 点击"连接服务器"建立TCP连接
echo 5. 使用你的用户名密码登录
echo.
pause