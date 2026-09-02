#!/usr/bin/env python3
# coding=utf-8
'''
L511G platform prepare.

tos.py runs this before the cmake configure step, with the chip name -- if the
project picked one -- as argv[1]. Its only job is to make the vendor toolchain
available: the arm-none-eabi GCC that both the TuyaOpen side (through
toolchain_file.cmake) and the vendor SDK side (PLAT/build_g_*.sh) compile with.

The toolchain is unpacked one level above the platform directory, in
platform/tools/gcc-arm-none-eabi-10-2020-q4-major -- the same place the L511C
platform keeps it, so both share a single unpacked copy.
'''

import os
import shutil
import stat
import sys
import tarfile
import tempfile
import zipfile

TOOLCHAIN_NAME = "gcc-arm-none-eabi-10-2020-q4-major"
TOOLCHAIN_URL = ("https://airtake-public-data-1254153901.cos.ap-shanghai"
                 ".myqcloud.com/smart/embed/pruduct/l511a_0.0.1.zip")
# The package is ~200MB and holds the toolchain in a subdirectory; it is served
# with a .zip name but is really a gzipped tar, so the archive type is sniffed
# from the magic bytes rather than taken from the URL.
CC_REL_PATH = os.path.join("bin", "arm-none-eabi-gcc")

PLATFORM_ROOT = os.path.dirname(os.path.abspath(__file__))
TOOLS_ROOT = os.path.join(os.path.dirname(PLATFORM_ROOT), "tools")
TOOLCHAIN_PATH = os.path.join(TOOLS_ROOT, TOOLCHAIN_NAME)

# Vendor scripts that git may have handed us without the execute bit (a
# Windows checkout, an unpacked release tarball).
NEED_EXEC = [
    os.path.join("PLAT", "build_g_pvm.sh"),
    os.path.join("PLAT", "build_g_pm.sh"),
    os.path.join("PLAT", "output_download.sh"),
]


def toolchain_ready(toolchain_path):
    cc = os.path.join(toolchain_path, CC_REL_PATH)
    return os.path.isfile(cc) or os.path.isfile(cc + ".exe")


def download(url, save_path):
    print(f"Downloading toolchain from {url}")
    try:
        import requests
    except ImportError:
        requests = None

    if requests is not None:
        response = requests.get(url, stream=True, timeout=30)
        response.raise_for_status()
        total = int(response.headers.get("Content-Length", 0))
        done = 0
        step = 0
        with open(save_path, 'wb') as f:
            for chunk in response.iter_content(chunk_size=1 << 20):
                if not chunk:
                    continue
                f.write(chunk)
                done += len(chunk)
                step = report_progress(done, total, step)
    else:
        from urllib.request import urlopen
        with urlopen(url, timeout=30) as response:
            total = int(response.headers.get("Content-Length", 0))
            done = 0
            step = 0
            with open(save_path, 'wb') as f:
                while True:
                    chunk = response.read(1 << 20)
                    if not chunk:
                        break
                    f.write(chunk)
                    done += len(chunk)
                    step = report_progress(done, total, step)
    print(f"Downloaded {os.path.getsize(save_path)} bytes.")


def report_progress(done, total, last_step):
    '''Print at most one line per 10%, so build logs stay readable.'''
    if not total:
        return last_step
    step = int(done * 10 / total)
    if step != last_step:
        print(f"  {done * 100 // total}% ({done}/{total})")
    return step


def extract(archive_path, dst_root):
    with open(archive_path, 'rb') as f:
        magic = f.read(4)

    if magic[:2] == b'PK':
        print("Extracting zip archive ...")
        with zipfile.ZipFile(archive_path) as zf:
            zf.extractall(dst_root)
    elif magic[:2] == b'\x1f\x8b':
        print("Extracting tar.gz archive ...")
        with tarfile.open(archive_path, 'r:gz') as tf:
            tf.extractall(dst_root)
    else:
        raise RuntimeError(f"Unknown archive format: {magic!r}")


def find_toolchain_dir(search_root):
    for root, dirs, _files in os.walk(search_root):
        if TOOLCHAIN_NAME in dirs:
            return os.path.join(root, TOOLCHAIN_NAME)
    return ""


def restore_exec_bits(toolchain_path):
    '''
    A zip archive carries no unix permissions, so every compiler driver comes
    out non-executable. Mark everything under bin/ and libexec/ runnable.
    '''
    for sub in ("bin", "libexec", os.path.join("arm-none-eabi", "bin")):
        sub_root = os.path.join(toolchain_path, sub)
        if not os.path.isdir(sub_root):
            continue
        for root, _dirs, files in os.walk(sub_root):
            for name in files:
                path = os.path.join(root, name)
                mode = os.stat(path).st_mode
                os.chmod(path, mode | stat.S_IXUSR | stat.S_IXGRP |
                         stat.S_IXOTH)


def install_toolchain():
    if toolchain_ready(TOOLCHAIN_PATH):
        print(f"Toolchain already exists: {TOOLCHAIN_PATH}")
        return True

    os.makedirs(TOOLS_ROOT, exist_ok=True)
    tmp_root = tempfile.mkdtemp(prefix="l511g_toolchain_", dir=TOOLS_ROOT)
    archive_path = os.path.join(tmp_root, "toolchain.pkg")
    try:
        download(TOOLCHAIN_URL, archive_path)
        extract(archive_path, tmp_root)
        os.remove(archive_path)

        extracted = find_toolchain_dir(tmp_root)
        if not extracted:
            raise RuntimeError(f"[{TOOLCHAIN_NAME}] not found in the archive.")

        # A previous run may have left a partial tree behind; replace it.
        if os.path.exists(TOOLCHAIN_PATH):
            shutil.rmtree(TOOLCHAIN_PATH)
        shutil.move(extracted, TOOLCHAIN_PATH)
        restore_exec_bits(TOOLCHAIN_PATH)
    except Exception as e:
        print(f"Error: install toolchain failed: {e}")
        return False
    finally:
        shutil.rmtree(tmp_root, ignore_errors=True)

    if not toolchain_ready(TOOLCHAIN_PATH):
        print(f"Error: {CC_REL_PATH} missing under {TOOLCHAIN_PATH}.")
        return False

    print(f"Toolchain installed: {TOOLCHAIN_PATH}")
    return True


def ensure_exec_bits():
    for rel in NEED_EXEC:
        path = os.path.join(PLATFORM_ROOT, rel)
        if not os.path.isfile(path):
            continue
        mode = os.stat(path).st_mode
        os.chmod(path, mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)


def main():
    chip = sys.argv[1] if len(sys.argv) > 1 else ""
    print(f"Preparing platform L511G [{chip or 'default'}] ...")

    if not install_toolchain():
        sys.exit(1)
    ensure_exec_bits()

    sys.exit(0)


if __name__ == "__main__":
    main()
