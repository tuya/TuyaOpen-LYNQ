
HTTPCLIENT_DIR         := $(TOP)/PLAT/middleware/thirdparty/httpclient
CFLAGS_INC       +=  -I $(HTTPCLIENT_DIR) 
              
CFLAGS += -DFEATURE_HTTPC_ENABLE
HTTPCLIENT_SRC_DIRS += $(HTTPCLIENT_DIR) 
                 

HTTPCLIENT_EXCLUDE_FILES :=


ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc


$(BUILDDIR)/lib/libhttpclient.a: $(HTTPCLIENT_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libhttpclient.lib

$(BUILDDIR)/lib/libhttpclient.lib: $(HTTPCLIENT_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
