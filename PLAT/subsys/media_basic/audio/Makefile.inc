SUBSYS_AUDIO_DIR  := $(TOP)/PLAT/subsys/media_basic/audio
CFLAGS_INC += -I $(SUBSYS_AUDIO_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_AUDIO_ENABLE \
          -DFEATURE_SUBSYS_RECORD_ENABLE

ifeq ($(SUBSYS_CODEC_ENABLE),y)
include $(SUBSYS_AUDIO_DIR)/codec/Makefile.inc
endif
ifeq ($(SUBSYS_TTS_ENABLE),y)
include $(SUBSYS_AUDIO_DIR)/tts/Makefile.inc
endif

SUBSYS_AUDIO_SRC_DIRS += $(SUBSYS_AUDIO_DIR)/src

SUBSYS_AUDIO_EXCLUDE_FILES :=

SUBSYS_AUDIO_CSRC = $(foreach dir, $(SUBSYS_AUDIO_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_AUDIO_CFILES = $(filter-out $(SUBSYS_AUDIO_EXCLUDE_FILES), $(SUBSYS_AUDIO_CSRC))
SUBSYS_AUDIO_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_AUDIO_CFILES))
SUBSYS_AUDIO_COBJSTEMP :=$(subst $(SUBSYS_AUDIO_DIR),PLAT/subsys/media_basic/audio,$(SUBSYS_AUDIO_COBJSTEMP))
SUBSYS_AUDIO_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_AUDIO_COBJSTEMP))
SUBSYS_AUDIO_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_AUDIO_COBJS))
PPFILES += $(SUBSYS_AUDIO_PPFILES)


ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_AUDIO_COBJS:.o=.d)
endif
-include $(SUBSYS_AUDIO_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_audio.a

$(BUILDDIR)/lib/libsubsys_audio.a: $(SUBSYS_AUDIO_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_audio.lib

$(BUILDDIR)/lib/libsubsys_audio.lib: $(SUBSYS_AUDIO_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
