#!/bin/bash
# 参数说明：
# $1 - params path: echo_app_top
# $2 - user cmd: build / clean / ...

print_not_null()
{
    if [ x"$1" = x"" ]; then
        return 1
    fi

    echo "$1"
}

set -e
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)

BUILD_PARAM_DIR=$(cd -- "$1" && pwd)
BUILD_PARAM_FILE=$BUILD_PARAM_DIR/build_param.config
. "$BUILD_PARAM_FILE"

EXAMPLE_NAME=$CONFIG_PROJECT_NAME
EXAMPLE_VER=$CONFIG_PROJECT_VERSION
HEADER_DIR=$OPEN_HEADER_DIR
LIBS_DIR=$OPEN_LIBS_DIR
LIBS=$PLATFORM_NEED_LIBS
OUTPUT_DIR=$BIN_OUTPUT_DIR
USER_CMD=$2
BOARD_NAME=$PLATFORM_BOARD

PLAT_DIR=$SCRIPT_DIR/PLAT
if [ -n "$OUTPUT_DIR" ] && [ "${OUTPUT_DIR#/}" = "$OUTPUT_DIR" ]; then
    OUTPUT_DIR=$PLAT_DIR/$OUTPUT_DIR
fi

APP_BIN_NAME=$EXAMPLE_NAME
APP_VERSION=$EXAMPLE_VER

if [ "$USER_CMD" = "build" ]; then
    USER_CMD=all
fi

echo "Start Compile"
cd "$PLAT_DIR"
if [ "$USER_CMD" = "clean" ];then
	./build.sh clean
	echo "*************************************************************************"
	echo "************************CLEAN SUCCESS************************************"
	echo "*************************************************************************"
	exit 0
fi

export TUYA_LIB_DIR=$LIBS_DIR
export TUYA_LIBS="$LIBS"
export TUYA_APP_KCONFIG_DIR=${BUILD_PARAM_DIR}/../include
./build.sh

BIN_DIR=$PLAT_DIR/L511C_Y6E_download
BINPKG=$BIN_DIR/L511C_Y6E.binpkg
ELF=$BIN_DIR/ap_tuyaopen.elf
if [ ! -f "$BINPKG" ]; then
    echo "Error: vendor build did not produce $BINPKG" >&2
    exit 1
fi
if [ ! -f "$ELF" ]; then
    echo "Error: vendor build did not produce $ELF" >&2
    exit 1
fi
if [ -z "$APP_BIN_NAME" ] || [ -z "$APP_VERSION" ] || [ -z "$OUTPUT_DIR" ]; then
    echo "Error: project name, version, or output directory is empty" >&2
    exit 1
fi

mkdir -p "$OUTPUT_DIR"
rm -rf -- "$OUTPUT_DIR/L511C_Y6E_download"
cp -a "$BIN_DIR" "$OUTPUT_DIR/"
# Remove a stale archive from the previous ZIP-based output flow.
rm -f -- "$OUTPUT_DIR/${APP_BIN_NAME}_${APP_VERSION}.zip"
cp -p "$BINPKG" "$OUTPUT_DIR/${APP_BIN_NAME}_QIO_${APP_VERSION}.bin"
cp -p "$BINPKG" "$OUTPUT_DIR/${APP_BIN_NAME}_QIO_${APP_VERSION}.binpkg"
cp -p "$ELF" "$OUTPUT_DIR/${APP_BIN_NAME}_${APP_VERSION}.elf"
