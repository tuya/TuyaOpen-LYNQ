#!/bin/bash

# copy tuyaos libs
cd ${SDK_ROOT}/libs

module_name=L511C_Y7PVM
chip_type=ec718pm
CPBIN_SUBPATH=ims
board_name=ec7xx_ref_1h00
target_name=${board_name}_${chip_type}
PLATFORM_LIBS="${VENDOR_SDK_ROOT}/sdk/PLAT/gccout/${target_name}/ap/tuyaos_adapter/lib/libtuyaos_adapter.a \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/gccout/${target_name}/ap/tuyaos_adapter/lib/libL511C_Y7.a \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/gccout/${target_name}/ap/tuyaos_adapter/lib/libfinsh.a   \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/gccout/${target_name}/ap/tuyaos_adapter/lib/liblzma.a  \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/gccout/${target_name}/ap/tuyaos_adapter/lib/libopenatcmd.a \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/gccout/${target_name}/ap/tuyaos_adapter/lib/libopendriver.a    \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/gccout/${target_name}/ap/tuyaos_adapter/lib/libstartup.a   \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/gccout/${target_name}/ap/tuyaos_adapter/lib/libsubsys_amr.a    \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/gccout/${target_name}/ap/tuyaos_adapter/lib/libsubsys_apphub.a \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/gccout/${target_name}/ap/tuyaos_adapter/lib/libsubsys_audio.a  \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/gccout/${target_name}/ap/tuyaos_adapter/lib/libsubsys_console.a    \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/gccout/${target_name}/ap/tuyaos_adapter/lib/libsubsys_media.a  \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/gccout/${target_name}/ap/tuyaos_adapter/lib/libsubsys_openhal.a    \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/gccout/${target_name}/ap/tuyaos_adapter/lib/libsubsys_openplayer.a \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/gccout/${target_name}/ap/tuyaos_adapter/lib/libsubsys_openrecorder.a   \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/gccout/${target_name}/ap/tuyaos_adapter/lib/libsubsys_pcm.a    \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/gccout/${target_name}/ap/tuyaos_adapter/lib/libsubsys_sensorhub.a  \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/gccout/${target_name}/ap/tuyaos_adapter/lib/libsubsys_storage.a    \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/gccout/${target_name}/ap/tuyaos_adapter/lib/libsubsys_syslog.a \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/gccout/${target_name}/ap/tuyaos_adapter/lib/libsubsys_wav.a    \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/gccout/${target_name}/ap/tuyaos_adapter/lib/libuartservice.a   \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/openlib/${module_name}/lib/libcjson.a   \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/openlib/${module_name}/lib/libdriver.a  \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/openlib/${module_name}/lib/libfreertos.a    \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/openlib/${module_name}/lib/libhttpclient.a  \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/openlib/${module_name}/lib/libimsnv.a   \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/openlib/${module_name}/lib/libiperf.a   \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/openlib/${module_name}/lib/liblfs.a \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/openlib/${module_name}/lib/liblwip.a    \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/openlib/${module_name}/lib/libmiddleware_ec.a \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/openlib/${module_name}/lib/libopenapi.a    \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/openlib/${module_name}/lib/libping.a    \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/openlib/${module_name}/lib/libpsnv.a    \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/openlib/${module_name}/lib/libsntp.a    \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/openlib/${module_name}/lib/libtcpipmgr.a    \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/openlib/${module_name}/lib/libyrcompress.a  \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/prebuild/PLAT/lib/gcc/${chip_type}/${CPBIN_SUBPATH}/libccio.a  \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/prebuild/PLAT/lib/gcc/${chip_type}/${CPBIN_SUBPATH}/libdriver_private.a    \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/prebuild/PLAT/lib/gcc/${chip_type}/${CPBIN_SUBPATH}/libmiddleware_ec_private.a \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/prebuild/PLAT/lib/gcc/${chip_type}/${CPBIN_SUBPATH}/libosa.a   \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/prebuild/PLAT/lib/gcc/${chip_type}/${CPBIN_SUBPATH}/libusb_private.a   \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/prebuild/PS/lib/gcc/${chip_type}/${CPBIN_SUBPATH}/libims.a \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/prebuild/PS/lib/gcc/${chip_type}/${CPBIN_SUBPATH}/libimsxml.a  \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/prebuild/PS/lib/gcc/${chip_type}/${CPBIN_SUBPATH}/libps.a  \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/prebuild/PS/lib/gcc/${chip_type}/${CPBIN_SUBPATH}/libpsif.a    \
                ${VENDOR_SDK_ROOT}/sdk/PLAT/prebuild/PS/lib/gcc/${chip_type}/${CPBIN_SUBPATH}/libpsl1.a"


for lib in ${PLATFORM_LIBS}; do
    cp -rf ${lib} ./
done

# copy sysfw
cd ${SDK_ROOT}/sysfw
# copy bootloader
# ls ${VENDOR_SDK_ROOT}/sdk/PLAT/gccout/${target_name}/ap/bootloader/
cp ${VENDOR_SDK_ROOT}/sdk/PLAT/gccout/${target_name}/ap/bootloader/ap_bootloader.* .
cp ${VENDOR_SDK_ROOT}/sdk/PLAT/tools/comdb_cust_lib.txt .
cp ${VENDOR_SDK_ROOT}/sdk/PLAT/prebuild/FW/lib/gcc/${chip_type}/audio/cp-demo-flash.bin .
cp ${VENDOR_SDK_ROOT}/sdk/PLAT/mbtk/releasepack/L511C_Y7PM/${chip_type}_PrMgrCfg.json.bin  .
cp ${VENDOR_SDK_ROOT}/sdk/PLAT/mbtk/releasepack/L511C_Y7PM/format_${chip_type}.json.bin .
cp ${VENDOR_SDK_ROOT}/sdk/PLAT/mbtk/releasepack/L511C_Y7PM/config_${chip_type}_prd_uart.baseini.bin .
cp ${VENDOR_SDK_ROOT}/sdk/PLAT/mbtk/releasepack/L511C_Y7PM/config_${chip_type}_prd_usb.baseini.bin .
cp ${VENDOR_SDK_ROOT}/sdk/PLAT/gccout/${target_name}/ap/tuyaos_adapter/comdb.txt . 
cp ${VENDOR_SDK_ROOT}/sdk/PLAT/gccout/${target_name}/ap/tuyaos_adapter/mem_map.txt .
cp ${VENDOR_SDK_ROOT}/sdk/PLAT/mbtk/releasepack/L511C_Y7PM/config_*.ini .
cp ${VENDOR_SDK_ROOT}/sdk/PLAT/mbtk/releasepack/L511C_Y7PM/format* .
cp ${VENDOR_SDK_ROOT}/sdk/PLAT/device/target/board/${board_name}/ap/gcc/sectionInfo_${chip_type}.json .

# copy ldscripts
cd ${SDK_ROOT}/ldscripts
cp -rf ${VENDOR_SDK_ROOT}/sdk/PLAT/gccout/${target_name}/ap/tuyaos_adapter/PLAT/project/${board_name}/ap/apps/tuyaos_adapter/${board_name}_flash_output.ld ./flash_output.ld
