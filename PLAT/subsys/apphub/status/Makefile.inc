SUBSYS_STATUS_DIR  := $(TOP)/PLAT/subsys/apphub/status
CFLAGS_INC += -I $(SUBSYS_STATUS_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_STATUS_ENABLE

SUBSYS_STATUS_SRC_DIRS += $(SUBSYS_STATUS_DIR)/src

SUBSYS_STATUS_EXCLUDE_FILES :=

SUBSYS_STATUS_CSRC = $(foreach dir, $(SUBSYS_STATUS_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_STATUS_CFILES = $(filter-out $(SUBSYS_STATUS_EXCLUDE_FILES), $(SUBSYS_STATUS_CSRC))
SUBSYS_STATUS_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_STATUS_CFILES))
SUBSYS_STATUS_COBJSTEMP :=$(subst $(SUBSYS_STATUS_DIR),PLAT/subsys/apphub/status,$(SUBSYS_STATUS_COBJSTEMP))
SUBSYS_STATUS_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_STATUS_COBJSTEMP))
SUBSYS_STATUS_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_STATUS_COBJS))
PPFILES += $(SUBSYS_STATUS_PPFILES)


ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_STATUS_COBJS:.o=.d)
endif
-include $(SUBSYS_STATUS_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_status.a

$(BUILDDIR)/lib/libsubsys_status.a: $(SUBSYS_STATUS_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_status.lib

$(BUILDDIR)/lib/libsubsys_status.lib: $(SUBSYS_STATUS_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
