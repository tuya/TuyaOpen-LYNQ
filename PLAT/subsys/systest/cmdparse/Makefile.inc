CMDPARSE_DIR  := $(TOP)/PLAT/subsys/systest/cmdparse
CFLAGS_INC += -I $(CMDPARSE_DIR)/inc
              
CFLAGS += -DFEATURE_SUBSYS_CMDPARSE_ENABLE

CMDPARSE_SRC_DIRS += $(CMDPARSE_DIR)/src

CMDPARSE_EXCLUDE_FILES :=

CMDPARSE_CSRC = $(foreach dir, $(CMDPARSE_SRC_DIRS), $(wildcard $(dir)/*.c))
CMDPARSE_CFILES = $(filter-out $(CMDPARSE_EXCLUDE_FILES), $(CMDPARSE_CSRC))
CMDPARSE_COBJSTEMP := $(patsubst %.c, %.o, $(CMDPARSE_CFILES))
CMDPARSE_COBJSTEMP :=$(subst $(CMDPARSE_DIR),PLAT/subsys/systest/cmdparse,$(CMDPARSE_COBJSTEMP))
CMDPARSE_COBJS := $(addprefix $(BUILDDIR)/, $(CMDPARSE_COBJSTEMP))
CMDPARSE_PPFILES := $(patsubst %.o, %.pp, $(CMDPARSE_COBJS))
PPFILES += $(CMDPARSE_PPFILES)

ifneq ($(SUBSYS_CONSOLE_ENABLE),y)
ifeq ($(SUBSYS_UARTSERVICE_ENABLE),y)
include $(TOP)/PLAT/subsys/apphub/console/uartservice/Makefile.inc
endif
endif

ifneq ($(MAKECMDGOALS), build-unilog)
-include $(CMDPARSE_COBJS:.o=.d)
endif
-include $(CMDPARSE_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libcmdparse.a

$(BUILDDIR)/lib/libcmdparse.a: $(CMDPARSE_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libcmdparse.lib

$(BUILDDIR)/lib/libcmdparse.lib: $(CMDPARSE_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
