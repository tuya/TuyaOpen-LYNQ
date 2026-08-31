SUBSYS_OPENSDK_DIR  := $(TOP)/PLAT/subsys/opensdk
CFLAGS_INC += -I $(SUBSYS_OPENSDK_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_OPENSDK_ENABLE

SUBSYS_OPENSDK_SRC_DIRS += $(SUBSYS_OPENSDK_DIR)/src

SUBSYS_OPENSDK_EXCLUDE_FILES :=

SUBSYS_OPENSDK_CSRC = $(foreach dir, $(SUBSYS_OPENSDK_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_OPENSDK_CFILES = $(filter-out $(SUBSYS_OPENSDK_EXCLUDE_FILES), $(SUBSYS_OPENSDK_CSRC))
SUBSYS_OPENSDK_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_OPENSDK_CFILES))
SUBSYS_OPENSDK_COBJSTEMP :=$(subst $(SUBSYS_OPENSDK_DIR),PLAT/subsys/opensdk,$(SUBSYS_OPENSDK_COBJSTEMP))
SUBSYS_OPENSDK_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_OPENSDK_COBJSTEMP))
SUBSYS_OPENSDK_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_OPENSDK_COBJS))
PPFILES += $(SUBSYS_OPENSDK_PPFILES)


ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_OPENSDK_COBJS:.o=.d)
endif
-include $(SUBSYS_OPENSDK_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_opensdk.a

$(BUILDDIR)/lib/libsubsys_opensdk.a: $(SUBSYS_OPENSDK_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_opensdk.lib

$(BUILDDIR)/lib/libsubsys_opensdk.lib: $(SUBSYS_OPENSDK_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
