#ifndef __GC6133_H__
#define __GC6133_H__
#include <stdint.h>
#include <stdbool.h>
#define GC6133_I2C_ADDR 0x40

#define GC6133_DEFAULT_VB_LINE_NUMS (2)
#define GC6133_DEFAULT_START_OFFSET_LINE_NUMS (8)
#define GC6133_DEFAULT_ALL_LINE_NUMS (330)
#define GC6133_DEFAULT_VTS                                          \
    (GC6133_DEFAULT_VB_LINE_NUMS + GC6133_DEFAULT_START_OFFSET_LINE_NUMS + \
     GC6133_DEFAULT_ALL_LINE_NUMS)
#define GC6133_DEFAULT_MAXIM_FPS (1500)
#define GC6133_DEFAULT_MINUM_FPS (100)

#define GC6133_MAX_EV_VALUE (0x58)
#define GC6133_MIN_EV_VALUE (0x10)
#define GC6133_DEFAULT_EV_VALUE (0x48)

#define GC6133_MAX_CONTRAST_VALUE (0x58)
#define GC6133_MIN_CONTRAST_VALUE (0x28)
#define GC6133_DEFAULT_CONTRAST_VALUE (0x4A)

#define GC6133_MAX_SATURATION_VALUE (0x80)
#define GC6133_MIN_SATURATION_VALUE (0x00)
#define GC6133_DEFAULT_SATURATION_VALUE (0x40)

#define GC6133_MAX_SHARPEN_VALUE (0x0F)
#define GC6133_MIN_SHARPEN_VALUE (0x00)
#define GC6133_DEFAULT_SHARPEN_VALUE (0x08)


int gc6133_get_reg_list(uint8_t mode, uint32_t **reg_list, uint32_t *count);
int gc6133_set_mirror_flip(uint8_t direct);
int gc6133_set_fps(uint32_t fps);
int gc6133_get_sensor_id(uint32_t *id);
int gc6133_set_ev(uint8_t ev);
int gc6133_set_contrast(uint8_t contrast);
int gc6133_set_saturation(uint8_t sat);
int gc6133_set_sharpness(uint8_t sharp);
int gc6133_set_awb(bool awbEnable, uint8_t scene);
int gc6133_set_gamma(uint8_t *table, uint32_t size);
int gc6133_set_ae(uint8_t aeMode);
int gc6133_set_scene(uint8_t scene);

#endif
