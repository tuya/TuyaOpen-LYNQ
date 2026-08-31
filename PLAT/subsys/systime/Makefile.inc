SUBSYS_SYSTIME_DIR  := $(TOP)/PLAT/subsys/systime
CFLAGS_INC += -I $(SUBSYS_SYSTIME_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_SYSTIME_ENABLE \
          -DFEATURE_PS_TCPIP_SNTP_SERVICE_ENABLE

SUBSYS_SYSTIME_SRC_DIRS += $(SUBSYS_SYSTIME_DIR)/src

SUBSYS_SYSTIME_EXCLUDE_FILES :=

SUBSYS_SYSTIME_CSRC = $(foreach dir, $(SUBSYS_SYSTIME_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_SYSTIME_CFILES = $(filter-out $(SUBSYS_SYSTIME_EXCLUDE_FILES), $(SUBSYS_SYSTIME_CSRC))
SUBSYS_SYSTIME_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_SYSTIME_CFILES))
SUBSYS_SYSTIME_COBJSTEMP :=$(subst $(SUBSYS_SYSTIME_DIR),PLAT/subsys/systime,$(SUBSYS_SYSTIME_COBJSTEMP))
SUBSYS_SYSTIME_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_SYSTIME_COBJSTEMP))
SUBSYS_SYSTIME_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_SYSTIME_COBJS))
PPFILES += $(SUBSYS_SYSTIME_PPFILES)


ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_SYSTIME_COBJS:.o=.d)
endif
-include $(SUBSYS_SYSTIME_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_systime.a

$(BUILDDIR)/lib/libsubsys_systime.a: $(SUBSYS_SYSTIME_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_systime.lib

$(BUILDDIR)/lib/libsubsys_systime.lib: $(SUBSYS_SYSTIME_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
