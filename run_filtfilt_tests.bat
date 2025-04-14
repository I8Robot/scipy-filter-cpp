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
set "MINICONDA_PATH=D:\SoftWare\Miniconda3" @REM 修改为你的Miniconda路径
set "ACTIVATE_CMD=%MINICONDA_PATH%\Scripts\activate.bat"
set "ENV_NAME=scipy-filter-cpp" @REM 修改为你的环境名称

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
echo Activating Miniconda environment '%ENV_NAME%'...
cd "%RESULTS_DIR%"
call "%ACTIVATE_CMD%" "%MINICONDA_PATH%"
call conda activate %ENV_NAME%

:: 创建一个临时Python脚本来运行测试
echo import sys, os >> temp_run_test.py
echo sys.path.append(r"%TEST_DIR%") >> temp_run_test.py
echo os.chdir(r"%RESULTS_DIR%") >> temp_run_test.py
echo from test_filtfilt_compare import main >> temp_run_test.py
echo main() >> temp_run_test.py

:: 运行临时Python脚本
python temp_run_test.py
if %errorlevel% neq 0 (
  echo WARNING: Python comparison test returned non-zero code: %errorlevel%
)

:: 清理临时文件
del temp_run_test.py

echo.
echo --------------------------------------
echo Running visualization...

:: 对于可视化，我们需要复制可视化脚本到结果目录以避免导入问题
copy "%TEST_DIR%\visualize_filtfilt_comparison.py" "%RESULTS_DIR%\visualize_filtfilt_comparison.py"
cd "%RESULTS_DIR%"
:: 确保使用conda环境
call "%ACTIVATE_CMD%" "%MINICONDA_PATH%"
call conda activate %ENV_NAME%
python visualize_filtfilt_comparison.py .

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