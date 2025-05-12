# 绘图需要
import matplotlib.pyplot as plt
import numpy as np
# 数据解析需要
import re

def parse_snr_data(file_path):
    """
    从文本文件解析 SNR 数据
    格式示例：34.9783s: SNR = 32.3951 dB
    """
    # 定义正则表达式模式
    pattern = re.compile(
        r'^\s*([0-9]+\.?[0-9]+)s\s*:\s*SNR\s*=\s*([0-9]+\.?[0-9]+)\s*dB\s*$',
        re.IGNORECASE
    )
    
    distance_list = []
    snr_list = []

    try:
        with open(file_path, 'r') as file:        
            for line_num, line in enumerate(file, 1):
                line = line.strip()
                if not line:
                    continue  # 跳过空行
                
                # 尝试匹配数据格式
                match = pattern.match(line)
                if match:
                    # 提取时间和 SNR 值
                    time = float(match.group(1))
                    snr = float(match.group(2))
                    distance = 5 + 2 * time
                    distance_list.append(distance)
                    snr_list.append(snr)
                    
    except FileNotFoundError:
        print(f"错误: 文件 '{file_path}' 不存在")
    except Exception as e:
        print(f"发生异常: {str(e)}")
    
    return distance_list, snr_list

def parse_throughput_data(file_path):
    """
    从文本文件解析 Throughput 数据
    格式示例：3.01s: Throughput = 97.4802 Mbps
    """
    # 定义正则表达式模式
    pattern = re.compile(
        r'^\s*(\d+\.?\d*)s\s*:\s*Throughput\s*=\s*(\d+\.?\d*)\s*Mbps\s*$',
        re.IGNORECASE
    )
    
    throughput_list = []
    distance_list = []

    try:
        with open(file_path, 'r') as file:            
            for line_num, line in enumerate(file, 1):
                line = line.strip()
                if not line:
                    continue  # 跳过空行
                
                # 尝试匹配数据格式
                match = pattern.match(line)
                if match:
                    # 提取时间和 throughput 值，计算距离
                    time = float(match.group(1))
                    throughput = float(match.group(2))
                    distance = 5 + 2 * time
                    distance_list.append(distance)
                    throughput_list.append(throughput)
    
    except FileNotFoundError:
        print(f"错误: 文件 '{file_path}' 不存在")
    except Exception as e:
        print(f"发生异常: {str(e)}")
    
    return distance_list, throughput_list

def snr_plot_init():
    # 设置图像尺寸 (宽10英寸, 高6英寸)
    plt.figure(figsize=(10, 6))  

    # 图表装饰
    plt.title("Figure", fontsize=14, pad=20)  # 标题及字号
    plt.xlabel("distance", fontsize=12)    # X轴标签
    plt.ylabel("snr", fontsize=12)    # Y轴标签

    # 坐标轴范围
    plt.xlim(0, 80)
    plt.ylim(0, 60)

    # 网格线配置
    plt.grid(True, 
            linestyle=':', 
            alpha=0.6, 
            color='gray')
    
def throughput_plot_init():
    # 设置图像尺寸 (宽10英寸, 高6英寸)
    plt.figure(figsize=(10, 6))  

    # 图表装饰
    plt.title("Figure", fontsize=14, pad=20)  # 标题及字号
    plt.xlabel("distance", fontsize=12)    # X轴标签
    plt.ylabel("throughputs", fontsize=12)    # Y轴标签

    # 坐标轴范围
    plt.xlim(0, 80)
    plt.ylim(0, 120)

    # 网格线配置
    plt.grid(True, 
            linestyle=':', 
            alpha=0.6, 
            color='gray')

def plot(x_data, y_data, color='blue', label=None):
    # 绘制折线图+散点图（组合样式）
    line = plt.plot(x_data, y_data, 
                    marker='o',          # 数据点样式：圆形
                    markersize=0.01,        # 点大小
                    markerfacecolor='#ff7f0e',  # 点填充色
                    markeredgecolor='#2d3436',  # 点边框色
                    linestyle='-',       # 线型：实线
                    linewidth=2,         # 线宽
                    label=label
                    )
    
    plt.legend(loc='best')

if __name__ == "__main__":
    import sys
    if len(sys.argv) != 4:
        print("使用方法: python <脚本路径> <Friis文件路径> <TwoRayGround文件路径> <LogNormal文件路径>")
        sys.exit(1)
    
    input_file_1 = sys.argv[1]
    input_file_2 = sys.argv[2]
    input_file_3 = sys.argv[3]

    # 解析数据
    # Friis类型信道数据
    Friis_snr_distance, Friis_snrs = parse_snr_data(input_file_1)
    Friis_throughput_distance, Friis_throughputs = parse_throughput_data(input_file_1)
    # TwoRayGround 类型信道数据
    TwoRayGround_snr_distance, TwoRayGround_snrs = parse_snr_data(input_file_2)
    TwoRayGround_throughput_distance, TwoRayGround_throughputs = parse_throughput_data(input_file_2)
    # LogNormal 类型信道数据
    LogNormal_snr_distance, LogNormal_snrs = parse_snr_data(input_file_3)
    LogNormal_throughput_distance, LogNormal_throughputs = parse_throughput_data(input_file_3)

    # 绘制SNR图表
    snr_plot_init()
    plot(Friis_snr_distance, Friis_snrs, 'blue',  'Friis')
    plot(TwoRayGround_snr_distance, TwoRayGround_snrs, 'green',  'TwoRayGround')
    plot(LogNormal_snr_distance, LogNormal_snrs, 'red',  'LogNormal')
    # 绘制Throughput图表
    throughput_plot_init()
    plot(Friis_throughput_distance, Friis_throughputs, 'blue',  'Friis')
    plot(TwoRayGround_throughput_distance, TwoRayGround_throughputs, 'green',  'TwoRayGround')
    plot(LogNormal_throughput_distance, LogNormal_throughputs, 'red',  'LogNormal')

    # 显示图表
    plt.show()
