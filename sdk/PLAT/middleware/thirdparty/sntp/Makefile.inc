ifeq ($(BUILD_USE_PREBUILD_LIB), n)
SNTP_DIR        := $(TOP)/PROTOCOL/SRC/tcpip/sntp
else
SNTP_DIR        := $(TOP)/PLAT/middleware/thirdparty/sntp
endif
              
CFLAGS += -I $(SNTP_DIR)


ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc


$(BUILDDIR)/lib/libsntp.a: $(SNTP_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil


lib-y += libsntp.lib

$(BUILDDIR)/lib/libsntp.lib: $(SNTP_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
