
MQTT_DIR      := $(TOP)/PLAT/middleware/thirdparty/mqtt

ifeq ($(OS),freertos)
CFLAGS_INC    += -I $(TOP)/PLAT/os/freertos/inc               \
                  -I $(TOP)/PLAT/os/freertos/CMSIS/inc        \
                  -I $(TOP)/PLAT/middleware/thirdparty/cjson  \
                  -I $(MQTT_DIR)/MQTTPacket/src               \
                  -I $(MQTT_DIR)/MQTTClient-C/src/FreeRTOS    \
                  -I $(MQTT_DIR)/MQTTClient-C/src             \
                  -I $(MQTT_DIR)   

MQTT_SRC_DIRS += $(MQTT_DIR)/MQTTPacket/src            \
                 $(MQTT_DIR)/MQTTClient-C/src/FreeRTOS \
                 $(MQTT_DIR)/MQTTClient-C/src          \
                 $(MQTT_DIR)
endif

ifeq ($(OS),liteos)
CFLAGS_INC    += -I $(TOP)/PLAT/os/liteos_m-v3.1.4/kal/cmsis_ec/common/inc               \
                  -I $(TOP)/PLAT/os/liteos_m-v3.1.4/kal/cmsis_ec/ap/inc        \
                  -I $(TOP)/PLAT/middleware/thirdparty/cjson  \
                  -I $(MQTT_DIR)/MQTTPacket/src               \
                  -I $(MQTT_DIR)/MQTTClient-C/src/liteos    \
                  -I $(MQTT_DIR)/MQTTClient-C/src             \
                  -I $(MQTT_DIR)   
              

MQTT_SRC_DIRS += $(MQTT_DIR)/MQTTPacket/src            \
                 $(MQTT_DIR)/MQTTClient-C/src/liteos \
                 $(MQTT_DIR)/MQTTClient-C/src          \
                 $(MQTT_DIR)
endif

ifeq ($(MBEDTLS_WITH_MQTT_TLS),y)
    CFLAGS_INC    += -I $(MQTT_DIR)/MQTTClient-C/src/eigencomm

    MQTT_SRC_DIRS += $(MQTT_DIR)/MQTTClient-C/src/eigencomm
else
    CFLAGS += -DFEATURE_MQTT_ENABLE
endif


MQTT_EXCLUDE_FILES :=

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

$(BUILDDIR)/lib/libmqtt.a: $(MQTT_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libmqtt.lib

$(BUILDDIR)/lib/libmqtt.lib: $(MQTT_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
