libdriver-y :=
libdriverprivate-y :=

ifeq ($(CORE),ap)
include $(TOP)/PLAT/driver/board/$(TARGET)/Makefile.inc
endif

include $(TOP)/PLAT/driver/hal/Makefile.inc
include $(TOP)/PLAT/driver/chip/$(CHIP)/$(CORE)/Makefile.inc
include $(TOP)/PLAT/driver/chip/$(CHIP)/common/Makefile.inc


ifeq ($(TOOLCHAIN),GCC)

$(BUILDDIR)/lib/libdriver.a: $(DRIVER_OBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

ifeq ($(BUILD_USE_PREBUILD_LIB),n)
ifneq ($(DRIVER_PRIVATE_OBJS),)
  lib-y += libdriver_private.a
endif
endif
$(BUILDDIR)/lib/libdriver_private.a: $(DRIVER_PRIVATE_OBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^


ifeq ($(BUILD_USE_USB_LIB),n)
ifneq ($(USB_PRIVATE_OBJS),)
  lib-y += libusb_private.a
endif
ifneq ($(USBBL_PRIV_OBJS),)
  lib-y += libusbbl_priv.a
endif
endif
$(BUILDDIR)/lib/libusb_private.a: $(USB_PRIVATE_OBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^
	
$(BUILDDIR)/lib/libusbbl_priv.a: $(USBBL_PRIV_OBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^	
endif

ifeq ($(TOOLCHAIN),ARMCC)

ifneq ($(DRIVER_OBJS),)
lib-y += libdriver.lib
endif

$(BUILDDIR)/lib/libdriver.lib: $(DRIVER_OBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

ifeq ($(BUILD_USE_PREBUILD_LIB),n)
ifneq ($(DRIVER_PRIVATE_OBJS),)
  lib-y += libdriver_private.lib
endif
endif

$(BUILDDIR)/lib/libdriver_private.lib: $(DRIVER_PRIVATE_OBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^



ifeq ($(BUILD_USE_USB_LIB),n)
ifneq ($(USB_PRIVATE_OBJS),)
  lib-y += libusb_private.lib
endif
endif
$(BUILDDIR)/lib/libusb_private.lib: $(USB_PRIVATE_OBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^


endif