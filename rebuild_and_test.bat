@echo off
echo ========================================
echo  JLU Emshop 系统 - 快速重新编译和测试
echo ========================================
echo.

echo [1/4] 重新编译 C++ DLL...
cd cpp
call build_oop_jni.bat
if %errorlevel% neq 0 (
    echo ❌ C++ 编译失败！
    pause
    exit /b 1
)
cd ..

echo.
echo [2/4] 复制 DLL 到 Java 项目...
copy /Y cpp\emshop_native_oop.dll java\src\emshop_native_oop.dll
if %errorlevel% neq 0 (
    echo ❌ 复制 DLL 到 Java 失败！
    pause
    exit /b 1
)

echo.
echo [3/4] 清理并重新编译 Qt 客户端...
echo 清理旧的构建文件...
if exist qtclient\build rmdir /s /q qtclient\build
mkdir qtclient\build
copy /Y cpp\emshop_native_oop.dll qtclient\build\
copy /Y cpp\libmysql.dll qtclient\build\

echo 配置 Qt 项目...
cmake -S qtclient -B qtclient\build -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=D:/Qt/6.9.1/mingw_64
if %errorlevel% neq 0 (
    echo ❌ Qt 配置失败！
    pause
    exit /b 1
)

echo 构建 Qt 项目...
cmake --build qtclient\build -- -j
if %errorlevel% neq 0 (
    echo ❌ Qt 编译失败！
    pause
    exit /b 1
)

echo 复制 DLL 到 Qt 输出目录...
copy /Y cpp\emshop_native_oop.dll qtclient\build\src\
copy /Y cpp\libmysql.dll qtclient\build\src\

echo.
echo [4/4] 验证编译结果...
if not exist "java\src\emshop_native_oop.dll" (
    echo ❌ Java 目录中找不到 DLL！
    pause
    exit /b 1
)
if not exist "qtclient\build\src\emshop_qtclient.exe" (
    echo ❌ Qt 可执行文件未生成！
    pause
    exit /b 1
)
if not exist "qtclient\build\src\emshop_native_oop.dll" (
    echo ❌ Qt 输出目录中找不到 DLL！
    pause
    exit /b 1
)

echo.
echo ✅ 所有组件编译成功！
echo.
echo 接下来的步骤：
echo 1. 在新终端运行: cd java ^&^& mvn exec:java@server
echo 2. 在新终端运行: cd qtclient\build\src ^&^& emshop_qtclient.exe
echo 3. 测试促销创建和退款审批功能
echo.
pause
