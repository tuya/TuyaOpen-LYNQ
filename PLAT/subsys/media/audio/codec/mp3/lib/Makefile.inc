MP3_DIR := $(TOP)/PLAT/subsys/media/audio/codec/mp3/lib

CFLAGS_INC += -I $(MP3_DIR)/libmad-0.15.1b/msvc++ \
              -I $(MP3_DIR)/libmad-0.15.1b

MP3_SRC_DIRS += $(MP3_DIR)/libmad-0.15.1b \
                $(MP3_DIR)

CFLAGS += -DFEATURE_MP3_ENABLED

MP3_EXCLUDE_FILES :=
MP3_CSRC = $(foreach dir, $(MP3_SRC_DIRS), $(wildcard $(dir)/*.c))
MP3_CFILES = $(filter-out $(MP3_EXCLUDE_FILES), $(MP3_CSRC))
MP3_COBJSTEMP := $(patsubst %.c, %.o, $(MP3_CFILES))
MP3_COBJSTEMP :=$(subst $(MP3_DIR),PLAT/subsys/media/audio/codec/mp3/lib,$(MP3_COBJSTEMP))
MP3_COBJS := $(addprefix $(BUILDDIR)/, $(MP3_COBJSTEMP))
MP3_PPFILES := $(patsubst %.o, %.pp, $(MP3_COBJS))
PPFILES += $(MP3_PPFILES)

ifneq ($(MAKECMDGOALS), unilog)
-include $(MP3_COBJS:.o=.d)
endif

-include $(MP3_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)
CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc
lib-y += libmp3.a
$(BUILDDIR)/lib/libmp3.a: $(MP3_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^
endif