SUBSYS_OPENRECORD_DIR  := $(TOP)/PLAT/subsys/openrecorder
CFLAGS_INC += -I $(SUBSYS_OPENRECORD_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_OPENRECORDER_API_ENABLE

SUBSYS_OPENRECORD_SRC_DIRS += $(SUBSYS_OPENRECORD_DIR)/src

SUBSYS_OPENRECORD_EXCLUDE_FILES :=

SUBSYS_OPENRECORD_CSRC = $(foreach dir, $(SUBSYS_OPENRECORD_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_OPENRECORD_CFILES = $(filter-out $(SUBSYS_OPENRECORD_EXCLUDE_FILES), $(SUBSYS_OPENRECORD_CSRC))
SUBSYS_OPENRECORD_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_OPENRECORD_CFILES))
SUBSYS_OPENRECORD_COBJSTEMP :=$(subst $(SUBSYS_OPENRECORD_DIR),PLAT/subsys/openrecorder,$(SUBSYS_OPENRECORD_COBJSTEMP))
SUBSYS_OPENRECORD_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_OPENRECORD_COBJSTEMP))
SUBSYS_OPENRECORD_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_OPENRECORD_COBJS))
PPFILES += $(SUBSYS_OPENRECORD_PPFILES)


ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_OPENRECORD_COBJS:.o=.d)
endif
-include $(SUBSYS_OPENRECORD_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_openrecorder.a

$(BUILDDIR)/lib/libsubsys_openrecorder.a: $(SUBSYS_OPENRECORD_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_openrecorder.lib

$(BUILDDIR)/lib/libsubsys_openrecorder.lib: $(SUBSYS_OPENRECORD_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
