#!/usr/bin/env python3
# coding=utf-8
"""TuyaOpen build wrapper for the L511C_Y6E vendor build."""

import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys


PLATFORM_ROOT = Path(__file__).resolve().parent
PLAT_ROOT = PLATFORM_ROOT / "PLAT"
DOWNLOAD_DIR = PLAT_ROOT / "L511C_Y6E_download"
BINPKG_NAME = "L511C_Y6E.binpkg"
ELF_NAME = "ap_tuyaopen.elf"
TOOLCHAIN_PATH = PLATFORM_ROOT.parent / "tools" / "gcc-arm-none-eabi-10-2020-q4-major"


def load_build_params(build_param_dir):
    param_file = Path(build_param_dir) / "build_param.json"
    try:
        with param_file.open("r", encoding="utf-8") as param_fp:
            params = json.load(param_fp)
    except (OSError, json.JSONDecodeError) as exc:
        print(f"Error: cannot read build parameters [{param_file}]: {exc}")
        return None
    if not isinstance(params, dict):
        print(f"Error: build parameters must be a JSON object [{param_file}].")
        return None

    required = (
        "OPEN_LIBS_DIR",
        "PLATFORM_NEED_LIBS",
        "BIN_OUTPUT_DIR",
        "PLATFORM_CHIP",
        "PLATFORM_BOARD",
        "CONFIG_PROJECT_NAME",
        "CONFIG_PROJECT_VERSION",
    )
    missing = [key for key in required if not params.get(key)]
    if missing:
        print(f"Error: missing build parameters: {', '.join(missing)}")
        return None
    if params["PLATFORM_CHIP"] != "L511C_Y6E" or params["PLATFORM_BOARD"] != "L511C_Y6E":
        print("Error: this build wrapper supports only L511C_Y6E.")
        return None
    return params


def build_env(params, build_param_dir):
    env = os.environ.copy()
    env["GCCLIB_PATH"] = str(TOOLCHAIN_PATH)
    env["PATH"] = os.pathsep.join(
        [str(TOOLCHAIN_PATH / "bin"), env.get("PATH", "")]
    )
    env["TUYA_LIB_DIR"] = params["OPEN_LIBS_DIR"]
    env["TUYA_LIBS"] = params["PLATFORM_NEED_LIBS"]
    env["TUYA_APP_KCONFIG_DIR"] = str(
        (Path(build_param_dir) / ".." / "include").resolve()
    )
    return env


def run_vendor_build(args, env):
    command = ["bash", "./build.sh", *args]
    print(f"Run [{' '.join(command)}] in {PLAT_ROOT}")
    return subprocess.run(command, cwd=PLAT_ROOT, env=env, check=False).returncode


def bind_package_configs(package_dir, package_name):
    """Point this platform's USB/UART configs at the application package."""
    for name in ("config_custom_usb.ini", "config_custom_uart.ini"):
        path = Path(package_dir) / name
        if not path.is_file():
            continue
        original = path.read_bytes()
        content, count = re.subn(
            rb"(?m)^(arg_pkg_path_val[ \t]*=[ \t]*)[^\r\n]*",
            lambda match: match[1] + ("..\\" + package_name).encode("utf-8"),
            original,
        )
        if count != 1:
            raise ValueError(f"expected one arg_pkg_path_val in {path}")
        path.write_bytes(content)


def collect_output(params):
    binpkg = DOWNLOAD_DIR / BINPKG_NAME
    elf = DOWNLOAD_DIR / ELF_NAME
    for required in (binpkg, elf):
        if not required.is_file() or required.stat().st_size == 0:
            print(f"Error: vendor build did not produce {required}")
            return False

    output_dir = Path(params["BIN_OUTPUT_DIR"])
    if not output_dir.is_absolute():
        output_dir = PLAT_ROOT / output_dir
    output_dir.mkdir(parents=True, exist_ok=True)

    package_dir = output_dir / DOWNLOAD_DIR.name
    shutil.rmtree(package_dir, ignore_errors=True)
    shutil.copytree(DOWNLOAD_DIR, package_dir)

    app_name = params["CONFIG_PROJECT_NAME"]
    app_version = params["CONFIG_PROJECT_VERSION"]
    shutil.copy2(binpkg, output_dir / f"{app_name}_QIO_{app_version}.bin")
    shutil.copy2(binpkg, output_dir / f"{app_name}_QIO_{app_version}.binpkg")
    bind_package_configs(package_dir, f"{app_name}_QIO_{app_version}.binpkg")
    shutil.copy2(elf, output_dir / f"{app_name}_{app_version}.elf")
    (output_dir / f"{app_name}_{app_version}.zip").unlink(missing_ok=True)

    print(f"Output: {output_dir / f'{app_name}_QIO_{app_version}.bin'}")
    return True


def build(params, env):
    # Remove only these expected outputs so an interrupted vendor build cannot
    # make an old package or ELF look like a fresh successful result.
    for name in (BINPKG_NAME, ELF_NAME):
        (DOWNLOAD_DIR / name).unlink(missing_ok=True)

    result = run_vendor_build([], env)
    if result != 0:
        print(f"Error: vendor build failed with exit code {result}.")
        return False
    return collect_output(params)


def main():
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <build_param_dir> <build|clean>")
        return 2

    build_param_dir = Path(sys.argv[1]).resolve()
    command = sys.argv[2]
    if command not in ("build", "clean"):
        print(f"Error: unknown command [{command}].")
        return 2

    params = load_build_params(build_param_dir)
    if params is None:
        return 1
    env = build_env(params, build_param_dir)

    if command == "clean":
        result = run_vendor_build(["clean"], env)
        if result != 0:
            print(f"Error: vendor clean failed with exit code {result}.")
        return result

    return 0 if build(params, env) else 1


if __name__ == "__main__":
    sys.exit(main())
