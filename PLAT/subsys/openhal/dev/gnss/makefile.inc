GNSS_DIR := $(TOP)/PLAT/subsys/openhal/dev/gnss
CFLAGS_INC += -I $(GNSS_DIR)
GNSS_EXCLUDE_FILES :=
GNSS_C_FILES :=
OPENHAL_GNSS_ICOE   ?= y
ifeq ($(OPENHAL_GNSS_ICOE), y)
CFLAGS += -DHAL_GNSS_ICOE_BUILTIN
CFLAGS += -DGNSS_MAIN_PAD=48
CFLAGS += -DGNSS_MAIN_GPIO=23
CFLAGS += -DGNSS_BCKP_PAD=53
CFLAGS += -DGNSS_BCKP_GPIO=28
CFLAGS_INC += -I $(GNSS_DIR)/icoe
GNSS_C_FILES += $(GNSS_DIR)/icoe/icoe_adap.c
GNSS_C_FILES += $(GNSS_DIR)/icoe/icoe_xmodem.c
GNSS_C_FILES += $(GNSS_DIR)/icoe/icoe_load_fw.c
GNSS_C_FILES += $(GNSS_DIR)/icoe/icoe_gnss_nmea.c
endif

GNSS_CFILES = $(filter-out $(GNSS_EXCLUDE_FILES), $(GNSS_C_FILES))
GNSS_COBJSTEMP := $(patsubst %.c, %.o, $(GNSS_CFILES))
GNSS_COBJSTEMP := $(subst $(TOP)/$(GNSS_DIR),$(GNSS_DIR),$(GNSS_COBJSTEMP))
GNSS_COBJS := $(addprefix $(BUILDDIR)/, $(GNSS_COBJSTEMP))

GNSS_PPFILES := $(patsubst %.o, %.pp, $(GNSS_COBJS))
CUSTLIBPPFILES += $(GNSS_PPFILES)

-include $(GNSS_COBJS:.o=.dd)
ifneq ($(MAKECMDGOALS), unilog)
-include $(GNSS_COBJS:.o=.d)
endif

lib-y += libgnss.a
$(BUILDDIR)/lib/libgnss.a: $(GNSS_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^