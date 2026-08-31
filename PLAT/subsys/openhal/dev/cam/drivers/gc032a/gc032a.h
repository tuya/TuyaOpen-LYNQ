#ifndef __GC032A_H__
#define __GC032A_H__
#include <stdint.h>
#include <stdbool.h>

#define GC032A_I2C_ADDR 0x21

/* SENSOR PIXEL/LINE NUMBERS IN ONE PERIOD */
#define GC032A_SERIAL_PV_PERIOD_PIXEL_NUMS (640)
#define GC032A_SERIAL_PV_PERIOD_LINE_NUMS (480)
#define GC032A_SERIAL_FULL_PERIOD_PIXEL_NUMS (640)
#define GC032A_SERIAL_FULL_PERIOD_LINE_NUMS (480)

/* SENSOR LINELENGTH&FRAMELENGTH LIMITATION */
#define GC032A_SERIAL_MAX_PV_LINELENGTH (106 + 694)
#define GC032A_SERIAL_MAX_PV_FRAMELENGTH (112 + 488)

#define GC032A_DEFAULT_VB_LINE_NUMS (16)
#define GC032A_DEFAULT_START_OFFSET_LINE_NUMS (8)
#define GC032A_DEFAULT_ALL_LINE_NUMS (488)
#define GC032A_DEFAULT_VTS                                                 \
    (GC032A_DEFAULT_VB_LINE_NUMS + GC032A_DEFAULT_START_OFFSET_LINE_NUMS + \
     GC032A_DEFAULT_ALL_LINE_NUMS)
#define GC032A_DEFAULT_MINUM_FPS (100)

#define GC032A_MAX_EV_VALUE (0x58)
#define GC032A_MIN_EV_VALUE (0x10)
#define GC032A_DEFAULT_EV_VALUE (0x48)

#define GC032A_MAX_CONTRAST_VALUE (0x48)
#define GC032A_MIN_CONTRAST_VALUE (0x28)
#define GC032A_DEFAULT_CONTRAST_VALUE (0x40)

#define GC032A_MAX_SATURATION_VALUE (0x80)
#define GC032A_MIN_SATURATION_VALUE (0x00)
#define GC032A_DEFAULT_SATURATION_VALUE (0x40)

#define GC032A_MAX_SHARPEN_VALUE (0xFF)
#define GC032A_MIN_SHARPEN_VALUE (0x00)
#define GC032A_DEFAULT_SHARPEN_VALUE (0xaa)

#define GC032A_GAMMA_TABLE_SIZE (22)

int gc032a_get_reg_list(uint8_t mode, uint32_t **reg_list, uint32_t *count);
int gc032a_set_mirror_flip(uint8_t direct);
int gc032a_set_fps(uint32_t fps);
int gc032a_get_sensor_id(uint32_t *id);
int gc032a_set_ev(uint8_t ev);
int gc032a_set_contrast(uint8_t contrast);
int gc032a_set_saturation(uint8_t sat);
int gc032a_set_sharpness(uint8_t sharp);
int gc032a_set_awb(bool awbEnable, uint8_t scene);
int gc032a_set_gamma(uint8_t *table, uint32_t size);
int gc032a_set_ae(uint8_t aeMode);
int gc032a_set_scene(uint8_t scene);

#endif
