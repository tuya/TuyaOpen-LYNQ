
YRCOMPRESS_DIR      := $(TOP)/PLAT/middleware/developed/yrcompress
CFLAGS_INC    += -I $(TOP)/PLAT/os/freertos/inc               \
                  -I $(TOP)/PLAT/os/freertos/CMSIS/inc        \
                  -I $(TOP)/PLAT/middleware/thirdparty/cjson  \
                  -I $(YRCOMPRESS_DIR)
              
CFLAGS += -DFEATURE_YRCOMPRESS_ENABLE

YRCOMPRESS_SRC_DIRS += $(YRCOMPRESS_DIR)

YRCOMPRESS_EXCLUDE_FILES :=


ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc


$(BUILDDIR)/lib/libyrcompress.a: $(YRCOMPRESS_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libyrcompress.lib

$(BUILDDIR)/lib/libyrcompress.lib: $(YRCOMPRESS_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
