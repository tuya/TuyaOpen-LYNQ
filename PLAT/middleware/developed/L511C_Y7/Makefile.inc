#model id
CFLAGS += -DMBTK_MODEL_ID=\"L511C-Y7\"
#hw version
CFLAGS += -DMBTK_HW_VERSION=\"V1\"

buidl_target_DIR := $(TOP)/PLAT/middleware/developed/$(MBTK_PRODUCT_CHIP)
buidl_target_REL_DIR := PLAT/middleware/developed/$(MBTK_PRODUCT_CHIP)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/inc \
              -I $(TOP)/PLAT/os/freertos/CMSIS/inc \
              -I $(TOP)/PLAT/middleware/developed/open_api/inc \
              -I $(buidl_target_DIR)

buidl_target_SRC_DIR += $(buidl_target_DIR)
buidl_target_EXCLUDE_SRCS :=

buidl_target_SRCS = $(foreach dir, $(buidl_target_SRC_DIR), $(wildcard $(dir)/*.c))
buidl_target_CFILES = $(filter-out $(buidl_target_EXCLUDE_SRCS), $(buidl_target_SRCS))
buidl_target_OBJSTEMP := $(patsubst %.c, %.o, $(buidl_target_CFILES))
buidl_target_OBJSTEMP := $(subst $(buidl_target_DIR),$(buidl_target_REL_DIR),$(buidl_target_OBJSTEMP))
buidl_target_OBJS     := $(addprefix $(BUILDDIR)/, $(buidl_target_OBJSTEMP))
buidl_target_PPFILES  := $(patsubst %.o, %.pp, $(buidl_target_OBJS))
PPFILES += $(buidl_target_PPFILES)

ifneq ($(MAKECMDGOALS), build-unilog)
-include $(buidl_target_OBJS:.o=.d)
endif
-include $(buidl_target_OBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

lib-y += libL511C_Y7.a

$(BUILDDIR)/lib/libL511C_Y7.a: $(buidl_target_OBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif