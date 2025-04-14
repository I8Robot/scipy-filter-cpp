#include "signaltools.h"
#include "filter_design.h"
#include <Eigen/Dense>
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// read data from file
Eigen::VectorXd read_signal_from_file(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<double> data;

    if (!file.is_open()) {
        std::cerr << "Cann't open file: " << filename.c_str() << std::endl;
        return Eigen::VectorXd(0);
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            try {
                double value = std::stod(line);
                data.push_back(value);
            }
            catch (const std::exception& e) {
                std::cerr << "Parse error: " << e.what() << L" in line: " << line.c_str() << std::endl;
            }
        }
    }

    file.close();

    // tranform to Eigen::VectorXd
    Eigen::VectorXd signal(data.size());
    for (size_t i = 0; i < data.size(); i++) {
        signal(i) = data[i];
    }

    return signal;
}

// Function to generate a test signal
Eigen::VectorXd generate_test_signal(int n, double fs) {
    Eigen::VectorXd t(n);
    Eigen::VectorXd x(n);
    
    for (int i = 0; i < n; ++i) {
        t(i) = i / fs;
        // Generate a signal with multiple frequency components + noise
        x(i) = std::sin(2 * M_PI * 5 * t(i)) +            // 5 Hz component (low freq)
               0.5 * std::sin(2 * M_PI * 40 * t(i)) +      // 40 Hz component (mid freq)
               0.3 * std::sin(2 * M_PI * 80 * t(i)) +      // 80 Hz component (high freq)
               0.2 * (2.0 * std::rand() / RAND_MAX - 1.0); // Noise
    }
    
    return x;
}

// Function to design a Butterworth filter
std::pair<Eigen::VectorXd, Eigen::VectorXd> butter_lowpass(int order, double cutoff_freq, double fs) {
    // This is a simplified version - in practice you'd use a proper filter design library
    double wc = 2 * cutoff_freq / fs;  // Normalized cutoff frequency
    
    // For demonstration, let's just create a simple first-order filter
    // In a real application, you'd want proper filter design
    Eigen::VectorXd b(order + 1);
    Eigen::VectorXd a(order + 1);
    
    // Simple first-order lowpass filter coefficients
    double alpha = std::tan(M_PI * wc / 2);
    double alphaSquared = alpha * alpha;
    
    // For first order
    if (order == 1) {
        b(0) = alpha / (1 + alpha);
        b(1) = b(0);
        a(0) = 1.0;
        a(1) = (alpha - 1) / (alpha + 1);
    }
    // For second order
    else if (order == 2) {
        double c = 1 + 2 * alpha + alphaSquared;
        b(0) = alphaSquared / c;
        b(1) = 2 * b(0);
        b(2) = b(0);
        a(0) = 1.0;
        a(1) = 2 * (alphaSquared - 1) / c;
        a(2) = (1 - 2 * alpha + alphaSquared) / c;
    }
    // For second order
    else if (order == 4) {
        b(0) = 0.499815;
        b(1) = -1.99925999;
        b(2) = 2.99888999;
        b(3) = b(1);
        b(4) = b(0);
        a(0) = 1.0;
        a(1) = -2.63862774;
        a(2) = 2.76930979;
        a(3) = -1.33928076;
        a(4) = 0.24982167;
    }
    // Higher orders would need a proper design approach
    else {
        // Default to a simple first-order filter for demonstration
        b = Eigen::VectorXd::Ones(2) * 0.5;
        a = Eigen::VectorXd::Zero(2);
        a(0) = 1.0;
    }
    
    return {b, a};
}

// Save results to a CSV file for visualization
void save_to_csv(const std::string& filename, 
                const Eigen::VectorXd& t, 
                const Eigen::VectorXd& original, 
                const Eigen::VectorXd& filtered) {
    std::ofstream file(filename);
    
    file << "Time,Original,Filtered\n";
    for (int i = 0; i < t.size(); ++i) {
        file << t(i) << "," << original(i) << "," << filtered(i) << "\n";
    }
    
    file.close();
    std::cout << "Results saved to " << filename << std::endl;
}

// 转换滤波器系数：从std::vector<double>到Eigen::VectorXd
std::pair<Eigen::VectorXd, Eigen::VectorXd> convert_filter_coeffs(const std::vector<double>& b, const std::vector<double>& a) {
    Eigen::VectorXd b_eigen(b.size());
    Eigen::VectorXd a_eigen(a.size());
    
    for (size_t i = 0; i < b.size(); ++i) {
        b_eigen(i) = b[i];
    }
    for (size_t i = 0; i < a.size(); ++i) {
        a_eigen(i) = a[i];
    }
    
    return {b_eigen, a_eigen};
}

// Low-pass filter example function
void lowpass_filter_example(double fs = 120.0, int n = 1000) {
    std::cout << "\n========== Low-Pass Filter Example ==========\n" << std::endl;
    
    // 参数设置
    const double cutoff = 10.0;  // 截止频率 (Hz)
    const int order = 4;        // 滤波器阶数
    
    // 确保Wn在(0,1)范围内
    double wn_normalized = cutoff / (fs/2.0);
    if (wn_normalized <= 0.0 || wn_normalized >= 1.0) {
        std::cout << "Warning: Normalized cutoff frequency must be in range (0,1), current value: " << wn_normalized << std::endl;
        wn_normalized = std::min(std::max(wn_normalized, 0.01), 0.99);
        std::cout << "Adjusted to: " << wn_normalized << std::endl;
    }
    
    // 生成测试信号
    Eigen::VectorXd x = generate_test_signal(n, fs);
    
    // 创建时间向量用于绘图
    Eigen::VectorXd t(n);
    for (int i = 0; i < n; ++i) {
        t(i) = i / fs;
    }
    
    // 设计滤波器
    std::vector<double> Wn = {wn_normalized}; // 使用确保在(0,1)范围内的归一化截止频率
    auto [b, a] = filter::butter(order, Wn, "low");
    
    // 将系数转换为Eigen::VectorXd以便与signaltools函数兼容
    auto [b_eigen, a_eigen] = convert_filter_coeffs(b, a);
    
    // 应用filtfilt零相位滤波
    Eigen::VectorXd y = signaltools::filtfilt(b_eigen, a_eigen, x);
    
    // 应用标准lfilter（用于比较）
    Eigen::VectorXd y_lfilter = signaltools::lfilter_output_only(b_eigen, a_eigen, x);
    
    // 保存结果到CSV用于可视化
    save_to_csv("lowpass_filtfilt_results.csv", t, x, y);
    save_to_csv("lowpass_lfilter_results.csv", t, x, y_lfilter);
    
    std::cout << "Low-pass filtering completed!" << std::endl;
    std::cout << "Cutoff frequency: " << cutoff << " Hz (normalized to: " << wn_normalized << ")" << std::endl;
    std::cout << "Filter order: " << order << std::endl;
}

// High-pass filter example function
void highpass_filter_example(double fs = 120.0, int n = 1000) {
    std::cout << "\n========== High-Pass Filter Example ==========\n" << std::endl;
    
    // 参数设置
    const double cutoff = 30.0;  // 截止频率 (Hz)
    const int order = 4;        // 滤波器阶数
    
    // 确保Wn在(0,1)范围内
    double wn_normalized = cutoff / (fs/2.0);
    if (wn_normalized <= 0.0 || wn_normalized >= 1.0) {
        std::cout << "Warning: Normalized cutoff frequency must be in range (0,1), current value: " << wn_normalized << std::endl;
        wn_normalized = std::min(std::max(wn_normalized, 0.01), 0.99);
        std::cout << "Adjusted to: " << wn_normalized << std::endl;
    }
    
    // 生成测试信号
    Eigen::VectorXd x = generate_test_signal(n, fs);
    
    // 创建时间向量用于绘图
    Eigen::VectorXd t(n);
    for (int i = 0; i < n; ++i) {
        t(i) = i / fs;
    }
    
    // 设计高通滤波器
    std::vector<double> Wn = {wn_normalized}; // 使用确保在(0,1)范围内的归一化截止频率
    auto [b, a] = filter::butter(order, Wn, "high");
    
    // 将系数转换为Eigen格式
    auto [b_eigen, a_eigen] = convert_filter_coeffs(b, a);
    
    // 应用filtfilt零相位滤波
    Eigen::VectorXd y = signaltools::filtfilt(b_eigen, a_eigen, x);
    
    // 应用标准lfilter（用于比较）
    Eigen::VectorXd y_lfilter = signaltools::lfilter_output_only(b_eigen, a_eigen, x);
    
    // 保存结果到CSV用于可视化
    save_to_csv("highpass_filtfilt_results.csv", t, x, y);
    save_to_csv("highpass_lfilter_results.csv", t, x, y_lfilter);
    
    std::cout << "High-pass filtering completed!" << std::endl;
    std::cout << "Cutoff frequency: " << cutoff << " Hz (normalized to: " << wn_normalized << ")" << std::endl;
    std::cout << "Filter order: " << order << std::endl;
}

// Band-pass filter example function
void bandpass_filter_example(double fs = 120.0, int n = 1000) {
    std::cout << "\n========== Band-Pass Filter Example ==========\n" << std::endl;
    
    // 参数设置
    const double low_cutoff = 20.0;   // 低截止频率 (Hz)
    const double high_cutoff = 55.0;  // 高截止频率 (Hz) - 修改为小于Nyquist的值
    const int order = 4;              // 滤波器阶数
    
    // 确保Wn在(0,1)范围内
    double low_wn = low_cutoff / (fs/2.0);
    double high_wn = high_cutoff / (fs/2.0);
    
    if (low_wn <= 0.0 || low_wn >= 1.0) {
        std::cout << "Warning: Low normalized cutoff frequency must be in range (0,1), current value: " << low_wn << std::endl;
        low_wn = std::min(std::max(low_wn, 0.01), 0.99);
        std::cout << "Adjusted to: " << low_wn << std::endl;
    }
    
    if (high_wn <= 0.0 || high_wn >= 1.0) {
        std::cout << "Warning: High normalized cutoff frequency must be in range (0,1), current value: " << high_wn << std::endl;
        high_wn = std::min(std::max(high_wn, 0.01), 0.99);
        std::cout << "Adjusted to: " << high_wn << std::endl;
    }
    
    if (low_wn >= high_wn) {
        std::cout << "Error: Low cutoff frequency must be less than high cutoff frequency" << std::endl;
        low_wn = high_wn * 0.5; // 简单调整
        std::cout << "Adjusted low cutoff frequency to: " << low_wn << std::endl;
    }
    
    // 生成测试信号
    Eigen::VectorXd x = generate_test_signal(n, fs);
    
    // 创建时间向量用于绘图
    Eigen::VectorXd t(n);
    for (int i = 0; i < n; ++i) {
        t(i) = i / fs;
    }
    
    // 设计带通滤波器
    std::vector<double> Wn = {low_wn, high_wn}; // 使用确保在(0,1)范围内的归一化截止频率
    auto [b, a] = filter::butter(order, Wn, "band");
    
    // 将系数转换为Eigen格式
    auto [b_eigen, a_eigen] = convert_filter_coeffs(b, a);
    
    // 应用filtfilt零相位滤波
    Eigen::VectorXd y = signaltools::filtfilt(b_eigen, a_eigen, x);
    
    // 应用标准lfilter（用于比较）
    Eigen::VectorXd y_lfilter = signaltools::lfilter_output_only(b_eigen, a_eigen, x);
    
    // 保存结果到CSV用于可视化
    save_to_csv("bandpass_filtfilt_results.csv", t, x, y);
    save_to_csv("bandpass_lfilter_results.csv", t, x, y_lfilter);
    
    std::cout << "Band-pass filtering completed!" << std::endl;
    std::cout << "Passband range: " << low_cutoff << " Hz - " << high_cutoff << " Hz" << std::endl;
    std::cout << "Normalized passband range: " << low_wn << " - " << high_wn << std::endl;
    std::cout << "Filter order: " << order << std::endl;
}

// Band-stop filter example function
void bandstop_filter_example(double fs = 120.0, int n = 1000) {
    std::cout << "\n========== Band-Stop Filter Example ==========\n" << std::endl;
    
    // 参数设置
    const double low_cutoff = 30.0;  // 低截止频率 (Hz)
    const double high_cutoff = 50.0; // 高截止频率 (Hz)
    const int order = 4;           // 滤波器阶数
    
    // 确保Wn在(0,1)范围内
    double low_wn = low_cutoff / (fs/2.0);
    double high_wn = high_cutoff / (fs/2.0);
    
    if (low_wn <= 0.0 || low_wn >= 1.0) {
        std::cout << "Warning: Low normalized cutoff frequency must be in range (0,1), current value: " << low_wn << std::endl;
        low_wn = std::min(std::max(low_wn, 0.01), 0.99);
        std::cout << "Adjusted to: " << low_wn << std::endl;
    }
    
    if (high_wn <= 0.0 || high_wn >= 1.0) {
        std::cout << "Warning: High normalized cutoff frequency must be in range (0,1), current value: " << high_wn << std::endl;
        high_wn = std::min(std::max(high_wn, 0.01), 0.99);
        std::cout << "Adjusted to: " << high_wn << std::endl;
    }
    
    if (low_wn >= high_wn) {
        std::cout << "Error: Low cutoff frequency must be less than high cutoff frequency" << std::endl;
        low_wn = high_wn * 0.5; // 简单调整
        std::cout << "Adjusted low cutoff frequency to: " << low_wn << std::endl;
    }
    
    // 生成测试信号
    Eigen::VectorXd x = generate_test_signal(n, fs);
    
    // 创建时间向量用于绘图
    Eigen::VectorXd t(n);
    for (int i = 0; i < n; ++i) {
        t(i) = i / fs;
    }
    
    // 设计带阻滤波器
    std::vector<double> Wn = {low_wn, high_wn}; // 使用确保在(0,1)范围内的归一化截止频率
    auto [b, a] = filter::butter(order, Wn, "stop");
    
    // 将系数转换为Eigen格式
    auto [b_eigen, a_eigen] = convert_filter_coeffs(b, a);
    
    // 应用filtfilt零相位滤波
    Eigen::VectorXd y = signaltools::filtfilt(b_eigen, a_eigen, x);
    
    // 应用标准lfilter（用于比较）
    Eigen::VectorXd y_lfilter = signaltools::lfilter_output_only(b_eigen, a_eigen, x);
    
    // 保存结果到CSV用于可视化
    save_to_csv("bandstop_filtfilt_results.csv", t, x, y);
    save_to_csv("bandstop_lfilter_results.csv", t, x, y_lfilter);
    
    std::cout << "Band-stop filtering completed!" << std::endl;
    std::cout << "Stopband range: " << low_cutoff << " Hz - " << high_cutoff << " Hz" << std::endl;
    std::cout << "Normalized stopband range: " << low_wn << " - " << high_wn << std::endl;
    std::cout << "Filter order: " << order << std::endl;
}

int main() {
    std::cout << "SciPy Filter C++ Implementation Example" << std::endl;
    std::cout << "============================" << std::endl;
    
    // 设置采样频率和样本数
    const double fs = 120.0;  // 采样频率 (Hz)
    const int n = 1000;      // 样本数
    
    // 运行各种滤波器示例
    lowpass_filter_example(fs, n);
    highpass_filter_example(fs, n);
    bandpass_filter_example(fs, n);
    bandstop_filter_example(fs, n);
    
    std::cout << "\nAll filter examples completed!" << std::endl;
    std::cout << "Results have been saved to CSV files, which can be visualized using Python/Matplotlib or Excel." << std::endl;
    
    return 0;
} 