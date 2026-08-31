SUBSYS_FLASHEX_DIR  := $(TOP)/PLAT/subsys/storage/flashex
CFLAGS_INC += -I $(SUBSYS_FLASHEX_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_FLASHEX_ENABLE

SUBSYS_FLASHEX_SRC_DIRS += $(SUBSYS_FLASHEX_DIR)/src

SUBSYS_FLASHEX_EXCLUDE_FILES :=

SUBSYS_FLASHEX_CSRC = $(foreach dir, $(SUBSYS_FLASHEX_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_FLASHEX_CFILES = $(filter-out $(SUBSYS_FLASHEX_EXCLUDE_FILES), $(SUBSYS_FLASHEX_CSRC))
SUBSYS_FLASHEX_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_FLASHEX_CFILES))
SUBSYS_FLASHEX_COBJSTEMP :=$(subst $(SUBSYS_FLASHEX_DIR),PLAT/subsys/storage/flashex,$(SUBSYS_FLASHEX_COBJSTEMP))
SUBSYS_FLASHEX_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_FLASHEX_COBJSTEMP))
SUBSYS_FLASHEX_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_FLASHEX_COBJS))
PPFILES += $(SUBSYS_FLASHEX_PPFILES)

ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_FLASHEX_COBJS:.o=.d)
endif
-include $(SUBSYS_FLASHEX_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_flashex.a

$(BUILDDIR)/lib/libsubsys_flashex.a: $(SUBSYS_FLASHEX_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_flashex.lib

$(BUILDDIR)/lib/libsubsys_flashex.lib: $(SUBSYS_FLASHEX_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
