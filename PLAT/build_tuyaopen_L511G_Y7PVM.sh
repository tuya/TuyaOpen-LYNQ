#!/bin/bash
# TuyaOpen entry point for the L511G_Y7PVM module.
#
# The build itself -- every feature switch, the merge/package path, fcelf and
# mem_map -- is GccBuild_L511G_Y7PVM.sh, run unmodified. All this does is
# select the tuyaopen application project instead of app_demo, and let
# build_example.py's exported GCCLIB_PATH through. Both work because that script
# takes those two values as ${VAR:-<OEM default>}; nothing else in PLAT/ is
# touched, so a vendor SDK sync has two lines to re-apply. See
# ../README-build.md.
#
# build_example.py invokes this as: bash <this script> [clean]

cd "$(dirname "$0")" || exit 1

OEM_SCRIPT=./GccBuild_L511G_Y7PVM.sh

# A sync that restored the OEM's hard-coded assignments would silently build
# app_demo and let build_example.py publish it as this application's firmware.
for var in PROJECT_NAME GCCLIB_PATH
do
	if ! grep -q "export ${var}=\${${var}:-" "$OEM_SCRIPT"
	then
		echo "ERROR: $OEM_SCRIPT no longer honours \$${var}."
		echo "       A vendor SDK sync has overwritten it; re-apply the"
		echo "       \${${var}:-...} form. See README-build.md."
		exit 1
	fi
done

export PROJECT_NAME=tuyaopen

if [ "$1" = "clean" ]
then
	rm -rf ./gccout
	echo "clean all done ok..."
	exit 0
fi

if [ $# -ne 0 ]
then
	echo "unknown command $*"
	exit 1
fi

exec bash "$OEM_SCRIPT"
