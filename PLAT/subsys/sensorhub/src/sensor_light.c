/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of EigenComm Ltd.
 * File name:    sensor_light.c
 * Description:  EC718 
 * History:      Rev1.0   2023-04-20
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_SENSORHUB_ENABLE
#include <stdint.h>
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "cmsis_os2.h"
#include <stdio.h>
#include <stdbool.h>
#include "bsp_custom.h"
#include "bsp.h"

#include "sensorhub.h"
#include "sw_i2c.h"
#include "hw_i2c.h"

#include "bh1750.h"
#include "sensor_light.h"

static bus_t light_bus;

void sensor_light_init(void)
{
    // light_bus = sw_i2c_default();
    light_bus = hw_i2c_default(0);
    // light_bus.i2c_init();
    bh1750_init(light_bus);
}


void sensor_light_test(void)
{

}


void sensor_light_loop(void)
{
    uint16_t lux = bh1750_lux_once(light_bus);
    printf("\r\n---------------------------%d lux\r\n",lux);
    // printf("\r\nlight:%d lux\r\n",lux);
}

#endif