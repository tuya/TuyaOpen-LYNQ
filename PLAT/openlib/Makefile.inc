
OPENLIB_DIR   ?= $(TOP)/PLAT/openlib

ifeq ($(BINNAME),ap_bootloader)
    PREBUILDLIBS += $(OPENLIB_DIR)/$(MBTK_PRODUCT_TARGET)/boot/libdriver.a \
                    $(OPENLIB_DIR)/$(MBTK_PRODUCT_TARGET)/boot/libmiddleware_ec.a \
                    $(OPENLIB_DIR)/$(MBTK_PRODUCT_TARGET)/boot/libuecc.a
else
    PREBUILDLIBS += $(OPENLIB_DIR)/$(MBTK_PRODUCT_TARGET)/lib/libdriver.a \
                    $(OPENLIB_DIR)/$(MBTK_PRODUCT_TARGET)/lib/libfreertos.a \
                    $(OPENLIB_DIR)/$(MBTK_PRODUCT_TARGET)/lib/libmiddleware_ec.a \
                    $(OPENLIB_DIR)/$(MBTK_PRODUCT_TARGET)/lib/libpsnv.a \
                    $(OPENLIB_DIR)/$(MBTK_PRODUCT_TARGET)/lib/libtcpipmgr.a \
                    $(OPENLIB_DIR)/$(MBTK_PRODUCT_TARGET)/lib/libopenapi.a \
					$(OPENLIB_DIR)/$(MBTK_PRODUCT_TARGET)/lib/libccio.a

    ifeq ($(THIRDPARTY_YRCOMPRESS_ENABLE),y)
    PREBUILDLIBS += $(OPENLIB_DIR)/$(MBTK_PRODUCT_TARGET)/lib/libyrcompress.a
    endif

    ifeq ($(THIRDPARTY_BZIP2_ENABLE),y)
    PREBUILDLIBS += $(OPENLIB_DIR)/$(MBTK_PRODUCT_TARGET)/lib/libbzip2.a
    endif

    ifeq ($(THIRDPARTY_MQTT_ENABLE),y)
    PREBUILDLIBS += $(OPENLIB_DIR)/$(MBTK_PRODUCT_TARGET)/lib/libmqtt.a
    endif

    ifeq ($(THIRDPARTY_HTTPC_ENABLE),y)
    PREBUILDLIBS += $(OPENLIB_DIR)/$(MBTK_PRODUCT_TARGET)/lib/libhttpclient.a
    endif

    ifeq ($(THIRDPARTY_LWIP_ENABLE),y)
    PREBUILDLIBS += $(OPENLIB_DIR)/$(MBTK_PRODUCT_TARGET)/lib/liblwip.a
    endif

    ifeq ($(THIRDPARTY_MBEDTLS_ENABLE),y)
    PREBUILDLIBS += $(OPENLIB_DIR)/$(MBTK_PRODUCT_TARGET)/lib/libmbedtls.a
    endif

    ifeq ($(THIRDPARTY_IPERF_ENABLE),y)
    PREBUILDLIBS += $(OPENLIB_DIR)/$(MBTK_PRODUCT_TARGET)/lib/libiperf.a
    endif

    ifeq ($(THIRDPARTY_PING_ENABLE),y)
    PREBUILDLIBS += $(OPENLIB_DIR)/$(MBTK_PRODUCT_TARGET)/lib/libping.a
    endif

    ifeq ($(THIRDPARTY_LITTEFS_ENABLE),y)
    PREBUILDLIBS += $(OPENLIB_DIR)/$(MBTK_PRODUCT_TARGET)/lib/liblfs.a
    endif

    ifeq ($(THIRDPARTY_LIBSNTP_ENABLE),y)
    PREBUILDLIBS += $(OPENLIB_DIR)/$(MBTK_PRODUCT_TARGET)/lib/libsntp.a
    endif

    ifeq ($(THIRDPARTY_CJSON_ENABLE),y)
    PREBUILDLIBS += $(OPENLIB_DIR)/$(MBTK_PRODUCT_TARGET)/lib/libcjson.a
    endif

    ifeq ($(findstring y, $(THIRDPARTY_CMCC_DM_ENABLE)$(THIRDPARTY_CTCC_DM_ENABLE)$(THIRDPARTY_CUCC_DM_ENABLE)), y)
    PREBUILDLIBS += $(OPENLIB_DIR)/$(MBTK_PRODUCT_TARGET)/lib/libdm.a
    endif

    ifeq ($(THIRDPARTY_CTWING_CERTI_ENABLE),y)
    PREBUILDLIBS += $(OPENLIB_DIR)/$(MBTK_PRODUCT_TARGET)/lib/libctwing.a
    endif

    ifeq ($(findstring y, $(THIRDPARTY_WAKAAMA_ENABLE)$(THIRDPARTY_CTWING1_5_ENABLE)), y)
    PREBUILDLIBS += $(OPENLIB_DIR)/$(MBTK_PRODUCT_TARGET)/lib/libwakaamacore.a
    endif

    ifeq ($(THIRDPARTY_CTWING1_5_ENABLE),y)
    PREBUILDLIBS += $(OPENLIB_DIR)/$(MBTK_PRODUCT_TARGET)/lib/libctlwm2m.a
    endif

    ifeq ($(THIRDPARTY_ERCOAP_ENABLE),y)
    PREBUILDLIBS += $(OPENLIB_DIR)/$(MBTK_PRODUCT_TARGET)/lib/libercoap.a
    endif

    ifeq ($(THIRDPARTY_UECC_ENABLE),y)
    PREBUILDLIBS += $(OPENLIB_DIR)/$(MBTK_PRODUCT_TARGET)/lib/libuecc.a
    endif

    ifeq ($(IMS_MODE_ENABLE),y)
    PREBUILDLIBS += $(OPENLIB_DIR)/$(MBTK_PRODUCT_TARGET)/lib/libimsnv.a
    endif
	
endif
