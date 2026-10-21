CTW_DIR        := $(TOP)/PLAT/middleware/thirdparty/ctwing

CFLAGS_INC     +=  -I $(TOP)/PLAT/middleware/thirdparty/ctwing
              

ctwing_CSRC += $(CTW_DIR)/ctw_task.c \
               $(CTW_DIR)/ctw_http.c \
               $(CTW_DIR)/ctw_tcp.c \
               $(CTW_DIR)/ctw_mqtt.c
			   
CFLAGS += -DFEATURE_CTWING_CERTI_ENABLE 

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc
CFLAGS_INC += -I $(GCCLIB_PATH)/arm-none-eabi/include


$(BUILDDIR)/lib/libctwing.a: $(ctwing_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil


lib-y += libctwing.lib

$(BUILDDIR)/lib/libctwing.lib: $(ctwing_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
