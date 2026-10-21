import sys
import subprocess
import json
import os
import argparse

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
LFSUTIL = os.path.join(SCRIPT_DIR, "lfsutil")

def import_files_to_fs(pre_file_pkg, import_ext=True, fs_path=None):
    # fs_path can be provided to control the output lfs bin path
    if fs_path is None:
        fs_path = './ex_tuya_fs_data.bin' if import_ext else './tuya_fs_data.bin'
    with open(pre_file_pkg, 'r', encoding='utf-8') as json_file:
        data = json.load(json_file)

    def extract_levels(file_path):
        parts = file_path.split('/')
        levels = []
        for i in range(1, len(parts)):
            levels.append('/' + '/'.join(parts[1:i + 1]))
        return levels

    # lfsutil -w -d 不幂等：对已存在的目录返回 -17 (LFS_ERR_EXIST)。
    # 这里跟踪已创建过的目录路径,避免对每个文件都重复创建父目录。
    created_dirs = set()
    for file_info in data["files"]:
        file_path = file_info["file"]
        local_file_path = file_info["local_file"]
        print(local_file_path)
        levels = extract_levels(file_path)
        num_elements = len(levels)
        for idx, level in enumerate(levels, start=1):
            if idx == num_elements:
                subprocess.run([LFSUTIL, '-i', fs_path, '-w', '-f', levels[idx-1], '-F', local_file_path], check=True)
            elif level not in created_dirs:
                subprocess.run([LFSUTIL, '-i', fs_path, '-w', '-d', level], check=True)
                created_dirs.add(level)


def get_pkgfile_info(flash_file_cfg_path,
                     inr_size=0x80000, inr_out=None, 
                     ext_size=0x1000000, ext_out=None):

    pre_inr_file_pkg = os.path.join(flash_file_cfg_path, 'inr_prepack.json')
    if os.path.exists(pre_inr_file_pkg):
        # 先读取 json，若 files 为空则跳过生成 bin 文件
        try:
            with open(pre_inr_file_pkg, 'r', encoding='utf-8') as jf:
                inr_data = json.load(jf)
        except Exception as e:
            print(f"警告：无法读取或解析 {pre_inr_file_pkg}: {e}")
            inr_data = None

        if not inr_data or not inr_data.get('files'):
            print(f"跳过：{pre_inr_file_pkg} 中没有要打包的文件，未生成内部 bin")
        else:
            lf_size = inr_size
            out_path = inr_out if inr_out is not None else './tuya_fs_data.bin'
            # create/prepare the lfs file with given size
            result = subprocess.run([LFSUTIL, '-i', out_path, '-c', 'block_size=4096', '--read_size=256', '-S', str(lf_size)], check=True)
            import_files_to_fs(pre_inr_file_pkg, False, fs_path=out_path)

    pre_ext_file_pkg = os.path.join(flash_file_cfg_path, 'ext_prepack.json')
    if os.path.exists(pre_ext_file_pkg):
        # 先读取 json，若 files 为空则跳过生成 bin 文件
        try:
            with open(pre_ext_file_pkg, 'r', encoding='utf-8') as jf:
                ext_data = json.load(jf)
        except Exception as e:
            print(f"警告：无法读取或解析 {pre_ext_file_pkg}: {e}")
            ext_data = None

        if not ext_data or not ext_data.get('files'):
            print(f"跳过：{pre_ext_file_pkg} 中没有要打包的文件，未生成外部 bin")
        else:
            lf_size = ext_size
            out_path = ext_out if ext_out is not None else './ext_tuya_fs_data.bin'
            result = subprocess.run([LFSUTIL, '-i', out_path, '-c', 'block_size=4096', '--read_size=256', '-S', str(lf_size)], check=True)
            import_files_to_fs(pre_ext_file_pkg, True, fs_path=out_path)


def main():
    parser = argparse.ArgumentParser(description='Pack files into LFS images based on prepack jsons')
    parser.add_argument('cfg_path', help='Path to the flash file cfg directory containing inr_prepack.json and/or ext_prepack.json')

    # INR (internal) options
    parser.add_argument('--inr-size', type=lambda x: int(x, 0), default=0x80000,
                        help='Size of the internal LFS file (supports hex, e.g. 0x80000)')
    parser.add_argument('--inr-out', default=None,
                        help='Output path for internal LFS binary (default: ./tuya_fs_data.bin)')

    # EXT options
    parser.add_argument('--ext-size', type=lambda x: int(x, 0), default=0x1000000,
                        help='Size of the external LFS file (supports hex, e.g. 0x1000000)')
    parser.add_argument('--ext-out', default=None,
                        help='Output path for external LFS binary (default: ./ext_tuya_fs_data.bin)')

    args = parser.parse_args()

    get_pkgfile_info(args.cfg_path,
                     inr_size=args.inr_size, inr_out=args.inr_out, 
                     ext_size=args.ext_size, ext_out=args.ext_out)


if __name__ == '__main__':
    if os.path.exists("tuya_fs_data.bin"):
        os.remove("tuya_fs_data.bin")
    if os.path.exists("ext_tuya_fs_data.bin"):
        os.remove("ext_tuya_fs_data.bin")
    main()