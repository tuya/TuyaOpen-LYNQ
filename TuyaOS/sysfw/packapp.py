import os

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
FCELF = os.path.join(SCRIPT_DIR, "fcelf")
import json
import time
import shutil
import subprocess
import sys
import argparse


def packet_ec718_ap_app_pkg(apbinpkgpath, appbinpkgpath, outfilename, inr_lfs_file, ext_lfs_file, inr_lfs_addr, inr_lfs_size, ext_lfs_size):
    fsimg = None
    appaddr = None

    if not os.path.exists(apbinpkgpath):
        print("Notice", f"文件({apbinpkgpath})不存在")
        return   
    
    if os.path.exists(appbinpkgpath):
        with open(appbinpkgpath, 'rb') as file:
            data = file.read(4)
            appaddr = int.from_bytes(data, byteorder='little') - 64
            appsize = os.path.getsize(appbinpkgpath)
            # print("appaddr=%x appsize=%d" % (appaddr, appsize))
        
    wincmdunzip = f"{FCELF} -E -input {apbinpkgpath} -info imagedata_test.info -dir pkg_extract_tmp_test"
    subprocess.run(wincmdunzip, shell=True)
    
    if os.path.exists("pkg_extract_tmp_test") and os.path.exists("imagedata_test.info"):
        pass
    else:
        print("Notice", "解压失败")
        return

    with open("imagedata_test.info", 'r') as fp:
        imginfo = json.loads(fp.read())
        memmapfilecontent = f"\
            #define EF_IMG_MERGE_LNA 0x80000000\n\
            #define EF_IMG_MERGE_LIMIT {ext_lfs_size}\n\
            #define PKGFLXLFS_IMG_MERGE_LNA {inr_lfs_addr}\n\
            #define PKGFLXLFS_IMG_MERGE_LIMIT {inr_lfs_size}\n\
            #define PKGFLXCALI_PKGIMG_MERGE_LNA 0x9f3000\n\
            #define PKGFLXCALI_PKGIMG_MERGE_LIMIT 0xc000\n"

        packetinput = []
        for item in imginfo['imageinfo']:
            if item['addr'] > 0 and item['flashsize'] > 0:
                memmapfilecontent += f"#define {item['type']}_IMG_MERGE_LNA {'0x%x' % (item['addr'])}\n"
                memmapfilecontent += f"#define {item['type']}_IMG_MERGE_LIMIT {'0x%x' % (item['flashsize'])}\n"
                packetinput.append(f"-input ./pkg_extract_tmp_test/{item['file']} -addrname {item['type']}_IMG_MERGE_LNA -flashsize {item['type']}_IMG_MERGE_LIMIT")

        if os.path.exists(appbinpkgpath):        
            memmapfilecontent += f"#define PKGFLXAPP_APP0_LNA {'0x%x' % (appaddr)}\n"
            memmapfilecontent += f"#define PKGFLXAPP_APP0_LIMIT {'0x%x' % (appsize)}\n"
            packetinput.append(f"-input {appbinpkgpath} -addrname PKGFLXAPP_APP0_LNA -flashsize PKGFLXAPP_APP0_LIMIT")
               
        with open("memmap.h", "w") as fp:
            fp.write(memmapfilecontent)

        pkgcmd = ""
        if inr_lfs_file is not None:
            # -input ./feedback_fs_data.bin -addrname PKGFLXLFS_LNA -flashsize PKGFLXLFS_SIZE
            pkgcmd += f" -input {inr_lfs_file} -addrname PKGFLXLFS_IMG_MERGE_LNA -flashsize PKGFLXLFS_IMG_MERGE_LIMIT"
        if ext_lfs_file is not None:
            # -input ./ex_feedback_fs_data.bin -addrname EF_IMG_LFS_LNA -flashsize EF_IMG_LFS_SIZE
            pkgcmd += f" -input {ext_lfs_file} -addrname EF_IMG_MERGE_LNA -flashsize EF_IMG_MERGE_LIMIT"

        wincmd = f"{FCELF} -M {' '.join(packetinput)} {pkgcmd} -pkgmode {imginfo['pkgarg']['pkgmode']} -banoldtool 1 -productname {imginfo['pkgarg']['productname']} -def memmap.h -outfile {outfilename}.binpkg"
        # print("------------wincmd %s" % (wincmd))
        subprocess.run(wincmd, shell=True)
    shutil.rmtree("pkg_extract_tmp_test")
    os.remove("imagedata_test.info")
    # os.remove("memmap.h")
    if fsimg is not None:
        os.remove(fsimg)
    print(f"pack success, out file is: {os.getcwd()}{os.sep}{outfilename}.binpkg")
    
if __name__ == "__main__":
    # mk_fs_img("E:\\at-test-framework\\fibocom", 1024*1024)
    # exit()
    parser = argparse.ArgumentParser(description='Pack EC718 AP app binaries into binpkg format')
    parser.add_argument('ap_file', help='Path to the AP binary package file')
    parser.add_argument('app_file', help='Path to the APP binary file')
    parser.add_argument('output_name', help='Output file name (without .binpkg extension)')
    parser.add_argument('--inr-flash', default=None, help='Optional: Path to internal flash LFS file')
    parser.add_argument('--ext-flash', default=None, help='Optional: Path to external flash LFS file')
    parser.add_argument('--ext-size', default=0x1000000, type=lambda x: int(x, 0), help='External flash size (default: 0x1000000)')
    parser.add_argument('--inr-size', default=0x80000, type=int, help='Internal flash size (default: 0x80000)')
    parser.add_argument('--inr-addr', default=0xB47000, type=int, help='Internal flash size (default: 0xB47000)')

    args = parser.parse_args()
    
    # Check if flash files exist
    inr_flash_file = args.inr_flash if args.inr_flash and os.path.exists(args.inr_flash) else None
    ext_flash_file = args.ext_flash if args.ext_flash and os.path.exists(args.ext_flash) else None
    
    packet_ec718_ap_app_pkg(args.ap_file, args.app_file, args.output_name, inr_flash_file, ext_flash_file, args.inr_addr, args.inr_size, args.ext_size)
