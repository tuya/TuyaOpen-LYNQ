SUBSYS_WEBSOCKET_DIR  := $(TOP)/PLAT/subsys/comm/websocket
CFLAGS_INC += -I $(SUBSYS_WEBSOCKET_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_WEBSOCKET_ENABLE

SUBSYS_WEBSOCKET_SRC_DIRS += $(SUBSYS_WEBSOCKET_DIR)/src

SUBSYS_WEBSOCKET_EXCLUDE_FILES :=

SUBSYS_WEBSOCKET_CSRC = $(foreach dir, $(SUBSYS_WEBSOCKET_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_WEBSOCKET_CFILES = $(filter-out $(SUBSYS_WEBSOCKET_EXCLUDE_FILES), $(SUBSYS_WEBSOCKET_CSRC))
SUBSYS_WEBSOCKET_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_WEBSOCKET_CFILES))
SUBSYS_WEBSOCKET_COBJSTEMP :=$(subst $(SUBSYS_WEBSOCKET_DIR),PLAT/subsys/comm/websocket,$(SUBSYS_WEBSOCKET_COBJSTEMP))
SUBSYS_WEBSOCKET_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_WEBSOCKET_COBJSTEMP))
SUBSYS_WEBSOCKET_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_WEBSOCKET_COBJS))
PPFILES += $(SUBSYS_WEBSOCKET_PPFILES)


ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_WEBSOCKET_COBJS:.o=.d)
endif
-include $(SUBSYS_WEBSOCKET_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_websocket.a

$(BUILDDIR)/lib/libsubsys_websocket.a: $(SUBSYS_WEBSOCKET_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_websocket.lib

$(BUILDDIR)/lib/libsubsys_websocket.lib: $(SUBSYS_WEBSOCKET_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
