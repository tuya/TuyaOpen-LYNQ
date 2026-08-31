SUBSYS_OPENHAL_DIR  := $(TOP)/PLAT/subsys/openhal

CFLAGS_INC += 	-I $(SUBSYS_OPENHAL_DIR)			\
				-I $(SUBSYS_OPENHAL_DIR)/dev/btn	\
				-I $(SUBSYS_OPENHAL_DIR)/dev/tp		\
				-I $(SUBSYS_OPENHAL_DIR)/dev/gnss   \
				-I $(SUBSYS_OPENHAL_DIR)/dev/sensorhub \
				-I $(SUBSYS_OPENHAL_DIR)/inc 		\
				-I $(SUBSYS_OPENHAL_DIR)/dev 		\
				-I $(SUBSYS_OPENHAL_DIR)/dev/tp/tp_func/cst816 \
				-I $(SUBSYS_OPENHAL_DIR)/dev/tp/tp_func/axs15231 \
				-I $(SUBSYS_OPENHAL_DIR)/dev/tp/tp_func/ft6336 \
				-I $(SUBSYS_OPENHAL_DIR)/dev/tp/tp_func/st77922

SUBSYS_OPENHAL_SRC_DIRS += $(SUBSYS_OPENHAL_DIR)	\
				$(SUBSYS_OPENHAL_DIR)/dev/btn		\
				$(SUBSYS_OPENHAL_DIR)/dev/tp		\
				$(SUBSYS_OPENHAL_DIR)/dev/gnss      \
				$(SUBSYS_OPENHAL_DIR)/dev/sensorhub \
				$(SUBSYS_OPENHAL_DIR)/src		    \
				$(SUBSYS_OPENHAL_DIR)/dev  			\
				$(SUBSYS_OPENHAL_DIR)/dev/tp/tp_func/cst816 \
				$(SUBSYS_OPENHAL_DIR)/dev/tp/tp_func/axs15231 \
				$(SUBSYS_OPENHAL_DIR)/dev/tp/tp_func/ft6336 \
				$(SUBSYS_OPENHAL_DIR)/dev/tp/tp_func/st77922

ifeq ($(SUBSYS_MEDIA_BASIC_ENABLE)_$(SUBSYS_AUDIO_ENABLE),y_y)
CFLAGS_INC += -I $(SUBSYS_OPENHAL_DIR)/dev/codec
SUBSYS_OPENHAL_SRC_DIRS += $(SUBSYS_OPENHAL_DIR)/dev/codec
endif

CFLAGS += 	-DFEATURE_SUBSYS_OPENHAL_ENABLE		\
			-DFEATURE_OPENHAL_RTOS_ENABLE

ifeq ($(SUBSYS_OPENHAL_SPI_ENABLE),y)
CFLAGS += -DFEATURE_SUBSYS_OPENHAL_SPI_ENABLE
endif

include $(SUBSYS_OPENHAL_DIR)/dev/lcd/makefile.inc

ifeq ($(SUBSYS_OPENHAL_CAMERA_ENABLE),y)
include $(SUBSYS_OPENHAL_DIR)/dev/cam/makefile.inc
endif

ifeq ($(SUBSYS_OPENHAL_GNSS_ENABLE),y)
include $(SUBSYS_OPENHAL_DIR)/dev/gnss/makefile.inc
endif


SUBSYS_OPENHAL_EXCLUDE_FILES :=

SUBSYS_OPENHAL_CSRC = $(foreach dir, $(SUBSYS_OPENHAL_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_OPENHAL_CFILES = $(filter-out $(SUBSYS_OPENHAL_EXCLUDE_FILES), $(SUBSYS_OPENHAL_CSRC))
SUBSYS_OPENHAL_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_OPENHAL_CFILES))
SUBSYS_OPENHAL_COBJSTEMP :=$(subst $(SUBSYS_OPENHAL_DIR),PLAT/subsys/openhal,$(SUBSYS_OPENHAL_COBJSTEMP))
SUBSYS_OPENHAL_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_OPENHAL_COBJSTEMP))
SUBSYS_OPENHAL_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_OPENHAL_COBJS))
PPFILES += $(SUBSYS_OPENHAL_PPFILES)

ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_OPENHAL_COBJS:.o=.d)
endif
-include $(SUBSYS_OPENHAL_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_openhal.a

$(BUILDDIR)/lib/libsubsys_openhal.a: $(SUBSYS_OPENHAL_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_openhal.lib

$(BUILDDIR)/lib/libsubsys_openhal.lib: $(SUBSYS_OPENHAL_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
