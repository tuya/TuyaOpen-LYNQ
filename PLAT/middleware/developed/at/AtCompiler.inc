##################################################################################################################
##ril module APIs
##################################################################################################################
ifeq ($(BUILD_RIL_AT_API_ENABLE),y)
CFLAGS += -DFEATURE_RIL_AT_API_ENABLE
endif

##################################################################################################################
##plat audio module at commands
##################################################################################################################
ifeq ($(BUILD_PLAT_AUDIO_AT_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_AUDIO_AT_ENABLE -DFEATURE_PLAT_PER_ECCODEC_ENABLE

ifeq ($(BUILD_PLAT_AUDIO_ECADCFG_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_AUDIO_ECADCFG_ENABLE
endif

ifeq ($(BUILD_FIXED_AUDIO_SOURCE_ENABLE),y)
CFLAGS += -DFEATURE_FIXED_AUDIO_SOURCE_ENABLE
endif

ifeq ($(BUILD_CODEC_OUTSIDE_ENABLE),y)
CFLAGS += -DFEATURE_CODEC_OUTSIDE_ENABLE
endif
endif

##################################################################################################################
##plat customer's module at commands
##################################################################################################################
ifeq ($(BUILD_PLAT_CUST_AT_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_CUST_AT_ENABLE

ifeq ($(BUILD_PLAT_CUST_CGMI_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_CUST_CGMI_ENABLE
endif

ifeq ($(BUILD_PLAT_CUST_CGMM_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_CUST_CGMM_ENABLE
endif

ifeq ($(BUILD_PLAT_CUST_GMM_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_CUST_GMM_ENABLE
endif

ifeq ($(BUILD_PLAT_CUST_CGMR_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_CUST_CGMR_ENABLE
endif

ifeq ($(BUILD_PLAT_CUST_CGSN_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_CUST_CGSN_ENABLE
endif

ifeq ($(BUILD_PLAT_CUST_ATI_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_CUST_ATI_ENABLE
endif

ifeq ($(BUILD_PLAT_CUST_ATnW_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_CUST_ATnW_ENABLE
endif

ifeq ($(BUILD_PLAT_CUST_ATnF_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_CUST_ATnF_ENABLE
endif

ifeq ($(BUILD_PLAT_CUST_ATnV_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_CUST_ATnV_ENABLE
endif
endif

##################################################################################################################
##plat mem module at commands
##################################################################################################################
ifeq ($(BUILD_PLAT_MEM_AT_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_MEM_AT_ENABLE

ifeq ($(BUILD_PLAT_MEM_ECSHOWMEM_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_MEM_ECSHOWMEM_ENABLE
endif

ifeq ($(BUILD_PLAT_MEM_ECHEAPINFO_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_MEM_ECHEAPINFO_ENABLE
endif

ifeq ($(BUILD_PLAT_MEM_ECMEM32_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_MEM_ECMEM32_ENABLE
endif

ifeq ($(BUILD_PLAT_MEM_ECDLFCMEM_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_MEM_ECDLFCMEM_ENABLE
endif

ifeq ($(BUILD_PLAT_MEM_ECFSINFO_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_MEM_ECFSINFO_ENABLE
endif

ifeq ($(BUILD_PLAT_MEM_RLCHK_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_MEM_RLCHK_ENABLE
endif

ifeq ($(BUILD_PLAT_MEM_ECFLASHMONITORINFO_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_MEM_ECFLASHMONITORINFO_ENABLE -DFS_FILE_OPERATION_STATISTIC
endif
endif

##################################################################################################################
##plat pmu module at commands
##################################################################################################################
ifeq ($(BUILD_PLAT_PMU_AT_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_PMU_AT_ENABLE

ifeq ($(BUILD_PLAT_PMU_ECPMUCFG_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_PMU_ECPMUCFG_ENABLE
endif

ifeq ($(BUILD_PLAT_PMU_ECSYSMONCFG_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_PMU_ECSYSMONCFG_ENABLE
endif

ifeq ($(BUILD_PLAT_PMU_ECVOTECHK_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_PMU_ECVOTECHK_ENABLE
endif

ifeq ($(BUILD_PLAT_PMU_ECPMUSTATUS_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_PMU_ECPMUSTATUS_ENABLE
endif

ifeq ($(BUILD_PLAT_PMU_ECPURC_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_PMU_ECPURC_ENABLE
endif

ifeq ($(BUILD_PLAT_PMU_ECSCLK_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_PMU_ECSCLK_ENABLE
endif
endif

##################################################################################################################
##plat per module at commands
##################################################################################################################
ifeq ($(BUILD_PLAT_PER_AT_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_PER_AT_ENABLE

ifeq ($(BUILD_PLAT_PER_IPR_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_PER_IPR_ENABLE
endif

ifeq ($(BUILD_PLAT_PER_ICF_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_PER_ICF_ENABLE
endif

ifeq ($(BUILD_PLAT_PER_IFC_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_PER_IFC_ENABLE
endif

ifeq ($(BUILD_PLAT_PER_ECUSBSYS_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_PER_ECUSBSYS_ENABLE
endif

ifeq ($(BUILD_PLAT_PER_ECLEDMODE_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_PER_ECLEDMODE_ENABLE
endif

ifeq ($(BUILD_PLAT_PER_ECPALARM_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_PER_ECPALARM_ENABLE
endif

ifeq ($(BUILD_PLAT_PER_ECADC_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_PER_ECADC_ENABLE -DFEATURE_ATADC_ENABLE
endif

ifeq ($(BUILD_PLAT_REF_CR_PER_MLED_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_REF_CR_PER_MLED_ENABLE
endif

ifeq ($(BUILD_PLAT_REF_CR_PER_ADC_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_REF_CR_PER_ADC_ENABLE
endif

ifeq ($(BUILD_PLAT_REF_CR_PER_GPIO_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_REF_CR_PER_GPIO_ENABLE
endif

endif

##################################################################################################################
##plat misc module at commands
##################################################################################################################
ifeq ($(BUILD_PLAT_MISC_AT_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_MISC_AT_ENABLE

ifeq ($(BUILD_PLAT_MISC_ECPCFG_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_MISC_ECPCFG_ENABLE
endif

ifeq ($(BUILD_PLAT_MISC_ECRST_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_MISC_ECRST_ENABLE
endif

ifeq ($(BUILD_PLAT_MISC_ECPOWD_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_MISC_ECPOWD_ENABLE
endif

ifeq ($(BUILD_PLAT_MISC_SYSTEST_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_MISC_SYSTEST_ENABLE
endif

ifeq ($(BUILD_PLAT_MISC_ECSYSTEST_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_MISC_ECSYSTEST_ENABLE
endif

ifeq ($(BUILD_PLAT_MISC_ECTASKINFO_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_MISC_ECTASKINFO_ENABLE
endif

ifeq ($(BUILD_PLAT_MISC_ECTASKHISTINFO_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_MISC_ECTASKHISTINFO_ENABLE
endif

ifeq ($(BUILD_PLAT_MISC_ECLOGDBVER_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_MISC_ECLOGDBVER_ENABLE
endif

ifeq ($(BUILD_PLAT_MISC_ECDUMPCHK_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_MISC_ECDUMPCHK_ENABLE
endif

ifeq ($(BUILD_PLAT_MISC_ECBTOFFSETDBG_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_MISC_ECBTOFFSETDBG_ENABLE
endif

ifeq ($(BUILD_PLAT_MISC_ECFUSEMR_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_MISC_ECFUSEMR_ENABLE
endif

ifeq ($(BUILD_PLAT_MISC_ECSIMO_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_MISC_ECSIMO_ENABLE
endif

ifeq ($(BUILD_PLAT_MISC_ECPRFINFO_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_MISC_ECPRFINFO_ENABLE
endif

ifeq ($(BUILD_PLAT_REF_CR_MISC_MPOF_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_REF_CR_MISC_MPOF_ENABLE
endif

ifeq ($(BUILD_PLAT_REF_CR_MISC_MREBOOT_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_REF_CR_MISC_MREBOOT_ENABLE
endif

ifeq ($(BUILD_PLAT_REF_CR_MISC_MLPMCFG_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_REF_CR_MISC_MLPMCFG_ENABLE
endif

endif

##################################################################################################################
##plat product module at commands
##################################################################################################################
ifeq ($(BUILD_PLAT_PROD_AT_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_PROD_AT_ENABLE

ifeq ($(BUILD_PLAT_PROD_ECATE_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_PROD_ECATE_ENABLE
endif

ifeq ($(BUILD_PLAT_PROD_ECVERSION_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_PROD_ECVERSION_ENABLE
endif

ifeq ($(BUILD_PLAT_PROD_ECIPR_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_PROD_ECIPR_ENABLE
endif

ifeq ($(BUILD_PLAT_PROD_ECSLEEP_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_PROD_ECSLEEP_ENABLE
endif

ifeq ($(BUILD_PLAT_PROD_ECSAVEFAC_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_PROD_ECSAVEFAC_ENABLE
endif

ifeq ($(BUILD_PLAT_PROD_ECRFTEST_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_PROD_ECRFTEST_ENABLE
endif

ifeq ($(BUILD_PLAT_PROD_ECGMDATA_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_PROD_ECGMDATA_ENABLE
endif

ifeq ($(BUILD_PLAT_PROD_ECNPICFG_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_PROD_ECNPICFG_ENABLE
endif

ifeq ($(BUILD_PLAT_PROD_ECRFNST_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_PROD_ECRFNST_ENABLE
endif

ifeq ($(BUILD_PLAT_PROD_ECRFSTAT_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_PROD_ECRFSTAT_ENABLE
endif

ifeq ($(BUILD_PLAT_PROD_ECPRODMODE_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_PROD_ECPRODMODE_ENABLE -DFEATURE_PRODMODE_ENABLE
endif

ifeq ($(BUILD_PLAT_PROD_ECCHIPVER_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_PROD_ECCHIPVER_ENABLE
endif

ifeq ($(BUILD_PLAT_PROD_ECDIEXY_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_PROD_ECDIEXY_ENABLE
endif
endif

##################################################################################################################
##plat application module at commands
##################################################################################################################
ifeq ($(BUILD_PLAT_CMUX_AT_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_CMUX_AT_ENABLE
endif

ifeq ($(BUILD_PLAT_HTTP_AT_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_HTTP_AT_ENABLE
endif

ifeq ($(BUILD_PLAT_MQTT_AT_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_MQTT_AT_ENABLE
endif

ifeq ($(BUILD_PLAT_REF_CR_MQTT_AT_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_REF_CR_MQTT_AT_ENABLE
endif

#ifeq ($(BUILD_PLAT_SSL_AT_ENABLE),y)
#CFLAGS += -DFEATURE_PLAT_SSL_AT_ENABLE
#endif

ifeq ($(BUILD_PLAT_FS_AT_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_FS_AT_ENABLE -DFEATURE_FILE_AT_ENABLE
endif

ifeq ($(BUILD_PLAT_ONENET_AT_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_ONENET_AT_ENABLE
endif

ifeq ($(BUILD_PLAT_CTLWM2M_AT_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_CTLWM2M_AT_ENABLE
endif

ifeq ($(BUILD_PLAT_FOTA_AT_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_FOTA_AT_ENABLE
endif

ifeq ($(BUILD_PLAT_ECOTA_AT_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_ECOTA_AT_ENABLE
endif

ifeq ($(BUILD_PLAT_REF_CR_FOTA_AT_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_REF_CR_FOTA_AT_ENABLE
endif

ifeq ($(BUILD_PLAT_REF_CR_LBS_AT_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_REF_CR_LBS_AT_ENABLE
endif

ifeq ($(BUILD_PLAT_REF_CR_FILE_AT_ENABLE),y)
CFLAGS += -DFEATURE_PLAT_REF_CR_FILE_AT_ENABLE
endif

##################################################################################################################
##ps general at commands
##################################################################################################################
ifeq ($(BUILD_PS_GEN_AT_ENABLE), y)
CFLAGS += -DFEATURE_PS_GEN_AT_ENABLE -DFEATURE_PS_COMMON_AT_ENABLE

ifeq ($(BUILD_PS_GEN_AT_Q_ENABLE),y)
CFLAGS += -DFEATURE_PS_GEN_AT_Q_ENABLE
endif

ifeq ($(BUILD_PS_GEN_AT_E_ENABLE),y)
CFLAGS += -DFEATURE_PS_GEN_AT_E_ENABLE
endif

ifeq ($(BUILD_PS_GEN_AT_T_ENABLE),y)
CFLAGS += -DFEATURE_PS_GEN_AT_T_ENABLE
endif

ifeq ($(BUILD_PS_GEN_AT_V_ENABLE),y)
CFLAGS += -DFEATURE_PS_GEN_AT_V_ENABLE
endif

ifeq ($(BUILD_PS_GEN_AT_L_ENABLE),y)
CFLAGS += -DFEATURE_PS_GEN_AT_L_ENABLE
endif

ifeq ($(BUILD_PS_GEN_AT_M_ENABLE),y)
CFLAGS += -DFEATURE_PS_GEN_AT_M_ENABLE
endif

ifeq ($(BUILD_PS_GEN_AT_S0_ENABLE),y)
CFLAGS += -DFEATURE_PS_GEN_AT_S0_ENABLE
endif

ifeq ($(BUILD_PS_GEN_AT_nC_ENABLE),y)
CFLAGS += -DFEATURE_PS_GEN_AT_nC_ENABLE
endif

ifeq ($(BUILD_PS_GEN_AT_nD_ENABLE),y)
CFLAGS += -DFEATURE_PS_GEN_AT_nD_ENABLE
endif

ifeq ($(BUILD_PS_GEN_AT_D_ENABLE),y)
CFLAGS += -DFEATURE_PS_GEN_AT_D_ENABLE
endif

ifeq ($(BUILD_PS_GEN_AT_O_ENABLE),y)
CFLAGS += -DFEATURE_PS_GEN_AT_O_ENABLE
endif

ifeq ($(BUILD_PS_GEN_AT_H_ENABLE),y)
CFLAGS += -DFEATURE_PS_GEN_AT_H_ENABLE
endif

ifeq ($(BUILD_PS_GEN_AT_A_ENABLE),y)
CFLAGS += -DFEATURE_PS_GEN_AT_A_ENABLE
endif

ifeq ($(BUILD_PS_GEN_CSCS_ENABLE),y)
CFLAGS += -DFEATURE_PS_GEN_CSCS_ENABLE
endif

ifeq ($(BUILD_PS_GEN_CMEE_ENABLE),y)
CFLAGS += -DFEATURE_PS_GEN_CMEE_ENABLE
endif

ifeq ($(BUILD_CMEE_2_VERBOSE_ENABLE),y)
CFLAGS += -DFEATURE_CMEE_2_VERBOSE_ENABLE
endif

ifeq ($(BUILD_PS_GEN_ECURC_ENABLE),y)
CFLAGS += -DFEATURE_PS_GEN_ECURC_ENABLE
endif

ifeq ($(BUILD_PS_GEN_ECURCCFG_ENABLE),y)
CFLAGS += -DFEATURE_PS_GEN_ECURCCFG_ENABLE
endif

ifeq ($(BUILD_PS_GEN_ECPPPHUP_ENABLE),y)
CFLAGS += -DFEATURE_PS_GEN_ECPPPHUP_ENABLE
endif

ifeq ($(BUILD_PS_GEN_CLCK_ENABLE), y)
CFLAGS += -DFEATURE_PS_GEN_CLCK_ENABLE
endif

endif

##################################################################################################################
##ps dev module at commands
##################################################################################################################
ifeq ($(BUILD_PS_DEV_AT_ENABLE),y)
CFLAGS += -DFEATURE_PS_DEV_AT_ENABLE

ifeq ($(BUILD_PS_DEV_CFUN_ENABLE),y)
CFLAGS += -DFEATURE_PS_DEV_CFUN_ENABLE
endif

ifeq ($(BUILD_PS_DEV_ECCGSN_ENABLE),y)
CFLAGS += -DFEATURE_PS_DEV_ECCGSN_ENABLE
endif

ifeq ($(BUILD_PS_DEV_ECNETDEVMAC_ENABLE),y)
CFLAGS += -DFEATURE_PS_DEV_ECNETDEVMAC_ENABLE
endif

ifeq ($(BUILD_PS_DEV_ECCGSNLOCK_ENABLE),y)
CFLAGS += -DFEATURE_PS_DEV_ECCGSNLOCK_ENABLE
endif

ifeq ($(BUILD_PS_DEV_ECSTATUS_ENABLE),y)
CFLAGS += -DFEATURE_PS_DEV_ECSTATUS_ENABLE
endif

ifeq ($(BUILD_PS_DEV_ECSTATIS_ENABLE),y)
CFLAGS += -DFEATURE_PS_DEV_ECSTATIS_ENABLE
endif

ifeq ($(BUILD_PS_DEV_ECBAND_ENABLE),y)
CFLAGS += -DFEATURE_PS_DEV_ECBAND_ENABLE
endif

ifeq ($(BUILD_PS_DEV_ECFREQ_ENABLE),y)
CFLAGS += -DFEATURE_PS_DEV_ECFREQ_ENABLE
endif

ifeq ($(BUILD_PS_DEV_ECCFG_ENABLE),y)
CFLAGS += -DFEATURE_PS_DEV_ECCFG_ENABLE
endif

ifeq ($(BUILD_PS_DEV_ECMEASCFG_ENABLE),y)
CFLAGS += -DFEATURE_PS_DEV_ECMEASCFG_ENABLE
endif

ifeq ($(BUILD_PS_DEV_ECRMFPLMN_ENABLE),y)
CFLAGS += -DFEATURE_PS_DEV_ECRMFPLMN_ENABLE
endif

ifeq ($(BUILD_PS_DEV_ECBCINFO_ENABLE),y)
CFLAGS += -DFEATURE_PS_DEV_ECBCINFO_ENABLE
endif

ifeq ($(BUILD_PS_DEV_ECPSTEST_ENABLE),y)
CFLAGS += -DFEATURE_PS_DEV_ECPSTEST_ENABLE
endif

ifeq ($(BUILD_PS_DEV_ECEVENTSTATIS_ENABLE),y)
CFLAGS += -DFEATURE_PS_DEV_ECEVENTSTATIS_ENABLE
endif

ifeq ($(BUILD_PS_DEV_ECNASTCFG_ENABLE),y)
CFLAGS += -DFEATURE_PS_DEV_ECNASTCFG_ENABLE
endif

ifeq ($(BUILD_PS_DEV_ECWIFISCAN_ENABLE),y)
CFLAGS += -DFEATURE_PS_DEV_ECWIFISCAN_ENABLE
endif

ifeq ($(BUILD_PS_DEV_ECBARCELL_ENABLE),y)
CFLAGS += -DFEATURE_PS_DEV_ECBARCELL_ENABLE
endif

ifeq ($(BUILD_PS_DEV_ECJDC_ENABLE),y)
CFLAGS += -DFEATURE_PS_DEV_ECJDC_ENABLE
endif

endif
##################################################################################################################
##ps emm module at commands
##################################################################################################################
ifeq ($(BUILD_PS_EMM_AT_ENABLE), y)
CFLAGS += -DFEATURE_PS_MM_AT_ENABLE

ifeq ($(BUILD_PS_EMM_CSQ_ENABLE),y)
CFLAGS += -DFEATURE_PS_EMM_CSQ_ENABLE
endif

ifeq ($(BUILD_PS_EMM_CESQ_ENABLE),y)
CFLAGS += -DFEATURE_PS_EMM_CESQ_ENABLE
endif

ifeq ($(BUILD_PS_EMM_CREG_ENABLE),y)
CFLAGS += -DFEATURE_PS_EMM_CREG_ENABLE
endif

ifeq ($(BUILD_PS_EMM_COPS_ENABLE),y)
CFLAGS += -DFEATURE_PS_EMM_COPS_ENABLE
endif

ifeq ($(BUILD_PS_EMM_CPSMS_ENABLE),y)
CFLAGS += -DFEATURE_PS_EMM_CPSMS_ENABLE
endif

ifeq ($(BUILD_PS_EMM_CEDRXS_ENABLE),y)
CFLAGS += -DFEATURE_PS_EMM_CEDRXS_ENABLE
endif

ifeq ($(BUILD_PS_EMM_CEDRXRDP_ENABLE),y)
CFLAGS += -DFEATURE_PS_EMM_CEDRXRDP_ENABLE
endif

ifeq ($(BUILD_PS_EMM_CCIOTOPT_ENABLE),y)
CFLAGS += -DFEATURE_PS_EMM_CCIOTOPT_ENABLE
endif

ifeq ($(BUILD_PS_EMM_CRCES_ENABLE),y)
CFLAGS += -DFEATURE_PS_EMM_CRCES_ENABLE
endif

ifeq ($(BUILD_PS_EMM_CCLK_ENABLE),y)
CFLAGS += -DFEATURE_PS_EMM_CCLK_ENABLE
endif

ifeq ($(BUILD_PS_EMM_CTZR_ENABLE),y)
CFLAGS += -DFEATURE_PS_EMM_CTZR_ENABLE
endif

ifeq ($(BUILD_PS_EMM_CTZU_ENABLE),y)
CFLAGS += -DFEATURE_PS_EMM_CTZU_ENABLE
endif

ifeq ($(BUILD_PS_EMM_ECPLMNS_ENABLE),y)
CFLAGS += -DFEATURE_PS_EMM_ECPLMNS_ENABLE
endif

ifeq ($(BUILD_PS_EMM_ECCESQS_ENABLE),y)
CFLAGS += -DFEATURE_PS_EMM_ECCESQS_ENABLE
endif

ifeq ($(BUILD_PS_EMM_ECPSMR_ENABLE),y)
CFLAGS += -DFEATURE_PS_EMM_ECPSMR_ENABLE
endif

ifeq ($(BUILD_PS_EMM_ECPTWEDRXS_ENABLE),y)
CFLAGS += -DFEATURE_PS_EMM_ECPTWEDRXS_ENABLE
endif

ifeq ($(BUILD_PS_EMM_ECEMMTIME_ENABLE),y)
CFLAGS += -DFEATURE_PS_EMM_ECEMMTIME_ENABLE
endif

ifeq ($(BUILD_PS_EMM_ECPLMNRESELECT_ENABLE),y)
CFLAGS += -DFEATURE_PS_EMM_ECPLMNRESELECT_ENABLE
endif

ifeq ($(BUILD_PS_EMM_ECPLMNINTERVAL_ENABLE),y)
CFLAGS += -DFEATURE_PS_EMM_ECPLMNINTERVAL_ENABLE
endif

ifeq ($(BUILD_PS_EMM_ECCONNREL_ENABLE),y)
CFLAGS += -DFEATURE_PS_EMM_ECCONNREL_ENABLE
endif

ifeq ($(BUILD_PS_EMM_ECMMER_ENABLE), y)
CFLAGS += -DFEATURE_PS_EMM_ECMMER_ENABLE
endif

ifeq ($(BUILD_PS_EMM_ECEHPLMNLIST_ENABLE), y)
CFLAGS += -DFEATURE_PS_EMM_ECEHPLMNLIST_ENABLE
endif

endif

##################################################################################################################
##ps eps  module at commands
##################################################################################################################

ifeq ($(BUILD_PS_EPS_AT_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_AT_ENABLE

ifeq ($(BUILD_PS_EPS_CGATT_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_CGATT_ENABLE
endif

ifeq ($(BUILD_PS_EPS_ECEMCATT_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_EMCATT_ENABLE
endif

ifeq ($(BUILD_PS_EPS_CGDATA_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_CGDATA_ENABLE
endif

ifeq ($(BUILD_PS_EPS_CGTFT_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_CGTFT_ENABLE
endif

ifeq ($(BUILD_PS_EPS_CGCMOD_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_CGCMOD_ENABLE
endif

ifeq ($(BUILD_PS_EPS_CGDCONT_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_CGDCONT_ENABLE
endif

ifeq ($(BUILD_PS_EPS_CGACT_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_CGACT_ENABLE
endif

ifeq ($(BUILD_PS_EPS_CGEQOS_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_CGEQOS_ENABLE
endif

ifeq ($(BUILD_PS_EPS_CGEQOSRDP_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_CGEQOSRDP_ENABLE
endif

ifeq ($(BUILD_PS_EPS_CGCONTRDP_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_CGCONTRDP_ENABLE
endif

ifeq ($(BUILD_PS_EPS_CGSCONTRDP_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_CGSCONTRDP_ENABLE
endif

ifeq ($(BUILD_PS_EPS_CGTFTRDP_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_CGTFTRDP_ENABLE
endif

ifeq ($(BUILD_PS_EPS_CEREG_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_CEREG_ENABLE
endif

ifeq ($(BUILD_PS_EPS_CSCON_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_CSCON_ENABLE
endif

ifeq ($(BUILD_PS_EPS_CSODCP_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_CSODCP_ENABLE
endif

ifeq ($(BUILD_PS_EPS_CRTDCP_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_CRTDCP_ENABLE
endif

ifeq ($(BUILD_PS_EPS_CGAUTH_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_CGAUTH_ENABLE
endif

ifeq ($(BUILD_PS_EPS_CIPCA_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_CIPCA_ENABLE
endif

ifeq ($(BUILD_PS_EPS_CGAPNRC_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_CGAPNRC_ENABLE
endif

ifeq ($(BUILD_PS_EPS_CGEREP_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_CGEREP_ENABLE
endif

ifeq ($(BUILD_PS_EPS_CGDSCONT_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_CGDSCONT_ENABLE
endif

ifeq ($(BUILD_PS_EPS_CGPADDR_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_CGPADDR_ENABLE
endif

ifeq ($(BUILD_PS_EPS_CEER_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_CEER_ENABLE
endif

ifeq ($(BUILD_PS_EPS_CEMODE_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_CEMODE_ENABLE
endif

ifeq ($(BUILD_PS_EPS_CEVDP_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_CEVDP_ENABLE
endif

ifeq ($(BUILD_PS_EPS_CPSDO_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_CPSDO_ENABLE
endif

ifeq ($(BUILD_PS_EPS_ECCIOTPLANE_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_ECCIOTPLANE_ENABLE
endif

ifeq ($(BUILD_PS_EPS_ECSENDDATA_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_ECSENDDATA_ENABLE
endif

ifeq ($(BUILD_PS_EPS_ECGDCNT_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_ECGDCNT_ENABLE
endif

ifeq ($(BUILD_PS_EPS_ECAUGDCNT_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_ECAUGDCNT_ENABLE
endif

ifeq ($(BUILD_PS_EPS_ECSCLKEX_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_ECSCLKEX_ENABLE
endif

ifeq ($(BUILD_PS_EPS_CNEC_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_CNEC_ENABLE
endif

ifeq ($(BUILD_PS_EPS_ECSMER_ENABLE), y)
CFLAGS += -DFEATURE_PS_EPS_ECSMER_ENABLE
endif

endif


##################################################################################################################
##ps sim  module at commands
##################################################################################################################

ifeq ($(BUILD_PS_SIM_AT_ENABLE), y)
CFLAGS += -DFEATURE_PS_SIM_AT_ENABLE

ifeq ($(BUILD_PS_SIM_AT_CIMI_ENABLE), y)
CFLAGS += -DFEATURE_PS_SIM_AT_CIMI_ENABLE
endif

ifeq ($(BUILD_PS_SIM_AT_CPIN_ENABLE), y)
CFLAGS += -DFEATURE_PS_SIM_AT_CPIN_ENABLE
endif

ifeq ($(BUILD_PS_SIM_AT_ECICCID_ENABLE), y)
CFLAGS += -DFEATURE_PS_SIM_AT_ECICCID_ENABLE
endif

ifeq ($(BUILD_PS_SIM_AT_ECSIMCFG_ENABLE), y)
CFLAGS += -DFEATURE_PS_SIM_AT_ECSIMCFG_ENABLE
endif

ifeq ($(BUILD_PS_SIM_AT_CPWD_ENABLE), y)
CFLAGS += -DFEATURE_PS_SIM_AT_CPWD_ENABLE
endif

ifeq ($(BUILD_PS_SIM_AT_CPINR_ENABLE), y)
CFLAGS += -DFEATURE_PS_SIM_AT_CPINR_ENABLE
endif

ifeq ($(BUILD_PS_SIM_AT_CSIM_ENABLE), y)
CFLAGS += -DFEATURE_PS_SIM_AT_CSIM_ENABLE
endif

ifeq ($(BUILD_PS_SIM_AT_CRSM_ENABLE), y)
CFLAGS += -DFEATURE_PS_SIM_AT_CRSM_ENABLE
endif

ifeq ($(BUILD_PS_SIM_AT_ECSIMSLEEP_ENABLE), y)
CFLAGS += -DFEATURE_PS_SIM_AT_ECSIMSLEEP_ENABLE
endif

ifeq ($(BUILD_PS_SIM_AT_CCHO_ENABLE), y)
CFLAGS += -DFEATURE_PS_SIM_AT_CCHO_ENABLE
endif

ifeq ($(BUILD_PS_SIM_AT_CGLA_ENABLE), y)
CFLAGS += -DFEATURE_PS_SIM_AT_CGLA_ENABLE
endif

ifeq ($(BUILD_PS_SIM_AT_CCHC_ENABLE), y)
CFLAGS += -DFEATURE_PS_SIM_AT_CCHC_ENABLE
endif

ifeq ($(BUILD_PS_SIM_AT_ECSWC_ENABLE), y)
CFLAGS += -DFEATURE_PS_SIM_AT_ECSWC_ENABLE
endif

ifeq ($(BUILD_PS_SIM_AT_ECSIMRM_ENABLE), y)
CFLAGS += -DFEATURE_PS_SIM_AT_ECSIMRM_ENABLE
endif

ifeq ($(BUILD_PS_SIM_AT_CNUM_ENABLE), y)
CFLAGS += -DFEATURE_PS_SIM_AT_CNUM_ENABLE
endif

ifeq ($(BUILD_PS_SIM_AT_ECUSATP_ENABLE), y)
CFLAGS += -DFEATURE_PS_SIM_AT_ECUSATP_ENABLE
endif

ifeq ($(BUILD_PS_SIM_AT_CPOL_ENABLE), y)
CFLAGS += -DFEATURE_PS_SIM_AT_CPOL_ENABLE
endif

ifeq ($(BUILD_PS_SIM_AT_CPLS_ENABLE), y)
CFLAGS += -DFEATURE_PS_SIM_AT_CPLS_ENABLE
endif

endif

##################################################################################################################
##ps sms module at commands
##################################################################################################################
ifeq ($(BUILD_PS_SMS_AT_ENABLE), y)
CFLAGS += -DFEATURE_PS_SMS_AT_ENABLE

ifeq ($(BUILD_PS_SMS_CMGS_ENABLE), y)
CFLAGS += -DFEATURE_PS_SMS_CMGS_ENABLE
endif

ifeq ($(BUILD_PS_SMS_CMGC_ENABLE), y)
CFLAGS += -DFEATURE_PS_SMS_CMGC_ENABLE
endif

ifeq ($(BUILD_PS_SMS_CMGR_ENABLE), y)
CFLAGS += -DFEATURE_PS_SMS_CMGR_ENABLE
endif

ifeq ($(BUILD_PS_SMS_CMGW_ENABLE), y)
CFLAGS += -DFEATURE_PS_SMS_CMGW_ENABLE
endif

ifeq ($(BUILD_PS_SMS_CSCA_ENABLE), y)
CFLAGS += -DFEATURE_PS_SMS_CSCA_ENABLE
endif

ifeq ($(BUILD_PS_SMS_CNMI_ENABLE), y)
CFLAGS += -DFEATURE_PS_SMS_CNMI_ENABLE
endif

ifeq ($(BUILD_PS_SMS_CMMS_ENABLE), y)
CFLAGS += -DFEATURE_PS_SMS_CMMS_ENABLE
endif

ifeq ($(BUILD_PS_SMS_CMGD_ENABLE), y)
CFLAGS += -DFEATURE_PS_SMS_CMGD_ENABLE
endif

ifeq ($(BUILD_PS_SMS_CMGL_ENABLE), y)
CFLAGS += -DFEATURE_PS_SMS_CMGL_ENABLE
endif

ifeq ($(BUILD_PS_SMS_CSMS_ENABLE), y)
CFLAGS += -DFEATURE_PS_SMS_CSMS_ENABLE
endif

ifeq ($(BUILD_PS_SMS_CPMS_ENABLE), y)
CFLAGS += -DFEATURE_PS_SMS_CPMS_ENABLE
endif

ifeq ($(BUILD_PS_SMS_CNMA_ENABLE), y)
CFLAGS += -DFEATURE_PS_SMS_CNMA_ENABLE
endif

ifeq ($(BUILD_PS_SMS_CMGF_ENABLE), y)
CFLAGS += -DFEATURE_PS_SMS_CMGF_ENABLE
endif

ifeq ($(BUILD_PS_SMS_CSMP_ENABLE), y)
CFLAGS += -DFEATURE_PS_SMS_CSMP_ENABLE
endif

ifeq ($(BUILD_PS_SMS_CSCB_ENABLE), y)
CFLAGS += -DFEATURE_PS_SMS_CSCB_ENABLE
endif

ifeq ($(BUILD_PS_SMS_CSDH_ENABLE), y)
CFLAGS += -DFEATURE_PS_SMS_CSDH_ENABLE
endif

ifeq ($(BUILD_PS_SMS_CMSS_ENABLE), y)
CFLAGS += -DFEATURE_PS_SMS_CMSS_ENABLE
endif

ifeq ($(BUILD_PS_SMS_ECSMSSEND_ENABLE), y)
CFLAGS += -DFEATURE_PS_SMS_ECSMSSEND_ENABLE
endif

ifeq ($(BUILD_PS_SMS_ECCMGS_ENABLE), y)
CFLAGS += -DFEATURE_PS_SMS_ECCMGS_ENABLE
endif

ifeq ($(BUILD_PS_SMS_ECCMGR_ENABLE), y)
CFLAGS += -DFEATURE_PS_SMS_ECCMGR_ENABLE
endif
endif

##################################################################################################################
##phy config at commands
##################################################################################################################
ifeq ($(BUILD_PHY_CONFIG_AT_ENABLE), y)
CFLAGS += -DFEATURE_PHY_CONFIG_AT_ENABLE

ifeq ($(BUILD_PHY_CONFIG_ECPHYCFG_ENABLE), y)
CFLAGS += -DFEATURE_PHY_CONFIG_ECPHYCFG_ENABLE
endif
ifeq ($(BUILD_PHY_CONFIG_ECEDRXSIMU_ENABLE), y)
CFLAGS += -DFEATURE_PHY_CONFIG_ECEDRXSIMU_ENABLE
endif
endif

##################################################################################################################
##ps tcpip  module EC SOCKET and SKT SOCKET at commands
##################################################################################################################

ifeq ($(BUILD_PS_TCPIP_ECSOCK_SKTSOCK_AT_ENABLE), y)
CFLAGS += -DFEATURE_PS_TCPIP_ECSOCK_SKTSOCK_AT_ENABLE
endif

##################################################################################################################
##ps tcpip  iperf service commands
##################################################################################################################

ifeq ($(THIRDPARTY_IPERF_ENABLE), y)
CFLAGS += -DFEATURE_PS_TCPIP_IPERF_SERVICE_ENABLE
endif

##################################################################################################################
##ps tcpip  ping service commands
##################################################################################################################

ifeq ($(THIRDPARTY_PING_ENABLE), y)
CFLAGS += -DFEATURE_PS_TCPIP_PING_SERVICE_ENABLE
endif

##################################################################################################################
##ps tcpip  common service AT commands
##################################################################################################################

ifeq ($(BUILD_PS_TCPIP_NETCONFIG_AT_ENABLE), y)
CFLAGS += -DFEATURE_PS_TCPIP_NETCONFIG_AT_ENABLE
endif


##################################################################################################################
##ps tcpip  sntp service commands
##################################################################################################################

ifeq ($(THIRDPARTY_LIBSNTP_ENABLE), y)
CFLAGS += -DFEATURE_PS_TCPIP_SNTP_SERVICE_ENABLE
endif

##################################################################################################################
##ps tcpip  api
##################################################################################################################

ifeq ($(BUILD_PS_TCPIP_API_ENABLE), y)
CFLAGS += -DFEATURE_PS_TCPIP_API_ENABL
endif

##################################################################################################################
##ps tcpip ipsec AT :  AT+ECIPSECCREATE/ECIPSECDELETE
##################################################################################################################
ifeq ($(BUILD_PS_TCPIP_IPSEC_AT_ENABLE), y)
CFLAGS += -DFEATURE_PS_TCPIP_IPSEC_AT_ENABLE
endif

##################################################################################################################
##ps ref at commands
##################################################################################################################

ifeq ($(BUILD_AT_REF_QR),y)

ifeq ($(BUILD_PS_REF_QR_TCPIP_QSOCK_AT_ENABLE),y)
CFLAGS += -DFEATURE_PS_REF_QR_TCPIP_QSOCK_AT_ENABLE
endif

ifeq ($(BUILD_PS_REF_QR_TCPIP_QNETCONFIG_AT_ENABLE),y)
CFLAGS += -DFEATURE_PS_REF_QR_TCPIP_QNETCONFIG_AT_ENABLE
endif

ifeq ($(BUILD_PS_REF_QR_TCPIP_QPING_SERVICE_ENABLE),y)
CFLAGS += -DFEATURE_PS_REF_QR_TCPIP_QPING_SERVICE_ENABLE
endif

ifeq ($(BUILD_PS_REF_QR_TCPIP_QSNTP_SERVICE_ENABLE),y)
CFLAGS += -DFEATURE_PS_REF_QR_TCPIP_QSNTP_SERVICE_ENABLE
endif

ifeq ($(BUILD_PS_REF_QR_EPS_AT_ENABLE),y)
CFLAGS += -DFEATURE_PS_REF_QR_EPS_AT_ENABLE
endif

ifeq ($(BUILD_PS_REF_QR_COMMON_AT_ENABLE),y)
CFLAGS += -DFEATURE_PS_REF_QR_COMMON_AT_ENABLE
endif

endif

ifeq ($(BUILD_AT_REF_CR),y)

ifeq ($(BUILD_PS_REF_CR_EPS_AT_ENABLE),y)
CFLAGS += -DFEATURE_PS_REF_CR_EPS_AT_ENABLE
endif

ifeq ($(BUILD_PS_REF_CR_MCCID_ENABLE),y)
CFLAGS += -DFEATURE_PS_REF_CR_MCCID_ENABLE
endif

ifeq ($(BUILD_PS_REF_CR_ICCID_ENABLE),y)
CFLAGS += -DFEATURE_PS_REF_CR_ICCID_ENABLE
endif

ifeq ($(BUILD_PS_REF_CR_MLOCKFREQ_ENABLE),y)
CFLAGS += -DFEATURE_PS_REF_CR_MLOCKFREQ_ENABLE
endif

ifeq ($(BUILD_PS_REF_CR_MCSEARFCN_ENABLE),y)
CFLAGS += -DFEATURE_PS_REF_CR_MCSEARFCN_ENABLE
endif

ifeq ($(BUILD_PS_REF_CR_MUESTATS_ENABLE),y)
CFLAGS += -DFEATURE_PS_REF_CR_MUESTATS_ENABLE
endif

ifeq ($(BUILD_PS_REF_CR_MBAND_ENABLE),y)
CFLAGS += -DFEATURE_PS_REF_CR_MBAND_ENABLE
endif

ifeq ($(BUILD_PS_REF_CR_MIPCALL_ENABLE),y)
CFLAGS += -DFEATURE_PS_REF_CR_MIPCALL_ENABLE
endif

ifeq ($(BUILD_PS_REF_CR_MWIFISCAN_ENABLE),y)
CFLAGS += -DFEATURE_PS_REF_CR_MWIFISCAN_ENABLE
endif

ifeq ($(BUILD_PS_REF_CR_TCPIP_SOCK_AT_ENABLE),y)
CFLAGS += -DFEATURE_PS_REF_CR_TCPIP_SOCK_AT_ENABLE
endif

ifeq ($(BUILD_PS_REF_CR_TCPIP_PING_SERVICE_ENABLE),y)
CFLAGS += -DFEATURE_PS_REF_CR_TCPIP_PING_SERVICE_ENABLE
endif

ifeq ($(BUILD_PS_REF_CR_TCPIP_SNTP_SERVICE_ENABLE),y)
CFLAGS += -DFEATURE_PS_REF_CR_TCPIP_SNTP_SERVICE_ENABLE
endif

ifeq ($(BUILD_PS_REF_CR_TCPIP_NETCONFIG_AT_ENABLE),y)
CFLAGS += -DFEATURE_PS_REF_CR_TCPIP_NETCONFIG_AT_ENABLE
endif

ifeq ($(BUILD_PS_REF_CR_MEID_ENABLE),y)
CFLAGS += -DFEATURE_PS_REF_CR_MEID_AT_ENABLE
endif


endif
##################################################################################################################
##cust mifi at commands
##################################################################################################################
ifeq ($(BUILD_PLAT_MIFI_AT_ENABLE), y)
CFLAGS += -DFEATURE_MIFI_AT_ENABLE
endif