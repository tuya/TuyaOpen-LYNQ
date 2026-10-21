#!/usr/bin/env python3
"""
脚本用于生成 ext_prepack.json 和 inr_prepack.json 配置文件
用法: python generate_prepack.py <path>
"""

import os
import sys
import json
from pathlib import Path


def get_files_in_directory(directory):
    """获取目录下所有文件的列表（包括子目录中的文件）"""
    files = []
    if os.path.isdir(directory):
        try:
            for root, dirs, filenames in os.walk(directory):
                for filename in filenames:
                    # 跳过 lfsutil 自身在 cwd 写出的日志文件，避免污染打包
                    if filename == 'lfsutil.log':
                        continue
                    # 获取相对于目录的路径
                    full_path = os.path.join(root, filename)
                    rel_path = os.path.relpath(full_path, directory)
                    files.append(rel_path)
        except PermissionError:
            print(f"错误：无法访问目录 {directory}")
            return []
    return sorted(files)


def create_ext_prepack(path, ext_path="/ext", src_dir="ext_file"):
    """生成 ext_prepack.json 文件"""
    ext_dir = os.path.join(path, src_dir)

    if not os.path.isdir(ext_dir):
        print(f"{src_dir} not exit,ignore ext_prepack.json")
        return False

    files = get_files_in_directory(ext_dir)
    
    # 构建 ext_prepack.json 内容
    prepack_config = {
        "_comment": [
            "This is the configuration file to pre-pack files to pac.",
            "It contains a list of _files_, and for each file,",
            "_file_ is the absolute path in target, and _local_file_",
            "is the path in host. When _local_file_ is relative path,",
            "it is related to the directory of this configuration file.",
            "Also @SOURCE_TOP_DIR@ and @BINARY_TOP_DIR@ will be substitued",
            "to the absolte of current build.",
            "",
            "When it is empty, PREPACK won't be inserted into pac."
        ],
        "files": []
    }
    
    # 添加文件列表（使用绝对路径作为 local_file）
    for file in files:
        local_path = os.path.abspath(os.path.join(path, src_dir, file))
        prepack_config["files"].append({
            "file": f"{ext_path}/{file}",
            "local_file": local_path
        })
    
    # 写入文件
    output_path = os.path.join(path, 'ext_prepack.json')
    # 如果已存在则先删除
    try:
        if os.path.exists(output_path):
            os.remove(output_path)
    except OSError as e:
        print(f"警告：无法删除已存在的文件 {output_path}: {e}")

    try:
        with open(output_path, 'w', encoding='utf-8') as f:
            json.dump(prepack_config, f, indent=4, ensure_ascii=False)
        print(f"✓ 已生成 {output_path}")
        return True
    except IOError as e:
        print(f"错误：无法写入 {output_path}: {e}")
        return False


def create_inr_prepack(path):
    """生成 inr_prepack.json 文件"""
    inr_dir = os.path.join(path, 'inr_file')
    
    if not os.path.isdir(inr_dir):
        print(f"inr_file not exit,ignore inr_prepack.json")
        return False
    
    files = get_files_in_directory(inr_dir)
    
    # 构建 inr_prepack.json 内容
    prepack_config = {
        "_comment": [
            "This is the configuration file to pre-pack files to pac.",
            "It contains a list of _files_, and for each file,",
            "_file_ is the absolute path in target, and _local_file_",
            "is the path in host. When _local_file_ is relative path,",
            "it is related to the directory of this configuration file.",
            "Also @SOURCE_TOP_DIR@ and @BINARY_TOP_DIR@ will be substitued",
            "to the absolte of current build.",
            "",
            "When it is empty, PREPACK won't be inserted into pac."
        ],
        "files": []
    }
    
    # 添加文件列表（使用绝对路径作为 local_file）
    for file in files:
        local_path = os.path.abspath(os.path.join(path, 'inr_file', file))
        prepack_config["files"].append({
            "file": f"./{file}",
            "local_file": local_path
        })
    
    # 写入文件
    output_path = os.path.join(path, 'inr_prepack.json')
    # 如果已存在则先删除
    try:
        if os.path.exists(output_path):
            os.remove(output_path)
    except OSError as e:
        print(f"警告：无法删除已存在的文件 {output_path}: {e}")

    try:
        with open(output_path, 'w', encoding='utf-8') as f:
            json.dump(prepack_config, f, indent=4, ensure_ascii=False)
        print(f"✓ 已生成 {output_path}")
        return True
    except IOError as e:
        print(f"错误：无法写入 {output_path}: {e}")
        return False


def main():
    if len(sys.argv) < 2:
        print("用法: python generate_prepack.py <path> [ext_path] [src_dir]")
        print("例如: python generate_prepack.py /home/freedom/share/tuyaos_3.11_ai/vendor/le270/tools")
        sys.exit(1)

    ext_path = "/ext"
    target_path = sys.argv[1]
    if len(sys.argv) > 2:
        ext_path = sys.argv[2]

    src_dir = "ext_file"
    if len(sys.argv) > 3:
        src_dir = sys.argv[3]
    
    # 验证路径是否存在
    if not os.path.isdir(target_path):
        print(f"错误：路径不存在或不是目录: {target_path}")
        sys.exit(1)
    
    # 在生成前，先删除可能存在的旧 json 文件（无论源目录是否存在），避免残留旧配置
    for fname in ("ext_prepack.json", "inr_prepack.json"):
        old_path = os.path.join(target_path, fname)
        try:
            if os.path.exists(old_path):
                os.remove(old_path)
                # print(f"已删除旧文件: {old_path}")
        except OSError as e:
            print(f"警告：无法删除已存在的文件 {old_path}: {e}")
    
    # 检查并创建 ext_prepack.json
    ext_result = create_ext_prepack(target_path, ext_path, src_dir)

    # 检查并创建 inr_prepack.json
    inr_result = create_inr_prepack(target_path)
    
    # 总结
    # if ext_result or inr_result:
    #     print("\n✓ 完成！")

if __name__ == '__main__':
    main()
