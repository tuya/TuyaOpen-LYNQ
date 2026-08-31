SUBSYS_TELECOM_DIR  := $(TOP)/PLAT/subsys/comm/telecom
CFLAGS_INC += -I $(SUBSYS_TELECOM_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_TELECOM_ENABLE

SUBSYS_TELECOM_SRC_DIRS += $(SUBSYS_TELECOM_DIR)/src

SUBSYS_TELECOM_EXCLUDE_FILES :=

SUBSYS_TELECOM_CSRC = $(foreach dir, $(SUBSYS_TELECOM_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_TELECOM_CFILES = $(filter-out $(SUBSYS_TELECOM_EXCLUDE_FILES), $(SUBSYS_TELECOM_CSRC))
SUBSYS_TELECOM_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_TELECOM_CFILES))
SUBSYS_TELECOM_COBJSTEMP :=$(subst $(SUBSYS_TELECOM_DIR),PLAT/subsys/comm/telecom,$(SUBSYS_TELECOM_COBJSTEMP))
SUBSYS_TELECOM_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_TELECOM_COBJSTEMP))
SUBSYS_TELECOM_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_TELECOM_COBJS))
PPFILES += $(SUBSYS_TELECOM_PPFILES)


ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_TELECOM_COBJS:.o=.d)
endif
-include $(SUBSYS_TELECOM_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_telecom.a

$(BUILDDIR)/lib/libsubsys_telecom.a: $(SUBSYS_TELECOM_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_telecom.lib

$(BUILDDIR)/lib/libsubsys_telecom.lib: $(SUBSYS_TELECOM_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
