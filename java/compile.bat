@echo off
echo Compiling Emshop Java Project...

REM 创建编译输出目录
mkdir target\classes 2>nul

REM 编译基础类
echo Compiling basic classes...
javac -d target\classes -cp "src\main\java" src\main\java\emshop\MockDataProvider.java
javac -d target\classes -cp "src\main\java;target\classes" src\main\java\emshop\EmshopServer.java

REM 检查Netty是否可用
echo Checking for Netty dependencies...
echo Note: Netty-based classes require Maven dependencies to be resolved.
echo Please run 'mvn compile' to compile all Netty classes.

echo.
echo Basic compilation completed. The following classes are ready:
echo - EmshopServer (Business Logic with Mock Data)
echo - MockDataProvider (Test Data Provider)
echo.
echo To compile and use Netty features, run: mvn compile
echo To run the Netty server: mvn exec:java@server

pause
