SUBSYS_ZIP_DIR  := $(TOP)/PLAT/subsys/zip

CFLAGS_INC += -I $(SUBSYS_ZIP_DIR)/zip

CFLAGS += -DFEATURE_SUBSYS_ZIP_ENABLE

SUBSYS_ZIP_SRC_DIRS += $(SUBSYS_ZIP_DIR)/zip

SUBSYS_ZIP_EXCLUDE_FILES :=

SUBSYS_ZIP_CSRC = $(foreach dir, $(SUBSYS_ZIP_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_ZIP_CFILES = $(filter-out $(SUBSYS_ZIP_EXCLUDE_FILES), $(SUBSYS_ZIP_CSRC))
SUBSYS_ZIP_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_ZIP_CFILES))
SUBSYS_ZIP_COBJSTEMP :=$(subst $(SUBSYS_ZIP_DIR),PLAT/subsys/zip,$(SUBSYS_ZIP_COBJSTEMP))
SUBSYS_ZIP_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_ZIP_COBJSTEMP))
SUBSYS_ZIP_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_ZIP_COBJS))
PPFILES += $(SUBSYS_ZIP_PPFILES)

ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_ZIP_COBJS:.o=.d)
endif
-include $(SUBSYS_ZIP_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_zip.a

$(BUILDDIR)/lib/libsubsys_zip.a: $(SUBSYS_ZIP_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_zip.lib

$(BUILDDIR)/lib/libsubsys_zip.lib: $(SUBSYS_ZIP_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
