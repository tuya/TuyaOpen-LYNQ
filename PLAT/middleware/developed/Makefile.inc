MW_HOST_DIRS := $(TOP)/PLAT/middleware/developed
CFLAGS += -DARM_MATH_CM3
ifeq ($(CHIP),ec618)
CFLAGS_INC += -I $(MW_HOST_DIRS)/nvram/inc
else
CFLAGS_INC += -I $(MW_HOST_DIRS)/nvram/ec7xx/inc
endif

CFLAGS_INC += -I $(MW_HOST_DIRS)/debug/inc \
              -I $(MW_HOST_DIRS)/cms/psdial/inc \
              -I $(MW_HOST_DIRS)/cms/cms/inc \
              -I $(MW_HOST_DIRS)/cms/psil/inc \
              -I $(MW_HOST_DIRS)/cms/pslpp/inc \
              -I $(MW_HOST_DIRS)/cms/pslpp/nbasn1/inc \
              -I $(MW_HOST_DIRS)/cms/psstk/inc \
              -I $(MW_HOST_DIRS)/cms/sockmgr/inc \
              -I $(MW_HOST_DIRS)/cms/cmsnetlight/inc \
              -I $(MW_HOST_DIRS)/ecapi/aal/inc        \
              -I $(MW_HOST_DIRS)/ecapi/appmwapi/inc \
              -I $(MW_HOST_DIRS)/ecapi/psapi/inc \
              -I $(MW_HOST_DIRS)/simbip/inc        \
              -I $(MW_HOST_DIRS)/common/inc        \
              -I $(CROSS_COMPILE)/../../PACK/ARM/CMSIS/5.0.1/CMSIS/include

ifeq ($(BUILD_PS), y)
include $(TOP)/PLAT/middleware/developed/psnv/Makefile.inc
include $(TOP)/PLAT/middleware/developed/tcpipmgr/Makefile.inc
ifeq ($(BUILD_IMS), y)
include $(TOP)/PLAT/middleware/developed/imsnv/Makefile.inc
endif
endif

ifeq ($(THIRDPARTY_YRCOMPRESS_ENABLE),y)
include $(TOP)/PLAT/middleware/developed/yrcompress/Makefile.inc
endif

ifeq ($(BUILD_USE_PREBUILD_LIB), n)
CFLAGS_INC += -I $(TOP)/PROTOCOL/SRC/tcpip/netmgr/inc \
              -I $(TOP)/PROTOCOL/SRC/ps/ccm/cmi       \
              -I $(TOP)/PROTOCOL/SRC/tcpip/lwip/src/include \
              -I $(TOP)/PROTOCOL/SRC/tcpip/lwip/src/include/lwip

else
CFLAGS_INC += -I $(LIBDIR)/PS/inc \
              -I $(TOP)/PLAT/middleware/thirdparty/lwip/src/include \
              -I $(TOP)/PLAT/middleware/thirdparty/lwip/src/include/lwip
endif

ifeq ($(AT_EXAMPLE_ENABLE),y)
CFLAGS += -DFEATURE_EXAMPLE_ENABLE
endif

ifeq ($(MIDDLEWARE_ECAPI_AAL_ENABLE),y)
MW_SRC_DIRS += $(MW_HOST_DIRS)/ecapi/aal/src
endif

ifeq ($(MIDDLEWARE_NVRAM_ENABLE),y)
ifeq ($(CHIP),ec618)
    MW_SRC_DIRS += $(MW_HOST_DIRS)/nvram/src
else
    MW_SRC_DIRS += $(MW_HOST_DIRS)/nvram/ec7xx/src
endif
#    MW_SRC_DIRS += $(MW_HOST_DIRS)/nvram/data/$(CHIP)
endif


ifeq ($(MIDDLEWARE_CCIO_ENABLE),y)
include $(MW_HOST_DIRS)/ccio/Makefile.inc
MW_SRC_DIRS += $(MW_HOST_DIRS)/ccio/custom/src \
               $(MW_HOST_DIRS)/ccio/device/src \
               $(MW_HOST_DIRS)/ccio/service/src
endif

ifeq ($(MIDDLEWARE_CMS_ENABLE),y)
     MW_SRC_DIRS         += $(MW_HOST_DIRS)/cms/psdial/src \
                            $(MW_HOST_DIRS)/ecapi/psapi/src \
							$(MW_HOST_DIRS)/cms/psil/src \
                            $(MW_HOST_DIRS)/ecapi/appmwapi/src \
                            $(MW_HOST_DIRS)/cms/cms/src  \
                            $(MW_HOST_DIRS)/common/src   \
                            $(MW_HOST_DIRS)/cms/cmsnetlight/src \
							$(MW_HOST_DIRS)/cms/sockmgr/src

     MW_PRIVATE_SRC_DIRS += $(MW_HOST_DIRS)/cms/pslpp/src \
                            $(MW_HOST_DIRS)/cms/pslpp/nbasn1/src \
                            $(MW_HOST_DIRS)/cms/psstk/src \

     #MW_PRIVATE_EXCLUDE_FILES += $(MW_HOST_DIRS)/cms/pslpp/nbasn1/src/perdecenc.c
endif
ifeq ($(MIDDLEWARE_SIMBIP_ENABLE),y)
     MW_PRIVATE_SRC_DIRS += $(MW_HOST_DIRS)/simbip/src
endif

ifeq ($(MIDDLEWARE_FOTA_ENABLE), y)
include $(MW_HOST_DIRS)/fota/Makefile.inc
endif

ifeq ($(MIDDLEWARE_FOTAPAR_ENABLE), y)
CFLAGS_DEFS += -DFEATURE_FOTAPAR_ENABLE

ifeq ($(MIDDLEWARE_FOTA_FS_ENABLE), y)
#'FOTA_NVM_FS_ENABLE' for compatibility, and to be deleted in future!
CFLAGS += -DFEATURE_FOTA_FS_ENABLE -DFOTA_NVM_FS_ENABLE
endif

ifeq ($(MIDDLEWARE_FOTA_SIGN_ENABLE), y)
$(warning make fota sign key####################)
EC_SECURE_TOOL := $(TOP)/PLAT/tools/ecsecure.exe
SYSFOTA_VERIFY_KEY_FILE_SRC := PLAT/middleware/developed/fota/custom/key/fota_key.c
SYSFOTA_KEYPAIR_PEM_FILE := $(TOP)/PLAT/middleware/developed/fota/custom/key/fota_keys.pem
$(TOP)/$(SYSFOTA_VERIFY_KEY_FILE_SRC):
	$(EC_SECURE_TOOL) FOTAKEY=$(TOP)/$(SYSFOTA_VERIFY_KEY_FILE_SRC) BLPEM=$(SYSFOTA_KEYPAIR_PEM_FILE) CODE=1
SYSFOTA_VERIFY_KEY_FILE_OBJ := $(SYSFOTA_VERIFY_KEY_FILE_SRC:.c=.o)
CFLAGS += -DFEATURE_FOTA_SIGN_EANBLE
endif
CFLAGS_INC  += -I $(MW_HOST_DIRS)/fota/pub \
               -I $(MW_HOST_DIRS)/fota/custom/inc

MW_SRC_DIRS += $(MW_HOST_DIRS)/fota/custom/src
endif

ifeq ($(MIDDLEWARE_USB_CCID_ENABLE),y)
CFLAGS += -DFEATURE_USB_CCID_ENABLE
CFLAGS_INC += -I $(MW_HOST_DIRS)/usb_class/ccid/inc
MW_SRC_DIRS += $(MW_HOST_DIRS)/usb_class/ccid/src
endif

ifeq ($(BUILD_PS_ROHC_ENABLE),y)
CFLAGS +=  -DROHC_ENABLE_DEFINE=1
else
CFLAGS +=  -DROHC_ENABLE_DEFINE=0
endif

ifeq ($(BUILD_AT),y)

CFLAGS_INC  += -I $(MW_HOST_DIRS)/at/atdecoder/inc \
              -I $(MW_HOST_DIRS)/at/atps/inc \
              -I $(MW_HOST_DIRS)/at/atps/inc/cnfind \
              -I $(MW_HOST_DIRS)/at/atcust/inc \
              -I $(MW_HOST_DIRS)/at/atcust/inc/cnfind \
              -I $(MW_HOST_DIRS)/at/atentity/inc \
              -I $(MW_HOST_DIRS)/at/atreply/inc

CFLAGS += -DFEATURE_AT_ENABLE

include $(MW_HOST_DIRS)/at/AtCompiler.inc

ATCMD_TABLE_CFILES += $(MW_HOST_DIRS)/at/atcust/src/atec_cust_cmd_table.c
ATCMD_TABLE_CFILES += $(MW_HOST_DIRS)/at/atps/src/atec_cmd_table.c

ifeq ($(BUILD_AT_REF),y)
CFLAGS_INC += -I $(MW_HOST_DIRS)/at/atref/inc \
              -I $(MW_HOST_DIRS)/at/atref/inc/cnfind


MW_SRC_DIRS += $(MW_HOST_DIRS)/at/atref/src	\
               $(MW_HOST_DIRS)/at/atref/src/cnfind

CFLAGS += -DFEATURE_REF_AT_ENABLE

ATCMD_TABLE_CFILES += $(MW_HOST_DIRS)/at/atref/src/atec_ref_cmd_table.c

endif
MW_SRC_DIRS += $(MW_HOST_DIRS)/at/atcust/src/cnfind \
               $(MW_HOST_DIRS)/at/atcust/src \
               $(MW_HOST_DIRS)/at/atentity/src	\
               $(MW_HOST_DIRS)/at/atps/src	\
               $(MW_HOST_DIRS)/at/atps/src/cnfind \
               $(MW_HOST_DIRS)/at/atreply/src     \
			   $(MW_HOST_DIRS)/at/atdecoder/src


#########################################################

ifeq ($(MBTK_OPENCPU_SUPPORT), y)
include $(MW_HOST_DIRS)/Version.inc
include $(MW_HOST_DIRS)/build_config.inc
include $(MW_HOST_DIRS)/$(MBTK_PRODUCT_CHIP)/Makefile.inc

CFLAGS_DEFS += -DMBTK_PRODUCT_BOARD_$(MBTK_PRODUCT_BOARD)
CFLAGS += -DMBTK_PRODUCT_CHIP_$(MBTK_PRODUCT_CHIP)
CFLAGS += -DMBTK_PRODUCT_TARGET_$(MBTK_PRODUCT_TARGET)
CFLAGS += -DMBTK_PRODUCT_BAND_$(MBTK_PRODUCT_BAND)
endif

#########################################################

ifeq ($(BUILD_AT_DEBUG), y)
BUILD_EXCLUDE_ATDEBUG :=
CFLAGS += -DFEATURE_ATDEBUG_ENABLE
else
BUILD_EXCLUDE_ATDEBUG := $(MW_HOST_DIRS)/at/atps/src/atec_debug.c \
                         $(MW_HOST_DIRS)/at/atps/src/atec_tcpip.c
endif

BUILD_EXCLUDE_ATSTUB  := $(MW_HOST_DIRS)/cms/cms/src/cms_at_stub.c



ifeq ($(THIRDPARTY_HTTPC_ENABLE)_$(MBEDTLS_WITH_HTTP_TLS), n_y)
$(error MBEDTLS_WITH_HTTP_TLS depends on THIRDPARTY_HTTPC_ENABLE)
endif



#########################################################
ifeq ($(THIRDPARTY_LIBSNTP_ENABLE), y)
THIRD_PARTY_EXCLUDE_LIBSNTP :=
else
THIRD_PARTY_EXCLUDE_LIBSNTP := 	$(MW_HOST_DIRS)/ecapi/appmwapi/src/reqsntpapi.c \
                             		$(MW_HOST_DIRS)/at/atps/src/atec_sntp.c
endif

#########################################################
ifeq ($(BUILD_PLAT_PER_ECADC_ENABLE), y)
THIRD_PARTY_EXCLUDE_ADC :=
else
THIRD_PARTY_EXCLUDE_ADC := 	$(MW_HOST_DIRS)/at/atcust/src/cnfind/atec_adc_cnf_ind.c \
                                $(MW_HOST_DIRS)/at/atcust/src/atec_adc.c \
                                $(MW_HOST_DIRS)/at/atentity/src/at_adc_task.c
endif


#########################################################
ifeq ($(AT_EXAMPLE_ENABLE), y)
THIRD_PARTY_EXCLUDE_EXAMPLE :=
else
THIRD_PARTY_EXCLUDE_EXAMPLE := 	$(MW_HOST_DIRS)/at/atcust/src/cnfind/atec_example_cnf_ind.c \
                                $(MW_HOST_DIRS)/at/atcust/src/atec_example.c \
                                $(MW_HOST_DIRS)/at/atentity/src/at_example_task.c
endif

#########################################################
ifeq ($(BUILD_PLAT_FS_AT_ENABLE), y)
THIRD_PARTY_EXCLUDE_FILE_AT :=
else
THIRD_PARTY_EXCLUDE_FILE_AT := 	$(MW_HOST_DIRS)/at/atcust/src/cnfind/atec_file_cnf_ind.c \
                                $(MW_HOST_DIRS)/at/atcust/src/atec_file.c \
                                $(MW_HOST_DIRS)/at/atentity/src/at_file_task.c
endif

#########################################################
ifeq ($(MIDDLEWARE_FEATURE_PLATTEST_ENABLE),y)
CFLAGS += -DFEATURE_PLATTEST_ENABLE
BUILD_EXCLUDE_PLATTEST :=
else
BUILD_EXCLUDE_PLATTEST := $(MW_HOST_DIRS)/at/atcust/src/atec_plat_test.c
endif


#########################################################
ifeq ($(THIRDPARTY_CTWING_CERTI_ENABLE), y)
THIRD_PARTY_EXCLUDE_CTWING :=
else
THIRD_PARTY_EXCLUDE_CTWING := $(MW_HOST_DIRS)/at/atentity/src/at_ctwing_task.c \
                             $(MW_HOST_DIRS)/at/atcust/src/cnfind/atec_ctwing_cnf_ind.c \
                             $(MW_HOST_DIRS)/at/atcust/src/atec_ctwing.c
endif

######################For BUILD_AT#############################
endif

#########################################################
ifeq ($(BUILD_PLAT_HTTP_AT_ENABLE)_$(THIRDPARTY_HTTPC_ENABLE), y_y)
THIRD_PARTY_EXCLUDE_HTTPC :=
else
THIRD_PARTY_EXCLUDE_HTTPC := $(MW_HOST_DIRS)/at/atentity/src/at_http_task.c \
                             $(MW_HOST_DIRS)/at/atcust/src/cnfind/atec_http_cnf_ind.c \
                             $(MW_HOST_DIRS)/at/atcust/src/atec_http.c
endif

#########################################################
ifneq ($(BUILD_PLAT_SSL_AT_ENABLE)_$(MBEDTLS_WITH_HTTP_TLS), n_n)
THIRD_PARTY_EXCLUDE_MBEDTLS :=
else
THIRD_PARTY_EXCLUDE_MBEDTLS := $(MW_HOST_DIRS)/ecapi/appmwapi/src/ec_sslcmd_api.c \
                               $(MW_HOST_DIRS)/at/atentity/src/at_ssl_task.c \
                               $(MW_HOST_DIRS)/at/atcust/src/cnfind/atec_ssl_cnf_ind.c \
                               $(MW_HOST_DIRS)/at/atcust/src/atec_ssl.c
endif

#########################################################
ifeq ($(BUILD_PLAT_MQTT_AT_ENABLE)_$(THIRDPARTY_MQTT_ENABLE), y_y)
THIRD_PARTY_EXCLUDE_MQTT :=
else
THIRD_PARTY_EXCLUDE_MQTT := $(MW_HOST_DIRS)/ecapi/appmwapi/src/ec_mqtt_api.c \
                            $(MW_HOST_DIRS)/at/atcust/src/cnfind/atec_mqtt_cnf_ind.c \
                            $(MW_HOST_DIRS)/at/atcust/src/atec_mqtt.c \
                            $(MW_HOST_DIRS)/at/atentity/src/at_mqtt_task.c
endif

#########################################################
ifeq ($(THIRDPARTY_CMCC_DM_ENABLE), y)
THIRD_PARTY_EXCLUDE_DM :=
else
    ifeq ($(THIRDPARTY_CTCC_DM_ENABLE), y)
    THIRD_PARTY_EXCLUDE_DM :=
	else
        ifeq ($(THIRDPARTY_CUCC_DM_ENABLE), y)
        THIRD_PARTY_EXCLUDE_DM :=
        else
        THIRD_PARTY_EXCLUDE_DM := $(MW_HOST_DIRS)/at/atcust/src/atec_dm.c \
                                  $(MW_HOST_DIRS)/at/atcust/src/cnfind/atec_dm_cnf_ind.c
        endif
	endif
endif

#########################################################
ifeq ($(BUILD_PLAT_CTLWM2M_AT_ENABLE), y)
THIRD_PARTY_EXCLUDE_CTWING1_5 :=
else
THIRD_PARTY_EXCLUDE_CTWING1_5 := $(MW_HOST_DIRS)/ecapi/appmwapi/src/ec_ctlwm2m_api.c \
                             $(MW_HOST_DIRS)/at/atentity/src/at_ctlwm2m_task.c \
                             $(MW_HOST_DIRS)/at/atcust/src/cnfind/atec_ctlwm2m_cnf_ind.c \
                             $(MW_HOST_DIRS)/at/atcust/src/atec_ctlwm2m1_5.c
endif

#########################################################
ifeq ($(BUILD_PLAT_ONENET_AT_ENABLE), y)
THIRD_PARTY_EXCLUDE_ONENET :=
else
THIRD_PARTY_EXCLUDE_ONENET := $(MW_HOST_DIRS)/at/atentity/src/at_onenet_task.c \
                             $(MW_HOST_DIRS)/at/atcust/src/cnfind/atec_onenet_cnf_ind.c \
                             $(MW_HOST_DIRS)/at/atcust/src/atec_onenet.c
endif

#########################################################
ifeq ($(BUILD_PS_TCPIP_API_ENABLE), y)
THIRD_PARTY_EXCLUDE_PS_TCPIP_API :=
else
THIRD_PARTY_EXCLUDE_PS_TCPIP_API := $(MW_HOST_DIRS)/ecapi/appmwapi/src/ec_tcpip_api.c \
                                    $(MW_HOST_DIRS)/ecapi/appmwapi/src/ec_tcpip_api_entity.c
endif

#########################################################
ifeq ($(DRIVER_VEM_CFG_ENABLE), y)
MW_EXCLUDE_NVM_AUDIO_CFG :=
else
MW_EXCLUDE_NVM_AUDIO_CFG := $(MW_HOST_DIRS)/common/src/mw_nvm_audio.c
endif

ifeq ($(BUILD_PLAT_MIFI_AT_ENABLE), y)
THIRD_PARTY_EXCLUDE_MIFI_AT :=
MW_EXCLUDE_NVM_MIFI_CFG :=
else
THIRD_PARTY_EXCLUDE_MIFI_AT := $(MW_HOST_DIRS)/at/atcust/src/atec_mifi.c
MW_EXCLUDE_NVM_MIFI_CFG :=$(MW_HOST_DIRS)/common/src/mw_nvm_mifi.c
endif

#########################################################
MW_EXCLUDE_CCIO      :=


MW_EXCLUDE_FILES := $(THIRD_PARTY_EXCLUDE_MQTT)      \
                    $(THIRD_PARTY_EXCLUDE_HTTPC)     \
                    $(THIRD_PARTY_EXCLUDE_MBEDTLS)   \
                    $(THIRD_PARTY_EXCLUDE_LIBSNTP)   \
                    $(THIRD_PARTY_EXCLUDE_EXAMPLE)   \
                    $(BUILD_EXCLUDE_ATDEBUG)         \
                    $(BUILD_EXCLUDE_ATSTUB)          \
                    $(BUILD_EXCLUDE_PLATTEST)        \
                    $(THIRD_PARTY_EXCLUDE_ADC)       \
                    $(THIRD_PARTY_EXCLUDE_DM)        \
                    $(MW_EXCLUDE_CCIO)               \
                    $(THIRD_PARTY_EXCLUDE_CTWING)    \
                    $(THIRD_PARTY_EXCLUDE_CTWING1_5)    \
                    $(THIRD_PARTY_EXCLUDE_ONENET)    \
                    $(MW_EXCLUDE_NVM_AUDIO_CFG) \
                    $(THIRD_PARTY_EXCLUDE_PS_TCPIP_API) \
                    $(THIRD_PARTY_EXCLUDE_MIFI_AT) \
                    $(MW_EXCLUDE_NVM_MIFI_CFG)


ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

$(BUILDDIR)/lib/libmiddleware_ec.a: $(MW_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

$(BUILDDIR)/lib/libmiddleware_ec_private.a: $(MW_PRIVATE_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

## if no objs is needed to compile, just not intitate the compile process
ifneq ($(MW_COBJS),)
	lib-y += libmiddleware_ec.lib
endif
ifneq ($(MW_PRIVATE_COBJS),)
ifeq ($(BUILD_USE_PREBUILD_LIB),n)
	lib-y += libmiddleware_ec_private.lib
endif
endif

$(BUILDDIR)/lib/libmiddleware_ec.lib: $(MW_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

$(BUILDDIR)/lib/libmiddleware_ec_private.lib: $(MW_PRIVATE_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif

