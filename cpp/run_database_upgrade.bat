@echo off
REM 数据库升级脚本执行器
REM 使用方法: run_database_upgrade.bat [password]

SET DB_USER=root
SET DB_NAME=emshop
SET SQL_FILE=database_upgrade_v1.1.0.sql

IF "%1"=="" (
    echo 请输入MySQL root密码:
    set /p DB_PASS=密码: 
) ELSE (
    SET DB_PASS=%1
)

echo.
echo ========================================
echo 正在执行数据库升级...
echo 数据库: %DB_NAME%
echo 脚本文件: %SQL_FILE%
echo ========================================
echo.

mysql -u %DB_USER% -p%DB_PASS% %DB_NAME% < %SQL_FILE%

IF %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo 数据库升级成功!
    echo ========================================
) ELSE (
    echo.
    echo ========================================
    echo 数据库升级失败! 错误代码: %ERRORLEVEL%
    echo ========================================
)

pause
