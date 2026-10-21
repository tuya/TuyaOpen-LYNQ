ifeq ($(BUILD_USE_PREBUILD_LIB), n)
PINGDIR        := $(TOP)/PROTOCOL/SRC/tcpip/ping/src
else
PINGDIR        := $(TOP)/PLAT/middleware/thirdparty/ping/src
endif

CFLAGS_INC += -I $(PINGDIR)

PING_SRC_DIRS += $(PINGDIR)


PING_EXCLUDE_FILES :=                                   

ifeq ($(TOOLCHAIN),GCC)

$(BUILDDIR)/lib/libping.a: $(PING_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

lib-y += libping.lib

$(BUILDDIR)/lib/libping.lib: $(PING_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
