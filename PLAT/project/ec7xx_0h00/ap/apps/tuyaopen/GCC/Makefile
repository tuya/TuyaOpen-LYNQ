
AVAILABLE_TARGETS = ec7xx_0h00
TOOLCHAIN         = GCC
BINNAME           = ap_$(PROJECT)
TOP               := ../../../../../../..
TKL_INC_PATH    := $(TOP)/tuyaos/tuyaos_adapter/include 	\
				   $(TOP)/../../src/common/include	\
				   ${TUYA_APP_KCONFIG_DIR}

include $(TOP)/PLAT/mbtk/target/$(BUILD_PRO_TARGET)/Makefile.inc
#---------------------------------------------------------------------------------
## -----------------------Tuya-----------------------------
TKL_SRC_PATH := $(TOP)/tuyaos/tuyaos_adapter/src \
			 $(TOP)/tuyaos/tuyaos_adapter/include/utilities/src
TKL_SRCS := $(foreach dir,$(TKL_SRC_PATH),$(wildcard $(dir)/*.c))
TKL_OBJ := $(patsubst %.c, %.o, $(TKL_SRCS))
TKL_OBJ := $(subst $(TOP)/,,$(TKL_OBJ))

CFLAGS_INC        += -I ../inc \
					 -I $(TOP)/FIRMWARE/SRC/CAT1/Common/Inc \
					 -I $(TOP)/PLAT/driver/chip/$(CHIP)/ap/src/usb

TUYA_INC := $(addprefix -I , $(shell find $(TKL_INC_PATH) -type d))
CFLAGS_INC += ${TUYA_INC}

# #---------------------------------------------------------------------------------
ADAPTER_DIR := $(TOP)/PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src
ADAPTER_REL_DIR := PLAT/project/$(TARGET)/$(CORE)/apps/$(PROJECT)/src
ADAPTER_SRCS = $(wildcard $(ADAPTER_DIR)/*.c)
obj-y := $(patsubst %.c, %.o, $(ADAPTER_SRCS))
obj-y := $(subst $(ADAPTER_DIR),$(ADAPTER_REL_DIR),$(obj-y))
obj-y += $(TKL_OBJ)

include $(TOP)/PLAT/tools/scripts/Makefile.rules
CFLAGS += -Wno-unused-function -Wno-unused-variable

ADAPTER_OBJS     := $(addprefix $(BUILDDIR)/, $(obj-y))
ADAPTER_PPFILES  := $(patsubst %.o, %.pp, $(ADAPTER_OBJS))
PPFILES += $(ADAPTER_PPFILES)

LIBS += ${TUYA_LIB_DIR}/libtuyaos.a
LIBS += ${TUYA_LIB_DIR}/libtuyaapp.a

$(BUILDDIR)/lib/libtuyaos_adapter.a: $(ADAPTER_OBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

tuyaos_adapter: $(BUILDDIR)/lib/libtuyaos_adapter.a $(LIBS) $(LD_COMPILED_FILE) $(BUILDDIR)/mem_map.txt
	$(eval LIBS += $(BUILDDIR)/lib/libtuyaos_adapter.a)

elf:
	@echo "make elf"

