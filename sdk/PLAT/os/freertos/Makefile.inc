
FREERTOS_DIR := $(TOP)/PLAT/os/freertos


CFLAGS_INC += -I$(TOP)/PLAT/os/freertos/inc          \
              -I$(TOP)/PLAT/os/freertos/CMSIS/common/inc   \
              -I$(TOP)/PLAT/os/freertos/CMSIS/$(CORE)/inc   \
              -I$(TOP)/PLAT/os/freertos/portable/mem/tlsf

ifeq ($(BUILD_OS),y)
FREERTOS_SRC_DIRS += $(FREERTOS_DIR)/src            \
                     $(FREERTOS_DIR)/CMSIS/common/src \
                     $(FREERTOS_DIR)/CMSIS/$(CORE)/src

CFLAGS_DEFS += -DFEATURE_OS_ENABLE 
CFLAGS_DEFS += -DFEATURE_FREERTOS_ENABLE 
                    
ifeq ($(TOOLCHAIN),ARMCC)
FREERTOS_SRC_DIRS += $(FREERTOS_DIR)/portable/keil  \
                     $(FREERTOS_DIR)/portable/mem/tlsf
endif

ifeq ($(TOOLCHAIN),GCC)
FREERTOS_SRC_DIRS += $(FREERTOS_DIR)/portable/gcc  \
                     $(FREERTOS_DIR)/portable/mem/tlsf
CFLAGS += -DconfigUSE_NEWLIB_REENTRANT=1
LDFLAGS += -Wl,--wrap=_malloc_r -Wl,--wrap=_free_r -Wl,--wrap=_realloc_r
endif

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc


$(BUILDDIR)/lib/libfreertos.a: $(FREERTOS_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libfreertos.lib

$(BUILDDIR)/lib/libfreertos.lib: $(FREERTOS_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
endif