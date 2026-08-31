SUBSYS_PCM_DIR  := $(TOP)/PLAT/subsys/media/audio/codec/pcm
CFLAGS_INC += -I $(SUBSYS_PCM_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_MED_PCM_ENABLE

SUBSYS_PCM_SRC_DIRS += $(SUBSYS_PCM_DIR)/src

SUBSYS_PCM_EXCLUDE_FILES :=

SUBSYS_PCM_CSRC = $(foreach dir, $(SUBSYS_PCM_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_PCM_CFILES = $(filter-out $(SUBSYS_PCM_EXCLUDE_FILES), $(SUBSYS_PCM_CSRC))
SUBSYS_PCM_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_PCM_CFILES))
SUBSYS_PCM_COBJSTEMP :=$(subst $(SUBSYS_PCM_DIR),PLAT/subsys/media/audio/codec/pcm,$(SUBSYS_PCM_COBJSTEMP))
SUBSYS_PCM_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_PCM_COBJSTEMP))
SUBSYS_PCM_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_PCM_COBJS))
PPFILES += $(SUBSYS_PCM_PPFILES)


ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_PCM_COBJS:.o=.d)
endif
-include $(SUBSYS_PCM_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_pcm.a

$(BUILDDIR)/lib/libsubsys_pcm.a: $(SUBSYS_PCM_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_pcm.lib

$(BUILDDIR)/lib/libsubsys_pcm.lib: $(SUBSYS_PCM_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
