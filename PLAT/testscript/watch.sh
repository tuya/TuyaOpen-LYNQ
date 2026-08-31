#!/bin/bash

set -e

LFS_TOOL="testscript/lfsutil"

DEST_DIR="project/${BOARD_NAME}/ap/apps/${PROJECT_NAME}/lfs/"

prepack_action() {
    rm -f lfsutil.log
    if [ -f "$DEST_DIR/ex_lfs.bin" ]; then
        rm -f "$DEST_DIR/ex_lfs.bin"
    fi
    cp testscript/ext_lfs_2M5.bin testscript/ex_lfs.bin
    for file in $(find "testscript/watch_res" -type f); do
        filename=$(basename "$file")
        "${LFS_TOOL}" -i ./testscript/ex_lfs.bin -w -f "${filename}" -F "$file"
        if [ $? -eq 0 ]; then
            echo "Successfully packed: $filename"
        else
            echo "Failed to pack: $filename"
        fi
    done
    for file in $(find "${DEST_DIR}" -type f); do
        filename=$(basename "$file")
        "${LFS_TOOL}" -i ./testscript/ex_lfs.bin -w -f "${filename}" -F "$file"
        # 检查命令是否成功执行
        if [ $? -eq 0 ]; then
            echo "Successfully packed: $filename"
        else
            echo "Failed to pack: $filename"
        fi
    done
    "${LFS_TOOL}" -i testscript/ex_lfs.bin -s
    "${LFS_TOOL}" -i testscript/ex_lfs.bin -l -d /

    mv testscript/ex_lfs.bin "${DEST_DIR}"
    rm -f lfsutil.log
}

# Check if EX_LFS_SIZE_DEC is greater than 0 and if ex_lfs.bin does not exist in DEST_DIR
if [ -f "testscript/ext_lfs_2M5.bin" ]; then
    prepack_action
else
    echo "Skipping prepack_action."
fi