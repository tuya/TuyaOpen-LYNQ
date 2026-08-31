#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import csv
import re
import sys
import os


def extract_warnings_to_csv(log_path, csv_path):
    """
    从编译日志中提取所有警告信息并保存到CSV文件
    """
    warnings = []
    
    try:
        with open(log_path, 'r', encoding='utf-8') as f:
            lines = f.readlines()
            
        # 查找所有包含warning的行并提取信息
        for i, line in enumerate(lines):
            if 'warning:' in line.lower():
                # 使用正则表达式匹配警告信息
                warning_pattern = r'([^\s]+):(\d+):\d+: warning: ([^\[]+)(\[-[^\]]+\])?'
                match = re.search(warning_pattern, line)
                
                if match:
                    file_path = match.group(1)
                    line_number = match.group(2)
                    warning_reason = match.group(3).strip()
                    suggestion = match.group(4).strip() if match.group(4) else ''
                    
                    # 对过长的警告原因进行总结，去除不必要的信息
                    if "implicit declaration of function" in warning_reason:
                        warning_reason = "implicit declaration of function"
                    elif "defined but not used" in warning_reason:
                        warning_reason = "defined but not used"
                    elif "unused variable" in warning_reason:
                        warning_reason = "unused variable"
                    elif "unused const variable" in warning_reason:
                        warning_reason = "unused const variable"
                    
                    # 移除文件路径中的../../../../../../../PLAT/部分
                    file_path = file_path.replace('../../../../../../../PLAT/', '')
                    
                    # 组合文件名和行号
                    file_with_line = f"{file_path}:{line_number}"
                    
                    warnings.append({
                        '类型': warning_reason,
                        '位置': file_with_line,
                        '备注': suggestion
                    })
                else:
                    # 如果不匹配标准格式，仍然记录信息
                    warnings.append({
                        '类型': line.strip(),
                        '位置': '',
                        '备注': ''
                    })
        
        # 去除完全重复的条目
        unique_warnings = []
        seen_warnings = set()
        for warning in warnings:
            # 创建一个用于比较的键，忽略具体的位置信息
            warning_key = (warning['类型'], warning['备注'])
            if warning_key not in seen_warnings:
                seen_warnings.add(warning_key)
                unique_warnings.append(warning)
        
        # 按类型对警告进行分组和排序
        unique_warnings.sort(key=lambda x: x['类型'])
        
        # 合并相同类型的相邻行
        merged_warnings = []
        for warning in unique_warnings:
            if merged_warnings and merged_warnings[-1]['类型'] == warning['类型']:
                # 如果与上一行类型相同，则清空类型字段
                merged_warnings.append({
                    '类型': '',
                    '位置': warning['位置'],
                    '备注': warning['备注']
                })
            else:
                # 如果与上一行类型不同，则正常显示
                merged_warnings.append(warning)
                
        # 只有在找到警告时才创建CSV文件
        if merged_warnings:
            # 写入CSV文件
            csv_file_path = csv_path
            with open(csv_file_path, 'w', newline='', encoding='utf-8') as csvfile:
                fieldnames = ['类型', '位置', '备注']
                writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
                writer.writeheader()
                for warning in merged_warnings:
                    writer.writerow(warning)
                
            print(f"警告信息已保存到: {csv_file_path}")
        else:
            print("未找到警告信息")
            
    except Exception as e:
        print(f"处理警告日志时出错: {str(e)}")


def main():
    if len(sys.argv) != 3:
        print("用法: python extract_warnings.py <编译日志文件> <输出CSV文件>")
        print("例如: python extract_warnings.py outbuildlog.txt warning.csv")
        sys.exit(1)
    
    log_path = sys.argv[1]
    csv_path = sys.argv[2]
    
    if not os.path.exists(log_path):
        print(f"错误: 编译日志文件 '{log_path}' 不存在")
        sys.exit(1)
    
    extract_warnings_to_csv(log_path, csv_path)


if __name__ == "__main__":
    main()