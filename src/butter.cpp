#include "filter_design.h"
#include <cmath>
#include <stdexcept>

namespace filter {

std::tuple<RealVector, RealVector> butter(
    int N, 
    const RealVector& Wn, 
    const std::string& btype, 
    bool analog, 
    const std::string& output, 
    double fs) {
    
    /*
     * Butterworth digital and analog filter design.
     * Design an Nth-order digital or analog Butterworth filter and return the filter coefficients.
     *
     * Parameters:
     * -----------
     * N : int
     *    The order of the filter.
     * Wn : array_like
     *    The critical frequency or frequencies. For lowpass and highpass filters, Wn is a scalar;
     *    for bandpass and bandstop filters, Wn is a length-2 sequence.
     * btype : {'lowpass', 'highpass', 'bandpass', 'bandstop'}, optional
     *    The type of filter. Default is 'lowpass'.
     * analog : bool, optional
     *    When True, return an analog filter, otherwise a digital filter is returned.
     * output : {'ba', 'zpk', 'sos'}, optional
     *    Type of output. Default is 'ba'.
     * fs : float, optional
     *    The sampling frequency of the digital system.
     *
     * Returns:
     * --------
     * b, a : ndarray, ndarray
     *    Numerator (b) and denominator (a) polynomials of the IIR filter.
     *    Only returned if output='ba'.
     */
    
    // Validate input parameters
    if (N <= 0) {
        throw std::invalid_argument("Filter order must be a positive integer");
    }
    
    // Call the more general iirfilter function with ftype='butter'
    return iirfilter(N, Wn, 0.0, 0.0, btype, analog, "butter", output, fs);
}

std::tuple<ComplexVector, ComplexVector, double> butter_zpk(
    int N, 
    const RealVector& Wn, 
    const std::string& btype, 
    bool analog, 
    double fs) {
    
    /*
     * Butterworth digital and analog filter design in zero-pole-gain format.
     * Design an Nth-order digital or analog Butterworth filter and return the zeros, poles, and gain.
     */
    
    // Validate input parameters
    if (N <= 0) {
        throw std::invalid_argument("Filter order must be a positive integer");
    }
    
    // Call the ZPK version of iirfilter with ftype='butter'
    return iirfilter_zpk(N, Wn, 0.0, 0.0, btype, analog, "butter", fs);
}

Matrix butter_sos(
    int N, 
    const RealVector& Wn, 
    const std::string& btype, 
    bool analog, 
    double fs) {
    
    /*
     * Butterworth digital and analog filter design in second-order sections format.
     * Design an Nth-order digital or analog Butterworth filter and return the SOS matrix.
     */
    
    // Validate input parameters
    if (N <= 0) {
        throw std::invalid_argument("Filter order must be a positive integer");
    }
    
    // Call the SOS version of iirfilter with ftype='butter'
    return iirfilter_sos(N, Wn, 0.0, 0.0, btype, analog, "butter", fs);
}

} // namespace filter 