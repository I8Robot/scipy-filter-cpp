#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# visualize_butter_comparison.py
# 用于可视化比较Butterworth滤波器实现的脚本

import numpy as np
import matplotlib.pyplot as plt
from scipy import signal
import os
import sys
from matplotlib.backends.backend_pdf import PdfPages
import re
import matplotlib
from matplotlib import font_manager

# 设置中文字体支持
def setup_chinese_fonts():
    # 在Windows系统上尝试使用系统中文字体
    if sys.platform.startswith('win'):
        # 尝试常见的中文字体
        for font in ['SimHei', 'Microsoft YaHei', 'SimSun', 'KaiTi']:
            try:
                # 检查字体是否可用
                font_path = font_manager.findfont(font, fallback_to_default=False)
                if font_path:
                    plt.rcParams['font.family'] = ["DejaVu Sans", font, 'sans-serif'] # 符号字体优先，中文备用
                    print(f"使用中文字体: {font}")
                    return True
            except:
                continue
    
    # 如果找不到适合的中文字体，使用英文标签
    print("找不到合适的中文字体，将使用英文标签")
    return False

# 配置matplotlib字体
use_chinese = setup_chinese_fonts()
#use_chinese = False

# 根据中文字体可用性决定使用中文还是英文标签
def get_labels():
    if use_chinese:
        return {
            "title": "滤波器响应比较: Python vs C++",
            "magnitude_response": "幅度响应",
            "phase_response": "相位响应",
            "normalized_freq": "标准化频率 (×π rad/sample)",
            "magnitude_db": "幅度 (dB)",
            "phase_rad": "相位 (弧度)",
            "passband_zoom": "通带放大",
            "magnitude_zoom": "幅度放大",
            "max_mag_diff": "最大幅度差异",
            "mean_mag_diff": "平均幅度差异",
            "max_phase_diff": "最大相位差异",
            "mean_phase_diff": "平均相位差异",
            "b_coeff_match": "b系数匹配",
            "a_coeff_match": "a系数匹配",
            "max_diff": "最大差异",
            "test_case": "测试用例",
            "lowpass": "低通滤波器",
            "highpass": "高通滤波器",
            "bandpass": "带通滤波器",
            "bandstop": "带阻滤波器"
        }
    else:
        return {
            "title": "Filter Response Comparison: Python vs C++",
            "magnitude_response": "Magnitude Response",
            "phase_response": "Phase Response",
            "normalized_freq": "Normalized Frequency (×π rad/sample)",
            "magnitude_db": "Magnitude (dB)",
            "phase_rad": "Phase (radians)",
            "passband_zoom": "Passband Zoom",
            "magnitude_zoom": "Magnitude Zoom",
            "max_mag_diff": "Max Magnitude Diff",
            "mean_mag_diff": "Mean",
            "max_phase_diff": "Max Phase Diff",
            "mean_phase_diff": "Mean",
            "b_coeff_match": "b coefficients match",
            "a_coeff_match": "a coefficients match",
            "max_diff": "max diff",
            "test_case": "Test Case",
            "lowpass": "lowpass",
            "highpass": "highpass",
            "bandpass": "bandpass",
            "bandstop": "bandstop"
        }

# 获取标签
labels = get_labels()

# 转换测试用例描述为适当的语言
def translate_description(desc):
    if not use_chinese:
        return desc  # 如果不使用中文，保持原始描述
    
    # 英文描述转为中文
    desc = desc.replace("order lowpass", "阶低通滤波器")
    desc = desc.replace("order highpass", "阶高通滤波器") 
    desc = desc.replace("order bandpass", "阶带通滤波器")
    desc = desc.replace("order bandstop", "阶带阻滤波器")
    return desc

def read_coeffs_from_file(filename):
    """Read coefficients from a file"""
    with open(filename, 'r') as f:
        content = f.read()
    
    b_line = re.search(r'b = \[(.*?)\]', content, re.DOTALL)
    a_line = re.search(r'a = \[(.*?)\]', content, re.DOTALL)
    
    if b_line and a_line:
        b_str = b_line.group(1)
        a_str = a_line.group(1)
        
        # Convert to arrays
        b = np.array(eval(f"[{b_str}]"))
        a = np.array(eval(f"[{a_str}]"))
        
        return b, a
    else:
        print(f"Error parsing file: {filename}")
        return None, None

def plot_frequency_response(test_case, py_file, cpp_file, pdf):
    """Plot frequency response for both Python and C++ implementations"""
    py_b, py_a = read_coeffs_from_file(py_file)
    cpp_b, cpp_a = read_coeffs_from_file(cpp_file)
    
    if py_b is None or cpp_b is None:
        return
    
    # 处理描述
    description = translate_description(test_case['description'])
    
    # Create a new figure
    fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(10, 12))
    fig.suptitle(f"{labels['title']}: {description}", fontsize=14)
    
    # Compute frequency response
    w, py_h = signal.freqz(py_b, py_a)
    _, cpp_h = signal.freqz(cpp_b, cpp_a)
    
    # Convert to magnitude and phase
    py_mag = 20 * np.log10(np.maximum(np.abs(py_h), 1e-10))
    py_phase = np.unwrap(np.angle(py_h))
    
    cpp_mag = 20 * np.log10(np.maximum(np.abs(cpp_h), 1e-10))
    cpp_phase = np.unwrap(np.angle(cpp_h))
    
    # Magnitude response
    ax1.plot(w/np.pi, py_mag, 'b-', label='Python')
    ax1.plot(w/np.pi, cpp_mag, 'r--', label='C++')
    ax1.set_title(labels['magnitude_response'])
    ax1.set_xlabel(labels['normalized_freq'])
    ax1.set_ylabel(labels['magnitude_db'])
    ax1.set_xlim(0, 1.0)
    ax1.legend()
    ax1.grid(True)
    
    # Zoom in on pass/stop band
    if test_case["btype"] == "low":
        y_min = min(np.nanmin(py_mag[:int(len(w)*0.2)]), np.nanmin(cpp_mag[:int(len(w)*0.2)]))
        y_max = max(np.nanmax(py_mag[:int(len(w)*0.2)]), np.nanmax(cpp_mag[:int(len(w)*0.2)]))
        ax2.plot(w[:int(len(w)*0.2)]/np.pi, py_mag[:int(len(w)*0.2)], 'b-', label='Python')
        ax2.plot(w[:int(len(w)*0.2)]/np.pi, cpp_mag[:int(len(w)*0.2)], 'r--', label='C++')
        ax2.set_title(labels['passband_zoom'])
        ax2.set_xlim(0, 0.2)
    elif test_case["btype"] == "high":
        y_min = min(np.nanmin(py_mag[int(len(w)*0.8):]), np.nanmin(cpp_mag[int(len(w)*0.8):]))
        y_max = max(np.nanmax(py_mag[int(len(w)*0.8):]), np.nanmax(cpp_mag[int(len(w)*0.8):]))
        ax2.plot(w[int(len(w)*0.8):]/np.pi, py_mag[int(len(w)*0.8):], 'b-', label='Python')
        ax2.plot(w[int(len(w)*0.8):]/np.pi, cpp_mag[int(len(w)*0.8):], 'r--', label='C++')
        ax2.set_title(labels['passband_zoom'])
        ax2.set_xlim(0.8, 1.0)
    else:
        # For bandpass/bandstop, just show the whole thing with tight y-limits
        y_min = min(np.nanmin(py_mag), np.nanmin(cpp_mag))
        y_max = max(np.nanmax(py_mag), np.nanmax(cpp_mag))
        ax2.plot(w/np.pi, py_mag, 'b-', label='Python')
        ax2.plot(w/np.pi, cpp_mag, 'r--', label='C++')
        ax2.set_title(labels['magnitude_zoom'])
        ax2.set_xlim(0, 1.0)
    
    # 确保y_min和y_max是有效值
    if np.isnan(y_min) or np.isinf(y_min):
        y_min = -100
    if np.isnan(y_max) or np.isinf(y_max):
        y_max = 0
    
    # 确保y_min < y_max
    if y_min >= y_max:
        y_min = -100
        y_max = 0
    
    # Set y-limits with some padding
    padding = (y_max - y_min) * 0.1
    ax2.set_ylim(y_min - padding, y_max + padding)
    ax2.set_xlabel(labels['normalized_freq'])
    ax2.set_ylabel(labels['magnitude_db'])
    ax2.legend()
    ax2.grid(True)
    
    # Phase response
    ax3.plot(w/np.pi, py_phase, 'b-', label='Python')
    ax3.plot(w/np.pi, cpp_phase, 'r--', label='C++')
    ax3.set_title(labels['phase_response'])
    ax3.set_xlabel(labels['normalized_freq'])
    ax3.set_ylabel(labels['phase_rad'])
    ax3.set_xlim(0, 1.0)
    ax3.legend()
    ax3.grid(True)
    
    # Error statistics - 安全计算差异，避免NaN
    abs_diff_mag = np.abs(np.nan_to_num(py_mag - cpp_mag))
    abs_diff_phase = np.abs(np.nan_to_num(py_phase - cpp_phase))
    max_abs_diff_mag = np.nanmax(abs_diff_mag)
    max_abs_diff_phase = np.nanmax(abs_diff_phase)
    mean_abs_diff_mag = np.nanmean(abs_diff_mag)
    mean_abs_diff_phase = np.nanmean(abs_diff_phase)
    
    # 处理无效统计值
    if np.isnan(max_abs_diff_mag) or np.isinf(max_abs_diff_mag):
        max_abs_diff_mag = float('NaN')
    if np.isnan(max_abs_diff_phase) or np.isinf(max_abs_diff_phase):
        max_abs_diff_phase = float('NaN')
    if np.isnan(mean_abs_diff_mag) or np.isinf(mean_abs_diff_mag):
        mean_abs_diff_mag = float('NaN')
    if np.isnan(mean_abs_diff_phase) or np.isinf(mean_abs_diff_phase):
        mean_abs_diff_phase = float('NaN')
    
    fig.text(0.1, 0.01, 
             f"{labels['max_mag_diff']}: {max_abs_diff_mag:.6f} dB, {labels['mean_mag_diff']}: {mean_abs_diff_mag:.6f} dB\n"
             f"{labels['max_phase_diff']}: {max_abs_diff_phase:.6f} rad, {labels['mean_phase_diff']}: {mean_abs_diff_phase:.6f} rad",
             ha='left', fontsize=10)
    
    # Add coefficients comparison
    b_match = np.allclose(py_b, cpp_b, rtol=1e-10, atol=1e-10)
    a_match = np.allclose(py_a, cpp_a, rtol=1e-10, atol=1e-10)
    b_maxdiff = np.max(np.abs(py_b - cpp_b))
    a_maxdiff = np.max(np.abs(py_a - cpp_a))
    
    fig.text(0.6, 0.01,
             f"{labels['b_coeff_match']}: {'✓' if b_match else '✗'} ({labels['max_diff']}: {b_maxdiff:.6e})\n"
             f"{labels['a_coeff_match']}: {'✓' if a_match else '✗'} ({labels['max_diff']}: {a_maxdiff:.6e})",
             ha='left', fontsize=10)
    
    plt.tight_layout(rect=[0, 0.03, 1, 0.97])
    pdf.savefig(fig)
    plt.close(fig)

def main():
    # Check if results directory is specified as command line argument
    results_dir = "."
    if len(sys.argv) > 1:
        results_dir = sys.argv[1]
        print(f"使用结果目录: {results_dir}" if use_chinese else f"Using results directory: {results_dir}")
    
    # Test cases - same as the C++ and Python test cases
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
    
    # Create PDF output path
    pdf_path = os.path.join(results_dir, 'butter_comparison.pdf')
    
    # Create a PDF with all plots
    with PdfPages(pdf_path) as pdf:
        for i, tc in enumerate(test_cases):
            py_file = os.path.join(results_dir, f"py_butter_{i+1}.txt")
            cpp_file = os.path.join(results_dir, f"cpp_butter_{i+1}.txt")
            
            if os.path.exists(py_file) and os.path.exists(cpp_file):
                print(f"正在生成测试用例 {i+1} 的图表: {translate_description(tc['description'])}" if use_chinese else 
                      f"Generating plots for test case {i+1}: {tc['description']}")
                plot_frequency_response(tc, py_file, cpp_file, pdf)
            else:
                print(f"测试用例 {i+1}: 找不到比较文件" if use_chinese else 
                      f"Test case {i+1}: Files not found for comparison")
    
    print(f"\n图表已保存到 {pdf_path}" if use_chinese else f"\nPlots have been saved to {pdf_path}")

if __name__ == "__main__":
    main() 