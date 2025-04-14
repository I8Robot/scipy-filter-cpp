#include "filter_design.h"
#include <cmath>
#include <algorithm>
#include <functional>

namespace filter {

ZPK lp2lp_zpk(const ComplexVector& z, const ComplexVector& p, double k, double wo) {
    /*
     * Transform a lowpass filter prototype to a different frequency.
     * Return an analog low-pass filter with cutoff frequency wo
     * from an analog low-pass filter prototype with unity cutoff frequency.
     * 
     * This is derived from the s-plane substitution: s -> s/wo
     */
    
    // Make copies of inputs
    ComplexVector z_lp = z;
    ComplexVector p_lp = p;
    double k_lp = k;
    
    // Calculate relative degree
    int degree = relative_degree(z, p);
    
    // Scale all points radially from origin to shift cutoff frequency
    for (auto& zero : z_lp) {
        zero *= wo;
    }
    
    for (auto& pole : p_lp) {
        pole *= wo;
    }
    
    // Each shifted pole decreases gain by wo, each shifted zero increases it.
    // Cancel out the net change to keep overall gain the same
    k_lp *= std::pow(wo, degree);
    
    return {z_lp, p_lp, k_lp};
}

ZPK lp2hp_zpk(const ComplexVector& z, const ComplexVector& p, double k, double wo) {
    /*
     * Transform a lowpass filter prototype to a highpass filter.
     * Return an analog high-pass filter with cutoff frequency wo
     * from an analog low-pass filter prototype with unity cutoff frequency.
     * 
     * This is derived from the s-plane substitution: s -> wo/s
     */
    
    // Make copies of inputs
    ComplexVector z_hp;
    ComplexVector p_hp;
    double k_hp = k;
    
    // Calculate relative degree
    int degree = relative_degree(z, p);
    
    // Invert positions and scale for cutoff frequency
    for (auto& zero : z) {
        if (std::abs(zero) > 1e-10) {  // Avoid division by zero
            z_hp.push_back(wo / zero);
        }
    }
    
    for (auto& pole : p) {
        p_hp.push_back(wo / pole);
    }
    
    // Add zeros at the origin to match the original system's behavior
    z_hp.resize(z_hp.size() + degree, std::complex<double>(0.0, 0.0));
    
    // Compute the gain adjustment - 使用与Python一致的方法
    std::complex<double> z_prod(1.0, 0.0);
    std::complex<double> p_prod(1.0, 0.0);
    
    for (auto& zero : z) {
        z_prod *= -zero;
    }
    
    for (auto& pole : p) {
        p_prod *= -pole;
    }
    
    k_hp = k * std::real(z_prod / p_prod);
    
    return {z_hp, p_hp, k_hp};
}

ZPK lp2bp_zpk(const ComplexVector& z, const ComplexVector& p, double k, double wo, double bw) {
    /*
     * Transform a lowpass filter prototype to a bandpass filter.
     * Return an analog band-pass filter with center frequency wo and bandwidth bw
     * from an analog low-pass filter prototype with unity cutoff frequency.
     * 
     * This is derived from the s-plane substitution: s -> (s^2 + wo^2)/(s * bw)
     */
    
    // Make copies of inputs
    ComplexVector z_bp;
    ComplexVector p_bp;
    double k_bp = k;
    
    // Calculate relative degree
    int degree = relative_degree(z, p);
    
    // 准备临时容器，模拟Python的concatenate操作
    ComplexVector z_plus, z_minus;
    ComplexVector p_plus, p_minus;
    
    // Transform zeros
    for (auto& zero : z) {
        std::complex<double> zero_temp = zero * (bw/2);
        
        // We need to handle complex square root carefully
        std::complex<double> sqrt_term = std::sqrt(std::pow(zero_temp, 2) - std::pow(wo, 2));
        
        // 添加到临时容器中
        z_plus.push_back(zero_temp + sqrt_term);
        z_minus.push_back(zero_temp - sqrt_term);
    }
    
    // Transform poles
    for (auto& pole : p) {
        std::complex<double> pole_temp = pole * (bw/2);
        
        // We need to handle complex square root carefully
        std::complex<double> sqrt_term = std::sqrt(std::pow(pole_temp, 2) - std::pow(wo, 2));
        
        // 添加到临时容器中
        p_plus.push_back(pole_temp + sqrt_term);
        p_minus.push_back(pole_temp - sqrt_term);
    }
    
    // 模拟Python concatenate操作：先添加所有加项，再添加所有减项
    z_bp.insert(z_bp.end(), z_plus.begin(), z_plus.end());
    z_bp.insert(z_bp.end(), z_minus.begin(), z_minus.end());
    
    p_bp.insert(p_bp.end(), p_plus.begin(), p_plus.end());
    p_bp.insert(p_bp.end(), p_minus.begin(), p_minus.end());
    
    // Add zeros at the origin (s=0) 
    // Python: z_bp = append(z_bp, zeros(degree))
    for (int i = 0; i < degree; i++) {
        z_bp.push_back(std::complex<double>(0.0, 0.0));
    }
    
    // Calculate the gain adjustment - 使用与Python一致的计算方式
    // Python: k_bp = k * bw**degree
    k_bp = k * std::pow(bw, degree);
    
    return {z_bp, p_bp, k_bp};
}

ZPK lp2bs_zpk(const ComplexVector& z, const ComplexVector& p, double k, double wo, double bw) {
    /*
     * Transform a lowpass filter prototype to a bandstop filter.
     * Return an analog band-stop filter with center frequency wo and stopband width bw
     * from an analog low-pass filter prototype with unity cutoff frequency.
     * 
     * This is derived from the s-plane substitution: s -> (s * bw)/(s^2 + wo^2)
     */
    
    // Make copies of inputs
    ComplexVector z_bs;
    ComplexVector p_bs;
    double k_bs = k;
    
    // Calculate relative degree
    int degree = relative_degree(z, p);
    
    // Transform zeros (converting to highpass first)
    ComplexVector z_hp;
    for (auto& zero : z) {
        if (std::abs(zero) > 1e-10) {  // Avoid division by zero
            z_hp.push_back((bw/2) / zero);
        }
    }
    
    // Transform poles (converting to highpass first)
    ComplexVector p_hp;
    for (auto& pole : p) {
        p_hp.push_back((bw/2) / pole);
    }
    
    // 准备临时容器，模拟Python的concatenate操作
    ComplexVector z_plus, z_minus;
    ComplexVector p_plus, p_minus;
    
    // 转换零点并保持正确的顺序
    for (auto& zero : z_hp) {
        std::complex<double> sqrt_term = std::sqrt(std::pow(zero, 2) - std::pow(wo, 2));
        
        // 添加到临时容器中
        z_plus.push_back(zero + sqrt_term);
        z_minus.push_back(zero - sqrt_term);
    }
    
    // 转换极点并保持正确的顺序
    for (auto& pole : p_hp) {
        std::complex<double> sqrt_term = std::sqrt(std::pow(pole, 2) - std::pow(wo, 2));
        
        // 添加到临时容器中
        p_plus.push_back(pole + sqrt_term);
        p_minus.push_back(pole - sqrt_term);
    }
    
    // 模拟Python concatenate操作：先添加所有加项，再添加所有减项
    z_bs.insert(z_bs.end(), z_plus.begin(), z_plus.end());
    z_bs.insert(z_bs.end(), z_minus.begin(), z_minus.end());
    
    p_bs.insert(p_bs.end(), p_plus.begin(), p_plus.end());
    p_bs.insert(p_bs.end(), p_minus.begin(), p_minus.end());
    
    // Move any zeros that were at infinity to the center of the stopband
    // 完全模拟Python的行为: 先添加所有+j*wo的零点，再添加所有-j*wo的零点
    // Python: z_bs = append(z_bs, full(degree, +1j*wo))
    for (int i = 0; i < degree; i++) {
        z_bs.push_back(std::complex<double>(0.0, wo));
    }
    
    // Python: z_bs = append(z_bs, full(degree, -1j*wo))
    for (int i = 0; i < degree; i++) {
        z_bs.push_back(std::complex<double>(0.0, -wo));
    }
    
    // 计算增益 - 使用与Python一致的方法
    // k_bs = k * real(prod(-z) / prod(-p))
    std::complex<double> z_prod(1.0, 0.0);
    std::complex<double> p_prod(1.0, 0.0);
    
    for (auto& zero : z) {
        z_prod *= -zero;
    }
    
    for (auto& pole : p) {
        p_prod *= -pole;
    }
    
    k_bs = k * std::real(z_prod / p_prod);
    
    return {z_bs, p_bs, k_bs};
}

} // namespace filter 