SUBSYS_GNSS_DIR  := $(TOP)/PLAT/subsys/gnss
CFLAGS_INC += -I $(SUBSYS_GNSS_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_GNSS_ENABLE


SUBSYS_GNSS_SRC_DIRS += $(SUBSYS_GNSS_DIR)/src

SUBSYS_GNSS_EXCLUDE_FILES :=

SUBSYS_GNSS_CSRC = $(foreach dir, $(SUBSYS_GNSS_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_GNSS_CFILES = $(filter-out $(SUBSYS_GNSS_EXCLUDE_FILES), $(SUBSYS_GNSS_CSRC))
SUBSYS_GNSS_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_GNSS_CFILES))
SUBSYS_GNSS_COBJSTEMP :=$(subst $(SUBSYS_GNSS_DIR),PLAT/subsys/gnss,$(SUBSYS_GNSS_COBJSTEMP))
SUBSYS_GNSS_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_GNSS_COBJSTEMP))
SUBSYS_GNSS_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_GNSS_COBJS))
PPFILES += $(SUBSYS_GNSS_PPFILES)


ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_GNSS_COBJS:.o=.d)
endif
-include $(SUBSYS_GNSS_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_gnss.a

$(BUILDDIR)/lib/libsubsys_gnss.a: $(SUBSYS_GNSS_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_gnss.lib

$(BUILDDIR)/lib/libsubsys_gnss.lib: $(SUBSYS_GNSS_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
