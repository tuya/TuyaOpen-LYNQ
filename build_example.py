#!/usr/bin/env python3
# coding=utf-8
'''
L511G example build.

Invoked by the top-level cmake target `example` as

    build_example.py <build_param_dir> <build|clean>

with the platform directory as the working directory. By this point TuyaOpen
has already produced libtuyaos.a / libtuyaapp.a with the arm-none-eabi
toolchain, so all that is left is the vendor side: compile the tuyaos_adapter
sources, link them against those libs and package bootloader + AP + CP images
into the flashable .binpkg.

The vendor makefiles pick TuyaOpen up through three environment variables --
TUYA_LIB_DIR, TUYA_LIBS and TUYA_APP_KCONFIG_DIR -- exported below.
'''

import json
import os
import shutil
import subprocess
import sys

from build_setup import get_variant, TOOLCHAIN_PATH

PLATFORM_ROOT = os.path.dirname(os.path.abspath(__file__))
PLAT_ROOT = os.path.join(PLATFORM_ROOT, "PLAT")


def parse_param(build_param_dir):
    param_file = os.path.join(build_param_dir, "build_param.json")
    if not os.path.isfile(param_file):
        print(f"Error: not found [{param_file}].")
        return {}
    try:
        with open(param_file, 'r', encoding='utf-8') as f:
            return json.load(f)
    except Exception as e:
        print(f"Error: parse [{param_file}] failed: {e}")
        return {}


def build_env(param, build_param_dir):
    env = os.environ.copy()
    env["GCCLIB_PATH"] = TOOLCHAIN_PATH
    env["PATH"] = os.pathsep.join(
        [os.path.join(TOOLCHAIN_PATH, "bin"), env.get("PATH", "")])

    # TuyaOpen's own archives and the generated tuya_kconfig.h; the vendor
    # adapter makefile links the former and includes the latter.
    env["TUYA_LIB_DIR"] = param.get("OPEN_LIBS_DIR", "")
    env["TUYA_LIBS"] = param.get("PLATFORM_NEED_LIBS", "")
    env["TUYA_APP_KCONFIG_DIR"] = os.path.normpath(
        os.path.join(build_param_dir, "..", "include"))
    return env


def run_vendor_script(variant, args, env):
    script = variant["script"]
    cmd = ["bash", script] + args
    print(f"Run [{' '.join(cmd)}] in {PLAT_ROOT}")
    return subprocess.run(cmd, cwd=PLAT_ROOT, env=env).returncode


def download_dir(variant):
    return os.path.join(PLAT_ROOT, f"{variant['build_pro_target']}_download")


def clean(variant, env):
    run_vendor_script(variant, ["clean"], env)
    for path in (download_dir(variant), os.path.join(PLAT_ROOT, "gccout")):
        shutil.rmtree(path, ignore_errors=True)
        print(f"Removed {path}")
    return True


def collect_output(variant, param):
    '''
    Copy the vendor download package next to the other TuyaOpen artifacts, and
    publish the image under the name tos.py looks for
    (<app>_QIO_<version>.bin).
    '''
    src_dir = download_dir(variant)
    binpkg = os.path.join(src_dir, f"{variant['build_pro_name']}.binpkg")
    if not os.path.isfile(binpkg):
        print(f"Error: not found {binpkg}")
        return False

    app_name = param.get("CONFIG_PROJECT_NAME", "")
    app_ver = param.get("CONFIG_PROJECT_VERSION", "")
    out_dir = param.get("BIN_OUTPUT_DIR", "")
    if not app_name or not out_dir:
        print("Error: build param missing project name or output dir.")
        return False

    os.makedirs(out_dir, exist_ok=True)
    pkg_dir = os.path.join(out_dir, os.path.basename(src_dir))
    shutil.rmtree(pkg_dir, ignore_errors=True)
    shutil.copytree(src_dir, pkg_dir)

    qio = os.path.join(out_dir, f"{app_name}_QIO_{app_ver}.bin")
    shutil.copy2(binpkg, qio)
    shutil.copy2(binpkg, os.path.join(
        out_dir, f"{app_name}_QIO_{app_ver}.binpkg"))

    # Keep the elf around under the app's name: tos.py's crash decoding and
    # addr2line both want it.
    elf = os.path.join(src_dir, "ap_tuyaos_adapter.elf")
    if os.path.isfile(elf):
        shutil.copy2(elf, os.path.join(
            out_dir, f"{app_name}_{app_ver}.elf"))

    print(f"Output: {qio}")
    return True


def build(variant, param, env):
    binpkg = os.path.join(download_dir(variant),
                          f"{variant['build_pro_name']}.binpkg")
    if os.path.isfile(binpkg):
        os.remove(binpkg)

    # The OEM scripts end their success path in completeHandle(), which exits
    # 1, so the return code cannot tell success from failure here -- the
    # packaged image is the only reliable signal.
    rc = run_vendor_script(variant, [], env)
    if not os.path.isfile(binpkg):
        print(f"Error: vendor build failed (exit {rc}), no {binpkg}")
        return False
    return collect_output(variant, param)


def main():
    if len(sys.argv) < 3:
        print(f"Error: usage: {sys.argv[0]} <build_param_dir> <build|clean>")
        sys.exit(1)
    build_param_dir = os.path.abspath(sys.argv[1])
    user_cmd = sys.argv[2]

    param = parse_param(build_param_dir)
    if not param:
        sys.exit(1)

    variant = get_variant(param.get("PLATFORM_CHIP", ""))
    if not variant:
        sys.exit(1)

    env = build_env(param, build_param_dir)
    if user_cmd == "clean":
        sys.exit(0 if clean(variant, env) else 1)
    if user_cmd != "build":
        print(f"Error: unknown command [{user_cmd}].")
        sys.exit(1)

    sys.exit(0 if build(variant, param, env) else 1)


if __name__ == "__main__":
    main()
