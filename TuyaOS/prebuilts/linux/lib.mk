SDK_ROOT := $(shell pwd)/../..
LIB_NAME ?= $(notdir $(shell pwd))

include $(SDK_ROOT)/prebuilts/linux/env.mk

# INC_PATH := $(shell find $(SDK_ROOT)/include -type d)
define find_includes
$(shell find $(SDK_ROOT)/include -name .git -prune -o -name lib_tflm -prune -o -type d -print)
endef
INC_PATH := $(foreach dir,$(call find_includes),$(if $(wildcard $(dir)/.),$(dir)))
LIB_TFLM_DIR := $(SDK_ROOT)/include/lib_tflm
ifneq ("$(wildcard $(LIB_TFLM_DIR)/.)","")
    INC_PATH += $(LIB_TFLM_DIR)
    INC_PATH +=$(LIB_TFLM_DIR)/third_party/flatbuffers/include
    INC_PATH +=$(LIB_TFLM_DIR)/third_party/gemmlowp
	CXXFLAGS += -DTF_LITE_MICRO_ENABLED -DTF_LITE_MCU_DEBUG_LOG -DTF_LITE_STATIC_MEMORY -DTF_LITE_STRIP_ERROR_STRINGS -DCMSIS_NN
endif
INC_FLAGS:=$(addprefix -I, $(INC_PATH))

# CFLAGS += $(addprefix -I, $(INC_PATH))
CFLAGS += -Iinclude
CXXFLAGS+= -Iinclude

OUTPUT := build
# Find all .c and .cc files in src directory and its subdirectories
SRCS_C := $(shell find src/ -name "*.c")
SRCS_CC := $(shell find src/ -name "*.cc")

# Generate the corresponding .o files in $(OUTPUT)/objs directory with subdirectories
OBJS_C := $(SRCS_C:src/%.c=$(OUTPUT)/objs/%.o)
OBJS_CC := $(SRCS_CC:src/%.cc=$(OUTPUT)/objs/%.o)
OBJS := $(OBJS_C) $(OBJS_CC)


all: lib$(LIB_NAME).a

lib$(LIB_NAME).a: $(OBJS)
	@echo "AR rcs $@ $^"
	@$(AR) rcs $(OUTPUT)/$@ $^

# Compile .c files to .o files
$(OBJS_C): $(OUTPUT)/objs/%.o : src/%.c
	@mkdir -p $(dir $@)
	@echo "CC $<"
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(INC_FLAGS) -g -c -o $@ $<

# Compile .cc files to .o files
$(OBJS_CC): $(OUTPUT)/objs/%.o : src/%.cc
	@mkdir -p $(dir $@)
	@echo "CXX $<"
	@$(CXX) $(CXXFLAGS) $(EXTRA_CFLAGS) $(INC_FLAGS) -g -c -o $@ $<

clean:
	rm -rf build

.PHONY: all clean

