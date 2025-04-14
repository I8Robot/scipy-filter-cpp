#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import numpy as np
from scipy import signal
import json
import os

def save_coefficients(filename, b, a):
    """Save coefficients to a file for comparison with C++ output"""
    with open(filename, 'w') as f:
        f.write(f"b = {b.tolist()}\n")
        f.write(f"a = {a.tolist()}\n")
    print(f"Coefficients saved to {filename}")

def print_coefficients(name, coeffs):
    """Print coefficients with high precision"""
    print(f"{name}: {coeffs.tolist()}")

def main():
    print("======= Testing Python Butterworth Filter Design =======")
    
    # Get the current directory and the C++ output directory
    current_dir = os.path.dirname(os.path.abspath(__file__))
    cpp_output_dir = os.path.join(current_dir, "..", "build", "Release")
    
    # Test cases - same as the C++ test cases
    test_cases = [
        # Low-pass filters
        {"order": 2, "Wn": 0.1, "btype": "low", "description": "2nd order lowpass (Wn=0.1)"},
        {"order": 4, "Wn": 0.2, "btype": "low", "description": "4th order lowpass (Wn=0.2)"},
        {"order": 6, "Wn": 0.3, "btype": "low", "description": "6th order lowpass (Wn=0.3)"},
        
        # High-pass filters
        {"order": 2, "Wn": 0.15, "btype": "high", "description": "2nd order highpass (Wn=0.15)"},
        {"order": 4, "Wn": 0.25, "btype": "high", "description": "4th order highpass (Wn=0.25)"},
        {"order": 6, "Wn": 0.35, "btype": "high", "description": "6th order highpass (Wn=0.35)"},
        
        # Band-pass filters
        {"order": 2, "Wn": [0.1, 0.4], "btype": "band", "description": "2nd order bandpass (Wn=[0.1, 0.4])"},
        {"order": 4, "Wn": [0.1, 0.3], "btype": "band", "description": "4th order bandpass (Wn=[0.1, 0.3])"},
        
        # Band-stop filters
        {"order": 2, "Wn": [0.1, 0.4], "btype": "stop", "description": "2nd order bandstop (Wn=[0.1, 0.4])"},
        {"order": 4, "Wn": [0.1, 0.3], "btype": "stop", "description": "4th order bandstop (Wn=[0.1, 0.3])"}
    ]
    
    # Process each test case
    for i, tc in enumerate(test_cases):
        print(f"\n===== Test Case {i+1}: {tc['description']} =====")
        
        try:
            # Call SciPy's butter function
            b, a = signal.butter(tc["order"], tc["Wn"], tc["btype"], analog=False, output='ba')
            
            # Print results
            print_coefficients("b", b)
            print_coefficients("a", a)
            
            # Save to file for C++ comparison
            filename = f"py_butter_{i+1}.txt"
            save_coefficients(filename, b, a)
        except Exception as e:
            print(f"Error: {e}")
    
    print("\nAll tests completed. Compare with C++ results.")
    
    # Add a comparison function to directly compare Python and C++ outputs
    print("\n======= Comparing Python and C++ Results =======")
    for i in range(1, len(test_cases) + 1):
        py_file = f"py_butter_{i}.txt"
        cpp_file = f"cpp_butter_{i}.txt"
        
        if os.path.exists(py_file) and os.path.exists(cpp_file):
            print(f"\n----- Test Case {i} -----")
            # Parse Python results
            with open(py_file, 'r') as f:
                py_content = f.read()
            py_b = eval(py_content.split('\n')[0].replace('b = ', ''))
            py_a = eval(py_content.split('\n')[1].replace('a = ', ''))
            
            # Parse C++ results
            with open(cpp_file, 'r') as f:
                cpp_content = f.read()
            cpp_b = eval(cpp_content.split('\n')[0].replace('b = ', ''))
            cpp_a = eval(cpp_content.split('\n')[1].replace('a = ', ''))
            
            # Compare results
            b_match = np.allclose(np.array(py_b), np.array(cpp_b), rtol=1e-14, atol=1e-14)
            a_match = np.allclose(np.array(py_a), np.array(cpp_a), rtol=1e-14, atol=1e-14)
            
            print(f"b coefficients match: {'✓' if b_match else '✗'}")
            print(f"a coefficients match: {'✓' if a_match else '✗'}")
            
            if not b_match or not a_match:
                print("Differences found:")
                if not b_match:
                    print("B coefficients:")
                    print(f"  Python: {py_b}")
                    print(f"  C++:    {cpp_b}")
                    print(f"  Abs diff: {np.abs(np.array(py_b) - np.array(cpp_b)).max()}")
                if not a_match:
                    print("A coefficients:")
                    print(f"  Python: {py_a}")
                    print(f"  C++:    {cpp_a}")
                    print(f"  Abs diff: {np.abs(np.array(py_a) - np.array(cpp_a)).max()}")
        else:
            print(f"Test case {i}: Files not found for comparison")

if __name__ == "__main__":
    main() 