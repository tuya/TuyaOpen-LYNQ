/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_spi.c
 * Description:  openhal spi entry source file
 * History:      Rev1.0   2024-02-23
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_OPENHAL_SPI_ENABLE
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "cmsis_os2.h"
#include "Driver_Common.h"
#include "system_ec7xx.h"
#include DEBUG_LOG_HEADER_FILE
#include "bsp.h"
#include "bsp_spi.h"
#include "devicemanager.h"
#include "api_comm.h"
#include "api_spi.h"


#define TRANSFER_DATA_WIDTH     8
#define SPI_POLLING_DMA_SPLIT   14 // DMA: Greater than or equal to 14
#if (RTE_SPI0_IO_MODE == DMA_MODE)
#define SPI_DMA_ENABLE
#ifdef SPI_DMA_ENABLE
static osEventFlagsId_t gSpiFlags = NULL;
#endif
#define SPI_BUFFER_SIZE     8191
#endif


static uint8_t  gSpiBuffer[SPI_BUFFER_SIZE] = {0};
static uint32_t gSpiClk[EC_SPI_INDEX_LIMIT] = {0};
static uint32_t gSpiMode[EC_SPI_INDEX_LIMIT] = {0};
static uint32_t sSpiUsrIdList[EC_SPI_INDEX_LIMIT] = {0};
static uint16_t sSpiUsrIdSeed[EC_SPI_INDEX_LIMIT] = {0};
static IsrFunc sSpiIsrFunc[EC_UART_INDEX_LIMIT] = {NULL};

//D16-31:control  x0001   
//D0-15:baudrate  x0480=115200
static uint32_t sSpiStartupList[EC_SPI_INDEX_LIMIT] = {
    0x0, 0x0, 0x10060, 0x10480
};
static uint32_t sSpiRuntimeList[EC_SPI_INDEX_LIMIT] = {0xFFFFFFFF};

#if (RTE_SPI0 == 1)
extern ARM_DRIVER_SPI Driver_SPI0;
#endif
#if (RTE_SPI1 == 1)
extern ARM_DRIVER_SPI Driver_SPI1;
#endif
static ARM_DRIVER_SPI *gSpiDrv[EC_SPI_INDEX_LIMIT] =
{
#if (RTE_SPI0 == 1)
    &Driver_SPI0,
#else
    NULL,
#endif
#if (RTE_SPI1 == 1)
    &Driver_SPI1
#else
    NULL
#endif
};
/**
  \fn          
  \brief   
  \return
*/
static uint32_t usrId_to_index(uint32_t usrId)
{ 
    // ASSERT(usrId > 0);
    uint32_t index = (uint32_t)(usrId & OPEN_HAL_PORT_MUSK); 
    if(index >= EC_SPI_INDEX_START && index < EC_SPI_INDEX_LIMIT)
    {
        if(index == (sSpiUsrIdList[index] & OPEN_HAL_PORT_MUSK))
        {
            return index;
        }
    }
    return EC_SPI_INDEX_LIMIT;
}

/**
  \fn         api_spi_query()    
  \brief      查询状态
  \return     状态枚举
*/
api_ret_t api_spi_query(uint32_t usrId)
{
    // ASSERT(usrId > 0);
    uint32_t index = usrId_to_index(usrId);
    // ASSERT(index < EC_SPI_INDEX_LIMIT);
    // printf("\r\nusrId[x%08X]:%d < %d",usrId,index,EC_SPI_INDEX_LIMIT); 
    if(index >= EC_SPI_INDEX_START && index < EC_SPI_INDEX_LIMIT)
    {
        if(sSpiUsrIdList[index] == OPEN_HAL_STAT_UNUSED)
        {
            return OPEN_HAL_FREE;
        }
        else if(sSpiUsrIdList[index] & OPEN_HAL_STAT_MUSK)
        {
            return OPEN_HAL_USED;
        }
        else{
            return OPEN_HAL_IDLE;
        }
    }
    return OPEN_HAL_NONE;
}

#ifdef SPI_DMA_ENABLE
static void SPI_Callback(uint32_t event)
{
    if (event & ARM_SPI_EVENT_TRANSFER_COMPLETE)
    {
        osEventFlagsSet(gSpiFlags, 0x01);
    }
    else
    {
        ECPLAT_PRINTF(UNILOG_PLA_APP, SPI_Callback, P_ERROR, "event=0x%X", event);
    }
}
#endif

static uint8_t spiConfig(uint32_t index)
{
    uint8_t ret = 0;
    if(gSpiDrv[index] != NULL)
    {
#ifdef SPI_DMA_ENABLE
        if (gSpiFlags == NULL)
        {
            gSpiFlags = osEventFlagsNew(NULL);
        }
        gSpiDrv[index]->Initialize(SPI_Callback);
#else
        gSpiDrv[index]->Initialize(NULL);
#endif
        gSpiDrv[index]->PowerControl(ARM_POWER_FULL);
        gSpiDrv[index]->Control(ARM_SPI_MODE_MASTER | gSpiMode[index] | ARM_SPI_DATA_BITS(TRANSFER_DATA_WIDTH) |
                                ARM_SPI_MSB_LSB     | ARM_SPI_SS_MASTER_SW, gSpiClk[index]); // 76.8M Failure at High Temperatures
        ret = 1;
    }

    return ret;
}

static uint8_t spiStartSend(uint32_t index, void *data, uint32_t num)
{
    uint8_t ret = 0;
    if(gSpiDrv[index] != NULL)
    {
        gSpiDrv[index]->Send(data, num);
        ret = 1;
    }

    return ret;
}

static uint8_t spiStopSend(uint32_t index)
{
    uint8_t ret = 0;
    if(gSpiDrv[index] != NULL)
    {
        gSpiDrv[index]->StopSend();
        ret = 1;
    }

    return ret;
}

/**
  \fn          
  \brief   默认状态配置
  \return
*/
static uint32_t spi_set_free(uint32_t index)
{
    if(index >= EC_SPI_INDEX_START && index < EC_SPI_INDEX_LIMIT)
    {
        sSpiUsrIdList[index] = OPEN_HAL_STAT_UNUSED;
        sSpiIsrFunc[index] = NULL;
        sSpiRuntimeList[index] = sSpiStartupList[index];
        if(gSpiDrv[index] != NULL)
        {
        }
        return 1;  
    }
    return 0;
}

/**
  \fn          
  \brief     
  \return usrId
*/
static uint32_t spi_set_idle(uint32_t index)
{
    uint32_t usrId = 0;
    if(index >= EC_SPI_INDEX_START && index < EC_SPI_INDEX_LIMIT)
    {
        if(sSpiUsrIdList[index] == OPEN_HAL_STAT_UNUSED){
            sSpiUsrIdList[index] = index;
            sSpiUsrIdSeed[index] ++;
            sSpiUsrIdList[index] |= (uint32_t)(sSpiUsrIdSeed[index] << 16);
            usrId = sSpiUsrIdList[index];
            // SYSLOG_INFO("spi_set_idle[x%X]:0x%08X\r\n",index,sSpiUsrIdList[index]);
        }
        else if(sSpiUsrIdList[index] & OPEN_HAL_STAT_MUSK){
            sSpiUsrIdList[index] &= ~(OPEN_HAL_STAT_MUSK);
            usrId = sSpiUsrIdList[index];
        }
    }
    return usrId;
}

/**
  \fn          
  \brief    
  \return
*/
static uint32_t spi_set_used(uint32_t usrId)
{
    // EC_ASSERT(usrId > 0,0,0,0);
    api_ret_t ret = api_spi_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    // ASSERT(index < EC_SPI_INDEX_LIMIT);
    if(ret == OPEN_HAL_IDLE)
    {
        if(index >= EC_SPI_INDEX_START && index < EC_SPI_INDEX_LIMIT)
        {
            sSpiUsrIdList[index] |= OPEN_HAL_STAT_MUSK;
            ret = OPEN_HAL_DONE;
        }
    }
    return ret;
}

/**
  \fn          
  \brief    上电初始化
  \return
*/
int api_spi_startup(void* para)
{
    for(int i=EC_SPI_INDEX_START;i<EC_SPI_INDEX_LIMIT;i++)
    {
        spi_set_free(i);
    }
}


/**
  \fn          
  \brief    
  \return
*/
uint32_t api_spi_create(uint32_t index,void *cfg)
{
    uint32_t usrId = spi_set_idle(index);  
    if(usrId)
    {
        if(cfg != NULL)
        {
        } 
        SYSLOG_INFO("spi%d[0x%08X]\r\n",index,usrId); 
    }
    return index;
}
/**
  \fn          
  \brief        
  \return 
*/
api_ret_t api_spi_delete(uint32_t usrId)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_spi_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_IDLE)
    {
        spi_set_free(index);
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
api_ret_t api_spi_open(uint32_t usrId,void *cfg,size_t timeout)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_spi_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_IDLE)
    {
        if(cfg != NULL)
        {

        } 
        spi_set_used(usrId);
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
api_ret_t api_spi_close(uint32_t usrId)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_spi_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_USED)
    {
        spi_set_idle(index);
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
api_ret_t api_spi_ioctl(uint32_t usrId,api_spi_ioctl_t type, uint32_t para)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_spi_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    index = usrId;
    // if(ret == OPEN_HAL_USED)
    {
        switch (type) {
            case OPEN_SPI_IOCTL_CLK:
                gSpiClk[index] = para;
                break;
            case OPEN_SPI_IOCTL_MODE:
                gSpiMode[index] = para;
                break;
            case OPEN_SPI_IOCTL_CONFIG:
                spiConfig(index);
                break;
            case OPEN_SPI_IOCTL_START_SEND:
                spiStartSend(index, *((void **)para), ((uint32_t *)para)[1]);
                break;
            case OPEN_SPI_IOCTL_STOP_SEND:
                spiStopSend(index);
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
api_ret_t api_spi_pmctl(uint32_t usrId,open_hal_pm_t *cfg, size_t count)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_spi_query(usrId);
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

static void spiReadBuffer(ARM_DRIVER_SPI *spiDrv, uint8_t *bufferIn, uint32_t length)
{
    uint32_t index = 0;
    uint32_t count = length / SPI_BUFFER_SIZE;
    uint32_t len   = length % SPI_BUFFER_SIZE;

    memset(gSpiBuffer, 0xFF, ((count > 0) ? SPI_BUFFER_SIZE : len));

    for (index=0; index<count; index++)
    {
        spiDrv->Transfer(gSpiBuffer, &bufferIn[index * SPI_BUFFER_SIZE], SPI_BUFFER_SIZE);
#ifdef SPI_DMA_ENABLE
        if (osEventFlagsWait(gSpiFlags, 0x01, osFlagsWaitAll, 100) == osErrorTimeout)
        {
            SYSLOG_DEBUG("SPI read timeout.\r\n");
            ECPLAT_PRINTF(UNILOG_PLA_APP, sdSpiReadBuffer1, P_DEBUG, "SPI read timeout.");
        }
#endif
    }

    if (len > 0)
    {
        spiDrv->Transfer(gSpiBuffer, &bufferIn[index * SPI_BUFFER_SIZE], len);
#ifdef SPI_DMA_ENABLE
        if ((len >= SPI_POLLING_DMA_SPLIT) && (osEventFlagsWait(gSpiFlags, 0x01, osFlagsWaitAll, 100) == osErrorTimeout))
        {
            SYSLOG_DEBUG("SPI read timeout.\r\n");
            ECPLAT_PRINTF(UNILOG_PLA_APP, sdSpiReadBuffer2, P_DEBUG, "SPI read timeout.");
        }
#endif
    }
}

static void spiWriteBuffer(ARM_DRIVER_SPI *spiDrv, uint8_t *bufferOut, uint32_t length)
{
    uint32_t index = 0;
    uint32_t count = length / SPI_BUFFER_SIZE;
    uint32_t len   = length % SPI_BUFFER_SIZE;

    for (index=0; index<count; index++)
    {
        spiDrv->Transfer(&bufferOut[index * SPI_BUFFER_SIZE], gSpiBuffer, SPI_BUFFER_SIZE);
#ifdef SPI_DMA_ENABLE
        if (osEventFlagsWait(gSpiFlags, 0x01, osFlagsWaitAll, 100) == osErrorTimeout)
        {
            SYSLOG_DEBUG("SPI write timeout.\r\n");
            ECPLAT_PRINTF(UNILOG_PLA_APP, sdSpiWriteBuffer1, P_DEBUG, "SPI write timeout.");
        }
#endif
    }

    if (len > 0)
    {
        spiDrv->Transfer(&bufferOut[index * SPI_BUFFER_SIZE], gSpiBuffer, len);
#ifdef SPI_DMA_ENABLE
        if ((len >= SPI_POLLING_DMA_SPLIT) && (osEventFlagsWait(gSpiFlags, 0x01, osFlagsWaitAll, 100) == osErrorTimeout))
        {
            SYSLOG_DEBUG("SPI write timeout.\r\n");
            ECPLAT_PRINTF(UNILOG_PLA_APP, sdSpiWriteBuffer2, P_DEBUG, "SPI write timeout.");
        }
#endif
    }
}

static uint8_t spiReadWriteByte(ARM_DRIVER_SPI *spiDrv, uint8_t dataOut)
{
    uint8_t dataIn = 0;

    spiDrv->Transfer(&dataOut, &dataIn, 1);

    return dataIn;
}

api_ret_t api_spi_write(uint32_t usrId, void* buf, size_t count)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_spi_query(usrId);
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

api_ret_t api_spi_read(uint32_t usrId, void* buf, size_t count)
{
    api_ret_t ret = api_spi_query(usrId);
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

uint8_t api_spi_read_write_byte(uint32_t usrId, uint8_t dataOut)
{
    uint8_t dataIn = 0;
    api_ret_t ret = api_spi_query(usrId);
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
