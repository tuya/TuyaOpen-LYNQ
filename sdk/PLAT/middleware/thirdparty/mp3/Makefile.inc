MP3_DIR := $(TOP)/PLAT/middleware/thirdparty/mp3

CFLAGS_INC += -I $(TOP)/PLAT/middleware/thirdparty/mp3/libmad-0.15.1b/msvc++ \
              -I $(TOP)/PLAT/middleware/thirdparty/mp3/libmad-0.15.1b

MP3_SRC_DIRS += $(MP3_DIR)/libmad-0.15.1b \
                $(MP3_DIR)

CFLAGS += -DFEATURE_MP3_ENABLED

MP3_EXCLUDE_FILES :=

ifeq ($(TOOLCHAIN),GCC)
CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

$(BUILDDIR)/lib/libmp3.a: $(MP3_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^
endif