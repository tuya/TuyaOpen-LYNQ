/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_uart.c
 * Description:  openhal uart entry source file
 * History:      Rev1.0   2024-02-23
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "Driver_Common.h"
#include "system_ec7xx.h"
#include DEBUG_LOG_HEADER_FILE
#include "bsp.h"
#include "bsp_usart.h"
#include "devicemanager.h"
#include "api_comm.h"
#include "api_uart.h"

static uint32_t sUartUsrIdList[EC_UART_INDEX_LIMIT] = {0};
static uint16_t sUartUsrIdSeed[EC_UART_INDEX_LIMIT] = {0};
static IsrFunc sUartIsrFunc[EC_UART_INDEX_LIMIT] = {NULL};

//D16-31:control  x0001   
//D0-15:baudrate  x0480=115200
static uint32_t sUartStartupList[EC_UART_INDEX_LIMIT] = {
    0x0, 0x0, 0x10060, 0x10480
};
static uint32_t sUartRuntimeList[EC_UART_INDEX_LIMIT] = {0xFFFFFFFF};

extern ARM_DRIVER_USART Driver_USART0;
extern ARM_DRIVER_USART Driver_USART1;
#if (RTE_UART2)
extern ARM_DRIVER_USART Driver_USART2;
#endif //(RTE_UART2)
// extern ARM_DRIVER_USART Driver_USART3;
static ARM_DRIVER_USART *uartDrv[EC_UART_INDEX_LIMIT] = {
    &Driver_USART0, 
    &Driver_USART1,
#if (RTE_UART2)
    &Driver_USART2, 
#endif //(RTE_UART2)
    // &Driver_USART3,
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
    if(index >= EC_UART_INDEX_START && index < EC_UART_INDEX_LIMIT)
    {
        if(index == (sUartUsrIdList[index] & OPEN_HAL_PORT_MUSK))
        {
            return index;
        }
    }
    return EC_UART_INDEX_LIMIT;
}

/**
  \fn         api_uart_query()    
  \brief      查询状态
  \return     状态枚举
*/
api_ret_t api_uart_query(uint32_t usrId)
{
    ASSERT(usrId > 0);
    uint32_t index = usrId_to_index(usrId);
    // ASSERT(index < EC_UART_INDEX_LIMIT);
    // printf("\r\nusrId[x%08X]:%d < %d",usrId,index,EC_UART_INDEX_LIMIT); 
    if(index >= EC_UART_INDEX_START && index < EC_UART_INDEX_LIMIT)
    {
        if(sUartUsrIdList[index] == OPEN_HAL_STAT_UNUSED)
        {
            return OPEN_HAL_FREE;
        }
        else if(sUartUsrIdList[index] & OPEN_HAL_STAT_MUSK)
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
  \brief    中断处理  
  \return
*/
static void api_uart2_isr_handle(uint32_t event)
{
    if(sUartIsrFunc[2] != NULL){
        sUartIsrFunc[2](event);
    }
}
static void api_uart3_isr_handle(uint32_t event)
{
    if(sUartIsrFunc[3] != NULL){
        sUartIsrFunc[3](event);
    }
}
/**
  \fn          
  \brief     
  \return
*/
static uint8_t uart_control_set(uint32_t index,uint16_t control)
{
    uint8_t ret = 0;
    uint32_t baudrate = 100 * (sUartRuntimeList[index] & 0x0000FFFF);
    uint16_t contrbit = sUartRuntimeList[index] > 16;
    if(uartDrv[index]!=NULL){
        if(contrbit != control){
            uartDrv[index]->Control(control,baudrate);
            sUartRuntimeList[index] &= 0x0000FFFF;
            sUartRuntimeList[index] |= (control < 16);
            ret = 1;
        }
    }
    SYSLOG_INFO("[%d][x%08X],%d,%d\r\n",index,uartDrv[index],control,ret);
    return ret;
}
/**
  \fn          
  \brief     
  \return
*/
static uint8_t uart_baudrate_set(uint32_t index,uint32_t value)
{
    uint8_t ret = 0;
    uint32_t baudrate = 100 * (sUartRuntimeList[index] & 0x0000FFFF);
    uint16_t contrbit = sUartRuntimeList[index] > 16;
    if(uartDrv[index]!=NULL){
        if(baudrate != value){
            uartDrv[index]->Control(contrbit,value);
            sUartRuntimeList[index] &= 0xFFFF0000;
            sUartRuntimeList[index] += (value/100);
            ret = 1;
        }
    }
    SYSLOG_INFO("[%d][x%08X],%d,%d\r\n",index,uartDrv[index],value,ret); 
    return ret;
}

// // Pin initialize
// PadConfig_t config;
// PAD_getDefaultConfig(&config);

// config.mux = usart->pins.pin_tx->funcNum;
// PAD_setPinConfig(usart->pins.pin_tx->pinNum, &config);

// config.pullSelect = PAD_PULL_INTERNAL;
// config.pullUpEnable = PAD_PULL_UP_ENABLE;
// config.pullDownEnable = PAD_PULL_DOWN_DISABLE;
// config.mux = usart->pins.pin_rx->funcNum;

// PAD_setPinConfig(usart->pins.pin_rx->pinNum, &config);

// if(usart->pins.pin_cts)
// {
//     config.mux = usart->pins.pin_cts->funcNum;
//     PAD_setPinConfig(usart->pins.pin_cts->pinNum, &config);
// }

// if(usart->pins.pin_rts)
// {
//     config.mux = usart->pins.pin_rts->funcNum;
//     PAD_setPinConfig(usart->pins.pin_rts->pinNum, &config);
// }
static int32_t uart_Initialize(ARM_USART_SignalEvent_t cb_event, USART_RESOURCES *usart)
{
    int32_t returnCode;
    if (usart->info->flags & USART_FLAG_INITIALIZED) return ARM_DRIVER_OK;

    // Initialize USART run-time resources
    usart->info->cb_event = cb_event;
    memset(&(usart->info->rx_status), 0, sizeof(USART_STATUS));
    usart->info->xfer.send_active           = 0U;
    usart->info->xfer.tx_def_val            = 0U;
    if (usart->dma_tx)
    {
        returnCode = DMA_openChannel(usart->dma_tx->instance);
        if (returnCode == ARM_DMA_ERROR_CHANNEL_ALLOC)
            return ARM_DRIVER_ERROR;
        else
            usart->dma_tx->channel = returnCode;

        DMA_setChannelRequestSource(usart->dma_tx->instance, usart->dma_tx->channel, (DmaRequestSource_e)usart->dma_tx->request);
        DMA_rigisterChannelCallback(usart->dma_tx->instance, usart->dma_tx->channel, usart->dma_tx->callback);
    }
    if (usart->dma_rx)
    {
        returnCode = DMA_openChannel(usart->dma_rx->instance);

        if (returnCode == ARM_DMA_ERROR_CHANNEL_ALLOC)
            return ARM_DRIVER_ERROR;
        else
            usart->dma_rx->channel = returnCode;

        DMA_setChannelRequestSource(usart->dma_rx->instance, usart->dma_rx->channel, (DmaRequestSource_e)usart->dma_rx->request);
        DMA_rigisterChannelCallback(usart->dma_rx->instance, usart->dma_rx->channel, usart->dma_rx->callback);
    }
    usart->info->flags = USART_FLAG_INITIALIZED;  // USART is initialized
    return ARM_DRIVER_OK;
}

/**
  \fn          
  \brief   默认状态配置
  \return
*/
static uint32_t uart_set_free(uint32_t index)
{
    if(index >= EC_UART_INDEX_START && index < EC_UART_INDEX_LIMIT)
    {
        sUartUsrIdList[index] = OPEN_HAL_STAT_UNUSED;
        sUartIsrFunc[index] = NULL;
        sUartRuntimeList[index] = sUartStartupList[index];
        if(uartDrv[index] != NULL)
        {
            // uartDrv[index]->PowerControl(ARM_POWER_OFF);
            // uartDrv[index]->Uninitialize();
            uint32_t baudrate = 100 * (sUartRuntimeList[index] & 0x0000FFFF);
            uint16_t contrbit = sUartRuntimeList[index] > 16;
            if(baudrate> 1200){
                if(index==2) uartDrv[index]->Initialize(api_uart2_isr_handle);
                else if(index==3) uartDrv[index]->Initialize(api_uart3_isr_handle);
                else uartDrv[index]->Initialize(NULL);
                uartDrv[index]->PowerControl(ARM_POWER_FULL);
                uartDrv[index]->Control(contrbit,baudrate);
            }
            SYSLOG_INFO("[x%08X]x%04X-%d\r\n",uartDrv[index],contrbit,baudrate);
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
static uint32_t uart_set_idle(uint32_t index)
{
    uint32_t usrId = 0;
    if(index >= EC_UART_INDEX_START && index < EC_UART_INDEX_LIMIT)
    {
        if(sUartUsrIdList[index] == OPEN_HAL_STAT_UNUSED){
            sUartUsrIdList[index] = index;
            sUartUsrIdSeed[index] ++;
            sUartUsrIdList[index] |= (uint32_t)(sUartUsrIdSeed[index] << 16);
            usrId = sUartUsrIdList[index];
            // SYSLOG_INFO("uart_set_idle[x%X]:0x%08X\r\n",index,sUartUsrIdList[index]);
        }
        else if(sUartUsrIdList[index] & OPEN_HAL_STAT_MUSK){
            sUartUsrIdList[index] &= ~(OPEN_HAL_STAT_MUSK);
            usrId = sUartUsrIdList[index];
        }
    }
    return usrId;
}

/**
  \fn          
  \brief    
  \return
*/
static uint32_t uart_set_used(uint32_t usrId)
{
    EC_ASSERT(usrId > 0,0,0,0);
    api_ret_t ret = api_uart_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    // ASSERT(index < EC_UART_INDEX_LIMIT);
    if(ret == OPEN_HAL_IDLE)
    {
        if(index >= EC_UART_INDEX_START && index < EC_UART_INDEX_LIMIT)
        {
            sUartUsrIdList[index] |= OPEN_HAL_STAT_MUSK;
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
int api_uart_startup(void* para)
{
    for(int i=EC_UART_INDEX_START;i<EC_UART_INDEX_LIMIT;i++)
    {
        uart_set_free(i);
    }
}


/**
  \fn          
  \brief    
  \return
*/
uint32_t api_uart_create(uint32_t index,void *cfg)
{
    uint32_t usrId = uart_set_idle(index);  
    if(usrId)
    {
        if(cfg != NULL)
        {
        } 
        SYSLOG_INFO("uart%d[0x%08X]\r\n",index,usrId); 
    }
    return usrId;
}
/**
  \fn          
  \brief        
  \return 
*/
api_ret_t api_uart_delete(uint32_t usrId)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_uart_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_IDLE)
    {
        uart_set_free(index);
        ret = OPEN_HAL_DONE;
    }
    SYSLOG_INFO("uart%d[0x%08X]:%d\r\n",index,usrId,ret); 
    return ret;
}

/**
  \fn          
  \brief        
  \return
*/
api_ret_t api_uart_open(uint32_t usrId,void *cfg,size_t timeout)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_uart_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_IDLE)
    {
        if(cfg != NULL)
        {

        } 
        uart_set_used(usrId);
        ret = OPEN_HAL_DONE;
    }
    SYSLOG_INFO("uart%d[0x%08X]:0x%X\r\n",index,usrId,ret); 
    // printf("\r\ni2c%d[0x%08X]open:0x%X",index,usrId,ret); 
    return ret;
}

/**
  \fn          
  \brief        
  \return
*/
api_ret_t api_uart_close(uint32_t usrId)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_uart_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_USED)
    {
        uart_set_idle(index);
        ret = OPEN_HAL_DONE;
    }
    SYSLOG_INFO("uart%d[0x%08X]:ret %d\r\n",index,usrId,ret); 
    return ret;
}
/**
  \fn          
  \brief        
  \return
*/
api_ret_t api_uart_ioctl(uint32_t usrId,api_uart_ioctl_t type, void *para)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_uart_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_USED)
    {
        switch (type) {
            case OPEN_UART_IOCTL_ISR_CB:
                sUartIsrFunc[index] = (IsrFunc *)para;
                break;
            case OPEN_UART_IOCTL_BAUDRATE:
                uart_baudrate_set(index,*(uint32_t *)para);
                break;
            case OPEN_UART_IOCTL_CONTROL:
                uart_control_set(index,*(uint16_t *)para);
                break;
            case GET_RX_COUNT:
                if (uartDrv[index] != NULL)
                {
                    uint32_t count = uartDrv[index]->GetRxCount();
                    memcpy(para, &count, sizeof(uartDrv[index]->GetRxCount()));
                }
            default:
                break;
        }
        ret = OPEN_HAL_DONE;
    }
    SYSLOG_INFO("[0x%08X],%d,0x%08X,%d\r\n",usrId,type,sUartRuntimeList[index],ret);
    return ret;
}
/**
  \fn          
  \brief    
  \return
*/
api_ret_t api_uart_pmctl(uint32_t usrId,open_hal_pm_t *cfg, size_t count)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_uart_query(usrId);
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
/**
  \fn          
  \brief  polling mode    
  \return
*/
api_ret_t api_uart_write(uint32_t usrId, void* buf, size_t count)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_uart_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_USED)
    { 
        if(buf != NULL && count){
            uartDrv[index]->SendPolling(buf,count);
            // uartDrv[index]->Send(buf,count);
        }
        ret = OPEN_HAL_DONE;
    }
    SYSLOG_INFO("[x%08X][x%08X],%d,%d\r\n",usrId,uartDrv[index],count,ret);
    return ret;
}

/**
  \fn          
  \brief    
  \return
*/
api_ret_t api_uart_read(uint32_t usrId, void* buf, size_t count)
{
    api_ret_t ret = api_uart_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_USED)
    { 
        if(buf != NULL && count){
            uartDrv[index]->Receive(buf,count);
        }
        ret = OPEN_HAL_DONE;
    }
    SYSLOG_INFO("[x%08X][x%08X],%d,%d\r\n",usrId,uartDrv[index],count,ret);
    return ret;
}
/**
  \fn          
  \brief    
  \return
*/

static void test_uart_cb(uint32_t event)
{
    if(event & ARM_USART_EVENT_RX_TIMEOUT)
    {
  
    }
    if(event & ARM_USART_EVENT_RECEIVE_COMPLETE)
    {
      
    }
    if(event & ARM_USART_EVENT_SEND_COMPLETE)
    {
   
    }
    if(event & (ARM_USART_EVENT_RX_BREAK | ARM_USART_EVENT_RX_FRAMING_ERROR | ARM_USART_EVENT_RX_PARITY_ERROR))
    {
   
    }
}

int api_test_uart(void)
{ 
    // uint32_t test_uart_id = 0;
    // for(int i=EC_UART_INDEX_START;i<EC_UART_INDEX_LIMIT;i++)
    // {
    //     test_uart_id = api_uart_create(i,NULL);
    //     EC_API_CHECK(api_uart_open(test_uart_id,NULL,0));
    //     EC_API_CHECK(api_uart_ioctl(test_uart_id,OPEN_UART_IOCTL_ISR_CB,test_uart_cb));
    //     uint32_t baudrate = 9600;
    //     EC_API_CHECK(api_uart_ioctl(test_uart_id,OPEN_UART_IOCTL_BAUDRATE,&baudrate));
    //     uint8_t buffer[] = {0x12,0x34,0x56,0x78,0};
    //     EC_API_CHECK(api_uart_write(test_uart_id,buffer,sizeof(buffer)));
    //     EC_API_CHECK(api_uart_read(test_uart_id,buffer,sizeof(buffer)));
    //     open_hal_pm_t cfg = {0};
    //     EC_API_CHECK(api_uart_pmctl(test_uart_id,&cfg,0));
    //     EC_API_CHECK(api_uart_close(test_uart_id));
    //     EC_API_CHECK(api_uart_delete(test_uart_id));
    // }
}
