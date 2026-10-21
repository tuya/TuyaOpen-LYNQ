UI_WATCH_DIR := $(TOP)/PLAT/subsys/apphub/watch

ifeq ($(SUBSYS_GUI_WATCH_ENABLE),y)
CFLAGS += -DFEATURE_SUBSYS_WATCH_ENABLE
CFLAGS_INC 	+= 	-I $(UI_WATCH_DIR)		\
				-I $(UI_WATCH_DIR)/menu	\
				-I $(UI_WATCH_DIR)/ui

UI_WATCH_DIRS +=	$(UI_WATCH_DIR)/			\
					$(UI_WATCH_DIR)/ui			\
					$(UI_WATCH_DIR)/menu		\
					$(UI_WATCH_DIR)/fonts	    \
                    $(UI_WATCH_DIR)/apps

ifeq ($(WATCHFACE_34_2_DIAL),y)
CFLAGS += -DENABLE_FACE_34_2_DIAL
CFLAGS_INC 	+= 	-I $(UI_WATCH_DIR)/faces/34_2_dial	
UI_WATCH_DIRS +=	$(UI_WATCH_DIR)/faces/34_2_dial				\
					$(UI_WATCH_DIR)/faces/34_2_dial/assets	
endif
ifeq ($(WATCHFACE_75_2_DIAL),y)
CFLAGS += -DENABLE_FACE_75_2_DIAL
CFLAGS_INC 	+= 	-I $(UI_WATCH_DIR)/faces/75_2_dial	
UI_WATCH_DIRS +=	$(UI_WATCH_DIR)/faces/75_2_dial				\
					$(UI_WATCH_DIR)/faces/75_2_dial/assets	
endif
ifeq ($(WATCHFACE_SMART_RESIZED),y)
CFLAGS += -DENABLE_FACE_SMART_RESIZED
CFLAGS_INC 	+= 	-I $(UI_WATCH_DIR)/faces/smart_resized	
UI_WATCH_DIRS +=	$(UI_WATCH_DIR)/faces/smart_resized			\
					$(UI_WATCH_DIR)/faces/smart_resized/assets	
endif

CFLAGS_INC 	+= 	-I $(UI_WATCH_DIR)/apps/aplipay	
UI_WATCH_DIRS +=	$(UI_WATCH_DIR)/apps/aplipay

UI_WATCH_EXCLUDE_FILES :=

UI_WATCH_CSRC = $(foreach dir, $(UI_WATCH_DIRS), $(wildcard $(dir)/*.c))
UI_WATCH_CFILES = $(filter-out $(UI_WATCH_EXCLUDE_FILES), $(UI_WATCH_CSRC))
UI_WATCH_COBJSTEMP := $(patsubst %.c, %.o, $(UI_WATCH_CFILES))
UI_WATCH_COBJSTEMP :=$(subst $(UI_WATCH_DIR),PLAT/subsys/apphub/watch,$(UI_WATCH_COBJSTEMP))
UI_WATCH_COBJS := $(addprefix $(BUILDDIR)/, $(UI_WATCH_COBJSTEMP))
UI_WATCH_PPFILES := $(patsubst %.o, %.pp, $(UI_WATCH_COBJS))
PPFILES += $(UI_WATCH_PPFILES)

ifneq ($(MAKECMDGOALS), build-unilog)
-include $(UI_WATCH_COBJS:.o=.d)
endif
-include $(UI_WATCH_COBJS:.o=.dd)

ifeq ($(TOOLCHAIN),GCC)
lib-y += libui_watch.a
$(BUILDDIR)/lib/libui_watch.a: $(UI_WATCH_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^
endif
endif	#SUBSYS_GUI_WATCH_ENABLE