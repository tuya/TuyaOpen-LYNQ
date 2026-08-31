
TTS_DIR := $(TOP)/PLAT/open_source/tts
TTS_REL_DIR := PLAT/open_source/tts

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/inc \
              -I $(TOP)/PLAT/os/freertos/CMSIS/inc \
              -I $(TTS_DIR)/inc \
              -I $(TTS_DIR)

CFLAGS += -DMBTK_OPENCPU_SUPPORT_TTS

TTS_SRC_DIR += $(TTS_DIR)
TTS_EXCLUDE_SRCS :=

TTS_SRCS = $(foreach dir, $(TTS_SRC_DIR), $(wildcard $(dir)/*.c))
TTS_CFILES = $(filter-out $(TTS_EXCLUDE_SRCS), $(TTS_SRCS))
TTS_OBJSTEMP := $(patsubst %.c, %.o, $(TTS_CFILES))
TTS_OBJSTEMP := $(subst $(TTS_DIR),$(TTS_REL_DIR),$(TTS_OBJSTEMP))
TTS_OBJS     := $(addprefix $(BUILDDIR)/, $(TTS_OBJSTEMP))
TTS_PPFILES  := $(patsubst %.o, %.pp, $(TTS_OBJS))
PPFILES += $(TTS_PPFILES)

ifneq ($(MAKECMDGOALS), build-unilog)
-include $(TTS_OBJS:.o=.d)
endif
-include $(TTS_OBJS:.o=.dd)

PREBUILDLIBS += $(TOP)/PLAT/open_source/tts/lib/libaisound50.a

ifeq ($(TOOLCHAIN),GCC)

lib-y += libtts.a

$(BUILDDIR)/lib/libtts.a: $(TTS_OBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif
