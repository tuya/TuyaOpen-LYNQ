##
# @file platform_config.cmake
#/

####################################################
# By configuring the variable PLATFORM_PUBINC,
# the header file in the platform is provided to TuyaOpen for use.
####################################################

list_subdirectories(PLATFORM_PUBINC ${PLATFORM_PATH}/tuyaos/tuyaos_adapter)

# Core compile view: self-contained lwip_config_cat.h and arch/cc.h for root
# CMake translation units (tuyaopen-core). Lives outside tuyaos_adapter/include
# so the vendor Makefile's include scan never picks it up.
set(PLATFORM_CORE_LWIP_VIEW "${CMAKE_CURRENT_LIST_DIR}/tuyaos/core_lwip_view")
list(INSERT PLATFORM_PUBINC 0 "${PLATFORM_CORE_LWIP_VIEW}")
list(APPEND PLATFORM_PUBINC "${PLATFORM_PATH}/PLAT/middleware/thirdparty/lwip/src/include")

# This platform implements the TKL file system API used by core's FS ops card.
add_definitions(-DTUYAOPEN_HAS_TKL_FS=1)
