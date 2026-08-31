SUBSYS_INPUT_DIR  := $(TOP)/PLAT/subsys/apphub/input
CFLAGS_INC += -I $(SUBSYS_INPUT_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_INPUT_ENABLE

SUBSYS_INPUT_SRC_DIRS += $(SUBSYS_INPUT_DIR)/src

SUBSYS_INPUT_EXCLUDE_FILES :=

SUBSYS_INPUT_CSRC = $(foreach dir, $(SUBSYS_INPUT_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_INPUT_CFILES = $(filter-out $(SUBSYS_INPUT_EXCLUDE_FILES), $(SUBSYS_INPUT_CSRC))
SUBSYS_INPUT_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_INPUT_CFILES))
SUBSYS_INPUT_COBJSTEMP :=$(subst $(SUBSYS_INPUT_DIR),PLAT/subsys/apphub/input,$(SUBSYS_INPUT_COBJSTEMP))
SUBSYS_INPUT_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_INPUT_COBJSTEMP))
SUBSYS_INPUT_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_INPUT_COBJS))
PPFILES += $(SUBSYS_INPUT_PPFILES)


ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_INPUT_COBJS:.o=.d)
endif
-include $(SUBSYS_INPUT_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_input.a

$(BUILDDIR)/lib/libsubsys_input.a: $(SUBSYS_INPUT_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_input.lib

$(BUILDDIR)/lib/libsubsys_input.lib: $(SUBSYS_INPUT_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
