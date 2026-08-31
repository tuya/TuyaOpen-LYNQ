
CFLAGS_INC += -I$(TOP)/PLAT/driver/chip/$(CHIP)/common/inc \
        -I$(TOP)/PLAT/driver/chip/$(CHIP)/common/inc/xxdigRegs \
        -I$(TOP)/PLAT/driver/chip/$(CHIP)/common/inc/VPU

ifeq ($(EC718XM_VPU_ENHANCE),y)
CFLAGS_DEFS += -DFEATURE_VPU_ENHANCED
endif




