@echo off
chcp 65001 > nul
echo ========================================
echo 修复商品名称数据
echo ========================================
echo.

REM 设置 MySQL 连接参数
set MYSQL_HOST=localhost
set MYSQL_PORT=3306
set MYSQL_USER=root
set MYSQL_PASS=Quxc060122
set MYSQL_DB=emshop

echo 正在连接 MySQL 并执行修复脚本...
echo.

mysql -h%MYSQL_HOST% -P%MYSQL_PORT% -u%MYSQL_USER% -p%MYSQL_PASS% %MYSQL_DB% < fix_product_names.sql

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ✅ 商品名称修复成功！
    echo.
) else (
    echo.
    echo ❌ 修复失败！错误代码: %ERRORLEVEL%
    echo.
    echo 请检查:
    echo   1. MySQL 服务是否正在运行
    echo   2. 数据库连接参数是否正确
    echo   3. 用户是否有足够的权限
    echo.
)

pause
