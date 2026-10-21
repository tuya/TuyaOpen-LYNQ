ifeq ($(BUILD_USE_PREBUILD_LIB),n)
LWIP_HOST_DIRS := $(TOP)/PROTOCOL/SRC/tcpip/lwip/src
else
LWIP_HOST_DIRS := $(TOP)/PLAT/middleware/thirdparty/lwip/src
endif

CFLAGS_INC += -I $(LWIP_HOST_DIRS)/include                 \
              -I $(LWIP_HOST_DIRS)/include/posix           \
              -I $(TOP)/PLAT/os/osa/inc                   \
              -I $(TOP)/PLAT/os/freertos/inc                \
              -I $(TOP)/PLAT/os/freertos/CMSIS/inc	        \
              -I $(TOP)/PLAT/driver/chip/ec618/inc          \
              -I $(TOP)/PLAT/middleware/eigencomm/debug/inc



LWIP_SRC_DIRS += $(LWIP_HOST_DIRS)/core      \
     $(LWIP_HOST_DIRS)/core/ipv4              \
     $(LWIP_HOST_DIRS)/core/ipv6              \
     $(LWIP_HOST_DIRS)/api                    \
     $(LWIP_HOST_DIRS)/netif

ifeq ($(THIRDPARTY_PPP_ENABLE),y)
LWIP_SRC_DIRS += $(LWIP_HOST_DIRS)/netif/ppp\
                 $(LWIP_HOST_DIRS)/netif/ppp/polarssl

CFLAGS        += -DFEATURE_PPP_ENABLE
endif

LWIP_EXCLUDE_FILES :=  $(LWIP_HOST_DIRS)/netif/ethernetif.c \
                       $(LWIP_HOST_DIRS)/netif/lowpan6.c    \
                       $(LWIP_HOST_DIRS)/core/ipv4/dhcp.c    \
                       $(LWIP_HOST_DIRS)/core/ipv6/dhcp6.c    \
                       $(LWIP_HOST_DIRS)/netif/slipif.c


LWIPCONFIG_CFLAGS ?= -DLWIP_CONFIG_FILE=\"lwip_config_cat.h\"
CFLAGS += $(LWIPCONFIG_CFLAGS)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc


$(BUILDDIR)/lib/liblwip.a: $(LWIP_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += liblwip.lib

$(BUILDDIR)/lib/liblwip.lib: $(LWIP_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
