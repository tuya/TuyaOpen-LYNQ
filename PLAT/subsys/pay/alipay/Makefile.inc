ALIPAY_DIR      := $(TOP)/PLAT/subsys/pay/alipay
CFLAGS += -DARM_MATH_CM3

#system include
CFLAGS_INC    +=  -I $(TOP)/PLAT/os/freertos/inc \
                  -I $(TOP)/PLAT/os/freertos/CMSIS/inc

#share
CFLAGS_INC	  += -I $(ALIPAY_DIR)/share/common/inc

#app module
CFLAGS_INC    +=  -I $(ALIPAY_DIR)/app/aid_code/inc  \
				  -I $(ALIPAY_DIR)/app/antchain/inc  \
				  -I $(ALIPAY_DIR)/app/bind_net/inc  \
				  -I $(ALIPAY_DIR)/app/pay_code/inc  \
				  -I $(ALIPAY_DIR)/app/pay_result/inc \
				  -I $(ALIPAY_DIR)/app/setting/inc

#hal				  
CFLAGS_INC    +=  -I $(ALIPAY_DIR)/hal/os/inc \
				  -I $(ALIPAY_DIR)/hal/device_info/inc \
				  -I $(ALIPAY_DIR)/hal/se_v2/CIU98_D/inc \
				  -I $(ALIPAY_DIR)/hal/network/inc


#share lib
PREBUILDLIBS += $(ALIPAY_DIR)/share/common/lib/libshare_common_m3_gcc10.a

#app lib 
PREBUILDLIBS += $(ALIPAY_DIR)/app/aid_code/lib/libapp_aid_code_m3_gcc10.a \
				$(ALIPAY_DIR)/app/antchain/lib/libapp_antchain_m3_gcc10.a \
				$(ALIPAY_DIR)/app/bind_net/lib/libapp_bind_net_m3_gcc10.a \
				$(ALIPAY_DIR)/app/pay_code/lib/libapp_pay_code_m3_gcc10.a \
				$(ALIPAY_DIR)/app/pay_result/lib/libapp_pay_result_m3_gcc10.a \
				$(ALIPAY_DIR)/app/setting/lib/libapp_setting_m3_gcc10.a
				
#app depend 3rdparty lib 
PREBUILDLIBS += $(ALIPAY_DIR)/3rdparty/uEcc/lib/lib3rdparty_uEcc_m3_gcc10.a \
			    $(ALIPAY_DIR)/3rdparty/cJSON/lib/lib3rdparty_cJSON_m3_gcc10.a \
				$(ALIPAY_DIR)/3rdparty/mbedtls/lib/lib3rdparty_mbedtls_m3_gcc10.a \
				$(ALIPAY_DIR)/3rdparty/protobuf-c/lib/lib3rdparty_protobuf-c_m3_gcc10.a \
				$(ALIPAY_DIR)/3rdparty/sm3/lib/lib3rdparty_sm3_m3_gcc10.a

#app depend fwk lib 			
PREBUILDLIBS += $(ALIPAY_DIR)/fwk/common/lib/libfwk_common_m3_gcc10.a \
				$(ALIPAY_DIR)/fwk/bind_status/lib/libfwk_bind_status_m3_gcc10.a \
				$(ALIPAY_DIR)/fwk/crypt/lib/libfwk_crypt_m3_gcc10.a \
				$(ALIPAY_DIR)/fwk/secure_mode_se_v2/lib/libfwk_secure_mode_se_v2_m3_gcc10.a \
				$(ALIPAY_DIR)/fwk/network/lib/libfwk_network_m3_gcc10.a
				
				
				
ALIPAY_SRC_DIRS += $(ALIPAY_DIR)/app/antchain/src \
				   $(ALIPAY_DIR)/hal/device_info/src \
				   $(ALIPAY_DIR)/hal/network/src \
				   $(ALIPAY_DIR)/hal/os/src \
				    $(ALIPAY_DIR)/hal/se_v2 \
				   $(ALIPAY_DIR)/hal/se_v2/CIU98_D \
				   $(ALIPAY_DIR)/hal/se_v2/CIU98_D/src \
				   $(ALIPAY_DIR)/hal/se_v2/CIU98_D/port

		

ALIPAY_EXCLUDE_FILES :=

ALIPAY_CSRC = $(foreach dir, $(ALIPAY_SRC_DIRS), $(wildcard $(dir)/*.c))
ALIPAY_CFILES = $(filter-out $(ALIPAY_EXCLUDE_FILES), $(ALIPAY_CSRC))
ALIPAY_COBJSTEMP := $(patsubst %.c, %.o, $(ALIPAY_CFILES))
ALIPAY_COBJSTEMP :=$(subst $(ALIPAY_DIR),PLAT/subsys/pay/alipay,$(ALIPAY_COBJSTEMP))
ALIPAY_COBJS := $(addprefix $(BUILDDIR)/, $(ALIPAY_COBJSTEMP))
ALIPAY_PPFILES := $(patsubst %.o, %.pp, $(ALIPAY_COBJS))
PPFILES += $(ALIPAY_PPFILES)

ifneq ($(MAKECMDGOALS), unilog)
-include $(ALIPAY_COBJS:.o=.d)
endif
-include $(ALIPAY_COBJS:.o=.dd)


ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libalipay.a

$(BUILDDIR)/lib/libalipay.a: $(ALIPAY_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libalipay.lib

$(BUILDDIR)/lib/libalipay.lib: $(ALIPAY_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
