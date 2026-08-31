SUBSYS_MQTT_DIR  := $(TOP)/PLAT/subsys/comm/mqtt
CFLAGS_INC += -I $(SUBSYS_MQTT_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_MQTT_ENABLE \
          -DFEATURE_SUBSYS_MQTT_ONENET_ENABLE

SUBSYS_MQTT_SRC_DIRS += $(SUBSYS_MQTT_DIR)/src

SUBSYS_MQTT_EXCLUDE_FILES :=

SUBSYS_MQTT_CSRC = $(foreach dir, $(SUBSYS_MQTT_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_MQTT_CFILES = $(filter-out $(SUBSYS_MQTT_EXCLUDE_FILES), $(SUBSYS_MQTT_CSRC))
SUBSYS_MQTT_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_MQTT_CFILES))
SUBSYS_MQTT_COBJSTEMP :=$(subst $(SUBSYS_MQTT_DIR),PLAT/subsys/comm/mqtt,$(SUBSYS_MQTT_COBJSTEMP))
SUBSYS_MQTT_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_MQTT_COBJSTEMP))
SUBSYS_MQTT_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_MQTT_COBJS))
PPFILES += $(SUBSYS_MQTT_PPFILES)


ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_MQTT_COBJS:.o=.d)
endif
-include $(SUBSYS_MQTT_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_mqtt.a

$(BUILDDIR)/lib/libsubsys_mqtt.a: $(SUBSYS_MQTT_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_mqtt.lib

$(BUILDDIR)/lib/libsubsys_mqtt.lib: $(SUBSYS_MQTT_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
