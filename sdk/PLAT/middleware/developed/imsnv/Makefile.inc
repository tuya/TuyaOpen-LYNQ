ifeq ($(BUILD_USE_PREBUILD_LIB), n)
IMSNV_DIR        := $(TOP)/PROTOCOL/SRC/ims/base/nv/src
CFLAGS_INC += -I $(TOP)/PROTOCOL/SRC/ims/base/nv/inc
else
IMSNV_DIR        := $(TOP)/PLAT/middleware/developed/imsnv/src
CFLAGS_INC += -I $(TOP)/PLAT/middleware/developed/imsnv/inc
endif
              
CFLAGS += -I $(IMSNV_DIR)

IMSNV_SRC_DIRS += $(IMSNV_DIR)
IMSNV_EXCLUDE_FILES :=
IMSNV_CSRC = $(foreach dir, $(IMSNV_SRC_DIRS), $(wildcard $(dir)/*.c))
IMSNV_CFILES = $(filter-out $(IMSNV_EXCLUDE_FILES), $(IMSNV_CSRC))
IMSNV_COBJSTEMP := $(patsubst %.c, %.o, $(IMSNV_CFILES))
ifeq ($(BUILD_USE_PREBUILD_LIB), n)
IMSNV_COBJSTEMP :=$(subst $(IMSNV_DIR),PROTOCOL/SRC/ims/base/nv/src,$(IMSNV_COBJSTEMP))
else
IMSNV_COBJSTEMP :=$(subst $(IMSNV_DIR),PLAT/middleware/developed/imsnv/src,$(IMSNV_COBJSTEMP))
endif
IMSNV_COBJS := $(addprefix $(BUILDDIR)/, $(IMSNV_COBJSTEMP))
IMSNV_PPFILES := $(patsubst %.o, %.pp, $(IMSNV_COBJS))
PPFILES += $(IMSNV_PPFILES)

ifneq ($(MAKECMDGOALS), unilog)
-include $(IMSNV_COBJS:.o=.d)
endif
-include $(IMSNV_COBJS:.o=.dd)



ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libimsnv.a

$(BUILDDIR)/lib/libimsnv.a: $(IMSNV_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil


lib-y += libimsnv.lib

$(BUILDDIR)/lib/libimsnv.lib: $(IMSNV_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
