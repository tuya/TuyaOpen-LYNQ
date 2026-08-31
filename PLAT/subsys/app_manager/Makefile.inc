SUBSYS_APP_MANAGER_DIR  := $(TOP)/PLAT/subsys/app_manager
CFLAGS_INC += -I $(SUBSYS_APP_MANAGER_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_APP_MANAGER_ENABLE

SUBSYS_APP_MANAGER_SRC_DIRS += $(SUBSYS_APP_MANAGER_DIR)/src

SUBSYS_APP_MANAGER_EXCLUDE_FILES :=

SUBSYS_APP_MANAGER_CSRC = $(foreach dir, $(SUBSYS_APP_MANAGER_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_APP_MANAGER_CFILES = $(filter-out $(SUBSYS_APP_MANAGER_EXCLUDE_FILES), $(SUBSYS_APP_MANAGER_CSRC))
SUBSYS_APP_MANAGER_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_APP_MANAGER_CFILES))
SUBSYS_APP_MANAGER_COBJSTEMP :=$(subst $(SUBSYS_APP_MANAGER_DIR),PLAT/subsys/app_manager,$(SUBSYS_APP_MANAGER_COBJSTEMP))
SUBSYS_APP_MANAGER_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_APP_MANAGER_COBJSTEMP))
SUBSYS_APP_MANAGER_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_APP_MANAGER_COBJS))
PPFILES += $(SUBSYS_APP_MANAGER_PPFILES)

ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_APP_MANAGER_COBJS:.o=.d)
endif
-include $(SUBSYS_APP_MANAGER_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_app_manager.a

$(BUILDDIR)/lib/libsubsys_app_manager.a: $(SUBSYS_APP_MANAGER_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_app_manager.lib

$(BUILDDIR)/lib/libsubsys_app_manager.lib: $(SUBSYS_APP_MANAGER_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
