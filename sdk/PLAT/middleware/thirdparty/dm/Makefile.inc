DM_DIR        := $(TOP)/PLAT/middleware/thirdparty/dm

CFLAGS_INC     +=  -I $(TOP)/PLAT/middleware/thirdparty/dm \
                   -I $(TOP)/PLAT/middleware/thirdparty/dm/include
              


ifeq ($(THIRDPARTY_CMCC_DM_ENABLE),y)
           
CFLAGS += -DFEATURE_CMCC_DM_ENABLE -DCIS_EMBED
endif

ifeq ($(THIRDPARTY_CTCC_DM_ENABLE),y)
CFLAGS += -DFEATURE_CTCC_DM_ENABLE 
endif

ifeq ($(THIRDPARTY_CUCC_DM_ENABLE),y)
DM_SRC_DIRS += $(DM_DIR)/cucc

CFLAGS += -DFEATURE_CUCC_DM_ENABLE 
endif

DM_EXCLUDE_FILES :=
DM_CSRC = $(foreach dir, $(DM_SRC_DIRS), $(wildcard $(dir)/*.c))

ifeq ($(THIRDPARTY_CMCC_DM_ENABLE),y)
DM_CSRC += $(DM_DIR)/cmcc_dm.c 
endif
ifeq ($(THIRDPARTY_CTCC_DM_ENABLE),y)
DM_CSRC += $(DM_DIR)/ctcc_dm.c
endif
ifeq ($(THIRDPARTY_CUCC_DM_ENABLE),y)
DM_CSRC += $(DM_DIR)/cucc_dm.c
endif

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc
CFLAGS_INC += -I $(GCCLIB_PATH)/arm-none-eabi/include


$(BUILDDIR)/lib/libdm.a: $(DM_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil


lib-y += libdm.lib

$(BUILDDIR)/lib/libdm.lib: $(DM_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
