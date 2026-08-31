opus_DIR        := $(TOP)/PLAT/middleware/thirdparty/opus

CFLAGS_INC     +=  -I $(TOP)/PLAT/middleware/thirdparty/opus \
                    -I $(TOP)/PLAT/middleware/thirdparty/opus/include \
	           		-I $(TOP)/PLAT/os/freertos/inc               \
                  	-I $(TOP)/PLAT/os/freertos/CMSIS/inc        \
                  	-I $(TOP)/PLAT/os/osa/inc
              
opus_SRC_DIRS += $(opus_DIR)/source \
                        $(opus_DIR)
CFLAGS_DEFS += -DFIXED_POINT_OPUS	
CFLAGS_DEFS += -DDISABLE_FLOAT_API			 
opus_EXCLUDE_FILES :=
opus_CSRC = $(foreach dir, $(opus_SRC_DIRS), $(wildcard $(dir)/*.c))
opus_CFILES = $(filter-out $(opus_EXCLUDE_FILES), $(opus_CSRC))
opus_COBJSTEMP := $(patsubst %.c, %.o, $(opus_CFILES))
opus_COBJSTEMP :=$(subst $(opus_DIR),PLAT/middleware/thirdparty/opus,$(opus_COBJSTEMP))
opus_COBJS := $(addprefix $(BUILDDIR)/, $(opus_COBJSTEMP))
opus_PPFILES := $(patsubst %.o, %.pp, $(opus_COBJS))
PPFILES += $(opus_PPFILES)


ifneq ($(MAKECMDGOALS), unilog)
-include $(opus_COBJS:.o=.d)
endif
-include $(opus_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libopus.a

$(BUILDDIR)/lib/libopus.a: $(opus_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil


lib-y += libopus.lib

$(BUILDDIR)/lib/libopus.lib: $(opus_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
