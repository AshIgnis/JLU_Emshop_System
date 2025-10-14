@echo off
cd /d "%~dp0java"
echo ========================================
echo   JLU Emshop Backend Server
echo ========================================
echo.
echo [*] Checking DLL files...
if exist emshop_native_oop.dll (
    echo [√] emshop_native_oop.dll found
) else (
    echo [X] emshop_native_oop.dll missing!
    copy /Y "..\cpp\emshop_native_oop.dll" .
)

if exist libmysql.dll (
    echo [√] libmysql.dll found
) else (
    echo [X] libmysql.dll missing!
    copy /Y "..\cpp\libmysql.dll" .
)

echo.
echo [*] Starting backend server...
echo [*] Press Ctrl+C to stop the server
echo.
mvn exec:java@server
