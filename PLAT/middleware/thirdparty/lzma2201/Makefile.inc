
LZMA_DIR      := $(TOP)/PLAT/middleware/thirdparty/lzma2201/C
CFLAGS_INC    += -I $(TOP)/PLAT/os/freertos/inc               \
                  -I $(TOP)/PLAT/os/freertos/CMSIS/inc        \
                  -I $(TOP)/PLAT/middleware/thirdparty/cjson  \
                  -I $(LZMA_DIR) \
				  -I $(LZMA_DIR)/wrapper
              

LZMA_SRC_DIRS += $(LZMA_DIR) \
                 $(LZMA_DIR)/wrapper


LZMA_EXCLUDE_FILES :=

LZMA_CSRC = $(foreach dir, $(LZMA_SRC_DIRS), $(wildcard $(dir)/*.c))
LZMA_CFILES = $(filter-out $(LZMA_EXCLUDE_FILES), $(LZMA_CSRC))
LZMA_COBJSTEMP := $(patsubst %.c, %.o, $(LZMA_CFILES))
LZMA_COBJSTEMP :=$(subst $(LZMA_DIR),PLAT/middleware/thirdparty/lzma2201/C,$(LZMA_COBJSTEMP))
LZMA_COBJS := $(addprefix $(BUILDDIR)/, $(LZMA_COBJSTEMP))
LZMA_PPFILES := $(patsubst %.o, %.pp, $(LZMA_COBJS))
PPFILES += $(LZMA_PPFILES)

CFLAGS += -DFEATURE_LZMA_ENABLE

ifneq ($(MAKECMDGOALS), unilog)
-include $(LZMA_COBJS:.o=.d)
endif
-include $(LZMA_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += liblzma.a

$(BUILDDIR)/lib/liblzma.a: $(LZMA_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += liblzma.lib

$(BUILDDIR)/lib/liblzma.lib: $(LZMA_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
