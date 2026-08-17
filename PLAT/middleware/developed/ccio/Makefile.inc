CCIO_LOCAL_REL_DIR  := PLAT/middleware/developed/ccio
CCIO_LOCAL_ABS_DIR  := $(TOP)/$(CCIO_LOCAL_REL_DIR)


CFLAGS_INC          += -I$(CCIO_LOCAL_ABS_DIR)/pub\
                       -I$(CCIO_LOCAL_ABS_DIR)/device/inc\
                       -I$(CCIO_LOCAL_ABS_DIR)/device/inc/dal\
                       -I$(CCIO_LOCAL_ABS_DIR)/device/inc/dal/scom\
                       -I$(CCIO_LOCAL_ABS_DIR)/entity/inc\
                       -I$(CCIO_LOCAL_ABS_DIR)/core/inc\
                       -I$(CCIO_LOCAL_ABS_DIR)/service/inc\
                       -I$(CCIO_LOCAL_ABS_DIR)/custom/inc

LDFLAGS             +=


ifeq ($(MIDDLEWARE_CCIO_ENABLE), y)

CFLAGS              += -DFEATURE_CCIO_ENABLE

CCIO_LOCAL_SRC_DIR  := $(CCIO_LOCAL_ABS_DIR)/core/src\
                       $(CCIO_LOCAL_ABS_DIR)/entity/src \
                       $(CCIO_LOCAL_ABS_DIR)/device/src \
                       $(CCIO_LOCAL_ABS_DIR)/device/src/dal \
                       $(CCIO_LOCAL_ABS_DIR)/device/src/dal/scom \
                       $(CCIO_LOCAL_ABS_DIR)/service/src \
                       $(CCIO_LOCAL_ABS_DIR)/custom/src 

CCIO_EXCLUDE_SRCS   :=
endif



ifeq ($(TOOLCHAIN),GCC)

ifneq ($(CCIO_LOCAL_OBJS),)
	lib-y += libccio.a
endif


endif

ifeq ($(TOOLCHAIN),ARMCC)

ifneq ($(CCIO_LOCAL_OBJS),)
	lib-y += libccio.lib
endif


$(BUILDDIR)/lib/libccio.lib: $(CCIO_LOCAL_OBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^	

endif

