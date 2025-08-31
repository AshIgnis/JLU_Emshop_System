@echo off
echo Building Emshop JNI Library...

set MINGW_HOME=D:\mingw\x86_64-15.2.0-release-win32-seh-ucrt-rt_v13-rev0\mingw64
set JAVA_HOME=C:\Program Files\Java\jdk-21
set MYSQL_HOME=D:\MySQL

echo.
echo Step 1: Compiling source to object file...
"%MINGW_HOME%\bin\g++.exe" ^
    -c emshop_native_impl_fixed.cpp ^
    -o emshop_native_impl_fixed.o ^
    -I"%JAVA_HOME%\include" ^
    -I"%JAVA_HOME%\include\win32" ^
    -I"%MYSQL_HOME%\include" ^
    -std=c++11 ^
    -Wall ^
    -fPIC ^
    -DWIN32_LEAN_AND_MEAN ^
    -DNOMINMAX

if %ERRORLEVEL% NEQ 0 (
    echo Compilation failed!
    pause
    exit /b 1
)

echo Step 2: Creating bin directory...
if not exist "bin" mkdir "bin"

echo.
echo Step 3: Linking to create DLL...
"%MINGW_HOME%\bin\g++.exe" ^
    -shared ^
    -o bin\emshop_native.dll ^
    emshop_native_impl_fixed.o ^
    -L"%MYSQL_HOME%\lib" ^
    -lmysql ^
    -Wl,--out-implib,bin\emshop_native.lib

if %ERRORLEVEL% NEQ 0 (
    echo Linking failed!
    pause
    exit /b 1
)

echo.
echo Success! JNI library created: bin\emshop_native.dll
echo Import library created: bin\emshop_native.lib
echo.
pause
