/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_gpio.c
 * Description:  ec7xx openhal gpio entry source file
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

#include "clock.h"
#include "bsp.h"
#include "mem_map.h"
#include "clock.h"
#include "slpman.h"
#include "cms_api.h"
#include "timer.h"
#include "pad.h"
#include "gpio.h"
#include "ic.h"
#include "devicemanager.h"

#include "api_comm.h"
#if defined CHIP_EC716
#else   //718
const int8_t list_gpio2pad[] = {
    15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,
    11,12,13,14,
    45,46,47,48,49,50,51,52,53,                             //9xAON
    35,36,37,38,39,40,41,42,43,44
};
const int8_t list_pad2gpio[] = {
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,           //<15
    0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,      //<35
    29,30,31,32,33,34,35,36,37,38,
    20,21,22,23,24,25,26,27,28
};
#endif

//此表后续会移至特定的FLASH存储区域
uint8_t gpioStartupList[EC_GPIO_INDEX_LIMIT] = {
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x40, 0x0, //20-29
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,  
};
static uint8_t sGpioRuntimeList[EC_GPIO_INDEX_LIMIT] = {0xFF};
//D0-D7:index
//D8-D15:stat
//D16-D31:seed
static uint32_t sGpioUsrIdList[EC_GPIO_INDEX_LIMIT] = {0};
static uint16_t sGpioUsrIdSeed[EC_GPIO_INDEX_LIMIT] = {0};
// static IsrFunc sGpioIsrFunc[EC_GPIO_INDEX_LIMIT] = {NULL};
static uint16_t portMusk[3] = {0};
static IsrFunc gpio_isr_cb[3][16] = {NULL};
#ifdef FEATURE_OPENHAL_RTOS_ENABLE
#define EC_PAD_EVT_INDEX_MID        (EC_GPIO_INDEX_START+24)
static osEventFlagsId_t osGpioBitList0_23 = NULL;  
static osEventFlagsId_t osGpioBitList24_38 = NULL;  
#endif
#define EPAT_LOG(subId, debugLevel, format, ...)  \
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, subId, debugLevel, format, ##__VA_ARGS__)
/**
  \fn          
  \brief   usrID生成方式逆向算法  
  \return
*/
static uint32_t usrId_to_gpio(uint32_t usrId)
{ 
    ASSERT(usrId > 0);
    uint32_t gpio = (uint32_t)(usrId & OPEN_HAL_PORT_MUSK); //位提取
    ASSERT(gpio < EC_GPIO_INDEX_LIMIT);
    if(gpio == (sGpioUsrIdList[gpio] & OPEN_HAL_PORT_MUSK))
    {
        return gpio;
    }
    return EC_GPIO_INDEX_LIMIT;
}
/**
  \fn          
  \brief     
  \return
*/
static void gpioIsrSet(int8_t gpio,IsrFunc cb)
{
    int8_t port = (int8_t)(gpio/16);
    int8_t pin = (int8_t)(gpio%16);
    if(cb != NULL){
        portMusk[port] |= (1<<pin);
    }
    else {
        portMusk[port] &= ~(1<<pin);
    }
    gpio_isr_cb[port][pin] = cb;
}
/**
  \fn          
  \brief     
  \return
*/
uint8_t gpio_cfg_conv(GpioPinConfig_t *pinConfig,gpioConfigLite_t *val)
{
    uint8_t ret = 0;
    if(pinConfig==NULL)
    {
        if(val!=NULL) ret = *(uint8_t*)val;
    }
    else if(val==NULL)   
    {
        gpioConfigLite_t liteConfig = {0};
        liteConfig.Direction = pinConfig->pinDirection;
        liteConfig.Default = pinConfig->misc.initOutput;
        liteConfig.Interrupt = pinConfig->misc.interruptConfig;
        ret = *(uint8_t *)&liteConfig;
    }
    else if(val->Interrupt < 6U)  
    {
        pinConfig->pinDirection = val->Direction;
        pinConfig->misc.interruptConfig = val->Interrupt;
        pinConfig->misc.initOutput = val->Default;
        ret = *(uint8_t*)val;
    }
    return ret;
}
/**
  \fn          
  \brief      查询gpio状态
  \return     状态值
*/
api_ret_t api_gpio_query(uint32_t usrId)
{
    // ASSERT(usrId > 0);
    uint32_t index = usrId_to_gpio(usrId);
    // ASSERT(gpio < EC_GPIO_INDEX_LIMIT);
    // printf("\r\nusrId[x%08X]:%d < %d",usrId,gpio,EC_GPIO_INDEX_LIMIT); 
    if(index >= EC_GPIO_INDEX_START && index < EC_GPIO_INDEX_LIMIT)
    {
        if(sGpioUsrIdList[index] == OPEN_HAL_STAT_UNUSED)
        {
            return OPEN_HAL_FREE;
        }
        else if(sGpioUsrIdList[index] & OPEN_HAL_STAT_MUSK)
        {
            return OPEN_HAL_USED;
        }
        else{
            return OPEN_HAL_IDLE;
        }
    }
    return OPEN_HAL_NONE;
}
/**
  \fn          
  \brief    未create状态,delete调用
  \return
*/
static uint32_t gpio_set_free(uint32_t index,gpioConfigLite_t *cfg)
{
    // ASSERT(index < EC_GPIO_INDEX_LIMIT);
    GpioPinConfig_t pinConfig = {0};
    if(index >= EC_GPIO_INDEX_START && index < EC_GPIO_INDEX_LIMIT)
    {
        sGpioRuntimeList[index] = gpio_cfg_conv(&pinConfig,cfg);
        GPIO_pinConfig(index/16, index%16, &pinConfig); 
        sGpioUsrIdList[index] = OPEN_HAL_STAT_UNUSED;
        uint8_t pin=index%16;
        uint8_t port=index/16;
        gpio_isr_cb[port][pin] = NULL; 
        // printf("\r\ngpio%d free[x%X,x%X]:0x%08X",index,ret,gpioStartupList[index],*(uint32_t *)&pinConfig); 
        return 1;  
    }
    return 0;
}
/**
  \fn          
  \brief    已创建，未使用状态   
  \return
*/
static uint32_t gpio_set_idle(uint32_t index)
{
    ASSERT(index < EC_GPIO_INDEX_LIMIT);
    #ifdef FEATURE_OPENHAL_RTOS_ENABLE
    #endif
    if(sGpioUsrIdList[index] == OPEN_HAL_STAT_UNUSED){
        sGpioUsrIdList[index] = index;
        sGpioUsrIdSeed[index] ++;
        sGpioUsrIdList[index] |= (uint32_t)(sGpioUsrIdSeed[index] << 16);
        // printf("\r\ngpio_set_idle[x%X]:0x%08X",index,sGpioUsrIdList[index]);
        return sGpioUsrIdList[index];   
    }
    else if(sGpioUsrIdList[index] & OPEN_HAL_STAT_MUSK){
        sGpioUsrIdList[index] &= ~(OPEN_HAL_STAT_MUSK);
        return sGpioUsrIdList[index]; 
    }
    return 0;
}
/**
  \fn          
  \brief    已创建，已使用状态   
  \return
*/
static uint32_t gpio_set_used(uint32_t usrId)
{
    api_ret_t stat = api_gpio_query(usrId);
    if(stat != OPEN_HAL_IDLE){
        return 0;
    }
    uint32_t index = usrId_to_gpio(usrId);
    // ASSERT(index < EC_GPIO_INDEX_LIMIT);
    sGpioUsrIdList[index] |= OPEN_HAL_STAT_MUSK;
    #ifdef FEATURE_OPENHAL_RTOS_ENABLE
    #endif
    return 0;
}
/**
  \fn
  \brief    gpio_isr_cb input para active flags
  \return
*/
static void api_gpio_isr_handle(void)
{
    for(uint8_t port=0; port<3; port++) 
    {
        uint16_t gpioMask = GPIO_saveAndSetIrqMask(port);
        uint16_t gpioFlag = GPIO_getInterruptFlags(port);
        uint16_t actflags = gpioFlag & portMusk[port];
        // EPAT_LOG(api_gpio_isr_handle, P_INFO, "portMusk 0x%X,0x%X,0x%X",portMusk[port],gpioFlag,actflags);
        if(actflags){
            for(uint8_t pin=0; pin<16; pin++) 
            {
                if((actflags & (1 << pin))){
                    GPIO_clearInterruptFlags(port,(1 << pin));
                    // uint32_t value = (uint32_t *) gpio_isr_cb[port][pin];
                    // EPAT_LOG(gpioIsrCB, P_INFO, "port%d,pin%d,0x%x",port,pin,value-1);
                    if(gpio_isr_cb[port][pin]!=NULL){
                        gpio_isr_cb[port][pin](actflags);
                    }
                }
            }
        }
        GPIO_restoreIrqMask(port, gpioMask);
    }
}
/**
  \fn
  \brief
  \return
*/
#if (UI_WATCH_USED == 1)
extern void tpGpioCallback(void);
static void api_gpio_isr_debug(void)
{
    uint16_t portIrqMask = GPIO_saveAndSetIrqMask(TP_IRQ_GPIO_INSTANCE);
    if (GPIO_getInterruptFlags(TP_IRQ_GPIO_INSTANCE) & (1U << TP_IRQ_GPIO_PIN))
    {
        tpGpioCallback();
        GPIO_clearInterruptFlags(TP_IRQ_GPIO_INSTANCE, 1U << TP_IRQ_GPIO_PIN);
    }
    GPIO_restoreIrqMask(TP_IRQ_GPIO_INSTANCE, portIrqMask);
}
#endif
/**
  \fn          
  \brief     所有GPIO上电初始化为配置状态  
  \return
*/
int api_gpio_startup(void* para)
{
    for(int i=EC_GPIO_INDEX_START;i<EC_GPIO_INDEX_LIMIT;i++)
    {
        if(para!=NULL) gpio_set_free(i,(gpioConfigLite_t *)para+i);
        else gpio_set_free(i,&gpioStartupList[i]);
    }
    for(uint8_t port=0; port<3; port++) {
        portMusk[port] = 0;
        for(uint8_t pin=0; pin<16; pin++) {
            gpio_isr_cb[port][pin] = NULL;
        }
    }
    #if (UI_WATCH_USED == 1)
    XIC_SetVector(PXIC1_GPIO_IRQn, api_gpio_isr_debug);
    #else
    XIC_SetVector(PXIC1_GPIO_IRQn, api_gpio_isr_handle);
    #endif
    
    XIC_EnableIRQ(PXIC1_GPIO_IRQn);
    #ifdef FEATURE_OPENHAL_RTOS_ENABLE
    osGpioBitList0_23 = osEventFlagsNew(NULL);
    osGpioBitList24_38 = osEventFlagsNew(NULL);
    #endif
}

/**
  \fn          
  \brief     
  \return
*/
uint32_t api_gpio_default(char* str,gpioConfigLite_t *cfg)
{
    if(str == NULL) return 0;
    char  *head   = NULL;
    char *item_str[7]; 
    int num = 0;  
    head = strtok(str, ",");
    while( head != NULL && num<7) 
    {
        item_str[num] = head;
        head = strtok(NULL, ",");
        num ++; 
    }
    uint32_t index = atoi(item_str[0]);
    uint8_t value = atoi(item_str[1]);
    if(value) {
        cfg->Locked = 1;
    }
    else cfg->Locked = 0;
    value = atoi(item_str[2]);
    if(value) {
        cfg->Direction = 1;
    }
    else cfg->Direction = 0;
    value = atoi(item_str[3]);
    if(value) {
        cfg->Default = 1;
    }
    else cfg->Default = 0;
    value = atoi(item_str[4]);
    if(value) {
        cfg->Active = 1;
    }
    else cfg->Active = 0;
    value = atoi(item_str[5]);
    if(value < 7U) {
        cfg->Interrupt = value;
    }
    else cfg->Interrupt = 0;
    SYSLOG_INFO("[%d][0x%02X]\r\n",index,*(uint8_t *)cfg);
    return index;
}

/**
  \fn          
  \brief    gpio创建，index 为 gpio0-gpio38 对应编号 0-38
  \return   usrId
*/
uint32_t api_gpio_create(uint32_t index,void *cfg)
{
    // ASSERT(index < EC_GPIO_INDEX_LIMIT);
    if(sGpioRuntimeList[index] & 0x80) return 0;
    uint32_t usrId = gpio_set_idle(index);
    if(usrId)
    {
        if(cfg != NULL)
        {
            GpioPinConfig_t *pinConfig = (GpioPinConfig_t *)cfg;
            if(pinConfig->pinDirection<2U && pinConfig->misc.interruptConfig<6U){
                sGpioRuntimeList[index] = gpio_cfg_conv(pinConfig,NULL);
                GPIO_pinConfig(index/16, index%16, pinConfig);
            } 
        } 
    }
    // SYSLOG_INFO("[%d][0x%08X]\r\n",index,usrId);
    return usrId;
}

/**
  \fn          
  \brief        
  \return 
*/
api_ret_t api_gpio_delete(uint32_t usrId)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_gpio_query(usrId);
    uint32_t index = usrId_to_gpio(usrId);
    // ASSERT(index < EC_GPIO_INDEX_LIMIT);
    if(ret == OPEN_HAL_IDLE)
    {
        if(gpio_set_free(index,&gpioStartupList[index]))
        {
            
        }
        ret = OPEN_HAL_DONE;
    }
    // SYSLOG_INFO("[%d][0x%08X]-%d\r\n",index,usrId,ret);
    return ret;
}


/**
  \fn          
  \brief        
  \return
*/
api_ret_t api_gpio_open(uint32_t usrId,void *cfg,size_t timeout)
{
    ASSERT(usrId > 0);
    api_ret_t ret = api_gpio_query(usrId);
    uint32_t index = usrId_to_gpio(usrId);
    // ASSERT(index < EC_GPIO_INDEX_LIMIT);
    if(ret == OPEN_HAL_IDLE)
    {
        if(cfg != NULL)
        {
            GpioPinConfig_t *pinConfig = (GpioPinConfig_t *)cfg;
            if(pinConfig->pinDirection<2U && pinConfig->misc.interruptConfig<6U){
                sGpioRuntimeList[index] = gpio_cfg_conv(pinConfig,NULL);
                GPIO_pinConfig(index/16, index%16, pinConfig);
            } 
        } 
        gpio_set_used(usrId);
        ret = OPEN_HAL_DONE;
        // printf("\r\ngpio:%d,0x%08X,0x%X",index,sGpioUsrIdList[index],*(uint32_t *)config);
    }
    // SYSLOG_INFO("[%d][0x%08X]-%d\r\n",index,usrId,ret);
    return ret;
}

/**
  \fn          
  \brief        
  \return
*/
api_ret_t api_gpio_close(uint32_t usrId)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_gpio_query(usrId);
    uint32_t index = usrId_to_gpio(usrId);
    // ASSERT(index < EC_GPIO_INDEX_LIMIT);
    if(ret == OPEN_HAL_USED)
    {  
        gpio_set_idle(index);
        ret = OPEN_HAL_DONE;
    }
    // SYSLOG_INFO("[%d][0x%08X]-%d\r\n",index,usrId,ret);
    return ret;
}

/**
  \fn          
  \brief        
  \return
*/
api_ret_t api_gpio_ioctl(uint32_t usrId,api_gpio_ioctl_t type, void *para)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_gpio_query(usrId);
    uint32_t index = usrId_to_gpio(usrId);
    if(ret == OPEN_HAL_USED)
    {
        uint8_t val = 0;
        uint8_t pin=index%16;
        uint8_t port=index/16;
        if(para!=NULL) val = *(uint8_t *)para;
        GpioPinConfig_t gpioConfig = {0};
        // GpioInterruptConfig_e config = GPIO_INTERRUPT_DISABLED;
        switch (type) {
            case OPEN_GPIO_IOCTL_ISR_CB:
                // gpio_isr_cb[port][pin] = (IsrFunc *)para;
                gpioIsrSet(index,para);
                break;
            case OPEN_GPIO_IOCTL_DIR_OUT:
                sGpioRuntimeList[index] |= 0x40;
                if(val < 2U){
                    if(val) sGpioRuntimeList[index] |= 0x20;
                    else sGpioRuntimeList[index] &= ~(0x20);
                }
                gpio_cfg_conv(&gpioConfig,&sGpioRuntimeList[index]);
                GPIO_pinConfig(index/16, index%16, &gpioConfig);
                // gpio_set_output(index, level);
                break;
            case OPEN_GPIO_IOCTL_OUT_ACT:
                if(val < 2U){
                    if(val) sGpioRuntimeList[index] |= 0x10;
                    else sGpioRuntimeList[index] &= ~(0x10);
                }
                break;
            case OPEN_GPIO_IOCTL_DIR_IN:
                sGpioRuntimeList[index] &= ~(0x40);
                if(val < 6U){
                    sGpioRuntimeList[index] &= ~(0x07);
                    sGpioRuntimeList[index] += val; 
                }
                gpio_cfg_conv(&gpioConfig,&sGpioRuntimeList[index]);
                GPIO_pinConfig(index/16, index%16, &gpioConfig);
                break;
            case OPEN_GPIO_IOCTL_LOCKED:
                if(val) sGpioRuntimeList[index] |= 0x80;
                else sGpioRuntimeList[index] &= ~(0x80);
                break;
            default:
                break;
        }
        ret = OPEN_HAL_DONE;
    }
    // SYSLOG_INFO("[%d][0x%08X]-%d\r\n",index,usrId,ret);
    return ret;
}

/**
  \fn          
  \brief    对设备功耗和模式进行配置  
  \return
*/
api_ret_t api_gpio_pmctl(uint32_t usrId,open_hal_pm_t *cfg, size_t count)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_gpio_query(usrId);
    uint32_t index = usrId_to_gpio(usrId);
    // ASSERT(index < EC_GPIO_INDEX_LIMIT);
    if(ret == OPEN_HAL_USED)
    {
        if(cfg->runtime == RUNTIME_SUSPEND){
            if(cfg->mode == PM_LOWPOW){
                // printf("\r\ngpio[0x%08X,%d]pmctl:0x%X,0x%X",usrId,index,cfg->runtime,cfg->mode);
            }
        }
        ret = OPEN_HAL_DONE;
    }
    // SYSLOG_INFO("[%d][0x%08X]-%d\r\n",index,usrId,ret);
    return ret;
}

/**
  \fn          
  \brief    对设备进行写操作  
  \return
*/
api_ret_t api_gpio_write(uint32_t usrId, void* buf, size_t count)
{
    ASSERT(usrId > 0);
    api_ret_t ret = api_gpio_query(usrId);
    uint32_t index = usrId_to_gpio(usrId);
    // ASSERT(index < EC_GPIO_INDEX_LIMIT);
    if(ret == OPEN_HAL_USED)
    {
        uint32_t port = index/16;
        uint16_t pin = index%16;
        uint16_t val = 0;
        if(buf!=NULL) val = *(uint16_t *)buf; //GPIO_pinWrite输入参数uint16_t
        if(sGpioRuntimeList[index] & 0x10){
            if(val) val = 0;
            else val = 1;
        }
        else if(val) val = 1;
        GPIO_pinWrite(port, 1 << pin, val << pin);
        // printf("\r\ngpio%d[0x%08X]write:[%d]0x%X",index,usrId,sGpioActiveList[index],val);
        ret = OPEN_HAL_DONE;
    }
    // SYSLOG_INFO("[%d][0x%08X]-%d\r\n",index,usrId,ret);
    return ret;
}
/**
  \fn          
  \brief    对设备进行读操作   
  \return
*/
api_ret_t api_gpio_read(uint32_t usrId, void* buf, size_t count)
{
    ASSERT(usrId > 0);
    api_ret_t ret = api_gpio_query(usrId);
    uint32_t index = usrId_to_gpio(usrId);
    if(ret == OPEN_HAL_USED)
    {
        // ASSERT(index < EC_GPIO_INDEX_LIMIT);
        uint32_t port = index/16;
        uint16_t pin = index%16;
        uint32_t val = GPIO_pinRead(port,pin); //GPIO_pinRead返回长度为uint32_t
        // SYSLOG_INFO("pin:0x%X",val,sGpioRuntimeList[index]);
        if(buf!=NULL){
            if(sGpioRuntimeList[index] & 0x10)
            {
                if(val) *(uint8_t *)buf = 0;
                else *(uint8_t *)buf = 1;
            }
            else *(uint8_t *)buf = val;
        }   
        ret = OPEN_HAL_DONE;
    }
    // SYSLOG_INFO("[%d][0x%08X]%d\r\n",index,usrId,ret);
    return ret;
}
/**
  \fn          
  \brief        
  \return
*/
int api_register_gpio(api_register_inf *func)
{
    func->open = api_gpio_open;
    func->close = api_gpio_close;
    func->query = api_gpio_query;
    func->pmctl = api_gpio_pmctl;
    func->ioctl = api_gpio_ioctl;
    func->read = api_gpio_read;
    func->write = api_gpio_write;
}

/**
  \fn          
  \brief    测试接口
  \return
*/
int api_test_gpio(void)
{ 
    uint32_t test_gpio_id = 0;
    GpioPinConfig_t pinConfig = {0};
    pinConfig.pinDirection = GPIO_DIRECTION_OUTPUT;
    pinConfig.misc.initOutput = 0;
    pinConfig.pinDirection = GPIO_DIRECTION_INPUT;
    pinConfig.misc.interruptConfig = GPIO_INTERRUPT_BOTH_EDGE;
    printf("\r\ntest gpio[%d-%d]:",EC_GPIO_INDEX_START,EC_GPIO_INDEX_LIMIT); 
    for(int i=EC_GPIO_INDEX_START;i<EC_GPIO_INDEX_LIMIT;i++)
    {
        printf("\r\ntest gpio[%d]", i);
        test_gpio_id = api_gpio_create(i,&pinConfig);
        EC_API_CHECK(api_gpio_open(test_gpio_id,NULL,0));
        uint8_t level = 0;
        EC_API_CHECK(api_gpio_ioctl(test_gpio_id,OPEN_GPIO_IOCTL_DIR_OUT,&level));
        uint8_t active = 0;
        EC_API_CHECK(api_gpio_ioctl(test_gpio_id,OPEN_GPIO_IOCTL_OUT_ACT,&active));
        level = 1;
        EC_API_CHECK(api_gpio_write(test_gpio_id,&level,1));
        EC_API_CHECK(api_gpio_read(test_gpio_id,&level,1));
        // SYSLOG_INFO("[0x%08X]read %d\r\n",test_gpio_id,level);
        open_hal_pm_t cfg = {
            .mode = PM_LOWPOW,
            .runtime = RUNTIME_SUSPEND
        };
        EC_API_CHECK(api_gpio_pmctl(test_gpio_id,&cfg,sizeof(cfg)));
        EC_API_CHECK(api_gpio_close(test_gpio_id));
        EC_API_CHECK(api_gpio_delete(test_gpio_id));
    }
    for(int i=EC_GPIO_INDEX_START;i<EC_GPIO_INDEX_LIMIT;i++)
    {
        printf("[%d].[0x%08X],0x%02X,0x%02X\r\n",i,sGpioUsrIdList[i],gpioStartupList[i],sGpioRuntimeList[i]);
    }
    printf("-----gpio test end-----\r\n");
}
