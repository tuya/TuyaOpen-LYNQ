#!/bin/bash
# TuyaOpen entry point for the L511G_Y7PM module.
#
# The build itself -- every feature switch, the merge/package path, fcelf and
# mem_map -- is GccBuild_L511G_Y7PM.sh, run unmodified. All this does is
# select the tuyaopen application project instead of app_demo, through the
# ${PROJECT_NAME} override that script honours. The toolchain comes from
# ${GCCLIB_PATH}, which build_example.py exports and which that script
# otherwise defaults to platform/tools.
#
# build_example.py invokes this as: bash <this script> [clean]

cd "$(dirname "$0")" || exit 1

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

exec bash ./GccBuild_L511G_Y7PM.sh
