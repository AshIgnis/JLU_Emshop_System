@echo off
REM JLU Emshop System - 一键构建与测试脚本
REM 功能: 编译C++ JNI库 -> 运行Java单元测试 -> 生成测试报告
REM 
REM 作者: JLU Emshop Team
REM 日期: 2025-10-13
REM 版本: 1.0.0

setlocal enabledelayedexpansion

echo ============================================
echo   JLU Emshop系统 - 一键构建与测试
echo ============================================
echo.

set START_TIME=%TIME%
set ERROR_COUNT=0

REM 步骤1: 检查环境
echo [1/4] 检查构建环境...
echo.

REM 检查Java
java --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ❌ 错误: 未找到Java环境
    set /a ERROR_COUNT+=1
    goto :end_check
)
echo ✓ Java环境正常

REM 检查Maven
mvn --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ❌ 错误: 未找到Maven构建工具
    set /a ERROR_COUNT+=1
    goto :end_check
)
echo ✓ Maven构建工具正常

REM 检查g++
g++ --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ❌ 错误: 未找到g++编译器
    set /a ERROR_COUNT+=1
    goto :end_check
)
echo ✓ g++编译器正常

:end_check
if %ERROR_COUNT% neq 0 (
    echo.
    echo ❌ 环境检查失败，请安装缺失的工具后重试
    goto :failure
)

echo.
echo ============================================

REM 步骤2: 编译C++ JNI库
echo.
echo [2/4] 编译C++ JNI库...
echo.

cd /d "%~dp0cpp"
if not exist "build_oop_jni.bat" (
    echo ❌ 错误: 未找到构建脚本 build_oop_jni.bat
    goto :failure
)

call build_oop_jni.bat
if %errorlevel% neq 0 (
    echo ❌ C++ JNI库编译失败
    goto :failure
)

if not exist "emshop_native_oop.dll" (
    echo ❌ 错误: DLL文件未生成
    goto :failure
)

echo ✓ C++ JNI库编译成功
echo.
echo ============================================

REM 步骤3: 复制DLL到Java项目
echo.
echo [3/4] 部署JNI库到Java项目...
echo.

cd /d "%~dp0"

REM 创建lib目录（如果不存在）
if not exist "java\src\main\resources\lib" mkdir "java\src\main\resources\lib"

REM 复制DLL和依赖
copy /Y "cpp\emshop_native_oop.dll" "java\src\main\resources\lib\" >nul
if %errorlevel% neq 0 (
    echo ❌ DLL复制失败
    goto :failure
)

REM 复制MySQL依赖
if exist "cpp\libmysql.dll" (
    copy /Y "cpp\libmysql.dll" "java\src\main\resources\lib\" >nul
)

echo ✓ JNI库部署成功
echo   位置: java\src\main\resources\lib\emshop_native_oop.dll
echo.
echo ============================================

REM 步骤4: 运行Java单元测试
echo.
echo [4/4] 运行Java单元测试...
echo.

cd /d "%~dp0java"

echo 执行命令: mvn clean test
echo.

mvn clean test
set TEST_RESULT=%errorlevel%

echo.
echo ============================================

REM 结果汇总
echo.
echo 构建与测试完成
echo.

set END_TIME=%TIME%
echo 开始时间: %START_TIME%
echo 结束时间: %END_TIME%
echo.

if %TEST_RESULT% equ 0 (
    echo ✅ 所有测试通过！
    echo.
    echo 📊 测试报告位置:
    echo    - Surefire报告: java\target\surefire-reports\
    echo    - 文本报告: java\target\surefire-reports\*.txt
    echo    - XML报告: java\target\surefire-reports\*.xml
    echo.
    echo 📦 构建产物:
    echo    - JNI库: cpp\emshop_native_oop.dll
    echo    - Java类: java\target\classes\
    echo.
    goto :success
) else (
    echo ❌ 测试失败！请检查测试报告
    echo.
    echo 📊 失败详情请查看:
    echo    java\target\surefire-reports\
    echo.
    goto :failure
)

:success
echo ============================================
echo   🎉 构建与测试成功！
echo ============================================
pause
exit /b 0

:failure
echo ============================================
echo   ❌ 构建与测试失败！
echo ============================================
pause
exit /b 1
