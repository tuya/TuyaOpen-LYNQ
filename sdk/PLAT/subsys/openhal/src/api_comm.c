/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_comm.c
 * Description:  ec7xx openhal entry source file
 * History:      Rev1.0   2024-01-10
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <string.h> /* memset */
#include <stdlib.h> /* atoi */

#include "Driver_Common.h"
#include "system_ec7xx.h"
#include DEBUG_LOG_HEADER_FILE
#include "devicemanager.h"
#include "api_comm.h"

osEventFlagsId_t successEvent = NULL;       //传输成功输入
osEventFlagsId_t hwfaultEvent = NULL;       //硬件异常输入
osEventFlagsId_t exceptionEvt = NULL;       //硬件异常输出
static osThreadId_t exceptionHandle = NULL;

#define EPAT_LOG(subId, debugLevel, format, ...)  \
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, subId, debugLevel, format, ##__VA_ARGS__)
/**
  \fn          
  \brief    如果不可修改str则打印剩余所有内容    
  \return
*/
int open_hal_parse(char* str,void *output)
{
    if(str == NULL) return 0;
    char  *head   = NULL;
    char *line_str[16];
    int num = 0;
    head = strtok(str, "\n");
    while( head != NULL ) 
    {
        line_str[num] = head;
        head = strtok(NULL, "\n");  
        num ++; 
    }
    if (strstr(line_str[0], "paddr") != NULL){
        for (int i = 1; i < num; i++)
        {
            PadConfigLite_t liteConfig = {0};
            uint32_t index = api_pad_default(line_str[i],&liteConfig);
            if(index<EC_PAD_INDEX_LIMIT && output != NULL){
                *((uint8_t *)output+index) = *(uint8_t *)&liteConfig;
            }
        }
        // SYSLOG_INFO("output[%d] 0x%X,0x%X\r\n",num,*((uint8_t *)output+12),*((uint8_t *)output+13));
    }
    else if (strstr(line_str[0], "gpio") != NULL){
        for (int i = 1; i < num; i++)
        {
            gpioConfigLite_t liteConfig = {0};
            uint32_t index = api_gpio_default(line_str[i],&liteConfig);
            if(index<EC_GPIO_INDEX_LIMIT && output != NULL){
                *((uint8_t *)output+index) = *(uint8_t *)&liteConfig;
            }
        }
        // SYSLOG_INFO("output[%d] 0x%X,0x%X\r\n",num,*((uint8_t *)output+12),*((uint8_t *)output+13));
    }
    return num;
}
/**
  \fn          
  \brief        
  \return 
*/
int open_hal_startup(HalType_t tpye,void* para)
{
    int ret = OPEN_HAL_DONE;
    switch (tpye)
    {
        case HAL_PAD:
            ret = api_pad_startup(para);
            break;
        case HAL_GPIO:
            ret = api_gpio_startup(para);
            break;
        case HAL_UART:
            ret = api_uart_startup(para);
            break;
        case HAL_I2C:
            ret = api_i2c_startup(para);
            break;
        case DEV_TP:
            api_tp_startup(para);
            break;
        #ifdef FEATURE_HAL_SCREEN_ENABLE
        case DEV_SCR:
            api_scr_startup(para);
            break;
        #endif
        default:
            break;
    }
    return ret;
}

/**
  \fn          
  \brief        
  \return 
*/
api_ret_t open_hal_query(HalType_t tpye,uint32_t index)
{
    api_ret_t ret = OPEN_HAL_DONE;
    switch (tpye)
    {
        case HAL_PAD:
            ret = api_pad_query(index);
            break;
        case HAL_GPIO:
            ret = api_gpio_query(index);
            break;
        case HAL_UART:
            ret = api_uart_query(index);
            break;
        case HAL_I2C:
            ret = api_i2c_query(index);
            break;
        default:
            break;
    }
    return ret;
}

/**
  \fn          
  \brief        
  \return
*/
uint32_t open_hal_create(HalType_t tpye,uint32_t index,void *cfg)
{
    uint32_t usrId = 0;
    switch (tpye)
    {
        case HAL_PAD:
            usrId = api_pad_create(index,cfg);
            break;
        case HAL_GPIO:
            usrId = api_gpio_create(index,cfg);
            break;
        case HAL_UART:
            usrId = api_uart_create(index,cfg);
            break;
        case HAL_I2C:
            usrId = api_i2c_create(index,cfg);
            break;
        default:
            break;
    }
    return usrId;
}

/**
  \fn          
  \brief        
  \return
*/
int open_hal_open(HalType_t tpye,uint32_t index)
{
    switch (tpye)
    {
        // case HAL_PAD:
        //     usrId = api_pad_open(index,cfg);
        //     break;
        // case HAL_GPIO:
        //     usrId = api_gpio_create(index,cfg);
        //     break;
        // case HAL_UART:
        //     usrId = api_uart_create(index,cfg);
        //     break;
        // case HAL_I2C:
        //     usrId = api_i2c_create(index,cfg);
        //     break;
        // default:
        //     break;
    }
}

/**
  \fn          
  \brief        
  \return
*/
int open_hal_ioctl()
{

}

/**
  \fn          
  \brief        
  \return
*/
int open_hal_close()
{

}

/**
  \fn          
  \brief        
  \return
*/
int open_hal_delete()
{

}


/**
  \fn          
  \brief    
  \return
*/
int open_hal_api_test(HalType_t tpye)
{ 
    switch (tpye)
    {
        case HAL_PAD:
            api_test_pad();
        break;
        case HAL_GPIO:
            api_test_gpio();
        break;
        case HAL_UART:
            api_test_uart();
        break;
        case HAL_I2C:
            api_test_i2c();
        break;
        default:
            break;
    }
}
/**
  \fn
  \brief
  \return
*/
static void exceptionTask(void *argument)
{
    uint32_t flag = 0;
    while(1) {
        flag = osEventFlagsWait(hwfaultEvent, ((1U << FAULT_EVT_TOTAL) -1), osFlagsWaitAny, osWaitForever);
        if (flag < (1U << FAULT_EVT_TOTAL)) {
            if (flag & (1U << FAULT_LSPI_DMA_ERR)) {
                EPAT_LOG(exception_lspi_dma_err, P_INFO, "flag 0x%X", flag);
                osEventFlagsSet(exceptionEvt, (1U << EXCEPTION_LSPI_REINIT));
            }
            if (flag & (1U << FAULT_I2C0_BUS_ERR)) {
                EPAT_LOG(exception_i2c0_bus_err, P_INFO, "flag 0x%X", flag);
                osEventFlagsSet(exceptionEvt, (1U << EXCEPTION_I2C0_REINIT));
            }
            if (flag & (1U << FAULT_I2C1_BUS_ERR)) {
                EPAT_LOG(exception_i2c1_bus_err, P_INFO, "flag 0x%X", flag);
                osEventFlagsSet(exceptionEvt, (1U << EXCEPTION_I2C1_REINIT));
            }
        }
        osDelay(3);
    }
}
/**
  \fn
  \brief
  \return
*/
void *exceptionTaskInit(void)
{
    if(exceptionEvt == NULL){
        exceptionEvt = osEventFlagsNew(NULL); //output
    }
    if(hwfaultEvent == NULL){
        hwfaultEvent = osEventFlagsNew(NULL);   //input
    }
    if(successEvent == NULL){
        successEvent = osEventFlagsNew(NULL);   //input
    }
    for(HalType_t type=HAL_PAD;type<HAL_MAX;type++){
        open_hal_startup(type,NULL);
    }
    if(exceptionHandle == NULL){
        osThreadAttr_t exceptionTaskAttr = {
            .name = "halexpt",
            .stack_size = 1024,
            .priority = osPriorityNormal,
        };
        exceptionHandle = osThreadNew(exceptionTask, NULL, &exceptionTaskAttr);
    }
    return exceptionHandle;
}
