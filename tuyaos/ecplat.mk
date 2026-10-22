CFLAGS_INC += -I$(TOP)/../tuyaos/tuyaos_adapter/include
TUYA_ADAPTER_DIR := $(TOP)/../tuyaos/tuyaos_adapter/src
TUYA_ADAPTER_REL_DIR := ../tuyaos/tuyaos_adapter/src
TUYA_ADAPTER_SRCS := $(addprefix $(TUYA_ADAPTER_DIR)/, \
    app.c main.c tuyaopen_license.c \
    tkl_cellular.c tkl_cellular_base.c tkl_cellular_comm.c tkl_cellular_mds.c tkl_cellular_vbat.c cellular_rrc_ctrl.c \
    tkl_fs.c tkl_gpio.c tkl_memory.c tkl_mutex.c tkl_network.c tkl_ota.c tkl_output.c \
    tkl_queue.c tkl_semaphore.c tkl_sleep.c tkl_system.c tkl_thread.c tkl_timer.c tkl_uart.c)
TUYAOPEN_UTIL_DIR := $(TOP)/../../../tools/porting/adapter/utilities/src
TUYAOPEN_UTIL_REL_DIR := $(shell realpath --relative-to=$(CURDIR) $(TUYAOPEN_UTIL_DIR))
TUYAOPEN_INIT_DIR := $(TOP)/../../../tools/porting/adapter/init/src
TUYAOPEN_INIT_REL_DIR := $(shell realpath --relative-to=$(CURDIR) $(TUYAOPEN_INIT_DIR))
TUYAOPEN_UTIL_SRCS := $(addprefix $(TUYAOPEN_UTIL_DIR)/, tuya_list.c tuya_mem_heap.c tuya_queue.c tuya_ringbuf.c tuya_tools.c)
TUYAOPEN_INIT_SRCS := $(addprefix $(TUYAOPEN_INIT_DIR)/, tkl_init_common.c tkl_init_network.c tkl_init_system.c)
tuya_init_obj-y := $(patsubst %.c, %.o, $(TUYAOPEN_INIT_SRCS))
tuya_init_obj-y := $(subst $(TUYAOPEN_INIT_DIR),$(TUYAOPEN_INIT_REL_DIR),$(tuya_init_obj-y))
tuya_util_obj-y := $(patsubst %.c, %.o, $(TUYAOPEN_UTIL_SRCS))
tuya_util_obj-y := $(subst $(TUYAOPEN_UTIL_DIR),$(TUYAOPEN_UTIL_REL_DIR),$(tuya_util_obj-y))
tuya_obj-y := $(patsubst %.c, %.o, $(TUYA_ADAPTER_SRCS))
tuya_obj-y := $(subst $(TUYA_ADAPTER_DIR),$(TUYA_ADAPTER_REL_DIR),$(tuya_obj-y))

obj-y += ${tuya_obj-y} ${tuya_util_obj-y} ${tuya_init_obj-y}

