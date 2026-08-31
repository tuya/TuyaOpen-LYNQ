CHIP                        = ec7xx
CHIP_VERSION                = A0
CORE                        = ap
FPGA_DEBUG                  ?= n
ifeq ($(HIMS_ENABLE),true)
OS                           = liteos
else
OS                           = freertos
endif
BIN_COMPRESS                ?= n
EUTRAN_CAT_MODE             ?= n
BUILD_USE_PREBUILD_LIB      = y
BUILD_USE_USB_LIB           = y
DRIVER_BLUSB_CUTDOWN_DIR_EXIST = y
BUILD_FOR_KEY_CUSTOMER1     = n
BUILD_PS                    ?= y
BUILD_FW                    ?= n
#No mater BUILD_USE_PREBUILD_LIB, below is configurable.
DRIVER_DMA_ENABLE           ?= y
DRIVER_CACHE_ENABLE         ?= y
DRIVER_PMU_ENABLE           ?= y
DRIVER_FLASHRT_ENABLE       ?= y
DRIVER_FLASH_ENABLE         ?= y
DRIVER_CAMERA_ENABLE        ?= n
DRIVER_LCD_ENABLE           ?= n
DRIVER_GPR_ENABLE           ?= y
DRIVER_IC_ENABLE            ?= y
DRIVER_PWRKEY_ENABLE        ?= y
DRIVER_CHARGE_ENABLE        ?= y
DRIVER_ALARM_ENABLE         ?= y
DRIVER_CIPHER_ENABLE        ?= y
DRIVER_EFUSE_ENABLE         ?= y
DRIVER_SCT_ENABLE           ?= y
DRIVER_RNG_ENABLE           ?= y
DRIVER_SWIO_ENABLE          ?= y
DRIVER_MCUMODE_ENABLE       ?= y
DRIVER_ECMAIN_ENABLE        ?= y
DRIVER_PLATCFG_ENABLE       ?= y
DRIVER_IOCTRL_ENABLE        ?= y
RF_ADI_USED_ENABLE          ?= n
BUILD_EC_MW                 ?= y
BUILD_OS                    ?= y
BUILD_OSA                   ?= y
BUILD_AT                    ?= y
BUILD_AT_DEBUG              ?= y
#Choose if below driver need to be compiled or not
DRIVER_CAN_ENABLE           ?= n
DRIVER_USART_ENABLE         ?= y
DRIVER_LPUART_ENABLE        ?= y
DRIVER_SPI_ENABLE           ?= y
DRIVER_I2C_ENABLE           ?= y
DRIVER_I2S_ENABLE           ?= y
DRIVER_CODEC_ENABLE         ?= n
DRIVER_GPIO_ENABLE          ?= y
DRIVER_TIMER_ENABLE         ?= y
DRIVER_ONEWIRE_ENABLE       ?= y
DRIVER_EEPROM_ENABLE        ?= n
DRIVER_TIMER_ENABLE         ?= y
DRIVER_UNILOG_ENABLE        ?= y
DRIVER_VPU_ENABLE           ?= y
DRIVER_WDG_ENABLE           ?= y
DRIVER_ADC_ENABLE           ?= y
DRIVER_IPC_ENABLE           ?= y
DRIVER_RESET_ENABLE         ?= y
DRIVER_KPC_ENABLE           ?= n
DRIVER_ULDP_ENABLE          ?= y
DRIVER_UTFC_ENABLE          ?= y
DRIVER_USB_ENABLE           ?= y
DRIVER_EXCP_ENABLE          ?= y
DRIVER_RFCALI_ENABLE        ?= y
DRIVER_PHYHAL_ENABLE        ?= y
DRIVER_CPADC_ENABLE         ?= y
DRIVER_NTC_ENABLE           ?= n
DRIVER_EXSTORAGE_ENABLE     ?= n
DRIVER_PSRAM_ENABLE         ?= n
DRIVER_CPFLASH_ENABLE       ?= n
DRIVER_ACVOICE_ENG_ENABLE   ?= n
DRIVER_POWBACKOFF_ENABLE    ?= n
ifeq ($(DRIVER_EFUSE_ENABLE)_$(DRIVER_ADC_ENABLE),y_y)
DRIVER_HAL_ADC_ENABLE = y
else
DRIVER_HAL_ADC_ENABLE       ?= n
endif
ifeq ($(TYPE), ec718pm)
DRIVER_APM_ENABLE           ?= n
DRIVER_XPI_PSRAM_ENABLE     ?= y
endif
ifeq ($(TYPE), ec718um)
DRIVER_APM_ENABLE           ?= n
DRIVER_XPI_PSRAM_ENABLE     ?= y
endif
ifeq ($(TYPE), ec718sm)
DRIVER_APM_ENABLE           ?= n
DRIVER_XPI_PSRAM_ENABLE     ?= y
endif
ifeq ($(TYPE), ec718hm)
DRIVER_APM_ENABLE           ?= n
DRIVER_XPI_PSRAM_ENABLE     ?= y
endif
#choose if usb net automatically adapt to host os
USBNET_AUTO_ADAPT_ENABLE    ?= n
#Choose if below thirdparty sofware need to be compiled
THIRDPARTY_MQTT_ENABLE      ?= n
THIRDPARTY_HTTPC_ENABLE     ?= n
THIRDPARTY_LITTEFS_ENABLE   ?= y
THIRDPARTY_IPERF_ENABLE     ?= y
THIRDPARTY_CJSON_ENABLE     ?= n
THIRDPARTY_LWIP_ENABLE      ?= y
THIRDPARTY_PING_ENABLE      ?= y
THIRDPARTY_DHCPD_ENABLE     ?= y
THIRDPARTY_PPP_ENABLE       ?= y
THIRDPARTY_MBEDTLS_ENABLE   ?= y
THIRDPARTY_LIBSNTP_ENABLE   ?= y
THIRDPARTY_CTCC_DM_ENABLE   ?= n
THIRDPARTY_CUCC_DM_ENABLE   ?= n
THIRDPARTY_CMCC_DM_ENABLE   ?= n
THIRDPARTY_EXAMPLE_ENABLE   ?= n
THIRDPARTY_QUIRC_ENABLE     ?= n
THIRDPARTY_BARCODE_ENABLE   ?= n
THIRDPARTY_MP3DECODE_ENABLE ?= n
THIRDPARTY_MP3ENCODE_ENABLE ?= n
THIRDPARTY_AACDECODE_ENABLE ?= n
THIRDPARTY_AACENCODE_ENABLE ?= n
THIRDPARTY_YRCOMPRESS_ENABLE ?= n
THIRDPARTY_DHRYSTONE_ENABLE ?= n
THIRDPARTY_COREMARK_ENABLE  ?= n
THIRDPARTY_CTWING_CERTI_ENABLE ?= n
THIRDPARTY_LZMA_ENABLE         ?= n
THIRDPARTY_BZIP2_ENABLE        ?= n
THIRDPARTY_UECC_ENABLE        ?= n
#Choose if below ecomm middleware  need to be compiled
MIDDLEWARE_DEBUG_TRACE_ENABLE ?=n
MIDDLEWARE_ECAPI_AAL_ENABLE   ?=y
MIDDLEWARE_CCIO_ENABLE        ?=y
MIDDLEWARE_CMS_ENABLE         ?=y
MIDDLEWARE_SIMBIP_ENABLE      ?=y
MIDDLEWARE_NVRAM_ENABLE       ?=y
MIDDLEWARE_FOTA_ENABLE        ?=n
MIDDLEWARE_FOTAPAR_ENABLE     ?=n
MIDDLEWARE_FOTA_USBURC_ENABLE ?=n
MIDDLEWARE_FEATURE_PLATTEST_ENABLE ?=n
MIDDLEWARE_USB_CCID_ENABLE    ?=n
MIDDLEWARE_AMR_ENABLE         ?=n
MIDDLEWARE_VEM_ENABLE         ?=n
#add for AT REF,default is undefined 
BUILD_AT_REF_QR               ?=n
BUILD_AT_REF_CR               ?=n
BUILD_PS_ROHC_ENABLE          ?=n
BUILD_IMS                     ?=n
ifeq ($(LTO_ENABLE), true)
LTO_FEATURE_ENABLE ?= y
else
LTO_FEATURE_ENABLE ?= n
endif
ifeq ($(GCF_ENABLE), true)
GCF_FEATURE_ENABLE ?= y
else
GCF_FEATURE_ENABLE ?= n
endif
ifeq ($(USBNET_AUTO_ADAPT_ENABLE), y)
CFLAGS_DEFS += -DFEATURE_PLAT_CFG_FS_SUP_USBNET_ATA
endif
ifeq ($(PWR_TEST), true)
MIDDLEWARE_FEATURE_ATPWRTEST_ENABLE ?= y
CFLAGS_DEFS += -DFEATURE_AT_PWR_TEST
endif
#default AT config value, should align with normal at cmd project  
include $(TOP)/PLAT/device/target/board/$(BOARD_NAME)/ap/at_config.inc
ifeq ($(TYPE), ec718s)
  CFLAGS_DEFS += -DTYPE_EC718S
  CFLAGS += -D__EC718
  ifeq ($(CHIP_VERSION),A0)
  CFLAGS_DEFS += -DCHIP_EC718
  ASMFLAGS += --predefine "CHIP_EC718_A0 SETL {TRUE} "
  endif
else ifeq ($(TYPE), ec718h)
  CFLAGS_DEFS += -DTYPE_EC718H
  CFLAGS += -D__EC718
  ifeq ($(CHIP_VERSION),A0)
  CFLAGS_DEFS += -DCHIP_EC718
  ASMFLAGS += --predefine "CHIP_EC718_A0 SETL {TRUE} "
  endif
else ifeq ($(TYPE), ec718p)
  CFLAGS_DEFS += -DTYPE_EC718P
  CFLAGS += -D__EC718
  ifeq ($(CHIP_VERSION),A0)
  CFLAGS_DEFS += -DCHIP_EC718
  ASMFLAGS += --predefine "CHIP_EC718_A0 SETL {TRUE} "
  endif
else ifeq ($(TYPE), ec718u)
  CFLAGS_DEFS += -DTYPE_EC718U
  CFLAGS += -D__EC718
  ifeq ($(CHIP_VERSION),A0)
  CFLAGS_DEFS += -DCHIP_EC718
  ASMFLAGS += --predefine "CHIP_EC718_A0 SETL {TRUE} "
  endif
else ifeq ($(TYPE), ec716s)
  CFLAGS_DEFS += -DTYPE_EC716S
  CFLAGS += -D__EC716
  ifeq ($(CHIP_VERSION),A0)
  CFLAGS_DEFS += -DCHIP_EC716
  ASMFLAGS += --predefine "CHIP_EC716_A0 SETL {TRUE} "
  endif
else ifeq ($(TYPE), ec716e)
  CFLAGS_DEFS += -DTYPE_EC716E
  CFLAGS += -D__EC716
  ifeq ($(CHIP_VERSION),A0)
  CFLAGS_DEFS += -DCHIP_EC716
  ASMFLAGS += --predefine "CHIP_EC716_A0 SETL {TRUE} "
  endif
else ifeq ($(TYPE), ec718pm)
  CFLAGS_DEFS += -DTYPE_EC718PM
  CFLAGS_DEFS += -DTYPE_EC718M
  CFLAGS += -D__EC718
  ifeq ($(CHIP_VERSION),A0)
  CFLAGS_DEFS += -DCHIP_EC718
  ASMFLAGS += --predefine "CHIP_EC718_A0 SETL {TRUE} "
  endif
else ifeq ($(TYPE), ec718um)
  CFLAGS_DEFS += -DTYPE_EC718UM
  CFLAGS_DEFS += -DTYPE_EC718M
  CFLAGS += -D__EC718
  ifeq ($(CHIP_VERSION),A0)
  CFLAGS_DEFS += -DCHIP_EC718
  ASMFLAGS += --predefine "CHIP_EC718_A0 SETL {TRUE} "
  endif
else ifeq ($(TYPE), ec718sm)
  CFLAGS_DEFS += -DTYPE_EC718SM
  CFLAGS_DEFS += -DTYPE_EC718M
  CFLAGS += -D__EC718
  ifeq ($(CHIP_VERSION),A0)
  CFLAGS_DEFS += -DCHIP_EC718
  ASMFLAGS += --predefine "CHIP_EC718_A0 SETL {TRUE} "
  endif
else ifeq ($(TYPE), ec718hm)
  CFLAGS_DEFS += -DTYPE_EC718HM
  CFLAGS_DEFS += -DTYPE_EC718M
  CFLAGS += -D__EC718
  ifeq ($(CHIP_VERSION),A0)
  CFLAGS_DEFS += -DCHIP_EC718
  ASMFLAGS += --predefine "CHIP_EC718_A0 SETL {TRUE} "
  endif
endif
#CFLAGS += --no_integer_literal_pools
ifeq ($(CORE),ap)
CFLAGS += -DCORE_IS_AP
endif
