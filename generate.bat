@echo off
rem Save with ANSI encoding
setlocal enabledelayedexpansion

rem Set project root
set PROJECT_ROOT=%~dp0

rem Check Eigen directory
if not exist "%PROJECT_ROOT%\3rdparty\Eigen" (
    echo ERROR: Eigen directory not found at %PROJECT_ROOT%\3rdparty\Eigen
    echo Please make sure Eigen is placed in the 3rdparty\Eigen directory
    exit /b 1
)


rem Create build directory
if not exist "%PROJECT_ROOT%\build" mkdir "%PROJECT_ROOT%\build"
cd "%PROJECT_ROOT%\build"

rem Use Visual Studio 2022
set VS_VERSION=Visual Studio 17 2022

rem Clear CMake cache
if exist "CMakeCache.txt" del /f "CMakeCache.txt"

rem Generate VS solution
echo Using %VS_VERSION% to generate solution...
cmake -G "%VS_VERSION%" -A x64 -DCMAKE_CXX_FLAGS="/utf-8 /W3 /EHsc" -DCMAKE_CXX_STANDARD=14 ..

if %ERRORLEVEL% neq 0 (
    echo CMake generation failed!
    exit /b 1
)

rem Build project
echo Building project...
cmake --build . --config Release

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    echo See error messages above for details
    exit /b 1
)

echo.
echo =============================
echo Build completed!
echo.
echo Executable located at: %PROJECT_ROOT%\build\Release\
echo =============================

cd "%PROJECT_ROOT%"
endlocal 