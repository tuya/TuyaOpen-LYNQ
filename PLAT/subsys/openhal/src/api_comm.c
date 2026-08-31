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
#include "api_wakeup.h"
#include "servicemanager.h"

osEventFlagsId_t successEvent = NULL;       //传输成功输入
osEventFlagsId_t hwfaultEvent = NULL;       //硬件异常输入
osEventFlagsId_t exceptionEvt = NULL;       //硬件异常输出
static osThreadId_t exceptionHandle = NULL;

#ifdef EPAT_HAL_DEBUG
#define EPAT_LOG(subId, debugLevel, format, ...)  \
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, subId, debugLevel, format, ##__VA_ARGS__)
#else
#define EPAT_LOG(subId, debugLevel, format, ...)
#endif
/* ---------------------------------------------------------------------------------------------- */
/**
  \fn          
  \brief
  \return
*/
int32_t open_hal_parse(char* str, uint32_t *output)
{
    if(str == NULL) return 0;
    char  *head   = NULL;
    char *line_str[CSV_LIST_LEN_MAX] = {0};  // 初始化数组
    int num = 0;
    head = strtok(str, "\n");
    while( head != NULL ) 
    {
        line_str[num] = head;
        head = strtok(NULL, "\n");  
        num ++; 
    }
    if (strstr(line_str[0], CSV_LIST_STR_PAD) != NULL)
    {
        for (uint8_t i = 1; i < num; i++)
        {
            pad_config_t Config = {0};
            int32_t index = api_pad_parse(line_str[i],&Config);
            if(index <= 0){
                EPAT_LOG(open_hal_parse_pad, P_ERROR, "parse line %d,ret %d", i, index);
            }
            else {
                if(output != NULL)  // 执行数据导出
                {
                    *((pad_config_t *)output+index) = *(pad_config_t *)&Config;
                    // EPAT_LOG(open_hal_parse_pad, P_INFO, "%d:line %d,Config %x", index, i, Config);
                }
                else    // 执行数据配置
                {
                    api_pad_setup(index, &Config);
                }
            }
        }
    }
    else if (strstr(line_str[0], CSV_LIST_STR_GPIO) != NULL){
        for (uint8_t i = 1; i < num; i++)
        {
            gpio_config_t Config = {0};
            int32_t index = api_gpio_parse(line_str[i],&Config);
            if(index <= 0)  // 执行错误
            {
                EPAT_LOG(open_hal_parse_gpio, P_ERROR, "parse line %d,ret %d", i, index);
            }
            else {
                if(output != NULL)  // 执行数据导出
                {
                    *((gpio_config_t *)output+index) = *(gpio_config_t *)&Config;
                }
                else    // 执行数据配置
                {
                    api_gpio_setup(index, &Config);
                }
            }
        }
    }
    else if (strstr(line_str[0], CSV_LIST_STR_I2C) != NULL)
    {
        for (uint8_t i = 1; i < num; i++)
        {
            i2c_config_t Config = {0};
            int32_t index = api_i2c_parse(line_str[i], &Config);
            if(index <= 0)  // 执行错误
            {
                EPAT_LOG(open_hal_parse_i2c, P_ERROR, "parse line %d,ret %d", i, index);
            }
            else {
                if(output != NULL)  // 执行数据导出
                {
                    *((i2c_config_t *)output+index) = *(i2c_config_t *)&Config;
                }
                else    // 执行数据配置
                {
                    api_i2c_setup(index, &Config);
                }
            }
        }
    }
    else if (strstr(line_str[0], CSV_LIST_STR_UART) != NULL)
    {
        for (uint8_t i = 1; i < num; i++)
        {
            uart_config_t Config = {0};
            int32_t index = api_uart_parse(line_str[i], &Config);
            if(index <= 0)  // 执行错误
            {
                EPAT_LOG(open_hal_parse_uart, P_ERROR, "parse line %d,ret %d", i, index);
            }
            else {
                if(output != NULL)  // 执行数据导出
                {
                    *((uart_config_t *)output+index) = *(uart_config_t *)&Config;
                }
                else    // 执行数据配置
                {
                    api_uart_setup(index, &Config);
                }
            }
        }
    }
    else if (strstr(line_str[0], CSV_LIST_STR_SPI) != NULL)
    {
        for (uint8_t i = 1; i < num; i++)
        {
            spi_config_t Config = {0};
            int32_t index = api_spi_parse(line_str[i], &Config);
            if(index <= 0)  // 执行出现错误
            {
                EPAT_LOG(open_hal_parse_spi, P_ERROR, "parse line %d,ret %d", i, index);
            }
            else {
                if(output != NULL)  // 执行数据导出
                {
                    *((spi_config_t *)output+index) = *(spi_config_t *)&Config;
                }
                else    // 执行数据配置
                {
                    api_spi_setup(index, &Config);
                }
            }
        }
    }
    else if (strstr(line_str[0], CSV_LIST_STR_PWM) != NULL)
    {
        for (uint8_t i = 1; i < num; i++)
        {
            pwm_config_t Config = {0};
            int32_t index = api_pwm_parse(line_str[i], &Config);
            if(index <= 0)  // 执行出现错误
            {
                EPAT_LOG(open_hal_parse_pwm, P_ERROR, "parse line %d,ret %d", i, index);
            }
            else {
                if(output != NULL)  // 执行数据导出
                {
                    *((pwm_config_t *)output+index) = *(pwm_config_t *)&Config;
                }
                else    // 执行数据配置
                {
                    api_pwm_setup(index, &Config);
                }
            }
        }
    }
    else if (strstr(line_str[0], CSV_LIST_STR_WAKE) != NULL)
    {
        for (uint8_t i = 1; i < num; i++)
        {
            wakeup_config_t Config = {0};
            int32_t index = api_wakeup_parse(line_str[i], &Config);
            if(index <= 0)  // 执行出现错误
            {
                EPAT_LOG(open_hal_parse_wakeup, P_ERROR, "parse line %d,ret %d", i, index);
            }
            else {
                if(output != NULL)  // 执行数据导出
                {
                    *((wakeup_config_t *)output+index) = *(wakeup_config_t *)&Config;
                }
                else    // 执行数据配置
                {
                    api_wakeup_setup(index, &Config);
                }
            }
        }
    }
    return (num-1);
}

/**
  \fn          
  \brief        
  \return 
*/
// extern int8_t padList[RTE_PAD_NUM_MAX][4];
static int8_t *s_pad_List = NULL;
static int8_t *s_pin_List = NULL;
static int8_t *s_uart_List = NULL;
static int8_t *s_i2c_List = NULL;
static int8_t *s_spi_List = NULL;
static int8_t *s_pwm_List = NULL;
api_ret_t open_hal_startup(HalType_t type, void* para, void* out)
{
    api_ret_t ret = OPEN_HAL_NONE;
    switch (type)
    {
        case HAL_PAD:
            s_pad_List = api_pad_startup(para);
            if(out != NULL)
            {
                out = (void*)s_pad_List;
            }
            ret = OPEN_HAL_DONE;
            break;
        case HAL_GPIO:
            s_pin_List = api_gpio_startup(para, s_pad_List);
            if(out != NULL)
            {
                out = (void*)s_pin_List;
            }
            ret = OPEN_HAL_DONE;
            break;
        case HAL_UART:
            s_uart_List = api_uart_startup(para, s_pad_List);
            if(out != NULL)
            {
                out = (void*)s_uart_List;
            }
            ret = OPEN_HAL_DONE;
            break;
        case HAL_I2C:
            s_i2c_List = api_i2c_startup(para, s_pad_List);
            if(out != NULL)
            {
                out = (void*)s_i2c_List;
            }
            ret = OPEN_HAL_DONE;
            break;
        case HAL_SPI:
            s_spi_List = api_spi_startup(para, s_pad_List);
            if(out != NULL)
            {
                out = (void*)s_spi_List;
            }
            ret = OPEN_HAL_DONE;
            break;
        case HAL_PWM:
            s_pwm_List = api_pwm_startup(para, s_pad_List);
            if(out != NULL)
            {
                out = (void*)s_pwm_List;
            }
            ret = OPEN_HAL_DONE;
            break;
        case HAL_WAKEUP:
            ret = api_wakeup_startup(para);
            break;
#ifdef FEATURE_HAL_CAM_ENABLE
        case HAL_CSPI:
            ret = api_cspi_startup(para);
            break;
#endif
#ifdef FEATURE_HAL_SCREEN_ENABLE
        case HAL_LSPI:
            ret = api_lspi_startup(para);
            break;
#endif
        default:
            ret = OPEN_HAL_INVALID_PARA ;
            break;
    }
    return ret;
}

/**
  \fn          
  \brief        
  \return 
*/
api_ret_t open_hal_query(HalType_t type,uint32_t index)
{
    api_ret_t ret = OPEN_HAL_DONE;
    switch (type)
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
        case HAL_SPI:
            ret = api_spi_query(index);
            break;
        case HAL_PWM:
            ret = api_pwm_query(index);
            break;
        // case HAL_LSPI:
        //     ret = api_lspi_query(index);
        //     break;
        // case HAL_CSPI:
        //     ret = api_cspi_query(index);
        //     break;
        default:
            break;
    }
    return ret;
}
/* ---------------------------------------------------------------------------------------------- */
/**
  \fn          
  \brief        
  \return
*/
api_ret_t open_hal_create(HalType_t type, uint32_t index, void *cfg, void *out)
{
    uint32_t usrId = 0;
    api_ret_t ret = OPEN_HAL_NONE;
    switch (type)
    {
        case HAL_PAD:
            ret = api_pad_create(index, &usrId);
            break;
        case HAL_GPIO:
            ret = api_gpio_create(index,cfg, &usrId);   
            break;
        case HAL_UART:
            ret = api_uart_create(index, (uart_config_t *)cfg, &usrId);
            break;
        case HAL_I2C:
            ret = api_i2c_create(index, cfg, &usrId);
            break;
        case HAL_SPI:
            ret = api_spi_create(index, cfg, &usrId);
            break;
        case HAL_PWM:
            ret = api_pwm_create(index, cfg, &usrId);
            break;
        default:
            break;
    }
    if(out != NULL)
    {
        *(uint32_t *)out = usrId;
    }
    return ret;
}

/**
  \fn          
  \brief        
  \return
*/
api_ret_t open_hal_open(HalType_t type, uint32_t usrId, void *cfg, size_t timeout)
{
    api_ret_t ret = OPEN_HAL_DONE;
    switch (type)
    {
        case HAL_PAD:
            ret = api_pad_open(usrId, cfg, timeout);
            break;
        case HAL_GPIO:
            ret = api_gpio_open(usrId, cfg, timeout);
            break;
        case HAL_UART:
            ret = api_uart_open(usrId, cfg, timeout);
            break;
        case HAL_I2C:
            ret = api_i2c_open(usrId, cfg, timeout);
            break;
        case HAL_SPI:
            ret = api_spi_open(usrId, cfg, timeout);
            break;
        case HAL_PWM:
            ret = api_pwm_open(usrId, cfg, timeout);
            break;
        // case HAL_LSPI:
        //     ret = api_lspi_open(usrId, cfg, timeout);
        //     break;
        // case HAL_CSPI:
        //     ret = api_cspi_open(usrId, cfg, timeout);
        //     break;
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
api_ret_t open_hal_close(HalType_t type, uint32_t usrId)
{
    api_ret_t ret = OPEN_HAL_DONE;
    switch (type)
    {
        case HAL_PAD:
            ret = api_pad_close(usrId);
            break;
        case HAL_GPIO:
            ret = api_gpio_close(usrId);
            break;
        case HAL_UART:
            ret = api_uart_close(usrId);
            break;
        case HAL_I2C:
            ret = api_i2c_close(usrId);
            break;
        case HAL_SPI:
            ret = api_spi_close(usrId);
            break;
        case HAL_PWM:
            ret = api_pwm_close(usrId);
            break;
        // case HAL_LSPI:
        //     ret = api_lspi_close(usrId);
        //     break;
        // case HAL_CSPI:
        //     ret = api_cspi_close(usrId);
        //     break;
        // case HAL_APWM:
        //     ret = api_apwm_close(usrId);
        //     break;
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
api_ret_t open_hal_delete(HalType_t type, uint32_t usrId)
{
    api_ret_t ret = OPEN_HAL_DONE;
    switch (type)
    {
        case HAL_PAD:
            ret = api_pad_delete(usrId);
            break;
        case HAL_GPIO:
            ret = api_gpio_delete(usrId);
            break;
        case HAL_UART:
            ret = api_uart_delete(usrId);
            break;
        case HAL_I2C:
            ret = api_i2c_delete(usrId);
            break;
        case HAL_SPI:
            ret = api_spi_delete(usrId);
            break;
        // case HAL_LSPI:
        //     ret = api_lspi_delete(usrId);
        //     break;
        // case HAL_CSPI:
        //     ret = api_cspi_delete(usrId);
        //     break;
        case HAL_PWM:
            ret = api_pwm_delete(usrId);
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
api_ret_t open_hal_ioctl(HalType_t type, uint32_t usrId, uint32_t cmd, void *para)
{
    api_ret_t ret = OPEN_HAL_DONE;
    switch (type)
    {
        case HAL_PAD:
            ret = api_pad_ioctl(usrId, cmd, para);
            break;
        case HAL_GPIO:
            ret = api_gpio_ioctl(usrId, cmd, para);
            break;
        case HAL_UART:
            ret = api_uart_ioctl(usrId, cmd, para);
            break;
        case HAL_I2C:
            ret = api_i2c_ioctl(usrId, cmd, para);
            break;
        case HAL_SPI:
            ret = api_spi_ioctl(usrId, cmd, para);
            break;
        // case HAL_LSPI:
        //     ret = api_lspi_ioctl(usrId, cmd, para);
        //     break;
        // case HAL_CSPI:
        //     ret = api_cspi_ioctl(usrId, cmd, para);
        //     break;
        case HAL_PWM:
            ret = api_pwm_ioctl(usrId, cmd, para);
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
api_ret_t open_hal_pmctl(HalType_t type, uint32_t usrId, open_hal_pm_t *cfg, size_t count)
{
    api_ret_t ret = OPEN_HAL_DONE;
    switch (type)
    {
        case HAL_PAD:
            ret = api_pad_pmctl(usrId, cfg, count);
            break;
        case HAL_GPIO:
            ret = api_gpio_pmctl(usrId, cfg, count);
            break;
        case HAL_UART:
            ret = api_uart_pmctl(usrId, cfg, count);
            break;
        case HAL_I2C:
            ret = api_i2c_pmctl(usrId, cfg, count);
            break;
        case HAL_SPI:
            ret = api_spi_pmctl(usrId, cfg, count);
            break;
        // case HAL_LSPI:
        //     ret = api_lspi_pmctl(usrId, cfg, count);
        //     break;
        // case HAL_CSPI:
        //     ret = api_cspi_pmctl(usrId, cfg, count);
        //     break;
        case HAL_PWM:
            ret = api_pwm_pmctl(usrId, cfg, count);
            break;
        default:
            break;
    }
    return ret;
}
/* ---------------------------------------------------------------------------------------------- */
/**
  \fn          
  \brief
  \return
*/
api_ret_t open_hal_write(HalType_t type, uint32_t usrId, void* buf, size_t count)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = OPEN_HAL_INVALID_PARA;
    
    switch (type)
    {
        case HAL_UART:
            ret = api_uart_write(usrId, buf, count);
            break;
        case HAL_SPI:
            ret = api_spi_write(usrId, buf, count);
            break;
        // case HAL_LSPI:
        //     ret = api_lspi_write(usrId, buf, count);
        //     break;
        // case HAL_CSPI:
        //     ret = api_cspi_write(usrId, buf, count);
        //     break;
        // case HAL_I2C:
        //     ret = api_i2c_write(usrId, buf, count);
        //     break;
        case HAL_PWM:
            {
                ret = api_pwm_write(usrId, buf, count);
            }
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
api_ret_t open_hal_read(HalType_t type, uint32_t usrId, void* buf, size_t count)
{
    api_ret_t ret = OPEN_HAL_INVALID_PARA;
    
    switch (type)
    {
        case HAL_UART:
            ret = api_uart_read(usrId, buf, count);
            break;
        case HAL_SPI:
            ret = api_spi_read(usrId, buf, count);
            break;
        // case HAL_LSPI:
        //     ret = api_lspi_read(usrId, buf, count);
        //     break;
        // case HAL_CSPI:
        //     ret = api_cspi_read(usrId, buf, count);
        //     break;
        // case HAL_I2C:
        //     ret = api_i2c_read(usrId, buf, count);
        //     break;
        case HAL_PWM:
            {
                ret = api_pwm_read(usrId, buf, count);
            }
            break;
        default:
            break;
    }
    
    return ret;
}

/* ---------------------------------------------------------------------------------------------- */
/**
  \fn          
  \brief    
  \return
*/
int open_hal_api_test(HalType_t type)
{ 
    int ret = OPEN_HAL_DONE;
    switch (type)
    {
        case HAL_PAD:
            ret = api_test_pad();
            break;
        case HAL_GPIO:
            ret = api_test_gpio();
            break;
        case HAL_UART:
            ret = api_test_uart();
            break;
        case HAL_I2C:
            ret = api_test_i2c();
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
    if(exceptionHandle == NULL){
        osThreadAttr_t exceptionTaskAttr = {
            .name = "halexpt",
            .stack_size = 1024,
            .priority = osPriorityNormal,
        };
#if 0
        exceptionHandle = osThreadNew(exceptionTask, NULL, &exceptionTaskAttr);
#else
        char serviceName[32] = {0};
        snprintf(serviceName, sizeof(serviceName), "service:/%s", exceptionTaskAttr.name);
        Service_reg(serviceName, exceptionTask, NULL, exceptionTaskAttr.cb_mem, exceptionTaskAttr.cb_size, exceptionTaskAttr.stack_mem, exceptionTaskAttr.stack_size, exceptionTaskAttr.priority);
        exceptionHandle = (osThreadId_t)Service_start(serviceName);
#endif
    }
    return exceptionHandle;
}