@echo off
REM 运行限购功能升级脚本
mysql -u root -p emshop < add_purchase_limit.sql
pause