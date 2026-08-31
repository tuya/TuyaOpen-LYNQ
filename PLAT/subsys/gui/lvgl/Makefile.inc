LVGL_DIR        := $(TOP)/PLAT/subsys/gui/lvgl

CFLAGS_INC     +=  -I $(LVGL_DIR)

LVGL_SRC_DIRS += $(LVGL_DIR)	\
	$(LVGL_DIR)/demos			\
	$(LVGL_DIR)/demos/*			\
	$(LVGL_DIR)/demos/*/*		\
	$(LVGL_DIR)/src/core		\
	$(LVGL_DIR)/src/draw		\
	$(LVGL_DIR)/src/draw/*		\
	$(LVGL_DIR)/src/extra		\
	$(LVGL_DIR)/src/extra/*		\
	$(LVGL_DIR)/src/extra/*/*	\
	$(LVGL_DIR)/src/font		\
	$(LVGL_DIR)/src/hal			\
	$(LVGL_DIR)/src/misc		\
	$(LVGL_DIR)/src/widgets

LVGL_EXCLUDE_FILES :=
LVGL_CSRC = $(foreach dir, $(LVGL_SRC_DIRS), $(wildcard $(dir)/*.c))
LVGL_CFILES = $(filter-out $(LVGL_EXCLUDE_FILES), $(LVGL_CSRC))

LVGL_COBJSTEMP := $(patsubst %.c, %.o, $(LVGL_CFILES))
LVGL_COBJSTEMP :=$(subst $(LVGL_DIR),PLAT/subsys/gui/lvgl,$(LVGL_COBJSTEMP))
LVGL_COBJS := $(addprefix $(BUILDDIR)/, $(LVGL_COBJSTEMP))

LVGL_PPFILES := $(patsubst %.o, %.pp, $(LVGL_COBJS))
PPFILES += $(LVGL_PPFILES)

ifneq ($(MAKECMDGOALS), unilog)
-include $(LVGL_COBJS:.o=.d)
endif
-include $(LVGL_COBJS:.o=.dd)

CFLAGS += -DFEATURE_SUBSYS_GUI_LVGL_ENABLE
CFLAGS += -DLV_LVGL_H_INCLUDE_SIMPLE

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += liblvgl.a

$(BUILDDIR)/lib/liblvgl.a: $(LVGL_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil


lib-y += liblvgl.lib

$(BUILDDIR)/lib/liblvgl.lib: $(LVGL_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
