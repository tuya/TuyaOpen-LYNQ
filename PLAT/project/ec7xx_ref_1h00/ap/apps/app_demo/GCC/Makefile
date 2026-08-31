
AVAILABLE_TARGETS = ec7xx_ref_1h00
TOOLCHAIN         = GCC
BINNAME           = ap_$(PROJECT)
TOP              := ../../../../../../..


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


#---------------------------------------------------------------------------------

#custom change to enable TTS and ZBAR feature in SDK
SUBSYS_TTS_ENABLE = n
MBTK_OPENCPU_ZBAR = n

#referencing customer third-party libraries
PREBUILDLIBS += ../lib/libtest.a

#---------------------------------------------------------------------------------

CFLAGS_INC        += -I ../inc \
                     -I $(TOP)/FIRMWARE/SRC/CAT1/Common/Inc \
                     -I $(TOP)/PLAT/driver/chip/$(CHIP)/ap/src/usb

#app project base code, cannot be deleted
obj-y             += PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src/app.o \
                     PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src/bsp_custom.o

#various function demos, customers can choose to add
obj-y             += PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src/app_demo_menu.o \
                     PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src/os_demo.o \
                     PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src/fs_demo.o \
                     PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src/time_demo.o \
                     PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src/sleep_demo.o \
                     PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src/uart_demo.o \
                     PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src/gpio_demo.o \
                     PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src/adc_demo.o \
                     PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src/i2c_demo.o \
                     PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src/spi_demo.o \
                     PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src/pwm_demo.o \
                     PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src/network_demo.o \
                     PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src/socket_demo.o \
                     PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src/ping_demo.o \
                     PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src/lbs_demo.o \
                     PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src/ftp_demo.o \
                     PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src/http_demo.o \
                     PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src/fota_demo.o \
                     PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src/ex_flash_demo.o \


ifeq ($(MBTK_OPENCPU_AT), y)
obj-y   += PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src/at_demo.o
endif

ifeq ($(MBTK_OPENCPU_SMS), y)
obj-y   += PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src/sms_demo.o
endif

ifeq ($(MBTK_OPENCPU_TCP), y)
obj-y   += PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src/tcp_demo.o
endif

ifeq ($(MBTK_OPENCPU_SSL), y)
obj-y   += PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src/ssl_demo.o
endif

ifeq ($(THIRDPARTY_MQTT_ENABLE), y)
obj-y   += PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src/mqtt_demo.o
endif

ifeq ($(THIRDPARTY_CTWING1_5_ENABLE), y)
obj-y   += PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src/lwm2m_demo.o
endif

ifeq ($(SUBSYS_AUDIO_ENABLE), y)
obj-y   += PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src/audio_demo.o \
           PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src/rawData.o
endif


ifeq ($(MBTK_OPENCPU_CALL), y)
obj-y   += PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src/call_demo.o
endif

ifeq ($(MBTK_OPENCPU_GPS_SUPPORT), y)
obj-y   += PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src/gps_demo.o
endif
#---------------------------------------------------------------------------------
include $(TOP)/PLAT/tools/scripts/Makefile.rules

#Make all warnings into errors
#CFLAGS += -Werror -Wno-unused-variable -Wno-unused-but-set-variable

ifneq ($(OPENCPU_MODE_ENABLE),y)
ifneq ($(BUILD_AT),y)
$(error This example needs to modify "BUILD_AT" to "y" in device\target\board\$(TARGET)\$(CORE)\$(TARGET)_$(CORE).mk)
endif
endif
