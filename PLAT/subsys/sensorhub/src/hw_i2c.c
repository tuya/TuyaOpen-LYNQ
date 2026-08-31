/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of EigenComm Ltd.
 * File name:    hw_i2c.c
 * Description:  EC718 
 * History:      Rev1.0   2023-04-11
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "string.h"
#include "FreeRTOS.h"
#include "task.h"

#include "bsp_custom.h"

#include "bsp.h"
#include "hw_i2c.h"

static ARM_DRIVER_I2C   *i2cSensorhub = NULL;

uint8_t hw_i2c_init(uint8_t sn)
{
    #if (RTE_I2C0)
    extern ARM_DRIVER_I2C   Driver_I2C0;
    if(sn==0) i2cSensorhub = &CREATE_SYMBOL(Driver_I2C, 0);
    #endif
    #if (RTE_I2C1)
    extern ARM_DRIVER_I2C   Driver_I2C1;
    if(sn==1) i2cSensorhub = &CREATE_SYMBOL(Driver_I2C, 1);
    #endif
    if(i2cSensorhub != NULL){
        i2cSensorhub->Initialize(NULL);
        i2cSensorhub->PowerControl(ARM_POWER_FULL);
        i2cSensorhub->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);
        i2cSensorhub->Control(ARM_I2C_BUS_CLEAR, 0);
        return 1;
    }
    return 0;
}

uint8_t hw_i2c_wait(uint8_t dev_id,uint8_t *data, uint16_t len, uint32_t time) 
{ 
    delay_us(time);
    i2cSensorhub->MasterReceive(dev_id, data, len, false);
    return 0;
}

uint8_t hw_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint16_t len, uint8_t *data) 
{
    i2cSensorhub->MasterTransmit(dev_id, &reg_addr, 1, true);   
    i2cSensorhub->MasterReceive(dev_id, data, len, false);
    // printf("\n\rhi2c_read-x%02X-x%02X:",dev_id,reg_addr);
    // for(uint8_t i = 0; i<len; i++)
    // {
    //     printf("0x%02X ", data[i]);
    // }
    return 0;
}

uint8_t hw_i2c_send(uint8_t dev_id, uint8_t reg_addr, uint16_t len, uint8_t *data)
{
    uint8_t * tempBuffer = calloc(len+1,sizeof(char));
    memcpy(tempBuffer+1,data,len);
    tempBuffer[0] = reg_addr;
    i2cSensorhub->MasterTransmit(dev_id, tempBuffer, len+1, false);
    // printf("\n\rhi2c_send x%02X:",dev_id);
    // for(uint8_t i = 0; i<=len; i++)
    // {
    //     printf("0x%02X ", tempBuffer[i]);
    // }
    free(tempBuffer);
    return 0;
}

bus_t hw_i2c_default(uint8_t sn)
{
    bus_t i2c_bus = {
        .i2c_init = hw_i2c_init,
        .i2c_scan = NULL,
        .i2c_wait = hw_i2c_wait,
        .i2c_read = hw_i2c_read,
        .i2c_send = hw_i2c_send
    };
    i2c_bus.i2c_init(sn);
    return i2c_bus;
}
