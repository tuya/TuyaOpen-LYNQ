CFLAGS_INC += -I $(LIBDIR)/FW/inc

LIBS_TYPE := $(TYPE)

ifeq ($(BUILD_FW), y)
ifeq ($(TOOLCHAIN),GCC)

FW_PREBUILD_LIBS_DIR   ?= $(LIBDIR)/FW/lib/gcc/$(LIBS_TYPE)

PREBUILDLIBS += $(FW_PREBUILD_LIBS_DIR)/libfw.a
endif

ifeq ($(TOOLCHAIN),ARMCC)
FW_PREBUILD_LIBS_DIR   ?= $(LIBDIR)/FW/lib/keil/$(LIBS_TYPE)

PREBUILDLIBS += $(FW_PREBUILD_LIBS_DIR)/libfw.lib
endif
endif
