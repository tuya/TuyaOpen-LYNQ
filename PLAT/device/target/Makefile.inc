ifeq ($(TOOLCHAIN),GCC)
PRECINIT_FILE_PATH ?= PLAT/device/target/board/$(TARGET)/$(CORE)/gcc
SYSCALLS_FILE_PATH ?= PLAT/device/target/board/$(TARGET)/$(CORE)/gcc
SYSTEM_FILE_PATH   ?= PLAT/device/target/board/$(TARGET)/$(CORE)/src
STARTUP_FILE_PATH  ?= PLAT/device/target/board/$(TARGET)/$(CORE)/gcc
LINK_FILE_PATH     ?= PLAT/device/target/board/$(TARGET)/$(CORE)/gcc
endif

ifeq ($(TOOLCHAIN),ARMCC)
PRECINIT_FILE_PATH ?= PLAT/device/target/board/$(TARGET)/$(CORE)/keil
SYSTEM_FILE_PATH   ?= PLAT/device/target/board/$(TARGET)/$(CORE)/src
STARTUP_FILE_PATH  ?= PLAT/device/target/board/$(TARGET)/$(CORE)/keil
LINK_FILE_PATH     ?= PLAT/device/target/board/$(TARGET)/$(CORE)/keil
endif

##$(error "error  $(MAIN_FILE_PATH)")
CFLAGS_INC += -I$(TOP)/PLAT/device/target/board/common/ARMCM3/inc  \
              -I$(TOP)/PLAT/device/target/board/$(TARGET)/common/inc \
              -I$(TOP)/PLAT/device/target/board/$(TARGET)/common/pkginc


ifeq ($(TOOLCHAIN),GCC)

STARTUP_OBJS := $(addprefix $(BUILDDIR)/,$(libstartup-y))


ifneq ($(MAKECMDGOALS), unilog)
-include $(STARTUP_OBJS:.o=.d)
endif

CFLAGS_INC += -I$(TOP)/PLAT/device/target/board/$(TARGET)/$(CORE)/gcc \
              -I$(TOP)/PLAT/device/target/board/$(TARGET)/$(CORE)/inc

$(BUILDDIR)/lib/libstartup.a: $(STARTUP_OBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^


ifeq ($(SOC_DEBUG), y)
LD_FILE ?= $(TOP)/PLAT/device/target/board/$(TARGET)/$(CORE)/gcc/ec718xm/$(TARGET)_soc.ld
else

ifeq ($(EC718XM_LD_AND_PREC), y)
LD_FILE ?= $(TOP)/PLAT/device/target/board/$(TARGET)/$(CORE)/gcc/ec718xm/$(TARGET)_flash.ld
else
LD_FILE ?= $(TOP)/PLAT/device/target/board/$(TARGET)/$(CORE)/gcc/$(TARGET)_flash.ld
endif

endif

endif

ifeq ($(TOOLCHAIN),ARMCC)
libstartup-y += $(PRECINIT_FILE_PATH)/prec_init.o  \
                $(SYSTEM_FILE_PATH)/system_$(CHIP).o \
				$(STARTUP_FILE_PATH)/startup_$(CHIP).o

STARTUP_OBJS := $(addprefix $(BUILDDIR)/,$(libstartup-y))

ifneq ($(MAKECMDGOALS), unilog)
-include $(STARTUP_OBJS:.o=.d)
endif


##obj-y        += $(STARTUP_FILE_PATH)/startup_$(CHIP).o

CFLAGS_INC += -I$(TOP)/PLAT/device/target/board/$(TARGET)/$(CORE)/keil \
              -I$(TOP)/PLAT/device/target/board/$(TARGET)/$(CORE)/inc
lib-y += libstartup.lib

ifeq ($(FPGA_DEBUG),y)
linker-script-y := $(TOP)/$(LINK_FILE_PATH)/$(TARGET)_flash_fpga.sct
else
linker-script-y := $(TOP)/$(LINK_FILE_PATH)/$(TARGET)_flash.sct
endif


$(BUILDDIR)/lib/libstartup.lib: $(STARTUP_OBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif


