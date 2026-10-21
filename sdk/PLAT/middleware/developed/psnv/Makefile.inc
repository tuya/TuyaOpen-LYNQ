ifeq ($(BUILD_USE_PREBUILD_LIB), n)
PSNV_DIR        := $(TOP)/PROTOCOL/SRC/ps/nv/cfgnv/src
CFLAGS_INC += -I $(TOP)/PROTOCOL/SRC/ps/nv/cfgnv/inc
else
PSNV_DIR        := $(TOP)/PLAT/middleware/developed/psnv/src
CFLAGS_INC += -I $(TOP)/PLAT/middleware/developed/psnv/inc
endif
              
CFLAGS += -I $(PSNV_DIR)


ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc


$(BUILDDIR)/lib/libpsnv.a: $(PSNV_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil


lib-y += libpsnv.lib

$(BUILDDIR)/lib/libpsnv.lib: $(PSNV_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
