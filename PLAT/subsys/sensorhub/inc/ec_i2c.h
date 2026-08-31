#ifndef _EC_I2C_H_
#define _EC_I2C_H_

typedef enum {
    HW_I2C0 = 0, 
    HW_I2C1 ,  
    SW_I2C2 ,       //读取IMU异常
} ec_i2c_type;

typedef struct bus_func
{
    uint8_t (*i2c_init)(uint8_t);
    uint8_t (*i2c_scan)(uint8_t*,uint8_t);
    uint8_t (*i2c_wait)(uint8_t,uint8_t*,uint16_t,uint32_t);
    uint8_t (*i2c_read)(uint8_t,uint8_t,uint16_t,uint8_t*);
    uint8_t (*i2c_send)(uint8_t,uint8_t,uint16_t,uint8_t*);
}bus_t;

bus_t I2C_getDefault(ec_i2c_type type);

uint8_t ec_i2c_init(uint8_t sn);
uint8_t ec_i2c_wait(uint8_t dev_id, uint8_t *data, uint16_t len, uint32_t time) ;
uint8_t ec_i2c_send(uint8_t dev_id, uint8_t reg_addr, uint16_t len, uint8_t *data);
uint8_t ec_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint16_t len, uint8_t *data);
void ec_hal_i2c_set_id(uint32_t id);
uint8_t ec_hal_i2c_send(uint8_t addr, uint8_t reg, uint16_t len, uint8_t *data);
uint8_t ec_hal_i2c_read(uint8_t addr, uint8_t reg, uint16_t len, uint8_t *data);

#endif 
