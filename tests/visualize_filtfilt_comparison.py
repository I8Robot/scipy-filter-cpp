#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import numpy as np
import matplotlib.pyplot as plt
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
            "title": "滤波器实现比较: Python vs C++",
            "original": "原始信号",
            "signal_comparison": "信号对比(前500个样本)",
            "sample": "样本",
            "amplitude": "幅度", 
            "zoom": "局部放大",
            "abs_diff": "绝对差异",
            "log_scale": "对数尺度",
            "max_diff": "最大差异",
            "mean_diff": "平均差异",
            "std_diff": "标准差",
            "match": "结果匹配",
            "summary": "总结",
            "tests_passed": "测试通过",
            "lowpass": "低通滤波器",
            "highpass": "高通滤波器",
            "bandpass": "带通滤波器",
            "bandstop": "带阻滤波器"
        }
    else:
        return {
            "title": "Filter Implementation Comparison: Python vs C++",
            "original": "Original Signal",
            "signal_comparison": "Signal Comparison (First 500 Samples)",
            "sample": "Sample",
            "amplitude": "Amplitude", 
            "zoom": "Zoomed View",
            "abs_diff": "Absolute Difference",
            "log_scale": "Log Scale",
            "max_diff": "Max Difference",
            "mean_diff": "Mean Difference",
            "std_diff": "Std Deviation",
            "match": "Results Match",
            "summary": "Summary",
            "tests_passed": "Tests Passed",
            "lowpass": "Lowpass Filter",
            "highpass": "Highpass Filter",
            "bandpass": "Bandpass Filter",
            "bandstop": "Bandstop Filter"
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

def read_signal_from_file(filename):
    """从文件读取信号数据"""
    with open(filename, 'r') as f:
        content = f.read()
    
    if 'signal' in content:
        # 读取原始信号
        signal_str = re.search(r'signal = \[(.*?)\]', content, re.DOTALL).group(1)
        signal = np.array(eval(f"[{signal_str}]"))
        return signal
    elif 'filtered' in content:
        # 读取滤波后的信号
        filtered_str = re.search(r'filtered = \[(.*?)\]', content, re.DOTALL).group(1)
        filtered = np.array(eval(f"[{filtered_str}]"))
        return filtered
    else:
        print(f"无法解析文件: {filename}")
        return None

def plot_filtfilt_comparison(test_case, signal_file, py_file, cpp_file, pdf):
    """为Python和C++实现的filtfilt结果绘制比较图"""
    # 读取原始信号和滤波后的信号
    original_signal = read_signal_from_file(signal_file)
    py_filtered = read_signal_from_file(py_file)
    cpp_filtered = read_signal_from_file(cpp_file)
    
    if original_signal is None or py_filtered is None or cpp_filtered is None:
        print(f"无法读取信号数据，跳过测试用例 {test_case['description']}")
        return
    
    # 创建新图表
    fig, axs = plt.subplots(3, 1, figsize=(10, 12))
    
    # 处理描述
    description = translate_description(test_case['description'])
    fig.suptitle(f"FiltFilt {labels['title']}: {description}", fontsize=14)
    
    # 首先绘制完整信号对比
    axs[0].plot(original_signal[:500], 'g-', alpha=0.5, label=labels["original"])
    axs[0].plot(py_filtered[:500], 'b-', label='Python')
    axs[0].plot(cpp_filtered[:500], 'r--', label='C++')
    axs[0].set_title(labels['signal_comparison'])
    axs[0].set_xlabel(labels['sample'])
    axs[0].set_ylabel(labels['amplitude'])
    axs[0].legend()
    axs[0].grid(True)
    
    # 然后进行局部放大
    zoom_start = 100
    zoom_len = 100
    axs[1].plot(original_signal[zoom_start:zoom_start+zoom_len], 'g-', alpha=0.5, label=labels["original"])
    axs[1].plot(py_filtered[zoom_start:zoom_start+zoom_len], 'b-', label='Python')
    axs[1].plot(cpp_filtered[zoom_start:zoom_start+zoom_len], 'r--', label='C++')
    axs[1].set_title(f'{labels["zoom"]} ({labels["sample"]} {zoom_start}-{zoom_start+zoom_len})')
    axs[1].set_xlabel(labels['sample'])
    axs[1].set_ylabel(labels['amplitude'])
    axs[1].legend()
    axs[1].grid(True)
    
    # 绘制差异
    diff = np.abs(py_filtered - cpp_filtered)
    axs[2].semilogy(diff[:500], 'g-')
    axs[2].set_title(f'Python vs C++ {labels["abs_diff"]} ({labels["log_scale"]})')
    axs[2].set_xlabel(labels['sample'])
    axs[2].set_ylabel(labels['abs_diff'])
    axs[2].grid(True)
    
    # 计算误差统计信息
    max_diff = np.max(diff)
    mean_diff = np.mean(diff)
    std_diff = np.std(diff)
    match = max_diff < 1e-10
    
    # 添加误差统计信息
    fig.text(0.1, 0.01,
             f"{labels['max_diff']}: {max_diff:.6e}\n"
             f"{labels['mean_diff']}: {mean_diff:.6e}\n"
             f"{labels['std_diff']}: {std_diff:.6e}",
             ha='left', fontsize=10)
    
    # 添加匹配结果
    fig.text(0.7, 0.01,
             f"{labels['match']}: {'✓' if match else '✗'}",
             ha='left', fontsize=12, 
             color='green' if match else 'red',
             weight='bold')
    
    plt.tight_layout(rect=[0, 0.03, 1, 0.97])
    pdf.savefig(fig)
    plt.close(fig)

def main():
    # 检查是否指定了结果目录
    results_dir = "."
    if len(sys.argv) > 1:
        results_dir = sys.argv[1]
        print(f"使用结果目录: {results_dir}")
    
    # 与C++和Python测试用例相同的测试用例
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
    
    # 创建PDF输出路径
    pdf_path = os.path.join(results_dir, 'filtfilt_comparison.pdf')
    
    # 获取信号文件路径
    signal_file = os.path.join(results_dir, 'cpp_test_signal.txt')
    if not os.path.exists(signal_file):
        print(f"错误: 找不到测试信号文件 {signal_file}")
        return
    
    # 创建包含所有图表的PDF
    with PdfPages(pdf_path) as pdf:
        # 添加测试结果摘要页
        plt.figure(figsize=(10, 6))
        plt.axis('off')
        
        plt.text(0.5, 0.95, f"FiltFilt {labels['title']}", fontsize=16, ha='center', weight='bold')
        
        results_summary = []
        
        # 循环处理所有测试用例
        for i, tc in enumerate(test_cases):
            py_file = os.path.join(results_dir, f"py_filtfilt_{i+1}.txt")
            cpp_file = os.path.join(results_dir, f"cpp_filtfilt_{i+1}.txt")
            
            if os.path.exists(py_file) and os.path.exists(cpp_file):
                # 读取并比较结果
                py_filtered = read_signal_from_file(py_file)
                cpp_filtered = read_signal_from_file(cpp_file)
                
                if py_filtered is not None and cpp_filtered is not None:
                    # 计算差异
                    diff = np.abs(py_filtered - cpp_filtered)
                    max_diff = np.max(diff)
                    mean_diff = np.mean(diff)
                    match = max_diff < 1e-10
                    
                    # 翻译描述
                    description = translate_description(tc['description'])
                    
                    results_summary.append({
                        "case": i+1,
                        "description": description,
                        "match": match,
                        "max_diff": max_diff,
                        "mean_diff": mean_diff
                    })
                    
                # 生成比较图
                print(f"生成测试用例 {i+1} 的对比图: {tc['description']}")
                plot_filtfilt_comparison(tc, signal_file, py_file, cpp_file, pdf)
            else:
                print(f"测试用例 {i+1}: 找不到比较文件")
        
        # 在摘要页中添加结果汇总
        summary_text = ""
        for i, result in enumerate(results_summary):
            status = "✓" if result["match"] else "✗"
            color = "green" if result["match"] else "red"
            plt.text(0.1, 0.85 - i*0.05, 
                    f"{status} Test {result['case']}: {result['description']}",
                    fontsize=10, color=color)
            plt.text(0.8, 0.85 - i*0.05, 
                    f"{labels['max_diff']}: {result['max_diff']:.2e}",
                    fontsize=10)
        
        # 添加总体结果
        matches = sum(1 for r in results_summary if r["match"])
        plt.text(0.1, 0.1, 
                f"{labels['summary']}: {matches}/{len(results_summary)} {labels['tests_passed']}",
                fontsize=14, weight='bold',
                color='green' if matches == len(results_summary) else 'red')
        
        pdf.savefig()
        plt.close()
        
    print(f"\n图表已保存到 {pdf_path}")

if __name__ == "__main__":
    main() 