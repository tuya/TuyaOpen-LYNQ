/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_tp.c
 * Description:  ec7xx openHAL TP entry header file
 * History:      Rev1.0   2024-08-29
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "ostask.h"
#include "cmsis_os2.h"
#include "Driver_Common.h"
#include "system_ec7xx.h"
#include DEBUG_LOG_HEADER_FILE
#include "bsp.h"
#include "bsp_custom.h"
#include "devicemanager.h"
#include "api_comm.h"
#include "api_i2c.h"
#include "api_tp.h"

#define EPAT_LOG(subId, debugLevel, format, ...)  \
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, subId, debugLevel, format, ##__VA_ARGS__)

static uint32_t s_tpUsrIdList[EC_TP_INDEX_LIMIT] = {0};
static uint16_t s_tpUsrIdSeed[EC_TP_INDEX_LIMIT] = {0};

static api_tp_inf tp_device;
static api_tp_UsrId_t tpDev;
static api_tp_UsrId_t *active_tp;
/**
  \fn
  \brief    将用户层索引编号转换为序号，目前只支持1个同类外设
  \return
*/
static uint32_t usrId_to_index(uint32_t usrId)
{ 
    uint32_t index = (uint32_t)(usrId & OPEN_HAL_PORT_MUSK); 
    if(index == (s_tpUsrIdList[index] & OPEN_HAL_PORT_MUSK))
    {
        return index;
    }
    return EC_TP_INDEX_LIMIT;
}

/**
  \fn         api_tp_query()    
  \brief      查询状态
  \return     状态枚举
*/
api_ret_t api_tp_query(uint32_t usrId)
{
    // ASSERT(usrId > 0);
    uint32_t index = usrId_to_index(usrId);
    if(index >= EC_TP_INDEX_START && index < EC_TP_INDEX_LIMIT)
    {
        if(s_tpUsrIdList[index] == OPEN_HAL_STAT_UNUSED)
        {
            return OPEN_HAL_FREE;
        }
        else if(s_tpUsrIdList[index] & OPEN_HAL_STAT_MUSK)
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
  \brief   默认状态配置
  \return
*/
static uint32_t tp_set_free(uint32_t index)
{
    if(index >= EC_TP_INDEX_START && index < EC_TP_INDEX_LIMIT)
    {
        s_tpUsrIdList[index] = OPEN_HAL_STAT_UNUSED;
        // sI2cRuntimeList[index] = sI2cStartupList[index];
        return 1;  
    }
    return 0;
}
/**
  \fn          
  \brief
  \return
*/
static uint32_t tp_set_idle(uint32_t index)
{
    // ASSERT(index < EC_TP_INDEX_LIMIT);
    uint32_t ret = 0;
    if(s_tpUsrIdList[index] == OPEN_HAL_STAT_UNUSED){
        s_tpUsrIdList[index] = index;
        s_tpUsrIdSeed[index] ++;
        s_tpUsrIdList[index] |= (uint32_t)(s_tpUsrIdSeed[index] << 16);
        ret = s_tpUsrIdList[index];
    }
    else if(s_tpUsrIdList[index] & OPEN_HAL_STAT_MUSK){
        s_tpUsrIdList[index] &= ~(OPEN_HAL_STAT_MUSK);
        ret = s_tpUsrIdList[index];
    }
    return ret;
}
/**
  \fn
  \brief
  \return
*/
static uint32_t tp_set_used(uint32_t usrId)
{
    api_ret_t stat = api_tp_query(usrId);
    if(stat != OPEN_HAL_IDLE){
        return 0;
    }
    uint32_t index = usrId_to_index(usrId);
    s_tpUsrIdList[index] |= OPEN_HAL_STAT_MUSK;
    return 1;
}

/**
  \fn
  \brief
  \return
*/
int api_tp_startup(void* para)
{
    for(int i=EC_TP_INDEX_START;i<EC_TP_INDEX_LIMIT;i++)
    {
        tp_set_free(i);
    }
}
/**
  \fn
  \brief    输出设备的所有状态数据
  \return
*/
int api_tp_report(void)
{
    for(int i=EC_TP_INDEX_START;i<EC_TP_INDEX_LIMIT;i++)
    {
        EPAT_LOG(api_tp_report, P_INFO, "[%d]0x%X,0x%X",i,s_tpUsrIdList[i],s_tpUsrIdSeed[i]);
    }
    // EPAT_LOG(api_tp_report_ft6336, P_INFO, "0x%X,0x%X,0x%X,0x%X",tpDev.tpUsrId,tpDev.i2cUsrId,tpDev.PadUsrId,tpDev.GpioUsrId);
    // EPAT_LOG(api_tp_report_config, P_INFO, "0x%X,0x%X,0x%X,0x%X",tp_device.scan,tp_device.PadId,tp_device.GpioId,tp_device.gpioISR);
}

/**
  \fn
  \brief
  \return
*/
static void tp_bus_cb(uint32_t event)
{
    EPAT_LOG(tp_bus_cb, P_INFO, "event 0x%X", event);
}
/**
  \fn
  \brief
  \return
*/
#define FT6336_ADDR 	          0x38
#define FT6336_GET_FINGERNUM 	  0x02
#define FT6336_GET_LOC0 	      0x03
#define FT6336_GET_LOC1 	      0x09
int tp_scan_ft6336(uint32_t usrId,int16_t *pos)
{
    int fingerNum = 0;
    uint8_t temp[4]={0}; 
    if(tpDev.tpUsrId != usrId){
        EPAT_LOG(ft6336_scan, P_ERROR, "input 0x%x != 0x%x",usrId,tpDev.tpUsrId);
        return 0;
    }
    api_i2c_master_t buffer = {
        .addr = FT6336_ADDR,
        .reg = FT6336_GET_FINGERNUM,
        .data = temp,
        .num = 1,
    };
    api_ret_t ret = api_i2c_read(tpDev.i2cUsrId,&buffer,sizeof(buffer));   //0x10000
    if(ret == OPEN_HAL_DONE){   
        fingerNum = temp[0];
        if(fingerNum)
        {
            buffer.reg = FT6336_GET_LOC0;
            buffer.num = 4;
            api_i2c_read(tpDev.i2cUsrId,&buffer,sizeof(buffer));
            if(pos != NULL){
                pos[0]=((uint16_t)(temp[0]&0x0F)<<8)+temp[1];
                pos[1]=(((uint16_t)(temp[2]&0x0F)<<8)+temp[3]);
            }
        }
    }
    // EPAT_LOG(tp_scan_ft6336, P_INFO, "0x%x,0x%x,0x%x,x%d,y%d",usrId,tpDev.tpUsrId,tpDev.i2cUsrId,pos[0],pos[1]);
	return fingerNum;
}
/**
  \fn
  \brief
  \return
*/
#if (LCD_AXS15231_ENABLE == 1)
#define AXS_ADDR        0x3B
static int16_t last_x = 0, last_y = 0;
extern api_ret_t api_i2c_read_axs(uint32_t usrId, void* buf, size_t count);
static int tp_scan_axs15231(uint32_t usrId,int16_t *pos)
{
    int8_t stat = 0;
    uint8_t read_data[8] = {0};
    api_i2c_master_t buffer = {
        .addr = AXS_ADDR,
        .reg = 0,
        .data = read_data,
        .num = 8,
    };
    api_ret_t ret = api_i2c_read_axs(tpDev.i2cUsrId,&buffer,sizeof(buffer));
    if(ret == OPEN_HAL_DONE){
        stat = read_data[2]>>4;     //8=HOLD,4=RELEASED,0=PRESSED
        int8_t fingers = read_data[1];
        int16_t Xpos = ((read_data[2] & 0x0F) << 8) + read_data[3];
        int16_t Ypos = ((read_data[4] & 0x0F) << 8) + read_data[5];
        if(last_x != Xpos || last_y != Ypos){
            last_x = Xpos;
            last_y = Ypos;
            stat |= 0x01; 
            if(pos != NULL) {
                if(Xpos<=320) pos[0] = Xpos;
                if(Ypos<=480) pos[1] = Ypos;
            }
        }
        else if(stat == 0x4){    //RELEASED
            last_x = 0;
            last_y = 0;
            stat = 0x0;
        }
        else {
            EPAT_LOG(axs15231_tp_raw, P_INFO, "stat 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x",stat,\
            read_data[0],read_data[1],read_data[2],read_data[3],read_data[4],read_data[5]);
        }
    }
    return stat;
}
#endif
/**
  \fn
  \brief
  \return
*/
#if (LCD_ST77922_ENABLE == 1)
#define ST77922_ADDR    0x55
static int tp_scan_st77922(uint32_t usrId,int16_t *pos)
{
    int8_t stat = 0;
    uint8_t read_data[2*7+4] = {0xFF};  //0x14-0x1A  0x1B-0x21
    api_i2c_master_t buffer = {
        .addr = ST77922_ADDR,
        .reg = 0,
        .data = read_data,
        .num = 18,
    };
    api_ret_t ret = api_i2c_read_tp(tpDev.i2cUsrId,&buffer,sizeof(buffer));
    if(ret == OPEN_HAL_DONE){
        int16_t Xpos = 0;
        int16_t Ypos = 0;
        if(read_data[0] & 0x08){
            for (uint8_t i = 0; i < 2; i++) {
                uint8_t *p = &read_data[4+i*7];
                int16_t area = *(p + 4);
                if (*p & 0x80) {
                    stat |= 0x01;
                    Xpos = (uint16_t)(((uint16_t)(*p & 0x3F) << 8) | ((uint16_t)*(p + 1) & 0xFF));
                    Ypos = (uint16_t)(((uint16_t)(*(p + 2) & 0x3F) << 8) | ((uint16_t)*(p + 3) & 0xFF));
                    if(pos != NULL) {
                        if(Xpos<=480) pos[i*2+0] = Xpos;
                        if(Ypos<=480) pos[i*2+1] = Ypos;
                    }
                }
            }
            EPAT_LOG(tpScan, P_INFO, "[%d]addr=0x%x,0x%x",osKernelGetTickCount(),ST77922_ADDR,read_data[0]);
        }
    }
    return stat;
}
#endif
/**
  \fn
  \brief
  \return
*/
api_tp_infp api_tp_default(tpSupportList_e type)
{
    #ifdef TP_IRQ_PAD_INDEX
    tp_device.PadId = TP_IRQ_PAD_INDEX;
    tp_device.GpioId = (TP_IRQ_GPIO_INSTANCE*16+TP_IRQ_GPIO_PIN);
    tp_device.PadMux = TP_IRQ_PAD_ALT_FUNC;
    #endif
    tp_device.i2cId = 0;
    switch(type)
    {
        case HAL_TP_FT6336:
            tp_device.scan = tp_scan_ft6336;
            tp_device.gpioISR = NULL;
            break;
        #if (LCD_AXS15231_ENABLE == 1)
        case HAL_TP_AXS15231:
            tp_device.scan = tp_scan_axs15231;
            tp_device.gpioISR = NULL;
            break;
        #endif 
        #if (LCD_ST77922_ENABLE == 1)
        case HAL_TP_ST77922:
            tp_device.scan = tp_scan_st77922;
            tp_device.gpioISR = NULL;
            break;
        #endif 
        default:
            EPAT_LOG(api_tp_default, P_ERROR, "invalid type %d",type);
    }
    return &tp_device;
}

/**
  \fn
  \brief  将通过cfg传入初始化参数
  \return
*/
uint32_t api_tp_create(uint32_t index,void *cfg)
{
    uint32_t usrId = tp_set_idle(index);  
    // if(usrId)
    {
        tpDev.tpUsrId = usrId;
        if(cfg != NULL){
            api_tp_infp inf = (api_tp_infp)cfg;
            EPAT_LOG(api_tp_create_cfg, P_INFO, "cfg %d,%d,%d,%d,0x%x",inf->PadId,inf->PadMux,inf->GpioId,inf->i2cId,inf->gpioISR);
            PadConfig_t padConfig = {0};
            PAD_getDefaultConfig(&padConfig);
            padConfig.pullUpEnable = PAD_PULL_UP_DISABLE;
            padConfig.pullDownEnable = PAD_PULL_DOWN_DISABLE;
            padConfig.mux = inf->PadMux;
            tpDev.PadUsrId = api_pad_create(inf->PadId, &padConfig);
            api_pad_open(tpDev.PadUsrId,NULL,10);
            GpioPinConfig_t pinConfig = {0};
            pinConfig.pinDirection = GPIO_DIRECTION_INPUT;
            pinConfig.misc.interruptConfig = GPIO_INTERRUPT_RISING_EDGE;
            tpDev.GpioUsrId = api_gpio_create(inf->GpioId, &pinConfig);
            api_gpio_open(tpDev.GpioUsrId,NULL,10);
            api_gpio_ioctl(tpDev.GpioUsrId,OPEN_GPIO_IOCTL_ISR_CB,inf->gpioISR);
            tpDev.i2cUsrId = api_i2c_create(inf->i2cId,NULL);
            api_i2c_open(tpDev.i2cUsrId,NULL,1000);
            uint8_t speed = 2;
            api_i2c_ioctl(tpDev.i2cUsrId,OPEN_I2C_IOCTL_SPEED,&speed);
            api_i2c_ioctl(tpDev.i2cUsrId,OPEN_I2C_IOCTL_ISR_CB,tp_bus_cb);
        }
    }
end:
    EPAT_LOG(api_tp_create, P_INFO, "usrId 0x%x,index 0x%x,0x%x,0x%x,0x%x,0x%x",usrId,index,tpDev.tpUsrId,tpDev.PadUsrId,tpDev.GpioUsrId,tpDev.i2cUsrId);
    return usrId;
}

/**
  \fn
  \brief
  \return
*/
api_ret_t api_tp_delete(uint32_t usrId)
{
    ASSERT(usrId > 0);
    api_ret_t ret = api_tp_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    // ASSERT(index < EC_TP_INDEX_LIMIT);
    if(ret == OPEN_HAL_IDLE && index < EC_TP_INDEX_LIMIT)
    {
        if(tp_set_free(index))
        {
        }
        ret = OPEN_HAL_DONE;
    }
end:
    EPAT_LOG(api_tp_delete, P_INFO, "delete i2c%d", index);
    return ret;
}

/**
  \fn
  \brief
  \return
*/
api_ret_t api_tp_open(uint32_t usrId,void *cfg,size_t timeout)
{
    ASSERT(usrId > 0);
    api_ret_t ret = api_tp_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    // ASSERT(index < EC_TP_INDEX_LIMIT);
    if(ret == OPEN_HAL_IDLE && index < EC_TP_INDEX_LIMIT)
    {
        if(cfg != NULL)
        {
 
        }
        tp_set_used(usrId);
        ret = OPEN_HAL_DONE;
    }
end:
    EPAT_LOG(api_tp_open, P_INFO, "open i2c%d,ret %d", index,ret);
    return ret;
}
/**
  \fn
  \brief
  \return
*/
api_ret_t api_tp_close(uint32_t usrId)
{
    ASSERT(usrId > 0);
    api_ret_t ret = api_tp_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    // ASSERT(index < EC_TP_INDEX_LIMIT);
    if(ret == OPEN_HAL_USED && index < EC_TP_INDEX_LIMIT)
    {  
        tp_set_idle(index);
        ret = OPEN_HAL_DONE;
        // printf("\r\ni2c%d close[0x%08X]:0x%08X",index,usrId,s_tpUsrIdList[index]);
    }
end:
    EPAT_LOG(api_tp_close, P_INFO, "i2c%d,usrId 0x%X,0x%X,ret%d",index,usrId,s_tpUsrIdList[index],ret);
    return ret;
}
/**
  \fn
  \brief
  \return
*/
api_ret_t api_tp_ioctl(uint32_t usrId,api_tp_ioctl_t type, void *para)
{
    api_ret_t ret = api_tp_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_USED)
    {
        switch (type) {
            case OPEN_TP_USED_FT6336:
                active_tp = &tpDev;
                break;
            case OPEN_TP_USED_AXS15231:
                active_tp = &tpDev;
                break;
            default:
                break;
        }
        ret = OPEN_HAL_DONE;
    }
end:
    EPAT_LOG(api_tp_ioctl, P_INFO, "i2c%d,0x%X,type%d,ret%d",index,usrId,type,ret);
    // SYSLOG_INFO("[0x%08X],type%d,0x%02X,ret%d\r\n",usrId,type,sI2cRuntimeList[index],ret);
    return ret;
}
/**
  \fn
  \brief
  \return
*/
api_ret_t api_tp_pmctl(uint32_t usrId,open_hal_pm_t *cfg, size_t count)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_tp_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_USED)
    {
        if(cfg != NULL){
            if(count==0){
                // SYSLOG_INFO("[0x%08X]read\r\n",usrId);
            }
            else if(cfg->runtime == RUNTIME_SUSPEND){
                if(cfg->mode == PM_LOWPOW){
                    // SYSLOG_INFO("[0x%08X]set\r\n",usrId);
                }
            }
            ret = OPEN_HAL_DONE;
        }
    }
end:
    EPAT_LOG(api_tp_pmctl, P_INFO, "i2c%d,0x%X,ret%d",index,usrId,ret);
    return ret;
}
/**
  \fn
  \brief
  \return
*/
api_ret_t api_tp_write(uint32_t usrId, void* buf, size_t count)
{
    api_ret_t ret = api_tp_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_USED)
    {
        if(buf != NULL){
        }
        ret = OPEN_HAL_DONE;
    }
end:
    EPAT_LOG(api_tp_write, P_INFO, "i2c%d,0x%X,ret%d",index,usrId,ret);
    return ret;
}

/**
  \fn          
  \brief    
  \return
*/
api_ret_t api_tp_read(uint32_t usrId, void* buf, size_t count)
{
    api_ret_t ret = api_tp_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_USED)
    {
        if(buf != NULL){
            api_i2c_read(active_tp->i2cUsrId,(api_i2c_master_t *)buf,sizeof(api_i2c_master_t));
        }
        ret = OPEN_HAL_DONE;
    }
end:
    EPAT_LOG(api_tp_read, P_INFO, "0x%X,i2c%d,ret%d",usrId,index,ret);
    return ret;
}
/**
  \fn          
  \brief   测试完成需要释放对应资源 
  \return
*/
static void test_tp_cb(uint32_t event)
{
    // EPAT_LOG(test_tp_cb, P_INFO, "event 0x%X", event);
}

int api_test_tp(void)
{ 
    // uint32_t test_tp_id = 0;
    // uint32_t index = 0;
    // EPAT_LOG(api_test_tp, P_INFO, "test i2c%d", index);
    // for(int i=EC_TP_INDEX_START;index<EC_TP_INDEX_LIMIT;index++)
    {
        // test_tp_id = api_tp_create(index,NULL);
        // EC_API_CHECK(api_tp_open(test_tp_id,NULL,1000));
        // EC_API_CHECK(api_tp_ioctl(test_tp_id,OPEN_I2C_IOCTL_ISR_CB,test_tp_cb));
        // uint8_t speed = 2;
        // EC_API_CHECK(api_tp_ioctl(test_tp_id,OPEN_I2C_IOCTL_SPEED,&speed));
        // uint8_t temp[4]={0};    //test with minidkb tp
        // api_tp_master_t buffer = {
        //     .addr = 0x38,
        //     .reg = 0x02,    //read fingers
        //     .data = temp,
        //     .num = 1,
        // };
        // EC_API_CHECK(api_tp_read(test_tp_id,&buffer,sizeof(buffer)));
        // // EC_API_CHECK(api_tp_write(test_tp_id,&buffer,sizeof(buffer)));
        // // open_hal_pm_t cfg = {
        // //     .mode = PM_LOWPOW,
        // //     .runtime = RUNTIME_SUSPEND
        // // };
        // // EC_API_CHECK(api_tp_pmctl(test_tp_id,&cfg,sizeof(cfg)));
        // EC_API_CHECK(api_tp_close(test_tp_id));
        // EC_API_CHECK(api_tp_delete(test_tp_id));
    }
}