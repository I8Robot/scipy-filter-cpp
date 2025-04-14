#ifndef FILTER_DESIGN_H
#define FILTER_DESIGN_H

#include <Eigen/Dense>
#include <complex>
#include <vector>
#include <string>
#include <tuple>
#include <stdexcept>
#include <cmath>

namespace filter {

// Typedefs for easier type handling
using ComplexVector = std::vector<std::complex<double>>;
using RealVector = std::vector<double>;
using Matrix = Eigen::MatrixXd;
using ComplexMatrix = Eigen::MatrixXcd;

// ZPK representation
struct ZPK {
    ComplexVector z; // zeros
    ComplexVector p; // poles
    double k;       // gain
};

// Output formats
enum class OutputType {
    BA,  // Transfer function (b, a)
    ZPK, // Zeros, poles, gain
    SOS  // Second-order sections
};

// Filter types
enum class FilterType {
    LOWPASS,
    HIGHPASS,
    BANDPASS,
    BANDSTOP
};

// Constants
constexpr double pi = 3.14159265358979323846;

// Function declarations

// 调试函数 - 打印ZPK数据到命令行窗口
void print_zpk(const ComplexVector& zeros, const ComplexVector& poles, double gain, const std::string& title = "Filter ZPK");
void print_zpk(const ZPK& zpk, const std::string& title = "Filter ZPK");

// Main filter design functions
std::tuple<RealVector, RealVector> butter(
    int N, 
    const RealVector& Wn, 
    const std::string& btype = "low", 
    bool analog = false, 
    const std::string& output = "ba", 
    double fs = 0.0);

std::tuple<ComplexVector, ComplexVector, double> butter_zpk(
    int N, 
    const RealVector& Wn, 
    const std::string& btype = "low", 
    bool analog = false, 
    double fs = 0.0);

Matrix butter_sos(
    int N, 
    const RealVector& Wn, 
    const std::string& btype = "low", 
    bool analog = false, 
    double fs = 0.0);

// Internal functions
std::tuple<RealVector, RealVector> iirfilter(
    int N, 
    const RealVector& Wn, 
    double rp = 0.0, 
    double rs = 0.0, 
    const std::string& btype = "band", 
    bool analog = false, 
    const std::string& ftype = "butter", 
    const std::string& output = "ba", 
    double fs = 0.0);

std::tuple<ComplexVector, ComplexVector, double> iirfilter_zpk(
    int N, 
    const RealVector& Wn, 
    double rp = 0.0, 
    double rs = 0.0, 
    const std::string& btype = "band", 
    bool analog = false, 
    const std::string& ftype = "butter", 
    double fs = 0.0);

Matrix iirfilter_sos(
    int N, 
    const RealVector& Wn, 
    double rp = 0.0, 
    double rs = 0.0, 
    const std::string& btype = "band", 
    bool analog = false, 
    const std::string& ftype = "butter", 
    double fs = 0.0);

// Prototype functions
ZPK buttap(int N);

// Filter transformations
ZPK lp2lp_zpk(const ComplexVector& z, const ComplexVector& p, double k, double wo = 1.0);
ZPK lp2hp_zpk(const ComplexVector& z, const ComplexVector& p, double k, double wo = 1.0);
ZPK lp2bp_zpk(const ComplexVector& z, const ComplexVector& p, double k, double wo = 1.0, double bw = 1.0);
ZPK lp2bs_zpk(const ComplexVector& z, const ComplexVector& p, double k, double wo = 1.0, double bw = 1.0);

// Conversion functions
ZPK bilinear_zpk(const ComplexVector& z, const ComplexVector& p, double k, double fs);
std::tuple<RealVector, RealVector> zpk2tf(const ComplexVector& z, const ComplexVector& p, double k);
Matrix zpk2sos(const ComplexVector& z, const ComplexVector& p, double k, 
              const std::string& pairing = "nearest", bool analog = false);

// Utility functions
int relative_degree(const ComplexVector& z, const ComplexVector& p);
FilterType get_filter_type(const std::string& btype);
OutputType get_output_type(const std::string& output);
RealVector poly(const ComplexVector& roots);
RealVector validate_wn(const RealVector& Wn, const std::string& btype, double fs = 0.0);

} // namespace filter

#endif // FILTER_DESIGN_H 