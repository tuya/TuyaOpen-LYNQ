import os
import re

def remove_comments_from_c_code(code):
    # 去除单行注释
    code = re.sub(r'//.*', '', code)
    # 去除多行注释
    code = re.sub(r'/\*.*?\*/', '', code, flags=re.DOTALL)
    return code

def process_c_file(file_path):
    with open(file_path, 'r') as file:
        code = file.read()
    code_without_comments = remove_comments_from_c_code(code)
    with open(file_path, 'w') as file:
        file.write(code_without_comments)

def traverse_and_process_c_files(directory):
    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith('.c'):
                file_path = os.path.join(root, file)
                print(f'Processing {file_path}')
                process_c_file(file_path)

# 使用示例
current_directory = '.'
traverse_and_process_c_files(current_directory)
