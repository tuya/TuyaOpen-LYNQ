/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    opensdk misc.c
 * Description:  EC718 lwm2m demo entry source file
 * History:      Rev1.0   2023-8-7
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_OPENSDK_ENABLE
#include <stdio.h>

#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "bsp_custom.h"
#include "event_groups.h"
#include "osasys.h"
#include "ostask.h"
#include "ps_lib_api.h"
#include "cmisim.h"
#include "cmips.h"
#include "networkmgr.h"
#include "slpman.h"
#include "bsp.h"
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#ifdef FEATURE_SUBSYS_AUDIO_ENABLE
#include "audio.h"
#endif

#include "opensdk.h"

extern ARM_DRIVER_USART Driver_USART0;
extern ARM_DRIVER_USART Driver_USART1;


typedef void (*DDK_CB_FUNC)(void *argument,uint32_t event);




#if (RTE_UART2)
extern ARM_DRIVER_USART Driver_USART2;
static void *   uart2_lists = NULL;
static DDK_CB_FUNC  uart2_cb = NULL;
static void uart2_callback(uint32_t event)
{
    if(uart2_lists){
        uart2_cb(uart2_lists,event);
    }
}
#endif

#if (RTE_UART3)
extern ARM_DRIVER_USART Driver_USART3;
static void *   uart3_lists = NULL;
static DDK_CB_FUNC  uart3_cb = NULL;
static void uart3_callback(uint32_t event)
{
    if(uart3_lists){
        uart3_cb(uart3_lists,event);
    }
}
#endif

ARM_DRIVER_USART *uart_driver_get(uint8_t symbol,void *argument, uint32_t callback)
{
    ARM_DRIVER_USART * ret = NULL;
    // APP_TRACE(UART_INIT, 3, "%d,0x%X,0x%X",symbol,argument,callback);
    #if (RTE_UART2)
    if(symbol==2) 
    {
        ret = &CREATE_SYMBOL(Driver_USART, 2);
        uart2_lists = argument;
        uart2_cb = (DDK_CB_FUNC)callback;
        if(uart2_cb){
            ret->Initialize(uart2_callback);
        } 
    }
    #endif
    #if (RTE_UART3)
    if(symbol==3) 
    {
        ret = &CREATE_SYMBOL(Driver_USART, 3);
        uart3_lists = argument;
        uart3_cb = (DDK_CB_FUNC)callback;   
        if(uart3_cb){
            ret->Initialize(uart3_callback);
        } 
    }
    #endif
    return ret;
}
#if (RTE_I2C0)
extern ARM_DRIVER_I2C   Driver_I2C0;
static uint32_t         i2c0_lists = 0;
static DDK_CB_FUNC      i2c0_cb = NULL;
static void i2c0_callback(uint32_t event)
{
    if(i2c0_lists && i2c0_cb){
        i2c0_cb((void *)i2c0_lists,event);
    }
}
#endif
#if (RTE_I2C1)
extern ARM_DRIVER_I2C   Driver_I2C1;
static uint32_t         i2c1_lists = 0;
static DDK_CB_FUNC      i2c1_cb = NULL;
static void i2c1_callback(uint32_t event)
{
    if(i2c1_lists){
        i2c1_cb((void *)i2c1_lists,event);
    }
}
#endif
ARM_DRIVER_I2C *i2c_driver_get(uint8_t symbol,uint32_t calltable, uint32_t callback)
{
    ARM_DRIVER_I2C * ret = NULL;
    // APP_TRACE(I2C_INIT, 3, "%d,0x%X,0x%X",symbol,calltable,callback);
    #if (RTE_I2C0)
    if(symbol==0) 
    {
        ret = &CREATE_SYMBOL(Driver_I2C, 0);
        if(calltable && callback){
            i2c0_lists = calltable;
            i2c0_cb = (DDK_CB_FUNC)callback;
            ret->Initialize(i2c0_callback);
        } 
        else ret->Initialize(NULL);
    }
    #endif
    #if (RTE_I2C1)
    if(symbol==1) 
    {
        ret = &CREATE_SYMBOL(Driver_I2C, 1);
        if(calltable && callback){
            i2c1_lists = calltable;
            i2c1_cb = (DDK_CB_FUNC)callback;
            ret->Initialize(i2c1_callback);
        }
        else ret->Initialize(NULL);
    }
    #endif
    return ret;
}

ARM_DRIVER_SPI *spi_driver_get(uint8_t symbol,uint32_t calltable, uint32_t callback)
{
#if (RTE_SPI0)
extern ARM_DRIVER_SPI   Driver_SPI0;
    if(symbol==0) return &CREATE_SYMBOL(Driver_SPI, 0);
#endif
#if (RTE_SPI1)
extern ARM_DRIVER_SPI   Driver_SPI1;
    if(symbol==1) return &CREATE_SYMBOL(Driver_SPI, 1);
#endif
    return NULL;
}


#endif
