/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_pad.c
 * Description:  ec7xx openhal pad entry source file
 * History:      Rev1.0   2024-01-10
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
#include "pad.h"
#include "devicemanager.h"
#include "api_comm.h"

//默认加载的pad配置表，大小限制为芯片pad数量，可通过读固定区域方式获取
//使用中间层数据；1、有效减少存储数据大小，2、灵活增删项目
uint8_t padStartupList[EC_PAD_INDEX_LIMIT] = {
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, //dummy
    0x0, 0x0, 0x4, 0x4, 0x0, 0x0, 0x0, 0x0,         //11-18
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,         //19-26
    0x2E, 0x2E, 0x0, 0x0, 0x07, 0x07, 0x07, 0x07,   //27-34
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,         //35-42  
    0x0, 0x0, 0x2E, 0x2E, 0x0, 0x6, 0x6, 0x6,       //43-50
    0x6, 0x6, 0x28,                                  //51-53
};
static uint8_t sPadRuntimeList[EC_PAD_INDEX_LIMIT] = {0xFF};

//sPadUsrIdList管理设备ID，sPadUsrIdSeed管理ID便宜状态
//谁创建谁回收，删除设备需要匹配传入ID进行校验
static uint32_t sPadUsrIdList[EC_PAD_INDEX_LIMIT] = {0};
static uint16_t sPadUsrIdSeed[EC_PAD_INDEX_LIMIT] = {0};

//状态管理方式：NONE->FREE通过sPadUsrIdList赋值情况确认
//IDLE->USED->IDLE通过osPadBitList状态位确认，空闲置位1

#ifdef FEATURE_OPENHAL_RTOS_ENABLE
#define EC_PAD_EVT_INDEX_MID        (EC_PAD_INDEX_START+24)
static osEventFlagsId_t osPadBitList11_34 = NULL;  //根据具体芯片PAD编号定义，有效24-bit
static osEventFlagsId_t osPadBitList35_53 = NULL;
#endif

/**
  \fn          
  \brief     
  \return
*/
static uint32_t usrId_to_paddr(uint32_t usrId)
{ 
    uint32_t paddr = (uint32_t)(usrId & OPEN_HAL_PORT_MUSK); 
    if(paddr >= EC_PAD_INDEX_START && paddr < EC_PAD_INDEX_LIMIT)
    {
        if((sPadUsrIdList[paddr] & OPEN_HAL_PORT_MUSK) == paddr) return paddr;
    }
    else if(usrId == paddr) //传入的是paddr，特权处理
    {
        return paddr;
    }
    return EC_PAD_INDEX_LIMIT;
}

/**
  \fn          
  \brief     
  \return
*/
uint8_t pad_cfg_conv(PadConfig_t *padConfig,PadConfigLite_t *val)
{
    uint8_t ret = 0;
    if(padConfig==NULL)
    {
        if(val!=NULL) ret = *(uint8_t*)val;  //PadConfigLite_t -> uint8_t
    }
    else if(val==NULL)   //PadConfig_t -> uint8_t
    {
        PadConfigLite_t liteConfig = {0};
        liteConfig.mux = padConfig->mux;
        liteConfig.pullUpEnable = padConfig->pullUpEnable;
        liteConfig.pullDownEnable = padConfig->pullDownEnable;
        liteConfig.pullSelect = padConfig->pullSelect;
        liteConfig.inputControl = padConfig->inputControl;
        liteConfig.outputControl = padConfig->outputControl;
        ret = *(uint8_t *)&liteConfig;
    }
    else if(val->mux < 7U)   //PadConfigLite_t -> PadConfig_t
    {
        PAD_getDefaultConfig(padConfig);   //driveStrength等值
        padConfig->mux = val->mux;
        padConfig->inputControl = val->inputControl;
        padConfig->outputControl = val->outputControl;
        padConfig->pullSelect = val->pullSelect;
        padConfig->pullUpEnable = val->pullUpEnable;
        padConfig->pullDownEnable = val->pullDownEnable;
        ret = *(uint8_t*)val;
    }
    return ret;
}
/**
  \fn          
  \brief       
  \return
*/
static uint8_t pad_set_free(uint32_t paddr,PadConfigLite_t *cfg)
{
    if(paddr >= EC_PAD_INDEX_START && paddr < EC_PAD_INDEX_LIMIT)
    {
        if(cfg == NULL) return 0;
        if((cfg->mux & 0x07) != 0x07)
        {
            sPadUsrIdList[paddr] = OPEN_HAL_STAT_UNUSED;
            PadConfig_t padConfig = {0};
            sPadRuntimeList[paddr] = pad_cfg_conv(&padConfig,cfg);
            PAD_setPinConfig(paddr,&padConfig);
            return 1;
        }
        else sPadRuntimeList[paddr] = 0xFF;
    }
    return 0;
}
/**
  \fn          
  \brief       
  \return
*/
static uint32_t pad_set_used(uint32_t paddr)
{
    if(sPadRuntimeList[paddr] == 0xFF) return 0;
    if(paddr >= EC_PAD_INDEX_START && paddr < EC_PAD_INDEX_LIMIT)
    {
        sPadUsrIdList[paddr] |= OPEN_HAL_STAT_MUSK; //musk可细分状态
        // #ifdef FEATURE_OPENHAL_RTOS_ENABLE
        // if(paddr<EC_PAD_EVT_INDEX_MID && osPadBitList11_34 != NULL){
        //     uint32_t bitnum = 1 << (paddr - EC_PAD_INDEX_START);
        //     osEventFlagsClear(osPadBitList11_34,bitnum);
        // }
        // else if(paddr>=EC_PAD_EVT_INDEX_MID && osPadBitList35_53 != NULL){
        //     uint32_t bitnum = 1 << (paddr - EC_PAD_EVT_INDEX_MID);
        //     osEventFlagsClear(osPadBitList35_53,bitnum);
        // }
        // #endif
        return sPadUsrIdList[paddr];
    }
    return 0;
}
/**
  \fn          
  \brief    更新pad空闲状态    
  \return
*/
static uint32_t pad_set_idle(uint32_t paddr)
{
    if(sPadRuntimeList[paddr] == 0xFF) return 0;
    if(paddr >= EC_PAD_INDEX_START && paddr < EC_PAD_INDEX_LIMIT)
    {
        if(sPadUsrIdList[paddr] == OPEN_HAL_STAT_UNUSED){
            sPadUsrIdList[paddr] = paddr;
            sPadUsrIdSeed[paddr] ++;
            sPadUsrIdList[paddr] |= (uint32_t)(sPadUsrIdSeed[paddr] << 16);
            return sPadUsrIdList[paddr]; 
        }
        else if(sPadUsrIdList[paddr] & OPEN_HAL_STAT_MUSK){
            sPadUsrIdList[paddr] &= ~(OPEN_HAL_STAT_MUSK);
            return sPadUsrIdList[paddr]; 
        }
        #ifdef FEATURE_OPENHAL_RTOS_ENABLE
        // if(paddr<EC_PAD_EVT_INDEX_MID && osPadBitList11_34 != NULL){
        //     uint32_t bitnum = 1 << (paddr - EC_PAD_INDEX_START);
        //     osEventFlagsClear(osPadBitList11_34,bitnum);
        // }
        #endif
        return sPadUsrIdList[paddr];
    }
    return 0;
}
/**
  \fn          
  \brief      查询pad资源状态 ,uint32_t paddr/usrId
  \return     状态值
*/
api_ret_t api_pad_query(uint32_t usrId)
{
    // ASSERT(usrId > 0);
    uint32_t paddr = usrId_to_paddr(usrId);
    if(sPadRuntimeList[paddr] == 0xFF) return OPEN_HAL_LOCK;
    if(paddr < EC_PAD_INDEX_LIMIT)
    {
        if(sPadUsrIdList[paddr] == OPEN_HAL_STAT_UNUSED)
        {
            return OPEN_HAL_FREE;
        }
        else if(sPadUsrIdList[paddr] & OPEN_HAL_STAT_MUSK)
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
  \brief     上电加载默认初始化
  \return
*/
int api_pad_startup(void* para)
{
    for(int i=EC_PAD_INDEX_START;i<EC_PAD_INDEX_LIMIT;i++)
    {
        if(para!=NULL) pad_set_free(i,(PadConfigLite_t *)para+i);
        else pad_set_free(i,&padStartupList[i]);
    }
    #ifdef FEATURE_OPENHAL_RTOS_ENABLE
    osPadBitList11_34 = osEventFlagsNew(NULL);
    osPadBitList35_53 = osEventFlagsNew(NULL);
    #endif
}
/**
  \fn          
  \brief    
  \return
*/
uint32_t api_pad_default(char* str,PadConfigLite_t *cfg)
{
    if(str == NULL) return 0;
    char  *head   = NULL;
    char *item_str[8];  //pad配置文件每行8项
    int num = 0;  
    head = strtok(str, ",");
    while( head != NULL && num<8) 
    {
        item_str[num] = head;
        head = strtok(NULL, ",");
        num ++; 
    }
    uint32_t paddr = atoi(item_str[0]);
    cfg->mux = atoi(item_str[1]);
    cfg->pullUpEnable = atoi(item_str[2]);
    cfg->pullDownEnable = atoi(item_str[3]);
    cfg->pullSelect = atoi(item_str[4]);
    cfg->inputControl = atoi(item_str[5]);
    cfg->outputControl = atoi(item_str[6]);
    SYSLOG_INFO("pad%d,0x%02X\r\n",paddr,*(uint8_t *)cfg);
    return paddr;
}
/**
  \fn          
  \brief     
  \return
*/
uint32_t api_pad_create(uint32_t paddr,void *cfg)
{
    // ASSERT(paddr < EC_PAD_INDEX_LIMIT);
    uint32_t usrId = pad_set_idle(paddr);
    if(usrId)
    {
        if(cfg != NULL)
        {
            PadConfig_t *padConfig = (PadConfig_t *)cfg;
            if(padConfig->mux < 7U && padConfig->outputControl < 2U)
            {
                sPadRuntimeList[paddr] = pad_cfg_conv(padConfig,NULL);
                PAD_setPinConfig(paddr,padConfig);
            } 
        }
    }
    // SYSLOG_INFO("[%d][0x%08X]\r\n",paddr,usrId);
    return usrId;
}

/**
  \fn          
  \brief        
  \return
*/
api_ret_t api_pad_delete(uint32_t usrId)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_pad_query(usrId);
    uint32_t paddr = usrId_to_paddr(usrId);
    // ASSERT(paddr < EC_GPIO_INDEX_LIMIT);
    if(ret == OPEN_HAL_IDLE)
    {
        pad_set_free(paddr,&padStartupList[paddr]);
        ret = OPEN_HAL_DONE;
    }
    // SYSLOG_INFO("[%d][0x%08X]%d\r\n",paddr,usrId,ret);
    return ret;
}
/**
  \fn          
  \brief    acquire pad
  \return
*/
api_ret_t api_pad_open(uint32_t usrId,void *cfg,size_t timeout)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_pad_query(usrId);
    uint32_t paddr = usrId_to_paddr(usrId);
    if(ret == OPEN_HAL_IDLE)
    {
        if(cfg != NULL)
        {
            PadConfig_t *padConfig = (PadConfig_t *)cfg;
            if(padConfig->mux < 7U && padConfig->outputControl < 2U)
            {
                sPadRuntimeList[paddr] = pad_cfg_conv(padConfig,NULL);
                PAD_setPinConfig(paddr,padConfig);
            } 
        } 
        pad_set_used(paddr);
        ret = OPEN_HAL_DONE;
    }
    // SYSLOG_INFO("[%d][0x%08X]%d\r\n",paddr,usrId,ret);
    return ret;
}


/**
  \fn          
  \brief     release pad
  \return
*/
api_ret_t api_pad_close(uint32_t usrId)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_pad_query(usrId);
    uint32_t paddr = usrId_to_paddr(usrId);
    if(ret == OPEN_HAL_USED)
    {
        pad_set_idle(paddr);
        ret = OPEN_HAL_DONE;
    }
    // SYSLOG_INFO("[%d][0x%08X]%d\r\n",paddr,usrId,ret);
    return ret;
}
/**
  \fn          
  \brief    占用pad并回调，依赖rtos
  \return
*/
api_ret_t api_pad_apply(uint32_t usrId,void *cb)
{
    return OPEN_HAL_NONE;
}

/**
  \fn          
  \brief        
  \return
*/
api_ret_t api_pad_ioctl(uint32_t usrId,api_pad_ioctl_t type, void *para)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_pad_query(usrId);
    uint32_t paddr = usrId_to_paddr(usrId);
    PadConfig_t padConfig = {0};
    uint8_t val = 0;
    if(ret == OPEN_HAL_USED)
    {
        if(para!=NULL) val = *(uint8_t *)para;
        switch (type) {
            case OPEN_PAD_IOCTL_FUNC:
                if(val < 7U){
                    sPadRuntimeList[paddr] &= 0xF8;
                    sPadRuntimeList[paddr] += val;
                    pad_cfg_conv(&padConfig,&sPadRuntimeList[paddr]);
                    PAD_setPinConfig(paddr,&padConfig);
                }
                break;
            case OPEN_PAD_IOCTL_PULLUP:
                if(val < 2U){
                    if(val) sPadRuntimeList[paddr] |= 0x08;
                    else sPadRuntimeList[paddr] &= 0xF7;
                    pad_cfg_conv(&padConfig,&sPadRuntimeList[paddr]);
                    PAD_setPinConfig(paddr,&padConfig);
                    PAD_setPinPullConfig(paddr, PAD_INTERNAL_PULL_UP);
                }
                break;
            case OPEN_PAD_IOCTL_PULLDOWN:
                if(val < 2U){
                    if(val) sPadRuntimeList[paddr] |= 0x10;
                    else sPadRuntimeList[paddr] &= 0xEF;
                    pad_cfg_conv(&padConfig,&sPadRuntimeList[paddr]);
                    PAD_setPinConfig(paddr,&padConfig);
                    PAD_setPinPullConfig(paddr, PAD_INTERNAL_PULL_DOWN);
                }
                break;
            case OPEN_PAD_IOCTL_PULLSELECT:
                if(val < 2U){
                    if(val) sPadRuntimeList[paddr] |= 0x20; //PAD_PULL_INTERNAL
                    else sPadRuntimeList[paddr] &= 0xDF;
                    pad_cfg_conv(&padConfig,&sPadRuntimeList[paddr]);
                    PAD_setPinConfig(paddr,&padConfig);
                }
                break;
            default:
                break;
        }
        ret = OPEN_HAL_DONE;
    }
    // SYSLOG_INFO("[%d][0x%08X]%d\r\n",paddr,usrId,ret);
    return ret;
}
/**
  \fn          
  \brief    对设备功耗和模式进行配置  
  \return
*/
api_ret_t api_pad_pmctl(uint32_t usrId,open_hal_pm_t *cfg, size_t count)
{
    api_ret_t ret = api_pad_query(usrId);
    uint32_t paddr = usrId_to_paddr(usrId);
    if(ret == OPEN_HAL_USED)
    {
        if(cfg != NULL){
            if(count==0){
                // SYSLOG_INFO("[0x%08X]read pmctl\r\n",usrId);
            }
            else if(cfg->runtime == RUNTIME_SUSPEND){
                if(cfg->mode == PM_LOWPOW){
                    // SYSLOG_INFO("[0x%08X]SUSPEND+LOWPOW\r\n",usrId);
                }
            }
            ret = OPEN_HAL_DONE;
        }
    }
    // SYSLOG_INFO("[%d][0x%08X]%d\r\n",paddr,usrId,ret);
    return ret;
}

/**
  \fn          
  \brief    对设备进行写操作  
  \return
*/
int api_pad_write(uint32_t usrId, void* buf, size_t count)
{
    // PAD_setPinPullConfig(pad, PAD_INTERNAL_PULL_UP);
}

/**
  \fn          
  \brief    对设备进行读操作   
  \return
*/
int api_pad_read(uint32_t usrId, void* buf, size_t count)
{

}

/**
  \fn          
  \brief        
  \return
*/
int api_register_pad(api_register_inf *func)
{
    func->open = api_pad_open;
    func->close = api_pad_close;
    func->query = api_pad_query;
    func->pmctl = api_pad_pmctl;
    func->ioctl = api_pad_ioctl;
    func->read = api_pad_read;
    func->write = api_pad_write;
}

/**
  \fn          
  \brief    
  \return
*/
int api_test_pad(void)
{
    // uint32_t test_pad_id = 0;
    // PadConfig_t     padConfig = {0};
    // printf("\r\n-----pad[%d-%d]:\r\n",EC_PAD_INDEX_START,EC_PAD_INDEX_LIMIT); 
    // for(int i=EC_PAD_INDEX_START;i<EC_PAD_INDEX_LIMIT;i++)
    // {
    //     printf("[%d],0x%02X",i,sPadRuntimeList[i]);
    //     uint8_t ret = pad_cfg_conv(&padConfig,&sPadRuntimeList[i]);
    //     test_pad_id = api_pad_create(i, &padConfig);
    //     EC_API_CHECK(api_pad_open(test_pad_id,NULL,0));
    //     // uint8_t padmux = PAD_MUX_ALT6;
    //     // EC_API_CHECK(api_pad_ioctl(test_pad_id,OPEN_PAD_IOCTL_FUNC,&padmux));
    //     uint8_t pullselect = PAD_PULL_INTERNAL;
    //     EC_API_CHECK(api_pad_ioctl(test_pad_id,OPEN_PAD_IOCTL_PULLSELECT,&pullselect));
    //     open_hal_pm_t cfg = {0};
    //     EC_API_CHECK(api_pad_pmctl(test_pad_id,&cfg,0));
    //     EC_API_CHECK(api_pad_close(test_pad_id));
    //     EC_API_CHECK(api_pad_delete(test_pad_id));
    //     printf(",0x%02X,[0x%08X],0x%02X\r\n",sPadRuntimeList[i],test_pad_id,ret);
    // }

    // PAD_getDefaultConfig(&padConfig);
    // padConfig.mux = PAD_MUX_ALT0;
    // test_pad_id = api_pad_create(53, &padConfig);
    // EC_API_CHECK(api_pad_open(test_pad_id,NULL,0));
    // for(int i=EC_PAD_INDEX_START;i<EC_PAD_INDEX_LIMIT;i++)
    // {
    //     printf("[%d].[0x%08X],0x%02X,0x%02X\r\n",i,sPadUsrIdList[i],padStartupList[i],sPadRuntimeList[i]);
    // }
    // printf("-----pad test end-----\r\n");
}

