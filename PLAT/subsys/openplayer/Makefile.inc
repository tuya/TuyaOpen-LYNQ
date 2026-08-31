SUBSYS_OPENPLAYER_DIR  := $(TOP)/PLAT/subsys/openplayer
CFLAGS_INC += -I $(SUBSYS_OPENPLAYER_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_OPENPLAYER_API_ENABLE

SUBSYS_OPENPLAYER_SRC_DIRS += $(SUBSYS_OPENPLAYER_DIR)/src

SUBSYS_OPENPLAYER_EXCLUDE_FILES :=

SUBSYS_OPENPLAYER_CSRC = $(foreach dir, $(SUBSYS_OPENPLAYER_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_OPENPLAYER_CFILES = $(filter-out $(SUBSYS_OPENPLAYER_EXCLUDE_FILES), $(SUBSYS_OPENPLAYER_CSRC))
SUBSYS_OPENPLAYER_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_OPENPLAYER_CFILES))
SUBSYS_OPENPLAYER_COBJSTEMP :=$(subst $(SUBSYS_OPENPLAYER_DIR),PLAT/subsys/openplayer,$(SUBSYS_OPENPLAYER_COBJSTEMP))
SUBSYS_OPENPLAYER_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_OPENPLAYER_COBJSTEMP))
SUBSYS_OPENPLAYER_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_OPENPLAYER_COBJS))
PPFILES += $(SUBSYS_OPENPLAYER_PPFILES)


ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_OPENPLAYER_COBJS:.o=.d)
endif
-include $(SUBSYS_OPENPLAYER_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_openplayer.a

$(BUILDDIR)/lib/libsubsys_openplayer.a: $(SUBSYS_OPENPLAYER_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_openplayer.lib

$(BUILDDIR)/lib/libsubsys_openplayer.lib: $(SUBSYS_OPENPLAYER_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
