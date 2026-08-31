#ifndef __BF20A6_H__
#define __BF20A6_H__
#include <stdint.h>
#include <stdbool.h>

#define BF20A6_I2C_ADDR 0x6E

#define BF20A6_MAX_EV_VALUE (0x80)
#define BF20A6_MIN_EV_VALUE (0x10)
#define BF20A6_DEFAULT_EV_VALUE (0x48)

#define BF20A6_MAX_CONTRAST_VALUE (0xE0)
#define BF20A6_MIN_CONTRAST_VALUE (0x20)
#define BF20A6_DEFAULT_CONTRAST_VALUE (0x88)

#define BF20A6_MAX_SATURATION_VALUE (0xE0)
#define BF20A6_MIN_SATURATION_VALUE (0x00)
#define BF20A6_DEFAULT_SATURATION_VALUE (0xC8)

#define BF20A6_MAX_SHARPEN_VALUE (0x0C)
#define BF20A6_MIN_SHARPEN_VALUE (0x00)
#define BF20A6_DEFAULT_SHARPEN_VALUE (0x04)

#define BF20A6_DEFAULT_VB_LINE_NUMS (0)
#define BF20A6_DEFAULT_START_OFFSET_LINE_NUMS (0)
#define BF20A6_DEFAULT_ALL_LINE_NUMS (500)
#define BF20A6_DEFAULT_VTS                                                 \
    (BF20A6_DEFAULT_VB_LINE_NUMS + BF20A6_DEFAULT_START_OFFSET_LINE_NUMS + \
     BF20A6_DEFAULT_ALL_LINE_NUMS)

#define BF20A6_DEFAULT_MINUM_FPS (100)

#define BF20A6_GAMMA_TABLE_SIZE (14)

int bf20a6_get_reg_list(uint8_t mode, uint32_t **reg_list, uint32_t *count);
int bf20a6_set_mirror_flip(uint8_t direct);
int bf20a6_set_fps(uint32_t fps);
int bf20a6_get_sensor_id(uint32_t *id);
int bf20a6_set_ev(uint8_t ev);
int bf20a6_set_contrast(uint8_t contrast);
int bf20a6_set_saturation(uint8_t sat);
int bf20a6_set_sharpness(uint8_t sharp);
int bf20a6_set_awb(bool awbEnable, uint8_t scene);
int bf20a6_set_gamma(uint8_t *table, uint32_t size);
int bf20a6_set_ae(uint8_t aeMode);
int bf20a6_set_scene(uint8_t scene);

#endif
