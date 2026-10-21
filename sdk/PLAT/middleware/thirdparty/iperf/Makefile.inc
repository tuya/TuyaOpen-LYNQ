ifeq ($(BUILD_USE_PREBUILD_LIB), n)
IPERFDIR        := $(TOP)/PROTOCOL/SRC/tcpip/iperf/iperf-1.7.0
else
IPERFDIR        := $(TOP)/PLAT/middleware/thirdparty/iperf
endif

CFLAGS_INC += -I $(IPERFDIR)

IPERF_SRC_DIRS += $(IPERFDIR)


IPERF_EXCLUDE_FILES :=                                   


ifeq ($(TOOLCHAIN),GCC)



$(BUILDDIR)/lib/libiperf.a: $(IPERF_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

lib-y += libiperf.lib

$(BUILDDIR)/lib/libiperf.lib: $(IPERF_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
