CIS_VERSION     := NB_SDK2.3.0
CIS_DIR         := $(TOP)/PLAT/middleware/thirdparty/cis_onenet/$(CIS_VERSION)


CFLAGS          += -DCIS_EMBED -DHAVE_STRUCT_TIMESPEC -DFEATURE_CISONENET_ENABLE
CFLAGS_INC       +=  -I $(CIS_DIR)/src/ciscore \
                     -I $(CIS_DIR)/src/ciscore/std_object \
                     -I $(CIS_DIR)/src/ciscore/dm_utils   \
                     -I $(CIS_DIR)/src/adapter/freeRTOS

             

CIS_SRC_DIRS += $(CIS_DIR)/src/ciscore \
                 $(CIS_DIR)/src/ciscore/std_object \
                 $(CIS_DIR)/src/ciscore/dm_utils \
                 $(CIS_DIR)/src/adapter/freeRTOS


ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libcis_onenet.a

$(BUILDDIR)/lib/libcis_onenet.a: $(CIS_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil


$(BUILDDIR)/lib/libcis_onenet.lib: $(CIS_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
