#ifndef __BF30A2_H__
#define __BF30A2_H__
#include <stdint.h>
#include <stdbool.h>

#define BF30A2_I2C_ADDR 0x6E
#define BF30A2_MAX_EV_VALUE (0x80)
#define BF30A2_MIN_EV_VALUE (0x10)
#define BF30A2_DEFAULT_EV_VALUE (0x42)

#define BF30A2_MAX_CONTRAST_VALUE (0xc0)
#define BF30A2_MIN_CONTRAST_VALUE (0x40)
#define BF30A2_DEFAULT_CONTRAST_VALUE (0x80)

#define BF30A2_MAX_SATURATION_VALUE (0xD0)
#define BF30A2_MIN_SATURATION_VALUE (0x00)
#define BF30A2_DEFAULT_SATURATION_VALUE (0x98)

#define BF30A2_MAX_SHARPEN_VALUE (0x07)
#define BF30A2_MIN_SHARPEN_VALUE (0x00)
#define BF30A2_DEFAULT_SHARPEN_VALUE (0x04)

#define BF30A2_GAMMA_TABLE_SIZE (14)

int bf30a2_get_reg_list(uint8_t mode, uint32_t **reg_list, uint32_t *count);
int bf30a2_set_mirror_flip(uint8_t direct);
int bf30a2_set_fps(uint32_t fps);
int bf30a2_get_sensor_id(uint32_t *id);
int bf30a2_set_ev(uint8_t ev);
int bf30a2_set_contrast(uint8_t contrast);
int bf30a2_set_saturation(uint8_t sat);
int bf30a2_set_sharpness(uint8_t sharp);
int bf30a2_set_awb(bool awbEnable, uint8_t scene);
int bf30a2_set_gamma(uint8_t *table, uint32_t size);
int bf30a2_set_ae(uint8_t aeMode);
int bf30a2_set_scene(uint8_t scene);

#endif
