SUBSYS_COMMON_DIR  := $(TOP)/PLAT/subsys/common
CFLAGS_INC += -I $(SUBSYS_COMMON_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_COMMON_ENABLE


SUBSYS_COMMON_SRC_DIRS += $(SUBSYS_COMMON_DIR)/src

SUBSYS_COMMON_EXCLUDE_FILES :=

SUBSYS_COMMON_CSRC = $(foreach dir, $(SUBSYS_COMMON_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_COMMON_CFILES = $(filter-out $(SUBSYS_COMMON_EXCLUDE_FILES), $(SUBSYS_COMMON_CSRC))
SUBSYS_COMMON_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_COMMON_CFILES))
SUBSYS_COMMON_COBJSTEMP :=$(subst $(SUBSYS_COMMON_DIR),PLAT/subsys/common,$(SUBSYS_COMMON_COBJSTEMP))
SUBSYS_COMMON_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_COMMON_COBJSTEMP))
SUBSYS_COMMON_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_COMMON_COBJS))
PPFILES += $(SUBSYS_COMMON_PPFILES)


ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_COMMON_COBJS:.o=.d)
endif
-include $(SUBSYS_COMMON_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_common.a

$(BUILDDIR)/lib/libsubsys_common.a: $(SUBSYS_COMMON_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_common.lib

$(BUILDDIR)/lib/libsubsys_common.lib: $(SUBSYS_CAMERA_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^
endif
