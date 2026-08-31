SUBSYS_LAUNCHER_DIR  := $(TOP)/PLAT/subsys/apphub/launcher

CFLAGS_INC += -I $(SUBSYS_LAUNCHER_DIR)				\
			-I $(SUBSYS_LAUNCHER_DIR)/images 		\
			-I $(SUBSYS_LAUNCHER_DIR)/fonts 		\
			-I $(SUBSYS_LAUNCHER_DIR)/components 	\
			-I $(SUBSYS_LAUNCHER_DIR)/screens		

SUBSYS_LAUNCHER_SRC_DIRS := $(SUBSYS_LAUNCHER_DIR)	\
			$(SUBSYS_LAUNCHER_DIR)/images			\
			$(SUBSYS_LAUNCHER_DIR)/fonts			\
			$(SUBSYS_LAUNCHER_DIR)/components		\
			$(SUBSYS_LAUNCHER_DIR)/screens			

CFLAGS += -DFEATURE_SUBSYS_LAUNCHER_ENABLE

SUBSYS_LAUNCHER_EXCLUDE_FILES :=

SUBSYS_LAUNCHER_CSRC = $(foreach dir, $(SUBSYS_LAUNCHER_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_LAUNCHER_CFILES = $(filter-out $(SUBSYS_LAUNCHER_EXCLUDE_FILES), $(SUBSYS_LAUNCHER_CSRC))
SUBSYS_LAUNCHER_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_LAUNCHER_CFILES))
SUBSYS_LAUNCHER_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_LAUNCHER_COBJSTEMP))
SUBSYS_LAUNCHER_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_LAUNCHER_COBJS))
PPFILES += $(SUBSYS_LAUNCHER_PPFILES)

ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_LAUNCHER_COBJS:.o=.d)
endif
-include $(SUBSYS_LAUNCHER_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_launcher.a

$(BUILDDIR)/lib/libsubsys_launcher.a: $(SUBSYS_LAUNCHER_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_launcher.lib

$(BUILDDIR)/lib/libsubsys_launcher.lib: $(SUBSYS_LAUNCHER_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
