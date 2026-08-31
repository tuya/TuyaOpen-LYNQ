SUBSYS_G711_DIR  := $(TOP)/PLAT/subsys/media/audio/codec/g711
CFLAGS_INC += -I $(SUBSYS_G711_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_G711_ENABLE

SUBSYS_G711_SRC_DIRS += $(SUBSYS_G711_DIR)/src

SUBSYS_G711_EXCLUDE_FILES :=

SUBSYS_G711_CSRC = $(foreach dir, $(SUBSYS_G711_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_G711_CFILES = $(filter-out $(SUBSYS_G711_EXCLUDE_FILES), $(SUBSYS_G711_CSRC))
SUBSYS_G711_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_G711_CFILES))
SUBSYS_G711_COBJSTEMP :=$(subst $(SUBSYS_G711_DIR),PLAT/subsys/media/audio/codec/g711,$(SUBSYS_G711_COBJSTEMP))
SUBSYS_G711_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_G711_COBJSTEMP))
SUBSYS_G711_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_G711_COBJS))
PPFILES += $(SUBSYS_G711_PPFILES)


ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_G711_COBJS:.o=.d)
endif
-include $(SUBSYS_G711_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_g711.a

$(BUILDDIR)/lib/libsubsys_g711.a: $(SUBSYS_G711_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_g711.lib

$(BUILDDIR)/lib/libsubsys_g711.lib: $(SUBSYS_G711_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
