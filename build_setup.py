#!/usr/bin/env python3
# coding=utf-8
'''
L511G build setup.

tos.py runs this after platform_prepare.py and before the cmake configure step,
with argv = [project_name, platform, framework, chip?]. Everything it does has
to be doable *before* cmake has produced anything, so this is the vendor-side
preparation only:

  - resolve which module variant is being built (Y7PVM / Y7PM),
  - write the product selection the vendor makefiles read
    (PLAT/middleware/developed/build_config.inc),
  - put that variant's comdb library list where the vendor tools look for it,
  - restore the execute bit on the OEM scripts and host tools, which a plain
    zip/Windows checkout drops.

Compiling the vendor libraries is *not* done here: the adapter link pulls in
TuyaOpen's own libtuyaos.a / libtuyaapp.a, which only exist after cmake+ninja,
so that step belongs to the build_example stage.
'''

import os
import shutil
import stat
import sys

PLATFORM_ROOT = os.path.dirname(os.path.abspath(__file__))
PLAT_ROOT = os.path.join(PLATFORM_ROOT, "PLAT")
TOOLCHAIN_NAME = "gcc-arm-none-eabi-10-2020-q4-major"
TOOLCHAIN_PATH = os.path.join(os.path.dirname(PLATFORM_ROOT), "tools",
                              TOOLCHAIN_NAME)

# The two module variants this platform ships, keyed by their normalised chip
# name. The values mirror the exports at the top of the matching vendor script,
# which is the only place they are otherwise written down. Those scripts are
# generated from the OEM GccBuild_L511G_Y7P*.sh so the feature switches stay
# the ones the vendor validated.
VARIANTS = {
    "l511gy7pvm": {
        "chip": "L511G_Y7PVM",
        "script": "build_tuyaopen_L511G_Y7PVM.sh",
        "build_pro_board": "EC718P_Y7PM",
        "build_pro_chip": "L511C_Y7",
        "build_pro_name": "L511G_Y7PM",
        "build_pro_target": "L511G_Y7PVM",
    },
    "l511gy7pm": {
        "chip": "L511G_Y7PM",
        "script": "build_tuyaopen_L511G_Y7PM.sh",
        "build_pro_board": "EC718P_Y7PM",
        "build_pro_chip": "L511C_Y7",
        "build_pro_name": "L511G_Y7PM",
        "build_pro_target": "L511G_Y7PM",
    },
}
DEFAULT_CHIP = "L511G_Y7PM"


def normalize_chip(chip):
    return chip.strip().lower().replace("-", "").replace("_", "")


def get_variant(chip):
    '''
    Accepts what the project's CONFIG_CHIP_CHOICE holds -- "L511G_Y7PVM",
    "l511g_y7pm", ... -- and falls back to the default module when a project
    does not select a chip at all.
    '''
    key = normalize_chip(chip)
    # cmake passes "NONE" through when a project selects no chip at all.
    if not key or key == "none":
        key = normalize_chip(DEFAULT_CHIP)
    variant = VARIANTS.get(key)
    if variant is None:
        print(f"Error: unknown chip [{chip}].")
        print(f"Supported: {', '.join(v['chip'] for v in VARIANTS.values())}")
        return {}
    return variant


def check_env(variant):
    ok = True
    cc = os.path.join(TOOLCHAIN_PATH, "bin", "arm-none-eabi-gcc")
    if not os.path.isfile(cc):
        print(f"Error: toolchain not found [{cc}].")
        print("Run [tos.py build] again, or python platform_prepare.py, "
              "to install it.")
        ok = False

    script = os.path.join(PLAT_ROOT, variant["script"])
    if not os.path.isfile(script):
        print(f"Error: vendor build script not found [{script}].")
        ok = False

    target_root = os.path.join(PLAT_ROOT, "openlib",
                               variant["build_pro_target"])
    if not os.path.isdir(target_root):
        print(f"Error: vendor libs not found [{target_root}].")
        ok = False

    return ok


def write_build_config(variant):
    '''
    The vendor makefiles pick the product up from this generated include; the
    OEM build scripts write it too, but the adapter build reads it earlier than
    that when only a single target is built.
    '''
    config_dir = os.path.join(PLAT_ROOT, "middleware", "developed")
    if not os.path.isdir(config_dir):
        print(f"Error: not found {config_dir}")
        return False

    config_file = os.path.join(config_dir, "build_config.inc")
    lines = [
        f"MBTK_PRODUCT_BOARD={variant['build_pro_board']}",
        f"MBTK_PRODUCT_CHIP={variant['build_pro_chip']}",
        f"MBTK_PRODUCT_TARGET={variant['build_pro_target']}",
    ]
    with open(config_file, 'w', encoding='utf-8', newline='\n') as f:
        f.write("\n".join(lines) + "\n")
    print(f"Generated {config_file}")
    return True


def copy_comdb_list(variant):
    '''
    ./tools/comdb_cust_lib.txt tells the log-database tools which of the
    variant's prebuilt libraries to scan; each variant ships its own copy.
    '''
    src = os.path.join(PLAT_ROOT, "openlib", variant["build_pro_target"],
                       "comdb_cust_lib.txt")
    dst_dir = os.path.join(PLAT_ROOT, "tools")
    if not os.path.isfile(src):
        print(f"Warning: not found {src}, skip.")
        return True
    if not os.path.isdir(dst_dir):
        print(f"Error: not found {dst_dir}")
        return False

    shutil.copy2(src, os.path.join(dst_dir, "comdb_cust_lib.txt"))
    print(f"Copied {src} -> {dst_dir}")
    return True


def add_exec_bit(path):
    mode = os.stat(path).st_mode
    os.chmod(path, mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)


def restore_exec_bits():
    '''
    The OEM package is distributed as a zip, so the host tools (fcelf,
    LogPrePass, ecsecure, ...) can arrive without their execute bit. Mark every
    ELF under PLAT/tools runnable.

    The .sh files are deliberately left alone: everything runs them as
    `bash <script>`, so the execute bit buys nothing and chmod-ing vendor files
    on every build only produces diffs to puzzle over at SDK sync time.
    '''
    tools_root = os.path.join(PLAT_ROOT, "tools")
    for root, _dirs, files in os.walk(tools_root):
        for name in files:
            path = os.path.join(root, name)
            try:
                with open(path, 'rb') as f:
                    if f.read(4) != b'\x7fELF':
                        continue
            except OSError:
                continue
            add_exec_bit(path)


def main():
    project_name = sys.argv[1] if len(sys.argv) > 1 else ""
    framework = sys.argv[3] if len(sys.argv) > 3 else ""
    chip = sys.argv[4] if len(sys.argv) > 4 else ""

    variant = get_variant(chip)
    if not variant:
        sys.exit(1)

    print(f"Build setup: project=[{project_name}] framework=[{framework}] "
          f"chip=[{variant['chip']}] script=[{variant['script']}]")

    if not check_env(variant):
        sys.exit(1)
    if not write_build_config(variant):
        sys.exit(1)
    if not copy_comdb_list(variant):
        sys.exit(1)
    restore_exec_bits()

    sys.exit(0)


if __name__ == "__main__":
    main()
