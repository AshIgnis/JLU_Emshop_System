@echo off
REM JLU Emshop System - 面向对象JNI实现编译脚本
REM 使用Java 21和现代C++17标准
REM 
REM 作者: JLU Emshop Team
REM 日期: 2025-08-31
REM 版本: 2.0.0

echo ============================================
echo   JLU Emshop系统 - OOP JNI 实现编译
echo ============================================
echo.

REM 检查Java环境
echo [1/5] 检查Java环境...
java --version >nul 2>&1
if %errorlevel% neq 0 (
    echo 错误: 未找到Java环境
    pause
    exit /b 1
)
echo ✓ Java环境正常

REM 检查编译器
echo.
echo [2/5] 检查编译器...
g++ --version >nul 2>&1
if %errorlevel% neq 0 (
    echo 错误: 未找到g++编译器
    pause
    exit /b 1
)
echo ✓ g++编译器正常

REM 检查依赖文件
echo.
echo [3/5] 检查依赖文件...
if not exist "emshop_native_impl_oop.cpp" (
    echo 错误: 未找到源文件 emshop_native_impl_oop.cpp
    pause
    exit /b 1
)
if not exist "emshop_EmshopNativeInterface.h" (
    echo 错误: 未找到头文件 emshop_EmshopNativeInterface.h
    pause
    exit /b 1
)
if not exist "nlohmann_json.hpp" (
    echo 错误: 未找到JSON库头文件 nlohmann_json.hpp
    pause
    exit /b 1
)
if not exist "libmysql.dll" (
    echo 错误: 未找到MySQL库文件 libmysql.dll
    pause
    exit /b 1
)
echo ✓ 依赖文件完整

REM 清理旧文件
echo.
echo [4/5] 清理旧文件...
if exist "emshop_native_oop.dll" del "emshop_native_oop.dll"
if exist "emshop_native_oop.dll.manifest" del "emshop_native_oop.dll.manifest"
echo ✓ 旧文件已清理

REM 开始编译
echo.
echo [5/5] 开始编译...
echo 编译命令:
echo g++ -std=c++17 -shared -O2 -DNDEBUG ^
echo     "-IC:\Program Files\Java\jdk-21\include" ^
echo     "-IC:\Program Files\Java\jdk-21\include\win32" ^
echo     -I"D:\MySQL\include" ^
echo     -o emshop_native_oop.dll ^
echo     emshop_native_impl_oop.cpp libmysql.dll
echo.

g++ -std=c++17 -shared -O2 -DNDEBUG ^
    "-IC:\Program Files\Java\jdk-21\include" ^
    "-IC:\Program Files\Java\jdk-21\include\win32" ^
    -I"D:\MySQL\include" ^
    -o emshop_native_oop.dll ^
    emshop_native_impl_oop.cpp libmysql.dll

if %errorlevel% neq 0 {
    echo.
    echo ❌ 编译失败！
    echo 请检查错误信息并修复问题。
    pause
    exit /b 1
}

REM 编译成功检查
echo.
if exist "emshop_native_oop.dll" {
    echo ✅ 编译成功！
    echo.
    echo 生成的文件信息:
    dir emshop_native_oop.dll
    echo.
    echo 🎉 JNI库已准备就绪！
    echo 文件位置: %CD%\emshop_native_oop.dll
    echo.
    echo 使用说明:
    echo 1. 将 emshop_native_oop.dll 复制到Java项目的库路径
    echo 2. 确保 libmysql.dll 在系统PATH中或与程序在同一目录
    echo 3. 在Java中调用 System.loadLibrary("emshop_native_oop")
    echo 4. 使用EmshopNativeInterface类的静态方法
 } else {
    echo ❌ 编译失败！未生成DLL文件。
}

echo.
echo 编译完成。
pause
