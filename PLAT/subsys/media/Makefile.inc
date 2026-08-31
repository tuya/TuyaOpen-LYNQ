SUBSYS_MEDIA_DIR  := $(TOP)/PLAT/subsys/media
CFLAGS_INC += -I $(SUBSYS_MEDIA_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_MEDIA_ENABLE

ifeq ($(SUBSYS_MEDIA_STREAM_ENABLE),y)
CFLAGS += -DFEATURE_SUBSYS_MEDIA_STREAM_ENABLE
endif

ifeq ($(SUBSYS_AUDIO_ENABLE),y)
include $(SUBSYS_MEDIA_DIR)/audio/Makefile.inc
endif

ifeq ($(SUBSYS_VIDEO_ENABLE),y)
include $(SUBSYS_MEDIA_DIR)/video/Makefile.inc
endif

SUBSYS_MEDIA_SRC_DIRS += $(SUBSYS_MEDIA_DIR)/src

SUBSYS_MEDIA_EXCLUDE_FILES :=

SUBSYS_MEDIA_CSRC = $(foreach dir, $(SUBSYS_MEDIA_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_MEDIA_CFILES = $(filter-out $(SUBSYS_MEDIA_EXCLUDE_FILES), $(SUBSYS_MEDIA_CSRC))
SUBSYS_MEDIA_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_MEDIA_CFILES))
SUBSYS_MEDIA_COBJSTEMP :=$(subst $(SUBSYS_MEDIA_DIR),PLAT/subsys/media,$(SUBSYS_MEDIA_COBJSTEMP))
SUBSYS_MEDIA_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_MEDIA_COBJSTEMP))
SUBSYS_MEDIA_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_MEDIA_COBJS))
PPFILES += $(SUBSYS_MEDIA_PPFILES)


ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_MEDIA_COBJS:.o=.d)
endif
-include $(SUBSYS_MEDIA_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_media.a

$(BUILDDIR)/lib/libsubsys_media.a: $(SUBSYS_MEDIA_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_media.lib

$(BUILDDIR)/lib/libsubsys_media.lib: $(SUBSYS_MEDIA_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
