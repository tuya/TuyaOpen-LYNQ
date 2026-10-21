# Copyright (C) 2018 RDA Technologies Limited and/or its affiliates("RDA").
# All rights reserved.
#
# This software is supplied "AS IS" without any warranties.
# RDA assumes no responsibility or liability for the use of the software,
# conveys no license or title under any patent, copyright, or mask work
# right to the product. RDA reserves the right to make changes in the
# software without notification.  RDA also make no representation or
# warranty that such application will be suitable for the specified use
# without further testing or modification.

# Configures CMake for using GCC

set(CMAKE_SYSTEM_NAME           Generic)
set(CROSS_COMPILE arm-none-eabi-)
find_program(CMAKE_C_COMPILER   ${CROSS_COMPILE}gcc)
find_program(CMAKE_CXX_COMPILER ${CROSS_COMPILE}g++)
find_program(CMAKE_READELF      ${CROSS_COMPILE}readelf)

set(CMAKE_EXECUTABLE_SUFFIX_ASM .elf)
set(CMAKE_EXECUTABLE_SUFFIX_C .elf)
set(CMAKE_EXECUTABLE_SUFFIX_CXX .elf)

if (${CMAKE_C_COMPILER} STREQUAL "CMAKE_C_COMPILER-NOTFOUND")
message(FATAL_ERROR "\t\nFATAL_ERROR:\n\"${CROSS_COMPILE}gcc is not found !\"\n")
endif()
execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE gcc_version OUTPUT_STRIP_TRAILING_WHITESPACE)

# 设置编译器和链接器标志
set(abi_options "-mcpu=cortex-m3" "-mthumb" "-nostartfiles" "-mapcs-frame" "-specs=nano.specs" "-Os" "-ffunction-sections" "-fdata-sections"
                "-fno-isolate-erroneous-paths-dereference" "-freorder-blocks-algorithm=stc" "-Wall" "-Wno-format" "-gdwarf-2"
                "-Wall" "-mthumb" "-gdwarf-2" "-Wall" "-D__ASSEMBLY__" "-mthumb" "-gdwarf-2")
set(lto_link_option -specs=nano.specs -Wl,--cref -Wl,--check-sections -Wl,--gc-sections -Wl,--print-memory-usage
                    -Wl,--wrap=_malloc_r -Wl,--wrap=_free_r -Wl,--wrap=_realloc_r)

# 设置编译器定义
add_definitions(-DARM_MATH_CM3)

# 根据是否定义DEBUG来设置TRACE_LEVEL
if(DEFINED DEBUG)
  add_definitions(-DTRACE_LEVEL=5)
else()
  add_definitions(-DNDEBUG -DTRACE_LEVEL=4)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-exceptions -fno-rtti -fno-threadsafe-statics")

add_compile_options(${abi_options} -g -Os)
add_link_options(${abi_options} -Os ${lto_link_option})
# GNU ar will alreay create index
set(CMAKE_C_ARCHIVE_FINISH "")
set(CMAKE_CXX_ARCHIVE_FINISH "")
set(multilib_opions ${abi_options})
execute_process(COMMAND ${CMAKE_C_COMPILER} ${multilib_opions} --print-file-name libgcc.a
    OUTPUT_VARIABLE libgcc_file_name
    OUTPUT_STRIP_TRAILING_WHITESPACE
)