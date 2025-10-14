@echo off
echo ========================================
echo Testing DLL Loading and Basic Functions
echo ========================================
echo.

cd /d "%~dp0java"

echo [1/3] Checking DLL file...
if exist emshop_native_oop.dll (
    echo ✓ DLL exists: emshop_native_oop.dll
    dir emshop_native_oop.dll | findstr /C:"2025"
) else (
    echo ✗ DLL not found in java directory!
    echo Copying from cpp directory...
    copy /Y "..\cpp\emshop_native_oop.dll" "emshop_native_oop.dll"
)
echo.

echo [2/3] Starting server to test DLL loading...
echo Press Ctrl+C after you see "Emshop Netty Server started successfully"
echo.
start /wait cmd /c "mvn exec:java@server 2>&1 | findstr /C:"Native library" /C:"started successfully" /C:"Failed to load""

echo.
echo [3/3] Check the output above for:
echo   - "Native library 'emshop_native_oop' loaded successfully"
echo   - "Emshop Netty Server started successfully"
echo.
pause
