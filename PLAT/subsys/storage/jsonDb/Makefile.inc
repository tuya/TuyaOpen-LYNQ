SUBSYS_JSONDB_ENABLE := $(TOP)/PLAT/subsys/storage/jsonDb

CFLAGS_INC += -I $(SUBSYS_JSONDB_ENABLE)/inc

CFLAGS += -DFEATURE_SUBSYS_JSONDB_ENABLE

SUBSYS_JSONDB_SRC_DIRS := $(SUBSYS_JSONDB_ENABLE)/src

SUBSYS_JSONDB_EXCLUDE_FILES :=

SUBSYS_JSONDB_CSRC = $(foreach dir, $(SUBSYS_JSONDB_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_JSONDB_CFILES = $(filter-out $(SUBSYS_JSONDB_EXCLUDE_FILES), $(SUBSYS_JSONDB_CSRC))
SUBSYS_JSONDB_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_JSONDB_CFILES))
SUBSYS_JSONDB_COBJSTEMP :=$(subst $(SUBSYS_JSONDB_ENABLE),PLAT/subsys/storage/jsonDb,$(SUBSYS_JSONDB_COBJSTEMP))
SUBSYS_JSONDB_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_JSONDB_COBJSTEMP))
SUBSYS_JSONDB_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_JSONDB_COBJS))
PPFILES += $(SUBSYS_JSONDB_PPFILES)

ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_JSONDB_COBJS:.o=.d)
endif
-include $(SUBSYS_JSONDB_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_jsonDb.a

$(BUILDDIR)/lib/libsubsys_jsonDb.a: $(SUBSYS_JSONDB_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_jsonDb.lib

$(BUILDDIR)/lib/libsubsys_jsonDb.lib: $(SUBSYS_JSONDB_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
