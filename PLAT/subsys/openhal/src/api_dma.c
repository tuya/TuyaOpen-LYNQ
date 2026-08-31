/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_dma.c
 * Description:  openhal dma entry source file
 * History:      Rev1.0   2024-02-23
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_OPENHAL_DMA_ENABLE
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "cmsis_os2.h"
#include "Driver_Common.h"
#include "system_ec7xx.h"
#include DEBUG_LOG_HEADER_FILE
#include "bsp.h"
#include "devicemanager.h"
#include "api_comm.h"
#include "api_dma.h"

static uint32_t sDmaUsrIdList[EC_DMA_INDEX_LIMIT] = {0};

/**
  \fn         api_dma_query()    
  \brief      查询状态
  \return     状态枚举
*/
api_ret_t api_dma_query(uint32_t usrId)
{
    // ASSERT(usrId > 0);
    uint32_t index = usrId_to_index(usrId);
    // ASSERT(index < EC_DMA_INDEX_LIMIT);
    // printf("\r\nusrId[x%08X]:%d < %d",usrId,index,EC_DMA_INDEX_LIMIT); 
    if(index >= EC_DMA_INDEX_START && index < EC_DMA_INDEX_LIMIT)
    {
        if(sDmaUsrIdList[index] == OPEN_HAL_STAT_UNUSED)
        {
            return OPEN_HAL_FREE;
        }
        else if(sDmaUsrIdList[index] & OPEN_HAL_STAT_MUSK)
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
  \brief    上电初始化
  \return
*/
int api_dma_startup(void* para)
{
    for(int i=EC_DMA_INDEX_START;i<EC_DMA_INDEX_LIMIT;i++)
    {
        DMA_set_free(i);
    }
}


/**
  \fn          
  \brief    单项初始化，使用HAL统一的参数格式
  \return
*/
api_ret_t api_dma_setup(int8_t index, void* para)
{
    if(para == NULL) {

    }
    else if(index >= EC_DMA_INDEX_START && index < EC_DMA_INDEX_LIMIT) 
    {
        // TODO: 根据para配置DMA硬件
        EPAT_LOG(api_dma_setup, P_INFO, "dma %d,Config %x", index, *(uint32_t*)para);
        return OPEN_HAL_DONE;
    }
    return OPEN_HAL_INVALID_PARA;
}

/**
  \fn          
  \brief    
  \return
*/
uint32_t api_dma_create(uint32_t index,void *cfg)
{
    uint32_t usrId = DMA_set_idle(index);  
    if(usrId)
    {
        if(cfg != NULL)
        {
        } 
        SYSLOG_INFO("spi%d[0x%08X]\r\n",index,usrId); 
    }
    return usrId;
}
/**
  \fn          
  \brief        
  \return 
*/
api_ret_t api_dma_delete(uint32_t usrId)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_dma_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_IDLE)
    {
        DMA_set_free(index);
        ret = OPEN_HAL_DONE;
    }
    SYSLOG_INFO("spi%d[0x%08X]:%d\r\n",index,usrId,ret); 
    return ret;
}

/**
  \fn          
  \brief        
  \return
*/
api_ret_t api_dma_open(uint32_t usrId,void *cfg,size_t timeout)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_dma_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_IDLE)
    {
        if(cfg != NULL)
        {

        } 
        DMA_set_used(usrId);
        ret = OPEN_HAL_DONE;
    }
    SYSLOG_INFO("spi%d[0x%08X]:0x%X\r\n",index,usrId,ret); 
    // printf("\r\ni2c%d[0x%08X]open:0x%X",index,usrId,ret); 
    return ret;
}

/**
  \fn          
  \brief        
  \return
*/
api_ret_t api_dma_close(uint32_t usrId)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_dma_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_USED)
    {
        DMA_set_idle(index);
        ret = OPEN_HAL_DONE;
    }
    SYSLOG_INFO("spi%d[0x%08X]:ret %d\r\n",index,usrId,ret); 
    return ret;
}
/**
  \fn          
  \brief        
  \return
*/
api_ret_t api_dma_ioctl(uint32_t usrId,api_dma_ioctl_t type, uint32_t para)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_dma_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    // if(ret == OPEN_HAL_USED)
    {
        switch (type) {
            case OPEN_dma_IOCTL_CLK:
                gSpiClk[index] = para;
                break;
            case OPEN_dma_IOCTL_CONFIG:
                // spiConfig(index);    //TODO
                break;
            default:
                break;
        }
        ret = OPEN_HAL_DONE;
    }

    return ret;
}
/**
  \fn          
  \brief    
  \return
*/
api_ret_t api_dma_pmctl(uint32_t usrId,open_hal_pm_t *cfg, size_t count)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_dma_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_USED)
    {
        if(cfg != NULL){
            if(count==0){
                SYSLOG_INFO("[0x%08X]read\r\n",usrId);
            }
            else if(cfg->runtime == RUNTIME_SUSPEND){
                if(cfg->mode == PM_LOWPOW){
                    SYSLOG_INFO("[0x%08X] SUSPEND+LOWPOW\r\n",usrId);
                }
            }
            ret = OPEN_HAL_DONE;
        }
    }
    return ret;
}


api_ret_t api_dma_write(uint32_t usrId, void* buf, size_t count)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_dma_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_USED)
    { 
        if(buf != NULL && count){
            spiWriteBuffer(gSpiDrv[index], buf, count);
        }
        ret = OPEN_HAL_DONE;
    }

    return ret;
}

api_ret_t api_dma_read(uint32_t usrId, void* buf, size_t count)
{
    api_ret_t ret = api_dma_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_USED)
    { 
        if(buf != NULL && count){
            spiReadBuffer(gSpiDrv[index], buf, count);
        }
        ret = OPEN_HAL_DONE;
    }

    return ret;
}

uint8_t api_dma_read_write_byte(uint32_t usrId, uint8_t dataOut)
{
    uint8_t dataIn = 0;
    api_ret_t ret = api_dma_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_USED)
    { 
        if (dataOut != NULL){
            dataIn = spiReadWriteByte(gSpiDrv[index], dataOut);
        }
        ret = OPEN_HAL_DONE;
    }

    return dataIn;
}
#endif
