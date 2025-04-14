#include "../src/filter_design.h"
#include <Eigen/Dense>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>

// Function to print filter coefficients with high precision
void print_coefficients(const std::string& name, const filter::RealVector& coeffs) {
    std::cout << name << ": [";
    for (size_t i = 0; i < coeffs.size(); ++i) {
        std::cout << std::setprecision(16) << coeffs[i];
        if (i < coeffs.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;
}

// Save coefficients to a file for comparison with Python
void save_coefficients(const std::string& filename, 
                       const filter::RealVector& b, 
                       const filter::RealVector& a) {
    std::ofstream file(filename);
    file << std::setprecision(16);
    
    // Write b coefficients
    file << "b = [";
    for (size_t i = 0; i < b.size(); ++i) {
        file << b[i];
        if (i < b.size() - 1) {
            file << ", ";
        }
    }
    file << "]\n";
    
    // Write a coefficients
    file << "a = [";
    for (size_t i = 0; i < a.size(); ++i) {
        file << a[i];
        if (i < a.size() - 1) {
            file << ", ";
        }
    }
    file << "]\n";
    
    file.close();
    std::cout << "Coefficients saved to " << filename << std::endl;
}

int main() {
    std::cout << "======= Testing C++ Butterworth Filter Design =======" << std::endl;
    
    // Test cases
    struct TestCase {
        int order;
        filter::RealVector Wn;
        std::string type;
        std::string description;
    };
    
    std::vector<TestCase> test_cases = {
        // Low-pass filters
        {2, {0.1}, "low", "2nd order lowpass (Wn=0.1)"},
        {4, {0.2}, "low", "4th order lowpass (Wn=0.2)"},
        {6, {0.3}, "low", "6th order lowpass (Wn=0.3)"},
        
        // High-pass filters
        {2, {0.15}, "high", "2nd order highpass (Wn=0.15)"},
        {4, {0.25}, "high", "4th order highpass (Wn=0.25)"},
        {6, {0.35}, "high", "6th order highpass (Wn=0.35)"},
        
        // Band-pass filters
        {2, {0.1, 0.4}, "band", "2nd order bandpass (Wn=[0.1, 0.4])"},
        {4, {0.1, 0.3}, "band", "4th order bandpass (Wn=[0.1, 0.3])"},
        
        // Band-stop filters
        {2, {0.1, 0.4}, "stop", "2nd order bandstop (Wn=[0.1, 0.4])"},
        {4, {0.1, 0.3}, "stop", "4th order bandstop (Wn=[0.1, 0.3])"}
    };
    
    for (size_t i = 0; i < test_cases.size(); ++i) {
        const auto& tc = test_cases[i];
        std::cout << "\n===== Test Case " << (i+1) << ": " << tc.description << " =====" << std::endl;
        
        try {
            // Call C++ butter function
            auto [b, a] = filter::butter(tc.order, tc.Wn, tc.type, false, "ba");
            
            // Print results
            print_coefficients("b", b);
            print_coefficients("a", a);
            
            // Save to file for Python comparison
            std::string filename = "cpp_butter_" + std::to_string(i+1) + ".txt";
            save_coefficients(filename, b, a);
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    
    std::cout << "\nAll tests completed. Compare with Python results." << std::endl;
    return 0;
} 