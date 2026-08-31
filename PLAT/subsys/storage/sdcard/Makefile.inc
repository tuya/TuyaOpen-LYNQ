SDCARD_DIR  := $(TOP)/PLAT/subsys/storage/sdcard
CFLAGS_INC += -I $(SDCARD_DIR)/inc
              
CFLAGS += -DFEATURE_SUBSYS_SDCARD_ENABLE

SDCARD_SRC_DIRS += $(SDCARD_DIR)/src

SDCARD_EXCLUDE_FILES :=

SDCARD_CSRC = $(foreach dir, $(SDCARD_SRC_DIRS), $(wildcard $(dir)/*.c))
SDCARD_CFILES = $(filter-out $(SDCARD_EXCLUDE_FILES), $(SDCARD_CSRC))
SDCARD_COBJSTEMP := $(patsubst %.c, %.o, $(SDCARD_CFILES))
SDCARD_COBJSTEMP :=$(subst $(SDCARD_DIR),PLAT/subsys/storage/sdcard,$(SDCARD_COBJSTEMP))
SDCARD_COBJS := $(addprefix $(BUILDDIR)/, $(SDCARD_COBJSTEMP))
SDCARD_PPFILES := $(patsubst %.o, %.pp, $(SDCARD_COBJS))
PPFILES += $(SDCARD_PPFILES)


ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SDCARD_COBJS:.o=.d)
endif
-include $(SDCARD_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsdcard.a

$(BUILDDIR)/lib/libsdcard.a: $(SDCARD_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsdcard.lib

$(BUILDDIR)/lib/libsdcard.lib: $(SDCARD_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
