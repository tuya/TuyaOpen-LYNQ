SUBSYS_G726_DIR  := $(TOP)/PLAT/subsys/media_basic/audio/codec/g726
CFLAGS_INC += -I $(SUBSYS_G726_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_G726_ENABLE

SUBSYS_G726_SRC_DIRS += $(SUBSYS_G726_DIR)/src

SUBSYS_G726_EXCLUDE_FILES :=

SUBSYS_G726_CSRC = $(foreach dir, $(SUBSYS_G726_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_G726_CFILES = $(filter-out $(SUBSYS_G726_EXCLUDE_FILES), $(SUBSYS_G726_CSRC))
SUBSYS_G726_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_G726_CFILES))
SUBSYS_G726_COBJSTEMP :=$(subst $(SUBSYS_G726_DIR),PLAT/subsys/media_basic/audio/codec/g726,$(SUBSYS_G726_COBJSTEMP))
SUBSYS_G726_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_G726_COBJSTEMP))
SUBSYS_G726_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_G726_COBJS))
PPFILES += $(SUBSYS_G726_PPFILES)


ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_G726_COBJS:.o=.d)
endif
-include $(SUBSYS_G726_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_g726.a

$(BUILDDIR)/lib/libsubsys_g726.a: $(SUBSYS_G726_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_g726.lib

$(BUILDDIR)/lib/libsubsys_g726.lib: $(SUBSYS_G726_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
