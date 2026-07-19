@echo off
rem =============================================
rem   Unified C++ Examples Bundle: Windows Build
rem =============================================

echo Checking for CMake...
where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo [ERROR] cmake.exe was not found in your PATH.
    echo Please make sure CMake and MSYS2/MinGW64 toolchain are installed and in your PATH.
    echo Example PATH addition: C:\msys64\mingw64\bin
    exit /b 1
)

echo Checking for MinGW compiler...
where g++ >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo [ERROR] g++.exe was not found in your PATH.
    echo Please ensure the MSYS2/MinGW64 build toolchain is in your PATH.
    exit /b 1
)

set BASE_DIR=%~dp0
if "%BASE_DIR:~-1%"=="\" set BASE_DIR=%BASE_DIR:~0,-1%
set BUILD_DIR=%BASE_DIR%\build

if not exist "%BUILD_DIR%" (
    echo Creating build directory: %BUILD_DIR%
    mkdir "%BUILD_DIR%"
)

echo Configuring projects with CMake (MinGW Makefiles)...
cmake -G "MinGW Makefiles" -B "%BUILD_DIR%" -S "%BASE_DIR%"
if %ERRORLEVEL% neq 0 (
    echo [ERROR] CMake configuration failed.
    exit /b %ERRORLEVEL%
)

echo Building all projects...
cmake --build "%BUILD_DIR%"
if %ERRORLEVEL% neq 0 (
    echo [ERROR] CMake build failed.
    exit /b %ERRORLEVEL%
)

echo =============================================
echo   Build Completed Successfully!
echo =============================================
