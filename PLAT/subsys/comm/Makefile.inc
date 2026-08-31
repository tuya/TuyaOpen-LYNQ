SUBSYS_COMM_DIR  := $(TOP)/PLAT/subsys/comm
CFLAGS_INC += -I $(SUBSYS_COMM_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_COMM_ENABLE

ifeq ($(SUBSYS_MQTT_ENABLE),y)
include $(SUBSYS_COMM_DIR)/mqtt/Makefile.inc
endif

ifeq ($(SUBSYS_COMM_SOCKET_ENABLE),y)
include $(SUBSYS_COMM_DIR)/socket/Makefile.inc
endif

ifeq ($(SUBSYS_COMM_HTTPC_ENABLE),y)
include $(SUBSYS_COMM_DIR)/http/Makefile.inc
endif

ifeq ($(SUBSYS_COMM_HTTP_DOWNLOADER_ENABLE),y)
include $(SUBSYS_COMM_DIR)/http_downloader/Makefile.inc
endif

ifeq ($(SUBSYS_COMM_WEBSOCKET_ENABLE),y)
include $(SUBSYS_COMM_DIR)/websocket/Makefile.inc
endif

ifeq ($(SUBSYS_TELECOM_ENABLE),y)
include $(SUBSYS_COMM_DIR)/telecom/Makefile.inc
endif

SUBSYS_COMM_SRC_DIRS += $(SUBSYS_COMM_DIR)/src

SUBSYS_COMM_EXCLUDE_FILES :=

SUBSYS_COMM_CSRC = $(foreach dir, $(SUBSYS_COMM_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_COMM_CFILES = $(filter-out $(SUBSYS_COMM_EXCLUDE_FILES), $(SUBSYS_COMM_CSRC))
SUBSYS_COMM_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_COMM_CFILES))
SUBSYS_COMM_COBJSTEMP :=$(subst $(SUBSYS_COMM_DIR),PLAT/subsys/comm,$(SUBSYS_COMM_COBJSTEMP))
SUBSYS_COMM_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_COMM_COBJSTEMP))
SUBSYS_COMM_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_COMM_COBJS))
PPFILES += $(SUBSYS_COMM_PPFILES)


ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_COMM_COBJS:.o=.d)
endif
-include $(SUBSYS_COMM_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_comm.a

$(BUILDDIR)/lib/libsubsys_comm.a: $(SUBSYS_COMM_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_comm.lib

$(BUILDDIR)/lib/libsubsys_comm.lib: $(SUBSYS_COMM_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
