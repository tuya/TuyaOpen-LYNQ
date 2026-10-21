LFS_DIR        := $(TOP)/PLAT/middleware/thirdparty/littlefs

CFLAGS_INC     +=  -I $(LFS_DIR) \
                   -I $(LFS_DIR)/port \


LFS_SRC_DIRS += $(LFS_DIR) \
                $(LFS_DIR)/port


CFLAGS += -DLFS_NAME_MAX=63 -DLFS_DEBUG_TRACE

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc


$(BUILDDIR)/lib/liblfs.a: $(LFS_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil


lib-y += liblfs.lib

$(BUILDDIR)/lib/liblfs.lib: $(LFS_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
