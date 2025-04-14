#pragma once

#include "arraytools.h"
#include <Eigen/Dense>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <optional>
#include <complex>
#include <tuple>
#include <utility>
#include <iomanip>

namespace signaltools {

// Create a companion matrix as in SciPy's implementation
template <typename Scalar>
Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic>
companion(const Eigen::Matrix<Scalar, Eigen::Dynamic, 1>& a) {
    int n = a.size();
    
    if (n < 2) {
        throw std::invalid_argument("The length of `a` must be at least 2.");
    }
    
    if (a(0) == Scalar(0)) {
        throw std::invalid_argument("The first coefficient of `a` must not be zero.");
    }
    
    Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> c = 
        Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic>::Zero(n - 1, n - 1);
    
    // First row is -a[1:]/a[0]
    for (int i = 0; i < n - 1; ++i) {
        c(0, i) = -a(i + 1) / a(0);
    }
    
    // First subdiagonal is all ones
    for (int i = 0; i < n - 2; ++i) {
        c(i + 1, i) = 1;
    }
    
    return c;
}

// Helper function to validate padding for filtfilt
template <typename Scalar>
std::tuple<int, Eigen::Matrix<Scalar, Eigen::Dynamic, 1>> 
validatePad(const std::string& padtype, 
           std::optional<int> padlen, 
           const Eigen::Matrix<Scalar, Eigen::Dynamic, 1>& x, 
           int ntaps) {
    // Check padtype
    if (padtype != "even" && padtype != "odd" && padtype != "constant" && padtype != "none") {
        throw std::invalid_argument("Unknown value for padtype. padtype must be 'even', 'odd', 'constant', or 'none'.");
    }
    
    int edge;
    if (padtype == "none") {
        edge = 0;
    } else if (!padlen.has_value()) {
        // Original padding; preserved for backwards compatibility
        edge = ntaps * 3;
    } else {
        edge = padlen.value();
    }
    
    // Check if x is long enough
    if (x.size() <= edge) {
        throw std::invalid_argument("The length of the input vector x must be greater than padlen.");
    }
    
    // Create the extended signal
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> ext;
    if (padtype != "none" && edge > 0) {
        if (padtype == "even") {
            ext = evenExt(x, edge);
        } else if (padtype == "odd") {
            ext = oddExt(x, edge);
        } else if (padtype == "constant") {
            ext = constExt(x, edge);
        }
    } else {
        ext = x;
    }
    
    return std::make_tuple(edge, ext);
}

// Compute initial conditions for lfilter
template <typename Scalar>
Eigen::Matrix<Scalar, Eigen::Dynamic, 1> 
lfilterZi(const Eigen::Matrix<Scalar, Eigen::Dynamic, 1>& b, 
         const Eigen::Matrix<Scalar, Eigen::Dynamic, 1>& a) {
    // Check inputs
    if (a.size() < 1) {
        throw std::invalid_argument("There must be at least one 'a' coefficient.");
    }
    
    // Normalize coefficients if a[0] != 1.0
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> b_norm = (a[0] != Scalar(1.0)) ? b / a[0] : b;
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> a_norm = (a[0] != Scalar(1.0)) ? a / a[0] : a;
    
    // Determine filter order
    int n = std::max(a.size(), b.size());
    
    // Pad coefficients to the same length
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> b_padded = Eigen::Matrix<Scalar, Eigen::Dynamic, 1>::Zero(n);
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> a_padded = Eigen::Matrix<Scalar, Eigen::Dynamic, 1>::Zero(n);
    b_padded.head(b_norm.size()) = b_norm;
    a_padded.head(a_norm.size()) = a_norm;
    
    // Create the companion matrix for polynomial a_padded
    Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> A = companion(a_padded);
    
    // Create the I - A matrix (using the transpose of companion matrix, as in Python implementation)
    Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> IminusA = Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic>::Identity(n - 1, n - 1) - A.transpose();
    
    // Create the B vector
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> B(n - 1);
    for (int i = 0; i < n - 1; ++i) {
        B(i) = b_padded(i + 1) - a_padded(i + 1) * b_padded(0);
    }
    
    
    // Solve for initial conditions
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> zi = IminusA.colPivHouseholderQr().solve(B);

    Eigen::VectorXd residual = IminusA * zi - B;
    double residual_norm = residual.norm();
    
    return zi;
}

// lfilter function implementation with explicit initial conditions
template <typename Scalar>
std::tuple<Eigen::Matrix<Scalar, Eigen::Dynamic, 1>, Eigen::Matrix<Scalar, Eigen::Dynamic, 1>> 
lfilter_with_zi(const Eigen::Matrix<Scalar, Eigen::Dynamic, 1>& b,
        const Eigen::Matrix<Scalar, Eigen::Dynamic, 1>& a,
        const Eigen::Matrix<Scalar, Eigen::Dynamic, 1>& x,
        const Eigen::Matrix<Scalar, Eigen::Dynamic, 1>& zi) {
    // Normalize coefficients if a[0] != 1.0
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> b_norm = (a[0] != Scalar(1.0)) ? b / a[0] : b;
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> a_norm = (a[0] != Scalar(1.0)) ? a / a[0] : a;

    
    int len_x = x.size();
    int len_b = b_norm.size();
    int len_a = a_norm.size();
    
    // Length of the filter state
    int n = std::max(len_a, len_b) - 1;
    
    // Create output array 
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> y = Eigen::Matrix<Scalar, Eigen::Dynamic, 1>::Zero(len_x);
    
    // Initialize filter state
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> z = zi;
    if (z.size() != n) {
        throw std::invalid_argument("Length of zi must be equal to max(len(a), len(b)) - 1");
    }
    
    // Copy of the final filter state
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> zf = z;
    
    // Apply the filter
    for (int i = 0; i < len_x; ++i) {
        // Calculate output from input and state
        y(i) = b_norm(0) * x(i) + (n > 0 ? z(0) : 0);
        
        // Update state
        for (int j = 0; j < n - 1; ++j) {
            z(j) = z(j + 1);
            if (j + 1 < len_b) {
                z(j) += b_norm(j + 1) * x(i);
            }
            if (j + 1 < len_a) {
                z(j) -= a_norm(j + 1) * y(i);
            }
        }
        
        // Update last element of state
        if (n > 0) {
            z(n - 1) = 0;
            if (len_b > n) {
                z(n - 1) += b_norm(n) * x(i);
            }
            if (len_a > n) {
                z(n - 1) -= a_norm(n) * y(i);
            }
        }
    }
    
    // Copy the final state for the output
    zf = z;
    
    return std::make_tuple(y, zf);
}

// lfilter function implementation with optional initial conditions
template <typename Scalar>
std::tuple<Eigen::Matrix<Scalar, Eigen::Dynamic, 1>, Eigen::Matrix<Scalar, Eigen::Dynamic, 1>> 
lfilter(const Eigen::Matrix<Scalar, Eigen::Dynamic, 1>& b,
        const Eigen::Matrix<Scalar, Eigen::Dynamic, 1>& a,
        const Eigen::Matrix<Scalar, Eigen::Dynamic, 1>& x,
        std::optional<Eigen::Matrix<Scalar, Eigen::Dynamic, 1>> zi = std::nullopt) {
    // Length of the filter state
    int n = std::max(a.size(), b.size()) - 1;
    
    // Initialize filter state
    if (zi.has_value()) {
        return lfilter_with_zi(b, a, x, zi.value());
    } else {
        Eigen::Matrix<Scalar, Eigen::Dynamic, 1> z = Eigen::Matrix<Scalar, Eigen::Dynamic, 1>::Zero(n);
        return lfilter_with_zi(b, a, x, z);
    }
}

// Overload for lfilter without zi - just return the filtered signal without final state
template <typename Scalar>
Eigen::Matrix<Scalar, Eigen::Dynamic, 1>
lfilter_output_only(const Eigen::Matrix<Scalar, Eigen::Dynamic, 1>& b,
                    const Eigen::Matrix<Scalar, Eigen::Dynamic, 1>& a,
                    const Eigen::Matrix<Scalar, Eigen::Dynamic, 1>& x) {
    std::optional<Eigen::Matrix<Scalar, Eigen::Dynamic, 1>> opt_zi = std::nullopt;
    auto result = lfilter(b, a, x, opt_zi);
    return std::get<0>(result);
}

// Implementation of Gustafsson's method for filtfilt
template <typename Scalar>
std::tuple<Eigen::Matrix<Scalar, Eigen::Dynamic, 1>, 
          Eigen::Matrix<Scalar, Eigen::Dynamic, 1>, 
          Eigen::Matrix<Scalar, Eigen::Dynamic, 1>> 
filtfiltGust(const Eigen::Matrix<Scalar, Eigen::Dynamic, 1>& b,
             const Eigen::Matrix<Scalar, Eigen::Dynamic, 1>& a,
             const Eigen::Matrix<Scalar, Eigen::Dynamic, 1>& x,
             std::optional<int> irlen = std::nullopt) {
    // Determine filter order
    int order = std::max(a.size(), b.size()) - 1;
    
    // Handle trivial case
    if (order == 0) {
        Scalar scale = (b(0) / a(0)) * (b(0) / a(0));
        Eigen::Matrix<Scalar, Eigen::Dynamic, 1> y = x * scale;
        Eigen::Matrix<Scalar, Eigen::Dynamic, 1> empty;
        return std::make_tuple(y, empty, empty);
    }
    
    // Determine signal length
    int n = x.size();
    
    // Determine impulse response length
    int m;
    if (!irlen.has_value() || n <= 2 * irlen.value()) {
        m = n;
    } else {
        m = irlen.value();
    }
    
    // Create observability matrix (O in Gustafsson's paper)
    Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> Obs = Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic>::Zero(m, order);
    
    // Initialize first column of Obs with impulse response
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> zi = Eigen::Matrix<Scalar, Eigen::Dynamic, 1>::Zero(order);
    zi(0) = 1;
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> impulse = Eigen::Matrix<Scalar, Eigen::Dynamic, 1>::Zero(m);
    std::optional<Eigen::Matrix<Scalar, Eigen::Dynamic, 1>> opt_zi = zi;
    auto filtered_result = lfilter(b, a, impulse, opt_zi);
    using first_type = Eigen::Matrix<Scalar, Eigen::Dynamic, 1>;
    using second_type = Eigen::Matrix<Scalar, Eigen::Dynamic, 1>;
    Obs.col(0) = std::get<0>(filtered_result);
    
    // Fill the rest of the Obs matrix
    for (int k = 1; k < order; ++k) {
        for (int j = k; j < m; ++j) {
            Obs(j, k) = Obs(j - k, 0);
        }
    }
    
    // Obsr is O^R (row-reversed O)
    Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> Obsr = Obs.colwise().reverse();
    
    // Create S matrix
    Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> S(m, order);
    std::optional<Eigen::Matrix<Scalar, Eigen::Dynamic, 1>> no_zi;
    for (int i = 0; i < order; ++i) {
        Eigen::Matrix<Scalar, Eigen::Dynamic, 1> col = Obs.col(i).reverse();
        auto temp = lfilter(b, a, col, no_zi);
        S.col(i) = std::get<0>(temp);
    }
    
    // Sr is S^R (row-reversed S)
    Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> Sr = S.colwise().reverse();
    
    // M is [(S^R - O), (O^R - S)]
    Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> M;
    if (m == n) {
        M.resize(m, 2 * order);
        M.leftCols(order) = Sr - Obs;
        M.rightCols(order) = Obsr - S;
    } else {
        M.resize(2 * m, 2 * order);
        M.topLeftCorner(m, order) = Sr - Obs;
        M.bottomRightCorner(m, order) = Obsr - S;
    }
    
    // Naive forward-backward and backward-forward filters
    auto temp_f = lfilter(b, a, x, no_zi);
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> y_f = std::get<0>(temp_f);
    
    // Create reversed version for backward filter
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> y_f_reverse = y_f.reverse();
    auto temp_fb = lfilter(b, a, y_f_reverse, no_zi);
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> y_fb = std::get<0>(temp_fb).reverse();
    
    // Now do backward-forward
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> x_reverse = x.reverse();
    auto temp_b = lfilter(b, a, x_reverse, no_zi);
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> y_b = std::get<0>(temp_b).reverse();
    
    auto temp_bf = lfilter(b, a, y_b, no_zi);
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> y_bf = std::get<0>(temp_bf);
    
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> delta_y_bf_fb = y_bf - y_fb;
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> delta;
    
    if (m == n) {
        delta = delta_y_bf_fb;
    } else {
        delta.resize(2 * m);
        delta.head(m) = delta_y_bf_fb.head(m);
        delta.tail(m) = delta_y_bf_fb.tail(m);
    }
    
    // Solve least squares for optimal initial conditions
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> ic_opt = M.colPivHouseholderQr().solve(delta);
    
    // Create W matrix
    Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> W;
    if (m == n) {
        W.resize(m, 2 * order);
        W.leftCols(order) = Sr;
        W.rightCols(order) = Obsr;
    } else {
        W.resize(2 * m, 2 * order);
        W.topLeftCorner(m, order) = Sr;
        W.bottomRightCorner(m, order) = Obsr;
    }
    
    // Calculate optimal output
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> wic = W * ic_opt;
    
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> y_opt = y_fb;
    if (m == n) {
        y_opt += wic;
    } else {
        y_opt.head(m) += wic.head(m);
        y_opt.tail(m) += wic.tail(m);
    }
    
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> x0 = ic_opt.head(order);
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> x1 = ic_opt.tail(order);
    
    return std::make_tuple(y_opt, x0, x1);
}

// Main filtfilt function 
template <typename Scalar>
Eigen::Matrix<Scalar, Eigen::Dynamic, 1>
filtfilt(const Eigen::Matrix<Scalar, Eigen::Dynamic, 1>& b,
         const Eigen::Matrix<Scalar, Eigen::Dynamic, 1>& a,
         const Eigen::Matrix<Scalar, Eigen::Dynamic, 1>& x,
         const std::string& padtype = "odd",
         std::optional<int> padlen = std::nullopt,
         const std::string& method = "pad",
         std::optional<int> irlen = std::nullopt) {
    
    // Check method
    if (method != "pad" && method != "gust") {
        throw std::invalid_argument("method must be 'pad' or 'gust'.");
    }
    
    // Use Gustafsson's method if requested
    if (method == "gust") {
        auto result = filtfiltGust(b, a, x, irlen);
        using gust_first_type = Eigen::Matrix<Scalar, Eigen::Dynamic, 1>;
        using gust_second_type = std::tuple<Eigen::Matrix<Scalar, Eigen::Dynamic, 1>, 
                                           Eigen::Matrix<Scalar, Eigen::Dynamic, 1>>;
        return std::get<0>(result);
    }
    
    // Otherwise use padding method
    int ntaps = std::max(a.size(), b.size());
    auto pad_result = validatePad(padtype, padlen, x, ntaps);
    using pad_first_type = int;
    using pad_second_type = Eigen::Matrix<Scalar, Eigen::Dynamic, 1>;
    int edge = std::get<0>(pad_result);
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> ext = std::get<1>(pad_result);
    
    // Get the steady state of the filter's step response
    auto zi = lfilterZi(b, a);

    // Scale zi by first value of ext
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> zi_x = zi * ext(0);
    
    // Forward filter
    std::optional<Eigen::Matrix<Scalar, Eigen::Dynamic, 1>> opt_zi_x = zi_x;
    auto forward_result = lfilter(b, a, ext, opt_zi_x);
    using filter_first_type = Eigen::Matrix<Scalar, Eigen::Dynamic, 1>;
    using filter_second_type = Eigen::Matrix<Scalar, Eigen::Dynamic, 1>;
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> y = std::get<0>(forward_result);
    
    // Backward filter
    // Create zi*y0 for initial conditions
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> zi_y = zi * y(y.size() - 1);
    
    // Reverse y and apply filter
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> y_reverse = y.reverse();
    std::optional<Eigen::Matrix<Scalar, Eigen::Dynamic, 1>> opt_zi_y = zi_y;
    auto backward_result = lfilter(b, a, y_reverse, opt_zi_y);
    
    // Get result and reverse again
    y = std::get<0>(backward_result).reverse();
    
    // If there was padding, remove it
    if (edge > 0) {
        Eigen::Matrix<Scalar, Eigen::Dynamic, 1> result(x.size());
        result = y.segment(edge, x.size());
        return result;
    }
    
    return y;
}

} // namespace signaltools 