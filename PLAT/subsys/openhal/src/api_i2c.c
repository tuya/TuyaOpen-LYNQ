/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_i2c.c
 * Description:  openhal i2c entry source file
 * History:      Rev1.0   2024-02-23
 *
 ****************************************************************************/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Driver_Common.h"

#include DEBUG_LOG_HEADER_FILE
#include "Driver_I2C.h"
#include "api_comm.h"
#include "api_i2c.h"
#include "bsp.h"
#include "bsp_i2c.h"
#include "devicemanager.h"
#include "system_ec7xx.h"

#ifdef EPAT_HAL_DEBUG
#define EPAT_LOG(subId, debugLevel, format, ...)    \
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, subId, debugLevel, format, ##__VA_ARGS__)
#else
#define EPAT_LOG(subId, debugLevel, format, ...)
#endif

#if(RTE_I2C0_IO_MODE == DMA_MODE)
#error "DMA mode is not supported"
#endif
#if(RTE_I2C1_IO_MODE == DMA_MODE)
#error "DMA mode is not supported"
#endif

#define I2C_USE_HAL_LOCK
#ifdef I2C_USE_HAL_LOCK
#if(RTE_I2C0_IO_MODE == IRQ_MODE)
#error "IRQ mode is not supported"
#endif
#endif
/* ---------------------------------------------------------------------------------------------- */
typedef struct I2CCtx_
{
    ARM_DRIVER_I2C *i2c_dev[EC_I2C_INDEX_LIMIT+1];
    osMutexId_t i2c_mutex[EC_I2C_INDEX_LIMIT+1];
    bool i2c_idle[EC_I2C_INDEX_LIMIT+1];
    uint32_t usr_id[EC_I2C_INDEX_LIMIT+1];
    uint16_t usr_id_seed[EC_I2C_INDEX_LIMIT+1];
    IsrFunc isr_func[EC_I2C_INDEX_LIMIT+1];
    uint8_t speed[EC_I2C_INDEX_LIMIT+1];
    uint8_t slave_addr[EC_I2C_INDEX_LIMIT+1];
} I2CCtx_t;

static I2CCtx_t sI2cCtx = {.i2c_mutex = {NULL, NULL, NULL},
                           .i2c_dev = {NULL, NULL, NULL},
                           .i2c_idle = {true, true, true},
                           .usr_id = {0, 0, 0},
                           .usr_id_seed = {0, 0, 0},
                           .isr_func = {NULL, NULL, NULL},
                           .speed = {1, 1, 1},
                           .slave_addr = {0, 0, 0}};

#if(RTE_I2C0)
extern AP_PLAT_COMMON_DATA ARM_DRIVER_I2C Driver_I2C0;
#endif
#if(RTE_I2C1)
extern AP_PLAT_COMMON_DATA ARM_DRIVER_I2C Driver_I2C1;
#endif

volatile uint32_t i2c0_risr_reg = 0;  // i2c->reg->RISR;
volatile uint32_t i2c1_risr_reg = 0;  // i2c->reg->RISR;
/* ------------------------------------------ 资源查询表 ------------------------------------------ */
const int8_t i2c_SDA_PinTable[][3] = {
    {14, PAD_MUX_ALT2, 0},  //SWDIO1    I2C0_SDA
    {14, PAD_MUX_ALT3, 1},  //SWDIO1    I2C1_SDA
    {19, PAD_MUX_ALT2, 1},  //GPIO4     I2C1_SDA
    {23, PAD_MUX_ALT2, 1},  //GPIO8     I2C1_SDA
    {29, PAD_MUX_ALT2, 0},  //GPIO14    I2C0_SDA
    {31, PAD_MUX_ALT2, 0},  //GPIO16    I2C0_SDA
    {40, PAD_MUX_ALT2, 0},  //GPIO34    I2C0_SDA
    {43, PAD_MUX_ALT2, 1},  //GPIO37    I2C1_SDA
};
const int8_t i2c_SCL_PinTable[][3] = {
    {13, PAD_MUX_ALT2, 0},  //SWCLK1    I2C0_SCL
    {13, PAD_MUX_ALT3, 1},  //SWCLK1    I2C1_SCL
    {20, PAD_MUX_ALT2, 1},  //GPIO5     I2C1_SCL
    {24, PAD_MUX_ALT2, 1},  //GPIO9     I2C1_SCL
    {30, PAD_MUX_ALT2, 0},  //GPIO15    I2C0_SCL
    {32, PAD_MUX_ALT2, 0},  //GPIO17    I2C0_SCL
    {41, PAD_MUX_ALT2, 0},  //GPIO35    I2C0_SCL
    {42, PAD_MUX_ALT2, 1},  //GPIO36    I2C1_SCL
};
extern const int8_t list_pad2gpio[];
/* ----------------------------------------- 运行状态表 ------------------------------------------ */
// 增加运行表，从表中获取当前可用外设和状态：增加SW I2C2
AP_PLAT_COMMON_DATA static int8_t i2cList[EC_I2C_INDEX_LIMIT+1][4] = {
    {-1, ARM_I2C_BUS_SPEED_FAST, POLLING_MODE, -1},         /* I2C0: 400kHz/POLLING */
    {-1, ARM_I2C_BUS_SPEED_FAST, POLLING_MODE, -1},         /* I2C1: 400kHz/POLLING */
    {-1, ARM_I2C_BUS_SPEED_FAST, POLLING_MODE, -1}          /* I2C2: 400kHz/POLLING */
};
// 如果第一项为-1表示该I2C没有正确初始化，使用前需要先创建I2C
// ARM_I2C_BUS_SPEED_FAST = 400kHz

/**
  \fn
  \brief    用于确认i2c的引脚是否配置有效
  \return
*/
int8_t ec_i2c_checkout(uint8_t index, int8_t sda, int8_t scl)
{
    int8_t select = -1;
    int8_t mux = -1;
    mux = mux;
    if(index >= EC_I2C_INDEX_LIMIT){
        // 软件模式I2C
        if(check_pad_mux(sda, PAD_MUX_ALT0) == OPEN_HAL_DONE)
        {
            mux = PAD_MUX_ALT0;
            if(check_pad_mux(scl, PAD_MUX_ALT0) == OPEN_HAL_DONE){
                select = index;
            }
        }
    }
    else {
        for (uint8_t i = 0; i < sizeof(i2c_SDA_PinTable) / sizeof(i2c_SDA_PinTable[0]); i++) 
        {
            if (i2c_SDA_PinTable[i][0] == sda) 
            {
                for (uint8_t j = 0; j < sizeof(i2c_SCL_PinTable) / sizeof(i2c_SCL_PinTable[0]); j++) 
                {
                    if (i2c_SCL_PinTable[j][0] == scl && i2c_SCL_PinTable[j][2] == i2c_SDA_PinTable[i][2]) 
                    {
                        if(check_pad_mux(sda, i2c_SDA_PinTable[i][1]) == OPEN_HAL_DONE)
                        {
                            if(check_pad_mux(scl, i2c_SCL_PinTable[j][1]) == OPEN_HAL_DONE)
                            {
                                select = i2c_SCL_PinTable[j][2];
                                mux = i2c_SCL_PinTable[j][1];
                            }
                        }
                    }
                }
            }
        }
    }
    return select;
}

/**
  \fn          
  \brief
  \return
*/
static uint32_t usrId_to_index(uint32_t usrId)
{
    I2CCtx_t *ctx = &sI2cCtx;
    uint32_t index = (uint32_t)(usrId & OPEN_HAL_PORT_MUSK);
    if(index == (ctx->usr_id[index] & OPEN_HAL_PORT_MUSK))
    {
        return index;
    }
    return EC_I2C_INDEX_LIMIT;
}

/**
  \fn          
  \brief
  \return
*/
api_ret_t api_i2c_query(uint32_t usrId)
{
    I2CCtx_t *ctx = &sI2cCtx;
    uint32_t index = usrId_to_index(usrId);
    if(index >= EC_I2C_INDEX_START && index <= EC_I2C_INDEX_LIMIT)
    {
        if(ctx->usr_id[index] == OPEN_HAL_STAT_UNUSED)
        {
            return OPEN_HAL_FREE;
        }
        else if(ctx->usr_id[index] & OPEN_HAL_STAT_MUSK)
        {
            return OPEN_HAL_USED;
        }
        else
        {
            return OPEN_HAL_IDLE;
        }
    }
    return OPEN_HAL_NONE;
}
/* ---------------------------------------------------------------------------------------------- */

/**
  \fn          
  \brief
  \return
*/
static uint32_t i2c_set_free(uint32_t index)
{
    I2CCtx_t *ctx = &sI2cCtx;
    if(index >= EC_I2C_INDEX_START && index <= EC_I2C_INDEX_LIMIT)
    {
        ctx->usr_id[index] = OPEN_HAL_STAT_UNUSED;
        ctx->isr_func[index] = NULL;  // 清除中断绑定回调
        return 1;
    }
    else if(index >= EC_I2C_INDEX_LIMIT)
    {
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
    // ASSERT(index < EC_I2C_INDEX_LIMIT);
    I2CCtx_t *ctx = &sI2cCtx;
    uint32_t ret = ctx->usr_id[index];
    if(ctx->usr_id[index] == OPEN_HAL_STAT_UNUSED)
    {
        ctx->usr_id[index] = index;
        ctx->usr_id_seed[index] = 1;
        ctx->usr_id[index] |= (uint32_t)(ctx->usr_id_seed[index] << 16);
        ret = ctx->usr_id[index];
    }
    else if(ctx->usr_id[index] & OPEN_HAL_STAT_MUSK)
    {
        ctx->usr_id[index] &= ~(OPEN_HAL_STAT_MUSK);
        ret = ctx->usr_id[index];
    }
    EPAT_LOG(i2c_set_idle_2, P_INFO, "i2c%d,ctx 0x%x,ret 0x%x", index, ctx, ret);
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
    I2CCtx_t *ctx = &sI2cCtx;
    if(stat != OPEN_HAL_IDLE)
    {
        return 0;
    }
    uint32_t index = usrId_to_index(usrId);
    ctx->usr_id[index] |= OPEN_HAL_STAT_MUSK;
    return 1;
}


/**
  \fn          
  \brief
  \return
*/
int32_t api_i2c_parse(char* str, i2c_config_t *cfg)
{
    if(str == NULL || cfg == NULL){
        EPAT_LOG(api_i2c_parse_0, P_ERROR, "invalid para");
        return -1;
    } 
    uint8_t num = 0, ptr = 0;
    char *item_str[CSV_I2C_ITEM_MAX];
    char *head = strtok(str, ",");
    while(head != NULL && num < CSV_I2C_ITEM_MAX)
    {
        item_str[num] = head;
        head = strtok(NULL, ",");
        num ++; 
    }
    if(num < CSV_I2C_ITEM_MAX){
        EPAT_LOG(api_i2c_parse_1, P_ERROR, "input items %d < %d", num, CSV_I2C_ITEM_MAX);
        return -1;
    }
    int8_t index = atoi(item_str[ptr++]);
    if(index < EC_I2C_INDEX_START || index > EC_I2C_INDEX_LIMIT) 
    {
        EPAT_LOG(api_i2c_parse_2, P_ERROR, "i2c%d invalid", index);
        return -2;
    }
    else {
        cfg->port = index;
    }
    int32_t value = atoi(item_str[ptr++]);  //1: sda
    if(value >= EC_PAD_INDEX_START && value < EC_PAD_INDEX_LIMIT)
    {
        cfg->sda = value;
    }
    else {
        cfg->sda = 0;
    }
    value = atoi(item_str[ptr++]);  //2: scl
    if(value >= EC_PAD_INDEX_START && value < EC_PAD_INDEX_LIMIT)
    {
        cfg->scl = value;
    }
    else {
        cfg->scl = 0;
    }
    value = atoi(item_str[ptr++]);  //3: freq level
    if(value > 400) {
        cfg->clk = ARM_I2C_BUS_SPEED_FAST_PLUS;
    }
    else if(value > 100) {
        cfg->clk = ARM_I2C_BUS_SPEED_FAST;
    }
    else if(value > 0) {
        cfg->clk = ARM_I2C_BUS_SPEED_STANDARD;
    }
    value = atoi(item_str[ptr++]);  //4: mode
    if(value >= 0 && value < 4)
    {
        cfg->mode = value;
    }
    value = atoi(item_str[ptr++]);  //5: poll 模式
    if(value >= 0 && value < 2)
    {
        cfg->poll = value;
    }
    EPAT_LOG(api_i2c_parse, P_INFO, "i2c%d:sda %d,scl %d,clk %d,mode %d,poll %d;brief-%d:%s", \
        index, cfg->sda, cfg->scl, cfg->clk, cfg->mode, cfg->poll, ptr, head);
    return index;
}

/**
  \fn
  \brief    传入para参数为NULL则进行deinit, 由于底层都是标准CMSIS的i2cDevice接口，实际并没有再传出设备句柄
  \return
*/
api_ret_t api_i2c_setup(int8_t index, i2c_config_t* para)
{
    if(index < EC_I2C_INDEX_START || index > EC_I2C_INDEX_LIMIT)
    {
        EPAT_LOG(api_i2c_setup_0, P_ERROR, "i2c index %d over[%d,%d]", index, EC_I2C_INDEX_START, EC_I2C_INDEX_LIMIT);
        return OPEN_HAL_INVALID_PARA;
    }
    ARM_DRIVER_I2C *i2cDevice = NULL;
    if(index == 0){
        #if (defined RTE_I2C0)
        extern ARM_DRIVER_I2C Driver_I2C0 ;
        i2cDevice = &Driver_I2C0;
        #endif
    }
    #if (defined RTE_I2C1)
    else if(index == 1){
        extern ARM_DRIVER_I2C Driver_I2C1 ;
        i2cDevice = &Driver_I2C1;
    }
    #endif
    if(index >= EC_I2C_INDEX_LIMIT)  // SW I2C2
    {
        EPAT_LOG(api_i2c_setup_1, P_INFO, "sw i2c%d setup", index);
        extern api_ret_t api_sw_i2c_setup(int8_t index, i2c_config_t* para);
        return api_sw_i2c_setup(index, para);
    }
    else
    {
        if(para == NULL)
        {
            EPAT_LOG(api_i2c_setup_2, P_WARNING, "deinit i2c%d now(0x%x)", index, i2cDevice);
            if(i2cDevice != NULL)
            {
                i2cDevice->Uninitialize();
                // todo 确认关闭时钟
            }
        }
        else {
            int8_t check = ec_i2c_checkout(index, para->sda, para->scl);   // 如果配置的pad和输入的index不匹配，则返回错误
            if(check < 0)
            {
                EPAT_LOG(api_i2c_setup_3, P_ERROR, "i2c%d checkout %d fail", index, check);
                return OPEN_HAL_NONE;
            }
            if(i2cDevice != NULL)
            {
                i2cDevice->Uninitialize();
                i2cDevice->Initialize(NULL);
                i2cDevice->PowerControl(ARM_POWER_FULL);
                i2cDevice->Control(ARM_I2C_BUS_SPEED, para->clk);
                i2cDevice->Control(ARM_I2C_BUS_CLEAR, 0);
            }
            EPAT_LOG(api_i2c_setup_4, P_INFO, "i2c%d,checkout i2c%d,clk %d", index, check, para->clk);
        }
        return OPEN_HAL_DONE;
    }
    return OPEN_HAL_INVALID_PARA;
}

/**
  \fn
  \brief
  \return
*/
int8_t *api_i2c_startup(void *para, int8_t *pad)
{
    // int8_t (*padList)[4] = pad;
    for(int i = EC_I2C_INDEX_START; i < EC_I2C_INDEX_LIMIT; i++)
    {
        i2c_set_free(i);
    }
    return (int8_t *)i2cList;
}

/* ---------------------------------------------------------------------------------------------- */
/*为了兼容性，I2C0使用平台驱动hal实现，I2C1使用CMSIS驱动实现*/
#include "hal_i2c.h"
extern AP_PLAT_COMMON_BSS HalI2cPamram_t halI2cParam;
//extern int32_t halI2cInit(bool needLock);
extern int halI2cLock(void *arg);
extern int halI2cUnlock(void *arg);
static osSemaphoreId_t s_i2c1_lock = NULL;
/*为了I2C的兼容性，同时还要支持2路I2C，增加一组函数用于切换*/
static int i2c_temp_init(int index)
{
    I2CCtx_t *ctx = &sI2cCtx;
    if(index >= EC_I2C_INDEX_LIMIT)
    {
        return -1;
    }
    if(index == 0)
    {
        if(halI2cParam.i2cDrv == NULL)
        {
            halI2cInit(index,true);
            ctx->i2c_dev[0] = halI2cParam.i2cDrv;
        }
        //EPAT_LOG(i2c_temp_init_i2c0, P_INFO, "init i2c0: %x", ctx->i2c_dev[0]);
        return 0;
    }
#if(RTE_I2C1)
    if(s_i2c1_lock == NULL)
    {
        s_i2c1_lock = osSemaphoreNew(1, 1, NULL);
        if(s_i2c1_lock == NULL)
        {
            return -1;
        }
        osSemaphoreRelease(s_i2c1_lock);
    }

    if(!ctx->i2c_dev[1])
    {
        ctx->i2c_dev[1] = &Driver_I2C1;
        ctx->i2c_dev[1]->Initialize(NULL);
        ctx->i2c_dev[1]->PowerControl(ARM_POWER_FULL);
        ctx->i2c_dev[1]->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);
        ctx->i2c_dev[1]->Control(ARM_I2C_BUS_CLEAR, 0);
        EPAT_LOG(i2c_temp_init_i2c1, P_INFO, "init i2c1: %x", ctx->i2c_dev[1]);
    }
#endif
    return 0;
}

static int i2c_temp_lock(int index)
{
    if(index >= EC_I2C_INDEX_LIMIT)
    {
        return -1;
    }
    if(index == 0)
    {
        halI2cLock(halI2cParam.i2cSemId);
        return 0;
    }
    if(osSemaphoreAcquire(s_i2c1_lock, 1000) != 0)
    {
        return -1;
    }
    return 0;
}

/**
  \fn
  \brief
  \return
*/
static int i2c_temp_unlock(int index)
{
    if(index >= EC_I2C_INDEX_LIMIT)
    {
        return -1;
    }
    if(index == 0)
    {
        halI2cUnlock(halI2cParam.i2cSemId);
        return 0;
    }
    if(osSemaphoreRelease(s_i2c1_lock) != 0)
    {
        return -1;
    }
    return 0;
}


/* ---------------------------------------------------------------------------------------------- */

#ifndef I2C_USE_HAL_LOCK
static void api_i2c0_isr_handle(uint32_t event)
{
    // i2c0_risr_reg = I2C0->RISR;
    I2CCtx_t *ctx = &sI2cCtx;
    if(event & ARM_I2C_EVENT_TRANSFER_DONE)
    {
#ifdef FEATURE_OS_ENABLE
        if(event & ARM_I2C_EVENT_TRANSFER_INCOMPLETE)
        {
            osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C0_BUS_ERR));
            // osEventFlagsSet(halEventFlags_i2c[0], (1U <<
            // EVT_I2C_INCOMPLETE));
        }
        if(event & ARM_I2C_EVENT_ADDRESS_NACK)
        {
            osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C0_BUS_ERR));
            // osEventFlagsSet(halEventFlags_i2c[0], (1U <<
            // EVT_I2C_SLAVE_NACK));
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
        if(event == ARM_I2C_EVENT_TRANSFER_DONE)
        {
            osEventFlagsSet(successEvent, (1U << TRANS_EVT_I2C0_IRQ));
            if(ctx->isr_func[0] != NULL)
            {
                ctx->isr_func[0](event);
            }
        }  // ERROR with event 0x11 if IRQ mode
    }
    EPAT_LOG(i2c0_isr, P_INFO, "event 0x%X", event);
}

static void api_i2c1_isr_handle(uint32_t event)
{
    // i2c1_risr_reg = I2C1->RISR;
    I2CCtx_t *ctx = &sI2cCtx;
    if(event & ARM_I2C_EVENT_TRANSFER_DONE)
    {
#ifdef FEATURE_OS_ENABLE
        if(event & ARM_I2C_EVENT_TRANSFER_INCOMPLETE)
        {
            osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C1_BUS_ERR));
            // osEventFlagsSet(halEventFlags_i2c[1], (1U <<
            // EVT_I2C_INCOMPLETE));
        }
        if(event & ARM_I2C_EVENT_ADDRESS_NACK)
        {
            osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C1_BUS_ERR));
            // osEventFlagsSet(halEventFlags_i2c[1], (1U <<
            // EVT_I2C_SLAVE_NACK));
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
        if(event == ARM_I2C_EVENT_TRANSFER_DONE)
        {
            osEventFlagsSet(successEvent, (1U << TRANS_EVT_I2C1_IRQ));
            if(ctx->isr_func[1] != NULL)
            {
                ctx->isr_func[1](event);
            }
        }
    }
    EPAT_LOG(i2c1_isr, P_INFO, "event 0x%X", event);
}
#endif
/**
  \fn
  \brief
  \return
*/
uint32_t i2c_fault_wait(uint32_t usrId, size_t timeout)
{
    uint32_t index = usrId_to_index(usrId);
    ARM_I2C_STATUS status;
    I2CCtx_t *ctx = &sI2cCtx;
    index = index > EC_I2C_INDEX_LIMIT ? 0 : index;
    status = ctx->i2c_dev[index]->GetStatus();
    status = status;
    uint32_t flag = 0;
    uint32_t stat = 0;
    if(index == 0)
    {
#if(RTE_I2C1_IO_MODE == IRQ_MODE)
        flag = osEventFlagsWait(hwfaultEvent, (1U << FAULT_I2C0_BUS_ERR),
                                osFlagsWaitAny, timeout);
#endif
        i2c0_risr_reg = I2C0->RISR;
        stat = i2c0_risr_reg;
    }
    else if(index == 1)
    {
#if(RTE_I2C1_IO_MODE == IRQ_MODE)
        flag = osEventFlagsWait(hwfaultEvent, (1U << FAULT_I2C1_BUS_ERR),
                                osFlagsWaitAny, timeout);
#endif
        i2c1_risr_reg = I2C1->RISR;
        stat = i2c1_risr_reg;
    }
    if(flag < FAULT_EVT_TOTAL)
    {
        EPAT_LOG(i2c_fault_found, P_INFO,
                 "i2c%d:flag 0x%X,stat 0x%X,risr 0x%X,ThreadId 0x%x", index,
                 flag, status, stat, osThreadGetId());
    }
    return stat;
}

static uint32_t i2c_master_tx(uint32_t index, uint32_t addr, uint8_t *data,
                              uint32_t num, bool xfer, uint32_t timeout)
{
    uint32_t time_used = 0;
    I2CCtx_t *ctx = &sI2cCtx;
    uint32_t time_mark = osKernelGetTickCount();
    ARM_DRIVER_I2C *tpBus = ctx->i2c_dev[index];
    if(tpBus)
    {
        if(osMutexAcquire(ctx->i2c_mutex[index], timeout) == osOK)
        {
            ARM_I2C_STATUS status = tpBus->GetStatus();
            if(index == 0)
            {
#if(RTE_I2C0_IO_MODE == IRQ_MODE)
                osEventFlagsClear(successEvent, (1U << TRANS_EVT_I2C0_IRQ));
#else
                if(status.bus_error)
                {
                    osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C0_BUS_ERR));
                    EPAT_LOG(i2c0_master_tx_1, P_ERROR, "status 0x%x", status);
                }
#endif
            }
            else if(index == 1)
            {
#if(RTE_I2C1_IO_MODE == IRQ_MODE)
                osEventFlagsClear(successEvent, (1U << TRANS_EVT_I2C1_IRQ));
#else
                if(status.bus_error)
                {
                    osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C0_BUS_ERR));
                    EPAT_LOG(i2c1_master_tx_1, P_ERROR, "status 0x%x", status);
                }
#endif
            }
            tpBus->MasterTransmit(addr, data, num, xfer);
            if(index == 0)
            {
#if(RTE_I2C0_IO_MODE == IRQ_MODE)
                osEventFlagsWait(successEvent, (1U << TRANS_EVT_I2C0_IRQ),
                                 osFlagsNoClear, timeout);
                EPAT_LOG(i2c0_master_tx_2, P_INFO, "timeout %d", timeout);
#else
                status = tpBus->GetStatus();
                if(status.bus_error)
                {
                    osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C0_BUS_ERR));
                    EPAT_LOG(i2c0_master_tx_2, P_ERROR, "status 0x%x", status);
                }
#endif
            }
            else if(index == 1)
            {
#if(RTE_I2C1_IO_MODE == IRQ_MODE)
                osEventFlagsWait(successEvent, (1U << TRANS_EVT_I2C1_IRQ),
                                 osFlagsNoClear, timeout);
                EPAT_LOG(i2c1_master_tx_2, P_INFO, "timeout %d", timeout);
#else
                if(status.bus_error)
                {
                    osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C0_BUS_ERR));
                    EPAT_LOG(i2c1_master_tx_2, P_ERROR, "status 0x%x", status);
                }
#endif
            }
        }
        osMutexRelease(ctx->i2c_mutex[index]);
        time_used = (osKernelGetTickCount() - time_mark);
        if(time_used >= 1000 || time_used > timeout)
        {
            if(index == 1)
                osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C1_TIMEOUT));
            else
                osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C0_TIMEOUT));
            EPAT_LOG(i2c_master_tx_timeout, P_ERROR, "i2c%d used %d >= %d",
                     index, time_used, timeout);
        }
    }
    return time_used;
}
/**
  \fn
  \brief
  \return
*/
static uint32_t i2c_master_rx(uint32_t index, uint32_t addr, uint8_t *data,
                              uint32_t num, bool xfer, uint32_t timeout)
{
    uint32_t time_used = 0;
    I2CCtx_t *ctx = &sI2cCtx;
    uint32_t time_mark = osKernelGetTickCount();
    ARM_DRIVER_I2C *tpBus = ctx->i2c_dev[index];
    if(tpBus)
    {
        if(osMutexAcquire(ctx->i2c_mutex[index], timeout) == osOK)
        {
            ARM_I2C_STATUS status = tpBus->GetStatus();
            if(index == 0)
            {
#if(RTE_I2C0_IO_MODE == IRQ_MODE)
                osEventFlagsClear(successEvent, (1U << TRANS_EVT_I2C0_IRQ));
#else
                if(status.bus_error)
                {
                    osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C0_BUS_ERR));
                    EPAT_LOG(i2c0_master_rx_1, P_ERROR, "status 0x%x", status);
                }
#endif
            }
            else if(index == 1)
            {
#if(RTE_I2C1_IO_MODE == IRQ_MODE)
                osEventFlagsClear(successEvent, (1U << TRANS_EVT_I2C1_IRQ));
#else
                if(status.bus_error)
                {
                    osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C1_BUS_ERR));
                    EPAT_LOG(i2c1_master_rx_1, P_ERROR, "status 0x%x", status);
                }
#endif
            }
            tpBus->MasterReceive(addr, data, num, xfer);
            if(index == 0)
            {
#if(RTE_I2C0_IO_MODE == IRQ_MODE)
                osEventFlagsWait(successEvent, (1U << TRANS_EVT_I2C0_IRQ),
                                 osFlagsNoClear, timeout);
                EPAT_LOG(i2c0_master_rx_2, P_INFO, "timeout %d", timeout);
#else
                status = tpBus->GetStatus();
                if(status.bus_error)
                {
                    osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C0_BUS_ERR));
                    EPAT_LOG(i2c0_master_rx_2, P_ERROR, "status 0x%x", status);
                }
#endif
            }
            else if(index == 1)
            {
#if(RTE_I2C1_IO_MODE == IRQ_MODE)
                osEventFlagsWait(successEvent, (1U << TRANS_EVT_I2C1_IRQ),
                                 osFlagsNoClear, timeout);
                EPAT_LOG(i2c1_master_rx_2, P_INFO, "timeout %d", timeout);
#else
                status = tpBus->GetStatus();
                if(status.bus_error)
                {
                    osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C1_BUS_ERR));
                    EPAT_LOG(i2c1_master_rx_2, P_ERROR, "status 0x%x", status);
                }
#endif
            }
        }
        osMutexRelease(ctx->i2c_mutex[index]);
        time_used = (osKernelGetTickCount() - time_mark);
        if(time_used >= 1000 || time_used > timeout)
        {
            if(index == 1)
                osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C1_TIMEOUT));
            else
                osEventFlagsSet(hwfaultEvent, (1U << FAULT_I2C0_TIMEOUT));
            EPAT_LOG(i2c_master_rx_timeout, P_ERROR, "i2c%d used %d >= %d",
                     index, time_used, timeout);
        }
    }
    return time_used;
}

static uint8_t i2c_speed_set(ARM_DRIVER_I2C *drv, uint8_t level)
{
    if(drv != NULL && level)
    {
        drv->Control(ARM_I2C_BUS_SPEED, level);
    }
    return 0;
}

/* ---------------------------------------------------------------------------------------------- */
/**
  \fn          
  \brief   兼容接口 
  \return
*/
api_ret_t api_i2c_create(uint32_t index, I2cCfg_t *cfg, void *out)
{
    uint32_t usrId = 0;
    I2CCtx_t *ctx = &sI2cCtx;
    api_ret_t ret = OPEN_HAL_INVALID_PARA;
    if(index >= EC_I2C_INDEX_LIMIT)  // 模拟I2C
    {
        i2c_config_t* sw_cfg = (i2c_config_t*)cfg;
        extern uint32_t api_sw_i2c_create(int8_t index, i2c_config_t* cfg);
        usrId = api_sw_i2c_create(2, sw_cfg);
        ret = OPEN_HAL_DONE ;
    }
    else if(index >= EC_I2C_INDEX_START)
    {
        usrId = i2c_set_idle(index);
        if(usrId)
        {
            if(ctx->i2c_mutex[index] == NULL)
            {
                ctx->i2c_mutex[index] = osMutexNew(NULL);
                osMutexRelease(ctx->i2c_mutex[index]);
            }
            ret = OPEN_HAL_DONE ;
        }
        EPAT_LOG(api_i2c_create_2, P_INFO, "create i2c%d, mutex: 0x%x usrId: 0x%x",index, ctx->i2c_mutex[index], usrId);
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
api_ret_t api_i2c_delete(uint32_t usrId)
{
    api_ret_t ret = api_i2c_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_IDLE)
    {
        if(index < EC_I2C_INDEX_LIMIT)
        {
            if(i2c_set_free(index))
            {
            }
            if((index == 1) && (s_i2c1_lock != NULL))
            {
                i2c_temp_unlock(1);
                osSemaphoreDelete(s_i2c1_lock);
                s_i2c1_lock = NULL;
            }
        }
        else if(index == EC_I2C_INDEX_LIMIT)
        {
        }
        ret = OPEN_HAL_DONE;
    }
    EPAT_LOG(api_i2c_delete, P_INFO, "delete i2c%d", index);
    return ret;
}

/* ---------------------------------------------------------------------------------------------- */
ec_i2c_bus_t *i2cDev = NULL;    // 兼容当前接口（无输出指针），用于外部获取句柄
/**
  \fn          
  \brief    实际在此进行初始化操作
  \return
*/
api_ret_t api_i2c_open(uint32_t usrId, I2cCfg_t *cfg, size_t timeout)
{
    api_ret_t ret = api_i2c_query(usrId);
    I2CCtx_t *ctx = &sI2cCtx;
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_IDLE)
    {
        if(index < EC_I2C_INDEX_LIMIT)  // 历史遗留代码
        {
            #ifdef I2C_USE_HAL_LOCK
            i2c_temp_init(index);
            #else
            if(ctx->i2c_dev[index] != NULL)
            {
                ctx->i2c_dev[index]->Uninitialize();
                if(index == 0)
                    ctx->i2c_dev[index]->Initialize(api_i2c0_isr_handle);
                else if(index == 1)
                    ctx->i2c_dev[index]->Initialize(api_i2c1_isr_handle);
                ctx->i2c_dev[index]->PowerControl(ARM_POWER_FULL);
                ctx->i2c_dev[index]->Control(ARM_I2C_BUS_CLEAR, 0);
            }
            #endif
            if(cfg)
            {
                ctx->speed[index] = cfg->speed;
                ctx->i2c_dev[index]->Control(ARM_I2C_BUS_SPEED, ctx->speed[index] & 0x07);
                ctx->slave_addr[index] = cfg->slave_addr;
            }
        }
        else if(index == EC_I2C_INDEX_LIMIT) // 新增SW I2C2代码
        {
            extern api_ret_t api_sw_i2c_open(int8_t index, void **i2cDevice);
            api_sw_i2c_open(index, (void **)&i2cDev);
        }
        ctx->i2c_idle[index] = false;
        i2c_set_used(usrId);
        ret = OPEN_HAL_DONE;
        EPAT_LOG(api_i2c_open_2, P_INFO, "open i2c%d, slave:0x%x dev:0x%x",index, ctx->slave_addr[index], ctx->i2c_dev[index]);
    }
    return ret;
}

/**
  \fn          
  \brief        
  \return
*/
api_ret_t api_i2c_close(uint32_t usrId)
{
    I2CCtx_t *ctx = &sI2cCtx;
    api_ret_t ret = api_i2c_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_USED)
    {
        if(index < EC_I2C_INDEX_LIMIT){

        }
        else if(index == EC_I2C_INDEX_LIMIT) // 新增SW I2C2代码
        {

        }
        i2c_set_idle(index);
        ctx->i2c_idle[index] = true;
        ret = OPEN_HAL_DONE;
    }

    EPAT_LOG(api_i2c_close, P_INFO, "i2c%d,usrId 0x%X,0x%X,ret%d", index, usrId,
             ctx->usr_id[index], ret);
    return ret;
}

/* ---------------------------------------------------------------------------------------------- */

/**
  \fn          
  \brief        
  \return
*/
api_ret_t api_i2c_ioctl(uint32_t usrId, I2cIoctl_e type, void *para)
{
    api_ret_t ret = api_i2c_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    I2CCtx_t *ctx = &sI2cCtx;
    if(ret == OPEN_HAL_USED)
    {
        uint8_t speed = ctx->speed[index] & 0x07;
        switch(type)
        {
            case OPEN_I2C_IOCTL_ISR_CB:
                ctx->isr_func[index] = (IsrFunc)para;
                break;
            case OPEN_I2C_IOCTL_SPEED:
                speed = *(uint8_t *)para;
                speed &= 0xF8;
                i2c_speed_set(ctx->i2c_dev[index], speed);
                ctx->speed[index] += speed;
                break;
            case OPEN_I2C_IOCTL_SLAVE_ADDR:
                ctx->slave_addr[index] = *(uint8_t *)para;
            default:
                break;
        }
        ret = OPEN_HAL_DONE;
    }

    //EPAT_LOG(api_i2c_ioctl, P_INFO,
    //         "i2c%d, usrid:0x%X, type:%d, saddr: 0x%x, speed: %d, ret: %d",
    //         index, usrId, type, ctx->slave_addr[index], ctx->speed[index],
    //         ret);
    return ret;
}

/**
  \fn          
  \brief        
  \return
*/
api_ret_t api_i2c_pmctl(uint32_t usrId, open_hal_pm_t *cfg, size_t count)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_i2c_query(usrId);
    // uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_USED)
    {
        if(cfg != NULL)
        {
            if(count == 0)
            {
            }
            else if(cfg->runtime == RUNTIME_SUSPEND)
            {
                if(cfg->mode == PM_LOWPOW)
                {
                }
            }
            ret = OPEN_HAL_DONE;
        }
    }

    EPAT_LOG(api_i2c_pmctl, P_INFO, "i2c%d, 0x%X, ret%d", index, usrId, ret);
    return OPEN_HAL_DONE;
}

/* ---------------------------------------------------------------------------------------------- */

/**
  \fn          
  \brief        
  \return
*/
api_ret_t api_i2c_write(uint32_t usrId, void *buf, uint32_t count,
                        int xfer_pending)

{
    uint32_t time_used[3] = {0};
    time_used[0] = time_used[0];
    I2CCtx_t *ctx = &sI2cCtx;
    if((!buf) || (count == 0))
    {
        return OPEN_HAL_INVALID_PARA;
    }
    time_used[2] = osKernelGetTickCount();
    api_ret_t ret = api_i2c_query(usrId);
    uint32_t index = usrId_to_index(usrId);
#ifdef I2C_USE_HAL_LOCK
    i2c_temp_init(index);
    if(i2c_temp_lock(index) != 0)
    {
        ret = OPEN_HAL_LOCK;
        goto end;
    }
#endif
    if(ret == OPEN_HAL_USED)
    {
        time_used[0] = i2c_master_tx(index, ctx->slave_addr[index], buf, count,
                                     xfer_pending, 100);
        ret = OPEN_HAL_DONE;
    }
#ifdef I2C_USE_HAL_LOCK
    i2c_temp_unlock(index);
#endif
end:
    //EPAT_LOG(api_i2c_write, P_INFO,
    //         "usrid: 0x%X, i2c%d, addr: 0x%X, ret: %d,used: %d", usrId, index,
    //         ctx->slave_addr[index], ret,
    //         (osKernelGetTickCount() - time_used[2]));
    return ret;
}

/**
  \fn          
  \brief        
  \return
*/
api_ret_t api_i2c_read(uint32_t usrId, void *buf, uint32_t count,
                       int xfer_pending)
{
    uint32_t time_used[3] = {0};
    time_used[0] = time_used[0];
    I2CCtx_t *ctx = &sI2cCtx;
    time_used[2] = osKernelGetTickCount();
    api_ret_t ret = api_i2c_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if((!buf) || (count == 0))
    {
        return OPEN_HAL_INVALID_PARA;
    }
#ifdef I2C_USE_HAL_LOCK
    i2c_temp_init(index);
    int32_t lock = i2c_temp_lock(index);
    if(lock != 0)
    {
        lock = i2c_temp_lock(index);
        if(lock != 0)
        {
            ret = OPEN_HAL_LOCK;
            return ret;
        }
    }
#endif
    if(ret == OPEN_HAL_USED)
    {
        time_used[1] = i2c_master_rx(index, ctx->slave_addr[index], buf, count,
                                     xfer_pending, 100);
        ret = OPEN_HAL_DONE;
    }
#ifdef I2C_USE_HAL_LOCK
    i2c_temp_unlock(index);
#endif
    EPAT_LOG(api_i2c_read, P_INFO,
             "usrid: 0x%X, i2c%d, addr: 0x%X, ret: %d,used: %d", usrId, index,
             ctx->slave_addr[index], ret,
             (osKernelGetTickCount() - time_used[2]));
    return ret;
}
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
        api_i2c_create(index, NULL, &test_i2c_id);
        EC_API_CHECK(api_i2c_open(test_i2c_id, NULL, 1000));
        EC_API_CHECK(
            api_i2c_ioctl(test_i2c_id, OPEN_I2C_IOCTL_ISR_CB, test_i2c_cb));
        uint8_t speed = 2;
        EC_API_CHECK(api_i2c_ioctl(test_i2c_id, OPEN_I2C_IOCTL_SPEED, &speed));
        uint8_t addr = 0x38;
        EC_API_CHECK(
            api_i2c_ioctl(test_i2c_id, OPEN_I2C_IOCTL_SLAVE_ADDR, &addr));
        uint8_t temp[4] = {0};  // test with minidkb tp ft6336
        temp[0] = 0x02;
        EC_API_CHECK(api_i2c_write(test_i2c_id, temp, 1, true));
        EC_API_CHECK(api_i2c_read(test_i2c_id, temp, 1, false));
        EC_API_CHECK(api_i2c_close(test_i2c_id));
        EC_API_CHECK(api_i2c_delete(test_i2c_id));
    }
    return 0;
}