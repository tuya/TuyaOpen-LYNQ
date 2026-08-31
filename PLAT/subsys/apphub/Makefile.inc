SUBSYS_APPHUB_DIR  := $(TOP)/PLAT/subsys/apphub

CFLAGS_INC += -I $(SUBSYS_APPHUB_DIR)/inc

SUBSYS_APPHUB_SRC_DIRS += 	$(SUBSYS_APPHUB_DIR)/src

CFLAGS += -DFEATURE_SUBSYS_APPHUB_ENABLE

SUBSYS_APPHUB_EXCLUDE_FILES :=

SUBSYS_APPHUB_CSRC = $(foreach dir, $(SUBSYS_APPHUB_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_APPHUB_CFILES = $(filter-out $(SUBSYS_APPHUB_EXCLUDE_FILES), $(SUBSYS_APPHUB_CSRC))
SUBSYS_APPHUB_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_APPHUB_CFILES))
SUBSYS_APPHUB_COBJSTEMP :=$(subst $(SUBSYS_APPHUB_DIR),PLAT/subsys/apphub,$(SUBSYS_APPHUB_COBJSTEMP))
SUBSYS_APPHUB_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_APPHUB_COBJSTEMP))
SUBSYS_APPHUB_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_APPHUB_COBJS))
PPFILES += $(SUBSYS_APPHUB_PPFILES)


ifeq ($(SUBSYS_GUI_LAUNCHER_ENABLE),y)
include $(SUBSYS_APPHUB_DIR)/launcher/Makefile.inc
endif
ifeq ($(SUBSYS_GUI_WATCH_ENABLE),y)
include $(SUBSYS_APPHUB_DIR)/watch/Makefile.inc
endif
ifeq ($(SUBSYS_STATUS_ENABLE),y)
include $(SUBSYS_APPHUB_DIR)/status/Makefile.inc
endif
ifeq ($(SUBSYS_INPUT_ENABLE),y)
include $(SUBSYS_APPHUB_DIR)/input/Makefile.inc
endif
ifeq ($(SUBSYS_MODE_ENABLE),y)
include $(SUBSYS_APPHUB_DIR)/mode/Makefile.inc
endif
ifeq ($(SUBSYS_MISC_ENABLE),y)
include $(SUBSYS_APPHUB_DIR)/misc/Makefile.inc
endif

ifeq ($(SUBSYS_CONSOLE_ENABLE),y)
include $(SUBSYS_APPHUB_DIR)/console/Makefile.inc
endif

ifeq ($(SUBSYS_GUI_FPUI_ENABLE),y)
include $(SUBSYS_APPHUB_DIR)/fpui/Makefile.inc
CFLAGS += -DFEATURE_SUBSYS_GUI_FPUI_ENABLE
endif

ifeq ($(SUBSYS_FEATURES_ENABLE),y)
include $(SUBSYS_APPHUB_DIR)/features/Makefile.inc
endif

ifeq ($(SUBSYS_POWER_MANAGER_ENABLE),y)
CFLAGS += -DFEATURE_SUBSYS_POWER_MANAGER_ENABLE
endif

ifeq ($(SUBSYS_APP_MANAGER_ENABLE),y)
CFLAGS += -DFEATURE_SUBSYS_APP_MANAGER_ENABLE
endif

ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_APPHUB_COBJS:.o=.d)
endif
-include $(SUBSYS_APPHUB_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_apphub.a

$(BUILDDIR)/lib/libsubsys_apphub.a: $(SUBSYS_APPHUB_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_apphub.lib

$(BUILDDIR)/lib/libsubsys_apphub.lib: $(SUBSYS_APPHUB_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
