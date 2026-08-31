
TESTLIB_DIR := $(TOP)/PLAT/open_source/testlib
TESTLIB_REL_DIR := PLAT/open_source/testlib

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/inc \
              -I $(TOP)/PLAT/os/freertos/CMSIS/inc \
              -I $(TESTLIB_DIR)/inc \
              -I $(TESTLIB_DIR)


TESTLIB_SRC_DIR += $(TESTLIB_DIR)
TESTLIB_EXCLUDE_SRCS :=

TESTLIB_SRCS = $(foreach dir, $(TESTLIB_SRC_DIR), $(wildcard $(dir)/*.c))
TESTLIB_CFILES = $(filter-out $(TESTLIB_EXCLUDE_SRCS), $(TESTLIB_SRCS))
TESTLIB_OBJSTEMP := $(patsubst %.c, %.o, $(TESTLIB_CFILES))
TESTLIB_OBJSTEMP := $(subst $(TESTLIB_DIR),$(TESTLIB_REL_DIR),$(TESTLIB_OBJSTEMP))
TESTLIB_OBJS     := $(addprefix $(BUILDDIR)/, $(TESTLIB_OBJSTEMP))
TESTLIB_PPFILES  := $(patsubst %.o, %.pp, $(TESTLIB_OBJS))
PPFILES += $(TESTLIB_PPFILES)

ifneq ($(MAKECMDGOALS), build-unilog)
-include $(TESTLIB_OBJS:.o=.d)
endif
-include $(TESTLIB_OBJS:.o=.dd)


ifeq ($(TOOLCHAIN),GCC)

lib-y += libtestlib.a

$(BUILDDIR)/lib/libtestlib.a: $(TESTLIB_OBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif
