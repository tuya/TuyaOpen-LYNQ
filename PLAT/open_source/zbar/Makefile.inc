
ZBAR_DIR := $(TOP)/PLAT/open_source/zbar
ZBAR_REL_DIR := PLAT/open_source/zbar

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/inc \
              -I $(TOP)/PLAT/os/freertos/CMSIS/inc \
              -I $(ZBAR_DIR)/decoder \
              -I $(ZBAR_DIR)/qrcode \
              -I $(ZBAR_DIR)/inc \
              -I $(ZBAR_DIR)

CFLAGS += -DMBTK_OPENCPU_SUPPORT_ZBAR
CFLAGS += -Wno-parentheses -Wno-logical-not-parentheses -Wno-pointer-sign -Wno-unused-variable

ZBAR_SRC_DIR += $(ZBAR_DIR)/decoder
ZBAR_SRC_DIR += $(ZBAR_DIR)/qrcode
ZBAR_SRC_DIR += $(ZBAR_DIR)/src
ZBAR_SRC_DIR += $(ZBAR_DIR)
ZBAR_EXCLUDE_SRCS :=

ZBAR_SRCS = $(foreach dir, $(ZBAR_SRC_DIR), $(wildcard $(dir)/*.c))
ZBAR_CFILES = $(filter-out $(ZBAR_EXCLUDE_SRCS), $(ZBAR_SRCS))
ZBAR_OBJSTEMP := $(patsubst %.c, %.o, $(ZBAR_CFILES))
ZBAR_OBJSTEMP := $(subst $(ZBAR_DIR),$(ZBAR_REL_DIR),$(ZBAR_OBJSTEMP))
ZBAR_OBJS     := $(addprefix $(BUILDDIR)/, $(ZBAR_OBJSTEMP))
ZBAR_PPFILES  := $(patsubst %.o, %.pp, $(ZBAR_OBJS))
PPFILES += $(ZBAR_PPFILES)

ifneq ($(MAKECMDGOALS), build-unilog)
-include $(ZBAR_OBJS:.o=.d)
endif
-include $(ZBAR_OBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

lib-y += libzbar.a

$(BUILDDIR)/lib/libzbar.a: $(ZBAR_OBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif
