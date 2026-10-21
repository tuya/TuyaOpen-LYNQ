

ifeq ($(DRIVER_USBSMIMG_ENABLE),y)
CFLAGS_INC += -I$(TOP)/PLAT/driver/board/$(TARGET)/inc       \
              -I$(TOP)/PLAT/driver/chip/$(CHIP)/$(CORE)/inc           \
              -I$(TOP)/PLAT/driver/chip/$(CHIP)/$(CORE)/inc/pmu       \
              -I$(TOP)/PLAT/driver/chip/$(CHIP)/$(CORE)/inc/usb_bl       \
              -I$(TOP)/PLAT/driver/chip/$(CHIP)/$(CORE)/inc_cmsis    \
              -I$(TOP)/PLAT/driver/chip/$(CHIP)/$(CORE)/inc/usb_bl/usb_device  \
              -I$(TOP)/PLAT/driver/chip/$(CHIP)/$(CORE)/inc/usb_bl/usbcore_drv  \
              -I$(TOP)/PLAT/driver/chip/$(CHIP)/$(CORE)/inc/usb_bl/usbpcd_drv  \
              -I$(TOP)/PLAT/driver/chip/$(CHIP)/$(CORE)/inc/usb_bl/usb_misc  \
              -I$(TOP)/PLAT/driver/chip/$(CHIP)/$(CORE)/inc/usb_bl/usblpw
else
CFLAGS_INC += -I$(TOP)/PLAT/driver/board/$(TARGET)/inc       \
              -I$(TOP)/PLAT/driver/chip/$(CHIP)/$(CORE)/inc           \
              -I$(TOP)/PLAT/driver/chip/$(CHIP)/$(CORE)/inc/pmu       \
              -I$(TOP)/PLAT/driver/chip/$(CHIP)/$(CORE)/inc/usb       \
              -I$(TOP)/PLAT/driver/chip/$(CHIP)/$(CORE)/inc_cmsis    \
              -I$(TOP)/PLAT/driver/chip/$(CHIP)/$(CORE)/inc/usb/usb_device  \
              -I$(TOP)/PLAT/driver/chip/$(CHIP)/$(CORE)/inc/usb/usbcore_drv  \
              -I$(TOP)/PLAT/driver/chip/$(CHIP)/$(CORE)/inc/usb/usbpcd_drv  \
              -I$(TOP)/PLAT/driver/chip/$(CHIP)/$(CORE)/inc/usb/usb_misc  \
              -I$(TOP)/PLAT/driver/chip/$(CHIP)/$(CORE)/inc/usb/usblpw
endif

ifeq ($(TYPE), ec716s)
CFLAGS_INC += -I$(TOP)/PLAT/driver/chip/$(CHIP)/$(CORE)/inc/ec716
else ifeq ($(TYPE), ec716e)
CFLAGS_INC += -I$(TOP)/PLAT/driver/chip/$(CHIP)/$(CORE)/inc/ec716
else
CFLAGS_INC += -I$(TOP)/PLAT/driver/chip/$(CHIP)/$(CORE)/inc/ec718
endif

ifeq ($(DRIVER_PMU_ENABLE),y)
    CFLAGS      += -DPM_FEATURE_ENABLE
endif

ifeq ($(DRIVER_UNILOG_ENABLE),y)
    CFLAGS      += -DUINILOG_FEATURE_ENABLE
endif


ifeq ($(DRIVER_PSRAM_ENABLE),y)
    CFLAGS      += -DPSRAM_FEATURE_ENABLE
endif

#########################################################
ifeq ($(MBTK_OPENCPU_SUPPORT), y)
include $(TOP)/PLAT/driver/chip/$(CHIP)/$(CORE)/src_cmsis/Makefile.inc
endif
#########################################################

