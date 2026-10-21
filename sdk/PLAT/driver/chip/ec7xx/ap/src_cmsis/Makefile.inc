
OPENDRIVER_DIR := $(TOP)/PLAT/driver/chip/$(CHIP)/$(CORE)/src_cmsis
OPENDRIVER_REL_DIR := PLAT/driver/chip/$(CHIP)/$(CORE)/src_cmsis

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/inc \
              -I $(TOP)/PLAT/os/freertos/CMSIS/inc \
              -I $(OPENDRIVER_DIR)

ifeq ($(DRIVER_SPI_ENABLE),y)
ifneq ($(filter ec718%m, $(TYPE)),)
CFLAGS += -DDRIVER_SPI2_ENABLE
else
CFLAGS += -DDRIVER_SPI_ENABLE
endif
endif

ifeq ($(DRIVER_LPUART_ENABLE),y)
CFLAGS += -DDRIVER_LPUART_ENABLE
endif

ifeq ($(DRIVER_I2C_ENABLE),y)
CFLAGS += -DDRIVER_I2C_ENABLE
endif

ifeq ($(DRIVER_USART_ENABLE),y)
CFLAGS += -DDRIVER_USART_ENABLE
endif

OPENDRIVER_SRC_DIR += $(OPENDRIVER_DIR)
OPENDRIVER_EXCLUDE_SRCS :=

OPENDRIVER_SRCS = $(foreach dir, $(OPENDRIVER_SRC_DIR), $(wildcard $(dir)/*.c))
OPENDRIVER_CFILES = $(filter-out $(OPENDRIVER_EXCLUDE_SRCS), $(OPENDRIVER_SRCS))
OPENDRIVER_OBJSTEMP := $(patsubst %.c, %.o, $(OPENDRIVER_CFILES))
OPENDRIVER_OBJSTEMP := $(subst $(OPENDRIVER_DIR),$(OPENDRIVER_REL_DIR),$(OPENDRIVER_OBJSTEMP))
OPENDRIVER_OBJS     := $(addprefix $(BUILDDIR)/, $(OPENDRIVER_OBJSTEMP))
OPENDRIVER_PPFILES  := $(patsubst %.o, %.pp, $(OPENDRIVER_OBJS))
PPFILES += $(OPENDRIVER_PPFILES)

ifneq ($(MAKECMDGOALS), build-unilog)
-include $(OPENDRIVER_OBJS:.o=.d)
endif
-include $(OPENDRIVER_OBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

lib-y += libopendriver.a

$(BUILDDIR)/lib/libopendriver.a: $(OPENDRIVER_OBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif
