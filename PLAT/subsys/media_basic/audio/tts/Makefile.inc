SUBSYS_TTS_DIR  := $(TOP)/PLAT/subsys/media_basic/audio/tts
CFLAGS_INC += -I $(SUBSYS_TTS_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_TTS_ENABLE

CFLAGS_INC += -I $(SUBSYS_TTS_DIR)/lib/inc
PREBUILDLIBS += $(SUBSYS_TTS_DIR)/lib/libaisound50.a

SUBSYS_TTS_SRC_DIRS += $(SUBSYS_TTS_DIR)/src

SUBSYS_TTS_EXCLUDE_FILES :=

SUBSYS_TTS_CSRC = $(foreach dir, $(SUBSYS_TTS_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_TTS_CFILES = $(filter-out $(SUBSYS_TTS_EXCLUDE_FILES), $(SUBSYS_TTS_CSRC))
SUBSYS_TTS_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_TTS_CFILES))
SUBSYS_TTS_COBJSTEMP :=$(subst $(SUBSYS_TTS_DIR),PLAT/subsys/media_basic/audio/tts,$(SUBSYS_TTS_COBJSTEMP))
SUBSYS_TTS_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_TTS_COBJSTEMP))
SUBSYS_TTS_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_TTS_COBJS))
PPFILES += $(SUBSYS_TTS_PPFILES)


ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_TTS_COBJS:.o=.d)
endif
-include $(SUBSYS_TTS_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_tts.a

$(BUILDDIR)/lib/libsubsys_tts.a: $(SUBSYS_TTS_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_tts.lib

$(BUILDDIR)/lib/libsubsys_tts.lib: $(SUBSYS_TTS_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
