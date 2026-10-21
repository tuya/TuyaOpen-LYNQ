#!/bin/bash
#=================================================
# command list:
#    build.sh            -- make opencpu sdk
#    build.sh clean     -- clean opencpu sdk
#=================================================

CMD=""

echo "SYS_FW_NAME: ${SYS_FW_NAME}"
echo "SYS_FW_VER: ${SYS_FW_VER}"
echo "BUILD_TYPE: ${BUILD_TYPE}"
echo "HW_MOD: ${HW_MOD}"

VENDOR_SDK_ROOT=${PWD}/..
echo "vendor_sdk_root: ${VENDOR_SDK_ROOT}"
export VENDOR_SDK_ROOT=${VENDOR_SDK_ROOT}

FW_VER=`echo ${SYS_FW_VER} | cut -d'-' -f1`
echo "FW_VER: ${FW_VER}"

build_exit()
{
    echo "platform unsupported command: $@, quit!!!"
    exit 1
}

if [ $# -eq 1 ]; then
    CMD=$1
fi

if [ $# -gt 2 ]; then
    build_exit $@
fi

CPU_COUNT=$(cat /proc/cpuinfo | grep processor | wc -l)
if [ ${CPU_COUNT} -gt 8 ]; then
    CPU_COUNT=8
fi

clean_sdk()
{
    echo "clean_sdk"
    [ -d ${VENDOR_OUTPUT_PATH} ] || return 1
    cd ${VENDOR_OUTPUT_PATH}
    [ -f ${PACKAGE_NAME} ] && rm -rf ${PACKAGE_NAME}
    cd - > /dev/null
}

make_sdk()
{
    echo "make_sdk"
    SDK_ROOT=${VENDOR_SDK_ROOT}/../../output/TuyaOS
    export SDK_ROOT=${SDK_ROOT}
    echo SDK_ROOT=${SDK_ROOT}
    if [ ! -d "${SDK_ROOT}" ]; then
        echo "Error: ${SDK_ROOT} not exist"
        exit 1
    fi

    rm -rf ${SDK_ROOT}/scripts ${SDK_ROOT}/docs 
    rm -f ${SDK_ROOT}/build_app.sh ${SDK_ROOT}/prepare_app.sh
    [ -f ${SDK_ROOT}/Makefile ] && rm -f ${SDK_ROOT}/Makefile

    # copy cellualr build_app.sh scripts prebuilts
    cd ../TuyaOS
    cp -rf * ${SDK_ROOT}
    rm -rf ${SDK_ROOT}/docs/OpenCPU_User_Develop_Environment_Setup ${SDK_ROOT}/docs/OpenCPU_User_Programming_Manual
    cp docs/OpenCPU_User_Programming_Manual/OpenCPU_User_Programming_Manual.pdf ${SDK_ROOT}/docs
    cp docs/OpenCPU_User_Develop_Environment_Setup/OpenCPU_User_Develop_Environment_Setup_Manual.pdf ${SDK_ROOT}/docs
    cd -

    # Restore app Kconfig tooling (menuconfig / app_config / app_config_choice) from the IoT SDK tree.
    # The overlay above replaces scripts/ with a minimal set; merge in kconfiglib + scripts/mk/config.mk.
    SDK_IOT_ROOT="${VENDOR_SDK_ROOT}/../.."
    if [ -d "${SDK_IOT_ROOT}/scripts/kconfiglib" ] && [ -f "${SDK_IOT_ROOT}/scripts/mk/config.mk" ]; then
        mkdir -p "${SDK_ROOT}/scripts/mk"
        cp -f "${SDK_IOT_ROOT}/scripts/mk/config.mk" "${SDK_ROOT}/scripts/mk/"
        rm -rf "${SDK_ROOT}/scripts/kconfiglib"
        cp -a "${SDK_IOT_ROOT}/scripts/kconfiglib" "${SDK_ROOT}/scripts/"
        if [ -f "${SDK_IOT_ROOT}/scripts/get_app_config.sh" ]; then
            cp -f "${SDK_IOT_ROOT}/scripts/get_app_config.sh" "${SDK_ROOT}/scripts/"
            chmod a+x "${SDK_ROOT}/scripts/get_app_config.sh"
        fi
        echo "App Kconfig tools copied to ${SDK_ROOT}/scripts"
    else
        echo "Warning: ${SDK_IOT_ROOT}/scripts/kconfiglib or scripts/mk/config.mk missing, app Kconfig tools not merged"
    fi

    if [ ${HW_MOD} = "l511c_y7pm" ] ; then
        ./l511c_y7pm_platform_copy.sh
    elif [ ${HW_MOD} = "l511c_y7pvm" ] ; then
        ./l511c_y7pvm_platform_copy.sh
    elif [ ${HW_MOD} = "l511en_y7bv" ] ; then
        ./l511en_y7bv_platform_copy.sh
    elif [ ${HW_MOD} = "l511en_y7pvm" ] || [ ${HW_MOD} = "l511e_y7pvm" ] ; then
        ./l511e_y7pvm_platform_copy.sh
    elif [ ${HW_MOD} = "l511e_y7pm" ] || [ ${HW_MOD} = "l511en_y7pm" ] ; then
        ./l511e_y7pm_platform_copy.sh
    fi

    # copy tools
    cd ${SDK_ROOT}/tools
    cp -rf ${VENDOR_SDK_ROOT}/sdk/PLAT/tools/fcelf .

    # make sdk package
    cd $(dirname ${SDK_ROOT})
    [ -f ${VENDOR_OUTPUT_PATH}/${PACKAGE_NAME} ] && rm -f ${VENDOR_OUTPUT_PATH}/${PACKAGE_NAME}
    echo "make ${VENDOR_OUTPUT_PATH}/${PACKAGE_NAME}"
    tar zcf ${VENDOR_OUTPUT_PATH}/${PACKAGE_NAME} TuyaOS
    if [ ! -f ${VENDOR_OUTPUT_PATH}/${PACKAGE_NAME} ]; then
        echo "make ${VENDOR_OUTPUT_PATH}/${PACKAGE_NAME} failed!!!"
        exit 0
    fi
   
    if [ "${BUILD_TYPE}" == "debug" ]; then
        if [ ! -d ${SDK_ROOT}/prebuilts/linux/gcc-arm-none-eabi-10-2020-q4-major ]; then
            cp -rf ${VENDOR_SDK_ROOT}/toolchain/gcc-arm-none-eabi-10-2020-q4-major ${SDK_ROOT}/prebuilts/linux 
        fi
    fi
    echo "done"
}

if [ "${CMD}" = "clean" ]; then
    # clean target
    clean_sdk
else
    make_sdk
fi

