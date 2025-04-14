@echo off
setlocal enabledelayedexpansion

echo ========================================
echo FiltFilt Filter C++/Python Test Suite
echo ========================================
echo.

:: 获取当前目录
set "CURRENT_DIR=%~dp0"
set "BUILD_DIR=%CURRENT_DIR%build"
set "TEST_DIR=%CURRENT_DIR%tests"
set "RESULTS_DIR=%CURRENT_DIR%test_results_filtfilt"
set "PYTHON_PATH=python"

:: 创建输出结果目录
echo [1/6] Creating results directory...
if not exist "%RESULTS_DIR%" mkdir "%RESULTS_DIR%"

:: 创建并进入build目录
echo [2/6] Preparing build directory...
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%"

:: 配置CMake
echo [3/6] Configuring with CMake...
cmake .. -DBUILD_TESTS=ON
if %errorlevel% neq 0 (
  echo ERROR: CMake configuration failed.
  goto :error
)

:: 构建filtfilt测试程序
echo [4/6] Building filtfilt tests...
echo #Test target already added to CMakeLists.txt

:: 构建测试
cmake --build . --config Release --target test_filtfilt_compare
if %errorlevel% neq 0 (
  echo ERROR: Build failed.
  goto :error
)

:: 运行C++测试程序，将结果输出到指定目录
echo [5/6] Running C++ filtfilt test program...
cd "%BUILD_DIR%\Release"
echo Running C++ tests with results output to %RESULTS_DIR%...
:: 修改工作目录为结果目录
cd "%RESULTS_DIR%"
:: 运行测试程序
"%BUILD_DIR%\Release\test_filtfilt_compare.exe"
if %errorlevel% neq 0 (
  echo WARNING: C++ test program returned non-zero code: %errorlevel%
)

:: 运行Python比较脚本，将结果输出到指定目录
echo [6/6] Running Python filtfilt comparison tests...
cd "%RESULTS_DIR%"

:: 复制Python测试脚本到结果目录
copy "%TEST_DIR%\test_filtfilt_compare.py" "%RESULTS_DIR%\test_filtfilt_compare.py"

:: 显示Python信息
echo Python information:
%PYTHON_PATH% --version
echo.

:: 运行Python测试脚本
echo Running Python comparison script...
%PYTHON_PATH% test_filtfilt_compare.py
if %errorlevel% neq 0 (
  echo WARNING: Python comparison test returned non-zero code: %errorlevel%
)

:: 生成可视化比较结果
echo [7/7] Generating visualization...
copy "%TEST_DIR%\visualize_filtfilt_comparison.py" "%RESULTS_DIR%\visualize_filtfilt_comparison.py"
%PYTHON_PATH% visualize_filtfilt_comparison.py .
if %errorlevel% neq 0 (
  echo WARNING: Visualization generation returned non-zero code: %errorlevel%
)

echo.
echo Tests completed!
goto :end

:error
echo.
echo Test execution failed!
exit /b 1

:end
cd "%CURRENT_DIR%"
echo.
echo All operations completed.
echo All test results can be found in: %RESULTS_DIR%
endlocal 