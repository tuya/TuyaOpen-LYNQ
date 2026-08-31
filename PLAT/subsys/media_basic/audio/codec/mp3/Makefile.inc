SUBSYS_MP3_DIR  := $(TOP)/PLAT/subsys/media_basic/audio/codec/mp3
CFLAGS_INC += -I $(SUBSYS_MP3_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_MP3_ENABLE

ifeq ($(SUBSYS_SAMPLE_8TO48_16TO48_ENABLE),y)
CFLAGS += -DFEATURE_SUBSYS_SAMPLE_8TO48_16TO48_ENABLE
endif
ifeq ($(SUBSYS_CHANNEL_1TO2_ENABLE),y)
CFLAGS += -DFEATURE_SUBSYS_CHANNEL_1TO2_ENABLE
endif

include $(SUBSYS_MP3_DIR)/lib/Makefile.inc
CFLAGS_INC += -I $(SUBSYS_MP3_DIR)/lib/libmad-0.15.1b \
              -I $(SUBSYS_MP3_DIR)/lib/libmad-0.15.1b/msvc++

SUBSYS_MP3_SRC_DIRS += $(SUBSYS_MP3_DIR)/src

SUBSYS_MP3_EXCLUDE_FILES :=

SUBSYS_MP3_CSRC = $(foreach dir, $(SUBSYS_MP3_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_MP3_CFILES = $(filter-out $(SUBSYS_MP3_EXCLUDE_FILES), $(SUBSYS_MP3_CSRC))
SUBSYS_MP3_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_MP3_CFILES))
SUBSYS_MP3_COBJSTEMP :=$(subst $(SUBSYS_MP3_DIR),PLAT/subsys/media_basic/audio/codec/mp3,$(SUBSYS_MP3_COBJSTEMP))
SUBSYS_MP3_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_MP3_COBJSTEMP))
SUBSYS_MP3_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_MP3_COBJS))
PPFILES += $(SUBSYS_MP3_PPFILES)


ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_MP3_COBJS:.o=.d)
endif
-include $(SUBSYS_MP3_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_mp3.a

$(BUILDDIR)/lib/libsubsys_mp3.a: $(SUBSYS_MP3_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_mp3.lib

$(BUILDDIR)/lib/libsubsys_mp3.lib: $(SUBSYS_MP3_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
