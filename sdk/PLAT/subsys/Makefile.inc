SUBSYS_HOST_DIRS := $(TOP)/PLAT/subsys
CFLAGS += -DARM_MATH_CM3

CFLAGS_INC += -I $(SUBSYS_HOST_DIRS)/common/inc \
              -I $(SUBSYS_HOST_DIRS)/input/inc  \
              -I $(SUBSYS_HOST_DIRS)/status/inc \
              -I $(SUBSYS_HOST_DIRS)/apphub/inc \
              -I $(SUBSYS_HOST_DIRS)/misc/inc   \
              -I $(CROSS_COMPILE)/../../PACK/ARM/CMSIS/5.0.1/CMSIS/include

ifeq ($(SUBSYS_STORAGE_ENABLE),y)
CFLAGS_INC += -I $(SUBSYS_HOST_DIRS)/storage/inc
include $(TOP)/PLAT/subsys/storage/Makefile.inc
endif

ifeq ($(SUBSYS_GUI_ENABLE),y)
include $(SUBSYS_HOST_DIRS)/gui/Makefile.inc
endif

ifeq ($(SUBSYS_MEDIA_ENABLE),y)
CFLAGS_INC += -I $(SUBSYS_HOST_DIRS)/media/inc
include $(TOP)/PLAT/subsys/media/Makefile.inc
endif

ifeq ($(SUBSYS_MEDIA_BASIC_ENABLE),y)
include $(TOP)/PLAT/subsys/media_basic/Makefile.inc
endif

ifeq ($(SUBSYS_APPHUB_ENABLE),y)
include $(TOP)/PLAT/subsys/apphub/Makefile.inc
endif

ifeq ($(SUBSYS_OPENHAL_ENABLE),y)
include $(TOP)/PLAT/subsys/openhal/Makefile.inc
endif

ifeq ($(SUBSYS_SYSTEST_ENABLE),y)
include $(TOP)/PLAT/subsys/systest/Makefile.inc
endif

ifeq ($(SUBSYS_SYSTIME_ENABLE),y)
include $(TOP)/PLAT/subsys/systime/Makefile.inc
endif

ifeq ($(SUBSYS_SYSLOG_ENABLE),y)
include $(TOP)/PLAT/subsys/syslog/Makefile.inc
endif

ifeq ($(SUBSYS_COMM_ENABLE),y)
include $(TOP)/PLAT/subsys/comm/Makefile.inc
endif

ifeq ($(SUBSYS_APP_MANAGER_ENABLE),y)
include $(TOP)/PLAT/subsys/app_manager/Makefile.inc
endif

ifeq ($(SUBSYS_OPENSDK_ENABLE),y)
include $(TOP)/PLAT/subsys/opensdk/Makefile.inc
endif

ifeq ($(SUBSYS_OPENDDK_ENABLE),y)
include $(TOP)/PLAT/subsys/openddk/Makefile.inc
endif

ifeq ($(SUBSYS_OPENPLAYER_ENABLE),y)
include $(TOP)/PLAT/subsys/openplayer/Makefile.inc
endif

ifeq ($(SUBSYS_CAMERA_ENABLE),y)
include $(TOP)/PLAT/subsys/camera/Makefile.inc
endif

ifeq ($(SUBSYS_OPENRECORD_ENABLE),y)
include $(TOP)/PLAT/subsys/openrecorder/Makefile.inc
endif

ifeq ($(SUBSYS_GRAPHIC_ENABLE),y)
include $(TOP)/PLAT/subsys/graphic/Makefile.inc
endif

ifeq ($(SUBSYS_ALIPAY_ENABLE),y)
include $(TOP)/PLAT/subsys/pay/alipay/Makefile.inc
endif

ifeq ($(SUBSYS_GNSS_ENABLE),y)
include $(TOP)/PLAT/subsys/gnss/Makefile.inc
endif

ifeq ($(SUBSYS_SENSORHUB_ENABLE),y)
include $(TOP)/PLAT/subsys/sensorhub/Makefile.inc
endif

ifeq ($(SUBSYS_VOLUME_MANAGER_ENABLE),y)
include $(TOP)/PLAT/subsys/volume/Makefile.inc
endif

ifeq ($(SUBSYS_ZIP_ENABLE),y)
include $(TOP)/PLAT/subsys/zip/Makefile.inc
endif
