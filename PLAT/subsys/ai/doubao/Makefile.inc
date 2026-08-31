DOUBAO_DIR      := $(TOP)/PLAT/subsys/ai/doubao

CFLAGS_INC     += -I $(DOUBAO_DIR)/inc

DOUBAO_SRC_DIRS += $(DOUBAO_DIR)/src/common \
                    $(DOUBAO_DIR)/src/platform/ec71x

PREBUILDLIBS += $(DOUBAO_DIR)/lib/libVolcEngineRTCLite_a.a

DOUBAO_EXCLUDE_FILES :=

DOUBAO_CSRC = $(foreach dir, $(DOUBAO_SRC_DIRS), $(wildcard $(dir)/*.c))
DOUBAO_CFILES = $(filter-out $(DOUBAO_EXCLUDE_FILES), $(DOUBAO_CSRC))
DOUBAO_COBJSTEMP := $(patsubst %.c, %.o, $(DOUBAO_CFILES))
DOUBAO_COBJSTEMP :=$(subst $(DOUBAO_DIR),PLAT/subsys/ai/doubao,$(DOUBAO_COBJSTEMP))
DOUBAO_COBJS := $(addprefix $(BUILDDIR)/, $(DOUBAO_COBJSTEMP))
DOUBAO_PPFILES := $(patsubst %.o, %.pp, $(DOUBAO_COBJS))
PPFILES += $(DOUBAO_PPFILES)

CFLAGS += -DFEATURE_SUBSYS_AI_DOUBAO_ENABLE

ifneq ($(MAKECMDGOALS), unilog)
-include $(DOUBAO_COBJS:.o=.d)
endif
-include $(DOUBAO_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libVolcEngineRTCHal.a

$(BUILDDIR)/lib/libVolcEngineRTCHal.a: $(DOUBAO_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libVolcEngineRTCHal.lib

$(BUILDDIR)/lib/libVolcEngineRTCHal.lib: $(DOUBAO_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
