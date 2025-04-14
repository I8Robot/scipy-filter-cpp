@echo off
setlocal enabledelayedexpansion

set PROJECT_ROOT=%~dp0
set EIGEN_DIR=%PROJECT_ROOT%\3rdparty\Eigen
set EIGEN_VERSION=3.4.0
set EIGEN_URL=https://gitlab.com/libeigen/eigen/-/archive/%EIGEN_VERSION%/eigen-%EIGEN_VERSION%.zip
set EIGEN_ZIP=%PROJECT_ROOT%\eigen.zip

echo Downloading Eigen %EIGEN_VERSION%...
echo URL: %EIGEN_URL%

rem Check if PowerShell is available
powershell -Command "exit" 2>nul
if %ERRORLEVEL% neq 0 (
    echo PowerShell is not available. Please download Eigen manually from:
    echo %EIGEN_URL%
    echo And extract it to: %EIGEN_DIR%
    exit /b 1
)

rem Create directories
if not exist "%PROJECT_ROOT%\3rdparty" mkdir "%PROJECT_ROOT%\3rdparty"
if not exist "%EIGEN_DIR%" mkdir "%EIGEN_DIR%"

rem Download Eigen
echo Downloading Eigen...
powershell -Command "Invoke-WebRequest -Uri '%EIGEN_URL%' -OutFile '%EIGEN_ZIP%'"
if %ERRORLEVEL% neq 0 (
    echo Failed to download Eigen.
    exit /b 1
)

rem Extract Eigen
echo Extracting Eigen...
powershell -Command "Expand-Archive '%EIGEN_ZIP%' -DestinationPath '%PROJECT_ROOT%\3rdparty' -Force"
if %ERRORLEVEL% neq 0 (
    echo Failed to extract Eigen.
    exit /b 1
)

rem Move files to the correct location
echo Moving files...
powershell -Command "Move-Item '%PROJECT_ROOT%\3rdparty\eigen-%EIGEN_VERSION%\Eigen' '%EIGEN_DIR%' -Force"
powershell -Command "Move-Item '%PROJECT_ROOT%\3rdparty\eigen-%EIGEN_VERSION%\unsupported' '%EIGEN_DIR%' -Force"

rem Clean up
echo Cleaning up...
del "%EIGEN_ZIP%"
rmdir /s /q "%PROJECT_ROOT%\3rdparty\eigen-%EIGEN_VERSION%"

echo Eigen %EIGEN_VERSION% has been downloaded and installed successfully.
echo You can now run generate.bat to build the project.

endlocal 