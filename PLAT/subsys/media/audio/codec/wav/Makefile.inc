SUBSYS_WAV_DIR  := $(TOP)/PLAT/subsys/media/audio/codec/wav
CFLAGS_INC += -I $(SUBSYS_WAV_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_MED_WAV_ENABLE

SUBSYS_WAV_SRC_DIRS += $(SUBSYS_WAV_DIR)/src

SUBSYS_WAV_EXCLUDE_FILES :=

SUBSYS_WAV_CSRC = $(foreach dir, $(SUBSYS_WAV_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_WAV_CFILES = $(filter-out $(SUBSYS_WAV_EXCLUDE_FILES), $(SUBSYS_WAV_CSRC))
SUBSYS_WAV_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_WAV_CFILES))
SUBSYS_WAV_COBJSTEMP :=$(subst $(SUBSYS_WAV_DIR),PLAT/subsys/media/audio/codec/wav,$(SUBSYS_WAV_COBJSTEMP))
SUBSYS_WAV_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_WAV_COBJSTEMP))
SUBSYS_WAV_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_WAV_COBJS))
PPFILES += $(SUBSYS_WAV_PPFILES)


ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_WAV_COBJS:.o=.d)
endif
-include $(SUBSYS_WAV_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_wav.a

$(BUILDDIR)/lib/libsubsys_wav.a: $(SUBSYS_WAV_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_wav.lib

$(BUILDDIR)/lib/libsubsys_wav.lib: $(SUBSYS_WAV_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
