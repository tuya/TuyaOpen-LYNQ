SUBSYS_DISPLAY_DIR  := $(TOP)/PLAT/subsys/display
CFLAGS_INC += -I $(SUBSYS_DISPLAY_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_DISPLAY_ENABLE


SUBSYS_DISPLAY_SRC_DIRS += $(SUBSYS_DISPLAY_DIR)/src

SUBSYS_DISPLAY_EXCLUDE_FILES :=

SUBSYS_DISPLAY_CSRC = $(foreach dir, $(SUBSYS_DISPLAY_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_DISPLAY_CFILES = $(filter-out $(SUBSYS_DISPLAY_EXCLUDE_FILES), $(SUBSYS_DISPLAY_CSRC))
SUBSYS_DISPLAY_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_DISPLAY_CFILES))
SUBSYS_DISPLAY_COBJSTEMP :=$(subst $(SUBSYS_DISPLAY_DIR),PLAT/subsys/display,$(SUBSYS_DISPLAY_COBJSTEMP))
SUBSYS_DISPLAY_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_DISPLAY_COBJSTEMP))
SUBSYS_DISPLAY_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_DISPLAY_COBJS))
PPFILES += $(SUBSYS_DISPLAY_PPFILES)


ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_DISPLAY_COBJS:.o=.d)
endif
-include $(SUBSYS_DISPLAY_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_display.a

$(BUILDDIR)/lib/libsubsys_display.a: $(SUBSYS_DISPLAY_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_display.lib

$(BUILDDIR)/lib/libsubsys_display.lib: $(SUBSYS_CAMERA_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^
endif
