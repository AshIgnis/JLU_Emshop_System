@echo off
REM Qt 客户端构建脚本

echo ========================================
echo    Emshop Qt Client Build Script
echo ========================================
echo.

REM 检查 Qt 环境
where qmake >nul 2>&1
if errorlevel 1 (
    echo 错误: 未找到 qmake，请确保 Qt 已安装并添加到 PATH
    echo 建议设置 Qt 环境变量:
    echo   set PATH=C:\Qt\6.5.0\msvc2019_64\bin;%%PATH%%
    echo.
    pause
    exit /b 1
)

REM 检查 CMake
where cmake >nul 2>&1
if errorlevel 1 (
    echo 错误: 未找到 cmake，请安装 CMake 并添加到 PATH
    pause
    exit /b 1
)

echo Qt 和 CMake 检测成功
echo.

REM 创建构建目录
if not exist "build" mkdir build
cd build

echo 正在配置项目...
cmake .. -DCMAKE_BUILD_TYPE=Release

if errorlevel 1 (
    echo 错误: CMake 配置失败
    cd ..
    pause
    exit /b 1
)

echo.
echo 正在编译项目...
cmake --build . --config Release

if errorlevel 1 (
    echo 错误: 编译失败
    cd ..
    pause
    exit /b 1
)

echo.
echo ✓ 编译成功！
echo 可执行文件位置: build\bin\EmshopQtClient.exe
echo.

REM 询问是否运行
choice /M "是否立即运行客户端"
if errorlevel 2 goto :end

echo 正在启动客户端...
bin\EmshopQtClient.exe

:end
cd ..
echo.
echo 构建脚本执行完毕
pause