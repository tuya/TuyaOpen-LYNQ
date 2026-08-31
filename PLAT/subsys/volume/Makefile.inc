SUBSYS_VOLUME_DIR  := $(TOP)/PLAT/subsys/volume
CFLAGS_INC += -I $(SUBSYS_VOLUME_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_VOLUME_ENABLE

SUBSYS_VOLUME_SRC_DIRS += $(SUBSYS_VOLUME_DIR)/src

SUBSYS_VOLUME_EXCLUDE_FILES :=

SUBSYS_VOLUME_CSRC = $(foreach dir, $(SUBSYS_VOLUME_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_VOLUME_CFILES = $(filter-out $(SUBSYS_VOLUME_EXCLUDE_FILES), $(SUBSYS_VOLUME_CSRC))
SUBSYS_VOLUME_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_VOLUME_CFILES))
SUBSYS_VOLUME_COBJSTEMP :=$(subst $(SUBSYS_VOLUME_DIR),PLAT/subsys/volume,$(SUBSYS_VOLUME_COBJSTEMP))
SUBSYS_VOLUME_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_VOLUME_COBJSTEMP))
SUBSYS_VOLUME_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_VOLUME_COBJS))
PPFILES += $(SUBSYS_VOLUME_PPFILES)

ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_VOLUME_COBJS:.o=.d)
endif
-include $(SUBSYS_VOLUME_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_volume.a

$(BUILDDIR)/lib/libsubsys_volume.a: $(SUBSYS_VOLUME_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_volume.lib

$(BUILDDIR)/lib/libsubsys_volume.lib: $(SUBSYS_VOLUME_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
