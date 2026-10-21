#!/bin/sh

# CI系统传入的参数说明：
#
# $1 - 工具链项目名称，如： linux-ubuntu-6.2.0_64Bit
# $2 - 工具链构建版本，如： 2.3.3-beta.86
# $3 - 工具链打包路径，如： output/dist/linux-ubuntu-6.2.0_64Bit_2.3.3-beta.86.zip
#
# 例：
# ./ci_autopack.sh linux-ubuntu-6.2.0_64Bit 2.3.3-beta.86 output/dist/linux-ubuntu-6.2.0_64Bit_2.3.3-beta.86.zip


set -e

cd `dirname $0`

# 通过环境变量传递生成的产物包全路径名称
embcli update
export CI_PACKAGE_FULLNAME="$(pwd)/$3"

[ -d $CI_PACKAGE_FULLNAME ] && rm -rf $CI_PACKAGE_FULLNAME

mkdir -p `dirname $CI_PACKAGE_FULLNAME`
echo "CI_PACKAGE_FULLNAME :$CI_PACKAGE_FULLNAME"

TOOLCHAIN=l511c_toolchain

[ -d ${TOOLCHAIN} ] && rm -rf ${TOOLCHAIN}

mkdir -p ${TOOLCHAIN}
if [ -d ${TOOLCHAIN} ];then
    echo "Starting to copy toolchain ..."
    cp -rf ./toolchain/gcc-arm-none-eabi-10-2020-q4-major ${TOOLCHAIN}/ > /dev/null
    echo "Starting to zip toolchain ..."
    tar czvf ${TOOLCHAIN}.tar.gz ${TOOLCHAIN}/ > /dev/null
    mv ${TOOLCHAIN}.tar.gz $CI_PACKAGE_FULLNAME
    echo "End pack toolchain"
    rm -rf ${TOOLCHAIN}
fi

