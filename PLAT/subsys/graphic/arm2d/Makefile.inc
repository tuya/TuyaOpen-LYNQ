ARM2D_DIR		:= $(TOP)/PLAT/subsys/graphic/arm2d

CFLAGS_INC  += -I $(ARM2D_DIR)/Library/Include \
				-I $(ARM2D_DIR)/Helper/Include \
				-I $(ARM2D_DIR)/examples/common/controls \
				-I $(ARM2D_DIR)/examples/common/asset \
				-I $(ARM2D_DIR)/examples/benchmark \
				-I $(ARM2D_DIR)/examples/meter \
				-I $(ARM2D_DIR)/examples/watch \
				-I $(ARM2D_DIR)/../math \
				-I $(ARM2D_DIR)

ARM2D_SRC_DIRS +=  $(ARM2D_DIR)/Library/Source \
					$(ARM2D_DIR)/Helper/Source \
					$(ARM2D_DIR)/examples/common/controls \
					$(ARM2D_DIR)/examples/common/asset \
					$(ARM2D_DIR)/examples/benchmark \
					$(ARM2D_DIR)/examples/meter \
					$(ARM2D_DIR)/examples/watch \
					$(ARM2D_DIR)/../math \
                    $(ARM2D_DIR) 

ARM2D_EXCLUDE_FILES :=
ARM2D_CSRC = $(foreach dir, $(ARM2D_SRC_DIRS), $(wildcard $(dir)/*.c))
ARM2D_CFILES = $(filter-out $(ARM2D_EXCLUDE_FILES), $(ARM2D_CSRC))
ARM2D_COBJSTEMP := $(patsubst %.c, %.o, $(ARM2D_CFILES))
ARM2D_COBJSTEMP :=$(subst $(ARM2D_DIR),PLAT/subsys/graphic/arm2d,$(ARM2D_COBJSTEMP))
ARM2D_COBJS := $(addprefix $(BUILDDIR)/, $(ARM2D_COBJSTEMP))

ARM2D_PPFILES := $(patsubst %.o, %.pp, $(ARM2D_COBJS))
PPFILES += $(ARM2D_PPFILES)

ifneq ($(MAKECMDGOALS), unilog)
-include $(ARM2D_COBJS:.o=.d)
endif
-include $(ARM2D_COBJS:.o=.dd)

CFLAGS += -DFEATURE_THIRDPARTY_ARM2D_ENABLE
CFLAGS += -DRTE_Acceleration_Arm_2D_Extra_Controls
CFLAGS += -DRTE_Acceleration_Arm_2D_Helper_Disp_Adapter0
CFLAGS += -DRTE_Acceleration_Arm_2D_Extra_Benchmark
CFLAGS += -DRTE_Acceleration_Arm_2D_Extra_Benchmark_Watch_Panel

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libarm2d.a

$(BUILDDIR)/lib/libarm2d.a: $(ARM2D_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil


lib-y += libarm2d.lib

$(BUILDDIR)/lib/libarm2d.lib: $(ARM2D_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
