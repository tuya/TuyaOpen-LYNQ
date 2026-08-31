LFSEX_DIR  := $(TOP)/PLAT/subsys/storage/lfsex
CFLAGS_INC += -I $(LFSEX_DIR)/inc
              
CFLAGS += -DFEATURE_SUBSYS_LFSEX_ENABLE

LFSEX_SRC_DIRS += $(LFSEX_DIR)/src

LFSEX_EXCLUDE_FILES :=

LFSEX_CSRC = $(foreach dir, $(LFSEX_SRC_DIRS), $(wildcard $(dir)/lfsex_port_task.c))
LFSEX_CFILES = $(filter-out $(LFSEX_EXCLUDE_FILES), $(LFSEX_CSRC))
LFSEX_COBJSTEMP := $(patsubst %.c, %.o, $(LFSEX_CFILES))
LFSEX_COBJSTEMP :=$(subst $(LFSEX_DIR),PLAT/subsys/storage/lfsex,$(LFSEX_COBJSTEMP))
LFSEX_COBJS := $(addprefix $(BUILDDIR)/, $(LFSEX_COBJSTEMP))
LFSEX_PPFILES := $(patsubst %.o, %.pp, $(LFSEX_COBJS))
PPFILES += $(LFSEX_PPFILES)


ifneq ($(MAKECMDGOALS), build-unilog)
-include $(LFSEX_COBJS:.o=.d)
endif
-include $(LFSEX_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += liblfsex.a

$(BUILDDIR)/lib/liblfsex.a: $(LFSEX_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += liblfsex.lib

$(BUILDDIR)/lib/liblfsex.lib: $(LFSEX_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
