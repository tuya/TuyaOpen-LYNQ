CFLAGS_INC += -I$(TOP)/../tuyaos/tuyaos_adapter/include
CFLAGS_INC += -I$(TOP)/../tuyaos/tuyaos_adapter/src/ml/include
CFLAGS_INC += -I$(TOP)/../tuyaos/tuyaos_adapter/src/ml/src
TUYA_ADAPTER_DIR := $(TOP)/../tuyaos/tuyaos_adapter/src
TUYA_ADAPTER_REL_DIR := ../tuyaos/tuyaos_adapter/src
TUYA_ADAPTER_SRCS= $(wildcard $(TUYA_ADAPTER_DIR)/*.c)
TUYA_ADAPTER_SRCS+= $(wildcard $(TUYA_ADAPTER_DIR)/ml/src/*.c)
tuya_obj-y := $(patsubst %.c, %.o, $(TUYA_ADAPTER_SRCS))
tuya_obj-y := $(subst $(TUYA_ADAPTER_DIR),$(TUYA_ADAPTER_REL_DIR),$(tuya_obj-y))

obj-y += ${tuya_obj-y}

