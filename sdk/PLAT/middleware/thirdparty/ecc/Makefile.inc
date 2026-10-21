
UECC_DIR      := $(TOP)/PLAT/middleware/thirdparty/ecc

CFLAGS_INC     += -I $(UECC_DIR)/inc

UECC_SRC_DIRS += $(UECC_DIR)/src


UECC_EXCLUDE_FILES :=


ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc


$(BUILDDIR)/lib/libuecc.a: $(UECC_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libuecc.lib

$(BUILDDIR)/lib/libuecc.lib: $(UECC_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
