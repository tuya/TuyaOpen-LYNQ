FATFS_DIR        := $(TOP)/PLAT/subsys/storage/fatfs

CFLAGS_INC     +=  -I $(TOP)/PLAT/subsys/storage/fatfs/source
              

FATFS_SRC_DIRS += $(FATFS_DIR)/source
FATFS_EXCLUDE_FILES :=
FATFS_CSRC = $(foreach dir, $(FATFS_SRC_DIRS), $(wildcard $(dir)/*.c))
FATFS_CFILES = $(filter-out $(FATFS_EXCLUDE_FILES), $(FATFS_CSRC))
FATFS_COBJSTEMP := $(patsubst %.c, %.o, $(FATFS_CFILES))
FATFS_COBJSTEMP :=$(subst $(FATFS_DIR),PLAT/subsys/storage/fatfs,$(FATFS_COBJSTEMP))
FATFS_COBJS := $(addprefix $(BUILDDIR)/, $(FATFS_COBJSTEMP))

FATFS_PPFILES := $(patsubst %.o, %.pp, $(FATFS_COBJS))
PPFILES += $(FATFS_PPFILES)

ifneq ($(MAKECMDGOALS), unilog)
-include $(FATFS_COBJS:.o=.d)
endif
-include $(FATFS_COBJS:.o=.dd)

CFLAGS += -DFEATURE_SUBSYS_FATFS_ENABLE

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libfatfs.a

$(BUILDDIR)/lib/libfatfs.a: $(FATFS_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil


lib-y += libfatfs.lib

$(BUILDDIR)/lib/libfatfs.lib: $(FATFS_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
