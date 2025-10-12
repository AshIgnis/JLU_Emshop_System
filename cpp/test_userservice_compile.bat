@echo off
chcp 65001 >nul
echo ================================
echo UserService模块编译测试
echo ================================

set JAVA_HOME=C:\Program Files\Java\jdk-21
set MINGW_HOME=D:\mingw\x86_64-15.2.0-release-win32-seh-ucrt-rt_v13-rev0\mingw64
set MYSQL_HOME=D:\MySQL

set PATH=%MINGW_HOME%\bin;%PATH%

echo.
echo [1/3] 编译UserService.cpp...
g++ -std=c++17 -c^
  -I"%JAVA_HOME%\include"^
  -I"%JAVA_HOME%\include\win32"^
  -I"%MYSQL_HOME%\include"^
  -I".."^
  services\UserService.cpp^
  -o services\UserService.o^
  -DWIN32_LEAN_AND_MEAN -DNOMINMAX
  
if errorlevel 1 (
    echo [ERROR] UserService编译失败!
    pause
    exit /b 1
)

echo [SUCCESS] UserService编译成功!
echo.
echo 生成的目标文件: services\UserService.o

dir services\UserService.o

echo.
echo ================================
echo 编译测试完成!
echo ================================
pause
