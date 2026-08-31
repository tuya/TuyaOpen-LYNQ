SUBSYS_SOCKET_DIR  := $(TOP)/PLAT/subsys/comm/socket
CFLAGS_INC += -I $(SUBSYS_SOCKET_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_SOCKET_ENABLE

SUBSYS_SOCKET_SRC_DIRS += $(SUBSYS_SOCKET_DIR)/src

SUBSYS_SOCKET_EXCLUDE_FILES :=

SUBSYS_SOCKET_CSRC = $(foreach dir, $(SUBSYS_SOCKET_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_SOCKET_CFILES = $(filter-out $(SUBSYS_SOCKET_EXCLUDE_FILES), $(SUBSYS_SOCKET_CSRC))
SUBSYS_SOCKET_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_SOCKET_CFILES))
SUBSYS_SOCKET_COBJSTEMP :=$(subst $(SUBSYS_SOCKET_DIR),PLAT/subsys/comm/socket,$(SUBSYS_SOCKET_COBJSTEMP))
SUBSYS_SOCKET_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_SOCKET_COBJSTEMP))
SUBSYS_SOCKET_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_SOCKET_COBJS))
PPFILES += $(SUBSYS_SOCKET_PPFILES)


ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_SOCKET_COBJS:.o=.d)
endif
-include $(SUBSYS_SOCKET_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_socket.a

$(BUILDDIR)/lib/libsubsys_socket.a: $(SUBSYS_SOCKET_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_socket.lib

$(BUILDDIR)/lib/libsubsys_socket.lib: $(SUBSYS_SOCKET_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
