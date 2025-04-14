#include "filter_design.h"
#include <cmath>
#include <iostream>
#include <iomanip>

namespace filter {

// 添加一个调试函数，用于打印ZPK数据
void print_zpk(const ComplexVector& zeros, const ComplexVector& poles, double gain, const std::string& title) {
    std::cout << "\n===== " << title << " =====" << std::endl;
    
    // 打印增益
    std::cout << "Gain (k): " << std::setprecision(8) << gain << std::endl;
    
    // 打印零点
    std::cout << "\nZeros (" << zeros.size() << "):" << std::endl;
    for (size_t i = 0; i < zeros.size(); ++i) {
        const auto& z = zeros[i];
        std::cout << "  z[" << i << "] = " << std::setw(12) << std::setprecision(6) << z.real() 
                  << " + " << std::setw(12) << std::setprecision(6) << z.imag() << "i";
        
        // 计算和显示模值
        double magnitude = std::abs(z);
        std::cout << "  (|z| = " << std::setprecision(6) << magnitude << ")" << std::endl;
    }
    
    // 打印极点
    std::cout << "\nPoles (" << poles.size() << "):" << std::endl;
    for (size_t i = 0; i < poles.size(); ++i) {
        const auto& p = poles[i];
        std::cout << "  p[" << i << "] = " << std::setw(12) << std::setprecision(6) << p.real() 
                  << " + " << std::setw(12) << std::setprecision(6) << p.imag() << "i";
        
        // 计算和显示模值
        double magnitude = std::abs(p);
        std::cout << "  (|p| = " << std::setprecision(6) << magnitude << ")" << std::endl;
    }
    
    std::cout << "=====================\n" << std::endl;
}

// 重载函数，直接接受ZPK结构
void print_zpk(const ZPK& zpk, const std::string& title) {
    print_zpk(zpk.z, zpk.p, zpk.k, title);
}

std::tuple<RealVector, RealVector> iirfilter(
    int N, 
    const RealVector& Wn, 
    double rp, 
    double rs, 
    const std::string& btype, 
    bool analog, 
    const std::string& ftype, 
    const std::string& output, 
    double fs) {
    std::cout << "Starting iirfilter..." << std::endl;
    /*
     * IIR digital and analog filter design given order and critical points.
     * Design an Nth-order digital or analog filter and return the filter coefficients.
     */
    
    // Validate and convert frequency specifications
    RealVector wn = Wn;
    
    // Convert frequencies if fs is specified for digital filters
    if (fs > 0) {
        if (analog) {
            throw std::invalid_argument("fs cannot be specified for an analog filter");
        }
        
        // Normalize frequencies to Nyquist frequency
        for (auto& w : wn) {
            w = w / (fs/2);
        }
    }
    
    // Validate frequencies
    for (auto& w : wn) {
        if (w <= 0) {
            throw std::invalid_argument("Filter critical frequencies must be greater than 0");
        }
    }
    
    if (wn.size() > 1 && wn[0] >= wn[1]) {
        throw std::invalid_argument("First critical frequency must be less than the second one");
    }
    
    // Get filter type
    FilterType filter_type = get_filter_type(btype);
    
    // Verify output format
    OutputType output_type = get_output_type(output);
    
    // Check if we need to use Butterworth filter
    if (ftype != "butter") {
        throw std::invalid_argument("Only Butterworth filter is implemented");
    }
    
    // Get analog prototype
    std::cout << "Generating analog prototype..." << std::endl;
    ZPK zpk = buttap(N);
    print_zpk(zpk, "Analog prototype");
    
    // Pre-warp frequencies for digital filter design
    RealVector warped = wn;
    
    if (!analog) {
        for (auto& w : warped) {
            if (w <= 0 || w >= 1) {
                throw std::invalid_argument("Digital filter critical frequencies must be 0 < Wn < 1");
            }
            
            // Apply bilinear pre-warping
            double fs2 = 2.0;
            w = 2 * fs2 * std::tan(pi * w / fs2);
        }
    }
    std::cout << "warped: " << std::endl;
    for(const auto& w : warped) {
        std::cout << w << std::endl;
    }
    

    // Transform to lowpass, highpass, bandpass, or bandstop
    if (filter_type == FilterType::LOWPASS) {
        if (wn.size() != 1) {
            throw std::invalid_argument("Lowpass filter needs one cutoff frequency");
        }
        
        print_zpk(zpk, "Before lowpass transform");
        zpk = lp2lp_zpk(zpk.z, zpk.p, zpk.k, warped[0]);
        print_zpk(zpk, "After lowpass transform");
    }
    else if (filter_type == FilterType::HIGHPASS) {
        if (wn.size() != 1) {
            throw std::invalid_argument("Highpass filter needs one cutoff frequency");
        }
        
        print_zpk(zpk, "Before highpas transform");
        zpk = lp2hp_zpk(zpk.z, zpk.p, zpk.k, warped[0]);
        print_zpk(zpk, "After highpas transform");
    }
    else if (filter_type == FilterType::BANDPASS) {
        if (wn.size() != 2) {
            throw std::invalid_argument("Bandpass filter needs two cutoff frequencies");
        }
        
        double bw = warped[1] - warped[0];
        double wo = std::sqrt(warped[0] * warped[1]);
        
        std::cout << "Bandpass transformation: wo=" << wo << ", bw=" << bw << std::endl;
        print_zpk(zpk, "Before bandpass transform");
        
        zpk = lp2bp_zpk(zpk.z, zpk.p, zpk.k, wo, bw);
        print_zpk(zpk, "After bandpass transform");
    }
    else if (filter_type == FilterType::BANDSTOP) {
        if (wn.size() != 2) {
            throw std::invalid_argument("Bandstop filter needs two cutoff frequencies");
        }
        
        double bw = warped[1] - warped[0];
        double wo = std::sqrt(warped[0] * warped[1]);
        
        print_zpk(zpk, "Before bandstop transform");
        zpk = lp2bs_zpk(zpk.z, zpk.p, zpk.k, wo, bw);
        print_zpk(zpk, "After bandstop transform");
    }
    
    // Find discrete equivalent if necessary
    if (!analog) {
        std::cout << "Finding discrete equivalent..." << std::endl;
        double fs2 = 2.0;  // Default sampling rate for normalized filters
        zpk = bilinear_zpk(zpk.z, zpk.p, zpk.k, fs2);
    }
    
    // Transform to proper output type
    if (output_type == OutputType::BA) {
        // Return numerator and denominator polynomials
        return zpk2tf(zpk.z, zpk.p, zpk.k);
    }
    else {
        throw std::invalid_argument("Only 'ba' output format is implemented in this function");
    }
}

std::tuple<ComplexVector, ComplexVector, double> iirfilter_zpk(
    int N, 
    const RealVector& Wn, 
    double rp, 
    double rs, 
    const std::string& btype, 
    bool analog, 
    const std::string& ftype, 
    double fs) {
    
    /*
     * ZPK version of iirfilter
     */
    
    // Validate and convert frequency specifications
    RealVector wn = Wn;
    
    // Convert frequencies if fs is specified for digital filters
    if (fs > 0) {
        if (analog) {
            throw std::invalid_argument("fs cannot be specified for an analog filter");
        }
        
        // Normalize frequencies to Nyquist frequency
        for (auto& w : wn) {
            w = w / (fs/2);
        }
    }
    
    // Validate frequencies
    for (auto& w : wn) {
        if (w <= 0) {
            throw std::invalid_argument("Filter critical frequencies must be greater than 0");
        }
    }
    
    if (wn.size() > 1 && wn[0] >= wn[1]) {
        throw std::invalid_argument("First critical frequency must be less than the second one");
    }
    
    // Get filter type
    FilterType filter_type = get_filter_type(btype);
    
    // Check if we need to use Butterworth filter
    if (ftype != "butter") {
        throw std::invalid_argument("Only Butterworth filter is implemented");
    }
    
    // Get analog prototype
    ZPK zpk = buttap(N);
    
    // Pre-warp frequencies for digital filter design
    RealVector warped = wn;
    
    if (!analog) {
        for (auto& w : warped) {
            if (w <= 0 || w >= 1) {
                throw std::invalid_argument("Digital filter critical frequencies must be 0 < Wn < 1");
            }
            
            // Apply bilinear pre-warping
            double fs2 = 2.0;
            w = 2 * fs2 * std::tan(pi * w / fs2);
        }
    }

    for(const auto& w : warped) {
        std::cout << "warped: " << w << std::endl;
    }
    
    // Transform to lowpass, highpass, bandpass, or bandstop
    if (filter_type == FilterType::LOWPASS) {
        if (wn.size() != 1) {
            throw std::invalid_argument("Lowpass filter needs one cutoff frequency");
        }
        
        zpk = lp2lp_zpk(zpk.z, zpk.p, zpk.k, warped[0]);
    }
    else if (filter_type == FilterType::HIGHPASS) {
        if (wn.size() != 1) {
            throw std::invalid_argument("Highpass filter needs one cutoff frequency");
        }
        
        zpk = lp2hp_zpk(zpk.z, zpk.p, zpk.k, warped[0]);
    }
    else if (filter_type == FilterType::BANDPASS) {
        if (wn.size() != 2) {
            throw std::invalid_argument("Bandpass filter needs two cutoff frequencies");
        }
        
        double bw = warped[1] - warped[0];
        double wo = std::sqrt(warped[0] * warped[1]);
        
        zpk = lp2bp_zpk(zpk.z, zpk.p, zpk.k, wo, bw);
    }
    else if (filter_type == FilterType::BANDSTOP) {
        if (wn.size() != 2) {
            throw std::invalid_argument("Bandstop filter needs two cutoff frequencies");
        }
        
        double bw = warped[1] - warped[0];
        double wo = std::sqrt(warped[0] * warped[1]);
        
        zpk = lp2bs_zpk(zpk.z, zpk.p, zpk.k, wo, bw);
    }
    
    // Find discrete equivalent if necessary
    if (!analog) {
        double fs2 = 2.0;  // Default sampling rate for normalized filters
        zpk = bilinear_zpk(zpk.z, zpk.p, zpk.k, fs2);
    }
    
    // Return zeros, poles, gain
    return std::make_tuple(zpk.z, zpk.p, zpk.k);
}

Matrix iirfilter_sos(
    int N, 
    const RealVector& Wn, 
    double rp, 
    double rs, 
    const std::string& btype, 
    bool analog, 
    const std::string& ftype, 
    double fs) {
    
    /*
     * SOS version of iirfilter
     */
    
    // Get zpk representation
    auto zpk_result = iirfilter_zpk(N, Wn, rp, rs, btype, analog, ftype, fs);
    
    // Convert to SOS format
    return zpk2sos(
        std::get<0>(zpk_result), 
        std::get<1>(zpk_result), 
        std::get<2>(zpk_result), 
        "nearest", 
        analog
    );
}

} // namespace filter 