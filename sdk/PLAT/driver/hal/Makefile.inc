
CFLAGS_INC += -I$(TOP)/PLAT/driver/hal/common/inc \
              -I$(TOP)/PLAT/driver/hal/$(CHIP)/$(CORE)/inc \
              -I$(TOP)/PLAT/driver/hal/$(CHIP)/common/inc \

ifeq ($(TYPE), ec716s)
CFLAGS_INC += -I$(TOP)/PLAT/driver/hal/$(CHIP)/$(CORE)/inc/ec716
else ifeq ($(TYPE), ec716e)
CFLAGS_INC += -I$(TOP)/PLAT/driver/hal/$(CHIP)/$(CORE)/inc/ec716
else
CFLAGS_INC += -I$(TOP)/PLAT/driver/hal/$(CHIP)/$(CORE)/inc/ec718
endif



ifeq ($(THIRDPARTY_DHCPD_ENABLE),y)
CFLAGS += -DDHCPD_ENABLE_DEFINE=1
else
CFLAGS += -DDHCPD_ENABLE_DEFINE=0
endif

