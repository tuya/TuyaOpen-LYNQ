FINSH_DIR		:= $(TOP)/PLAT/subsys/apphub/console/finsh

CFLAGS_INC     	+=  -I $(FINSH_DIR)

FINSH_SRC_DIRS 	+= 	$(FINSH_DIR)

FINSH_EXCLUDE_FILES :=
FINSH_CSRC = $(foreach dir, $(FINSH_SRC_DIRS), $(wildcard $(dir)/*.c))
FINSH_CFILES = $(filter-out $(FINSH_EXCLUDE_FILES), $(FINSH_CSRC))
FINSH_COBJSTEMP := $(patsubst %.c, %.o, $(FINSH_CFILES))
FINSH_COBJSTEMP := $(subst $(FINSH_DIR),PLAT/subsys/apphub/console/finsh,$(FINSH_COBJSTEMP))
FINSH_COBJS := $(addprefix $(BUILDDIR)/, $(FINSH_COBJSTEMP))

FINSH_PPFILES := $(patsubst %.o, %.pp, $(FINSH_COBJS))
PPFILES += $(FINSH_PPFILES)

ifneq ($(MAKECMDGOALS), unilog)
-include $(FINSH_COBJS:.o=.d)
endif

-include $(FINSH_COBJS:.o=.dd)
CFLAGS += -DFEATURE_SUBSYS_FINSH_ENABLE

ifeq ($(TOOLCHAIN),GCC)
CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc
lib-y += libfinsh.a
$(BUILDDIR)/lib/libfinsh.a: $(FINSH_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^
endif

ifeq ($(TOOLCHAIN),ARMCC)
CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil
lib-y += libfinsh.lib
$(BUILDDIR)/lib/libfinsh.lib: $(FINSH_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^
endif
