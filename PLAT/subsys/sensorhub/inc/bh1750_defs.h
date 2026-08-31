
#ifndef __BH1750_DEFS_H_
#define __BH1750_DEFS_H_
#ifdef __cplusplus
extern "C" {
#endif


// 定义BH1750测量结果的高低位数据寄存器
#define BH1750_DATA_HIGH    0x00
#define BH1750_DATA_LOW     0x01

// No active state
#define BH1750_POWER_DOWN   0x00

// Wating for measurment command
#define BH1750_POWER_ON     0x01

// Reset data register value - not accepted in POWER_DOWN mode
#define BH1750_RESET        0x07

// Start measurement at 1lx resolution. Measurement time is approx 120ms.
#define BH1750_CONTINUOUS_HIGH_RES_MODE     0x10

// Start measurement at 0.5lx resolution. Measurement time is approx 120ms.
#define BH1750_CONTINUOUS_HIGH_RES_MODE_2   0x11

// Start measurement at 4lx resolution. Measurement time is approx 16ms.
#define BH1750_CONTINUOUS_LOW_RES_MODE      0x13

// Start measurement at 1lx resolution. Measurement time is approx 120ms.
// Device is automatically set to Power Down after measurement.
#define BH1750_ONE_TIME_HIGH_RES_MODE       0x20

// Start measurement at 0.5lx resolution. Measurement time is approx 120ms.
// Device is automatically set to Power Down after measurement.
#define BH1750_ONE_TIME_HIGH_RES_MODE_2     0x21

// Start measurement at 1lx resolution. Measurement time is approx 120ms.
// Device is automatically set to Power Down after measurement.
#define BH1750_ONE_TIME_LOW_RES_MODE        0x23

enum bh1750_intf {
    /*! SPI interface */
    SW_SPI_INTF,
    /*! I2C interface */
    HW_I2C_INTF
};



struct bh1750_data
{
    uint8_t status;
    uint8_t raw[2];
    uint16_t lux;
};

#ifdef __cplusplus
}
#endif
#endif
