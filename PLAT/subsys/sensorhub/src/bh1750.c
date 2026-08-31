/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of EigenComm Ltd.
 * File name:    bh1750.c
 * Description:  EC718 
 * History:      Rev1.0   2023-04-20
 *
 ****************************************************************************/
#include <stdint.h>
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"
#include "bsp.h"

#include "sensorhub.h"
#include "sw_i2c.h"
#include "bh1750.h"

//对具体器件功能的抽象，只和该传感器的芯片手册相关

uint16_t bh1750_lux_once(bus_t bus)
{
    uint8_t data[2]={0};
    bus.i2c_send(BH1750_7b_ADDR, BH1750_POWER_ON, data, 0);
    bus.i2c_send(BH1750_7b_ADDR, BH1750_RESET, data, 0);
    bus.i2c_send(BH1750_7b_ADDR, BH1750_ONE_TIME_HIGH_RES_MODE, data, 0);
    // bus.delay_ms(150);
    bus.i2c_wait(BH1750_7b_ADDR,data,2,150000);
    uint16_t lux = (uint16_t)((data[BH1750_DATA_HIGH]*256 + data[BH1750_DATA_LOW])/1.2);
    // printf("light:0x%02x 0x%02x,%d\r\n",data[BH1750_DATA_HIGH],data[BH1750_DATA_LOW],lux);
    return lux;
}

uint8_t bh1750_init(bus_t bus)
{
    uint8_t ret = 0;
    uint8_t data[2]={0};
    bus.i2c_send(BH1750_7b_ADDR, BH1750_POWER_ON, data, 0);
    bus.i2c_send(BH1750_7b_ADDR, BH1750_RESET, data, 0);
    bus.i2c_send(BH1750_7b_ADDR, BH1750_CONTINUOUS_LOW_RES_MODE, data, 0);
    // bus.delay_ms(20);
    bus.i2c_wait(BH1750_7b_ADDR,data,2,20000);
    if(data[BH1750_DATA_HIGH]!=0xff && data[BH1750_DATA_LOW]!=0xff){
        ret = 1;
    }
    // uint16_t lux = (uint16_t)((data[BH1750_DATA_HIGH]*256 + data[BH1750_DATA_LOW])/1.2);
    // printf("\r\nbh1750:0x%02x 0x%02x,%d\r\n",data[BH1750_DATA_HIGH],data[BH1750_DATA_LOW],lux);
    return ret;
}

