#!/bin/bash
# L511C-Y7PVM Platform Configuration
# This script defines platform-specific variables and build functions

# Platform identification
PLATFORM_NAME='l511c_y7pvm'
PLATFORM_ALIASES=("l511c_y7pm" "l511e_y7pm" "l511en_y7pvm" "l511en_y7pm")  # Other names that map to this platform

# Platform match function - called by build.sh to check if this config matches
platform_match() {
    local hw_mod="$1"
    
    # Check if matches platform name
    if [ "${hw_mod}" = "${PLATFORM_NAME}" ]; then
        return 0
    fi
    
    # Check if matches any alias
    for alias in "${PLATFORM_ALIASES[@]}"; do
        if [ "${hw_mod}" = "${alias}" ]; then
            return 0
        fi
    done
    
    return 1
}

# Platform paths and toolchain
setup_platform_env() {
    export VENDOR_SDK_PATH="vendor/L511C-Y7PVM"
    export USER_MANUAL_PATH="TuyaOS/docs/OpenCPU_User_Programming_Manual"
    export VENDOR_TOOLCHAIN_PATH=$(realpath ${VENDOR_SDK_PATH}/toolchain/gcc-arm-none-eabi-10-2020-q4-major/bin)
    export AR="arm-none-eabi-ar"
    export MODULE_SCRIPT_PATH=${VENDOR_SDK_PATH}/sdk/PLAT
    if [ -d ${MODULE_SCRIPT_PATH} ]; then
        return 0
    else 
        return 1
    fi
}

demo_clean()
{
    echo "Cleaning demo"
    cd ${VENDOR_SDK_PATH}/../..
    ./build_demo.sh clean
    local result=$?
    cd - > /dev/null
    return $result
}

demo_build()
{
    echo "Cleaning demo"
    cd ${VENDOR_SDK_PATH}/../..
    ./build_demo.sh
    local result=$?
    cd - > /dev/null
    return $result
}

# Platform-specific build function
platform_build() {
    echo "Building ${HW_MOD} platform"
    cd ${MODULE_SCRIPT_PATH} 
    if [ "${HW_MOD}" = 'l511c_y7pvm' ];then 
        ./build_c_pvm.sh lib
    elif [ "${HW_MOD}" = 'l511c_y7pm' ];then 
        ./build_c_pm.sh lib
    elif [ "${HW_MOD}" = 'l511e_y7pm' ] || [ "${HW_MOD}" = 'l511en_y7pm' ];then 
        ./build_e_pm.sh lib
    elif [ "${HW_MOD}" = 'l511en_y7pvm' ];then 
        ./build_e_pvm.sh lib
    fi
    local result=$?
    cd - > /dev/null
    return $result
}

# Platform-specific clean function
platform_clean() {
    echo "Cleaning ${HW_MOD} platform"
    cd ${MODULE_SCRIPT_PATH} 
    if [ "${HW_MOD}" = 'l511c_y7pvm' ];then 
        ./build_c_pvm.sh clean
    elif [ "${HW_MOD}" = 'l511c_y7pm' ];then 
        ./build_c_pm.sh clean
    elif [ "${HW_MOD}" = 'l511e_y7pm' ] || [ "${HW_MOD}" = 'l511en_y7pm' ];then 
        ./build_e_pm.sh clean
    elif [ "${HW_MOD}" = 'l511en_y7pvm' ];then 
        ./build_e_pvm.sh clean
    fi
    local result=$?
    cd - > /dev/null
    return $result
}

# Platform-specific SDK build function
platform_build_sdk() {
    echo "Building SDK for ${HW_MOD}"
    if [ -d ${VENDOR_SDK_PATH}/toolchain ]; then
        cd ${VENDOR_SDK_PATH}/toolchain
        ./build.sh
        cd - > /dev/null
    else
        echo "No toolchain found for ${HW_MOD}"
        return 1
    fi
}

# Platform-specific SDK clean function
platform_clean_sdk() {
    echo "Cleaning SDK for ${HW_MOD}"
    if [ -d ${VENDOR_SDK_PATH}/toolchain ]; then
        cd ${VENDOR_SDK_PATH}/toolchain
        ./build.sh clean
        cd - > /dev/null
    fi
}

platform_build_unilog() {
    echo "Building unilog for ${HW_MOD}"
    # cd ${MODULE_SCRIPT_PATH} 
    # ./build.sh unilog
    # cd - > /dev/null
}   

platform_clean_unilog() {
    echo "Cleaning unilog for ${HW_MOD}"
    # cd ${MODULE_SCRIPT_PATH} 
    # ./build.sh unilog clean
    # cd - > /dev/null
}