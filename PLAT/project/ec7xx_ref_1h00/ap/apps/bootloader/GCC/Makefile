AVAILABLE_TARGETS = ec7xx_ref_1h00
TOOLCHAIN         = GCC
BINNAME           = ap_bootloader
TOP              := ../../../../../../..

FLOAT_FLAG_ENABLE            = n

BIN_COMPRESS                 = n
FPGA_DEBUG                   = n
BUILD_EC_MW                  = n
BUILD_PS                     = n
BUILD_FW                     = n
BUILD_OS                     = n
BUILD_OSA                    = n
BUILD_AT                     = n
BUILD_AT_DEBUG               = n

MBTK_OPENCPU_SUPPORT         = n
include $(TOP)/PLAT/middleware/developed/build_config.inc

DRIVER_PMU_ENABLE            = n
DRIVER_FLASHRT_ENABLE        = n
DRIVER_ECMAIN_ENABLE         = n
DRIVER_UTFC_ENABLE           = n
DRIVER_VPU_ENABLE            = n
#DRIVER_GPR_ENABLE           = y
DRIVER_ULDP_ENABLE           = n
DRIVER_USB_ENABLE            = n
DRIVER_USBSMIMG_ENABLE       = y
ifeq ($(TYPE), ec718h)
DRIVER_CPFLASH_ENABLE        = y
endif
THIRDPARTY_MQTT_ENABLE       = n
THIRDPARTY_CISONENET_ENABLE  = n
THIRDPARTY_MBEDTLS_ENABLE    = n
THIRDPARTY_LITTEFS_ENABLE    = n
THIRDPARTY_RTT_ENABLE        = n
THIRDPARTY_LWIP_ENABLE       = n
THIRDPARTY_PING_ENABLE       = n
THIRDPARTY_IPERF_ENABLE      = n
THIRDPARTY_LIBSNTP_ENABLE    = n
THIRDPARTY_DHCPD_ENABLE      = n
THIRDPARTY_MP3_ENABLE        = n

DRIVER_DMA_ENABLE = n
DRIVER_USART_ENABLE = n
DRIVER_SPI_ENABLE = n
DRIVER_I2C_ENABLE = n
DRIVER_PMU_ENABLE = n
DRIVER_FLASHRT_ENABLE = n
DRIVER_TIMER_ENABLE = n
DRIVER_LPUART_ENABLE = n
DRIVER_UNILOG_ENABLE = n
DRIVER_CIPHER_ENABLE = y
DRIVER_SCT_ENABLE = n
DRIVER_SLCNT_ENABLE = n
DRIVER_MCUMODE_ENABLE = n
DRIVER_ADC_ENABLE     = n
DRIVER_PWRKEY_ENABLE = y
DRIVER_CHARGE_ENABLE = n
DRIVER_CAMERA_ENABLE = n
DRIVER_I2S_ENABLE = n
DRIVER_IPC_ENABLE = n
DRIVER_RESET_ENABLE = n
DRIVER_EXCP_ENABLE = n
DRIVER_CPADC_ENABLE = n
DRIVER_RFCALI_ENABLE = n
DRIVER_PHYHAL_ENABLE = n
DRIVER_ALARM_ENABLE = n

MIDDLEWARE_DEBUG_TRACE_ENABLE = n
MIDDLEWARE_CMS_ENABLE         = n
MIDDLEWARE_SIMBIP_ENABLE      = n
MIDDLEWARE_NVRAM_ENABLE       = y
MIDDLEWARE_RUNNV_ENABLE       = n
MIDDLEWARE_CCIO_ENABLE        = n
MIDDLEWARE_FOTA_ENABLE        = y
MIDDLEWARE_FOTAPAR_ENABLE     = y
MIDDLEWARE_FOTA_CORE2_ENABLE  = y
MIDDLEWARE_FOTA_USBURC_ENABLE = y
MIDDLEWARE_FOTA_FS_ENABLE     = n
ifeq ($(MIDDLEWARE_FOTA_CORE2_ENABLE), y)
#hls only for core2
MIDDLEWARE_FOTA_HLS_ENABLE    = y
endif
ifeq ($(MIDDLEWARE_FOTAPAR_ENABLE), y)
ifeq ($(FOTA_SIGN), fota_sign)
#signature of the fota *.par/.fpar
MIDDLEWARE_FOTA_SIGN_ENABLE   = y
endif
endif

#check fota settings
ifeq ($(MIDDLEWARE_FOTA_ENABLE)_$(MIDDLEWARE_FOTAPAR_ENABLE),y_n)
$(error "MIDDLEWARE_FOTAPAR_ENABLE" should be "y")
endif

ifeq ($(MIDDLEWARE_FOTA_USBURC_ENABLE)_$(MIDDLEWARE_FOTA_FS_ENABLE),y_y)
$(error FOTA "USBURC" and "NVM FS" can't be "y" simultaneously for lack of flash resource)
endif

ifeq ($(MIDDLEWARE_FOTA_FS_ENABLE),y)
THIRDPARTY_LITTEFS_ENABLE = y
endif

#always enable lto for bl code size reduction
LTO_ENABLE                    = true

THIRDPARTY_QUIRC_ENABLE       = n
THIRDPARTY_ZBAR_ENABLE        = n
THIRDPARTY_MINILZO_ENABLE     = n
THIRDPARTY_LZMA_ENABLE        = y
THIRDPARTY_UECC_ENABLE        = y

CONFIG_PROJ_APP_SECURITY_BOOT = n

ifeq ($(TYPE), ec718s)
#BIN_COMPRESS for post bin compress tool
#THIRDPARTY_LZMA_ENABLE LZMA feature compile en
#RAMCODE_COMPRESS_EN ramcode/RW decompress, used in code
#REL_COMPRESS_EN calibration back up compress en, used in code
CFLAGS += -DRAMCODE_COMPRESS_EN
CFLAGS += -DREL_COMPRESS_EN
THIRDPARTY_LZMA_ENABLE       = y
BIN_COMPRESS                 = y
endif

ifneq ($(findstring $(TYPE), ec716s ec716e ec718u),)
#BIN_COMPRESS for post bin compress tool
#THIRDPARTY_LZMA_ENABLE LZMA feature compile en
#RAMCODE_COMPRESS_EN ramcode/RW decompress, used in code
#REL_COMPRESS_EN calibration back up compress en, used in code
CFLAGS += -DRAMCODE_COMPRESS_EN
CFLAGS += -DREL_COMPRESS_EN
THIRDPARTY_LZMA_ENABLE       = y
BIN_COMPRESS                 = y
endif

ifneq ($(findstring $(TYPE), ec718p ec718pm ec718um ec718sm ec718hm),)
#BIN_COMPRESS for post bin compress tool
#THIRDPARTY_LZMA_ENABLE LZMA feature compile en
#RAMCODE_COMPRESS_EN ramcode/RW decompress, used in code
#REL_COMPRESS_EN calibration back up compress en, used in code
CFLAGS += -DRAMCODE_COMPRESS_EN
CFLAGS += -DREL_COMPRESS_EN
THIRDPARTY_LZMA_ENABLE       = y
BIN_COMPRESS                 = y
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


ifeq ($(OPENCPU), true)
OPENCPU_MODE_ENABLE = y
endif

ifeq ($(GCF_ENABLE), true)
GCF_FEATURE_ENABLE = y
endif

ifeq ($(MID_ENABLE), true)
MID_FEATURE_ENABLE = y
endif

ifeq ($(IOTAT_ENABLE), true)
IOTAT_FEATURE_ENABLE = y
endif

ifeq ($(GLO_ENABLE), true)
GLO_CALI_MODE_ENABLE = y
endif

ifeq ($(AUDIO_ENABLE), true)
MIDDLEWARE_VEM_ENABLE = y
MIDDLEWARE_AMR_ENABLE = y
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

ifeq ($(TYPE), ec718pm)
DRIVER_XPI_PSRAM_ENABLE     ?= y
DRIVER_PCACHE_ENABLE        ?= y
EC718XM_LD_AND_PREC = y
endif

ifeq ($(TYPE), ec718um)
DRIVER_XPI_PSRAM_ENABLE     ?= y
DRIVER_PCACHE_ENABLE        ?= y
EC718XM_LD_AND_PREC = y
endif

ifeq ($(TYPE), ec718sm)
DRIVER_XPI_PSRAM_ENABLE     ?= y
DRIVER_PCACHE_ENABLE        ?= y
EC718XM_LD_AND_PREC = y
endif

ifeq ($(TYPE), ec718hm)
DRIVER_XPI_PSRAM_ENABLE     ?= y
DRIVER_PCACHE_ENABLE        ?= y
EC718XM_LD_AND_PREC = y
endif

BL_PROJ_DIR  := PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)

CFLAGS_INC += -I ../inc                   \
              -I $(TOP)/$(BL_PROJ_DIR)/code/include \
              -I $(TOP)/$(BL_PROJ_DIR)/code/include/common \
              -I $(TOP)/$(BL_PROJ_DIR)/code/common/secure/hash/inc

obj-y += $(BL_PROJ_DIR)/code/main/ec_main.o \
         $(BL_PROJ_DIR)/code/main/bl_uart.o \
         $(BL_PROJ_DIR)/code/main/bl_print.o \
         $(BL_PROJ_DIR)/code/main/stub_os.o \
         $(BL_PROJ_DIR)/code/common/image/image.o \
         $(BL_PROJ_DIR)/code/common/secure/hash/src/sha256.o \
         $(BL_PROJ_DIR)/code/except/exception_handler.o 

ifeq ($(MIDDLEWARE_FOTA_FS_ENABLE),y)
obj-y += $(BL_PROJ_DIR)/code/main/bl_lfs.o
endif

PRECINIT_FILE_PATH = $(BL_PROJ_DIR)/$(TOOLCHAIN)
SYSTEM_FILE_PATH   = $(BL_PROJ_DIR)/code/main
STARTUP_FILE_PATH  = $(BL_PROJ_DIR)/$(TOOLCHAIN)
LINK_FILE_PATH     = $(BL_PROJ_DIR)/$(TOOLCHAIN)

ifeq ($(TYPE), ec718pm)
LD_FILE           := ec718xm/$(TARGET)_flash.ld
else ifeq ($(TYPE), ec718um)
LD_FILE           := ec718xm/$(TARGET)_flash.ld
else ifeq ($(TYPE), ec718sm)
LD_FILE           := ec718xm/$(TARGET)_flash.ld
else ifeq ($(TYPE), ec718hm)
LD_FILE           := ec718xm/$(TARGET)_flash.ld
else
LD_FILE           := $(TARGET)_flash.ld
endif

#security boot
ifeq ($(BUILD_ENV),linux)
EC_SECURE_TOOL    := $(TOP)/PLAT/tools/ecsecure
else
EC_SECURE_TOOL    := $(TOP)/PLAT/tools/ecsecure.exe
endif
ifeq ($(CONFIG_PROJ_APP_SECURITY_BOOT), y)
SYSTEM_VERIFY_KEY_FILE_SRC := $(BL_PROJ_DIR)/code/main/VerifyKeyFile.c
BL_KEYPAIR_PEM_FILE        := $(TOP)/$(BL_PROJ_DIR)/code/main/pub_key_bl.pem
SYS_KEYPAIR_PEM_FILE       := $(TOP)/$(BL_PROJ_DIR)/code/main/pub_key_sys.pem

$(TOP)/$(SYSTEM_VERIFY_KEY_FILE_SRC):
	$(EC_SECURE_TOOL) PUBKEY=$(TOP)/$(SYSTEM_VERIFY_KEY_FILE_SRC) BLPEM=$(BL_KEYPAIR_PEM_FILE) SYSPEM=$(SYS_KEYPAIR_PEM_FILE)

SYSTEM_VERIFY_KEY_FILE_OBJ := $(SYSTEM_VERIFY_KEY_FILE_SRC:.c=.o)

obj-y += $(SYSTEM_VERIFY_KEY_FILE_OBJ)
endif


include $(TOP)/PLAT/tools/scripts/Makefile.rules

#enable wdt
CFLAGS += -DWDT_FEATURE_ENABLE=1
ifeq ($(DRIVER_USBSMIMG_ENABLE),y)
CFLAGS += -DUSBC_USBMST_MGR_FEATURE_DISABLE=1
CFLAGS += -DUSB_DRV_SMALL_IMAGE=1
endif

CFLAGS += -DFEATURE_BOOTLOADER_PROJECT_ENABLE

# define these two macro for amr & 3A test
ifeq ($(MIDDLEWARE_AMR_ENABLE),y)
CFLAGS_DEFS += -DFEATURE_AMR_CP_ENABLE
endif
ifeq ($(MIDDLEWARE_VEM_ENABLE),y)
CFLAGS_DEFS += -DFEATURE_VEM_CP_ENABLE
endif

#Make all warnings into errors
CFLAGS += -Werror

ifeq ($(CONFIG_PROJ_APP_SECURITY_BOOT), y)
CFLAGS += -DCONFIG_PROJ_APP_SECURITY_BOOT
endif


ifeq ($(LTO_FEATURE_ENABLE),y)
LDFLAGS += -Wno-stringop-overflow
endif