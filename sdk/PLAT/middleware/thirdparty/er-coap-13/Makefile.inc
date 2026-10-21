ERCOAP_DIR  := $(TOP)/PLAT/middleware/thirdparty/er-coap-13

CFLAGS_INC  +=  -I $(TOP)/PLAT/middleware/thirdparty/er-coap-13

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc
CFLAGS_INC += -I $(GCCLIB_PATH)/arm-none-eabi/include

lib-y += libercoap.a

$(BUILDDIR)/lib/libercoap.a: $(ERCOAP_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)


$(BUILDDIR)/lib/libercoap.lib: $(ERCOAP_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
