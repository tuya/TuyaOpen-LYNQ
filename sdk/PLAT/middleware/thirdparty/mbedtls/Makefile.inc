MBEDTLS_DIR      := $(TOP)/PLAT/middleware/thirdparty/mbedtls
CFLAGS_INC    +=  -I $(MBEDTLS_DIR)/include  \
                  -I $(MBEDTLS_DIR)/include/mbedtls  \
                  -I $(MBEDTLS_DIR)/configs
 
MBEDTLS_SRC_DIRS += $(MBEDTLS_DIR)/library    

MBEDTLS_CFLAGS ?= -DMBEDTLS_CONFIG_FILE=\"config_ec_ssl_comm.h\"

ifeq ($(findstring y, $(BUILD_PLAT_SSL_AT_ENABLE)_$(MBEDTLS_WITH_HTTP_TLS)), y)
	CFLAGS += -DFEATURE_APP_TLS_ENABLE
endif

ifeq ($(MBEDTLS_WITH_HTTP_TLS),y)
	CFLAGS += -DFEATURE_HTTP_TLS_ENABLE
endif

ifeq ($(MBEDTLS_WITH_MQTT_TLS),y)
	CFLAGS += -DFEATURE_MQTT_TLS_ENABLE
endif

CFLAGS += $(MBEDTLS_CFLAGS)
CFLAGS += -DFEATURE_MBEDTLS_ENABLE
CFLAGS += -DMQTT_RAI_OPTIMIZE
#CFLAGS += -DHARDWARE_AES_CBC

MBEDTLS_EXCLUDE_FILES :=




ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc


$(BUILDDIR)/lib/libmbedtls.a: $(MBEDTLS_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libmbedtls.lib

$(BUILDDIR)/lib/libmbedtls.lib: $(MBEDTLS_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
