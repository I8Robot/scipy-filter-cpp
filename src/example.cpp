#include "filter_design.h"
#include <iostream>
#include <iomanip>
#include <vector>

// Helper function to print vectors
template<typename T>
void print_vector(const std::vector<T>& vec, const std::string& name) {
    std::cout << name << " = [";
    for (size_t i = 0; i < vec.size(); i++) {
        std::cout << vec[i];
        if (i < vec.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]\n";
}

// Helper function to print complex vectors
void print_complex_vector(const filter::ComplexVector& vec, const std::string& name) {
    std::cout << name << " = [";
    for (size_t i = 0; i < vec.size(); i++) {
        std::cout << "(" << vec[i].real() << " + " << vec[i].imag() << "j)";
        if (i < vec.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]\n";
}

// Helper function to print matrices
void print_matrix(const filter::Matrix& mat, const std::string& name) {
    std::cout << name << " = [\n";
    for (int i = 0; i < mat.rows(); i++) {
        std::cout << "  [";
        for (int j = 0; j < mat.cols(); j++) {
            std::cout << std::fixed << std::setprecision(6) << mat(i, j);
            if (j < mat.cols() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]";
        if (i < mat.rows() - 1) {
            std::cout << ",\n";
        }
    }
    std::cout << "\n]\n";
}

int main() {
    // Example 1: Design a lowpass Butterworth filter
    std::cout << "Example 1: Lowpass Butterworth filter\n";
    std::cout << "-------------------------------------\n";
    
    int N = 4;  // 4th order filter
    //filter::RealVector Wn = {0.5};  // Cutoff frequency at 0.5*Nyquist
    filter::RealVector Wn = { 10/(0.5*120) };  // Cutoff frequency at 0.5*Nyquist
    print_vector(Wn, "Wn");
    
    try {
        // BA format (numerator/denominator)
        auto ba = filter::butter(N, Wn, "lowpass", false, "ba", 0.0);
        print_vector(std::get<0>(ba), "b");
        print_vector(std::get<1>(ba), "a");
        
        // ZPK format (zeros/poles/gain)
        auto zpk = filter::butter_zpk(N, Wn, "lowpass", false, 0.0);
        print_complex_vector(std::get<0>(zpk), "zeros");
        print_complex_vector(std::get<1>(zpk), "poles");
        std::cout << "gain = " << std::get<2>(zpk) << std::endl;
        
        // SOS format (second-order sections)
        auto sos = filter::butter_sos(N, Wn, "lowpass", false, 0.0);
        print_matrix(sos, "sos");
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    std::cout << "\n";
    
    // Example 2: Design a bandpass Butterworth filter with analog=True
    std::cout << "Example 2: Analog bandpass Butterworth filter\n";
    std::cout << "------------------------------------------\n";
    
    N = 3;  // 3rd order filter
    Wn = {100.0, 200.0};  // Bandpass between 100 rad/s and 200 rad/s
    
    try {
        // BA format (numerator/denominator)
        auto ba = filter::butter(N, Wn, "bandpass", true, "ba", 0.0);
        print_vector(std::get<0>(ba), "b");
        print_vector(std::get<1>(ba), "a");
        
        // ZPK format (zeros/poles/gain)
        auto zpk = filter::butter_zpk(N, Wn, "bandpass", true, 0.0);
        print_complex_vector(std::get<0>(zpk), "zeros");
        print_complex_vector(std::get<1>(zpk), "poles");
        std::cout << "gain = " << std::get<2>(zpk) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    std::cout << "\n";
    
    // Example 3: Design a digital highpass Butterworth filter with fs
    std::cout << "Example 3: Digital highpass Butterworth filter with fs\n";
    std::cout << "---------------------------------------------------\n";
    
    N = 4;  // 2nd order filter
    //Wn = {1000.0};  // Cutoff at 1000 Hz
    Wn = { 10 / (0.5 * 120) };  // Cutoff frequency at 0.5*Nyquist
    print_vector(Wn, "Wn");
    double fs = 0.0;  // Sampling rate of 8000 Hz
    
    try {
        // BA format (numerator/denominator)
        auto ba = filter::butter(N, Wn, "highpass", false, "ba", fs);
        print_vector(std::get<0>(ba), "b");
        print_vector(std::get<1>(ba), "a");
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    return 0;
} 