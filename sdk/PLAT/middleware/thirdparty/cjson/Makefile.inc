CJSON_DIR        := $(TOP)/PLAT/middleware/thirdparty/cjson

CFLAGS_INC     +=  -I $(TOP)/PLAT/middleware/thirdparty/cjson
              

CFLAGS += -DFEATURE_CJSON_ENABLE

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc


$(BUILDDIR)/lib/libcjson.a: $(CJSON_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil


lib-y += libcjson.lib

$(BUILDDIR)/lib/libcjson.lib: $(CJSON_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
