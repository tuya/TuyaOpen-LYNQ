SUBSYS_MISC_DIR  := $(TOP)/PLAT/subsys/apphub/misc
CFLAGS_INC += -I $(SUBSYS_MISC_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_MISC_ENABLE

SUBSYS_MISC_SRC_DIRS += $(SUBSYS_MISC_DIR)/src

SUBSYS_MISC_EXCLUDE_FILES :=

SUBSYS_MISC_CSRC = $(foreach dir, $(SUBSYS_MISC_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_MISC_CFILES = $(filter-out $(SUBSYS_MISC_EXCLUDE_FILES), $(SUBSYS_MISC_CSRC))
SUBSYS_MISC_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_MISC_CFILES))
SUBSYS_MISC_COBJSTEMP :=$(subst $(SUBSYS_MISC_DIR),PLAT/subsys/apphub/misc,$(SUBSYS_MISC_COBJSTEMP))
SUBSYS_MISC_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_MISC_COBJSTEMP))
SUBSYS_MISC_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_MISC_COBJS))
PPFILES += $(SUBSYS_MISC_PPFILES)


ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_MISC_COBJS:.o=.d)
endif
-include $(SUBSYS_MISC_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_misc.a

$(BUILDDIR)/lib/libsubsys_misc.a: $(SUBSYS_MISC_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_misc.lib

$(BUILDDIR)/lib/libsubsys_misc.lib: $(SUBSYS_MISC_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
