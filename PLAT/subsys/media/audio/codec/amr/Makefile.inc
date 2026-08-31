SUBSYS_AMR_DIR  := $(TOP)/PLAT/subsys/media/audio/codec/amr
CFLAGS_INC += -I $(SUBSYS_AMR_DIR)/inc

ifeq ($(SUBSYS_AMR_ENABLE),y)
CFLAGS += -DFEATURE_SUBSYS_AMR_ENABLE
endif

ifeq ($(SUBSYS_AMR_RECORD_ENABLE),y)
CFLAGS += -DFEATURE_SUBSYS_AMR_RECORD_ENABLE
endif

SUBSYS_AMR_SRC_DIRS += $(SUBSYS_AMR_DIR)/src

SUBSYS_AMR_EXCLUDE_FILES :=

SUBSYS_AMR_CSRC = $(foreach dir, $(SUBSYS_AMR_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_AMR_CFILES = $(filter-out $(SUBSYS_AMR_EXCLUDE_FILES), $(SUBSYS_AMR_CSRC))
SUBSYS_AMR_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_AMR_CFILES))
SUBSYS_AMR_COBJSTEMP :=$(subst $(SUBSYS_AMR_DIR),PLAT/subsys/media/audio/codec/amr,$(SUBSYS_AMR_COBJSTEMP))
SUBSYS_AMR_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_AMR_COBJSTEMP))
SUBSYS_AMR_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_AMR_COBJS))
PPFILES += $(SUBSYS_AMR_PPFILES)


ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_AMR_COBJS:.o=.d)
endif
-include $(SUBSYS_AMR_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_amr.a

$(BUILDDIR)/lib/libsubsys_amr.a: $(SUBSYS_AMR_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_amr.lib

$(BUILDDIR)/lib/libsubsys_amr.lib: $(SUBSYS_AMR_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
