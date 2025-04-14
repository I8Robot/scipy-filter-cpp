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
        // Generate a signal with two frequency components + noise
        x(i) = std::sin(2 * M_PI * 5 * t(i)) +            // 5 Hz component
               0.5 * std::sin(2 * M_PI * 40 * t(i)) +      // 40 Hz component
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

int main() {
    // Test parameters
    
    const double fs = 120;  // Sampling frequency (Hz)
    const double cutoff = 10;  // Cutoff frequency (Hz)
    const int order = 4;  // Filter order
    
    // Generate test signal
   // Eigen::VectorXd x = generate_test_signal(n, fs);
    // from data.txt file read signal data
    Eigen::VectorXd x = read_signal_from_file("data.txt");
    const int n = x.rows();  // Number of samples
    
    // Create time vector for plotting
    Eigen::VectorXd t(n);
    for (int i = 0; i < n; ++i) {
        t(i) = i / fs;
    }
    
    // Design filter using butter from filter_design
    std::vector<double> Wn = {cutoff / (fs/2)}; // Normalize cutoff to Nyquist frequency
    auto [b, a] = filter::butter(order, Wn, "high", false, "ba");
    std::cout << "Wn: " << Wn[0] << std::endl;
    
    // Convert std::vector<double> to Eigen::VectorXd for compatibility with signaltools functions
    Eigen::VectorXd b_eigen(b.size());
    Eigen::VectorXd a_eigen(a.size());
    for (size_t i = 0; i < b.size(); ++i) {
        b_eigen(i) = b[i];
    }
    for (size_t i = 0; i < a.size(); ++i) {
        a_eigen(i) = a[i];
    }
    
    std::cout << "Filter coefficients:" << std::endl;
    std::cout << "b: " << b_eigen.transpose() << std::endl;
    std::cout << "a: " << a_eigen.transpose() << std::endl;
    
    // Apply filtfilt
    Eigen::VectorXd y = signaltools::filtfilt(b_eigen, a_eigen, x);
    
    // Apply standard lfilter (for comparison)
    Eigen::VectorXd y_lfilter = signaltools::lfilter_output_only(b_eigen, a_eigen, x);
    
    // Save results to CSV for visualization
    save_to_csv("filtfilt_results.csv", t, x, y);
    save_to_csv("lfilter_results.csv", t, x, y_lfilter);
    
    std::cout << "Filtering complete!" << std::endl;
    std::cout << "Note: The results have been saved to CSV files for visualization." << std::endl;
    std::cout << "You can plot these results using any plotting tool like Python/Matplotlib or Excel." << std::endl;
    
    return 0;
} 