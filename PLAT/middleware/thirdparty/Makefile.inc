
ifeq ($(THIRDPARTY_MQTT_ENABLE),y)
include $(TOP)/PLAT/middleware/thirdparty/mqtt/Makefile.inc
endif

ifeq ($(THIRDPARTY_REF_CR_MQTT_ENABLE),y)
include $(TOP)/PLAT/middleware/thirdparty/mqtt/Makefile.inc
endif

ifeq ($(THIRDPARTY_HTTPC_ENABLE),y)
include $(TOP)/PLAT/middleware/thirdparty/httpclient/Makefile.inc
endif

ifeq ($(THIRDPARTY_LWIP_ENABLE),y)
include $(TOP)/PLAT/middleware/thirdparty/lwip/Makefile.inc
endif

ifeq ($(THIRDPARTY_MBEDTLS_ENABLE),y)
include $(TOP)/PLAT/middleware/thirdparty/mbedtls/Makefile.inc
endif

ifeq ($(THIRDPARTY_IPERF_ENABLE),y)
include $(TOP)/PLAT/middleware/thirdparty/iperf/Makefile.inc
endif

ifeq ($(findstring y, $(THIRDPARTY_PING_ENABLE)$(BUILD_PS_REF_CR_TCPIP_PING_SERVICE_ENABLE)$(BUILD_PS_REF_QR_TCPIP_QPING_SERVICE_ENABLE)), y)
include $(TOP)/PLAT/middleware/thirdparty/ping/Makefile.inc
endif

ifeq ($(THIRDPARTY_LITTEFS_ENABLE),y)
include $(TOP)/PLAT/middleware/thirdparty/littlefs/Makefile.inc
endif

ifeq ($(findstring y, $(THIRDPARTY_LIBSNTP_ENABLE)$(BUILD_PS_REF_CR_TCPIP_SNTP_SERVICE_ENABLE)$(BUILD_PS_REF_QR_TCPIP_QSNTP_SERVICE_ENABLE)), y)
include $(TOP)/PLAT/middleware/thirdparty/sntp/Makefile.inc
endif

ifeq ($(THIRDPARTY_CJSON_ENABLE),y)
include $(TOP)/PLAT/middleware/thirdparty/cjson/Makefile.inc
endif

ifeq ($(findstring y, $(THIRDPARTY_CMCC_DM_ENABLE)$(THIRDPARTY_CTCC_DM_ENABLE)$(THIRDPARTY_CUCC_DM_ENABLE)), y)
include $(TOP)/PLAT/middleware/thirdparty/dm/Makefile.inc
endif

ifeq ($(THIRDPARTY_CTWING_CERTI_ENABLE),y)
include $(TOP)/PLAT/middleware/thirdparty/ctwing/Makefile.inc
endif

ifeq ($(THIRDPARTY_DHRYSTONE_ENABLE),y)
include $(TOP)/PLAT/middleware/thirdparty/dhrystone/Makefile.inc
endif

ifeq ($(THIRDPARTY_COREMARK_ENABLE),y)
include $(TOP)/PLAT/middleware/thirdparty/coremark/Makefile.inc
endif


ifeq ($(THIRDPARTY_ERCOAP_ENABLE),y)
include $(TOP)/PLAT/middleware/thirdparty/er-coap-13/Makefile.inc
endif
ifeq ($(THIRDPARTY_CTWING1_5_ENABLE),y)
include $(TOP)/PLAT/middleware/thirdparty/wakaama_core/Makefile.inc
include $(TOP)/PLAT/middleware/thirdparty/ctlwm2m/V1.5/Makefile.inc
endif
ifeq ($(THIRDPARTY_CISONENET_ENABLE),y)
include $(TOP)/PLAT/middleware/thirdparty/cis_onenet/Makefile.inc
endif

ifeq ($(THIRDPARTY_LZMA_ENABLE),y)
include $(TOP)/PLAT/middleware/thirdparty/lzma2201/Makefile.inc
endif

ifeq ($(THIRDPARTY_BZIP2_ENABLE),y)
include $(TOP)/PLAT/middleware/thirdparty/bzip2/Makefile.inc
endif

ifeq ($(THIRDPARTY_OPUS_ENABLE),y)
include $(TOP)/PLAT/middleware/thirdparty/opus/Makefile.inc
endif

ifeq ($(THIRDPARTY_SPEEX_ENABLE),y)
include $(TOP)/PLAT/middleware/thirdparty/multimedia/speex/Makefile.inc
endif

ifeq ($(THIRDPARTY_WEBRTC_ENABLE),y)
include $(TOP)/PLAT/middleware/thirdparty/multimedia/webrtc/Makefile.inc
endif

ifneq ($(BUILD_USE_PREBUILD_LIB),y)
ifeq ($(THIRDPARTY_PNGDECODE_ENABLE),y)
include $(TOP)/PLAT/middleware/thirdparty/multimedia/pngdecoder/Makefile.inc
endif
endif

ifneq ($(BUILD_USE_PREBUILD_LIB),y)
ifeq ($(THIRDPARTY_MMVIDEO_ENABLE),y)
include $(TOP)/PLAT/middleware/thirdparty/multimedia/mm/Makefile.inc
endif
endif

ifeq ($(THIRDPARTY_UECC_ENABLE),y)
include $(TOP)/PLAT/middleware/thirdparty/ecc/Makefile.inc
endif

