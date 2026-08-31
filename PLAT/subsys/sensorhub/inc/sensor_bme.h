
#ifndef __SENSOR_BME680_H__
#define __SENSOR_BME680_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "bme68x_defs.h"
/*
全球首款集成气体、气压、湿度和温度传感功能的环境传感器，SiP器件，集成了温度、湿度、气压和气体传感功能，其内部共有两颗MEMS芯片和一颗ASIC芯片
博世BME680的气压测量精度为± 12 Pa，BME680还能测量相对湿度和环境温度，偏置温度系数仅为1.5 Pa/K
*/

// I2C设备地址
#define BME680_7b_ADDR      BME68X_I2C_ADDR_HIGH

void sensor_bme_init(void);
void sensor_bme_test(void);
void sensor_bme_loop(void);

#ifdef __cplusplus
} 
#endif
#endif
