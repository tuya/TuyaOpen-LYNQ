PROJ=ml
FW_NAME ?= l511cy
FW_VER ?= 1.0.0
BUILD_PATH := build
OBJ_PATH := $(BUILD_PATH)/objs
SRC_PATH := src

SDK_ROOT := $(shell pwd)/../../../../../../../..
CROSS_ROOT_PATH := $(SDK_ROOT)/../toolchain/gcc-arm-none-eabi-10-2020-q4-major
CROSS_TOOL_PATH := $(CROSS_ROOT_PATH)/bin
CROSS_LIB_PATH := $(CROSS_ROOT_PATH)/arm-none-eabi/lib
TUYA_BASE_PATH := $(SDK_ROOT)/../../../output/libtuyaos/ccinclude/base
TKL_INC_PATH := $(SDK_ROOT)/../../../output/libtuyaos/ccinclude/tkl
CROSS_COMPILE := $(CROSS_TOOL_PATH)/arm-none-eabi-

CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
CPP := $(CROSS_COMPILE)gcc -E
AS := $(CROSS_COMPILE)as
LD := $(CROSS_COMPILE)ld
AR := $(CROSS_COMPILE)ar
NM := $(CROSS_COMPILE)nm
STRIP := $(CROSS_COMPILE)strip
RANLIB := $(CROSS_COMPILE)ranlib
OBJCOPY := $(CROSS_COMPILE)objcopy
OBJDUMP := $(CROSS_COMPILE)objdump
SIZE := $(CROSS_COMPILE)size

CFLAGS := -g3 -mcpu=cortex-m3 -mthumb -std=gnu99 -nostartfiles -mapcs-frame -specs=nano.specs -Os
CFLAGS += -ffunction-sections -fdata-sections -fno-isolate-erroneous-paths-dereference -freorder-blocks-algorithm=stc -Wall -Wno-format -gdwarf-2
CFLAGS += -fno-inline -mslow-flash-data -mcpu=cortex-m3 -mthumb --sysroot=${CROSS_LIB_PATH}/
CFLAGS += -DFW_NAME=\"$(FW_NAME)\" -DFW_VER=\"$(FW_VER)\"

CXXFLAGS := $(CFLAGS)

LDFLAGS := -Wl,-O1 -Wl,--hash-style=gnu -Wl,--as-needed

SRCS := ml.c ml_cp936.c ml_gsm.c ml_iso8859_1.c ml_utf16.c ml_utf8.c 

OBJS := $(patsubst %.c, $(OBJ_PATH)/%.o, $(SRCS))

INCLUDE := -I./include

all: lib${PROJ}.a

lib${PROJ}.a: ${OBJS}
	@echo "AR rcs $@ $^"
	@$(AR) rcs $(BUILD_PATH)/$@ $^

$(OBJ_PATH)/%.o: src/%.c
	@mkdir -p $(OBJ_PATH)
	@echo "CC $<..."
	@$(CC) $(CFLAGS) $(INCLUDE) -g -c -o $@ $<

clean:
	rm -rf $(BUILD_PATH)

.PHONY: all clean
