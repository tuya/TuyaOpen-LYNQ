
OPENATCMD_DIR := $(TOP)/PLAT/open_source/atcmd
OPENATCMD_REL_DIR := PLAT/open_source/atcmd

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/inc \
              -I $(TOP)/PLAT/os/freertos/CMSIS/inc \
			  -I $(TOP)/PLAT/middleware/developed/at/atref/inc \
              -I $(OPENATCMD_DIR)/inc \
              -I $(OPENATCMD_DIR)


CFLAGS += -DMBTK_OPENCPU_SUPPORT_ATCMD


OPENATCMD_SRC_DIR += $(OPENATCMD_DIR)
OPENATCMD_EXCLUDE_SRCS :=

OPENATCMD_SRCS = $(foreach dir, $(OPENATCMD_SRC_DIR), $(wildcard $(dir)/*.c))
OPENATCMD_CFILES = $(filter-out $(OPENATCMD_EXCLUDE_SRCS), $(OPENATCMD_SRCS))
OPENATCMD_OBJSTEMP := $(patsubst %.c, %.o, $(OPENATCMD_CFILES))
OPENATCMD_OBJSTEMP := $(subst $(OPENATCMD_DIR),$(OPENATCMD_REL_DIR),$(OPENATCMD_OBJSTEMP))
OPENATCMD_OBJS     := $(addprefix $(BUILDDIR)/, $(OPENATCMD_OBJSTEMP))
OPENATCMD_PPFILES  := $(patsubst %.o, %.pp, $(OPENATCMD_OBJS))
PPFILES += $(OPENATCMD_PPFILES)

ifneq ($(MAKECMDGOALS), build-unilog)
-include $(OPENATCMD_OBJS:.o=.d)
endif
-include $(OPENATCMD_OBJS:.o=.dd)


ifeq ($(TOOLCHAIN),GCC)

lib-y += libopenatcmd.a

$(BUILDDIR)/lib/libopenatcmd.a: $(OPENATCMD_OBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif
