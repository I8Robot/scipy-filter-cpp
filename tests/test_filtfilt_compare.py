#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import numpy as np
import sys
import traceback
try:
    import scipy
    from scipy import signal
    print(f"Successfully imported scipy version: {scipy.__version__}")
except ImportError as e:
    print(f"Error importing scipy: {e}")
    print("Make sure scipy is installed in your environment.")
    sys.exit(1)
import json
import os
import matplotlib.pyplot as plt

def save_filtered_signal(filename, filtered):
    """Save filtered signal to a file for comparison with C++ output"""
    with open(filename, 'w') as f:
        f.write(f"filtered = {filtered.tolist()}\n")
    print(f"Filtered signal saved to {filename}")

def main():
    print("======= Testing Python FiltFilt Implementation =======")
    
    # 加载C++生成的测试信号
    try:
        with open("cpp_test_signal.txt", 'r') as f:
            content = f.read()
            test_signal = np.array(eval(content.split('=')[1].strip()))
        print(f"Loaded test signal with length {len(test_signal)}")
    except Exception as e:
        print(f"Error loading test signal: {e}")
        print("Generating a new signal...")
        # 如果无法加载C++生成的信号，生成一个新的
        sample_rate = 1000.0  # 1kHz
        t = np.arange(1000) / sample_rate
        test_signal = (2.0 * np.sin(2 * np.pi * 5 * t) +
                 1.0 * np.sin(2 * np.pi * 50 * t) +
                 0.5 * np.sin(2 * np.pi * 120 * t) +
                 np.random.normal(0, 0.2, size=len(t)))
    
    # 测试用例 - 与C++测试用例相同
    test_cases = [
        # 低通滤波器
        {"order": 2, "Wn": 0.1, "btype": "low", "description": "2nd order lowpass (Wn=0.1)"},
        {"order": 4, "Wn": 0.2, "btype": "low", "description": "4th order lowpass (Wn=0.2)"},
        {"order": 6, "Wn": 0.01, "btype": "low", "description": "6th order lowpass (Wn=0.01)"},
        
        # 高通滤波器
        {"order": 2, "Wn": 0.15, "btype": "high", "description": "2nd order highpass (Wn=0.15)"},
        {"order": 4, "Wn": 0.25, "btype": "high", "description": "4th order highpass (Wn=0.25)"},
        {"order": 6, "Wn": 0.4, "btype": "high", "description": "6th order highpass (Wn=0.4)"},
        
        # 带通滤波器
        {"order": 2, "Wn": [0.1, 0.4], "btype": "band", "description": "2nd order bandpass (Wn=[0.1, 0.4])"},
        {"order": 4, "Wn": [0.1, 0.3], "btype": "band", "description": "4th order bandpass (Wn=[0.1, 0.3])"},
        
        # 带阻滤波器
        {"order": 2, "Wn": [0.1, 0.4], "btype": "stop", "description": "2nd order bandstop (Wn=[0.1, 0.4])"},
        {"order": 4, "Wn": [0.1, 0.3], "btype": "stop", "description": "4th order bandstop (Wn=[0.1, 0.3])"}
    ]
    
    # 处理每个测试用例
    for i, tc in enumerate(test_cases):
        print(f"\n===== Test Case {i+1}: {tc['description']} =====")
        
        try:
            print(f"Creating filter with: order={tc['order']}, Wn={tc['Wn']}, btype={tc['btype']}")
            # 创建Butterworth滤波器系数
            b, a = signal.butter(tc["order"], tc["Wn"], tc["btype"], analog=False, output='ba')
            print(f"Filter coefficients: b={b}..., a={a}...")
            
            # 应用filtfilt滤波器
            filtered = signal.filtfilt(b, a, test_signal)
            print(f"Filtered signal shape: {filtered.shape}")
            
            # 保存滤波后的信号
            filename = f"py_filtfilt_{i+1}.txt"
            save_filtered_signal(filename, filtered)
        except Exception as e:
            print(f"Error in test case {i+1}: {e}")
            traceback.print_exc()
    
    print("\nAll tests completed. Compare with C++ results.")
    
    # 比较Python和C++结果
    print("\n======= Comparing Python and C++ Results =======")
    
    max_diff_all = 0
    mean_diff_all = 0
    results = []
    
    for i in range(1, len(test_cases) + 1):
        py_file = f"py_filtfilt_{i}.txt"
        cpp_file = f"cpp_filtfilt_{i}.txt"
        
        if os.path.exists(py_file) and os.path.exists(cpp_file):
            print(f"\n----- Test Case {i}: {test_cases[i-1]['description']} -----")
            try:
                # 解析Python结果
                with open(py_file, 'r') as f:
                    py_content = f.read()
                py_filtered = np.array(eval(py_content.split('=')[1].strip()))
                
                # 解析C++结果
                with open(cpp_file, 'r') as f:
                    cpp_content = f.read()
                cpp_filtered = np.array(eval(cpp_content.split('=')[1].strip()))
                
                # 如果长度不同，选择较短的进行比较
                min_len = min(len(py_filtered), len(cpp_filtered))
                py_filtered = py_filtered[:min_len]
                cpp_filtered = cpp_filtered[:min_len]
                
                # 比较结果
                abs_diff = np.abs(py_filtered - cpp_filtered)
                max_diff = np.max(abs_diff)
                mean_diff = np.mean(abs_diff)
                
                match = max_diff < 1e-10
                
                print(f"Max absolute difference: {max_diff:.2e}")
                print(f"Mean absolute difference: {mean_diff:.2e}")
                print(f"Results match: {'✓' if match else '✗'}")
                
                max_diff_all = max(max_diff_all, max_diff)
                mean_diff_all += mean_diff / len(test_cases)
                results.append({"case": i, "match": match, "max_diff": max_diff, "mean_diff": mean_diff})
            except Exception as e:
                print(f"Error comparing results for test case {i}: {e}")
                traceback.print_exc()
        else:
            print(f"Test case {i}: Files not found for comparison")
    
    # 总体结果
    print("\n--- Overall Results ---")
    if results:
        match_count = sum(1 for r in results if r["match"])
        print(f"Tests passed: {match_count}/{len(results)}")
        print(f"Overall max difference: {max_diff_all:.2e}")
        print(f"Overall mean difference: {mean_diff_all:.2e}")
    else:
        print("No results to compare.")
    
    # 自动回答 'n' 不生成图表，这在批处理中有用
    print("\nDo you want to generate comparison plots? (y/n)")
    resp = 'n'  # 默认不生成图表，可以在运行时手动修改
    print(resp)  # 打印选择
    
    if resp == "y":
        for i in range(1, len(test_cases) + 1):
            py_file = f"py_filtfilt_{i}.txt"
            cpp_file = f"cpp_filtfilt_{i}.txt"
            
            if os.path.exists(py_file) and os.path.exists(cpp_file):
                # 加载Python和C++结果
                with open(py_file, 'r') as f:
                    py_filtered = np.array(eval(f.read().split('=')[1].strip()))
                
                with open(cpp_file, 'r') as f:
                    cpp_filtered = np.array(eval(f.read().split('=')[1].strip()))
                
                # 最小长度
                min_len = min(len(py_filtered), len(cpp_filtered))
                
                # 创建图
                plt.figure(figsize=(12, 8))
                
                # 绘制前200个点的对比
                plot_len = min(200, min_len)
                plt.subplot(2, 1, 1)
                plt.plot(py_filtered[:plot_len], 'b-', label='Python')
                plt.plot(cpp_filtered[:plot_len], 'r--', label='C++')
                plt.title(f"Test Case {i}: {test_cases[i-1]['description']}")
                plt.legend()
                plt.grid(True)
                
                # 绘制差异
                plt.subplot(2, 1, 2)
                plt.plot(np.abs(py_filtered[:min_len] - cpp_filtered[:min_len]), 'g-')
                plt.title('Absolute Difference')
                plt.yscale('log')
                plt.grid(True)
                
                plt.tight_layout()
                plt.savefig(f"compare_filtfilt_{i}.png")
                plt.close()
                
                print(f"Generated comparison plot for test case {i}")

if __name__ == "__main__":
    main() 