/******************************************************************************
 Copyright(C),CEC Huada Electronic Design Co.,Ltd.
 File name: 	    port_stm32l433_i2c.h
 Author:    	    zhengwd
 Version:		    V1.0
 Date:      	            2020-04-07
 Description:
 History:

******************************************************************************/
#ifndef _PORT_STM32L433_I2C_H_
#define _PORT_STM32L433_I2C_H_

/***************************************************************************
 * Include Header Files
 ***************************************************************************/
#include <stdint.h>
#include "hed_private.h"

#include "gpio.h"

#ifdef HED_I2C

/**************************************************************************
 * Global Macro Definition
 ***************************************************************************/
#define CRC_A 0
#define CRC_B 1 // 所有不等于CRC_A的CRCType参数都认为是CRC_B

#define PORT_I2C_HAL_TIMEOUT 50			 // ms
#define PORT_I2C_SE_RST_LOW_DELAY 200	 // us	T7
#define PORT_I2C_SE_RST_HIGH_DELAY 10000 // us	 T6

#define PORT_I2C_SE_PWR_OFF_DEALY 5000 // us	 5ms
#define PORT_I2C_SE_PWR_ON_DEALY 5000  // us	 5ms

/********************控制GPIO 定义*******************/

#if defined(HED_I2C_SE0)
// SE0 RST 控制IO
#define PORT_I2C_SE0_RST_PAD_INDEX (49)			//GPIO24
#define PORT_I2C_SE0_RST_IO_PORT (1)
#define PORT_I2C_SE0_RST_IO_PIN  (8)
#define PORT_I2C_SE0_RST_LOW() GPIO_pinWrite(PORT_I2C_SE0_RST_IO_PORT, 1 << PORT_I2C_SE0_RST_IO_PIN, 0)
#define PORT_I2C_SE0_RST_HIGH() GPIO_pinWrite(PORT_I2C_SE0_RST_IO_PORT, 1 << PORT_I2C_SE0_RST_IO_PIN, 1 << PORT_I2C_SE0_RST_IO_PIN)
#define PORT_IN2C_SE0_DEVICE_INDEX 		(0)
#endif

#if defined(HED_I2C_SE1)
// SE1 RST 控制IO
#define PORT_I2C_SE1_RST_IO_PORT 
#define PORT_I2C_SE1_RST_IO_PIN 
#define PORT_I2C_SE1_RST_LOW() 
#define PORT_I2C_SE1_RST_HIGH()
#define PORT_IN2C_SE0_DEVICE_INDEX 		(1)

#endif

/********************I2C 接口IO 定义*******************/
#define PORT_I2C_ADDRESS_2A (0x2A ) // be carefull!   the addr is 0x2A.
#define PORT_I2C_ADDRESS_2C (0x2C ) // be carefull!   the addr is 0x3C.


/**************************************************************************
 * Global Type Definition
 ***************************************************************************/
enum PORT_I2C_CTRL
{
    PORT_I2C_CTRL_POWER = 0x00000001,
    PORT_I2C_CTRL_RST = 0x00000002,
    PORT_I2C_CTRL_OTHER = 0x0000000F
};

typedef struct _i2c_comm_param_t
{
    void *i2c_handle;
    uint16_t slave_addr;
    uint8_t slave_id;
    bool locked;
} i2c_comm_param_t, *i2c_comm_param_pointer;

/**************************************************************************
 * Global Variable Declaration
 ***************************************************************************/
I2C_PERIPHERAL_DECLARE(I2C_PERIPHERAL_SE0);
I2C_PERIPHERAL_DECLARE(I2C_PERIPHERAL_SE1);

/**************************************************************************
 * Global Functon Declaration
 ***************************************************************************/

extern se_error_t port_i2c_periph_init(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph);
extern se_error_t port_i2c_periph_deinit(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph);
extern se_error_t port_i2c_periph_power_on(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph);
extern se_error_t port_i2c_periph_power_off(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph);
extern se_error_t port_i2c_periph_lock(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph);
extern se_error_t port_i2c_periph_unlock(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph);
extern se_error_t port_i2c_periph_transmit(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph, uint8_t *inbuf, uint32_t inbuf_len);
extern se_error_t port_i2c_periph_receive(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph, uint8_t *outbuf, uint32_t *outbuf_len);
extern se_error_t port_i2c_periph_control(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph, uint32_t ctrlcode, uint8_t *inbuf, uint32_t inbuf_len);

#endif // #ifdef HED_I2C
#endif /*_PORT_STM32L433_I2C_H*/
