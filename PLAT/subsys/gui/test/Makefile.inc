SUBSYS_UI_TEST_DIR  := $(TOP)/PLAT/subsys/gui/test

CFLAGS_INC += -I $(SUBSYS_UI_TEST_DIR)				\
			-I $(SUBSYS_UI_TEST_DIR)/status_bar 	\
			-I $(SUBSYS_UI_TEST_DIR)/page 			

SUBSYS_UI_TEST_SRC_DIRS += $(SUBSYS_UI_TEST_DIR)	\
			$(SUBSYS_UI_TEST_DIR)/status_bar		\
			$(SUBSYS_UI_TEST_DIR)/page				

CFLAGS += -DFEATURE_SUBSYS_GUI_TEST_ENABLE

SUBSYS_UI_TEST_EXCLUDE_FILES :=

SUBSYS_UI_TEST_CSRC = $(foreach dir, $(SUBSYS_UI_TEST_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_UI_TEST_CFILES = $(filter-out $(SUBSYS_UI_TEST_EXCLUDE_FILES), $(SUBSYS_UI_TEST_CSRC))
SUBSYS_UI_TEST_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_UI_TEST_CFILES))
SUBSYS_UI_TEST_COBJSTEMP :=$(subst $(SUBSYS_UI_TEST_DIR),PLAT/subsys/gui/test,$(SUBSYS_UI_TEST_COBJSTEMP))
SUBSYS_UI_TEST_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_UI_TEST_COBJSTEMP))
SUBSYS_UI_TEST_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_UI_TEST_COBJS))
PPFILES += $(SUBSYS_UI_TEST_PPFILES)


ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_UI_TEST_COBJS:.o=.d)
endif
-include $(SUBSYS_UI_TEST_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubui_test.a

$(BUILDDIR)/lib/libsubui_test.a: $(SUBSYS_UI_TEST_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubui_test.lib

$(BUILDDIR)/lib/libsubui_test.lib: $(SUBSYS_UI_TEST_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
