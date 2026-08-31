SUBSYS_FEATURES_DIR  := $(TOP)/PLAT/subsys/apphub/features

CFLAGS_INC += 	-I $(SUBSYS_FEATURES_DIR)/inc			

CFLAGS += -DFEATURE_SUBSYS_FEATURES_ENABLE

SUBSYS_FEATURES_SRC_DIRS := $(SUBSYS_FEATURES_DIR)/src

SUBSYS_FEATURES_EXCLUDE_FILES :=

SUBSYS_FEATURES_CSRC = $(foreach dir, $(SUBSYS_FEATURES_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_FEATURES_CFILES = $(filter-out $(SUBSYS_FEATURES_EXCLUDE_FILES), $(SUBSYS_FEATURES_CSRC))
SUBSYS_FEATURES_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_FEATURES_CFILES))
SUBSYS_FEATURES_COBJSTEMP :=$(subst $(SUBSYS_FEATURES_DIR),PLAT/subsys/apphub/features,$(SUBSYS_FEATURES_COBJSTEMP))
SUBSYS_FEATURES_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_FEATURES_COBJSTEMP))
SUBSYS_FEATURES_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_FEATURES_COBJS))
PPFILES += $(SUBSYS_FEATURES_PPFILES)

ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_FEATURES_COBJS:.o=.d)
endif
-include $(SUBSYS_FEATURES_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_features.a

$(BUILDDIR)/lib/libsubsys_features.a: $(SUBSYS_FEATURES_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_features.lib

$(BUILDDIR)/lib/libsubsys_features.lib: $(SUBSYS_FEATURES_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
