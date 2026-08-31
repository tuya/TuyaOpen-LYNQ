ifeq ($(SUBSYS_AI_DOUBAO_ENABLE),y)
CFLAGS += -DFEATURE_SUBSYS_AI_ENABLE
include $(TOP)/PLAT/subsys/ai/doubao/Makefile.inc
include $(TOP)/PLAT/subsys/ai/aiEngine/Makefile.inc
endif


