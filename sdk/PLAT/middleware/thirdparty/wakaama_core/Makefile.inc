WAKAAMA_CORE_DIR  := $(TOP)/PLAT/middleware/thirdparty/wakaama_core

CFLAGS_INC  +=  -I $(TOP)/PLAT/middleware/thirdparty/wakaama_core


ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc
CFLAGS_INC += -I $(GCCLIB_PATH)/arm-none-eabi/include

lib-y += libwakaamacore.a

$(BUILDDIR)/lib/libwakaamacore.a: $(WAKAAMA_CORE_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)


$(BUILDDIR)/lib/libwakaamacore.lib: $(WAKAAMA_CORE_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
