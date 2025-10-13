@echo off
chcp 65001 > nul
setlocal EnableDelayedExpansion

REM =====================================================================
REM JLU Emshop System - 一键数据库初始化脚本
REM 功能: 自动连接MySQL并执行初始化SQL
REM 作者: JLU Emshop Team
REM 日期: 2025-10-13
REM =====================================================================

echo ========================================
echo   JLU Emshop 数据库一键初始化
echo ========================================
echo.

REM 1. 检查MySQL是否安装
echo [1/4] 检查 MySQL 环境...
where mysql >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] 未找到 mysql 命令，请确认 MySQL 已安装并添加到 PATH
    echo 常见安装路径: C:\Program Files\MySQL\MySQL Server 8.0\bin
    pause
    exit /b 1
)

mysql --version
if %errorlevel% neq 0 (
    echo [ERROR] MySQL 命令异常
    pause
    exit /b 1
)
echo [OK] MySQL 环境正常
echo.

REM 2. 读取配置文件
echo [2/4] 读取数据库配置...
set CONFIG_FILE="%~dp0..\config.json"
if not exist %CONFIG_FILE% (
    echo [ERROR] 配置文件不存在: %CONFIG_FILE%
    echo 请先复制 config.example.json 为 config.json 并配置数据库信息
    pause
    exit /b 1
)

REM 简单提取配置（假设标准格式）
set DB_HOST=127.0.0.1
set DB_PORT=3306
set DB_USER=root
set DB_PASS=

REM 提示用户输入（也可以解析JSON，这里简化处理）
echo 当前配置:
echo   主机: %DB_HOST%
echo   端口: %DB_PORT%
echo   用户: %DB_USER%
echo.
echo 如需修改，请直接编辑 config.json
echo.

set /p DB_PASS="请输入 MySQL root 密码: "
if "%DB_PASS%"=="" (
    echo [ERROR] 密码不能为空
    pause
    exit /b 1
)

echo [OK] 配置读取成功
echo.

REM 3. 测试数据库连接
echo [3/4] 测试数据库连接...
mysql -h%DB_HOST% -P%DB_PORT% -u%DB_USER% -p%DB_PASS% -e "SELECT 1;" >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] 数据库连接失败，请检查:
    echo   1. MySQL 服务是否启动
    echo   2. 用户名密码是否正确
    echo   3. 主机和端口是否正确
    pause
    exit /b 1
)
echo [OK] 数据库连接成功
echo.

REM 4. 执行初始化SQL
echo [4/4] 执行数据库初始化...
set SQL_FILE="%~dp0emshop_database_init.sql"
if not exist %SQL_FILE% (
    echo [ERROR] SQL文件不存在: %SQL_FILE%
    pause
    exit /b 1
)

echo 正在执行 SQL 脚本...
mysql -h%DB_HOST% -P%DB_PORT% -u%DB_USER% -p%DB_PASS% < %SQL_FILE%
if %errorlevel% neq 0 (
    echo [ERROR] SQL 执行失败
    pause
    exit /b 1
)

echo [OK] 数据库初始化完成
echo.

REM 5. 验证初始化结果
echo ========================================
echo   初始化结果验证
echo ========================================
mysql -h%DB_HOST% -P%DB_PORT% -u%DB_USER% -p%DB_PASS% emshop -e "SELECT 'Database' as Type, DATABASE() as Name UNION ALL SELECT 'Users', CAST(COUNT(*) AS CHAR) FROM users UNION ALL SELECT 'Categories', CAST(COUNT(*) AS CHAR) FROM categories UNION ALL SELECT 'Products', CAST(COUNT(*) AS CHAR) FROM products UNION ALL SELECT 'Coupons', CAST(COUNT(*) AS CHAR) FROM coupons;"

echo.
echo ========================================
echo   初始化完成！
echo ========================================
echo.
echo 默认账户:
echo   管理员: admin / 123456
echo   测试用户: testuser / 123456
echo.
echo 数据库信息:
echo   地址: %DB_HOST%:%DB_PORT%
echo   数据库: emshop
echo   表数量: 10 张核心表
echo   初始商品: 10 件
echo   初始优惠券: 4 张
echo.
echo 下一步:
echo   1. 编译 C++ JNI 代码: build_oop_jni.bat
echo   2. 启动 Netty 服务器: mvn exec:java
echo   3. 运行 Qt 客户端: qtclient/build/emshop_qtclient.exe
echo.

pause
