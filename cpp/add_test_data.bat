@echo off
REM ============================================
REM JLU Emshop System - 添加测试数据脚本
REM 功能: 向数据库添加丰富的测试数据
REM ============================================

echo.
echo ========================================
echo   添加 Emshop 测试数据
echo ========================================
echo.

REM 检查 SQL 文件是否存在
if not exist "add_test_data.sql" (
    echo [错误] 找不到 add_test_data.sql 文件！
    echo 请确保在 cpp 目录下运行此脚本。
    pause
    exit /b 1
)

REM 从 config.json 读取数据库配置（可选，也可以直接硬编码）
echo [信息] 准备连接到 MySQL 数据库...
echo.

REM 设置数据库连接参数
set DB_HOST=localhost
set DB_PORT=3306
set DB_USER=root
set DB_NAME=emshop

REM 提示输入密码（安全考虑）
set /p DB_PASSWORD="请输入 MySQL root 密码: "

echo.
echo [信息] 正在执行 SQL 脚本...
echo.

REM 执行 SQL 脚本
mysql -h %DB_HOST% -P %DB_PORT% -u %DB_USER% -p%DB_PASSWORD% %DB_NAME% < add_test_data.sql

if %errorlevel% equ 0 (
    echo.
    echo ========================================
    echo   ✅ 测试数据添加成功！
    echo ========================================
    echo.
    echo 已添加的数据包括:
    echo   - 5 个测试用户 ^(密码: 123456^)
    echo   - 30+ 种商品 ^(包含高库存和低库存商品^)
    echo   - 多种类别: 电子产品、图书、服装、食品、家居
    echo   - 收货地址、购物车、订单、评论
    echo   - 用户通知 ^(已读和未读^)
    echo   - 优惠券
    echo.
    echo 测试账号:
    echo   用户名: testuser1 / testuser2 / testuser3
    echo   密码: 123456
    echo.
) else (
    echo.
    echo ========================================
    echo   ❌ 数据添加失败！
    echo ========================================
    echo.
    echo 可能的原因:
    echo   1. MySQL 密码错误
    echo   2. 数据库不存在或连接失败
    echo   3. SQL 语法错误
    echo.
    echo 请检查错误信息并重试。
    echo.
)

pause
