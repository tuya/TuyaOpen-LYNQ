#---------------------------------------------------------------------------------
##Description：
##global option for Plat config
##Use method: y is support;n is not support
#---------------------------------------------------------------------------------
CFLAGS_DEFS += -DRAMCODE_COMPRESS_EN
CFLAGS_DEFS += -DREL_COMPRESS_EN


ifeq ($(EUTRAN_MODE), cat_mode)
EUTRAN_CAT_MODE     = y
endif

#if need usb auto adapt support, set to y
#USBNET_AUTO_ADAPT_ENABLE=y

#enable below setting if OPENCPU_MODE is needed
ifeq ($(OPENCPU), true)
OPENCPU_MODE_ENABLE = y
endif

ifeq ($(GCF_ENABLE), true)
GCF_FEATURE_ENABLE = y
endif

ifeq ($(MID_ENABLE), true)
MID_FEATURE_ENABLE = y
endif

ifeq ($(IMS_ENABLE), true)
IMS_MODE_ENABLE = y
endif

ifeq ($(IMS_SMSONLY_ENABLE), true)
IMS_SMSONLY_MODE_ENABLE = y
endif

ifeq ($(AUDIO_ENABLE), true)
AUDIO_MODE_ENABLE = y
endif

ifeq ($(TYPE), ec716s)
ifeq ($(RAM_ENBALE), true)
MORE_RAM_ENABLE = y
endif
endif

ifeq ($(ROM_ENABLE), true)
MORE_ROM_ENABLE = y
ifeq ($(LESS_LOG), true)
LESS_LOG_ENABLE = y
endif
endif


THIRDPARTY_LZMA_ENABLE       = y
BIN_COMPRESS                 = y

DRIVER_ADC_ENABLE            = y
DRIVER_SPI_ENABLE            = n
DRIVER_I2C_ENABLE            = y
DRIVER_I2S_ENABLE            = n
DRIVER_LCD_ENABLE            = n
DRIVER_CPFLASH_ENABLE        = n
DRIVER_VPU_ENABLE            = n
DRIVER_EEPROM_ENABLE         = n
DRIVER_PSRAM_ENABLE          = n
DRIVER_PCACHE_ENABLE         = n
DRIVER_ONEWIRE_ENABLE        = n
DRIVER_ACVOICE_ENABLE        = n
DRIVER_EXSTORAGE_ENABLE      = n

MIDDLEWARE_USB_CCID_ENABLE   = n
MIDDLEWARE_FOTAPAR_ENABLE    = y
MIDDLEWARE_FOTA_FS_ENABLE    = n

BUILD_EC_MW                  = n
AT_EXAMPLE_ENABLE            = n
BUILD_PS_ROHC_ENABLE         = n

#--------------------------------------------
#Thirdparty Feature config
#--------------------------------------------

THIRDPARTY_HTTPC_ENABLE        = y
MBEDTLS_WITH_HTTP_TLS          = n
THIRDPARTY_MQTT_ENABLE         = n
MBEDTLS_WITH_MQTT_TLS          = n
THIRDPARTY_MBEDTLS_ENABLE      = n

#cjson is refered by MQTT
THIRDPARTY_CJSON_ENABLE        = n
THIRDPARTY_CTCC_DM_ENABLE      = n
THIRDPARTY_CUCC_DM_ENABLE      = n
THIRDPARTY_CMCC_DM_ENABLE      = n
THIRDPARTY_CTWING_CERTI_ENABLE = n

THIRDPARTY_CISONENET_ENABLE       = n

THIRDPARTY_WAKAAMA_ENABLE         = n
THIRDPARTY_CTWING1_5_ENABLE       = n
THIRDPARTY_ERCOAP_ENABLE          = n

THIRDPARTY_LIBSNTP_ENABLE         = y
THIRDPARTY_PING_ENABLE            = y
THIRDPARTY_YRCOMPRESS_ENABLE      = y
THIRDPARTY_IPERF_ENABLE           = n
THIRDPARTY_PPP_ENABLE             = n


ifeq ($(THIRDPARTY_CMCC_DM_ENABLE), y)
THIRDPARTY_CISONENET_ENABLE    = y
THIRDPARTY_ERCOAP_ENABLE       = y
endif

ifeq ($(MID_FEATURE_ENABLE), y)
THIRDPARTY_DHCPD_ENABLE  = n
else ifeq ($(MORE_RAM_ENABLE), y)
THIRDPARTY_DHCPD_ENABLE  = n
else ifeq ($(MORE_ROM_ENABLE), y)
THIRDPARTY_DHCPD_ENABLE  = n
else
THIRDPARTY_DHCPD_ENABLE  = y
endif



ifeq ($(EXCEPTION_FLASH_DUMP_ENABLE), y)
CFLAGS_DEFS += -DFEATURE_EXCEPTION_FLASH_DUMP_ENABLE
endif

ifeq ($(MIDDLEWARE_AMR_ENABLE), y)
CFLAGS_DEFS += -DFEATURE_AMR_CP_ENABLE
endif

ifeq ($(MIDDLEWARE_VEM_ENABLE), y)
CFLAGS_DEFS += -DFEATURE_VEM_CP_ENABLE
endif

ifeq ($(THIRDPARTY_JPEGCOMP_ENABLE),y)
CFLAGS_DEFS += -DFEATURE_JPEGCOMP_ENABLE
endif

ifeq ($(IMS_ENABLE), true)
CFLAGS_DEFS += -DFEATURE_IMS_ENABLE
CFLAGS_DEFS += -DFEATURE_IMS_CC_ENABLE
CFLAGS_DEFS += -DFEATURE_IMS_SMS_ENABLE
CFLAGS_DEFS += -DFEATURE_IMS_USE_PSRAM_ENABLE
CFLAGS_DEFS += -fno-strict-aliasing
endif

ifeq ($(IMS_UT_ENABLE), true)
CFLAGS_DEFS += -DFEATURE_IMS_UT_ENABLE
endif

ifeq ($(IMS_SMSONLY_ENABLE), true)
CFLAGS_DEFS += -DFEATURE_IMS_ENABLE
CFLAGS_DEFS += -DFEATURE_IMS_SMS_ENABLE
CFLAGS_DEFS += -fno-strict-aliasing
#        NO FEATURE_IMS_CC_ENABLE
endif

ifeq ($(AUDIO_ENABLE), true)
ifeq ($(BUILD_SUPPORT_APP_PCM_MEM_POOL), y)
CFLAGS_DEFS += -DFEATURE_SUPPORT_APP_PCM_MEM_POOL
endif
CFLAGS_DEFS += -DFEATURE_AUDIO_ENABLE
endif

ifneq ($(findstring $(TYPE), ec716s ec716e),)
ifeq ($(RAM_ENBALE), true)
CFLAGS_DEFS += -DFEATURE_MORERAM_ENABLE
endif
endif

ifeq ($(ROM_ENABLE), true)
CFLAGS_DEFS += -DFEATURE_MOREROM_ENABLE
ifeq ($(LESS_LOG), true)
CFLAGS_DEFS += -DFEATURE_LESSLOG_ENABLE
endif
endif


ifeq ($(THIRDPARTY_CMCC_DM_ENABLE), y)
THIRDPARTY_CISONENET_ENABLE    = y
THIRDPARTY_ERCOAP_ENABLE       = y
endif

#ensure AT command to be consistent with protocol
#such as mqtt/...
ifeq ($(THIRDPARTY_MQTT_ENABLE), n)
BUILD_PLAT_MQTT_AT_ENABLE      = n
endif

ifeq ($(THIRDPARTY_HTTPC_ENABLE), n)
BUILD_PLAT_HTTP_AT_ENABLE      = n
endif

ifeq ($(THIRDPARTY_MBEDTLS_ENABLE), n)
BUILD_PLAT_SSL_AT_ENABLE      = n
endif

ifeq ($(MIDDLEWARE_FOTAPAR_ENABLE), n)
BUILD_PLAT_FOTA_AT_ENABLE     = n
BUILD_PLAT_ECOTA_AT_ENABLE    = n
endif


BUILD_PS_TCPIP_API_ENABLE      = n

BUILD_AT                       = y
BUILD_AT_DEBUG                 = n
BUILD_AT_REF                   = y

MBTK_OPENCPU_SMS               = n
MBTK_OPENCPU_AT                = y
MBTK_OPENCPU_SSL               = n
MBTK_OPENCPU_TCP               = n

MBTK_OPENCPU_SUPPORT = y
CFLAGS += -DMBTK_OPENCPU_SUPPORT
CFLAGS += -DMBTK_OPENCPU_SUPPORT_USBSERL

#enable wdt
CFLAGS += -DWDT_FEATURE_ENABLE=1
CFLAGS += -DFEATURE_UART_HELP_DUMP_ENABLE

#Enable SIM hotswap feature with pad configration and jitter handle by required----not remove it
#CFLAGS += -DSIM_HOT_SWAP_FEATURE

##################################################################################################################
##Description:
##global option for RIL APIs
##Use method: y is support;n is not support
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_AT), y)
BUILD_RIL_AT_API_ENABLE      = y
endif

##################################################################################################################
##Description:
##global option for Plat AT commands
##Use method: y is support;n is not support
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_AT), y)
BUILD_PLAT_CUST_AT_ENABLE    = y
ifeq ($(PWR_TEST), y)
BUILD_PLAT_MEM_AT_ENABLE     = y
else
BUILD_PLAT_MEM_AT_ENABLE     = n
endif
BUILD_PLAT_PMU_AT_ENABLE     = y
BUILD_PLAT_PER_AT_ENABLE     = y
BUILD_PLAT_MISC_AT_ENABLE    = y
BUILD_PLAT_PROD_AT_ENABLE    = y

BUILD_PLAT_FOTA_AT_ENABLE    = y
BUILD_PLAT_ECOTA_AT_ENABLE   = n
BUILD_PLAT_CMUX_AT_ENABLE    = n
BUILD_PLAT_HTTP_AT_ENABLE    = n
BUILD_PLAT_MQTT_AT_ENABLE    = n
BUILD_PLAT_SSL_AT_ENABLE     = n
BUILD_PLAT_FS_AT_ENABLE      = n
BUILD_PLAT_AUDIO_AT_ENABLE   = n
BUILD_PLAT_ONENET_AT_ENABLE  = n
BUILD_PLAT_CTLWM2M_AT_ENABLE = n
endif

##################################################################################################################
##Description: BUILD_PLAT_CUST_AT_ENABLE
##global option for Plat product related AT commands
##Use method: y is support;n is not support
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_PLAT_CUST_AT_ENABLE), y)
#AT+CGMI
BUILD_PLAT_CUST_CGMI_ENABLE   = y
#AT+CGMM
BUILD_PLAT_CUST_CGMM_ENABLE   = y
#AT+GMM
BUILD_PLAT_CUST_GMM_ENABLE    = y
#AT+CGMR
BUILD_PLAT_CUST_CGMR_ENABLE   = y
#AT+CGSN
BUILD_PLAT_CUST_CGSN_ENABLE   = y
#ATI
BUILD_PLAT_CUST_ATI_ENABLE    = y
#AT&W
BUILD_PLAT_CUST_ATnW_ENABLE   = n
#AT&F
BUILD_PLAT_CUST_ATnF_ENABLE   = n
#AT&V
BUILD_PLAT_CUST_ATnV_ENABLE   = n
endif

##################################################################################################################
##Description: BUILD_PLAT_MEM_AT_ENABLE
##global option for Plat memory related AT commands
##Use method: y is support;n is not support
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_PLAT_MEM_AT_ENABLE), y)
#AT+ECSHOWMEM
BUILD_PLAT_MEM_ECSHOWMEM_ENABLE           = y
#AT+ECHEAPINFO
BUILD_PLAT_MEM_ECHEAPINFO_ENABLE          = y
#AT+ECMEM32
BUILD_PLAT_MEM_ECMEM32_ENABLE             = y
#AT+ECDLFCMEM
BUILD_PLAT_MEM_ECDLFCMEM_ENABLE           = y
#AT+ECFSINFO
BUILD_PLAT_MEM_ECFSINFO_ENABLE            = y
#AT+RLCHK
BUILD_PLAT_MEM_RLCHK_ENABLE               = y
#AT+ECFLASHMONITORINFO
BUILD_PLAT_MEM_ECFLASHMONITORINFO_ENABLE  = y
endif

##################################################################################################################
##Description: BUILD_PLAT_PMU_AT_ENABLE
##global option for Plat pmu related AT commands
##Use method: y is support;n is not support
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_PLAT_PMU_AT_ENABLE), y)
#AT+ECPMUCFG
BUILD_PLAT_PMU_ECPMUCFG_ENABLE     = y
#AT+ECSYSMONCFG
BUILD_PLAT_PMU_ECSYSMONCFG_ENABLE  = y
#AT+ECVOTECHK
BUILD_PLAT_PMU_ECVOTECHK_ENABLE    = y
#AT+ECPMUSTATUS
BUILD_PLAT_PMU_ECPMUSTATUS_ENABLE  = y
#AT+ECPURC
BUILD_PLAT_PMU_ECPURC_ENABLE       = y
#AT+ECSCLK
BUILD_PLAT_PMU_ECSCLK_ENABLE       = n
endif

##################################################################################################################
##Description: BUILD_PLAT_PER_AT_ENABLE
##global option for Plat peripheral related AT commands
##Use method: y is support;n is not support
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_PLAT_PER_AT_ENABLE), y)
#AT+IPR
BUILD_PLAT_PER_IPR_ENABLE        = y
#AT+ICF
BUILD_PLAT_PER_ICF_ENABLE        = n
#AT+IFC
BUILD_PLAT_PER_IFC_ENABLE        = y
#AT+ECUSBSYS
BUILD_PLAT_PER_ECUSBSYS_ENABLE   = y
#AT+ECLEDMODE
BUILD_PLAT_PER_ECLEDMODE_ENABLE  = n
#AT+ECPALARM
BUILD_PLAT_PER_ECPALARM_ENABLE   = n
#AT+ECADC
BUILD_PLAT_PER_ECADC_ENABLE      = y
endif

##################################################################################################################
##Description: BUILD_PLAT_MISC_AT_ENABLE
##global option for Plat miscellaneous AT commands
##Use method: y is support;n is not support
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_PLAT_MISC_AT_ENABLE), y)
#AT+ECPCFG
BUILD_PLAT_MISC_ECPCFG_ENABLE          = y
#AT+ECRST
BUILD_PLAT_MISC_ECRST_ENABLE           = y
#AT+ECPOWD
BUILD_PLAT_MISC_ECPOWD_ENABLE          = n
#AT^SYSTEST
BUILD_PLAT_MISC_SYSTEST_ENABLE         = n
#AT+ECSYSTEST
BUILD_PLAT_MISC_ECSYSTEST_ENABLE       = y
#AT+ECTASKINFO
BUILD_PLAT_MISC_ECTASKINFO_ENABLE      = n
#AT+ECTASKHISTINFO
BUILD_PLAT_MISC_ECTASKHISTINFO_ENABLE  = n
#AT+ECLOGDBVER
BUILD_PLAT_MISC_ECLOGDBVER_ENABLE      = y
#AT+ECDUMPCHK
BUILD_PLAT_MISC_ECDUMPCHK_ENABLE       = n
#AT+ECBTOFFSETDBG
BUILD_PLAT_MISC_ECBTOFFSETDBG_ENABLE   = n
#AT+ECFUSEMR
BUILD_PLAT_MISC_ECFUSEMR_ENABLE        = y
#AT+ECSIMO
BUILD_PLAT_MISC_ECSIMO_ENABLE          = n
#AT+ECPRFINFO
BUILD_PLAT_MISC_ECPRFINFO_ENABLE       = n
endif

##################################################################################################################
##Description: BUILD_PLAT_PROD_AT_ENABLE
##global option for Plat product related AT commands
##Use method: y is support;n is not support
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_PLAT_PROD_AT_ENABLE), y)
#AT+ECATE
BUILD_PLAT_PROD_ECATE_ENABLE       = y
#AT+ECVERSION
BUILD_PLAT_PROD_ECVERSION_ENABLE   = y
#AT+ECIPR
BUILD_PLAT_PROD_ECIPR_ENABLE       = y
#AT+ECSLEEP
BUILD_PLAT_PROD_ECSLEEP_ENABLE     = y
#AT+ECSAVEFAC
BUILD_PLAT_PROD_ECSAVEFAC_ENABLE   = y
#AT+ECRFTEST
BUILD_PLAT_PROD_ECRFTEST_ENABLE    = y
#AT+ECGMDATA
BUILD_PLAT_PROD_ECGMDATA_ENABLE    = y
#AT+ECNPICFG
BUILD_PLAT_PROD_ECNPICFG_ENABLE    = y
#AT+ECRFNST
BUILD_PLAT_PROD_ECRFNST_ENABLE     = y
#AT+ECRFSTAT
BUILD_PLAT_PROD_ECRFSTAT_ENABLE    = y
#AT+ECPRODMODE
BUILD_PLAT_PROD_ECPRODMODE_ENABLE  = y
endif

##################################################################################################################
##Description:
##global option for PS AT commands
##Use method: y is support;n is not support
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_AT), y)
BUILD_PS_GEN_AT_ENABLE  = y
BUILD_PS_DEV_AT_ENABLE  = y
BUILD_PS_EMM_AT_ENABLE  = y
BUILD_PS_EPS_AT_ENABLE  = y
BUILD_PS_SIM_AT_ENABLE  = y
BUILD_PS_SMS_AT_ENABLE  = y
BUILD_PHY_CONFIG_AT_ENABLE = y
BUILD_PS_TCPIP_ECSOCK_AT_ENABLE  = n
BUILD_PS_TCPIP_SKTSOCK_AT_ENABLE = n
BUILD_PS_TCPIP_NETCONFIG_AT_ENABLE = y
BUILD_PS_TCPIP_IPSEC_AT_ENABLE   = n
endif
##################################################################################################################
##name :BUILD_PS_GEN_AT_ENABLE
##Description:
##BUILD_PS_GEN_AT_ENABLE is to control whether include ps  global AT commands as: AT/ATQ/ATE/ATT/ATV/ATL/ATM/...etc
##Use method: y is support;n is not support.
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_PS_GEN_AT_ENABLE), y)
#AT via BUILD_PS_GEN_AT_ENABLE
#ATQ
BUILD_PS_GEN_AT_Q_ENABLE      = n
#ATE
BUILD_PS_GEN_AT_E_ENABLE      = y
#ATT
BUILD_PS_GEN_AT_T_ENABLE      = n
#ATV
BUILD_PS_GEN_AT_V_ENABLE      = n
#ATL
BUILD_PS_GEN_AT_L_ENABLE      = n
#ATM
BUILD_PS_GEN_AT_M_ENABLE      = n
#ATS0
BUILD_PS_GEN_AT_S0_ENABLE     = n
#AT&C
BUILD_PS_GEN_AT_nC_ENABLE     = n
#AT&D
BUILD_PS_GEN_AT_nD_ENABLE     = n
#ATD
BUILD_PS_GEN_AT_D_ENABLE      = n
#ATO
BUILD_PS_GEN_AT_O_ENABLE      = n
#ATH
BUILD_PS_GEN_AT_H_ENABLE      = n
#ATA
BUILD_PS_GEN_AT_A_ENABLE      = n
#AT+CSCS
BUILD_PS_GEN_CSCS_ENABLE      = n
#AT+CMEE
BUILD_PS_GEN_CMEE_ENABLE      = n
#AT+ECURC
BUILD_PS_GEN_ECURC_ENABLE     = y
#AT+ECURCCFG
BUILD_PS_GEN_ECURCCFG_ENABLE  = n
#AT+ECPPPHUP
BUILD_PS_GEN_ECPPPHUP_ENABLE  = n
#AT+CLCK
BUILD_PS_GEN_CLCK_ENABLE      = n
endif

##################################################################################################################
##name :BUILD_PS_DEV_AT_ENABLE
##Description:
##BUILD_PS_DEV_AT_ENABLE is to control whether include ps  global AT commands as: AT+CFUN/ECBAND/ECCFG/ECSTATUS/...etc
##Use method: y is support;n is not support.
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_PS_DEV_AT_ENABLE), y)
#AT+CFUN
BUILD_PS_DEV_CFUN_ENABLE    = y
#AT+ECCGSN
BUILD_PS_DEV_ECCGSN_ENABLE  = y
#AT+ECCGSNLOCK
BUILD_PS_DEV_ECCGSNLOCK_ENABLE = n
#AT+ECNETDEVMAC
BUILD_PS_DEV_ECNETDEVMAC_ENABLE = y
#AT+ECSTATUS
BUILD_PS_DEV_ECSTATUS_ENABLE  = y
#AT+ECSTATIS
BUILD_PS_DEV_ECSTATIS_ENABLE  = n
#AT+ECBAND
BUILD_PS_DEV_ECBAND_ENABLE    = y
#AT+ECFREQ
BUILD_PS_DEV_ECFREQ_ENABLE    = y
#AT+ECCFG
BUILD_PS_DEV_ECCFG_ENABLE     = y
#AT+ECMEASCFG
BUILD_PS_DEV_ECMEASCFG_ENABLE = n
#AT+ECRMFPLMN
BUILD_PS_DEV_ECRMFPLMN_ENABLE = y
#AT+ECBLOCKPLMNLIST
FEATURE_PS_DEV_ECBLOCKPLMNLIST_ENABLE = y
#AT+ECBCINFO
BUILD_PS_DEV_ECBCINFO_ENABLE  = y
#AT+ECPSTEST
BUILD_PS_DEV_ECPSTEST_ENABLE  = n
#AT+ECEVENTSTATIS
BUILD_PS_DEV_ECEVENTSTATIS_ENABLE  = n
#AT+ECNASTCFG
BUILD_PS_DEV_ECNASTCFG_ENABLE   = y
#AT+ECWIFISCAN
BUILD_PS_DEV_ECWIFISCAN_ENABLE  = n
#AT+ECBARCELL
BUILD_PS_DEV_ECBARCELL_ENABLE  = y
#AT+ECJDC
BUILD_PS_DEV_ECJDC_ENABLE = n
endif

##################################################################################################################
##name :BUILD_PS_EMM_AT_ENABLE
##Description:
##BUILD_PS_EMM_AT_ENABLE is to control whether include ps  mm AT commands as: AT+CESQ/COPS/CCIOTOPT/ECCESQS/...etc
##Use method: y is support;n is not support
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_PS_EMM_AT_ENABLE), y)
#AT+CSQ
BUILD_PS_EMM_CSQ_ENABLE   = y
#AT+CESQ
BUILD_PS_EMM_CESQ_ENABLE  = n
#AT+CREG
BUILD_PS_EMM_CREG_ENABLE  = y
#AT+COPS
BUILD_PS_EMM_COPS_ENABLE  = y
#AT+CPSMS
BUILD_PS_EMM_CPSMS_ENABLE     = y
#AT+CEDRXS
BUILD_PS_EMM_CEDRXS_ENABLE    = n
#AT+CEDRXRDP
BUILD_PS_EMM_CEDRXRDP_ENABLE  = n
#AT+CCIOTOPT
BUILD_PS_EMM_CCIOTOPT_ENABLE  = n
#AT+CRCES
BUILD_PS_EMM_CRCES_ENABLE = n
#AT+CCLK
BUILD_PS_EMM_CCLK_ENABLE  = y
#AT+CTZR
BUILD_PS_EMM_CTZR_ENABLE  = n
#AT+CTZU
BUILD_PS_EMM_CTZU_ENABLE  = n
#AT+ECPLMNS
BUILD_PS_EMM_ECPLMNS_ENABLE     = n
#AT+ECCESQSn
BUILD_PS_EMM_ECCESQS_ENABLE     = n
#AT+ECPSMR
BUILD_PS_EMM_ECPSMR_ENABLE      = n
#AT+ECPTWEDRXS
BUILD_PS_EMM_ECPTWEDRXS_ENABLE  = n
#AT+ECEMMTIME
BUILD_PS_EMM_ECEMMTIME_ENABLE   = n
#AT+ECPLMNRESELECT
BUILD_PS_EMM_ECPLMNRESELECT_ENABLE  = n
#AT+ECCONNREL
BUILD_PS_EMM_ECCONNREL_ENABLE  = n
#AT+ECMMER
BUILD_PS_EMM_ECMMER_ENABLE   = y
#AT+ECEHPLMNLIST
BUILD_PS_EMM_ECEHPLMNLIST_ENABLE   = y
endif

##################################################################################################################
##name :BUILD_PS_EPS_AT_ENABLE
##Description:
##BUILD_PS_EPS_AT_ENABLE is to control whether include ESP  AT commands as: AT+CGATT/CGDCONT/CGACT/CGCONTRDP/CGAUTH..etc
##Use method: y is support;n is not support
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_PS_EPS_AT_ENABLE), y)
#AT+CGATT
BUILD_PS_EPS_CGATT_ENABLE  = y
#AT+ECEMCATT
BUILD_PS_EPS_ECEMCATT_ENABLE  = n
#AT+CGDATA
BUILD_PS_EPS_CGDATA_ENABLE = n

#AT+CGTFT
BUILD_PS_EPS_CGTFT_ENABLE  = y
#AT+CGCMOD
BUILD_PS_EPS_CGCMOD_ENABLE = y
#AT+CGEQOS
BUILD_PS_EPS_CGEQOS_ENABLE  = y
#AT+CGDSCONT
BUILD_PS_EPS_CGDSCONT_ENABLE = y

#AT+CEMODE
BUILD_PS_EPS_CEMODE_ENABLE = y
#AT+CGDCONT
BUILD_PS_EPS_CGDCONT_ENABLE = y
#AT+CGACT
BUILD_PS_EPS_CGACT_ENABLE   = y
#AT+CGEQOSRDP
BUILD_PS_EPS_CGEQOSRDP_ENABLE = n
#AT+CGCONTRDP
BUILD_PS_EPS_CGCONTRDP_ENABLE = n
#AT+CGSCONTRDP
BUILD_PS_EPS_CGSCONTRDP_ENABLE = n
#AT+CGTFTRDP
BUILD_PS_EPS_CGTFTRDP_ENABLE   = n
#AT+CEREG
BUILD_PS_EPS_CEREG_ENABLE = y
#AT+CSCON
BUILD_PS_EPS_CSCON_ENABLE = n
#AT+CSODCP
BUILD_PS_EPS_CSODCP_ENABLE = n
#AT+CRTDCP
BUILD_PS_EPS_CRTDCP_ENABLE = n
#AT+CGAUTH
BUILD_PS_EPS_CGAUTH_ENABLE = y
#AT+CIPCA
BUILD_PS_EPS_CIPCA_ENABLE  = n
#AT+CGAPNRC
BUILD_PS_EPS_CGAPNRC_ENABLE = n
#AT+CGEREP
BUILD_PS_EPS_CGEREP_ENABLE  = n
#AT+CGPADDR
BUILD_PS_EPS_CGPADDR_ENABLE  = y
#AT+CEER
BUILD_PS_EPS_CEER_ENABLE   = n
#AT+CEVDP
BUILD_PS_EPS_CEVDP_ENABLE = n
#AT+CPSDO
BUILD_PS_EPS_CPSDO_ENABLE  = n
#AT+ECCIOTPLANE
BUILD_PS_EPS_ECCIOTPLANE_ENABLE = n
#AT+ECSENDDATA
BUILD_PS_EPS_ECSENDDATA_ENABLE  = n
#AT+ECGDCNT
BUILD_PS_EPS_ECGDCNT_ENABLE     = n
#AT+ECAUGDCNT
BUILD_PS_EPS_ECAUGDCNT_ENABLE   = n
#AT+ECSCLKEX
BUILD_PS_EPS_ECSCLKEX_ENABLE    = n
#AT+ECSMER
BUILD_PS_EPS_ECSMER_ENABLE   = y
endif

##################################################################################################################
##name :BUILD_PS_SIM_AT_ENABLE
##Description:
##BUILD_PS_SIM_AT_ENABLE is to control whether include SIM AT commands as: AT+CIMI/CPIN/ECICCID/ECSIMCFG..etc
##Use method: y is support;n is not support
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_PS_SIM_AT_ENABLE), y)
#AT+CIMI
BUILD_PS_SIM_AT_CIMI_ENABLE         = y
#AT+CPIN
BUILD_PS_SIM_AT_CPIN_ENABLE         = y
#AT+ECICCID
BUILD_PS_SIM_AT_ECICCID_ENABLE      = y
#AT+ECSIMCFG
BUILD_PS_SIM_AT_ECSIMCFG_ENABLE     = y
#AT+CPWD
BUILD_PS_SIM_AT_CPWD_ENABLE         = n
#AT+CPINR
BUILD_PS_SIM_AT_CPINR_ENABLE        = n
#AT+ECSIMSLEEP
BUILD_PS_SIM_AT_ECSIMSLEEP_ENABLE   = n
#AT+CSIM
BUILD_PS_SIM_AT_CSIM_ENABLE         = n
#AT+CRSM
BUILD_PS_SIM_AT_CRSM_ENABLE         = n
#AT+CCHO
BUILD_PS_SIM_AT_CCHO_ENABLE         = n
#AT+CGLA
BUILD_PS_SIM_AT_CGLA_ENABLE         = n
#AT+CCHC
BUILD_PS_SIM_AT_CCHC_ENABLE         = n
#AT+ECSWC
BUILD_PS_SIM_AT_ECSWC_ENABLE        = n
#AT+ECSIMRM
BUILD_PS_SIM_AT_ECSIMRM_ENABLE      = y
#AT+CNUM
BUILD_PS_SIM_AT_CNUM_ENABLE         = n
#AT+ECUSATP
BUILD_PS_SIM_AT_ECUSATP_ENABLE      = n
#AT+CPOL
BUILD_PS_SIM_AT_CPOL_ENABLE         = n
#AT+CPLS
BUILD_PS_SIM_AT_CPLS_ENABLE         = n
endif

##################################################################################################################
##name :BUILD_PS_SMS_AT_ENABLE
##Description:
##BUILD_PS_SMS_AT_ENABLE is to control whether include ps SMS AT commands as: AT+CMGS/CMGR/CMGW/CNMI/...etc
##Use method: y is support; n is not support
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_PS_SMS_AT_ENABLE), y)
#AT+CMGS
BUILD_PS_SMS_CMGS_ENABLE   = y
#AT+CMGC
BUILD_PS_SMS_CMGC_ENABLE   = n
#AT+CMGR
BUILD_PS_SMS_CMGR_ENABLE   = y
#AT+CMGW
BUILD_PS_SMS_CMGW_ENABLE   = y
#AT+CSCA
BUILD_PS_SMS_CSCA_ENABLE   = y
#AT+CNMI
BUILD_PS_SMS_CNMI_ENABLE   = y
#AT+CMMS
BUILD_PS_SMS_CMMS_ENABLE   = y
#AT+CMGD
BUILD_PS_SMS_CMGD_ENABLE   = y
#AT+CMGL
BUILD_PS_SMS_CMGL_ENABLE   = n
#AT+CSMS
BUILD_PS_SMS_CSMS_ENABLE   = y
#AT+CPMS
BUILD_PS_SMS_CPMS_ENABLE   = y
#AT+CNMA
BUILD_PS_SMS_CNMA_ENABLE   = n
#AT+CMGF
BUILD_PS_SMS_CMGF_ENABLE   = y
#AT+CSMP
BUILD_PS_SMS_CSMP_ENABLE   = y
#AT+CSCB
BUILD_PS_SMS_CSCB_ENABLE   = n
#AT+CSDH
BUILD_PS_SMS_CSDH_ENABLE   = n
#AT+CMSS
BUILD_PS_SMS_CMSS_ENABLE   = y
#AT+ECSMSSEND
BUILD_PS_SMS_ECSMSSEND_ENABLE   = n
#AT+ECCMGS
BUILD_PS_SMS_ECCMGS_ENABLE   = n
#AT+ECCMGR
BUILD_PS_SMS_ECCMGR_ENABLE   = n
endif

##################################################################################################################
##name :BUILD_PHY_CONFIG_AT_ENABLE
##Description:
##BUILD_PHY_CONFIG_AT_ENABLE is to control whether include PHY CONFIG AT commands
##Use method: y is support;n is not support
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_PHY_CONFIG_AT_ENABLE), y)
#AT+ECPHYCFG
BUILD_PHY_CONFIG_ECPHYCFG_ENABLE   = y
#AT+ECEDRXSIMU
BUILD_PHY_CONFIG_ECEDRXSIMU_ENABLE = n
endif

##################################################################################################################
##name :BUILD_PS_TCPIP_ECSOCK_AT_ENABLE
##Description:
##BUILD_PS_TCPIP_ECSOCK_AT_ENABLE is to control whether include TCPIP EC SOCKET AT commands
##Use method: y is support;n is not support
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_PS_TCPIP_ECSOCK_AT_ENABLE), y)
# do nothing here
endif
##################################################################################################################
##name :BUILD_PS_TCPIP_SKTSOCK_AT_ENABLE
##Description:
##BUILD_PS_TCPIP_ECSOCK_AT_ENABLE is to control whether include TCPIP SKT SOCKET AT commands
##Use method: y is support;n is not support
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_PS_TCPIP_SKTSOCK_AT_ENABLE), y)
# do nothing here
endif
##################################################################################################################
##name :BUILD_PS_TCPIP_API_ENABLE
##Description:
##BUILD_PS_TCPIP_API_ENABLE is to control whether include EC TCPIP API
##Use method: y is support;n is not support
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_PS_TCPIP_API_ENABLE), y)
# do nothing here
endif
##################################################################################################################
##name :BUILD_AT_REF
##Description:
##BUILD_AT_REF is to control whether include REF AT commands
##Use method: y is support;n is not support
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_AT_REF), y)
#define QI sockets command      AT+QIOPEN/QICLOSE/QIRD/QISDE/QIGETERROR/QISTATE/QISEND/QISENDEX/QISWTMD
BUILD_PS_REF_TCPIP_QSOCK_AT_ENABLE    = n

#define QNETCONIG command      AT+QIDNSCFG/QIDNSGIP/QNETDEVCTL
BUILD_PS_REF_TCPIP_QNETCONFIG_AT_ENABLE = n

#define QPING command      AT+QPING
BUILD_PS_REF_TCPIP_QPING_SERVICE_ENABLE = n

#define QNTP command      AT+QNTP
BUILD_PS_REF_TCPIP_QSNTP_SERVICE_ENABLE = n

#define QI socket command:   AT+QCCID/QENG/QCELL/QWIFISCAN/QPOWD/QSPN/QIACT/QIDEACT/QIREGAPP/QICSGP/QGDCNT/QAUGDCNT
BUILD_PS_REF_EPS_AT_ENABLE     = n

#define QI socket command:  AT+QICFG/QCFG
BUILD_PS_REF_COMMON_AT_ENABLE  = y
endif
