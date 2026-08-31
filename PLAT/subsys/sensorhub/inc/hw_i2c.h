
#ifndef __HW_I2C_H__
#define __HW_I2C_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "ec_i2c.h"

bus_t hw_i2c_default(uint8_t sn);
uint8_t hw_i2c_init(uint8_t sn);
uint8_t hw_i2c_wait(uint8_t dev_id, uint8_t *data, uint16_t len, uint32_t time) ;
uint8_t hw_i2c_send(uint8_t dev_id, uint8_t reg_addr, uint16_t len, uint8_t *data);
uint8_t hw_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint16_t len, uint8_t *data);

#ifdef __cplusplus
}
#endif
#endif
