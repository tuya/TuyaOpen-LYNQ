/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_comm.h
 * Description:  ec7xx openHAL comm entry header file
 * History:      Rev1.0   2024-01-11
 *
 ****************************************************************************/
#include "clock.h"

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
	HAL_WAKEUP		= 18,
	HAL_MAX ,
} HalType_t;

// 方便人工查找对应关系
enum {
    A4_GPIO16   = 11,
    A4_GPIO17 ,
    A4_GPIO18 ,
    A4_GPIO19   = 14,
    A0_GPIO0 ,  //port0
    A0_GPIO1 ,
    A0_GPIO2 ,
    A0_GPIO3 ,
    A0_GPIO4 ,
    A0_GPIO5 ,
    A0_GPIO6 ,
    A0_GPIO7 ,
    A0_GPIO8 ,
    A0_GPIO9 ,
    A0_GPIO10 ,
    A0_GPIO11 ,
    A0_GPIO12 ,
    A0_GPIO13 ,
    A0_GPIO14 ,
    A0_GPIO15 = 30,
    A0_GPIO16 , //port1
    A0_GPIO17 ,
    A0_GPIO18 ,
    A0_GPIO19 ,
    A0_GPIO29 ,
    A0_GPIO30 ,
    A0_GPIO31 = 37, 
    A0_GPIO32 , //port2
    A0_GPIO33 ,
    A0_GPIO34 ,
    A0_GPIO35 ,
    A0_GPIO36 ,
    A0_GPIO37 ,
    A0_GPIO38 = 44, 
    A0_GPIO20 , //port1
    A0_GPIO21 ,
    A0_GPIO22 ,
    A0_GPIO23 ,
    A0_GPIO24 ,
    A0_GPIO25 ,
    A0_GPIO26 ,
    A0_GPIO27 ,
    A0_GPIO28 = 53
} ;

#include <stdint.h>
#include "api_def.h"
#include "api_pad.h"
#include "api_gpio.h"
#include "api_uart.h"
#include "api_i2c.h"
#include "api_spi.h"
#include "api_pwm.h"
#include "api_wakeup.h"
#ifdef FEATURE_HAL_SCREEN_ENABLE
#include "api_lspi.h"
#include "api_scr.h"
#endif
#ifdef FEATURE_HAL_CAMERA_ENABLE
#include "api_cspi.h"
#include "api_cam.h"
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
api_ret_t open_hal_startup(HalType_t type, void* para, void* out);
int32_t open_hal_parse(char* str,uint32_t *output);	// output == NULL 则对解析得到的数据执行配置操作
api_ret_t open_hal_query(HalType_t type, uint32_t index);
api_ret_t open_hal_create(HalType_t type, uint32_t index, void *cfg, void *out);
api_ret_t open_hal_open(HalType_t type, uint32_t usrId, void *cfg, size_t timeout);
api_ret_t open_hal_close(HalType_t type, uint32_t usrId);
api_ret_t open_hal_delete(HalType_t type, uint32_t usrId);
api_ret_t open_hal_ioctl(HalType_t type, uint32_t usrId, uint32_t cmd, void *para);
api_ret_t open_hal_pmctl(HalType_t type, uint32_t usrId, open_hal_pm_t *cfg, size_t count);
api_ret_t open_hal_write(HalType_t type, uint32_t usrId, void* buf, size_t count);
api_ret_t open_hal_read(HalType_t type, uint32_t usrId, void* buf, size_t count);
int open_hal_api_test(HalType_t type);
extern bool GPR_clockEnableCheck(ClockId_e id);


#ifndef RTE_PAD_NUM_MAX
#define RTE_PAD_NUM_MAX		(43)
#endif
#ifndef RTE_GPIO_NUM_MAX
#define RTE_GPIO_NUM_MAX    (39)
#endif
#ifndef CSV_LIST_LEN_MAX
#define CSV_LIST_LEN_MAX    (32)
#endif
#ifndef CSV_PAGE_LEN_MAX
#define CSV_PAGE_LEN_MAX    (512)
#endif
#define CSV_LIST_STR_PAD    "paddr"
#define CSV_LIST_STR_GPIO   "gpio"
#define CSV_LIST_STR_I2C    "i2c"
#define CSV_LIST_STR_SPI    "spi"
#define CSV_LIST_STR_UART   "uart"
#define CSV_LIST_STR_PWM    "pwm"
#define CSV_LIST_STR_WAKE	"wakeup"

#ifdef __cplusplus
}
#endif
#endif /* _API_COMM_H_ */