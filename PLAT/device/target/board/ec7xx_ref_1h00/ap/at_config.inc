##################################################################################################################
##Description：
##global option for RIL APIs
##Use method: y is support;n is not support
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_AT), y)
BUILD_RIL_AT_API_ENABLE      ?= y
endif

##################################################################################################################
##Description：
##global option for PLAT AT commands
##Use method: y is support;n is not support
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_AT), y)
BUILD_PLAT_CUST_AT_ENABLE    ?= y
BUILD_PLAT_MEM_AT_ENABLE     ?= y
BUILD_PLAT_PMU_AT_ENABLE     ?= y
BUILD_PLAT_PER_AT_ENABLE     ?= y
BUILD_PLAT_MISC_AT_ENABLE    ?= y
BUILD_PLAT_PROD_AT_ENABLE    ?= y
BUILD_PLAT_FOTA_AT_ENABLE    ?= y
BUILD_PLAT_ECOTA_AT_ENABLE   ?= n
BUILD_PLAT_CMUX_AT_ENABLE    ?= n
BUILD_PLAT_HTTP_AT_ENABLE    ?= y
BUILD_PLAT_MQTT_AT_ENABLE    ?= y
BUILD_PLAT_SSL_AT_ENABLE     ?= y
BUILD_PLAT_FS_AT_ENABLE      ?= y
BUILD_PLAT_AUDIO_AT_ENABLE   ?= n
BUILD_PLAT_ONENET_AT_ENABLE  ?= n
BUILD_PLAT_CTLWM2M_AT_ENABLE ?= n

ifeq ($(BUILD_AT_REF_CR), y)
BUILD_PLAT_REF_CR_FOTA_AT_ENABLE ?= y
BUILD_PLAT_REF_CR_MQTT_AT_ENABLE ?= n
BUILD_PLAT_REF_CR_LBS_AT_ENABLE  ?= n
endif
endif

##################################################################################################################
##Description：BUILD_PLAT_AUDIO_AT_ENABLE
##global option for Plat audio related AT commands
##Use method: y is support;n is not support
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_PLAT_AUDIO_AT_ENABLE), y)
#AT+CGMI
BUILD_PLAT_AUDIO_ECADCFG_ENABLE   ?= n
endif

##################################################################################################################
##Description：BUILD_PLAT_CUST_AT_ENABLE
##global option for Plat product related AT commands
##Use method: y is support;n is not support
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_PLAT_CUST_AT_ENABLE), y)
#AT+CGMI
BUILD_PLAT_CUST_CGMI_ENABLE   ?= y
#AT+CGMM
BUILD_PLAT_CUST_CGMM_ENABLE   ?= y
#AT+GMM
BUILD_PLAT_CUST_GMM_ENABLE    ?= y
#AT+CGMR
BUILD_PLAT_CUST_CGMR_ENABLE   ?= y
#AT+CGSN
BUILD_PLAT_CUST_CGSN_ENABLE   ?= y
#ATI
BUILD_PLAT_CUST_ATI_ENABLE    ?= y
#AT&W
BUILD_PLAT_CUST_ATnW_ENABLE   ?= y
#AT&F
BUILD_PLAT_CUST_ATnF_ENABLE   ?= y
#AT&V
BUILD_PLAT_CUST_ATnV_ENABLE   ?= y
endif

##################################################################################################################
##Description：BUILD_PLAT_MEM_AT_ENABLE
##global option for Plat memory related AT commands
##Use method: y is support;n is not support
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_PLAT_MEM_AT_ENABLE), y)
#AT+ECSHOWMEM
BUILD_PLAT_MEM_ECSHOWMEM_ENABLE           ?= y
#AT+ECHEAPINFO
BUILD_PLAT_MEM_ECHEAPINFO_ENABLE          ?= y
#AT+ECMEM32
BUILD_PLAT_MEM_ECMEM32_ENABLE             ?= y
#AT+ECDLFCMEM
BUILD_PLAT_MEM_ECDLFCMEM_ENABLE           ?= y
#AT+ECFSINFO
BUILD_PLAT_MEM_ECFSINFO_ENABLE            ?= y
#AT+RLCHK
BUILD_PLAT_MEM_RLCHK_ENABLE               ?= y
#AT+ECFLASHMONITORINFO
BUILD_PLAT_MEM_ECFLASHMONITORINFO_ENABLE  ?= y
endif

##################################################################################################################
##Description：BUILD_PLAT_PMU_AT_ENABLE
##global option for Plat pmu related AT commands
##Use method: y is support;n is not support
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_PLAT_PMU_AT_ENABLE), y)
#AT+ECPMUCFG
BUILD_PLAT_PMU_ECPMUCFG_ENABLE     ?= y
#AT+ECSYSMONCFG
BUILD_PLAT_PMU_ECSYSMONCFG_ENABLE  ?= y
#AT+ECVOTECHK
BUILD_PLAT_PMU_ECVOTECHK_ENABLE    ?= y
#AT+ECPMUSTATUS
BUILD_PLAT_PMU_ECPMUSTATUS_ENABLE  ?= y
#AT+ECPURC
BUILD_PLAT_PMU_ECPURC_ENABLE       ?= y
#AT+ECSCLK
BUILD_PLAT_PMU_ECSCLK_ENABLE       ?= y
endif

##################################################################################################################
##Description：BUILD_PLAT_PER_AT_ENABLE
##global option for Plat peripheral related AT commands
##Use method: y is support;n is not support
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_PLAT_PER_AT_ENABLE), y)
#AT+IPR
BUILD_PLAT_PER_IPR_ENABLE        ?= y
#AT+ICF
BUILD_PLAT_PER_ICF_ENABLE        ?= y
#AT+IFC
BUILD_PLAT_PER_IFC_ENABLE        ?= y
#AT+ECUSBSYS
BUILD_PLAT_PER_ECUSBSYS_ENABLE   ?= y
ifeq ($(BUILD_AT_REF_CR), y)
#AT+MLED
BUILD_PLAT_REF_CR_PER_MLED_ENABLE?= y
#AT+ECLEDMODE
BUILD_PLAT_PER_ECLEDMODE_ENABLE   = n
else
#AT+ECLEDMODE
BUILD_PLAT_PER_ECLEDMODE_ENABLE  ?= y
endif
#AT+ECPALARM
BUILD_PLAT_PER_ECPALARM_ENABLE   ?= y
#AT+ECADC
ifeq ($(BUILD_AT_REF_CR), y)
#disable AT+ECADC
BUILD_PLAT_PER_ECADC_ENABLE      = n
#define ML AT command:   AT+MADC/MCHIPINFO/MGPIO
BUILD_PLAT_REF_CR_PER_ADC_ENABLE ?= y
BUILD_PLAT_REF_CR_PER_GPIO_ENABLE ?= y
else
BUILD_PLAT_PER_ECADC_ENABLE      ?= y
endif
endif

##################################################################################################################
##Description：BUILD_PLAT_MISC_AT_ENABLE
##global option for Plat miscellaneous AT commands
##Use method: y is support;n is not support
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_PLAT_MISC_AT_ENABLE), y)
#AT+ECPCFG
BUILD_PLAT_MISC_ECPCFG_ENABLE          ?= y
#AT+ECRST
BUILD_PLAT_MISC_ECRST_ENABLE           ?= y
ifeq ($(BUILD_AT_REF_CR), y)
#AT+ECPOWD
BUILD_PLAT_MISC_ECPOWD_ENABLE           = n
#AT+MPOF
BUILD_PLAT_REF_CR_MISC_MPOF_ENABLE     ?= y
else
#AT+ECPOWD
BUILD_PLAT_MISC_ECPOWD_ENABLE          ?= y
endif
#AT^SYSTEST
BUILD_PLAT_MISC_SYSTEST_ENABLE         ?= y
#AT+ECSYSTEST
BUILD_PLAT_MISC_ECSYSTEST_ENABLE       ?= y
#AT+ECTASKINFO
BUILD_PLAT_MISC_ECTASKINFO_ENABLE      ?= y
#AT+ECTASKHISTINFO
BUILD_PLAT_MISC_ECTASKHISTINFO_ENABLE  ?= y
#AT+ECLOGDBVER
BUILD_PLAT_MISC_ECLOGDBVER_ENABLE      ?= y
#AT+ECDUMPCHK
BUILD_PLAT_MISC_ECDUMPCHK_ENABLE       ?= y
#AT+ECBTOFFSETDBG
BUILD_PLAT_MISC_ECBTOFFSETDBG_ENABLE   ?= y
#AT+ECFUSEMR
BUILD_PLAT_MISC_ECFUSEMR_ENABLE        ?= y
#AT+ECSIMO
BUILD_PLAT_MISC_ECSIMO_ENABLE          ?= n
#AT+ECPRFINFO
BUILD_PLAT_MISC_ECPRFINFO_ENABLE       ?= n
ifeq ($(BUILD_AT_REF_CR), y)
#AT+MREBOOT
BUILD_PLAT_REF_CR_MISC_MREBOOT_ENABLE  ?= y
#AT+MLPMCFG
BUILD_PLAT_REF_CR_MISC_MLPMCFG_ENABLE  ?= y
endif
endif

##################################################################################################################
##Description：BUILD_PLAT_PROD_AT_ENABLE
##global option for Plat product related AT commands
##Use method: y is support;n is not support
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_PLAT_PROD_AT_ENABLE), y)
#AT+ECATE
BUILD_PLAT_PROD_ECATE_ENABLE       ?= y
#AT+ECVERSION
BUILD_PLAT_PROD_ECVERSION_ENABLE   ?= y
#AT+ECIPR
BUILD_PLAT_PROD_ECIPR_ENABLE       ?= y
#AT+ECSLEEP
BUILD_PLAT_PROD_ECSLEEP_ENABLE     ?= y
#AT+ECSAVEFAC
BUILD_PLAT_PROD_ECSAVEFAC_ENABLE   ?= y
#AT+ECRFTEST
BUILD_PLAT_PROD_ECRFTEST_ENABLE    ?= y
#AT+ECGMDATA
BUILD_PLAT_PROD_ECGMDATA_ENABLE    ?= y
#AT+ECNPICFG
BUILD_PLAT_PROD_ECNPICFG_ENABLE    ?= y
#AT+ECRFNST
BUILD_PLAT_PROD_ECRFNST_ENABLE     ?= y
#AT+ECRFSTAT
BUILD_PLAT_PROD_ECRFSTAT_ENABLE    ?= y
#AT+ECPRODMODE
BUILD_PLAT_PROD_ECPRODMODE_ENABLE  ?= n
#AT+ECCHIPVER
BUILD_PLAT_PROD_ECCHIPVER_ENABLE   ?= n
endif

##################################################################################################################
##Description：
##global option for PS AT commands
##Use method: y is support;n is not support
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_AT), y)
BUILD_PS_GEN_AT_ENABLE  ?= y
BUILD_PS_DEV_AT_ENABLE  ?= y
BUILD_PS_EMM_AT_ENABLE  ?= y
BUILD_PS_EPS_AT_ENABLE  ?= y
BUILD_PS_SIM_AT_ENABLE  ?= y
BUILD_PS_SMS_AT_ENABLE  ?= y
BUILD_PHY_CONFIG_AT_ENABLE ?= y
BUILD_PS_TCPIP_ECSOCK_SKTSOCK_AT_ENABLE  ?= y
BUILD_PS_TCPIP_NETCONFIG_AT_ENABLE ?=y
BUILD_PS_TCPIP_IPSEC_AT_ENABLE   ?=n
endif
##################################################################################################################
##name :BUILD_PS_GEN_AT_ENABLE
##Description：
##BUILD_PS_GEN_AT_ENABLE is to control whether include ps  global AT commands as: AT/ATQ/ATE/ATT/ATV/ATL/ATM/...etc
##Use method: y is support;n is not support.
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_PS_GEN_AT_ENABLE), y)
#AT via BUILD_PS_GEN_AT_ENABLE
#ATQ
BUILD_PS_GEN_AT_Q_ENABLE     ?= y
#ATE
BUILD_PS_GEN_AT_E_ENABLE     ?= y
#ATT
BUILD_PS_GEN_AT_T_ENABLE     ?= y
#ATV
BUILD_PS_GEN_AT_V_ENABLE     ?= y
#ATL
BUILD_PS_GEN_AT_L_ENABLE     ?= y
#ATM
BUILD_PS_GEN_AT_M_ENABLE     ?= y
#ATS0
BUILD_PS_GEN_AT_S0_ENABLE    ?= y
#AT&C
BUILD_PS_GEN_AT_nC_ENABLE    ?= y
#AT&D
BUILD_PS_GEN_AT_nD_ENABLE    ?= y
#ATD
BUILD_PS_GEN_AT_D_ENABLE     ?= y
#ATO
BUILD_PS_GEN_AT_O_ENABLE     ?= y
#ATH
BUILD_PS_GEN_AT_H_ENABLE     ?= y
#ATA
BUILD_PS_GEN_AT_A_ENABLE     ?= y
#AT+CSCS
BUILD_PS_GEN_CSCS_ENABLE     ?= y
#AT+CMEE
BUILD_PS_GEN_CMEE_ENABLE     ?= y
#AT+CMEE=2
BUILD_CMEE_2_VERBOSE_ENABLE  ?= y
#AT+ECURC
BUILD_PS_GEN_ECURC_ENABLE    ?= y
#AT+ECURCCFG
BUILD_PS_GEN_ECURCCFG_ENABLE ?= y
#AT+ECPPPHUP
BUILD_PS_GEN_ECPPPHUP_ENABLE ?= y
#AT+CLCK
BUILD_PS_GEN_CLCK_ENABLE     ?= y
endif
##################################################################################################################
##name :BUILD_PS_DEV_AT_ENABLE
##Description：
##BUILD_PS_DEV_AT_ENABLE is to control whether include ps  global AT commands as: AT+CFUN/ECBAND/ECCFG/ECSTATUS/...etc
##Use method: y is support;n is not support.
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_PS_DEV_AT_ENABLE), y)

#AT+CFUN
BUILD_PS_DEV_CFUN_ENABLE    ?= y
#AT+ECCGSN
BUILD_PS_DEV_ECCGSN_ENABLE  ?= y
#AT+ECCGSNLOCK
BUILD_PS_DEV_ECCGSNLOCK_ENABLE  ?= y
#AT+ECNETDEVMAC
BUILD_PS_DEV_ECNETDEVMAC_ENABLE  ?= y
#AT+ECSTATUS
BUILD_PS_DEV_ECSTATUS_ENABLE  ?= y
#AT+ECSTATIS
BUILD_PS_DEV_ECSTATIS_ENABLE  ?= y
#AT+ECBAND
BUILD_PS_DEV_ECBAND_ENABLE    ?=  y
#AT+ECFREQ
BUILD_PS_DEV_ECFREQ_ENABLE    ?=  y
#AT+ECCFG
BUILD_PS_DEV_ECCFG_ENABLE     ?=  y
#AT+ECMEASCFG
BUILD_PS_DEV_ECMEASCFG_ENABLE ?=  n
#AT+ECRMFPLMN
BUILD_PS_DEV_ECRMFPLMN_ENABLE ?=  y
#AT+ECBLOCKPLMNLIST
BUILD_PS_DEV_ECBLOCKPLMNLIST_ENABLE ?=  y
#AT+ECBCINFO
BUILD_PS_DEV_ECBCINFO_ENABLE  ?=  y
#AT+ECPSTEST
BUILD_PS_DEV_ECPSTEST_ENABLE  ?=  y
#AT+ECEVENTSTATIS
BUILD_PS_DEV_ECEVENTSTATIS_ENABLE  ?=  y
#AT+ECNASTCFG
BUILD_PS_DEV_ECNASTCFG_ENABLE   ?=  y
#AT+ECWIFISCAN
BUILD_PS_DEV_ECWIFISCAN_ENABLE  ?=  y
#AT+ECBARCELL
BUILD_PS_DEV_ECBARCELL_ENABLE   ?= y
endif
##################################################################################################################
##name :BUILD_PS_EMM_AT_ENABLE
##Description：
##BUILD_PS_EMM_AT_ENABLE is to control whether include ps  mm AT commands as: AT+CESQ/COPS/CCIOTOPT/ECCESQS/...etc
##Use method: y is support;n is not support
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_PS_EMM_AT_ENABLE), y)
#AT+CSQ
BUILD_PS_EMM_CSQ_ENABLE   ?=  y
#AT+CESQ
BUILD_PS_EMM_CESQ_ENABLE  ?=  y
#AT+CREG
BUILD_PS_EMM_CREG_ENABLE  ?=  y
#AT+COPS
BUILD_PS_EMM_COPS_ENABLE  ?=  y
#AT+CPSMS
BUILD_PS_EMM_CPSMS_ENABLE     ?= y
#AT+CEDRXS
BUILD_PS_EMM_CEDRXS_ENABLE    ?= y
#AT+CEDRXRDP
BUILD_PS_EMM_CEDRXRDP_ENABLE  ?= y
#AT+CCIOTOPT
BUILD_PS_EMM_CCIOTOPT_ENABLE  ?= y
#AT+CRCES
BUILD_PS_EMM_CRCES_ENABLE ?= y
#AT+CCLK
BUILD_PS_EMM_CCLK_ENABLE  ?= y
#AT+CTZR
BUILD_PS_EMM_CTZR_ENABLE  ?= y
#AT+CTZU
BUILD_PS_EMM_CTZU_ENABLE  ?= y
#AT+ECPLMNS
BUILD_PS_EMM_ECPLMNS_ENABLE     ?= y
#AT+ECCESQS
BUILD_PS_EMM_ECCESQS_ENABLE     ?= y
#AT+ECPSMR
BUILD_PS_EMM_ECPSMR_ENABLE      ?= y
#AT+ECPTWEDRXS
BUILD_PS_EMM_ECPTWEDRXS_ENABLE  ?= y
#AT+ECEMMTIME
BUILD_PS_EMM_ECEMMTIME_ENABLE   ?= y
#AT+ECPLMNRESELECT
BUILD_PS_EMM_ECPLMNRESELECT_ENABLE  ?= y
#AT+ECPLMNINTERVAL
BUILD_PS_EMM_ECPLMNINTERVAL_ENABLE  ?= y
#AT+ECCONNREL
BUILD_PS_EMM_ECCONNREL_ENABLE  ?= y
#AT+ECMMER
BUILD_PS_EMM_ECMMER_ENABLE   ?= y
#AT+ECEHPLMNLIST
BUILD_PS_EMM_ECEHPLMNLIST_ENABLE   ?= y
endif
##################################################################################################################
##name :BUILD_PS_EPS_AT_ENABLE
##Description：
##BUILD_PS_EPS_AT_ENABLE is to control whether include ESP  AT commands as: AT+CGATT/CGDCONT/CGACT/CGCONTRDP/CGAUTH..etc
##Use method: y is support;n is not support
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_PS_EPS_AT_ENABLE), y)
#AT+CGATT
BUILD_PS_EPS_CGATT_ENABLE  ?= y
#AT+ECEMCATT
BUILD_PS_EPS_ECEMCATT_ENABLE  ?= n
#AT+CGDATA
BUILD_PS_EPS_CGDATA_ENABLE ?= y
#AT+CGTFT
BUILD_PS_EPS_CGTFT_ENABLE  ?= y
#AT+CGCMOD
BUILD_PS_EPS_CGCMOD_ENABLE ?= y
#AT+CGDCONT
BUILD_PS_EPS_CGDCONT_ENABLE ?= y
#AT+CGACT
BUILD_PS_EPS_CGACT_ENABLE   ?= y
#AT+CGEQOS
BUILD_PS_EPS_CGEQOS_ENABLE  ?= y
#AT+CGEQOSRDP
BUILD_PS_EPS_CGEQOSRDP_ENABLE ?= y
#AT+CGCONTRDP
BUILD_PS_EPS_CGCONTRDP_ENABLE ?= y
#AT+CGSCONTRDP
BUILD_PS_EPS_CGSCONTRDP_ENABLE ?= y
#AT+CGTFTRDP
BUILD_PS_EPS_CGTFTRDP_ENABLE   ?= y
#AT+CEREG
BUILD_PS_EPS_CEREG_ENABLE ?= y
#AT+CSCON
BUILD_PS_EPS_CSCON_ENABLE ?= y
#AT+CSODCP
BUILD_PS_EPS_CSODCP_ENABLE ?= y
#AT+CRTDCP
BUILD_PS_EPS_CRTDCP_ENABLE ?= y
#AT+CGAUTH
BUILD_PS_EPS_CGAUTH_ENABLE ?= y
#AT+CIPCA
BUILD_PS_EPS_CIPCA_ENABLE  ?= y
#AT+CGAPNRC
BUILD_PS_EPS_CGAPNRC_ENABLE ?= y
#AT+CGEREP
BUILD_PS_EPS_CGEREP_ENABLE  ?= y
#AT+CGDSCONT
BUILD_PS_EPS_CGDSCONT_ENABLE ?= y
#AT+CGPADDR
BUILD_PS_EPS_CGPADDR_ENABLE  ?= y
#AT+CEER
BUILD_PS_EPS_CEER_ENABLE   ?= y
#AT+CEMODE
BUILD_PS_EPS_CEMODE_ENABLE ?= y
#AT+CEVDP
BUILD_PS_EPS_CEVDP_ENABLE ?= n
#AT+CPSDO
BUILD_PS_EPS_CPSDO_ENABLE  ?= y
#AT+ECCIOTPLANE
BUILD_PS_EPS_ECCIOTPLANE_ENABLE ?= y
#AT+ECSENDDATA
BUILD_PS_EPS_ECSENDDATA_ENABLE  ?= y
#AT+ECGDCNT
BUILD_PS_EPS_ECGDCNT_ENABLE     ?= y
#AT+ECAUGDCNT
BUILD_PS_EPS_ECAUGDCNT_ENABLE   ?= y
#AT+ECSCLKEX
BUILD_PS_EPS_ECSCLKEX_ENABLE    ?= y
#AT+CNEC
BUILD_PS_EPS_CNEC_ENABLE ?= y
#AT+ECSMER
BUILD_PS_EPS_ECSMER_ENABLE   ?= y
endif
##################################################################################################################
##name :BUILD_PS_SIM_AT_ENABLE
##Description：
##BUILD_PS_SIM_AT_ENABLE is to control whether include SIM AT commands as: AT+CIMI/CPIN/ECICCID/ECSIMCFG..etc
##Use method: y is support;n is not support
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_PS_SIM_AT_ENABLE), y)
#AT+CIMI
BUILD_PS_SIM_AT_CIMI_ENABLE         ?= y
#AT+CPIN
BUILD_PS_SIM_AT_CPIN_ENABLE         ?= y
#AT+ECICCID
BUILD_PS_SIM_AT_ECICCID_ENABLE      ?= y
#AT+ECSIMCFG
BUILD_PS_SIM_AT_ECSIMCFG_ENABLE     ?= y
#AT+CPWD
BUILD_PS_SIM_AT_CPWD_ENABLE         ?= y
#AT+CPINR
BUILD_PS_SIM_AT_CPINR_ENABLE        ?= y
#AT+ECSIMSLEEP
BUILD_PS_SIM_AT_ECSIMSLEEP_ENABLE   ?= y
#AT+CSIM
BUILD_PS_SIM_AT_CSIM_ENABLE         ?= y
#AT+CRSM
BUILD_PS_SIM_AT_CRSM_ENABLE         ?= y
#AT+CCHO
BUILD_PS_SIM_AT_CCHO_ENABLE         ?= y
#AT+CGLA
BUILD_PS_SIM_AT_CGLA_ENABLE         ?= y
#AT+CCHC
BUILD_PS_SIM_AT_CCHC_ENABLE         ?= y
#AT+ECSWC
BUILD_PS_SIM_AT_ECSWC_ENABLE        ?= y
#AT+ECSIMRM
BUILD_PS_SIM_AT_ECSIMRM_ENABLE      ?= y
#AT+CNUM
BUILD_PS_SIM_AT_CNUM_ENABLE         ?= y
#AT+ECUSATP
BUILD_PS_SIM_AT_ECUSATP_ENABLE      ?= y
#AT+CPOL
BUILD_PS_SIM_AT_CPOL_ENABLE         ?= y
#AT+CPLS
BUILD_PS_SIM_AT_CPLS_ENABLE         ?= y
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
BUILD_PS_SMS_CMGS_ENABLE   ?= y
#AT+CMGC
BUILD_PS_SMS_CMGC_ENABLE   ?= y
#AT+CMGR
BUILD_PS_SMS_CMGR_ENABLE   ?= y
#AT+CMGW
BUILD_PS_SMS_CMGW_ENABLE   ?= y
#AT+CSCA
BUILD_PS_SMS_CSCA_ENABLE   ?= y
#AT+CNMI
BUILD_PS_SMS_CNMI_ENABLE   ?= y
#AT+CMMS
BUILD_PS_SMS_CMMS_ENABLE   ?= y
#AT+CMGD
BUILD_PS_SMS_CMGD_ENABLE   ?= y
#AT+CMGL
BUILD_PS_SMS_CMGL_ENABLE   ?= y
#AT+CSMS
BUILD_PS_SMS_CSMS_ENABLE   ?= y
#AT+CPMS
BUILD_PS_SMS_CPMS_ENABLE   ?= y
#AT+CNMA
BUILD_PS_SMS_CNMA_ENABLE   ?= y
#AT+CMGF
BUILD_PS_SMS_CMGF_ENABLE   ?= y
#AT+CSMP
BUILD_PS_SMS_CSMP_ENABLE   ?= y
#AT+CSCB
BUILD_PS_SMS_CSCB_ENABLE   ?= y
#AT+CSDH
BUILD_PS_SMS_CSDH_ENABLE   ?= y
#AT+CMSS
BUILD_PS_SMS_CMSS_ENABLE   ?= y
#AT+ECSMSSEND
BUILD_PS_SMS_ECSMSSEND_ENABLE   ?= y
#AT+ECCMGS
BUILD_PS_SMS_ECCMGS_ENABLE   ?= y
#AT+ECCMGR
BUILD_PS_SMS_ECCMGR_ENABLE   ?= y
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
BUILD_PHY_CONFIG_ECPHYCFG_ENABLE   ?= y
#AT+ECEDRXSIMU
BUILD_PHY_CONFIG_ECEDRXSIMU_ENABLE ?= y
endif
##################################################################################################################
##name :BUILD_PS_TCPIP_ECSOCK_SKTSOCK_AT_ENABLE
##Description：
##BUILD_PS_TCPIP_ECSOCK_SKTSOCK_AT_ENABLE is to control whether include TCPIP EC SOCKET AT commands
##Use method: y is support;n is not support
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_PS_TCPIP_ECSOCK_SKTSOCK_AT_ENABLE), y)
# do nothing here
endif
##################################################################################################################
##name :BUILD_AT_REF_QR
##Description：
##BUILD_AT_REF_QR is to control whether include REF AT commands
##Use method: y is support;n is not support
##default value is y to support
##################################################################################################################
ifeq ($(BUILD_AT_REF_QR), y)
#define QI sockets command      AT+QIOPEN/QICLOSE/QIRD/QISDE/QIGETERROR/QISTATE/QISEND/QISENDEX/QISWTMD
BUILD_PS_REF_QR_TCPIP_QSOCK_AT_ENABLE    ?= y

#define QNETCONIG command      AT+QIDNSCFG/QIDNSGIP/QNETDEVCTL
BUILD_PS_REF_QR_TCPIP_QNETCONFIG_AT_ENABLE  ?= y

#define QPING command      AT+QPING
BUILD_PS_REF_QR_TCPIP_QPING_SERVICE_ENABLE  ?= y

#define QNTP command      AT+QNTP
BUILD_PS_REF_QR_TCPIP_QSNTP_SERVICE_ENABLE  ?= y

#define QI socket command：    AT+QCCID/QENG/QCELL/QWIFISCAN/QPOWD/QSPN/QIACT/QIDEACT/QIREGAPP/QICSGP/QGDCNT/QAUGDCNT
BUILD_PS_REF_QR_EPS_AT_ENABLE      ?= y

#define QI socket command：    AT+QICFG/QCFG
BUILD_PS_REF_QR_COMMON_AT_ENABLE   ?= y
endif

##################################################################################################################
##name :BUILD_AT_REF_CR
##Description:
##BUILD_AT_REF_CR is to control whether include REF CR AT commands
##Use method: y is support;n is not support
##default value is y to support
##################################################################################################################

ifeq ($(BUILD_AT_REF_CR), y)

#define ML AT command:   AT+MLOCKFREQ/MCSEARFCN/MUESTATS/MCCID/MBAND/MIPCALL/ICCID
BUILD_PS_REF_CR_EPS_AT_ENABLE     ?= y

#AT+MLOCKFREQ
BUILD_PS_REF_CR_MLOCKFREQ_ENABLE   ?= y
#AT+MCSEARFCN
BUILD_PS_REF_CR_MCSEARFCN_ENABLE   ?= y
#AT+MUESTATE
BUILD_PS_REF_CR_MUESTATS_ENABLE   ?= y
#AT+MCCID
BUILD_PS_REF_CR_MCCID_ENABLE   ?= y
#AT+ICCID
BUILD_PS_REF_CR_ICCID_ENABLE   ?= y
#AT+MBAND
BUILD_PS_REF_CR_MBAND_ENABLE   ?= y
#AT+MIPCALL
BUILD_PS_REF_CR_MIPCALL_ENABLE   ?= y
#AT+MWIFISCANCFG/AT+MWIFISCANSTART/AT+MWIFISCANSTOP/AT+MWIFISCANQUERY
BUILD_PS_REF_CR_MWIFISCAN_ENABLE   ?= y
#AT+MEID
BUILD_PS_REF_CR_MEID_ENABLE   ?= y

#define ML AT command:   AT+MIPOPEN/MIPCLOSE/MIPCFG/MIPTKA/MIPSEND/MIPRD/MIPMODE/MIPSTATE/MIPSACK/MIPURC
BUILD_PS_REF_CR_TCPIP_SOCK_AT_ENABLE ?= y
#AT+MPING
BUILD_PS_REF_CR_TCPIP_PING_SERVICE_ENABLE ?= y
#AT+MNTP
BUILD_PS_REF_CR_TCPIP_SNTP_SERVICE_ENABLE ?= y
#AT+DNSCFG/AT+DNSGIP/AT+MDIALUPCFG/AT+DIALUP
BUILD_PS_REF_CR_TCPIP_NETCONFIG_AT_ENABLE ?= y

endif

