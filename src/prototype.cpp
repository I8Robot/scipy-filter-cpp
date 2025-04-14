#include "filter_design.h"
#include <cmath>

namespace filter {

ZPK buttap(int N) {
    /*
     * Return (z,p,k) for analog prototype of Nth-order Butterworth filter.
     * The filter will have an angular (e.g., rad/s) cutoff frequency of 1.
     */
    
    if (std::abs(N) != N) {
        throw std::invalid_argument("Filter order must be a nonnegative integer");
    }
    
    // Zeros (empty for Butterworth)
    ComplexVector z;
    
    // Poles are located on a circle in the left half-plane
    ComplexVector p(N);
    for (int i = 0; i < N; i++) {
        // m = -N+1, -N+3, ..., N-1
        int m = -N + 1 + 2 * i;
        // Compute angle and use exp to place on unit circle, then negate for left half-plane
        p[i] = -std::exp(std::complex<double>(0, pi * m / (2 * N)));
    }
    
    // Gain for Butterworth is always 1
    double k = 1.0;
    
    return {z, p, k};
}

} // namespace filter 