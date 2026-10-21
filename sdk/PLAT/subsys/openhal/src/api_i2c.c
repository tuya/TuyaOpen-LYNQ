/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_i2c.c
 * Description:  openhal i2c entry source file
 * History:      Rev1.0   2024-02-23
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "Driver_Common.h"
#include DEBUG_LOG_HEADER_FILE
#include "system_ec7xx.h"
#include "bsp.h"
#include "bsp_i2c.h"
#include "devicemanager.h"
#include "api_i2c.h"
#include "api_comm.h"

#if (RTE_I2C0_IO_MODE == DMA_MODE)
#error "DMA mode is not supported"
#endif
#if (RTE_I2C1_IO_MODE == DMA_MODE)
#error "DMA mode is not supported"
#endif

#define EPAT_LOG(subId, debugLevel, format, ...)  \
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, subId, debugLevel, format, ##__VA_ARGS__)

#define I2C_USE_HAL_LOCK

#ifdef I2C_USE_HAL_LOCK
#if (RTE_I2C0_IO_MODE == IRQ_MODE)
#error "IRQ mode is not supported"
#endif
#endif // HAL I2C with polling mode

static bool _hal_i2c_idle[EC_I2C_INDEX_LIMIT] = {true};
static uint32_t sI2cUsrIdList[EC_I2C_INDEX_LIMIT] = {0};
static uint16_t sI2cUsrIdSeed[EC_I2C_INDEX_LIMIT] = {0};
static IsrFunc sI2cIsrFunc[EC_I2C_INDEX_LIMIT] = {NULL};
//D6-7:PowerControl
//D4-5:
//D0-3:ARM_I2C_BUS_SPEED
static uint8_t sI2cStartupList[EC_I2C_INDEX_LIMIT] = {
    0x0, 0x0,
};
static uint8_t sI2cRuntimeList[EC_I2C_INDEX_LIMIT] = {
    0x0, 0x0,
};
// #define ARM_I2C_BUS_SPEED_STANDARD      (0x01)      ///< Standard Speed (100kHz)
// #define ARM_I2C_BUS_SPEED_FAST          (0x02)      ///< Fast Speed     (400kHz)
// #define ARM_I2C_BUS_SPEED_FAST_PLUS     (0x03)      ///< Fast+ Speed    (  1MHz)
// #define ARM_I2C_BUS_SPEED_HIGH          (0x04)      ///< High Speed     (3.4MHz)
static uint16_t i2cSpeedList[]={0,100,400,1000,3400};
static osMutexId_t sI2cMutex[EC_I2C_INDEX_LIMIT] = {NULL,NULL};
#if (RTE_I2C0)
extern AP_PLAT_COMMON_DATA ARM_DRIVER_I2C Driver_I2C0;
#endif
#if (RTE_I2C1)
extern AP_PLAT_COMMON_DATA ARM_DRIVER_I2C Driver_I2C1;
#endif
static ARM_I2C_STATUS   i2cStatus = {0};
volatile uint32_t i2c0_risr_reg = 0;//i2c->reg->RISR;
volatile uint32_t i2c1_risr_reg = 0;//i2c->reg->RISR;
static ARM_DRIVER_I2C *i2cDrv[EC_I2C_INDEX_LIMIT] = 
{
    #if (RTE_I2C0)
    &Driver_I2C0, 
    #endif
    #if (RTE_I2C1)
    &Driver_I2C1,
    #endif
};
/**
  \fn
  \brief    为了兼容，只能在使用一路I2C的情况下使用
  \return
*/
#include "hal_i2c.h"
extern AP_PLAT_COMMON_BSS HalI2cPamram_t halI2cParam;
extern int32_t halI2cInit(uint8_t bus_num,bool needLock); //mbtk change
extern int halI2cLock(void* arg);
extern int halI2cUnlock(void* arg);
static bool hal_i2c0_used = false;
/**
  \fn
  \brief
  \return
*/
static uint32_t usrId_to_index(uint32_t usrId)
{ 
    uint32_t index = (uint32_t)(usrId & OPEN_HAL_PORT_MUSK); 
    if(index == (sI2cUsrIdList[index] & OPEN_HAL_PORT_MUSK))
    {
        return index;
    }
    return EC_I2C_INDEX_LIMIT;
}

/**
  \fn         api_i2c_query()    
  \brief      查询状态
  \return     状态枚举
*/
api_ret_t api_i2c_query(uint32_t usrId)
{
    // ASSERT(usrId > 0);
    uint32_t index = usrId_to_index(usrId);
    if(index >= EC_I2C_INDEX_START && index < EC_I2C_INDEX_LIMIT)
    {
        if(sI2cUsrIdList[index] == OPEN_HAL_STAT_UNUSED)
        {
            return OPEN_HAL_FREE;
        }
        else if(sI2cUsrIdList[index] & OPEN_HAL_STAT_MUSK)
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
  \brief    获取事件组  
  \return
*/
static void api_i2c0_isr_handle(uint32_t event)
{
    // i2c0_risr_reg = I2C0->RISR;
    if(event & ARM_I2C_EVENT_TRANSFER_DONE)
    {
        #ifdef FEATURE_OS_ENABLE
        if(event & ARM_I2C_EVENT_TRANSFER_INCOMPLETE)
        {
            osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C0_BUS_ERR));
            // osEventFlagsSet(halEventFlags_i2c[0], (1U << EVT_I2C_INCOMPLETE));
        }
        if(event & ARM_I2C_EVENT_ADDRESS_NACK)
        {
            osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C0_BUS_ERR));
            // osEventFlagsSet(halEventFlags_i2c[0], (1U << EVT_I2C_SLAVE_NACK));
        }
        if(event & ARM_I2C_EVENT_ARBITRATION_LOST)
        {
            osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C0_BUS_ERR));
            // osEventFlagsSet(halEventFlags_i2c[0], (1U << EVT_I2C_ARBI_LOST));
        }
        if(event & ARM_I2C_EVENT_BUS_ERROR)
        {
            osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C0_BUS_ERR));
            // osEventFlagsSet(halEventFlags_i2c[0], (1U << EVT_I2C_BUS_ERROR));
        }
        #endif
        if(event == ARM_I2C_EVENT_TRANSFER_DONE){
            osEventFlagsSet(successEvent, (1U << TRANS_EVT_I2C0_IRQ));
            if(sI2cIsrFunc[0] != NULL){
                sI2cIsrFunc[0](event);
            }
        }   //ERROR with event 0x11 if IRQ mode
    }
    EPAT_LOG(i2c0_isr, P_INFO, "event 0x%X", event);
}

/**
  \fn          
  \brief    中断处理  
  \return
*/

static void api_i2c1_isr_handle(uint32_t event)
{
    // i2c1_risr_reg = I2C1->RISR;
    if(event & ARM_I2C_EVENT_TRANSFER_DONE)
    {
        #ifdef FEATURE_OS_ENABLE
        if(event & ARM_I2C_EVENT_TRANSFER_INCOMPLETE)
        {
            osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C1_BUS_ERR));
            // osEventFlagsSet(halEventFlags_i2c[1], (1U << EVT_I2C_INCOMPLETE));
        }
        if(event & ARM_I2C_EVENT_ADDRESS_NACK)
        {
            osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C1_BUS_ERR));
            // osEventFlagsSet(halEventFlags_i2c[1], (1U << EVT_I2C_SLAVE_NACK));
        }
        if(event & ARM_I2C_EVENT_ARBITRATION_LOST)
        {
            osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C1_BUS_ERR));
            // osEventFlagsSet(halEventFlags_i2c[1], (1U << EVT_I2C_ARBI_LOST));
        }
        if(event & ARM_I2C_EVENT_BUS_ERROR)
        {
            osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C1_BUS_ERR));
            // osEventFlagsSet(halEventFlags_i2c[1], (1U << EVT_I2C_BUS_ERROR));
        }
        #endif
        if(event == ARM_I2C_EVENT_TRANSFER_DONE){
            osEventFlagsSet(successEvent, (1U << TRANS_EVT_I2C1_IRQ));
            if(sI2cIsrFunc[1] != NULL){
                sI2cIsrFunc[1](event);
            }
        }
    }
    EPAT_LOG(i2c1_isr, P_INFO, "event 0x%X", event);
}
/**
  \fn
  \brief
  \return
*/
uint32_t i2c_fault_wait(uint32_t usrId,size_t timeout)
{
    uint32_t index = usrId_to_index(usrId);
    ARM_I2C_STATUS status = i2cDrv[index]->GetStatus();
    uint32_t flag = 0;
    uint32_t stat = 0;
    if(index==0){
        #if (RTE_I2C1_IO_MODE == IRQ_MODE)
        flag = osEventFlagsWait(hwfaultEvent, (1U << FAULT_I2C0_BUS_ERR), osFlagsWaitAny, timeout);
        #endif
        i2c0_risr_reg = I2C0->RISR;
        stat = i2c0_risr_reg;
    } 
    else if(index==1){
        #if (RTE_I2C1_IO_MODE == IRQ_MODE)
        flag = osEventFlagsWait(hwfaultEvent, (1U << FAULT_I2C1_BUS_ERR), osFlagsWaitAny, timeout);
        #endif
        i2c1_risr_reg = I2C1->RISR;
        stat = i2c1_risr_reg;
    } 
    if (flag < FAULT_EVT_TOTAL) {
        EPAT_LOG(i2c_fault_found, P_INFO, "i2c%d:flag 0x%X,stat 0x%X,risr 0x%X,ThreadId 0x%x",\
            index,flag,status,stat,osThreadGetId());
    }
    return stat;
}

/**
  \fn
  \brief
  \return
*/
static uint32_t i2c_master_tx(uint32_t index,uint32_t addr,uint8_t *data,uint32_t num,bool xfer,uint32_t timeout)
{
    uint32_t time_used = 0;
    uint32_t time_mark = osKernelGetTickCount();
    ARM_DRIVER_I2C *tpBus = i2cDrv[index];
    if(tpBus){
        if (osMutexAcquire(sI2cMutex[index], timeout) == osOK)
        {
            ARM_I2C_STATUS status = tpBus->GetStatus();
            if(index==0){
                #if (RTE_I2C0_IO_MODE == IRQ_MODE)
                osEventFlagsClear(successEvent, (1U << TRANS_EVT_I2C0_IRQ));
                #else
                if(status.bus_error){
                    osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C0_BUS_ERR));
                    EPAT_LOG(i2c0_master_tx_1, P_ERROR, "status 0x%x",status);
                }
                #endif
            }
            else if(index==1){
                #if (RTE_I2C1_IO_MODE == IRQ_MODE)
                osEventFlagsClear(successEvent, (1U << TRANS_EVT_I2C1_IRQ));
                #else
                if(status.bus_error){
                    osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C0_BUS_ERR));
                    EPAT_LOG(i2c1_master_tx_1, P_ERROR, "status 0x%x",status);
                }
                #endif
            }
            tpBus->MasterTransmit(addr, data, num, xfer);
            if(index==0){
                #if (RTE_I2C0_IO_MODE == IRQ_MODE)
                osEventFlagsWait(successEvent, (1U << TRANS_EVT_I2C0_IRQ), osFlagsNoClear, timeout);
                EPAT_LOG(i2c0_master_tx_2, P_INFO, "timeout %d",timeout);
                #else
                status = tpBus->GetStatus();
                if(status.bus_error){
                    osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C0_BUS_ERR));
                    EPAT_LOG(i2c0_master_tx_2, P_ERROR, "status 0x%x",status);
                }
                #endif
            }
            else if(index==1){
                #if (RTE_I2C1_IO_MODE == IRQ_MODE)
                osEventFlagsWait(successEvent, (1U << TRANS_EVT_I2C1_IRQ), osFlagsNoClear, timeout);
                EPAT_LOG(i2c1_master_tx_2, P_INFO, "timeout %d",timeout);
                #else
                if(status.bus_error){
                    osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C0_BUS_ERR));
                    EPAT_LOG(i2c1_master_tx_2, P_ERROR, "status 0x%x",status);
                }
                #endif
            }
        }
        osMutexRelease(sI2cMutex[index]);
        time_used = (osKernelGetTickCount()-time_mark);
        if(time_used >= 1000 || time_used > timeout){
            if(index==1) osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C1_TIMEOUT));
            else osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C0_TIMEOUT));
            EPAT_LOG(i2c_master_tx_timeout, P_ERROR, "i2c%d used %d >= %d",index,time_used,timeout);
        }
    }
    // EPAT_LOG(i2c_master_tx, P_INFO, "used %d",(osKernelGetTickCount()-time_mark));
    return time_used;
}
/**
  \fn
  \brief
  \return
*/
static uint32_t i2c_master_rx(uint32_t index,uint32_t addr,uint8_t *data,uint32_t num,bool xfer,uint32_t timeout)
{
    uint32_t time_used = 0;
    uint32_t time_mark = osKernelGetTickCount();
    ARM_DRIVER_I2C *tpBus = i2cDrv[index];
    if(tpBus){
        if (osMutexAcquire(sI2cMutex[index], timeout) == osOK)
        {
            ARM_I2C_STATUS status = tpBus->GetStatus();
            if(index==0){
                #if (RTE_I2C0_IO_MODE == IRQ_MODE)
                osEventFlagsClear(successEvent, (1U << TRANS_EVT_I2C0_IRQ));
                #else
                if(status.bus_error){
                    osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C0_BUS_ERR));
                    EPAT_LOG(i2c0_master_rx_1, P_ERROR, "status 0x%x",status);
                }
                #endif
            }
            else if(index==1){
                #if (RTE_I2C1_IO_MODE == IRQ_MODE)
                osEventFlagsClear(successEvent, (1U << TRANS_EVT_I2C1_IRQ));
                #else
                if(status.bus_error){
                    osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C1_BUS_ERR));
                    EPAT_LOG(i2c1_master_rx_1, P_ERROR, "status 0x%x",status);
                }
                #endif
            }
            tpBus->MasterReceive(addr, data, num, xfer);
            if(index==0){
                #if (RTE_I2C0_IO_MODE == IRQ_MODE)
                osEventFlagsWait(successEvent, (1U << TRANS_EVT_I2C0_IRQ), osFlagsNoClear, timeout);
                EPAT_LOG(i2c0_master_rx_2, P_INFO, "timeout %d",timeout);
                #else
                status = tpBus->GetStatus();
                if(status.bus_error){
                    osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C0_BUS_ERR));
                    EPAT_LOG(i2c0_master_rx_2, P_ERROR, "status 0x%x",status);
                }
                #endif
            }
            else if(index==1){
                #if (RTE_I2C1_IO_MODE == IRQ_MODE)
                osEventFlagsWait(successEvent, (1U << TRANS_EVT_I2C1_IRQ), osFlagsNoClear, timeout);
                EPAT_LOG(i2c1_master_rx_2, P_INFO, "timeout %d",timeout);
                #else
                status = tpBus->GetStatus();
                if(status.bus_error){
                    osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C1_BUS_ERR));
                    EPAT_LOG(i2c1_master_rx_2, P_ERROR, "status 0x%x",status);
                }
                #endif
            }
        }
        osMutexRelease(sI2cMutex[index]);
        time_used = (osKernelGetTickCount()-time_mark);
        if(time_used >= 1000 || time_used > timeout){
            if(index==1) osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C1_TIMEOUT));
            else osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C0_TIMEOUT));
            EPAT_LOG(i2c_master_rx_timeout, P_ERROR, "i2c%d used %d >= %d",index,time_used,timeout);
        }
    }
    return time_used;
}
/**
  \fn
  \brief
  \return
*/
static uint8_t i2c_speed_set(ARM_DRIVER_I2C *drv,uint8_t level)
{
    if(drv!=NULL && level){
        drv->Control(ARM_I2C_BUS_SPEED,level);
    }
    return 0;
}
/**
  \fn          
  \brief   默认状态配置
  \return
*/
//D6-7:PowerControl
//D4-5:
//D0-3:ARM_I2C_BUS_SPEED
static uint32_t i2c_set_free(uint32_t index)
{
    if(index >= EC_I2C_INDEX_START && index < EC_I2C_INDEX_LIMIT)
    {
        sI2cUsrIdList[index] = OPEN_HAL_STAT_UNUSED;
        sI2cIsrFunc[index] = NULL; //清除中断绑定回调 
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
static uint32_t i2c_set_idle(uint32_t index)
{
    ASSERT(index < EC_I2C_INDEX_LIMIT);
    uint32_t ret = 0;
    if(sI2cUsrIdList[index] == OPEN_HAL_STAT_UNUSED){
        sI2cUsrIdList[index] = index;
        sI2cUsrIdSeed[index] ++;
        sI2cUsrIdList[index] |= (uint32_t)(sI2cUsrIdSeed[index] << 16);
        ret = sI2cUsrIdList[index];
        // SYSLOG_INFO("i2c_set_idle[x%X]:0x%08X\r\n",index,sI2cUsrIdList[index]);
    }
    else if(sI2cUsrIdList[index] & OPEN_HAL_STAT_MUSK){
        sI2cUsrIdList[index] &= ~(OPEN_HAL_STAT_MUSK);
        ret = sI2cUsrIdList[index];
    }
    return ret;
}
/**
  \fn          
  \brief    
  \return
*/
static uint32_t i2c_set_used(uint32_t usrId)
{
    api_ret_t stat = api_i2c_query(usrId);
    if(stat != OPEN_HAL_IDLE){
        return 0;
    }
    uint32_t index = usrId_to_index(usrId);
    sI2cUsrIdList[index] |= OPEN_HAL_STAT_MUSK;
    return 1;
}


/**
  \fn          
  \brief   
  \return
*/

int api_i2c_startup(void* para)
{
    for(int i=EC_I2C_INDEX_START;i<EC_I2C_INDEX_LIMIT;i++)
    {
        i2c_set_free(i); 
    }
}
/**
  \fn
  \brief
  \return
*/
uint32_t api_i2c_create(uint32_t index,void *cfg)
{
    uint32_t usrId = i2c_set_idle(index);  
    if(usrId)
    {
        if(sI2cMutex[index] == NULL){
            sI2cMutex[index] = osMutexNew(NULL);
            osMutexRelease(sI2cMutex[index]);
        }
        #ifdef I2C_USE_HAL_LOCK
        if(halI2cParam.i2cDrv == NULL){
            halI2cInit(index,true); //mbtk change
            i2cDrv[index] = halI2cParam.i2cDrv;
            hal_i2c0_used = true;
        }
        else if(i2cDrv[index] == halI2cParam.i2cDrv){
            hal_i2c0_used = true;
        }
        #else
        if(i2cDrv[index] != NULL){
            i2cDrv[index]->Uninitialize();
            if(index==0) i2cDrv[index]->Initialize(api_i2c0_isr_handle);
            else if(index==1) i2cDrv[index]->Initialize(api_i2c1_isr_handle);
            i2cDrv[index]->PowerControl(ARM_POWER_FULL);
            sI2cRuntimeList[index] = ARM_I2C_BUS_SPEED_FAST;
            uint8_t speed = sI2cRuntimeList[index] & 0x07;
            i2cDrv[index]->Control(ARM_I2C_BUS_SPEED,speed);
            i2cDrv[index]->Control(ARM_I2C_BUS_CLEAR, 0);
        }
        #endif
    }
end:
    EPAT_LOG(api_i2c_create, P_INFO, "create i2c%d,0x%x,0x%x",index,sI2cMutex[index],i2cDrv[index]);
    return usrId;
}
/**
  \fn          
  \brief        
  \return 
*/
api_ret_t api_i2c_delete(uint32_t usrId)
{
    ASSERT(usrId > 0);
    api_ret_t ret = api_i2c_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    // ASSERT(index < EC_I2C_INDEX_LIMIT);
    if(ret == OPEN_HAL_IDLE && index < EC_I2C_INDEX_LIMIT)
    {
        if(i2c_set_free(index))
        {
        }
        ret = OPEN_HAL_DONE;
    }
end:
    EPAT_LOG(api_i2c_delete, P_INFO, "delete i2c%d", index);
    return ret;
}

/**
  \fn
  \brief
  \return
*/
api_ret_t api_i2c_open(uint32_t usrId,void *cfg,size_t timeout)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_i2c_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_IDLE && index < EC_I2C_INDEX_LIMIT)
    {
        if(cfg != NULL)
        {
 
        } 
        #ifdef I2C_USE_HAL_LOCK
        halI2cInit(index,true); //mbtk change
        #endif
        _hal_i2c_idle[index] = false;
        i2c_set_used(usrId);
        ret = OPEN_HAL_DONE;
    }
end:
    EPAT_LOG(api_i2c_open, P_INFO, "open i2c%d,ret %d", index,ret);
    return ret;
}
/**
  \fn
  \brief
  \return
*/
api_ret_t api_i2c_close(uint32_t usrId)
{
    ASSERT(usrId > 0);
    api_ret_t ret = api_i2c_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    // ASSERT(index < EC_I2C_INDEX_LIMIT);
    if(ret == OPEN_HAL_USED && index < EC_I2C_INDEX_LIMIT)
    {  
        i2c_set_idle(index);
        _hal_i2c_idle[index] = true;
        ret = OPEN_HAL_DONE;
    }
end:
    EPAT_LOG(api_i2c_close, P_INFO, "i2c%d,usrId 0x%X,0x%X,ret%d",index,usrId,sI2cUsrIdList[index],ret);
    return ret;
}
/**
  \fn
  \brief
  \return
*/
api_ret_t api_i2c_ioctl(uint32_t usrId,api_i2c_ioctl_t type, void *para)
{
    api_ret_t ret = api_i2c_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_USED)
    {
        uint8_t speed = sI2cRuntimeList[index] & 0x07;
        switch (type) {
            case OPEN_I2C_IOCTL_ISR_CB:
                sI2cIsrFunc[index] = (IsrFunc *)para;
                break;
            case OPEN_I2C_IOCTL_SPEED:
                speed = *(uint8_t *)para;
                i2c_speed_set(i2cDrv[index],speed);
                sI2cRuntimeList[index] &= 0xF8;
                sI2cRuntimeList[index] += speed;
                break;
            default:
                break;
        }
        ret = OPEN_HAL_DONE;
    }
end:
    EPAT_LOG(api_i2c_ioctl, P_INFO, "i2c%d,0x%X,type%d,0x%X,ret%d",index,usrId,type,sI2cRuntimeList[index],ret);
    // SYSLOG_INFO("[0x%08X],type%d,0x%02X,ret%d\r\n",usrId,type,sI2cRuntimeList[index],ret);
    return ret;
}
/**
  \fn          
  \brief    
  \return
*/
api_ret_t api_i2c_pmctl(uint32_t usrId,open_hal_pm_t *cfg, size_t count)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_i2c_query(usrId);
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
    EPAT_LOG(api_i2c_pmctl, P_INFO, "i2c%d,0x%X,ret%d",index,usrId,ret);
    return ret;
}
/**
  \fn
  \brief
  \return
*/
api_ret_t api_i2c_write(uint32_t usrId, void* buf, size_t count)
{
    uint32_t time_used[3] = {0};
    time_used[2] = osKernelGetTickCount();
    api_ret_t ret = api_i2c_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    #ifdef I2C_USE_HAL_LOCK
    halI2cInit(index,true); //mbtk change
    if (halI2cLock(halI2cParam.i2cSemId) == osError)
    {
        ret = OPEN_HAL_LOCK;
        goto end;
    }
    #endif
    if(ret == OPEN_HAL_USED)
    {
        if(buf != NULL){
            api_i2c_master_t  *_i2c = (api_i2c_master_t*)buf;
            time_used[0] = i2c_master_tx(index,_i2c->addr, &_i2c->reg, 1, true,100);
            time_used[1] = i2c_master_rx(index,_i2c->addr,_i2c->data,_i2c->num,false,100);
        }
        ret = OPEN_HAL_DONE;
    }
    #ifdef I2C_USE_HAL_LOCK
    halI2cUnlock(halI2cParam.i2cSemId);
    #endif
end:
    EPAT_LOG(api_i2c_write, P_INFO, "0x%X,i2c%d,ret%d,used %d",usrId,index,ret,(osKernelGetTickCount()-time_used[2]));
    return ret;
}

/**
  \fn          
  \brief    
  \return
*/
api_ret_t api_i2c_read(uint32_t usrId, void* buf, size_t count)
{
    uint32_t time_used[3] = {0};    //measure time
    time_used[2] = osKernelGetTickCount();
    api_ret_t ret = api_i2c_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    #ifdef I2C_USE_HAL_LOCK
    halI2cInit(index,true); //mbtk change
    int32_t lock = halI2cLock(halI2cParam.i2cSemId);
    if (lock == osError)
    {
        lock = halI2cLock(halI2cParam.i2cSemId);
        if (lock == osError) {
            ret = OPEN_HAL_LOCK;
            return ret;
        }
    }
    #endif
    if(ret == OPEN_HAL_USED)
    {
        if(buf != NULL){
            api_i2c_master_t  *_i2c = (api_i2c_master_t*)buf;
            time_used[0] = i2c_master_tx(index,_i2c->addr, &_i2c->reg, 1, true, 100);
            time_used[1] = i2c_master_rx(index,_i2c->addr,_i2c->data,_i2c->num,false,100);
        }
        ret = OPEN_HAL_DONE;
    }
    #ifdef I2C_USE_HAL_LOCK
    halI2cUnlock(halI2cParam.i2cSemId);
    #endif
    // EPAT_LOG(api_i2c_read, P_INFO, "0x%X,i2c%d,ret%d,used %d",usrId,index,ret,(osKernelGetTickCount()-time_used[2]));
    return ret;
}
/**
  \fn
  \brief
  \return
*/
api_ret_t api_i2c_read_axs(uint32_t usrId, void* buf, size_t count)
{
    uint32_t time_used[3] = {0};    //measure time
    time_used[2] = osKernelGetTickCount();
    api_ret_t ret = api_i2c_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    #ifdef I2C_USE_HAL_LOCK
    halI2cInit(index,true); //mbtk change
    int32_t lock = halI2cLock(halI2cParam.i2cSemId);
    if (lock == osError)
    {
        lock = halI2cLock(halI2cParam.i2cSemId);
        if (lock == osError) {
            ret = OPEN_HAL_LOCK;
            return ret;
        }
    }
    #endif
    if(ret == OPEN_HAL_USED)
    {
        uint8_t read_cmd[11] = {0xb5,0xab,0xa5,0x5a,0x00,0x00,0x00,0x08,0x00,0x00,0x00};
        if(buf != NULL){
            api_i2c_master_t  *_i2c = (api_i2c_master_t*)buf;
            uint16_t rd_len = _i2c->num;
            if (rd_len > 0){
                read_cmd[6] = (uint8_t)(rd_len>>8);     //high
                read_cmd[7] = (uint8_t)(rd_len&0xff);   //low
            }
            time_used[0] = i2c_master_tx(index,_i2c->addr, read_cmd, sizeof(read_cmd), false, 100);
            time_used[1] = i2c_master_rx(index,_i2c->addr,_i2c->data,_i2c->num, false, 100);
        }
        ret = OPEN_HAL_DONE;
    }
    #ifdef I2C_USE_HAL_LOCK
    halI2cUnlock(halI2cParam.i2cSemId);
    #endif
    // EPAT_LOG(api_i2c_read_axs, P_INFO, "0x%X,i2c%d,ret%d,used %d,%d,%d",usrId,index,ret,time_used[0],time_used[1],(osKernelGetTickCount()-time_used[2]));
    return ret;
}
/**
  \fn
  \brief
  \return
*/
api_ret_t api_i2c_read_tp(uint32_t usrId, void* buf, size_t count)
{
    api_ret_t ret = api_i2c_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    #ifdef I2C_USE_HAL_LOCK
    if (halI2cParam.i2cSemId == PNULL){
        halI2cInit(index,true); //mbtk change
    }
    int32_t lock = halI2cLock(halI2cParam.i2cSemId);
    if (lock == osError){
        lock = halI2cLock(halI2cParam.i2cSemId);
        if (lock == osError) {
            ret = OPEN_HAL_LOCK;
        }
    }
    #endif
    if(ret == OPEN_HAL_USED){
        if(buf != NULL){
            api_i2c_master_t  *i2c = (api_i2c_master_t*)buf;
            uint8_t reg[2] = {0x00,0x10};
            i2c_master_tx(index,i2c->addr, reg, 2, false, 100);
            i2c_master_rx(index,i2c->addr, i2c->data, i2c->num, false, 100);
        }
        ret = OPEN_HAL_DONE;
    }
    #ifdef I2C_USE_HAL_LOCK
    halI2cUnlock(halI2cParam.i2cSemId);
    #endif
}
/**
  \fn          
  \brief   测试完成需要释放对应资源
  \return
*/
static void test_i2c_cb(uint32_t event)
{
    // EPAT_LOG(test_i2c_cb, P_INFO, "event 0x%X", event);
}

int api_test_i2c(void)
{ 
    uint32_t test_i2c_id = 0;
    uint32_t index = 0;
    EPAT_LOG(api_test_i2c, P_INFO, "test i2c%d", index);
    // for(int i=EC_I2C_INDEX_START;index<EC_I2C_INDEX_LIMIT;index++)
    {
        test_i2c_id = api_i2c_create(index,NULL);
        EC_API_CHECK(api_i2c_open(test_i2c_id,NULL,1000));
        EC_API_CHECK(api_i2c_ioctl(test_i2c_id,OPEN_I2C_IOCTL_ISR_CB,test_i2c_cb));
        uint8_t speed = 2;
        EC_API_CHECK(api_i2c_ioctl(test_i2c_id,OPEN_I2C_IOCTL_SPEED,&speed));
        uint8_t temp[4]={0};    //test with minidkb tp ft6336
        api_i2c_master_t buffer = {
            .addr = 0x38,
            .reg = 0x02,    //read fingers
            .data = temp,
            .num = 1,
        };
        EC_API_CHECK(api_i2c_read(test_i2c_id,&buffer,sizeof(buffer)));
        // EC_API_CHECK(api_i2c_write(test_i2c_id,&buffer,sizeof(buffer)));
        // open_hal_pm_t cfg = {
        //     .mode = PM_LOWPOW,
        //     .runtime = RUNTIME_SUSPEND
        // };
        // EC_API_CHECK(api_i2c_pmctl(test_i2c_id,&cfg,sizeof(cfg)));
        EC_API_CHECK(api_i2c_close(test_i2c_id));
        EC_API_CHECK(api_i2c_delete(test_i2c_id));
    }
}