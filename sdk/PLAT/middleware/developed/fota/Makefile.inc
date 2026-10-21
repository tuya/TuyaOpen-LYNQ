FOTA_LOCAL_REL_DIR  := PLAT/middleware/developed/fota
FOTA_LOCAL_ABS_DIR  := $(TOP)/$(FOTA_LOCAL_REL_DIR)

CFLAGS_INC          += -I$(FOTA_LOCAL_ABS_DIR)/upm/inc\
                       -I$(FOTA_LOCAL_ABS_DIR)/delta/inc

LDFLAGS             +=

FOTA_LOCAL_CORE2_MK := $(FOTA_LOCAL_ABS_DIR)/delta/Makefile.inc
ifeq ($(FOTA_LOCAL_CORE2_MK), $(wildcard $(FOTA_LOCAL_CORE2_MK)))
FOTA_CORE2_MK_FLAGS  = y
endif

ifeq ($(MIDDLEWARE_FOTA_ENABLE), y)
CFLAGS              += -DFEATURE_FOTA_ENABLE -DMIDDLEWARE_FOTA_ENABLE

ifeq ($(MIDDLEWARE_FOTA_USBURC_ENABLE), y)
CFLAGS += -DFEATURE_FOTA_USBURC_ENABLE
endif

FOTA_LOCAL_SRC_DIR  := $(FOTA_LOCAL_ABS_DIR)/upm/src

ifeq ($(MIDDLEWARE_FOTA_CORE2_ENABLE), y)
CFLAGS += -DFEATURE_FOTA_CORE2_ENABLE -DFOTA_PRESET_RAM_ENABLE=1

ifeq ($(MIDDLEWARE_FOTA_HLS_ENABLE), y)
CFLAGS_DEFS += -DFEATURE_FOTA_HLS_ENABLE
endif

ifeq ($(FOTA_CORE2_MK_FLAGS), y)
include $(FOTA_LOCAL_CORE2_MK)
endif
endif

FOTA_LOCAL_EXCLUDE_SRCS   :=
endif


FOTA_LOCAL_SRCS     := $(foreach srcdir, $(FOTA_LOCAL_SRC_DIR), $(wildcard $(srcdir)/*.c))
FOTA_LOCAL_SRCS     := $(filter-out $(FOTA_LOCAL_EXCLUDE_SRCS), $(FOTA_LOCAL_SRCS))
FOTA_LOCAL_OBJSTEMP := $(patsubst %.c, %.o, $(FOTA_LOCAL_SRCS))
FOTA_LOCAL_OBJS     := $(subst $(FOTA_LOCAL_ABS_DIR),$(FOTA_LOCAL_REL_DIR),$(FOTA_LOCAL_OBJSTEMP))
FOTA_LOCAL_OBJS     := $(addprefix $(BUILDDIR)/, $(FOTA_LOCAL_OBJS))
FOTA_LOCAL_PPFS     := $(patsubst %.o, %.pp, $(FOTA_LOCAL_OBJS))
LIBPPFILES          += $(FOTA_PRIVATE_LOCAL_PPFS)

#CFLAGS              += -D__MICROLIB
#LDFLAGS             += --library_type=microlib


ifneq ($(MAKECMDGOALS), unilog)
-include $(FOTA_LOCAL_OBJS:.o=.d)
endif
-include $(FOTA_LOCAL_OBJS:.o=.dd)


ifeq ($(TOOLCHAIN),GCC)
ifeq ($(BUILD_USE_PREBUILD_LIB),n)
ifeq ($(MIDDLEWARE_FOTA_CORE2_ENABLE), y)
ifneq ($(FOTA_CORE2_MK_FLAGS), y)
PREBUILDLIBS += $(FOTA_LOCAL_ABS_DIR)/delta/libs/libdeltapatch2.a
endif
else
PREBUILDLIBS += $(FOTA_LOCAL_ABS_DIR)/delta/libs/libdeltapatch.a
endif
endif

ifneq ($(FOTA_LOCAL_OBJS),)
ifeq ($(BUILD_USE_PREBUILD_LIB),n)
	lib-y += libfota.a
endif
endif

$(BUILDDIR)/lib/libfota.a: $(FOTA_LOCAL_OBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif


ifeq ($(TOOLCHAIN),ARMCC)
ifeq ($(BUILD_USE_PREBUILD_LIB),n)
PREBUILDLIBS += $(FOTA_LOCAL_ABS_DIR)/delta/libdeltapatch.lib
endif

ifneq ($(FOTA_LOCAL_OBJS),)
ifeq ($(BUILD_USE_PREBUILD_LIB),n)
	lib-y += libfota.lib
endif
endif

$(BUILDDIR)/lib/libfota.lib: $(FOTA_LOCAL_OBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif


