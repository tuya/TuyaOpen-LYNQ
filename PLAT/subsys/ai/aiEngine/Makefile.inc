AIENGINE_DIR      := $(TOP)/PLAT/subsys/ai/aiEngine

CFLAGS_INC     += -I $(AIENGINE_DIR)/inc

AIENGINE_SRC_DIRS += $(AIENGINE_DIR)/src

CFLAGS += -DFEATURE_SUBSYS_AI_DOUBAO_ENABLE

AIENGINE_EXCLUDE_FILES :=

AIENGINE_CSRC = $(foreach dir, $(AIENGINE_SRC_DIRS), $(wildcard $(dir)/*.c))
AIENGINE_CFILES = $(filter-out $(AIENGINE_EXCLUDE_FILES), $(AIENGINE_CSRC))
AIENGINE_COBJSTEMP := $(patsubst %.c, %.o, $(AIENGINE_CFILES))
AIENGINE_COBJSTEMP :=$(subst $(AIENGINE_DIR),PLAT/subsys/ai/aiEngine,$(AIENGINE_COBJSTEMP))
AIENGINE_COBJS := $(addprefix $(BUILDDIR)/, $(AIENGINE_COBJSTEMP))
AIENGINE_PPFILES := $(patsubst %.o, %.pp, $(AIENGINE_COBJS))
PPFILES += $(AIENGINE_PPFILES)

CFLAGS += -DFEATURE_SUBSYS_AI_ENGINE_ENABLE

ifneq ($(MAKECMDGOALS), unilog)
-include $(AIENGINE_COBJS:.o=.d)
endif
-include $(AIENGINE_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libaiEngine.a

$(BUILDDIR)/lib/libaiEngine.a: $(AIENGINE_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libaiEngine.lib

$(BUILDDIR)/lib/libaiEngine.lib: $(AIENGINE_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
