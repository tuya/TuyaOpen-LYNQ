
BZIP2_DIR      := $(TOP)/PLAT/middleware/thirdparty/bzip2


CFLAGS_INC     += -I $(BZIP2_DIR)

BZIP2_SRC_DIRS += $(BZIP2_DIR) 


CFLAGS += -DFEATURE_BZIP2_ENABLE


ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc


$(BUILDDIR)/lib/libbzip2.a: $(BZIP2_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libbzip2.lib

$(BUILDDIR)/lib/libbzip2.lib: $(BZIP2_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
