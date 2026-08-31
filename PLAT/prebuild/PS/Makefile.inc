CFLAGS_INC += -I $(LIBDIR)/PS/inc

LIBS_TYPE := $(TYPE)

ifeq ($(BUILD_PS), y)
ifeq ($(TOOLCHAIN),GCC)

ifneq ($(findstring $(TYPE), ec718p ec718u ec718pm ec718um ec718hm),)
ifeq ($(OPENCPU_MODE_ENABLE), y)
LIBSUBDIR := oc
ifeq ($(OPENCPU_MODE_ENABLE)_$(MORE_ROM_ENABLE), y_y)
LIBSUBDIR := rom
else ifeq ($(OPENCPU_MODE_ENABLE)_$(IMS_MODE_ENABLE), y_y)
LIBSUBDIR := ims
PREBUILDLIBS +=	$(PS_PREBUILD_LIBS_DIR)/libims.a
PREBUILDLIBS +=	$(PS_PREBUILD_LIBS_DIR)/libimsxml.a
else ifeq ($(OPENCPU_MODE_ENABLE)_$(IMS_SMSONLY_MODE_ENABLE), y_y)
LIBSUBDIR := isms
PREBUILDLIBS +=	$(PS_PREBUILD_LIBS_DIR)/libims.a
endif
else
LIBSUBDIR := full
endif

else ifeq ($(TYPE),ec716e)
ifeq ($(OPENCPU_MODE_ENABLE)_$(IMS_SMSONLY_MODE_ENABLE), y_y)
LIBSUBDIR := isms
PREBUILDLIBS +=	$(PS_PREBUILD_LIBS_DIR)/libims.a
endif

else# 716s 718s
ifeq ($(MID_ENABLE), true)
LIBSUBDIR := mid
else ifeq ($(IOTAT_ENABLE), true)
LIBSUBDIR := iotat
else ifeq ($(GCF_ENABLE), true)
LIBSUBDIR := gcf
else ifeq ($(RAM_ENBALE), true)
LIBSUBDIR := ram
else ifeq ($(ROM_ENABLE)_$(LESS_LOG), true_true)
LIBSUBDIR := rom_lesslog
else ifeq ($(OPENCPU)_$(LESS_LOG), true_true)
LIBSUBDIR := oc_lesslog
else 
ifeq ($(ROM_ENABLE), true)
LIBSUBDIR := rom
else
LIBSUBDIR := oc
endif
endif
endif

PS_PREBUILD_LIBS_DIR   ?= $(LIBDIR)/PS/lib/gcc/$(LIBS_TYPE)/$(LIBSUBDIR)


PREBUILDLIBS += $(PS_PREBUILD_LIBS_DIR)/libps.a   \
                $(PS_PREBUILD_LIBS_DIR)/libpsl1.a \
                $(PS_PREBUILD_LIBS_DIR)/libpsif.a 


endif

ifeq ($(TOOLCHAIN),ARMCC)

ifeq ($(OPENCPU), true)

ifeq ($(GCF_ENABLE), true)
PS_PREBUILD_LIBS_DIR   ?= $(LIBDIR)/PS/lib/keil/$(LIBS_TYPE)/gcf
else
PS_PREBUILD_LIBS_DIR   ?= $(LIBDIR)/PS/lib/keil/$(LIBS_TYPE)/oc
endif

else

PS_PREBUILD_LIBS_DIR   ?= $(LIBDIR)/PS/lib/keil/$(LIBS_TYPE)

endif


PREBUILDLIBS += $(PS_PREBUILD_LIBS_DIR)/libps.lib   \
                $(PS_PREBUILD_LIBS_DIR)/libpsl1.lib \
                $(PS_PREBUILD_LIBS_DIR)/libpsif.lib \
                $(PS_PREBUILD_LIBS_DIR)/libims.lib  \
                $(PS_PREBUILD_LIBS_DIR)/libimsxml.lib
endif
endif