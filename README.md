# SciPy C++ Filter Implementation

<div align="right">
  <a href="#english-version">English</a> | <a href="#中文版本">中文</a>
</div>

<h1 id="english-version">SciPy Digital Filter C++ Implementation</h1>

This project provides a C++ implementation of the SciPy digital filtering functions, focusing on the Butterworth filter design and zero-phase filtering with `filtfilt`. It's designed to be compatible with SciPy's implementation while providing the performance benefits of C++.

## Dependencies

- Eigen 3.4+ (obtained via Git during project setup)
- C++17 compatible compiler
- CMake 3.10+
- Git (for obtaining Eigen library)

## Installation

### 1. Clone the Repository

```bash
git clone https://github.com/your-username/scipy-filter-cpp.git
cd scipy-filter-cpp
```

### 2. Get Eigen Library

You can get the Eigen library in two ways:

#### Option 1: Using Git Submodule (Recommended)

```bash
git submodule add https://gitlab.com/libeigen/eigen.git 3rdparty/Eigen
git submodule update --init --recursive
```

#### Option 2: Manual Download

Download Eigen from [https://eigen.tuxfamily.org](https://eigen.tuxfamily.org) and extract it to the `3rdparty/Eigen` directory.

### 3. Build the Project

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## Core Components

The implementation is divided into three main header files:

### 1. filter_design.h

This header provides functions for designing digital filters, particularly Butterworth filters.

#### Key Functions:

- **`butter(N, Wn, btype, analog, output, fs)`**: Design an Nth-order Butterworth filter
  - `N`: Filter order
  - `Wn`: Critical frequency/frequencies
  - `btype`: Filter type ('low', 'high', 'band', 'stop')
  - `analog`: Boolean indicating if the filter is analog
  - `output`: Output format ('ba', 'zpk', 'sos')
  - `fs`: Sampling frequency
  - Returns: Filter coefficients (b, a)

- **`butter_zpk(N, Wn, btype, analog, fs)`**: Design a Butterworth filter in zero-pole-gain format
  - Returns: (zeros, poles, gain)

- **`lp2lp_zpk`, `lp2hp_zpk`, `lp2bp_zpk`, `lp2bs_zpk`**: Filter transformation functions

### 2. signaltools.h

This header provides functions for applying filters to signals.

#### Key Functions:

- **`filtfilt(b, a, x, padtype, padlen, method, irlen)`**: Zero-phase filtering
  - `b`: Numerator coefficients
  - `a`: Denominator coefficients
  - `x`: Input signal
  - `padtype`: Type of padding ('odd', 'even', 'constant', 'none')
  - `padlen`: Padding length
  - `method`: Method ('pad', 'gust')
  - `irlen`: Impulse response length
  - Returns: Filtered signal

- **`lfilter(b, a, x, zi)`**: Apply a digital filter forward
  - `zi`: Initial conditions
  - Returns: Filtered signal and final conditions

### 3. arraytools.h

This header provides array manipulation utilities to support filtering operations.

#### Key Functions:

- **`axisSlice`, `axisReverse`**: Functions for array manipulation
- **`oddExt`, `evenExt`, `constExt`**: Functions for extending arrays with different padding types

## Usage Examples

### Butterworth Filter Design Example

```cpp
#include "filter_design.h"
#include <iostream>

int main() {
    // Design a 4th-order lowpass Butterworth filter with cutoff at 0.2 (normalized frequency)
    int order = 4;
    filter::RealVector Wn = {0.2};
    auto [b, a] = filter::butter(order, Wn, "low");
    
    // Print filter coefficients
    std::cout << "Numerator coefficients (b):" << std::endl;
    for (auto coef : b) {
        std::cout << coef << " ";
    }
    std::cout << std::endl;
    
    std::cout << "Denominator coefficients (a):" << std::endl;
    for (auto coef : a) {
        std::cout << coef << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
```

### Zero-Phase Filtering Example

```cpp
#include "filter_design.h"
#include "signaltools.h"
#include <Eigen/Dense>
#include <iostream>
#include <vector>

int main() {
    // Design a 2nd-order Butterworth bandpass filter
    int order = 2;
    filter::RealVector Wn = {0.1, 0.4}; // Bandpass between 0.1 and 0.4
    auto [b, a] = filter::butter(order, Wn, "band");
    
    // Create a test signal (e.g., a sinusoid with noise)
    int n = 1000;
    Eigen::VectorXd t = Eigen::VectorXd::LinSpaced(n, 0, 10);
    Eigen::VectorXd x = (2.0 * t.array().sin() + 0.5 * Eigen::VectorXd::Random(n).array()).matrix();
    
    // Apply zero-phase filtering
    Eigen::VectorXd y = signaltools::filtfilt(
        Eigen::Map<Eigen::VectorXd>(b.data(), b.size()),
        Eigen::Map<Eigen::VectorXd>(a.data(), a.size()),
        x
    );
    
    // Print first few samples of original and filtered signals
    std::cout << "Original vs Filtered (first 10 samples):" << std::endl;
    for (int i = 0; i < 10; ++i) {
        std::cout << x(i) << " -> " << y(i) << std::endl;
    }
    
    return 0;
}
```

### Built-in Filter Examples in main.cpp

The project includes a comprehensive example application in `main.cpp` that demonstrates all four types of Butterworth filters (low-pass, high-pass, band-pass, and band-stop). Each filter type has its own example function:

1. **`lowpass_filter_example`**: Demonstrates a low-pass filter with cutoff at 10 Hz
   - Removes high-frequency components (>10 Hz) while preserving low frequencies
   - Useful for removing high-frequency noise from signals

2. **`highpass_filter_example`**: Demonstrates a high-pass filter with cutoff at 30 Hz
   - Removes low-frequency components (<30 Hz) while preserving high frequencies
   - Useful for removing baseline drift or low-frequency interference

3. **`bandpass_filter_example`**: Demonstrates a band-pass filter with passband from 20 Hz to 55 Hz
   - Preserves frequencies within the specified range while attenuating frequencies outside it
   - Useful for isolating a specific frequency band of interest

4. **`bandstop_filter_example`**: Demonstrates a band-stop filter with stopband from 30 Hz to 50 Hz
   - Attenuates frequencies within the specified range while preserving frequencies outside it
   - Useful for removing specific interference or noise within a known frequency range

All examples use a test signal containing multiple frequency components (5 Hz, 40 Hz, and 80 Hz) plus noise. The filtering is performed using both the `filtfilt` function (zero-phase filtering) and standard `lfilter` for comparison. Results are saved to CSV files for visualization.

The example also includes automatic validation and adjustment of cutoff frequencies to ensure they are within the valid range (0,1) when normalized by the Nyquist frequency (fs/2).

To run the examples:

```bash
cd scipy-filter-cpp
./build/src/SciPyFilter
```

The output will show the filter parameters used and the location of the CSV result files, which can be visualized using Python/Matplotlib or Excel.

## Testing

The implementation includes comprehensive tests to verify compatibility with SciPy:

1. **Butterworth Filter Tests**: Compares filter coefficients produced by C++ and Python implementations
2. **FiltFilt Tests**: Compares filtering results between C++ and Python implementations
3. **Visualization**: Generates plots comparing frequency responses and filtered signals

To run the tests, use:

```bash
cd scipy-filter-cpp
./run_butter_tests.bat  # For Butterworth filter tests
./run_filtfilt_tests.bat  # For filtfilt tests
```

Test results are stored in the `test_results_butter` and `test_results_filtfilt` directories, including:
- PDF visualizations comparing frequency responses
- Text files with filter coefficients
- Performance measurements

## License

This project is a C++ port of SciPy's digital filtering functions and is distributed under the same license as SciPy - the 3-clause BSD license.

```
Copyright (c) 2001-2002 Enthought, Inc. 2003, SciPy Developers.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```

The complete license details, including licenses for bundled third-party libraries, can be found in the `LICENSE.txt` and `LICENSES_bundled.txt` files.

<h1 id="中文版本">SciPy数字滤波器C++实现</h1>

本项目提供了SciPy数字滤波函数的C++实现，重点是巴特沃斯滤波器设计和使用`filtfilt`的零相位滤波。该实现与SciPy的API兼容，同时提供C++的性能优势。

## 依赖

- Eigen 3.4+（通过Git在项目设置期间获取）
- 支持C++17的编译器
- CMake 3.10+
- Git（用于获取Eigen库）

## 安装

### 1. 克隆仓库

```bash
git clone https://github.com/your-username/scipy-filter-cpp.git
cd scipy-filter-cpp
```

### 2. 获取Eigen库

您可以通过两种方式获取Eigen库：

#### 方式1：使用Git子模块（推荐）

```bash
git submodule add https://gitlab.com/libeigen/eigen.git 3rdparty/Eigen
git submodule update --init --recursive
```

#### 方式2：手动下载

从[https://eigen.tuxfamily.org](https://eigen.tuxfamily.org)下载Eigen并解压到`3rdparty/Eigen`目录。

### 3. 构建项目

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## 核心组件

该实现分为三个主要头文件：

### 1. filter_design.h

此头文件提供了设计数字滤波器的功能，特别是巴特沃斯滤波器。

#### 主要函数：

- **`butter(N, Wn, btype, analog, output, fs)`**: 设计N阶巴特沃斯滤波器
  - `N`: 滤波器阶数
  - `Wn`: 临界频率
  - `btype`: 滤波器类型（'low'低通, 'high'高通, 'band'带通, 'stop'带阻）
  - `analog`: 布尔值，指示滤波器是否为模拟滤波器
  - `output`: 输出格式（'ba', 'zpk', 'sos'）
  - `fs`: 采样频率
  - 返回值: 滤波器系数 (b, a)

- **`butter_zpk(N, Wn, btype, analog, fs)`**: 以零点-极点-增益格式设计巴特沃斯滤波器
  - 返回值: (zeros, poles, gain)

- **`lp2lp_zpk`, `lp2hp_zpk`, `lp2bp_zpk`, `lp2bs_zpk`**: 滤波器转换函数

### 2. signaltools.h

此头文件提供了将滤波器应用于信号的功能。

#### 主要函数：

- **`filtfilt(b, a, x, padtype, padlen, method, irlen)`**: 零相位滤波
  - `b`: 分子系数
  - `a`: 分母系数
  - `x`: 输入信号
  - `padtype`: 填充类型（'odd'奇, 'even'偶, 'constant'常数, 'none'无）
  - `padlen`: 填充长度
  - `method`: 方法（'pad', 'gust'）
  - `irlen`: 脉冲响应长度
  - 返回值: 滤波后的信号

- **`lfilter(b, a, x, zi)`**: 向前应用数字滤波器
  - `zi`: 初始条件
  - 返回值: 滤波后的信号和最终条件

### 3. arraytools.h

此头文件提供了支持滤波操作的数组操作实用程序。

#### 主要函数：

- **`axisSlice`, `axisReverse`**: 数组操作函数
- **`oddExt`, `evenExt`, `constExt`**: 使用不同填充类型扩展数组的函数

## 用法示例

### 巴特沃斯滤波器设计示例

```cpp
#include "filter_design.h"
#include <iostream>

int main() {
    // 设计一个截止频率为0.2（归一化频率）的4阶低通巴特沃斯滤波器
    int order = 4;
    filter::RealVector Wn = {0.2};
    auto [b, a] = filter::butter(order, Wn, "low");
    
    // 打印滤波器系数
    std::cout << "分子系数 (b):" << std::endl;
    for (auto coef : b) {
        std::cout << coef << " ";
    }
    std::cout << std::endl;
    
    std::cout << "分母系数 (a):" << std::endl;
    for (auto coef : a) {
        std::cout << coef << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
```

### 零相位滤波示例

```cpp
#include "filter_design.h"
#include "signaltools.h"
#include <Eigen/Dense>
#include <iostream>
#include <vector>

int main() {
    // 设计一个2阶巴特沃斯带通滤波器
    int order = 2;
    filter::RealVector Wn = {0.1, 0.4}; // 带通频率范围0.1到0.4
    auto [b, a] = filter::butter(order, Wn, "band");
    
    // 创建测试信号（如带噪声的正弦波）
    int n = 1000;
    Eigen::VectorXd t = Eigen::VectorXd::LinSpaced(n, 0, 10);
    Eigen::VectorXd x = (2.0 * t.array().sin() + 0.5 * Eigen::VectorXd::Random(n).array()).matrix();
    
    // 应用零相位滤波
    Eigen::VectorXd y = signaltools::filtfilt(
        Eigen::Map<Eigen::VectorXd>(b.data(), b.size()),
        Eigen::Map<Eigen::VectorXd>(a.data(), a.size()),
        x
    );
    
    // 打印原始信号和滤波后信号的前几个样本
    std::cout << "原始 vs 滤波后 (前10个样本):" << std::endl;
    for (int i = 0; i < 10; ++i) {
        std::cout << x(i) << " -> " << y(i) << std::endl;
    }
    
    return 0;
}
```

### main.cpp中的内置滤波器示例

本项目在`main.cpp`中包含了一个全面的示例应用程序，展示了四种巴特沃斯滤波器（低通、高通、带通和带阻）的使用。每种滤波器类型都有自己的示例函数：

1. **`lowpass_filter_example`**：演示截止频率为10 Hz的低通滤波器
   - 去除高频成分（>10 Hz），同时保留低频成分
   - 适用于去除信号中的高频噪声

2. **`highpass_filter_example`**：演示截止频率为30 Hz的高通滤波器
   - 去除低频成分（<30 Hz），同时保留高频成分
   - 适用于去除基线漂移或低频干扰

3. **`bandpass_filter_example`**：演示通带为20 Hz到55 Hz的带通滤波器
   - 保留指定范围内的频率，同时衰减范围外的频率
   - 适用于分离特定感兴趣的频带

4. **`bandstop_filter_example`**：演示阻带为30 Hz到50 Hz的带阻滤波器
   - 衰减指定范围内的频率，同时保留范围外的频率
   - 适用于去除已知频率范围内的特定干扰或噪声

所有示例都使用包含多个频率成分（5 Hz、40 Hz和80 Hz）加上噪声的测试信号。滤波使用`filtfilt`函数（零相位滤波）和标准`lfilter`进行比较。结果保存为CSV文件以便可视化。

该示例还包括截止频率的自动验证和调整，确保它们在归一化到奈奎斯特频率（fs/2）后位于有效范围(0,1)内。

要运行示例：

```bash
cd scipy-filter-cpp
./build/src/SciPyFilter
```

输出将显示所使用的滤波器参数和CSV结果文件的位置，这些文件可以使用Python/Matplotlib或Excel进行可视化。

## 测试

该实现包括全面的测试，以验证与SciPy的兼容性：

1. **巴特沃斯滤波器测试**: 比较C++和Python实现产生的滤波器系数
2. **FiltFilt测试**: 比较C++和Python实现之间的滤波结果
3. **可视化**: 生成比较频率响应和滤波信号的图表

要运行测试，请使用：

```bash
cd scipy-filter-cpp
./run_butter_tests.bat  # 用于巴特沃斯滤波器测试
./run_filtfilt_tests.bat  # 用于filtfilt测试
```

测试结果存储在`test_results_butter`和`test_results_filtfilt`目录中，包括：
- 比较频率响应的PDF可视化
- 包含滤波器系数的文本文件
- 性能测量结果

## 许可证

本项目是SciPy数字滤波功能的C++移植版，使用与SciPy相同的3条款BSD许可证进行分发。

```
Copyright (c) 2001-2002 Enthought, Inc. 2003, SciPy Developers.
All rights reserved.

在满足以下条件的情况下，允许以源代码和二进制形式重新分发和使用，无论是否修改：

1. 源代码的再分发必须保留上述版权声明、本条件列表和以下免责声明。

2. 以二进制形式再分发必须在随分发提供的文档和/或其他材料中复制上述版权声明、
   本条件列表和以下免责声明。

3. 未经特定的事先书面许可，不得使用版权持有人的名称或其贡献者的名称来
   认可或推广从本软件衍生的产品。

本软件由版权持有人和贡献者"按原样"提供，任何明示或暗示的保证，包括但不限于
对适销性和特定用途适用性的暗示保证均不做出。在任何情况下，版权持有人或贡献者
均不对任何直接、间接、偶然、特殊、示范性或后果性损害（包括但不限于采购替代
商品或服务；使用、数据或利润损失；或业务中断）承担责任，无论是因何种原因和
基于何种责任理论，无论是合同、严格责任或侵权行为（包括疏忽或其他），即使已
被告知可能发生此类损害的可能性，也不承担责任。
```

完整的许可证详情，包括捆绑的第三方库的许可证，可以在`LICENSE.txt`和`LICENSES_bundled.txt`文件中找到。 