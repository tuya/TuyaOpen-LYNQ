
AVAILABLE_TARGETS = ec7xx_ref_1h00
TOOLCHAIN         = GCC
BINNAME           = ap_$(PROJECT)
TOP              := ../../../../../../..


#platform configuration, customers should not modify this file
include $(TOP)/PLAT/mbtk/target/$(BUILD_PRO_TARGET)/Makefile.inc

# TuyaOpen root is two levels above the platform directory ($(TOP)).
TUYA_OPEN_PATH    := $(TOP)/../..
TUYA_BASE_PATH    := $(TUYA_OPEN_PATH)/src/common/include
TKL_INC_PATH      := $(TOP)/tuyaos/tuyaos_adapter/include \
                     $(TUYA_BASE_PATH) \
                     $(wildcard ${TUYA_APP_KCONFIG_DIR})

# openlib's libcjson.a stays in the link: vendor libraries call it, including
# extensions TuyaOpen's copy does not have. TuyaOpen's own cJSON symbols are
# renamed in platform_config.cmake so both can coexist.

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


#---------------------------------------------------------------------------------

#custom change to enable TTS and ZBAR feature in SDK
SUBSYS_TTS_ENABLE = n
MBTK_OPENCPU_ZBAR = n

#referencing customer third-party libraries

#---------------------------------------------------------------------------------

# ../inc carries this project's own mode_config.h; RTE_Device.h and
# bsp_custom.h come from the tuyaos_adapter project, which is where the
# Tuya-adapted versions for this module live.
CFLAGS_INC        += -I ../inc \
                     -I ../../tuyaos_adapter/inc \
                     -I $(TOP)/FIRMWARE/SRC/CAT1/Common/Inc \
                     -I $(TOP)/PLAT/driver/chip/$(CHIP)/ap/src/usb

# Every directory under the adapter's include tree is a header directory
# (tkl_*.h sit in per-peripheral subdirectories), so add them all.
TUYA_INC := $(addprefix -I , $(shell find $(TKL_INC_PATH) -type d))
CFLAGS_INC += ${TUYA_INC}

# TuyaOpen's generated configuration, for mem_map.txt and the adapter sources.
ifneq ($(wildcard $(TUYA_BASE_PATH)/tuya_iot_config.h),)
CFLAGS_DEFS += -include tuya_iot_config.h
endif

# this project's own sources: just the application entry
APP_DIR         := $(TOP)/PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src
APP_REL_DIR     := PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src
APP_SRCS        := $(wildcard $(APP_DIR)/*.c)
obj-y           := $(patsubst %.c, %.o, $(APP_SRCS))
obj-y           := $(subst $(APP_DIR),$(APP_REL_DIR),$(obj-y))

# The board glue and the audio tone data are the tuyaos_adapter project's, not
# copies of it: same module, same pinout, and rawData.c is 2.5MB the repository
# should hold once. Makefile.rules resolves sources through `vpath %.c $(TOP)`
# and BUILDDIR is per-PROJECT, so these build into this project's own tree.
ADAPTER_SRC_DIR := PLAT/project/$(TARGET)/$(CORE)/apps/tuyaos_adapter/src
obj-y           += $(ADAPTER_SRC_DIR)/bsp_custom.o \
                   $(ADAPTER_SRC_DIR)/rawData.o

# The TKL implementation lives in the platform's own tuyaos/ directory. Of the
# tkl_init_* glue only the parts this module has a driver for are built: it is a
# cellular module, so wifi and wired are left out. src/ml stays out too -- its
# ml_internal.h includes an ml.h the OEM package does not ship.
TKL_SRC_PATH    := $(TOP)/tuyaos/tuyaos_adapter/src \
                   $(TOP)/tuyaos/tuyaos_adapter/include/utilities/src
TKL_SRCS        := $(foreach dir,$(TKL_SRC_PATH),$(wildcard $(dir)/*.c))
TKL_INIT_DIR    := $(TOP)/tuyaos/tuyaos_adapter/include/init/src
TKL_SRCS        += $(TKL_INIT_DIR)/tkl_init_common.c \
                   $(TKL_INIT_DIR)/tkl_init_system.c \
                   $(TKL_INIT_DIR)/tkl_init_network.c \
                   $(TKL_INIT_DIR)/tkl_init_cellular.c
TKL_OBJ         := $(patsubst %.c, %.o, $(TKL_SRCS))
TKL_OBJ         := $(subst $(TOP)/,,$(TKL_OBJ))
obj-y           += $(TKL_OBJ)

#---------------------------------------------------------------------------------
include $(TOP)/PLAT/tools/scripts/Makefile.rules

# mem_map.txt depends on the flash sizes TuyaOpen's configuration carries.
ifneq ($(wildcard $(TUYA_BASE_PATH)/tuya_iot_config.h),)
$(BUILDDIR)/mem_map.txt: $(TUYA_BASE_PATH)/tuya_iot_config.h
endif

# TuyaOpen's own archives, produced by cmake+ninja before this build runs.
LIBS += ${TUYA_LIB_DIR}/libtuyaos.a
LIBS += ${TUYA_LIB_DIR}/libtuyaapp.a

#Make all warnings into errors
#CFLAGS += -Werror -Wno-unused-variable -Wno-unused-but-set-variable

ifneq ($(OPENCPU_MODE_ENABLE),y)
ifneq ($(BUILD_AT),y)
$(error This example needs to modify "BUILD_AT" to "y" in device\target\board\$(TARGET)\$(CORE)\$(TARGET)_$(CORE).mk)
endif
endif
