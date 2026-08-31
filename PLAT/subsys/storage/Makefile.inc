SUBSYS_STORAGE_DIR  := $(TOP)/PLAT/subsys/storage
CFLAGS_INC += -I $(SUBSYS_STORAGE_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_STORAGE_ENABLE \
          -DLFSEX_WITHOUT_THREAD          \
          -DSPI_MUTEX_ENABLE

SUBSYS_STORAGE_PARTITION_ENABLE = y

ifeq ($(SUBSYS_STORAGE_PARTITION_ENABLE),y)
CFLAGS += -DFEATURE_SUBSYS_STORAGE_PARTITION_ENABLE
endif

SUBSYS_STORAGE_SRC_DIRS += $(SUBSYS_STORAGE_DIR)/src

SUBSYS_STORAGE_EXCLUDE_FILES :=

SUBSYS_STORAGE_CSRC = $(foreach dir, $(SUBSYS_STORAGE_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_STORAGE_CFILES = $(filter-out $(SUBSYS_STORAGE_EXCLUDE_FILES), $(SUBSYS_STORAGE_CSRC))
SUBSYS_STORAGE_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_STORAGE_CFILES))
SUBSYS_STORAGE_COBJSTEMP :=$(subst $(SUBSYS_STORAGE_DIR),PLAT/subsys/storage,$(SUBSYS_STORAGE_COBJSTEMP))
SUBSYS_STORAGE_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_STORAGE_COBJSTEMP))
SUBSYS_STORAGE_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_STORAGE_COBJS))
PPFILES += $(SUBSYS_STORAGE_PPFILES)

ifeq ($(SUBSYS_LFSEX_ENABLE),y)
include $(SUBSYS_STORAGE_DIR)/lfsex/Makefile.inc
endif

ifeq ($(SUBSYS_FLASHEX_ENABLE),y)
include $(SUBSYS_STORAGE_DIR)/flashex/Makefile.inc
endif

ifeq ($(SUBSYS_FLASHDB_ENABLE),y)
include $(SUBSYS_STORAGE_DIR)/flashdb/Makefile.inc
endif

ifeq ($(SUBSYS_FATFS_ENABLE),y)
include $(SUBSYS_STORAGE_DIR)/fatfs/Makefile.inc
endif

ifeq ($(SUBSYS_SDCARD_ENABLE),y)
include $(SUBSYS_STORAGE_DIR)/sdcard/Makefile.inc
endif

ifeq ($(SUBSYS_BERKELEYDB_ENABLE),y)
include $(SUBSYS_STORAGE_DIR)/berkeleydb/Makefile.inc
endif

ifeq ($(SUBSYS_JSONDB_ENABLE),y)
include $(SUBSYS_STORAGE_DIR)/jsonDb/Makefile.inc
ifeq ($(SUBSYS_JSONDB_TEST_ENABLE),y)
CFLAGS += -DJSON_DB_TEST
endif
endif

ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_STORAGE_COBJS:.o=.d)
endif
-include $(SUBSYS_STORAGE_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_storage.a

$(BUILDDIR)/lib/libsubsys_storage.a: $(SUBSYS_STORAGE_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_storage.lib

$(BUILDDIR)/lib/libsubsys_storage.lib: $(SUBSYS_STORAGE_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
