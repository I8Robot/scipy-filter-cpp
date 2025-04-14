#include "filter_design.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <iostream>

namespace filter {

// Convert string filter type to enum
FilterType get_filter_type(const std::string& btype) {
    std::string type = btype;
    std::transform(type.begin(), type.end(), type.begin(), ::tolower);
    
    if (type == "lowpass" || type == "low" || type == "lp" || type == "l") {
        return FilterType::LOWPASS;
    } else if (type == "highpass" || type == "high" || type == "hp" || type == "h") {
        return FilterType::HIGHPASS;
    } else if (type == "bandpass" || type == "band" || type == "bp" || type == "pass") {
        return FilterType::BANDPASS;
    } else if (type == "bandstop" || type == "stop" || type == "bs" || type == "bands") {
        return FilterType::BANDSTOP;
    } else {
        throw std::invalid_argument("Unknown filter type: " + btype);
    }
}

// Convert string output type to enum
OutputType get_output_type(const std::string& output) {
    std::string type = output;
    std::transform(type.begin(), type.end(), type.begin(), ::tolower);
    
    if (type == "ba") {
        return OutputType::BA;
    } else if (type == "zpk") {
        return OutputType::ZPK;
    } else if (type == "sos") {
        return OutputType::SOS;
    } else {
        throw std::invalid_argument("Unknown output type: " + output);
    }
}

// Calculate the relative degree of a transfer function
int relative_degree(const ComplexVector& z, const ComplexVector& p) {
    int degree = static_cast<int>(p.size() - z.size());
    
    if (degree < 0) {
        throw std::invalid_argument("Improper transfer function. Must have at least as many poles as zeros.");
    }
    return degree;
}

// Compute polynomial coefficients from roots (equivalent to numpy.poly)
RealVector poly(const ComplexVector& roots) {
    if (roots.empty()) {
        return {1.0};
    }

    // Start with first root
    std::complex<double> root = roots[0];
    std::vector<std::complex<double>> p = {1.0, -root};
    
    // Multiply by (x - root) for each root
    for (size_t i = 1; i < roots.size(); i++) {
        root = roots[i];
        std::vector<std::complex<double>> temp(p.size() + 1, 0.0);
        
        for (size_t j = 0; j < p.size(); j++) {
            temp[j] += p[j];
            temp[j+1] += -root * p[j];
        }
        
        p = temp;
    }
    
    // Convert to real if the polynomial has only real coefficients
    RealVector result(p.size());
    for (size_t i = 0; i < p.size(); i++) {
        // Check if imaginary part is negligible (allowing for numerical errors)
        if (std::abs(p[i].imag()) > 1e-10) {
            throw std::runtime_error("Polynomial has complex coefficients");
        }
        result[i] = p[i].real();
    }
    
    return result;
}

// Validate and normalize frequency specifications
RealVector validate_wn(const RealVector& Wn, const std::string& btype, double fs) {
    FilterType filter_type = get_filter_type(btype);
    RealVector wn = Wn;
    
    // Check if frequencies are positive
    for (double w : wn) {
        if (w <= 0) {
            throw std::invalid_argument("Filter critical frequencies must be greater than 0");
        }
    }
    
    // If fs is specified, normalize frequencies
    if (fs > 0) {
        for (double& w : wn) {
            w = w / (fs/2);
        }
    }
    
    // Check if frequencies are in valid range for digital filters
    if (fs > 0 || fs == 0) { // Digital filter
        for (double w : wn) {
            if (w >= 1.0) {
                throw std::invalid_argument("Digital filter critical frequencies must be 0 < Wn < 1");
            }
        }
    }
    
    // For bandpass and bandstop filters, we need two frequencies
    if (filter_type == FilterType::BANDPASS || filter_type == FilterType::BANDSTOP) {
        if (wn.size() != 2) {
            throw std::invalid_argument("Bandpass and bandstop filters need two cutoff frequencies");
        }
        
        if (wn[0] >= wn[1]) {
            throw std::invalid_argument("First critical frequency must be less than the second one");
        }
    } else {
        // For lowpass and highpass, we need only one frequency
        if (wn.size() != 1) {
            throw std::invalid_argument("Lowpass and highpass filters need one cutoff frequency");
        }
    }
    
    return wn;
}

} // namespace filter 