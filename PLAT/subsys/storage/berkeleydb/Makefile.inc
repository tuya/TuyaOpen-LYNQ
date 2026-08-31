BERKDBDIR        := $(TOP)/PLAT/subsys/storage/berkeleydb

CFLAGS_INC     +=  -I $(TOP)/PLAT/subsys/storage/berkeleydb/include \
                    -I $(TOP)/PLAT/subsys/storage/berkeleydb/PORT/include              

BERKDBSRC_DIRS += $(BERKDBDIR)/btree \
				$(BERKDBDIR)/mpool \
                $(BERKDBDIR)/db


BERKDBEXCLUDE_FILES :=
BERKDBCSRC = $(foreach dir, $(BERKDBSRC_DIRS), $(wildcard $(dir)/*.c))
BERKDBCFILES = $(filter-out $(BERKDBEXCLUDE_FILES), $(BERKDBCSRC))
BERKDBCOBJSTEMP := $(patsubst %.c, %.o, $(BERKDBCFILES))
BERKDBCOBJSTEMP :=$(subst $(BERKDBDIR),PLAT/subsys/storage/berkeleydb,$(BERKDBCOBJSTEMP))
BERKDBCOBJS := $(addprefix $(BUILDDIR)/, $(BERKDBCOBJSTEMP))

BERKDBPPFILES := $(patsubst %.o, %.pp, $(BERKDBCOBJS))
PPFILES += $(BERKDBPPFILES)

ifneq ($(MAKECMDGOALS), unilog)
-include $(BERKDBCOBJS:.o=.d)
endif
-include $(BERKDBCOBJS:.o=.dd)

CFLAGS += -DFEATURE_SUBSYS_BERKDBENABLE

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libberkdb.a

$(BUILDDIR)/lib/libberkdb.a: $(BERKDBCOBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil


lib-y += libberkdb.lib

$(BUILDDIR)/lib/libberkdb.lib: $(BERKDBCOBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
