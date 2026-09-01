#!/usr/bin/env python3
"""Derive the tuyaopen build scripts from the OEM GccBuild scripts.

The OEM GccBuild_L511G_Y7P*.sh are the configurations the vendor validated on
this module. Rather than maintaining a separate entry script that drifts from
them, generate ours from those files so the only differences are the ones
listed here:

  * PROJECT_NAME, so the vendor build links our tuyaopen application project
    instead of the OEM's app_demo,
  * GCCLIB_PATH taken from the environment (build_example.py exports the
    toolchain TuyaOpen downloaded) with a repo-relative fallback,
  * the execute bit restored on PLAT/tools, which a zip/Windows checkout drops,
  * a `clean` argument, which build_example.py needs for `tos.py clean`,
  * mkdir -p of the build-log directory, since `tee` opens it before make runs.

Everything else -- feature switches, the merge/package path, mem_map and the
fcelf invocations -- stays byte-for-byte the OEM's. Running this twice is a
no-op; if the vendor ships new GccBuild scripts, re-run it and commit the
result.
"""
import io
import os
import sys

PLAT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "PLAT")

PAIRS = [
    ("GccBuild_L511G_Y7PM.sh", "build_tuyaopen_L511G_Y7PM.sh"),
    ("GccBuild_L511G_Y7PVM.sh", "build_tuyaopen_L511G_Y7PVM.sh"),
]

HEADER = """#!/bin/bash
# Generated from {src} -- see platform/L511G/README-build.md.
# Only difference from the OEM script: PROJECT_NAME is the tuyaopen application
# project, GCCLIB_PATH comes from the environment, PLAT/tools execute bits are
# restored, and a `clean` argument is accepted. Feature switches are the OEM's.
"""

# No explanatory comment goes with this line: the header above already records
# the difference, and a comment here would stack up against whatever comment the
# OEM script carries, making regeneration non-idempotent.
GCCLIB_NEW = (
    'export GCCLIB_PATH=${GCCLIB_PATH:-$(realpath "../../tools/'
    'gcc-arm-none-eabi-10-2020-q4-major")}\n'
)

EXEC_BIT = """
# The OEM package ships as a zip, so fcelf / LogPrePass / ecsecure can arrive
# without their execute bit.
if [ -d "./tools" ]; then
	find ./tools -type f -exec sh -c 'file "$1" 2>/dev/null | grep -q "ELF.*executable" && chmod +x "$1"' _ {} \\;
fi
"""

CLEAN = """
clean()
{
	make -$JOBNUMBER clean-gccall TYPE=$CHIP_TYPE TARGET=$BOARD_NAME PROJECT=$PROJECT_NAME CORE=$CORE_NAME
	rm -rf ./gccout/*
	echo "clean all done ok..."
	exit 0
}

# build_example.py runs this script with `clean` for `tos.py clean`.
if [ "$1" = "clean" ]
then
	clean
elif [ $# -ge 1 ]
then
	echo "unknown command $@"
	exit 1
fi
"""


def derive(src_name, dst_name):
    src = os.path.join(PLAT, src_name)
    dst = os.path.join(PLAT, dst_name)
    text = io.open(src, encoding='utf-8', newline='').read().replace("\r\n", "\n")

    # 1) the application project
    old_proj = "export PROJECT_NAME=app_demo\n"
    if old_proj not in text:
        return f"{src_name}: PROJECT_NAME line not found"
    text = text.replace(old_proj, "export PROJECT_NAME=tuyaopen\n", 1)

    # 2) toolchain location: keep the first export, drop commented-out variants
    out = []
    replaced_gcclib = False
    for line in text.split("\n"):
        stripped = line.strip()
        if stripped.startswith("export GCCLIB_PATH="):
            if not replaced_gcclib:
                out.append(GCCLIB_NEW.rstrip("\n"))
                replaced_gcclib = True
            continue
        if stripped.startswith("# export GCCLIB_PATH="):
            continue
        out.append(line)
    if not replaced_gcclib:
        return f"{src_name}: GCCLIB_PATH line not found"
    text = "\n".join(out)

    # 3) execute bits, right after the gccout directory is created
    anchor = 'if [ ! -e "gccout" ]; then\nmkdir gccout\nfi\n'
    if anchor not in text:
        return f"{src_name}: gccout mkdir block not found"
    text = text.replace(
        anchor,
        anchor + "\n# `tee` opens the build log before make runs, so its "
        "directory has to exist.\nmkdir -p ./gccout/$OUTPUT_NAME/$CORE_NAME\n"
        + EXEC_BIT, 1)

    # 4) clean support, after the OEM's fail handler and before the build starts
    anchor = 'starttime=$(date "+%Y/%m/%d %H:%M:%S")\n'
    if anchor not in text:
        return f"{src_name}: starttime line not found"
    text = text.replace(anchor, CLEAN + "\n" + anchor, 1)

    # 5) provenance header, replacing the OEM shebang
    if not text.startswith("#!/bin/bash\n"):
        return f"{src_name}: unexpected shebang"
    text = HEADER.format(src=src_name) + text[len("#!/bin/bash\n"):]

    io.open(dst, 'w', encoding='utf-8', newline='\n').write(text)
    os.chmod(dst, 0o755)
    return None


rc = 0
for src_name, dst_name in PAIRS:
    err = derive(src_name, dst_name)
    if err:
        print("ERROR: " + err)
        rc = 1
    else:
        print(f"written PLAT/{dst_name}  (from {src_name})")
sys.exit(rc)
