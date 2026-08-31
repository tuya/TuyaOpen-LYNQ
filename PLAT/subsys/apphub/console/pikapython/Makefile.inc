PIKA_DIR        := $(TOP)/PLAT/subsys/apphub/console/pikapython

CFLAGS_INC     +=  -I $(PIKA_DIR) 	\
				   -I $(PIKA_DIR)/pikascript-core \
				   -I $(PIKA_DIR)/pikascript-lib/pika_lvgl \
				   -I $(PIKA_DIR)/pikascript-lib/PikaStdLib \
				   -I $(PIKA_DIR)/pikascript-lib/PikaStdDevice \
				   -I $(PIKA_DIR)/pikascript-lib/ec7xx \
                   -I $(PIKA_DIR)/pikascript-api

PIKA_SRC_DIRS += $(PIKA_DIR)/pikascript-core \
				 $(PIKA_DIR)/pikascript-lib/time \
				 $(PIKA_DIR)/pikascript-lib/pika_lvgl \
				 $(PIKA_DIR)/pikascript-lib/PikaStdLib \
				 $(PIKA_DIR)/pikascript-lib/PikaStdDevice \
				 $(PIKA_DIR)/pikascript-lib/ec7xx \
				 $(PIKA_DIR)/pikascript-api \
				 $(PIKA_DIR)

PIKA_EXCLUDE_FILES :=
PIKA_CSRC = $(foreach dir, $(PIKA_SRC_DIRS), $(wildcard $(dir)/*.c))
PIKA_CFILES = $(filter-out $(PIKA_EXCLUDE_FILES), $(PIKA_CSRC))
PIKA_COBJSTEMP := $(patsubst %.c, %.o, $(PIKA_CFILES))
PIKA_COBJSTEMP :=$(subst $(PIKA_DIR),PLAT/subsys/apphub/console/pikapython,$(PIKA_COBJSTEMP))
PIKA_COBJS := $(addprefix $(BUILDDIR)/, $(PIKA_COBJSTEMP))

PIKA_PPFILES := $(patsubst %.o, %.pp, $(PIKA_COBJS))
PPFILES += $(PIKA_PPFILES)

ifneq ($(MAKECMDGOALS), unilog)
-include $(PIKA_COBJS:.o=.d)
endif
-include $(PIKA_COBJS:.o=.dd)

CFLAGS += -DFEATURE_SUBSYS_PIKAPYTHON_ENABLE

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libpikapython.a

$(BUILDDIR)/lib/libpikapython.a: $(PIKA_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil


lib-y += libpikapython.lib

$(BUILDDIR)/lib/libpikapython.lib: $(PIKA_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
