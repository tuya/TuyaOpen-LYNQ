SUBSYS_HTTP_DIR  := $(TOP)/PLAT/subsys/comm/http
CFLAGS_INC += -I $(SUBSYS_HTTP_DIR)/inc

CFLAGS += -DFEATURE_SUBSYS_HTTPC_ENABLE

SUBSYS_HTTP_SRC_DIRS += $(SUBSYS_HTTP_DIR)/src

SUBSYS_HTTP_EXCLUDE_FILES :=

SUBSYS_HTTP_CSRC = $(foreach dir, $(SUBSYS_HTTP_SRC_DIRS), $(wildcard $(dir)/*.c))
SUBSYS_HTTP_CFILES = $(filter-out $(SUBSYS_HTTP_EXCLUDE_FILES), $(SUBSYS_HTTP_CSRC))
SUBSYS_HTTP_COBJSTEMP := $(patsubst %.c, %.o, $(SUBSYS_HTTP_CFILES))
SUBSYS_HTTP_COBJSTEMP :=$(subst $(SUBSYS_HTTP_DIR),PLAT/subsys/comm/http,$(SUBSYS_HTTP_COBJSTEMP))
SUBSYS_HTTP_COBJS := $(addprefix $(BUILDDIR)/, $(SUBSYS_HTTP_COBJSTEMP))
SUBSYS_HTTP_PPFILES := $(patsubst %.o, %.pp, $(SUBSYS_HTTP_COBJS))
PPFILES += $(SUBSYS_HTTP_PPFILES)


ifneq ($(MAKECMDGOALS), build-unilog)
-include $(SUBSYS_HTTP_COBJS:.o=.d)
endif
-include $(SUBSYS_HTTP_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsubsys_http.a

$(BUILDDIR)/lib/libsubsys_http.a: $(SUBSYS_HTTP_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

lib-y += libsubsys_http.lib

$(BUILDDIR)/lib/libsubsys_http.lib: $(SUBSYS_HTTP_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
