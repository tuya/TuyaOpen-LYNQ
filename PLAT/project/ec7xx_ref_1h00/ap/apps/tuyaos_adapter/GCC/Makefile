
AVAILABLE_TARGETS = ec7xx_ref_1h00
TOOLCHAIN         = GCC
BINNAME           = ap_$(PROJECT)
TOP               := ../../../../../../..
# TuyaOpen root is two levels above the platform directory ($(TOP)).
TUYA_OPEN_PATH    := $(TOP)/../..
TUYA_BASE_PATH    := $(TUYA_OPEN_PATH)/src/common/include
TKL_INC_PATH      := $(TOP)/tuyaos/tuyaos_adapter/include \
                     $(TUYA_BASE_PATH) \
                     $(wildcard ${TUYA_APP_KCONFIG_DIR})

#platform configuration, customers should not modify this file
include $(TOP)/PLAT/mbtk/target/$(BUILD_PRO_TARGET)/Makefile.inc


ifeq ($(EXCEPTION_FLASH_DUMP_ENABLE), y)
CFLAGS_DEFS += -DFEATURE_EXCEPTION_FLASH_DUMP_ENABLE
endif

ifeq ($(MIDDLEWARE_AMR_ENABLE), y)
CFLAGS_DEFS += -DFEATURE_AMR_CP_ENABLE
endif

ifeq ($(MIDDLEWARE_VEM_ENABLE), y)
CFLAGS_DEFS += -DFEATURE_VEM_CP_ENABLE
endif


ifeq ($(IMS_ENABLE), true)
CFLAGS_DEFS += -DFEATURE_IMS_ENABLE
CFLAGS_DEFS += -DFEATURE_IMS_CC_ENABLE
CFLAGS_DEFS += -DFEATURE_IMS_SMS_ENABLE
ifneq ($(findstring $(TYPE), ec718p ec718u),)
CFLAGS_DEFS += -DFEATURE_IMS_USE_PSRAM_ENABLE
endif
CFLAGS_DEFS += -DFEATURE_IMS_EMC_ENABLE
CFLAGS_DEFS += -DFEATURE_IMS_UT_ENABLE
CFLAGS_DEFS += -fno-strict-aliasing
endif

ifeq ($(IMS_SMSONLY_ENABLE), true)
CFLAGS_DEFS += -DFEATURE_IMS_ENABLE
CFLAGS_DEFS += -DFEATURE_IMS_SMS_ENABLE
CFLAGS_DEFS += -fno-strict-aliasing
#        NO FEATURE_IMS_CC_ENABLE
endif

ifeq ($(AUDIO_ENABLE), true)
ifeq ($(BUILD_SUPPORT_APP_PCM_MEM_POOL), y)
CFLAGS_DEFS += -DFEATURE_SUPPORT_APP_PCM_MEM_POOL
endif
CFLAGS_DEFS += -DFEATURE_AUDIO_ENABLE
endif

ifneq ($(findstring $(TYPE), ec716s ec716e),)
ifeq ($(RAM_ENBALE), true)
CFLAGS_DEFS += -DFEATURE_MORERAM_ENABLE
endif
endif

ifeq ($(ROM_ENABLE), true)
CFLAGS_DEFS += -DFEATURE_MOREROM_ENABLE
endif

ifeq ($(LESS_LOG), true)
CFLAGS_DEFS += -DFEATURE_LESSLOG_ENABLE
endif

ifneq ($(AUDIO_PLAY_ORIGINAL_MODE_ENABLE),)
CFLAGS_DEFS += -DAUDIO_PLAY_ORIGINAL_MODE_ENABLE=$(AUDIO_PLAY_ORIGINAL_MODE_ENABLE)
endif
#---------------------------------------------------------------------------------

#custom change to enable TTS and ZBAR feature in SDK
SUBSYS_TTS_ENABLE = n
MBTK_OPENCPU_ZBAR = n

#referencing customer third-party libraries
# PREBUILDLIBS += ../lib/libtest.a

#---------------------------------------------------------------------------------

CFLAGS_INC        += -I ../inc \
                     -I $(TOP)/PLAT/project/$(TARGET)/$(CORE)/apps/app_demo/inc \
                     -I $(TOP)/FIRMWARE/SRC/CAT1/Common/Inc \
                     -I $(TOP)/PLAT/driver/chip/$(CHIP)/ap/src/usb

# Every directory under the adapter's include tree is a header directory
# (tkl_*.h sit in per-peripheral subdirectories), so add them all.
TUYA_INC := $(addprefix -I , $(shell find $(TKL_INC_PATH) -type d))
CFLAGS_INC += ${TUYA_INC}

# Use TuyaOS .config (EXF_FLASH_SIZE, etc.) when generating mem_map.txt and building adapter
ifneq ($(wildcard $(TUYA_BASE_PATH)/tuya_iot_config.h),)
CFLAGS_DEFS += -include tuya_iot_config.h
endif

ADAPTER_DIR := $(TOP)/PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src
ADAPTER_REL_DIR := PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src
ADAPTER_SRCS = $(wildcard $(ADAPTER_DIR)/*.c)
obj-y := $(patsubst %.c, %.o, $(ADAPTER_SRCS))
obj-y := $(subst $(ADAPTER_DIR),$(ADAPTER_REL_DIR),$(obj-y))

# The TKL implementation lives in the platform's own tuyaos/ directory. Of the
# tkl_init_* glue only the parts this module actually has a driver for are
# built: it is a cellular module, so wifi and wired are left out.
# src/ml is left out: its ml_internal.h includes an ml.h that the OEM package
# does not ship.
TKL_SRC_PATH := $(TOP)/tuyaos/tuyaos_adapter/src \
                $(TOP)/tuyaos/tuyaos_adapter/include/utilities/src
TKL_SRCS := $(foreach dir,$(TKL_SRC_PATH),$(wildcard $(dir)/*.c))
TKL_INIT_DIR := $(TOP)/tuyaos/tuyaos_adapter/include/init/src
TKL_SRCS += $(TKL_INIT_DIR)/tkl_init_common.c \
            $(TKL_INIT_DIR)/tkl_init_system.c \
            $(TKL_INIT_DIR)/tkl_init_network.c \
            $(TKL_INIT_DIR)/tkl_init_cellular.c
TKL_OBJ := $(patsubst %.c, %.o, $(TKL_SRCS))
TKL_OBJ := $(subst $(TOP)/,,$(TKL_OBJ))
obj-y += $(TKL_OBJ)
#---------------------------------------------------------------------------------
include $(TOP)/PLAT/tools/scripts/Makefile.rules

ifneq ($(wildcard $(TUYA_BASE_PATH)/tuya_iot_config.h),)
$(BUILDDIR)/mem_map.txt: $(TUYA_BASE_PATH)/tuya_iot_config.h
endif

# CFLAGS += -Wno-unused-function -Wno-unused-variable
ADAPTER_OBJS     := $(addprefix $(BUILDDIR)/, $(obj-y))
ADAPTER_PPFILES  := $(patsubst %.o, %.pp, $(ADAPTER_OBJS))
PPFILES += $(ADAPTER_PPFILES)

LIBS += ${TUYA_LIB_DIR}/libtuyaos.a
LIBS += ${TUYA_LIB_DIR}/libtuyaapp.a
$(BUILDDIR)/lib/libtuyaos_adapter.a: $(ADAPTER_OBJS)
#	@echo $(ADAPTER_OBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

tuyaos_adapter: $(BUILDDIR)/lib/libtuyaos_adapter.a $(LIBS) $(LD_COMPILED_FILE) $(BUILDDIR)/mem_map.txt
	$(eval LIBS += $(BUILDDIR)/lib/libtuyaos_adapter.a)

elf:
	@echo "make elf"
	@echo ${obj-y}

ifneq ($(OPENCPU_MODE_ENABLE),y)
ifneq ($(BUILD_AT),y)
$(error This example needs to modify "BUILD_AT" to "y" in device\target\board\$(TARGET)\$(CORE)\$(TARGET)_$(CORE).mk)
endif
endif
BUILD_USE_QR_CODE_LIB = n
ifeq ($(BUILD_USE_QR_CODE_LIB), y)
CFLAGS += -DUSE_DECODE_LIB
PREBUILDLIBS += $(TOP)/PLAT/middleware/thirdparty/multimedia/posdecoder/decoder.a
endif