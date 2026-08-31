SUBSYS_SYSINFO_DIR  := $(TOP)/PLAT/subsys/sysinfo
CFLAGS_INC += -I $(SUBSYS_SYSINFO_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_SYSINFO_ENABLE

SUBSYS_SYSINFO_SRC_DIRS += $(SUBSYS_SYSINFO_DIR)/src

SUBSYS_SYSINFO_EXCLUDE_FILES :=

SUBSYS_SYSINFO_CSRC = $(foreach dir, $(SUBSYS_SYSINFO_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_SYSINFO_CFILES = $(filter-out $(SUBSYS_SYSINFO_EXCLUDE_FILES), $(SUBSYS_SYSINFO_CSRC))
SUBSYS_SYSINFO_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_SYSINFO_CFILES))
SUBSYS_SYSINFO_COBJSTEMP :=$(subst $(SUBSYS_SYSINFO_DIR),PLAT/subsys/sysinfo,$(SUBSYS_SYSINFO_COBJSTEMP))
SUBSYS_SYSINFO_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_SYSINFO_COBJSTEMP))
SUBSYS_SYSINFO_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_SYSINFO_COBJS))
PPFILES += $(SUBSYS_SYSINFO_PPFILES)

ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_SYSINFO_COBJS:.o=.d)
endif
-include $(SUBSYS_SYSINFO_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_sysinfo.a

$(BUILDDIR)/lib/libsubsys_sysinfo.a: $(SUBSYS_SYSINFO_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_sysinfo.lib

$(BUILDDIR)/lib/libsubsys_sysinfo.lib: $(SUBSYS_SYSINFO_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
