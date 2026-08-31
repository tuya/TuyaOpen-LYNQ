UARTSERVICE_DIR  := $(TOP)/PLAT/subsys/apphub/console/uartservice

CFLAGS_INC += -I $(UARTSERVICE_DIR)/inc
              
CFLAGS += -DFEATURE_SUBSYS_UARTSERVICE_ENABLE

UARTSERVICE_SRC_DIRS += $(UARTSERVICE_DIR)/src

UARTSERVICE_EXCLUDE_FILES :=

UARTSERVICE_CSRC = $(foreach dir, $(UARTSERVICE_SRC_DIRS), $(wildcard $(dir)/*.c))
UARTSERVICE_CFILES = $(filter-out $(UARTSERVICE_EXCLUDE_FILES), $(UARTSERVICE_CSRC))
UARTSERVICE_COBJSTEMP := $(patsubst %.c, %.o, $(UARTSERVICE_CFILES))
UARTSERVICE_COBJSTEMP :=$(subst $(UARTSERVICE_DIR),PLAT/subsys/apphub/console/uartservice,$(UARTSERVICE_COBJSTEMP))
UARTSERVICE_COBJS := $(addprefix $(BUILDDIR)/, $(UARTSERVICE_COBJSTEMP))
UARTSERVICE_PPFILES := $(patsubst %.o, %.pp, $(UARTSERVICE_COBJS))
PPFILES += $(UARTSERVICE_PPFILES)


ifneq ($(MAKECMDGOALS), build-unilog)
-include $(UARTSERVICE_COBJS:.o=.d)
endif
-include $(UARTSERVICE_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libuartservice.a

$(BUILDDIR)/lib/libuartservice.a: $(UARTSERVICE_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libuartservice.lib

$(BUILDDIR)/lib/libuartservice.lib: $(UARTSERVICE_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
