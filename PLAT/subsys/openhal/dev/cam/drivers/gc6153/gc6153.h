#ifndef __GC6153_H__
#define __GC6153_H__
#include <stdint.h>
#include <stdbool.h>

#define GC6153_I2C_ADDR 0x40

#define GC6153_MAX_EV_VALUE (0x58)
#define GC6153_MIN_EV_VALUE (0x10)
#define GC6153_DEFAULT_EV_VALUE (0x48)

#define GC6153_MAX_CONTRAST_VALUE (0x58)
#define GC6153_MIN_CONTRAST_VALUE (0x28)
#define GC6153_DEFAULT_CONTRAST_VALUE (0x4B)

#define GC6153_MAX_SATURATION_VALUE (0x80)
#define GC6153_MIN_SATURATION_VALUE (0x00)
#define GC6153_DEFAULT_SATURATION_VALUE (0x40)

#define GC6153_MAX_SHARPEN_VALUE (0x0F)
#define GC6153_MIN_SHARPEN_VALUE (0x00)
#define GC6153_DEFAULT_SHARPEN_VALUE (0x08)

/* SENSOR LINELENGTH&FRAMELENGTH LIMITATION */
#define GC6153_GC032A_SERIAL_MAX_PV_LINELENGTH (4095 + 694)
#define GC6153_GC032A_SERIAL_MAX_PV_FRAMELENGTH (4095 + 488)

#define GC6153_DEFAULT_VB_LINE_NUMS (2)
#define GC6153_DEFAULT_START_OFFSET_LINE_NUMS (8 + 4)
#define GC6153_DEFAULT_ALL_LINE_NUMS (332)
#define GC6153_DEFAULT_VTS                                          \
    (GC6153_DEFAULT_VB_LINE_NUMS + GC6153_DEFAULT_START_OFFSET_LINE_NUMS + \
     GC6153_DEFAULT_ALL_LINE_NUMS)
#define GC6153_DEFAULT_MAXIM_FPS (1500)
#define GC6153_DEFAULT_MINUM_FPS (100)

int gc6153_get_reg_list(uint8_t mode, uint32_t **reg_list, uint32_t *count);
int gc6153_set_mirror_flip(uint8_t direct);
int gc6153_set_fps(uint32_t fps);
int gc6153_get_sensor_id(uint32_t *id);
int gc6153_set_ev(uint8_t ev);
int gc6153_set_contrast(uint8_t contrast);
int gc6153_set_saturation(uint8_t sat);
int gc6153_set_sharpness(uint8_t sharp);
int gc6153_set_awb(bool awbEnable, uint8_t scene);
int gc6153_set_gamma(uint8_t *table, uint32_t size);
int gc6153_set_ae(uint8_t aeMode);
int gc6153_set_scene(uint8_t scene);

#endif
