#pragma once

#include <Eigen/Dense>
#include <unsupported/Eigen/CXX11/Tensor>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <optional>

namespace signaltools {

// Template function to slice along a specific axis of a tensor
template <typename Derived>
Eigen::Tensor<typename Derived::Scalar, Derived::NumIndices> 
axisSlice(const Eigen::TensorBase<Derived>& a, 
          std::optional<int> start = std::nullopt, 
          std::optional<int> stop = std::nullopt, 
          std::optional<int> step = std::nullopt, 
          int axis = -1) {
    using Scalar = typename Derived::Scalar;
    static constexpr int NumDims = Derived::NumIndices;
    
    // Convert negative axis to positive
    if (axis < 0) axis += NumDims;
    
    // Get tensor dimensions
    Eigen::array<Eigen::Index, NumDims> dimensions = a.dimensions();
    
    // Define slice parameters
    Eigen::Index startIdx = start.value_or(0);
    Eigen::Index stopIdx = stop.value_or(dimensions[axis]);
    Eigen::Index stepVal = step.value_or(1);
    
    // Handle negative indices
    if (startIdx < 0) startIdx += dimensions[axis];
    if (stopIdx < 0) stopIdx += dimensions[axis];
    
    // Create slice spec for each dimension
    Eigen::array<Eigen::IndexPair<Eigen::Index>, NumDims> slice;
    for (int i = 0; i < NumDims; ++i) {
        if (i == axis) {
            slice[i] = {startIdx, stopIdx, stepVal};
        } else {
            slice[i] = {0, dimensions[i], 1};
        }
    }
    
    return a.slice(slice);
}

// Simplified version for 1D vectors
template <typename Scalar>
Eigen::Matrix<Scalar, Eigen::Dynamic, 1> 
axisSlice(const Eigen::Matrix<Scalar, Eigen::Dynamic, 1>& a, 
          std::optional<int> start = std::nullopt, 
          std::optional<int> stop = std::nullopt, 
          std::optional<int> step = std::nullopt) {
    
    int size = a.size();
    int startIdx = start.value_or(0);
    int stopIdx = stop.value_or(size);
    int stepVal = step.value_or(1);
    
    // Handle negative indices
    if (startIdx < 0) startIdx += size;
    if (stopIdx < 0) stopIdx += size;
    
    // Ensure indices are within bounds
    startIdx = std::max(0, std::min(size, startIdx));
    stopIdx = std::max(0, std::min(size, stopIdx));
    
    // Calculate number of elements
    int numElements = (stopIdx - startIdx + stepVal - 1) / stepVal;
    
    // Create result vector
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> result(numElements);
    
    // Fill result
    for (int i = 0, idx = startIdx; idx < stopIdx; idx += stepVal, ++i) {
        result(i) = a(idx);
    }
    
    return result;
}

// Function to reverse an array along a specific axis
template <typename Derived>
Eigen::Tensor<typename Derived::Scalar, Derived::NumIndices> 
axisReverse(const Eigen::TensorBase<Derived>& a, int axis = -1) {
    return axisSlice(a, std::nullopt, std::nullopt, -1, axis);
}

// Simplified version for 1D vectors
template <typename Scalar>
Eigen::Matrix<Scalar, Eigen::Dynamic, 1> 
axisReverse(const Eigen::Matrix<Scalar, Eigen::Dynamic, 1>& a) {
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> result = a;
    std::reverse(result.data(), result.data() + result.size());
    return result;
}

// Function for odd extension of an array
template <typename Scalar>
Eigen::Matrix<Scalar, Eigen::Dynamic, 1> 
oddExt(const Eigen::Matrix<Scalar, Eigen::Dynamic, 1>& x, int n, int axis = -1) {
    if (n < 1) {
        return x;
    }
    
    int size = x.size();
    if (n > size - 1) {
        throw std::runtime_error("Extension length n is too big. It must not exceed x.size()-1.");
    }
    
    // Extract left and right ends
    Scalar leftEnd = x(0);
    Scalar rightEnd = x(size - 1);
    
    // Create left and right extensions
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> leftExt(n);
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> rightExt(n);
    
    for (int i = 0; i < n; ++i) {
        leftExt(i) = 2 * leftEnd - x(n - i);
        rightExt(i) = 2 * rightEnd - x(size - 2 - i);
    }
    
    // Concatenate the parts
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> result(size + 2 * n);
    result.segment(0, n) = leftExt;
    result.segment(n, size) = x;
    result.segment(n + size, n) = rightExt;
    
    return result;
}

// Function for even extension of an array
template <typename Scalar>
Eigen::Matrix<Scalar, Eigen::Dynamic, 1> 
evenExt(const Eigen::Matrix<Scalar, Eigen::Dynamic, 1>& x, int n, int axis = -1) {
    if (n < 1) {
        return x;
    }
    
    int size = x.size();
    if (n > size - 1) {
        throw std::runtime_error("Extension length n is too big. It must not exceed x.size()-1.");
    }
    
    // Create left and right extensions
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> leftExt(n);
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> rightExt(n);
    
    for (int i = 0; i < n; ++i) {
        leftExt(i) = x(n - i - 1);
        rightExt(i) = x(size - 2 - i);
    }
    
    // Concatenate the parts
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> result(size + 2 * n);
    result.segment(0, n) = leftExt;
    result.segment(n, size) = x;
    result.segment(n + size, n) = rightExt;
    
    return result;
}

// Function for constant extension of an array
template <typename Scalar>
Eigen::Matrix<Scalar, Eigen::Dynamic, 1> 
constExt(const Eigen::Matrix<Scalar, Eigen::Dynamic, 1>& x, int n, int axis = -1) {
    if (n < 1) {
        return x;
    }
    
    int size = x.size();
    
    // Extract left and right ends
    Scalar leftEnd = x(0);
    Scalar rightEnd = x(size - 1);
    
    // Create left and right extensions
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> leftExt = Eigen::Matrix<Scalar, Eigen::Dynamic, 1>::Constant(n, leftEnd);
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> rightExt = Eigen::Matrix<Scalar, Eigen::Dynamic, 1>::Constant(n, rightEnd);
    
    // Concatenate the parts
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> result(size + 2 * n);
    result.segment(0, n) = leftExt;
    result.segment(n, size) = x;
    result.segment(n + size, n) = rightExt;
    
    return result;
}

} // namespace signaltools 