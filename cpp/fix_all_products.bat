@echo off
chcp 65001 > nul
echo ========================================
echo 修复所有商品名称数据
echo ========================================
echo.

REM 设置 MySQL 连接参数
set MYSQL_HOST=localhost
set MYSQL_PORT=3306
set MYSQL_USER=root
set MYSQL_PASS=Quxc060122
set MYSQL_DB=emshop

echo 正在连接 MySQL 并执行修复脚本...
echo 这将修复所有包含问号的商品名称...
echo.

mysql -h%MYSQL_HOST% -P%MYSQL_PORT% -u%MYSQL_USER% -p%MYSQL_PASS% %MYSQL_DB% < fix_all_products.sql

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ✅ 所有商品名称修复成功！
    echo.
    echo 修复的商品包括:
    echo   - 手机类: OPPO, 华为, 三星等
    echo   - 电脑类: 华为, 戴尔, 联想, MacBook等
    echo   - 图书类: 深入理解计算机系统, 算法导论, C++ Primer等
    echo   - 服饰类: HM, The North Face, Canada Goose等
    echo   - 家电类: 海尔, 美的, 格力, 西门子等
    echo   - 食品类: 大米, 苹果, 酱油等
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
