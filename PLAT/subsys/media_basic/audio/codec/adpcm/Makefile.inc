SUBSYS_ADPCM_DIR  := $(TOP)/PLAT/subsys/media_basic/audio/codec/adpcm
CFLAGS_INC += -I $(SUBSYS_ADPCM_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_ADPCM_ENABLE

SUBSYS_ADPCM_SRC_DIRS += $(SUBSYS_ADPCM_DIR)/src

SUBSYS_ADPCM_EXCLUDE_FILES :=

SUBSYS_ADPCM_CSRC = $(foreach dir, $(SUBSYS_ADPCM_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_ADPCM_CFILES = $(filter-out $(SUBSYS_ADPCM_EXCLUDE_FILES), $(SUBSYS_ADPCM_CSRC))
SUBSYS_ADPCM_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_ADPCM_CFILES))
SUBSYS_ADPCM_COBJSTEMP :=$(subst $(SUBSYS_ADPCM_DIR),PLAT/subsys/media_basic/audio/codec/adpcm,$(SUBSYS_ADPCM_COBJSTEMP))
SUBSYS_ADPCM_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_ADPCM_COBJSTEMP))
SUBSYS_ADPCM_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_ADPCM_COBJS))
PPFILES += $(SUBSYS_ADPCM_PPFILES)


ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_ADPCM_COBJS:.o=.d)
endif
-include $(SUBSYS_ADPCM_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_adpcm.a

$(BUILDDIR)/lib/libsubsys_adpcm.a: $(SUBSYS_ADPCM_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_adpcm.lib

$(BUILDDIR)/lib/libsubsys_adpcm.lib: $(SUBSYS_ADPCM_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
