SUBSYS_FPUI_DIR  := $(TOP)/PLAT/subsys/apphub/fpui

CFLAGS_INC += 	-I $(SUBSYS_FPUI_DIR)/inc			\
				-I $(SUBSYS_FPUI_DIR)/gui 				

CFLAGS += -DFEATURE_SUBSYS_FPUI_ENABLE

SUBSYS_FPUI_SRC_DIRS := $(SUBSYS_FPUI_DIR)/src		\
			$(SUBSYS_FPUI_DIR)/gui/fonts			\
			$(SUBSYS_FPUI_DIR)/gui/screens			\
			$(SUBSYS_FPUI_DIR)/gui			

SUBSYS_FPUI_EXCLUDE_FILES :=

SUBSYS_FPUI_CSRC = $(foreach dir, $(SUBSYS_FPUI_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_FPUI_CFILES = $(filter-out $(SUBSYS_FPUI_EXCLUDE_FILES), $(SUBSYS_FPUI_CSRC))
SUBSYS_FPUI_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_FPUI_CFILES))
SUBSYS_FPUI_COBJSTEMP :=$(subst $(SUBSYS_FPUI_DIR),PLAT/subsys/apphub/fpui,$(SUBSYS_FPUI_COBJSTEMP))
SUBSYS_FPUI_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_FPUI_COBJSTEMP))
SUBSYS_FPUI_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_FPUI_COBJS))
PPFILES += $(SUBSYS_FPUI_PPFILES)

ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_FPUI_COBJS:.o=.d)
endif
-include $(SUBSYS_FPUI_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_fpui.a

$(BUILDDIR)/lib/libsubsys_fpui.a: $(SUBSYS_FPUI_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_fpui.lib

$(BUILDDIR)/lib/libsubsys_fpui.lib: $(SUBSYS_FPUI_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
