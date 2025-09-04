@echo off
echo ====================================================================
echo JLU Emshop System - Database Initialization Script
echo ====================================================================
echo.

set MYSQL_HOST=127.0.0.1
set MYSQL_PORT=3306
set MYSQL_USER=root

echo Please make sure MySQL is running on %MYSQL_HOST%:%MYSQL_PORT%
echo.

:: 获取MySQL root密码
set /p MYSQL_PASSWORD="Enter MySQL root password: "

echo.
echo Connecting to MySQL...
echo.

:: 执行数据库初始化
mysql -h%MYSQL_HOST% -P%MYSQL_PORT% -u%MYSQL_USER% -p%MYSQL_PASSWORD% < emshop_database_init.sql

if %errorlevel% neq 0 (
    echo.
    echo [ERROR] Database initialization failed!
    echo Please check:
    echo 1. MySQL service is running
    echo 2. Connection parameters are correct
    echo 3. User has sufficient privileges
    pause
    exit /b 1
)

echo.
echo ====================================================================
echo Database initialization completed successfully!
echo ====================================================================
echo.

:: 验证数据库
echo Verifying database setup...
mysql -h%MYSQL_HOST% -P%MYSQL_PORT% -u%MYSQL_USER% -p%MYSQL_PASSWORD% < verify_database_connection.sql

echo.
echo ====================================================================
echo Verification completed!
echo ====================================================================
echo.
echo Database: emshop
echo Connection: %MYSQL_HOST%:%MYSQL_PORT%
echo.
echo Default accounts:
echo   Admin: admin / 123456
echo   Test User: testuser / 123456
echo.
echo You can now use the emshop database in your application.
echo.
pause
