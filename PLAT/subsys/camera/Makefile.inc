SUBSYS_CAMERA_DIR  := $(TOP)/PLAT/subsys/camera
CFLAGS_INC += -I $(SUBSYS_CAMERA_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_CAMERA_ENABLE


SUBSYS_CAMERA_SRC_DIRS += $(SUBSYS_CAMERA_DIR)/src

SUBSYS_CAMERA_EXCLUDE_FILES :=

SUBSYS_CAMERA_CSRC = $(foreach dir, $(SUBSYS_CAMERA_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_CAMERA_CFILES = $(filter-out $(SUBSYS_CAMERA_EXCLUDE_FILES), $(SUBSYS_CAMERA_CSRC))
SUBSYS_CAMERA_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_CAMERA_CFILES))
SUBSYS_CAMERA_COBJSTEMP :=$(subst $(SUBSYS_CAMERA_DIR),PLAT/subsys/camera,$(SUBSYS_CAMERA_COBJSTEMP))
SUBSYS_CAMERA_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_CAMERA_COBJSTEMP))
SUBSYS_CAMERA_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_CAMERA_COBJS))
PPFILES += $(SUBSYS_CAMERA_PPFILES)


ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_CAMERA_COBJS:.o=.d)
endif
-include $(SUBSYS_CAMERA_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_camera.a

$(BUILDDIR)/lib/libsubsys_camera.a: $(SUBSYS_CAMERA_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_camera.lib

$(BUILDDIR)/lib/libsubsys_camera.lib: $(SUBSYS_CAMERA_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
