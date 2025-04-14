#include "../src/filter_design.h"
#include "../src/signaltools.h"
#include <Eigen/Dense>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
#include <random>

// 定义M_PI（Windows平台上可能未定义）
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 生成测试信号
Eigen::VectorXd generateTestSignal(int length, double sampleRate) {
    Eigen::VectorXd signal = Eigen::VectorXd::Zero(length);
    
    // 添加三个不同频率的正弦波
    for (int i = 0; i < length; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        signal(i) = 
            20000.0 * std::sin(2 * M_PI * 5 * t) +     // 5Hz低频信号
            1000.0 * std::sin(2 * M_PI * 50 * t) +    // 50Hz中频信号
            500.0 * std::sin(2 * M_PI * 120 * t);    // 120Hz高频信号
    }
    
    // 添加一些噪声
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<double> noise(0, 0.2);
    
    for (int i = 0; i < length; ++i) {
        signal(i) += noise(gen);
    }
    
    return signal;
}

// 保存信号数据到文件
void saveSignal(const std::string& filename, 
                const Eigen::VectorXd& signal) {
    std::ofstream file(filename);
    file << std::setprecision(16);
    
    // 写入信号数据
    file << "signal = [";
    for (int i = 0; i < signal.size(); ++i) {
        file << signal(i);
        if (i < signal.size() - 1) {
            file << ", ";
        }
    }
    file << "]\n";
    
    file.close();
    std::cout << "Signal saved to " << filename << std::endl;
}

// 保存滤波后数据到文件
void saveFilteredSignal(const std::string& filename, 
                      const Eigen::VectorXd& filtered) {
    std::ofstream file(filename);
    file << std::setprecision(16);
    
    // 写入滤波后数据
    file << "filtered = [";
    for (int i = 0; i < filtered.size(); ++i) {
        file << filtered(i);
        if (i < filtered.size() - 1) {
            file << ", ";
        }
    }
    file << "]\n";
    
    file.close();
    std::cout << "Filtered signal saved to " << filename << std::endl;
}

// 辅助函数：将std::vector<double>转换为Eigen::VectorXd
Eigen::VectorXd vectorToEigen(const std::vector<double>& vec) {
    Eigen::VectorXd eigenVec(vec.size());
    for (size_t i = 0; i < vec.size(); ++i) {
        eigenVec(i) = vec[i];
    }
    return eigenVec;
}

int main() {
    std::cout << "======= Testing C++ FiltFilt Implementation =======" << std::endl;
    
    // 测试参数
    const int signalLength = 1000;
    const double sampleRate = 1000.0; // 1kHz采样率
    
    // 生成测试信号
    Eigen::VectorXd signal = generateTestSignal(signalLength, sampleRate);
    
    // 保存原始信号
    saveSignal("cpp_test_signal.txt", signal);
    
    // 测试用例：不同类型的滤波器
    struct TestCase {
        int order;
        filter::RealVector Wn;
        std::string type;
        std::string description;
    };
    
    std::vector<TestCase> test_cases = {
        // 低通滤波器
        {2, {0.1}, "low", "2nd order lowpass (Wn=0.1)"},
        {4, {0.2}, "low", "4th order lowpass (Wn=0.2)"},
        {6, {0.01}, "low", "6th order lowpass (Wn=0.01)"},
        
        // 高通滤波器
        {2, {0.15}, "high", "2nd order highpass (Wn=0.15)"},
        {4, {0.25}, "high", "4th order highpass (Wn=0.25)"},
        {6, {0.4}, "high", "6th order highpass (Wn=0.4)"},
        
        // 带通滤波器
        {2, {0.1, 0.4}, "band", "2nd order bandpass (Wn=[0.1, 0.4])"},
        {4, {0.1, 0.3}, "band", "4th order bandpass (Wn=[0.1, 0.3])"},
        
        // 带阻滤波器
        {2, {0.1, 0.4}, "stop", "2nd order bandstop (Wn=[0.1, 0.4])"},
        {4, {0.1, 0.3}, "stop", "4th order bandstop (Wn=[0.1, 0.3])"}
    };
    
    for (size_t i = 0; i < test_cases.size(); ++i) {
        const auto& tc = test_cases[i];
        std::cout << "\n===== Test Case " << (i+1) << ": " << tc.description << " =====" << std::endl;
        
        try {
            // 获取并打印ZPK表示，用于调试
            // auto [zeros, poles, gain] = filter::butter_zpk(tc.order, tc.Wn, tc.type, false);
            // filter::print_zpk(zeros, poles, gain, tc.description);
            
            // 创建Butterworth滤波器
            auto [b, a] = filter::butter(tc.order, tc.Wn, tc.type, false, "ba");
            
            // 打印滤波器系数
            std::cout << "b: " << std::endl;
            for (auto coeff : b) {
                std::cout << coeff << std::endl;
            }
            std::cout << "a: " << std::endl;
            for (auto coeff : a) {
                std::cout << coeff << std::endl;
            }
            
            // 将std::vector<double>转换为Eigen::VectorXd
            Eigen::VectorXd b_eigen = vectorToEigen(b);
            Eigen::VectorXd a_eigen = vectorToEigen(a);
            
            // 应用filtfilt滤波器
            std::cout << std::fixed << std::setprecision(16) << std::scientific;
            std::cout << "signal: " << signal(0) << std::endl;
            Eigen::VectorXd filtered = signaltools::filtfilt<double>(b_eigen, a_eigen, signal);
            
            // 保存滤波后的信号
            std::string filename = "cpp_filtfilt_" + std::to_string(i+1) + ".txt";
            saveFilteredSignal(filename, filtered);
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    
    std::cout << "\nAll tests completed. Compare with Python results." << std::endl;
    return 0;
} 