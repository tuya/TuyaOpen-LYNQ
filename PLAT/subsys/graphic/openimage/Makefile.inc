OPENIMAGEDIR		:= $(TOP)/PLAT/subsys/graphic/openimage

CFLAGS_INC  += -I $(OPENIMAGEDIR)/inc \
				-I $(OPENIMAGEDIR)

OPENIMAGESRC_DIRS +=  $(OPENIMAGEDIR)/src \
                    $(OPENIMAGEDIR) 

OPENIMAGEEXCLUDE_FILES :=
OPENIMAGECSRC = $(foreach dir, $(OPENIMAGESRC_DIRS), $(wildcard $(dir)/*.c))
OPENIMAGECFILES = $(filter-out $(OPENIMAGEEXCLUDE_FILES), $(OPENIMAGECSRC))
OPENIMAGECOBJSTEMP := $(patsubst %.c, %.o, $(OPENIMAGECFILES))
OPENIMAGECOBJSTEMP :=$(subst $(OPENIMAGEDIR),PLAT/subsys/graphic/openimage,$(OPENIMAGECOBJSTEMP))
OPENIMAGECOBJS := $(addprefix $(BUILDDIR)/, $(OPENIMAGECOBJSTEMP))

OPENIMAGEPPFILES := $(patsubst %.o, %.pp, $(OPENIMAGECOBJS))
PPFILES += $(OPENIMAGEPPFILES)

ifneq ($(MAKECMDGOALS), unilog)
-include $(OPENIMAGECOBJS:.o=.d)
endif
-include $(OPENIMAGECOBJS:.o=.dd)

CFLAGS += -DFEATURE_SUBSYS_GRAPHIC_OPENIMAGE_ENABLE

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libopenimage.a

$(BUILDDIR)/lib/libopenimage.a: $(OPENIMAGECOBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil


lib-y += libopenimage.lib

$(BUILDDIR)/lib/libopenimage.lib: $(OPENIMAGECOBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
