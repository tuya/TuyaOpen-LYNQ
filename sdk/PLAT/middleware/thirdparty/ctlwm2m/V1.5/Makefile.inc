
############################################################################
# SDK directory path
############################################################################

CTIOT_NB_SDK_DIR  := $(TOP)/PLAT/middleware/thirdparty/ctlwm2m/V1.5/src
LWM2M_CORE_DIR    := $(TOP)/PLAT/middleware/thirdparty/wakaama_core

############################################################################
# define micro
############################################################################
include $(TOP)/PLAT/middleware/thirdparty/ctlwm2m/V1.5/config_ctiot.mk

CFLAGS += -DPLATFORM_MCU_ECOM -DFEATURE_CTWINGV1_5_ENABLE -DLWM2M_BOOTSTRAP

ifeq ($(LWM2M_CLIENT_MODE),yes)
	CFLAGS += -DLWM2M_CLIENT_MODE
endif

ifeq ($(LWM2M_BIG_ENDIAN),yes)
	CFLAGS += -DLWM2M_BIG_ENDIAN
else
	CFLAGS += -DLWM2M_LITTLE_ENDIAN
endif

ifeq ($(WITH_MBEDTLS),yes)
	CFLAGS += -DWITH_MBEDTLS
	CFLAGS += -DFEATURE_CT_DTLS_ENABLE
endif

ifeq ($(USE_SM9),yes)
	CFLAGS += -DWITH_SM9
endif

ifeq ($(USE_FOTA),yes)
	CFLAGS += -DWITH_FOTA -DCTIOT_ABUP_FOTA_ENABLE
endif

#PREBUILDLIBS += $(TOP)/PLAT/prebuild/PLAT/lib/keil/libcortex_m3_armcc.lib
############################################################################
# include path
############################################################################
CFLAGS_INC += -I $(LWM2M_CORE_DIR)                                  \
              -I $(CTIOT_NB_SDK_DIR)/ctiot_client/common            \
			        -I $(CTIOT_NB_SDK_DIR)/ctiot_client/object19		    \
			        -I $(CTIOT_NB_SDK_DIR)/ctiot_client       		    \
			        -I $(CTIOT_NB_SDK_DIR)                        	  

############################################################################
# src path
############################################################################
CTLWM2M_SRC_DIRS += $(CTIOT_NB_SDK_DIR)/ctiot_client                    \
		    $(CTIOT_NB_SDK_DIR)/ctiot_client/common             \
                    $(CTIOT_NB_SDK_DIR)/ctiot_client/object19           \
		    $(CTIOT_NB_SDK_DIR)/ctiot_client/objects	        \
		    $(CTIOT_NB_SDK_DIR)/port

ifeq ($(USE_FOTA),yes)
	CFLAGS_INC +=-I $(CTIOT_NB_SDK_DIR)/ctiot_client/fota
	CTLWM2M_SRC_DIRS += $(CTIOT_NB_SDK_DIR)/ctiot_client/fota 
endif

CTLWM2M_EXCLUDE_FILES := 


ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc
CFLAGS_INC += -I $(GCCLIB_PATH)/arm-none-eabi/include

lib-y += libctlwm2m.a

$(BUILDDIR)/lib/libctlwm2m.a: $(CTLWM2M_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)


$(BUILDDIR)/lib/libctlwm2m.lib: $(CTLWM2M_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
