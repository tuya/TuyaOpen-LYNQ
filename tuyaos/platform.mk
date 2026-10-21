CROSSTOOL_PATH := ${ROOT_DIR}/vendor/L511C-Y7PVM/toolchain/gcc-arm-none-eabi-10-2020-q4-major
LIB_PATH := ${CROSSTOOL_PATH}/arm-none-eabi/lib

export PATH := ${CROSSTOOL_PATH}/bin:$(PATH)

TUYA_PLATFORM_CFLAGS := -g1 -mcpu=cortex-m3 -mthumb -std=gnu99 -nostartfiles -mapcs-frame -specs=nano.specs -Os
TUYA_PLATFORM_CFLAGS += -ffunction-sections -fdata-sections -fno-isolate-erroneous-paths-dereference -freorder-blocks-algorithm=stc -Wall -Wno-format
TUYA_PLATFORM_CFLAGS += -fno-inline -mslow-flash-data --sysroot=${LIB_PATH}/ -DARM_MATH_CM3
TUYA_PLATFORM_CFLAGS += -Dmalloc=tkl_system_malloc -Dcalloc=tkl_system_calloc -Dfree=tkl_system_free -Drealloc=tkl_system_realloc

TUYA_PLATFORM_CXXFLAGS := -g1 -mcpu=cortex-m3 -mthumb  -std=c++11 -nostartfiles -mapcs-frame -specs=nano.specs -Os -mthumb --sysroot=${LIB_PATH}/
TUYA_PLATFORM_CXXFLAGS += -ffunction-sections -fdata-sections -fno-isolate-erroneous-paths-dereference -freorder-blocks-algorithm=stc -Wall -Wno-format -DARM_MATH_CM3


