FLASHDB_DIR        := $(TOP)/PLAT/subsys/storage/flashdb

CFLAGS_INC     +=  -I $(TOP)/PLAT/subsys/storage/flashdb/port/fal/inc \
                   -I $(TOP)/PLAT/subsys/storage/flashdb/inc

FLASHDB_SRC_DIRS += $(FLASHDB_DIR)/src \
					$(FLASHDB_DIR)/port/fal/src \
					$(FLASHDB_DIR)/demos/ec718  \
					$(FLASHDB_DIR)/samples

FLASHDB_EXCLUDE_FILES :=
FLASHDB_CSRC = $(foreach dir, $(FLASHDB_SRC_DIRS), $(wildcard $(dir)/*.c))
FLASHDB_CFILES = $(filter-out $(FLASHDB_EXCLUDE_FILES), $(FLASHDB_CSRC))
FLASHDB_COBJSTEMP := $(patsubst %.c, %.o, $(FLASHDB_CFILES))
FLASHDB_COBJSTEMP :=$(subst $(FLASHDB_DIR),PLAT/subsys/storage/flashdb,$(FLASHDB_COBJSTEMP))
FLASHDB_COBJS := $(addprefix $(BUILDDIR)/, $(FLASHDB_COBJSTEMP))

FLASHDB_PPFILES := $(patsubst %.o, %.pp, $(FLASHDB_COBJS))
PPFILES += $(FLASHDB_PPFILES)

ifneq ($(MAKECMDGOALS), unilog)
-include $(FLASHDB_COBJS:.o=.d)
endif
-include $(FLASHDB_COBJS:.o=.dd)

CFLAGS += -DFEATURE_SUBSYS_FLASHDB_ENABLE

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libflashdb.a

$(BUILDDIR)/lib/libflashdb.a: $(FLASHDB_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil


lib-y += libflashdb.lib

$(BUILDDIR)/lib/libflashdb.lib: $(FLASHDB_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
