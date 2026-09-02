##
# @file toolchain_file.cmake
#/

####################################################
# Modify the content of this file
# according to the actual situation
# and configure the actual path of the compilation tool
####################################################

set(COMPILE_PREX "${PLATFORM_PATH}/../tools/gcc-arm-none-eabi-10-2020-q4-major/bin/arm-none-eabi-")

message(STATUS "Using cross compile prefix: ${COMPILE_PREX}")

set(CMAKE_C_COMPILER ${COMPILE_PREX}gcc)
set(CMAKE_CXX_COMPILER ${COMPILE_PREX}g++)
set(CMAKE_ASM_COMPILER ${COMPILE_PREX}gcc)
set(CMAKE_AR ${COMPILE_PREX}ar)
set(CMAKE_RANLIB ${COMPILE_PREX}ranlib)
set(CMAKE_STRIP ${COMPILE_PREX}strip)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# -fsanitize=address -fno-omit-frame-pointer
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} \
    -g3 \
    -mcpu=cortex-m3 \
    -mthumb \
    -std=gnu99 \
    -nostartfiles \
    -mapcs-frame \
    -specs=nano.specs \
    -Os \
    -ffunction-sections \
    -fdata-sections \
    -fno-isolate-erroneous-paths-dereference \
    -freorder-blocks-algorithm=stc \
    -Wall \
    -Wno-format \
    -gdwarf-2 \
    -fno-inline \
    -mslow-flash-data \
    -DARM_MATH_CM3"
)
