@echo off
echo ==========================================
echo JLU Emshop System - JNI 编译脚本 (Windows)
echo 数据库连接: 127.0.0.1:3306/emshop_db
echo ==========================================

REM 设置环境变量
set JAVA_HOME=C:\Program Files\Java\jdk-21
set MYSQL_HOME=D:\MySQL
set VCPKG_ROOT=D:\vcpkg

REM 检查必要的环境
echo 检查编译环境...

if not exist "%JAVA_HOME%" (
    echo 错误: 未找到 JAVA_HOME: %JAVA_HOME%
    echo 请确保安装了 JDK 21 并设置正确的路径
    pause
    exit /b 1
)

if not exist "%MYSQL_HOME%" (
    echo 错误: 未找到 MYSQL_HOME: %MYSQL_HOME%
    echo 请确保安装了 MySQL 并设置正确的路径
    pause
    exit /b 1
)

REM 创建输出目录
if not exist bin mkdir bin

echo.
echo 开始编译 JNI 库...

REM 编译命令
g++ -shared -fPIC ^
    -I"%JAVA_HOME%\include" ^
    -I"%JAVA_HOME%\include\win32" ^
    -I"%MYSQL_HOME%\include" ^
    -I"%VCPKG_ROOT%\installed\x64-windows\include" ^
    -L"%MYSQL_HOME%\lib" ^
    -L"%VCPKG_ROOT%\installed\x64-windows\lib" ^
    -ljsoncpp -lmysqlclient ^
    -o bin\emshop.dll ^
    emshop_native_impl.cpp

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ==========================================
    echo 编译成功! 
    echo 输出文件: bin\emshop.dll
    echo ==========================================
    
    REM 复制DLL到Java项目的lib目录
    if not exist ..\java\lib mkdir ..\java\lib
    copy bin\emshop.dll ..\java\lib\
    
    echo DLL已复制到Java项目目录
    
    REM 复制MySQL的DLL依赖
    if exist "%MYSQL_HOME%\lib\libmysql.dll" (
        copy "%MYSQL_HOME%\lib\libmysql.dll" ..\java\lib\
        echo MySQL依赖库已复制
    )
    
    echo.
    echo 现在可以运行Java项目测试JNI集成:
    echo cd ..\java
    echo mvn exec:java@server -Dexec.args="8090"
) else (
    echo.
    echo ==========================================
    echo 编译失败! 错误代码: %ERRORLEVEL%
    echo ==========================================
    echo.
    echo 可能的解决方案:
    echo 1. 检查是否安装了 MinGW-w64 或 MSYS2
    echo 2. 检查是否正确安装了 vcpkg 和依赖库
    echo 3. 确认所有路径设置正确
    echo 4. 运行以下命令安装依赖:
    echo    %VCPKG_ROOT%\vcpkg install jsoncpp:x64-windows
    echo    %VCPKG_ROOT%\vcpkg install libmysql:x64-windows
)

echo.
pause
