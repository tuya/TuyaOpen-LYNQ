SUBSYS_CODEC_DIR  := $(TOP)/PLAT/subsys/media/audio/codec
CFLAGS_INC += -I $(SUBSYS_CODEC_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_CODEC_ENABLE

ifeq ($(SUBSYS_MP3_ENABLE),y)
include $(SUBSYS_CODEC_DIR)/mp3/Makefile.inc
endif
ifeq ($(SUBSYS_WAV_ENABLE),y)
include $(SUBSYS_CODEC_DIR)/wav/Makefile.inc
endif
ifneq ($(SUBSYS_AMR_ENABLE)_$(SUBSYS_AMR_RECORD_ENABLE),n_n)
include $(SUBSYS_CODEC_DIR)/amr/Makefile.inc
endif
ifeq ($(SUBSYS_PCM_ENABLE),y)
include $(SUBSYS_CODEC_DIR)/pcm/Makefile.inc
endif
ifeq ($(SUBSYS_G726_ENABLE),y)
include $(SUBSYS_CODEC_DIR)/g726/Makefile.inc
endif
ifeq ($(SUBSYS_ADPCM_ENABLE),y)
include $(SUBSYS_CODEC_DIR)/adpcm/Makefile.inc
endif
ifeq ($(SUBSYS_G711_ENABLE),y)
include $(SUBSYS_CODEC_DIR)/g711/Makefile.inc
endif

SUBSYS_CODEC_SRC_DIRS += $(SUBSYS_CODEC_DIR)/src

SUBSYS_CODEC_EXCLUDE_FILES :=

SUBSYS_CODEC_CSRC = $(foreach dir, $(SUBSYS_CODEC_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_CODEC_CFILES = $(filter-out $(SUBSYS_CODEC_EXCLUDE_FILES), $(SUBSYS_CODEC_CSRC))
SUBSYS_CODEC_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_CODEC_CFILES))
SUBSYS_CODEC_COBJSTEMP :=$(subst $(SUBSYS_CODEC_DIR),PLAT/subsys/media/audio/codec,$(SUBSYS_CODEC_COBJSTEMP))
SUBSYS_CODEC_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_CODEC_COBJSTEMP))
SUBSYS_CODEC_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_CODEC_COBJS))
PPFILES += $(SUBSYS_CODEC_PPFILES)


ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_CODEC_COBJS:.o=.d)
endif
-include $(SUBSYS_CODEC_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_codec.a

$(BUILDDIR)/lib/libsubsys_codec.a: $(SUBSYS_CODEC_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_codec.lib

$(BUILDDIR)/lib/libsubsys_codec.lib: $(SUBSYS_CODEC_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
