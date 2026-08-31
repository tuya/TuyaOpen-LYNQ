CFLAGS_INC += -I$(TOP)/PLAT/driver/board/$(TARGET)/inc  \
              -I$(TOP)/PLAT/driver/board/$(TARGET)/inc/eeprom \
              -I$(TOP)/PLAT/driver/board/$(TARGET)/inc/camera \
              -I$(TOP)/PLAT/driver/board/$(TARGET)/inc/camera/cameraDev \
              -I$(TOP)/PLAT/driver/board/$(TARGET)/inc/codec \
              -I$(TOP)/PLAT/driver/board/$(TARGET)/inc/codec/codecDev \
              -I$(TOP)/PLAT/driver/board/$(TARGET)/inc/lcd/lcdDev \
              -I$(TOP)/PLAT/driver/board/$(TARGET)/inc/lcd \
			  -I$(TOP)/PLAT/driver/board/$(TARGET)/inc/tp/tpDev           \
              -I$(TOP)/PLAT/driver/board/$(TARGET)/inc/tp  \
              -I$(TOP)/PLAT/driver/board/$(TARGET)/inc/ntc \
              -I$(TOP)/PLAT/driver/board/$(TARGET)/inc/exstorage \
              -I$(TOP)/PLAT/driver/board/$(TARGET)/inc/keypad

ifeq ($(DRIVER_CODEC2601_ENABLE),y)
CFLAGS += -DCODEC_SC2601_ENABLE
endif
