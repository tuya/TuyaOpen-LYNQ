ifeq ($(BUILD_USE_PREBUILD_LIB), n)
TCPIPMGRDIR  := $(TOP)/PROTOCOL/SRC/tcpip/tcpipmgr
else
TCPIPMGRDIR  := $(TOP)/PLAT/middleware/developed/tcpipmgr
endif




CFLAGS_INC         += -I $(TCPIPMGRDIR)/app/inc  \
					  -I $(TCPIPMGRDIR)/common/inc



TCPIPMGR_SRC_DIRS  += $(TCPIPMGRDIR)/app/src   \
					  $(TCPIPMGRDIR)/common/src 

TCPIPMGR_EXCLUDE_FILES :=                                   


ifeq ($(TOOLCHAIN),GCC)


$(BUILDDIR)/lib/libtcpipmgr.a: $(TCPIPMGRDIR_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

lib-y += libtcpipmgr.lib

$(BUILDDIR)/lib/libtcpipmgr.lib: $(TCPIPMGRDIR_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
