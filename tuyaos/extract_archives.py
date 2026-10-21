#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
从 map 文件中提取所有用到的 .a 文件（静态库文件）
支持相对路径和绝对路径
"""

import re
import sys
import os
from pathlib import Path


def extract_archives_from_map(map_file_path, resolve_absolute=False):
    """
    从 map 文件中提取所有 .a 文件路径
    
    Args:
        map_file_path: map 文件的路径
        resolve_absolute: 是否解析为绝对路径
        
    Returns:
        去重后的 .a 文件路径列表
    """
    archives = set()
    
    # 正则表达式匹配 .a 文件路径
    # 格式: path/to/xxx.a(object.o)
    # 匹配以../或/开头，或者普通路径，但不匹配以点开头的section名
    pattern = re.compile(r'((?:\.\.\/|\/)[^\s]+\.a|(?<![./])[a-zA-Z_][^\s]*\.a)\(')
    
    try:
        with open(map_file_path, 'r', encoding='utf-8', errors='ignore') as f:
            for line in f:
                matches = pattern.findall(line)
                for archive_path in matches:
                    if resolve_absolute:
                        # 如果路径是相对路径，基于 map 文件所在目录解析
                        map_dir = os.path.dirname(os.path.abspath(map_file_path))
                        abs_path = os.path.normpath(os.path.join(map_dir, archive_path))
                        archives.add(abs_path)
                    else:
                        archives.add(archive_path)
    
    except FileNotFoundError:
        print(f"错误: 找不到文件 '{map_file_path}'", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"错误: 读取文件时出错: {e}", file=sys.stderr)
        sys.exit(1)
    
    return sorted(archives)


def main():
    if len(sys.argv) < 2:
        print("用法: python3 extract_archives.py <map文件路径> [--absolute]")
        print("示例: python3 extract_archives.py ap_app_demo.map")
        print("      python3 extract_archives.py ap_app_demo.map --absolute")
        sys.exit(1)
    
    map_file = sys.argv[1]
    resolve_absolute = '--absolute' in sys.argv or '-a' in sys.argv
    
    print(f"正在分析 map 文件: {map_file}")
    print(f"路径类型: {'绝对路径' if resolve_absolute else '相对路径'}")
    print("-" * 80)
    
    archives = extract_archives_from_map(map_file, resolve_absolute)
    
    if not archives:
        print("未找到任何 .a 文件")
        return
    
    print(f"\n找到 {len(archives)} 个不同的 .a 文件:\n")
    
    for i, archive in enumerate(archives, 1):
        print(f"{i:3d}. {archive}")
    
    # 可选：输出到文件
    output_file = map_file.rsplit('.', 1)[0] + '_archives.txt'
    with open(output_file, 'w', encoding='utf-8') as f:
        for archive in archives:
            f.write(f"{archive}\n")
    
    print(f"\n结果已保存到: {output_file}")


if __name__ == "__main__":
    main()
