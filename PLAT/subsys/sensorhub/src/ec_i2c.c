
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include "bsp.h"

#include "sw_i2c.h"
#include "hw_i2c.h"
#include "ec_i2c.h"
#include "api_i2c.h"

static ec_i2c_type s_used_i2c = SW_I2C2;
static uint32_t i2c_id = 0;
uint8_t ec_i2c_init(uint8_t sn)
{
    if(s_used_i2c == SW_I2C2) {
        return sw_i2c_init(sn);
    }
    else{
        return hw_i2c_init(sn);
    }
}

uint8_t ec_i2c_wait(uint8_t addr,uint8_t *data,uint16_t len,uint32_t time)
{
    if(s_used_i2c == SW_I2C2)
        return sw_i2c_wait(addr, data, len, time);
    else
        return hw_i2c_wait(addr, data, len, time);
}

uint8_t ec_i2c_send(uint8_t addr,uint8_t reg,uint16_t len,uint8_t *data)
{
    if(s_used_i2c == SW_I2C2)
        return sw_i2c_send(addr, reg, len, data);
    else
        return hw_i2c_send(addr, reg, len, data);
}

uint8_t ec_i2c_read(uint8_t addr,uint8_t reg,uint16_t len,uint8_t *data)
{
    if(s_used_i2c == SW_I2C2)
        return sw_i2c_read(addr, reg, len, data);
    else
        return hw_i2c_read(addr, reg, len, data);
}

bus_t I2C_getDefault(ec_i2c_type sn)
{
    if(sn >= SW_I2C2) {
        s_used_i2c = SW_I2C2;
        return sw_i2c_default();
    }
    else{
        s_used_i2c = sn;
        return hw_i2c_default(sn);
    }
}

void ec_hal_i2c_set_id(uint32_t id)

{
    i2c_id = id;
}

uint8_t ec_hal_i2c_send(uint8_t addr, uint8_t reg, uint16_t len, uint8_t *data)
{
    api_i2c_master_t buffer = {
        .addr = addr,
        .reg = reg,
        .data = data,
        .num = len,
    };
    return api_i2c_write(i2c_id, &buffer, sizeof(buffer));
}

uint8_t ec_hal_i2c_read(uint8_t addr, uint8_t reg, uint16_t len, uint8_t *data)
{
    // printf("addr,  reg,  len,  *data = 0x%02x, 0x%02x, %d, %d\r\n", addr, reg, len, *data);
    api_i2c_master_t buffer = {
        .addr = addr,
        .reg = reg,
        .data = data,
        .num = len,
    };
    return api_i2c_read(i2c_id, &buffer, sizeof(buffer));
}