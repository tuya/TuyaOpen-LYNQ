#!/bin/bash
###################################################################
# supported build command:
# 1 ./build_app.sh
# 2 ./build_app.sh clean
# 3 ./build_app.sh ${APP_NAME}
# 4 ./build_app.sh ${APP_NAME} ${APP_VERSION}
# 5 ./build_app.sh ${APP_NAME} clean
# 6 ./build_app.sh ${APP_PATH} ${APP_NAME} ${APP_VERSION}
# 7 ./build_app.sh ${APP_PATH} ${APP_NAME} ${APP_VERSION} clean
##################################################################

if [ $# -eq 3 -o $# -eq 4 ];then
    APP_ROOT_DIR=$1
    APP_NAME=$2
    APP_VERSION=$3
    USER_CMD=$4
elif [ $# -eq 0 ]; then
    USER_CMD=""
    APP_NAME=`bash ./scripts/get_sub_dir.sh apps`
    APP_VERSION=`cd apps/${APP_NAME} && bash ../../scripts/get_ver_tag.sh`
elif [ $# -eq 1 ]; then
    if [ "$1" = "clean" ]; then
        USER_CMD=$1
        APP_NAME=`bash ./scripts/get_sub_dir.sh apps`
        APP_VERSION=""
    else
        USER_CMD=""
        APP_NAME=$1
        APP_VERSION=`cd apps/${APP_NAME} && bash ../../scripts/get_ver_tag.sh`
    fi
else
    if [ "$2" = "clean" ]; then
        USER_CMD=$2
        APP_NAME=$1
        APP_VERSION=`cd apps/${APP_NAME} && bash ../../scripts/get_ver_tag.sh`
    else
        USER_CMD=""
        APP_NAME=$1
        APP_VERSION=$2
    fi
fi

if [ -z ${APP_VERSION} ]; then
    APP_VERSION=0.0.1
fi

APP_VERSION=`echo ${APP_VERSION} | cut -d'-' -f1`
echo APP_NAME=${APP_NAME}
echo APP_VERSION=${APP_VERSION}

cd `dirname $0`
CUR_DIR=$(pwd)

if [ -z ${APP_ROOT_DIR} ]; then
    APP_ROOT_DIR=apps/${APP_NAME}
fi

if [ ! -d ${APP_ROOT_DIR} ]; then
    echo "${APP_ROOT_DIR} not exist!!!"
    exit 0
fi

TOOLCHAIN_DIR=prebuilts/linux/gcc-arm-none-eabi-10-2020-q4-major
# 和L511A使用同一个工具链
TOOLCHAIN_REPO=https://airtake-public-data-1254153901.cos.ap-shanghai.myqcloud.com/smart/embed/pruduct/l511a_0.0.1.zip

download_toolchain()
{
    wget -O toolchain.tar ${TOOLCHAIN_REPO}
    echo "deploy toolchain, it will take some minutes, please wait..."
    tar zxf toolchain.tar
    mv l511a_toolchain/gcc-arm-none-eabi-10-2020-q4-major/ prebuilts/linux/
    rm -rf toolchain.tar l511a_toolchain
    if [ ! -d ${TOOLCHAIN_DIR} ]; then
        echo "ERROR: download toolchain from ${TOOLCHAIN_REPO} failed!!!"
        exit 1
    fi
    echo "deploy toolchain success!"
}

if [ ! -d ${TOOLCHAIN_DIR} ]; then
    echo "download toolchain before building..."
    download_toolchain
fi



# 判断apps目录下的应用是否存在CMakeLists,没有的话，从scripts中拷贝BuildApp_CMakeLists.txt
if [ ! -f ${APP_ROOT_DIR}/CMakeLists.txt ]; then
    if [ -f scripts/BuildApp_CMakeLists.txt ]; then
        cp scripts/BuildApp_CMakeLists.txt ${APP_ROOT_DIR}/CMakeLists.txt
        echo "CMakeLists.txt copied to ${APP_ROOT_DIR}"
    else
        echo "ERROR: scripts/BuildApp_CMakeLists.txt not found!"
        exit 1
    fi
fi

PACKAGE=l511c_package
PACKAGE_PATH=output/${PACKAGE}

# Read ext flash config from tuya_iot_config.h.
# NOTE: ext flash SIZE is no longer defined here; it comes from fs_info.json's
# ext_flash_len (bytes) when present, otherwise falls back to 16MB below.
_EXT_FLASH_SPI_INDEX=""
_ENABLE_EXT_FLASH=""
if [ -f "${CUR_DIR}/include/base/include/tuya_iot_config.h" ]; then
    _EXT_FLASH_SPI_INDEX=$(grep -s "^#define EXT_FLASH_SPI_INDEX" "${CUR_DIR}/include/base/include/tuya_iot_config.h" | awk '{print $3}')
    _ENABLE_EXT_FLASH=$(grep -s "^#define ENABLE_EXT_FLASH" "${CUR_DIR}/include/base/include/tuya_iot_config.h" | awk '{print $3}')
fi
echo "EXT flash SPI index: ${_EXT_FLASH_SPI_INDEX}"
echo "ENABLE_EXT_FLASH: ${_ENABLE_EXT_FLASH}"
EXT_FILE_SYSTEM_SIZE=$((16 * 1024 * 1024))  # default 16MB

# Parse fs_info.json when ENABLE_EXT_FLASH is enabled and the file exists.
# fs_src_path overrides the default "ext_file" source directory name.
# ext_flash_len (in bytes) overrides the default EXT_FILE_SYSTEM_SIZE.
FS_SRC_PATH=""
_FS_INFO_FILE="${CUR_DIR}/${APP_ROOT_DIR}/fs_info.json"
if [ "${_ENABLE_EXT_FLASH}" = "1" ] && [ -f "${_FS_INFO_FILE}" ]; then
    FS_SRC_PATH=$(python3 -c "import json; print(json.load(open('${_FS_INFO_FILE}')).get('fs_src_path','') or '')" 2>/dev/null)
    _EXT_FLASH_LEN=$(python3 -c "import json; v=json.load(open('${_FS_INFO_FILE}')).get('ext_flash_len',0); print(v if v else '')" 2>/dev/null)
    if [ -n "${_EXT_FLASH_LEN}" ]; then
        EXT_FILE_SYSTEM_SIZE=${_EXT_FLASH_LEN}
    fi
    echo "fs_info.json parsed: fs_src_path='${FS_SRC_PATH}', ext_flash_len='${_EXT_FLASH_LEN}'"
fi
echo "EXT_FILE_SYSTEM_SIZE: ${EXT_FILE_SYSTEM_SIZE}"

SCRIPTS_DIR=${CUR_DIR}/scripts
SYSFW_DIR=${CUR_DIR}/sysfw

# 把 fs_info.json 的 ext_flash_pwrctl / flash_control 字段写进指定 ini 的 [control] 段。
# 调用方必须传本次构建的副本，不要直接传 sysfw 下的共享文件：output/TuyaOS 里的
# sysfw 只生成一次（build.sh 的 make_tuyaos 看到目录已存在就直接返回），原地改写
# 会污染源文件，还会把上一个 app 的配置残留给下一个没配该字段的 app。
# 未配置这两个字段时脚本自身会跳过，不影响其它 app 的构建。
apply_ext_flash_pwrctl()
{
    # 只有会往 ext flash 写数据的构建才需要控制它的供电，与上面 fs_src_path 的判断保持一致
    if [ "${_ENABLE_EXT_FLASH}" != "1" ]; then
        echo "ENABLE_EXT_FLASH != 1, skip ext_flash power control"
        return 0
    fi
    [ -f "${_FS_INFO_FILE}" ] || return 0
    for ini in "$@"; do
        [ -f "${ini}" ] || continue
        python3 ${SCRIPTS_DIR}/patch_ext_flash_ini.py "${_FS_INFO_FILE}" "${ini}" || exit 1
    done
}

build_app()
{
    # 如果使用CMAKE+Ninja，则使能下面的代码
    source ./tools/core_launch.sh
    cd ${APP_ROOT_DIR}

    # 如果使用Ninja + CMake方式编译，请使用一下脚本，并且屏蔽make的编译命令。
    # 并且复制prebuilts/linux/CMakeLists_template.txt放到目标apps下，并且改名为CMakeLists.txt。
    mkdir -p build
    cd build
    cmake .. -G Ninja  -DAPP_NAME=${APP_NAME} -DAPP_VERSION=${APP_VERSION} -DTUYAOS_SDK_VER=38
    ninja
    cd ..

    # ---- ext_flash LFS image packing ----
    # Triggers only when ALL three conditions are met:
    #   1) ENABLE_EXT_FLASH == 1 in tuya_iot_config.h
    #   2) fs_info.json exists in APP_ROOT_DIR
    #   3) the fs_src_path directory from fs_info.json exists
    EXT_FLASH_MOUNT_PATH="/ext"

    if [ -n "${FS_SRC_PATH}" ] && [ -d "${FS_SRC_PATH}" ]; then
        echo "==== Generating ext_flash LFS image (src=${FS_SRC_PATH}, size=${EXT_FILE_SYSTEM_SIZE}) ===="
        _EXT_OUT_BIN=${CUR_DIR}/${APP_ROOT_DIR}/build/${APP_VERSION}/ext_tuya_fs_data.bin
        if ! python3 ${SCRIPTS_DIR}/generate_prepack.py ${CUR_DIR}/${APP_ROOT_DIR} ${EXT_FLASH_MOUNT_PATH} ${FS_SRC_PATH}; then
            echo "==== ERROR: generate_prepack.py failed, abort ===="
            exit 1
        fi
        if ! python3 ${SCRIPTS_DIR}/flash_file_pack.py ${CUR_DIR}/${APP_ROOT_DIR} \
                --ext-out ${_EXT_OUT_BIN} \
                --ext-size ${EXT_FILE_SYSTEM_SIZE}; then
            echo "==== ERROR: flash_file_pack.py failed; removing partial bin and abort ===="
            rm -f ${_EXT_OUT_BIN}
            exit 1
        fi
        echo "==== ext_flash LFS image done ===="
    else
        echo "==== Skip ext_flash LFS image: ENABLE_EXT_FLASH=${_ENABLE_EXT_FLASH}, fs_info.json or '${FS_SRC_PATH:-ext_file}' dir missing ===="
    fi

    # make clean && make APP_NAME=${APP_NAME} APP_VERSION=${APP_VERSION}
    if [ -f build/${APP_VERSION}/${APP_NAME}.binpkg ]; then
        rm -rf output && mkdir -p ${PACKAGE_PATH}

        # If ext_flash image exists, rebuild binpkg with fcelf directly
        if [ -f build/${APP_VERSION}/ext_tuya_fs_data.bin ]; then
            echo "==== Repackaging binpkg with ext_flash (fcelf -M) ===="

            # baseini 会被 fcelf 打进 binpkg。sysfw 下那份是跨 app 复用的共享文件，
            # 先拷到本次构建目录再改，既不改源文件，也不会把配置残留给下一个 app。
            cp ${SYSFW_DIR}/config_ec718pm_prd_uart.baseini.bin build/${APP_VERSION}/
            cp ${SYSFW_DIR}/config_ec718pm_prd_usb.baseini.bin build/${APP_VERSION}/
            apply_ext_flash_pwrctl \
                build/${APP_VERSION}/config_ec718pm_prd_uart.baseini.bin \
                build/${APP_VERSION}/config_ec718pm_prd_usb.baseini.bin

            # Create temp memmap with ext flash LFS definitions for fcelf -def
            cp ${SYSFW_DIR}/mem_map.txt build/${APP_VERSION}/mem_map_ext.txt
            echo "#define EF_IMG_EXLFS_LNA (0x80000000)" >> build/${APP_VERSION}/mem_map_ext.txt
            echo "#define EF_IMG_EXLFS_SIZE (${EXT_FILE_SYSTEM_SIZE})" >> build/${APP_VERSION}/mem_map_ext.txt

            if [ "${_EXT_FLASH_SPI_INDEX}" = "0" ]; then
                EXT_FLASH_ADDRNAME="EF_IMG_EXLFS_LNA"
            else
                EXT_FLASH_ADDRNAME="EFID01_IMG_EXLFS_LNA"
                echo "#define EFID01_IMG_EXLFS_LNA (0x80000000)" >> build/${APP_VERSION}/mem_map_ext.txt
            fi

            ${SYSFW_DIR}/fcelf -M \
                -input ${SYSFW_DIR}/ap_bootloader.bin -addrname BL_PKGIMG_LNA -flashsize BOOTLOADER_PKGIMG_LIMIT_SIZE \
                -input build/${APP_VERSION}/${APP_NAME}.bin -addrname AP_PKGIMG_LNA -flashsize AP_PKGIMG_LIMIT_SIZE \
                -input ${SYSFW_DIR}/cp-demo-flash.bin -addrname CP_PKGIMG_LNA -flashsize CP_PKGIMG_LIMIT_SIZE \
                -input ${SYSFW_DIR}/ec718pm_PrMgrCfg.json.bin -addrname XPKGDPRM_VIRTIMG_MERGE_LNA -flashsize XPKG_VIRTIMG_LOAD_SIZE \
                -input ${SYSFW_DIR}/format_ec718pm.json.bin -addrname XPKGDCMN_VIRTIMG_MERGE_LNA -flashsize XPKG_VIRTIMG_LOAD_SIZE \
                -input build/${APP_VERSION}/config_ec718pm_prd_uart.baseini.bin -addrname XPKGDCMN_VIRTIMG_MERGE_LNA -flashsize XPKG_VIRTIMG_LOAD_SIZE \
                -input build/${APP_VERSION}/config_ec718pm_prd_usb.baseini.bin -addrname XPKGDCMN_VIRTIMG_MERGE_LNA -flashsize XPKG_VIRTIMG_LOAD_SIZE \
                -input build/${APP_VERSION}/ext_tuya_fs_data.bin -addrname ${EXT_FLASH_ADDRNAME} -flashsize EF_IMG_EXLFS_SIZE \
                -pkgmode 1 -banoldtool 1 \
                -productname EC718PM_PRD \
                -def build/${APP_VERSION}/mem_map_ext.txt \
                -outfile build/${APP_VERSION}/${APP_NAME}_with_ext.binpkg

            if [ -f build/${APP_VERSION}/${APP_NAME}_with_ext.binpkg ]; then
                cp build/${APP_VERSION}/${APP_NAME}_with_ext.binpkg ${PACKAGE_PATH}/Custom.binpkg
                cp build/${APP_VERSION}/${APP_NAME}.binpkg ${PACKAGE_PATH}/Custom_Without_Extfs.binpkg
                echo "==== binpkg with ext_flash created ===="
            else
                echo "==== WARNING: fcelf repack failed, using original binpkg ===="
                cp build/${APP_VERSION}/${APP_NAME}.binpkg ${PACKAGE_PATH}/Custom.binpkg
            fi
        else
            cp build/${APP_VERSION}/${APP_NAME}.binpkg ${PACKAGE_PATH}/Custom.binpkg
        fi

        cp build/${APP_VERSION}/${APP_NAME}.bin ${PACKAGE_PATH}
        cp build/${APP_VERSION}/${APP_NAME}.elf ${PACKAGE_PATH}
        cp build/${APP_VERSION}/${APP_NAME}.map ${PACKAGE_PATH}
        [ -f build/${APP_VERSION}/ext_tuya_fs_data.bin ] && cp build/${APP_VERSION}/ext_tuya_fs_data.bin ${PACKAGE_PATH}
        cp ${SYSFW_DIR}/ap_bootloader.bin ${PACKAGE_PATH}
        cp ${SYSFW_DIR}/ap_bootloader.elf ${PACKAGE_PATH} 2>/dev/null
        cp ${SYSFW_DIR}/ap_bootloader.map ${PACKAGE_PATH} 2>/dev/null
        cp ${SYSFW_DIR}/comdb.txt ${PACKAGE_PATH} 2>/dev/null
        cp ${SYSFW_DIR}/config_*.ini ${PACKAGE_PATH} 2>/dev/null
        # 随包交付的 custom ini 也要带上 ext_flash 电源配置
        apply_ext_flash_pwrctl ${PACKAGE_PATH}/config_custom_uart.ini ${PACKAGE_PATH}/config_custom_usb.ini
        cp ${SYSFW_DIR}/format* ${PACKAGE_PATH} 2>/dev/null

        cd output
        mkdir -p ${APP_VERSION}
        cp ${PACKAGE}/Custom.binpkg ${APP_VERSION}/${APP_NAME}_QIO_${APP_VERSION}.binpkg
        cp ${PACKAGE}/Custom.binpkg ${APP_VERSION}/${APP_NAME}_UG_${APP_VERSION}.binpkg
        
        cd ${PACKAGE}
        pwd

        rm config_linux_uart.ini
        zip ${APP_NAME}_PROD_${APP_VERSION}.zip *
        mv ${APP_NAME}_PROD_${APP_VERSION}.zip ../${APP_VERSION}/
        cd -
    fi
}

clean_app()
{
    cd ${APP_ROOT_DIR}/build
    ninja clean
    [ -d output ] && rm -rf output
    echo "clean output succ"
}

if [ "${USER_CMD}" = "clean" ]; then
    clean_app
else
    build_app
fi

