/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_comm.h
 * Description:  ec7xx openHAL comm entry header file
 * History:      Rev1.0   2024-01-11
 *
 ****************************************************************************/
#ifndef  _API_COMM_H_
#define  _API_COMM_H_
#ifdef __cplusplus
extern "C" {
#endif
typedef enum
{
    HAL_PAD    		= 1,
    HAL_GPIO        = 2,
	HAL_AGPIO    	= 3,
	HAL_TIMER		= 4,
	HAL_UART	    = 5,
	HAL_PWM			= 6,
	HAL_APWM	    = 7,
	HAL_I2C			= 8,
	HAL_SPI			= 9,
	HAL_LSPI	    = 10,
	HAL_CSPI	    = 11,
	HAL_I2S			= 12,
	HAL_ONEWIRE		= 13,
	HAL_KPC			= 14,
	HAL_ADC			= 15,
	HAL_SWD			= 16,
	HAL_SIM			= 17,
	DEV_TP			= 18,
	DEV_SCR			= 19,
	HAL_MAX ,
} HalType_t;

#include <stdint.h>
#include "api_def.h"
#include "api_pad.h"
#include "api_gpio.h"
#include "api_uart.h"
#include "api_i2c.h"
#include "api_btn.h"
#include "cmsis_os2.h" //mbtk change
#ifdef FEATURE_HAL_SCREEN_ENABLE
#include "api_scr.h"
#endif
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#define EC_API_CHECK(x) do {                              			\
        api_ret_t ec_ret_ = (x);                           			\
        EC_ASSERT(ec_ret_ < OPEN_HAL_NONE,0,0,0);              		\
    } while(0)

typedef enum {
    TRANS_EVT_LSPI_DMA = 0,	//LSPI DMA完成
	TRANS_EVT_LSPI_BUS ,	//LSPI 传输完成
    TRANS_EVT_I2C0_IRQ ,
	TRANS_EVT_I2C1_IRQ ,
    TRANS_EVT_TOTAL
} portTransEvent_e;
extern osEventFlagsId_t successEvent;

typedef enum {
    FAULT_LSPI_DMA_ERR = 0,
	FAULT_LSPI_BUS_ERR ,
    FAULT_I2C0_BUS_ERR ,
	FAULT_I2C1_BUS_ERR ,
	FAULT_I2C0_TIMEOUT ,
	FAULT_I2C1_TIMEOUT ,
    FAULT_EVT_TOTAL
} portFaultEvent_e;
extern osEventFlagsId_t hwfaultEvent;	//对内输入事件

typedef enum {
    EXCEPTION_LSPI_REINIT = 0,
	EXCEPTION_I2C0_REINIT ,
    EXCEPTION_I2C1_REINIT ,
    EXCEPTION_TOTAL
} exceptionEvent_e;

extern osEventFlagsId_t exceptionEvt;	//对外输出事件

void *exceptionTaskInit(void);
int open_hal_startup(HalType_t tpye,void* para);
int open_hal_parse(char* str,void *output);
uint32_t open_hal_create(HalType_t tpye,uint32_t index,void *cfg);
int open_hal_api_test(HalType_t tpye);
#ifdef __cplusplus
}
#endif
#endif /* _API_COMM_H_ */