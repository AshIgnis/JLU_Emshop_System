@echo off
REM ============================================
REM 优惠券过期检查脚本
REM ============================================
REM 功能: 连接MySQL数据库执行优惠券过期检查
REM 建议: 使用Windows任务计划程序每天自动执行
REM ============================================

setlocal enabledelayedexpansion

echo ============================================
echo   优惠券过期检查 - JLU Emshop System
echo ============================================
echo.

REM MySQL连接配置(请根据实际情况修改)
set MYSQL_HOST=localhost
set MYSQL_PORT=3306
set MYSQL_USER=root
set MYSQL_PASSWORD=your_password_here
set MYSQL_DATABASE=emshop

REM MySQL路径
set MYSQL_PATH=D:\MySQL\bin\mysql.exe

REM 检查MySQL是否存在
if not exist "%MYSQL_PATH%" (
    echo [错误] MySQL客户端未找到: %MYSQL_PATH%
    echo 请修改脚本中的MYSQL_PATH变量
    pause
    exit /b 1
)

echo [1/3] 检查MySQL连接...
"%MYSQL_PATH%" -h%MYSQL_HOST% -P%MYSQL_PORT% -u%MYSQL_USER% -p%MYSQL_PASSWORD% -e "SELECT 1" >nul 2>&1
if errorlevel 1 (
    echo [错误] MySQL连接失败,请检查连接配置
    pause
    exit /b 1
)
echo [✓] MySQL连接成功

echo.
echo [2/3] 执行优惠券过期检查...
"%MYSQL_PATH%" -h%MYSQL_HOST% -P%MYSQL_PORT% -u%MYSQL_USER% -p%MYSQL_PASSWORD% %MYSQL_DATABASE% < expire_coupons_check.sql
if errorlevel 1 (
    echo [错误] 执行SQL脚本失败
    pause
    exit /b 1
)
echo [✓] 优惠券过期检查完成

echo.
echo [3/3] 生成执行日志...
set LOG_FILE=coupon_expire_check_%date:~0,4%%date:~5,2%%date:~8,2%_%time:~0,2%%time:~3,2%%time:~6,2%.log
echo 优惠券过期检查执行时间: %date% %time% > "%LOG_FILE%"
echo [✓] 日志已保存: %LOG_FILE%

echo.
echo ============================================
echo   优惠券过期检查完成！
echo ============================================
pause
