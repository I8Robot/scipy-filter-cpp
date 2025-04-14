#include "filter_design.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <functional>

namespace filter {

ZPK bilinear_zpk(const ComplexVector& z, const ComplexVector& p, double k, double fs) {
    /*
     * Return a digital IIR filter from an analog one using a bilinear transform.
     *
     * Transform a set of poles and zeros from the analog s-plane to the digital
     * z-plane using Tustin's method, which substitutes 2*fs*(z-1)/(z+1) for s,
     * maintaining the shape of the frequency response.
     */
    
    // Make copies of inputs
    ComplexVector z_copy = z;
    ComplexVector p_copy = p;
    
    // Calculate relative degree
    int degree = relative_degree(z_copy, p_copy);
    
    double fs2 = 2.0 * fs;
    
    // Bilinear transform the poles and zeros
    ComplexVector z_z;
    ComplexVector p_z;
    
    for (auto& zero : z_copy) {
        z_z.push_back((fs2 + zero) / (fs2 - zero));
    }
    
    for (auto& pole : p_copy) {
        p_z.push_back((fs2 + pole) / (fs2 - pole));
    }
    
    // Any zeros that were at infinity get moved to the Nyquist frequency (-1)
    for (int i = 0; i < degree; i++) {
        z_z.push_back(std::complex<double>(-1.0, 0.0));
    }
    
    // Compensate for gain change
    double k_z = k;
    
    // Calculate prod(fs2 - z) / prod(fs2 - p)
    std::complex<double> num_prod(1.0, 0.0);
    for (auto& zero : z_copy) {
        num_prod *= (fs2 - zero);
    }
    
    std::complex<double> den_prod(1.0, 0.0);
    for (auto& pole : p_copy) {
        den_prod *= (fs2 - pole);
    }
    
    k_z *= std::real(num_prod / den_prod);
    
    return {z_z, p_z, k_z};
}

std::tuple<RealVector, RealVector> zpk2tf(const ComplexVector& z, const ComplexVector& p, double k) {
    /*
     * Return polynomial transfer function representation from zeros and poles
     */
    
    // Convert zeros to polynomial coefficients
    RealVector b = poly(z);
    
    // Scale by gain
    for (auto& coef : b) {
        coef *= k;
    }
    
    // Convert poles to polynomial coefficients
    RealVector a = poly(p);
    
    return std::make_tuple(b, a);
}

Matrix zpk2sos(const ComplexVector& z, const ComplexVector& p, double k, 
              const std::string& pairing, bool analog) {
    /*
     * Return second-order sections from zeros, poles, and gain of a system
     *
     * This implementation focuses on the 'nearest' pairing option only, which attempts
     * to minimize the peak gain of each biquadratic section.
     */
    
    // Check pairing option
    if (pairing != "nearest") {
        throw std::invalid_argument("Only 'nearest' pairing is implemented");
    }
    
    ComplexVector zeros = z;
    ComplexVector poles = p;
    double gain = k;
    
    // Sort poles by distance from unit circle (for digital) or imaginary axis (for analog)
    std::sort(poles.begin(), poles.end(), [analog](const std::complex<double>& a, const std::complex<double>& b) {
        if (analog) {
            // Sort by distance from imaginary axis (real part)
            return std::abs(a.real()) < std::abs(b.real());
        } else {
            // Sort by distance from unit circle
            return std::abs(std::abs(a) - 1.0) < std::abs(std::abs(b) - 1.0);
        }
    });
    
    // Ensure we have enough zeros (add zeros at -1 for digital, 0 for analog)
    while (zeros.size() < poles.size()) {
        if (analog) {
            zeros.push_back(std::complex<double>(0.0, 0.0));
        } else {
            zeros.push_back(std::complex<double>(-1.0, 0.0));
        }
    }
    
    // Get number of sections (each section is a 2nd order filter)
    size_t n_sections = (poles.size() + 1) / 2;
    
    // Create SOS matrix (each row is [b0, b1, b2, a0, a1, a2])
    Matrix sos(n_sections, 6);
    
    // Process every section
    for (size_t i = 0; i < n_sections; i++) {
        ComplexVector section_zeros;
        ComplexVector section_poles;
        
        if (i == n_sections - 1 && poles.size() % 2 == 1) {
            // Last section for odd order (first order section)
            section_poles.push_back(poles.back());
            poles.pop_back();
            
            // Find the closest zero
            double min_dist = std::numeric_limits<double>::max();
            size_t min_idx = 0;
            
            for (size_t j = 0; j < zeros.size(); j++) {
                double dist = std::abs(zeros[j] - section_poles[0]);
                if (dist < min_dist) {
                    min_dist = dist;
                    min_idx = j;
                }
            }
            
            section_zeros.push_back(zeros[min_idx]);
            zeros.erase(zeros.begin() + min_idx);
        } else {
            // Regular second order section
            
            // Take the next pole (which should be closest to unit circle)
            section_poles.push_back(poles.back());
            poles.pop_back();
            
            // If this pole is complex, add its conjugate pair
            if (std::abs(section_poles[0].imag()) > 1e-10) {
                section_poles.push_back(std::conj(section_poles[0]));
                poles.erase(std::find(poles.begin(), poles.end(), std::conj(section_poles[0])));
            } else {
                // Real pole, find the closest real pole
                double min_dist = std::numeric_limits<double>::max();
                size_t min_idx = 0;
                
                for (size_t j = 0; j < poles.size(); j++) {
                    if (std::abs(poles[j].imag()) < 1e-10) {
                        double dist = std::abs(poles[j].real() - section_poles[0].real());
                        if (dist < min_dist) {
                            min_dist = dist;
                            min_idx = j;
                        }
                    }
                }
                
                if (!poles.empty()) {
                    section_poles.push_back(poles[min_idx]);
                    poles.erase(poles.begin() + min_idx);
                }
            }
            
            // Find matching zeros
            for (size_t j = 0; j < section_poles.size(); j++) {
                double min_dist = std::numeric_limits<double>::max();
                size_t min_idx = 0;
                
                for (size_t k = 0; k < zeros.size(); k++) {
                    double dist = std::abs(zeros[k] - section_poles[j]);
                    if (dist < min_dist) {
                        min_dist = dist;
                        min_idx = k;
                    }
                }
                
                if (!zeros.empty()) {
                    section_zeros.push_back(zeros[min_idx]);
                    zeros.erase(zeros.begin() + min_idx);
                }
            }
        }
        
        // Convert this section to transfer function coefficients
        auto tf = zpk2tf(section_zeros, section_poles, 1.0);
        RealVector b = std::get<0>(tf);
        RealVector a = std::get<1>(tf);
        
        // Normalize a0 to be 1
        double a0 = a[0];
        for (auto& coef : b) {
            coef /= a0;
        }
        for (auto& coef : a) {
            coef /= a0;
        }
        
        // Pad coefficients if necessary (ensure we have 3 coefficients in each)
        while (b.size() < 3) {
            b.push_back(0.0);
        }
        while (a.size() < 3) {
            a.push_back(0.0);
        }
        
        // Fill the SOS matrix row
        for (int j = 0; j < 3; j++) {
            sos(i, j) = b[j];
            sos(i, j+3) = a[j];
        }
    }
    
    // Apply overall gain to first section
    sos(0, 0) *= gain;
    sos(0, 1) *= gain;
    sos(0, 2) *= gain;
    
    return sos;
}

} // namespace filter 