SUBSYS_MINIZ_DIR  := $(TOP)/PLAT/subsys/zip/miniz

CFLAGS_INC += -I $(SUBSYS_MINIZ_DIR)

CFLAGS += -DFEATURE_SUBSYS_MINIZ_ENABLE

SUBSYS_MINIZ_SRC_DIRS += $(SUBSYS_MINIZ_DIR)

SUBSYS_MINIZ_EXCLUDE_FILES :=

SUBSYS_MINIZ_CSRC = $(foreach dir, $(SUBSYS_MINIZ_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_MINIZ_CFILES = $(filter-out $(SUBSYS_MINIZ_EXCLUDE_FILES), $(SUBSYS_MINIZ_CSRC))
SUBSYS_MINIZ_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_MINIZ_CFILES))
SUBSYS_MINIZ_COBJSTEMP :=$(subst $(SUBSYS_MINIZ_DIR),PLAT/subsys/zip/miniz,$(SUBSYS_MINIZ_COBJSTEMP))
SUBSYS_MINIZ_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_MINIZ_COBJSTEMP))
SUBSYS_MINIZ_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_MINIZ_COBJS))
PPFILES += $(SUBSYS_MINIZ_PPFILES)

ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_MINIZ_COBJS:.o=.d)
endif
-include $(SUBSYS_MINIZ_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_miniz.a

$(BUILDDIR)/lib/libsubsys_miniz.a: $(SUBSYS_MINIZ_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_miniz.lib

$(BUILDDIR)/lib/libsubsys_miniz.lib: $(SUBSYS_MINIZ_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
