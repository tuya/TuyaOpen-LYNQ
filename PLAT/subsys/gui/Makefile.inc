SUBSYS_GUI_DIRS := $(TOP)/PLAT/subsys/gui

CFLAGS_INC += -I $(SUBSYS_GUI_DIRS)

ifeq ($(SUBSYS_GUI_LVGL_ENABLE),y)
include $(SUBSYS_GUI_DIRS)/lvgl/Makefile.inc
else ifeq ($(SUBSYS_GUI_LVGL9_ENABLE),y)
include $(SUBSYS_GUI_DIRS)/lvgl9/Makefile.inc
endif
ifeq ($(SUBSYS_GUI_TEST_ENABLE),y)
include $(SUBSYS_GUI_DIRS)/test/Makefile.inc
endif