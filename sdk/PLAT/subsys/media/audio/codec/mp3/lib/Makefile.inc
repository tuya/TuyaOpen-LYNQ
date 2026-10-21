MP3_DIR := $(TOP)/PLAT/subsys/media/audio/codec/mp3/lib

CFLAGS_INC += -I $(MP3_DIR)/libmad-0.15.1b/msvc++ \
              -I $(MP3_DIR)/libmad-0.15.1b

MP3_SRC_DIRS += $(MP3_DIR)/libmad-0.15.1b \
                $(MP3_DIR)

CFLAGS += -DFEATURE_MP3_ENABLED

