

ifeq ($(OS),freertos)
include $(TOP)/PLAT/os/freertos/Makefile.inc
endif

ifeq ($(OS),liteos)
include $(TOP)/PLAT/os/liteos_m-v3.1.4/Makefile.inc
endif

ifeq ($(BUILD_USE_PREBUILD_LIB)_$(BUILD_OSA),n_y)
include $(TOP)/PLAT/os/osa/Makefile.inc
endif
