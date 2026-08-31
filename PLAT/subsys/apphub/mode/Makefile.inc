SUBSYS_MODE_DIR  := $(TOP)/PLAT/subsys/apphub/mode
CFLAGS_INC += -I $(SUBSYS_MODE_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_MODE_ENABLE

SUBSYS_MODE_SRC_DIRS += $(SUBSYS_MODE_DIR)/src

SUBSYS_MODE_EXCLUDE_FILES :=

SUBSYS_MODE_CSRC = $(foreach dir, $(SUBSYS_MODE_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_MODE_CFILES = $(filter-out $(SUBSYS_MODE_EXCLUDE_FILES), $(SUBSYS_MODE_CSRC))
SUBSYS_MODE_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_MODE_CFILES))
SUBSYS_MODE_COBJSTEMP :=$(subst $(SUBSYS_MODE_DIR),PLAT/subsys/apphub/mode,$(SUBSYS_MODE_COBJSTEMP))
SUBSYS_MODE_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_MODE_COBJSTEMP))
SUBSYS_MODE_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_MODE_COBJS))
PPFILES += $(SUBSYS_MODE_PPFILES)


ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_MODE_COBJS:.o=.d)
endif
-include $(SUBSYS_MODE_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_mode.a

$(BUILDDIR)/lib/libsubsys_mode.a: $(SUBSYS_MODE_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_mode.lib

$(BUILDDIR)/lib/libsubsys_mode.lib: $(SUBSYS_MODE_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
