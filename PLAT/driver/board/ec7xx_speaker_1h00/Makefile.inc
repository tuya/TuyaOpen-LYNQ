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
              -I$(TOP)/PLAT/driver/board/$(TARGET)/inc/exstorage

libdriver-$(DRIVER_PLATCFG_ENABLE)            += PLAT/driver/board/$(TARGET)/src/plat_config.o
libdriver-$(DRIVER_ECMAIN_ENABLE)             += PLAT/driver/board/$(TARGET)/src/bsp.o
libdriverprivate-$(DRIVER_ECMAIN_ENABLE)      += PLAT/driver/board/$(TARGET)/src/ec_main.o
libdriver-$(DRIVER_EEPROM_ENABLE)             += PLAT/driver/board/$(TARGET)/src/eeprom/eepRom.o
libdriver-$(DRIVER_CAMERA_ENABLE)             += PLAT/driver/board/$(TARGET)/src/camera/cameraDrv.o     \
                                              PLAT/driver/board/$(TARGET)/src/camera/cameraDev/gc032A.o     \
                                              PLAT/driver/board/$(TARGET)/src/camera/cameraDev/gc6153.o
                                              
ifeq ($(DRIVER_CODEC_ENABLE), y)
libdriver-$(DRIVER_CODEC8311_ENABLE)          += PLAT/driver/board/$(TARGET)/src/codec/codecDev/es8311.o
libdriver-$(DRIVER_CODEC8374_ENABLE)          += PLAT/driver/board/$(TARGET)/src/codec/codecDev/es8374.o
libdriver-$(DRIVER_CODEC8211_ENABLE)          += PLAT/driver/board/$(TARGET)/src/codec/codecDev/tm8211.o
libdriver-$(DRIVER_CODEC7111_ENABLE)          += PLAT/driver/board/$(TARGET)/src/codec/codecDev/es7111.o
libdriver-$(DRIVER_CODEC7149_ENABLE)          += PLAT/driver/board/$(TARGET)/src/codec/codecDev/es7149.o
libdriver-y                                   +=PLAT/driver/board/$(TARGET)/src/codec/codecDrv.o
endif

libdriver-$(DRIVER_LCD_ENABLE)                += PLAT/driver/board/$(TARGET)/src/lcd/lcdComm.o          \
                                              PLAT/driver/board/$(TARGET)/src/lcd/disFormat.o \
                                              PLAT/driver/board/$(TARGET)/src/lcd/lcdDrv.o

ifeq ($(DRIVER_LCD_ENABLE), y)
libdriver-$(DRIVER_LCD_ST7789_ENABLE)         += PLAT/driver/board/$(TARGET)/src/lcd/lcdDev/lcdDev_7789.o
libdriver-$(DRIVER_LCD_ST77903_ENABLE)        += PLAT/driver/board/$(TARGET)/src/lcd/lcdDev/lcdDev_77903.o
libdriver-$(DRIVER_LCD_AXS15231_ENABLE)       += PLAT/driver/board/$(TARGET)/src/lcd/lcdDev/lcdDev_15231.o
libdriver-$(DRIVER_LCD_CO5300_ENABLE)         += PLAT/driver/board/$(TARGET)/src/lcd/lcdDev/lcdDev_5300.o
endif

libdriver-$(DRIVER_TP_ENABLE)                 += PLAT/driver/board/$(TARGET)/src/tp/tpDrv.o             \
                                                 PLAT/driver/board/$(TARGET)/src/tp/tpComm.o      

libdriver-$(DRIVER_EXSTORAGE_ENABLE)          += PLAT/driver/board/$(TARGET)/src/exstorage/ex_flash.o \
                                                 PLAT/driver/board/$(TARGET)/src/exstorage/ex_storage.o

ifeq ($(DRIVER_TP_ENABLE), y)
libdriver-$(DRIVER_TP_FT6336_ENABLE)          += PLAT/driver/board/$(TARGET)/src/tp/tpDev/ft6336.o
endif