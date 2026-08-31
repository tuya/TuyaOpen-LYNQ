SUBSYS_SYSLOG_DIR  := $(TOP)/PLAT/subsys/syslog
CFLAGS_INC += -I $(SUBSYS_SYSLOG_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_SYSLOG_ENABLE

SUBSYS_SYSLOG_SRC_DIRS += $(SUBSYS_SYSLOG_DIR)/src

SUBSYS_SYSLOG_EXCLUDE_FILES :=

SUBSYS_SYSLOG_CSRC = $(foreach dir, $(SUBSYS_SYSLOG_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_SYSLOG_CFILES = $(filter-out $(SUBSYS_SYSLOG_EXCLUDE_FILES), $(SUBSYS_SYSLOG_CSRC))
SUBSYS_SYSLOG_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_SYSLOG_CFILES))
SUBSYS_SYSLOG_COBJSTEMP :=$(subst $(SUBSYS_SYSLOG_DIR),PLAT/subsys/syslog,$(SUBSYS_SYSLOG_COBJSTEMP))
SUBSYS_SYSLOG_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_SYSLOG_COBJSTEMP))
SUBSYS_SYSLOG_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_SYSLOG_COBJS))
PPFILES += $(SUBSYS_SYSLOG_PPFILES)

ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_SYSLOG_COBJS:.o=.d)
endif
-include $(SUBSYS_SYSLOG_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_syslog.a

$(BUILDDIR)/lib/libsubsys_syslog.a: $(SUBSYS_SYSLOG_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_syslog.lib

$(BUILDDIR)/lib/libsubsys_syslog.lib: $(SUBSYS_SYSLOG_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
