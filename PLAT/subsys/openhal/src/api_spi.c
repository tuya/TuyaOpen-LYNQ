/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_spi.c
 * Description:  openhal spi entry source file
 * History:      Rev1.0   2024-02-23
 *
 ****************************************************************************/
#if 1
#include <stdio.h>
#include <stdlib.h>
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

#ifdef EPAT_HAL_DEBUG
#define EPAT_LOG(subId, debugLevel, format, ...)  \
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, subId, debugLevel, format, ##__VA_ARGS__)
#else
#define EPAT_LOG(subId, debugLevel, format, ...)
#endif
#define TRANSFER_DATA_WIDTH     8
#define SPI_POLLING_DMA_SPLIT   14 // DMA: Greater than or equal to 14
#if (RTE_SPI0_IO_MODE == DMA_MODE)
#define SPI_DMA_ENABLE
#ifdef SPI_DMA_ENABLE
static osEventFlagsId_t gSpiFlags = NULL;
#endif
#endif

#define SPI_BUFFER_SIZE     8191

static uint8_t  gSpiBuffer[SPI_BUFFER_SIZE] = {0};
static uint32_t gSpiClk[EC_SPI_INDEX_LIMIT] = {0};
static uint32_t gSpiMode[EC_SPI_INDEX_LIMIT] = {0};
static uint32_t sSpiUsrIdList[EC_SPI_INDEX_LIMIT] = {0};
static uint16_t sSpiUsrIdSeed[EC_SPI_INDEX_LIMIT] = {0};
static IsrFunc sSpiIsrFunc[EC_UART_INDEX_LIMIT] = {NULL};

// static uint32_t sSpiStartupList[EC_SPI_INDEX_LIMIT] = {
//     0x0, 0x0, 0x10060, 0x10480
// };
// static uint32_t sSpiRuntimeList[EC_SPI_INDEX_LIMIT] = {0xFFFFFFFF};

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

#define SSP_FRST_ID     {6, 7}
#define SSP_FCLK_ID     {11, 12}
/* ---------------------------------------------------------------------------------------------- */
const int8_t spi_mosi_PinTable[][3] = {
    {24, PAD_MUX_ALT1, 0},  // 24 GPIO9  SPI0_MOSI
    {28, PAD_MUX_ALT1, 1},  // 28 GPIO13 SPI1_MOSI
};
const int8_t spi_miso_PinTable[][3] = {
    {25, PAD_MUX_ALT1, 0},  // 25 GPIO10 SPI0_MISO
    {29, PAD_MUX_ALT1, 1},  // 29 GPIO14 SPI1_MISO
};
const int8_t spi_sclk_PinTable[][3] = {
    {26, PAD_MUX_ALT1, 0},  // 26 GPIO11 SPI0_SCLK
    {30, PAD_MUX_ALT1, 1},  // 30 GPIO15 SPI1_SCLK
};

/* ----------------------------------------- 运行状态表 ------------------------------------------ */
AP_PLAT_COMMON_DATA static int8_t spiList[EC_SPI_INDEX_LIMIT][4] = {
    {-1, 51, DMA_MODE, -1},             /* SPI0: 51MHz/DMA_MODE */
    {-1, 26, POLLING_MODE, -1}          /* SPI1: 26MHz/POLLING */
};
// 如果第一项为-1表示该I2C没有正确初始化，使用前需要先创建

static osMutexId_t s_mutex[EC_SPI_INDEX_LIMIT] = {NULL};
/**
  \fn          api_ret_t api_spi_checkout(int8_t mosi, int8_t miso, int8_t sclk)
  \brief       检查SPI接口的引脚配置是否正确
  \param[in]   mosi   SPI主输出从输入引脚编号
  \param[in]   miso   SPI主输入从输出引脚编号
  \param[in]   sclk   SPI时钟引脚编号
  \return      检查结果，OPEN_HAL_DONE表示成功，其他值表示失败
  \details     该函数用于检查指定的SPI引脚配置是否正确。它会验证MOSI、MISO和SCLK引脚是否
               在预定义的引脚表中，并且对应的PAD是否已配置为SPI功能。
               只有当所有引脚都属于同一个SPI控制器时，检查才会成功。
*/
api_ret_t api_spi_checkout(int8_t mosi, int8_t miso, int8_t sclk)
{
    api_ret_t ret = OPEN_HAL_NONE;
    int8_t select = -1;
    int8_t mux = -1;
    if(mosi >= 0){
        for (uint8_t i = 0; i < sizeof(spi_mosi_PinTable)/sizeof(spi_mosi_PinTable[0]); i++) {
            if (spi_mosi_PinTable[i][0] == mosi) {
                mux = spi_mosi_PinTable[i][1] ;
                if (check_pad_mux(mosi, mux) == OPEN_HAL_DONE) {
                    select = spi_mosi_PinTable[i][2];
                    ret = OPEN_HAL_FREE;
                    // EPAT_LOGD("checkout spi%d-mosi%d-mux%d,%d", select, mosi, mux, ret);
                }
                else {
                    ret = OPEN_HAL_NONE;
                }
            }
        }
    }
    if(miso >= 0 && select >= 0)
    {
        for (uint8_t i = 0; i < sizeof(spi_miso_PinTable)/sizeof(spi_miso_PinTable[0]); i++) 
        {
            if (spi_miso_PinTable[i][0] == miso) 
            {
                if (check_pad_mux(miso, spi_miso_PinTable[i][1]) == OPEN_HAL_DONE && select == spi_miso_PinTable[i][2]) 
                {
                    ret = OPEN_HAL_FREE;
                    // EPAT_LOGD("checkout spi%d-miso%d-mux%d,%d", select, miso, mux, ret);
                }
            }
        }
    }
    if(sclk >= 0 && select >= 0 && ret == OPEN_HAL_FREE)
    {
        ret = OPEN_HAL_NONE;
        for (uint8_t i = 0; i < sizeof(spi_sclk_PinTable)/sizeof(spi_sclk_PinTable[0]); i++) 
        {
            if (spi_sclk_PinTable[i][0] == sclk) 
            {
                if (check_pad_mux(sclk, spi_sclk_PinTable[i][1]) == OPEN_HAL_DONE && select == spi_sclk_PinTable[i][2]) 
                {
                    ret = OPEN_HAL_DONE;
                    // EPAT_LOGD("checkout spi%d-sclk%d-mux%d,%d", select, sclk, mux, ret);
                }
            }
        }
    }
    else if(mosi < 0 && miso < 0)
    {
        // 其他的判断逻辑
    }
    return ret;
}

/* ---------------------------------------------------------------------------------------------- */
/**
  \fn          int32_t api_spi_parse(char* str, spi_config_t *cfg)
  \brief       解析SPI配置字符串
  \param[in]   str   配置字符串
  \param[out]  cfg   解析后的配置参数结构体指针
  \return      SPI索引编号
  \details     该函数用于解析CSV格式的SPI配置字符串，并填充到配置结构体中。
*/
int32_t api_spi_parse(char* str, spi_config_t *cfg)
{
    if(str == NULL || cfg == NULL){
        EPAT_LOG(api_spi_parse_0, P_ERROR, "invalid para");
        return -1;
    } 
    uint8_t ptr = 0;
    char *item_str[CSV_CFG_SPI_ITEMS];
    char *head = strtok(str, ",");
    while(head != NULL && ptr < CSV_CFG_SPI_ITEMS)
    {
        item_str[ptr] = head;
        head = strtok(NULL, ",");
        ptr ++;
    }
    if(ptr < CSV_CFG_SPI_ITEMS)
    {
        EPAT_LOG(api_spi_parse_1, P_ERROR, "input items %d < %d", ptr, CSV_CFG_SPI_ITEMS);
        return -1;
    }
    ptr = 0;
    int8_t index = atoi(item_str[ptr++]);
    if(index < EC_SPI_INDEX_START || index > EC_SPI_INDEX_LIMIT) 
    {
        EPAT_LOG(api_spi_parse_2, P_ERROR, "spi%d invalid", index);
        return -2;
    }
    // 如下为有效参数提取
    int32_t value = atoi(item_str[ptr++]);  //1: cs pad
    if(value >= EC_PAD_INDEX_START && value < EC_PAD_INDEX_LIMIT)
    {
        cfg->cs = value;
    }
    else {
        cfg->cs = 0;
        EPAT_LOG(csv_spi_item_2, P_ERROR, "cs pad%d invalid %d:%s", value, ptr, item_str[ptr-1]);
    }
    value = atoi(item_str[ptr++]);  //2: mosi pad
    if(value >= EC_PAD_INDEX_START && value <= 32)
    {
        cfg->mosi = value;
    }
    else {
        cfg->mosi = 0;
        EPAT_LOG(csv_spi_item_3, P_ERROR, "mosi pad%d invalid %d:%s", value, ptr, item_str[ptr-1]);
    }
    value = atoi(item_str[ptr++]);  //3: miso
    if(value >= EC_PAD_INDEX_START && value <= 32)
    {
        cfg->miso = value;
    }
    else {
        cfg->miso = 0;
        EPAT_LOG(csv_spi_item_4, P_ERROR, "miso pad%d invalid %d:%s", value, ptr, item_str[ptr-1]);
    }
    value = atoi(item_str[ptr++]);  //4: sclk
    if(value >= EC_PAD_INDEX_START && value <= 32)
    {
        cfg->sclk = value;
    }
    else {
        cfg->sclk = 0;
        EPAT_LOG(csv_spi_item_5, P_ERROR, "sclk pad%d invalid %d:%s", value, ptr, item_str[ptr-1]);
    }
    value = atoi(item_str[ptr++]);  //5: freq
    if(value >= 0 && value < 32)
    {
        cfg->freq = value;      // 时钟梯度化，压缩数据
    }
    else {
        cfg->freq = 0;
        EPAT_LOG(csv_spi_item_6, P_ERROR, "freq level %d invalid %d:%s", value, ptr, item_str[ptr-1]);
    }
    value = atoi(item_str[ptr++]);  //6: poll
    if(value >= 0 && value < 4)
    {
        cfg->poll = value;
    }
    else {
        EPAT_LOG(csv_spi_item_7, P_ERROR, "poll %d invalid %d:%s", value, ptr, item_str[ptr-1]);
    }
    value = atoi(item_str[ptr++]);  //7: mode
    if(value >= 0 && value < 16)
    {
        cfg->mode = value;
    }
    else {
        EPAT_LOG(csv_spi_item_8, P_ERROR, "mode %d invalid %d:%s", value, ptr, item_str[ptr-1]);
    }
    EPAT_LOG(api_spi_parse, P_INFO, "spi%d:cs %d,mosi %d,miso %d,sclk %d,freq %d,poll %d,mode %d", \
        index, cfg->cs, cfg->mosi, cfg->miso, cfg->sclk, cfg->freq, cfg->poll, cfg->mode);
    return index;
}

/* ---------------------------------------------------------------------------------------------- */
/**
  \fn          static uint32_t usrId_to_index(uint32_t usrId)
  \brief       根据用户ID获取SPI索引
  \param[in]   usrId  用户ID
  \return      SPI索引编号
  \details     该函数用于从用户ID中提取SPI索引编号。
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
  \fn          api_ret_t api_spi_query(uint32_t usrId)
  \brief       查询SPI设备状态
  \param[in]   usrId  SPI设备ID
  \return      SPI设备当前状态
  \details     该函数用于查询指定SPI设备的当前状态（空闲、使用中等）。
*/
api_ret_t api_spi_query(uint32_t usrId)
{
    // ASSERT(usrId > 0);
    uint32_t index = usrId_to_index(usrId);
    // ASSERT(index < EC_SPI_INDEX_LIMIT);
    if(index >= EC_SPI_INDEX_START && index < EC_SPI_INDEX_LIMIT)
    {
        if(s_mutex[index] == NULL)
        {
            // return OPEN_HAL_FREE;
        }
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

/**
  \fn          static uint32_t spi_set_free(uint32_t index)
  \brief       将SPI设备设置为空闲状态
  \param[in]   index  SPI索引编号
  \return      设置结果，1表示成功，0表示失败
  \details     该函数用于将指定的SPI设备设置为空闲状态，释放相关资源。
*/
static uint32_t spi_set_free(uint32_t index)
{
    if(index >= EC_SPI_INDEX_START && index < EC_SPI_INDEX_LIMIT)
    {
        sSpiUsrIdList[index] = OPEN_HAL_STAT_UNUSED;
        sSpiIsrFunc[index] = NULL;
        if(gSpiDrv[index] != NULL)
        {
        }
        return 1;  
    }
    return 0;
}

/**
  \fn          static uint32_t spi_set_idle(uint32_t index)
  \brief       将SPI设备设置为未使用状态
  \param[in]   index  SPI索引编号
  \return      用户ID
  \details     该函数用于将指定的SPI设备设置为未使用状态，准备供用户使用。
*/
static uint32_t spi_set_idle(uint32_t index)
{
    uint32_t usrId = 0;
    if(index >= EC_SPI_INDEX_START && index < EC_SPI_INDEX_LIMIT)
    {
        if(s_mutex[index] != NULL)
        {
            osMutexRelease(s_mutex[index]);
        }
        if(sSpiUsrIdList[index] == OPEN_HAL_STAT_UNUSED){
            sSpiUsrIdList[index] = index;
            sSpiUsrIdSeed[index] ++;
            sSpiUsrIdList[index] |= (uint32_t)(sSpiUsrIdSeed[index] << 16);
            usrId = sSpiUsrIdList[index];
        }
        else if(sSpiUsrIdList[index] & OPEN_HAL_STAT_MUSK){
            sSpiUsrIdList[index] &= ~(OPEN_HAL_STAT_MUSK);
            usrId = sSpiUsrIdList[index];
        }
    }
    return usrId;
}

/**
  \fn          static uint32_t spi_set_used(uint32_t usrId)
  \brief       将SPI设备设置为使用中状态
  \param[in]   usrId  SPI设备ID
  \return      设置结果，OPEN_HAL_DONE表示成功
  \details     该函数用于将指定的SPI设备设置为使用中状态。
*/
static uint32_t spi_set_used(uint32_t usrId)
{
    api_ret_t ret = api_spi_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(s_mutex[index] == NULL)
    {
        ret = OPEN_HAL_FREE;
    }
    else if (osMutexAcquire(s_mutex[index], 100) != osOK)
    {
        ret = OPEN_HAL_USED;
    }
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

/* ---------------------------------------------------------------------------------------------- */
/**
  \fn          static void SPI_Callback(uint32_t event)
  \brief       SPI DMA传输回调函数
  \param[in]   event  事件类型
  \return      无
  \details     该函数用于处理SPI DMA传输完成事件。
*/
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

/**
  \fn          api_ret_t api_spi_setup(int8_t index, spi_config_t* para)
  \brief       单项初始化，使用HAL统一的参数格式
  \param[in]   index  SPI索引编号
  \param[in]   para   SPI配置参数指针
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于初始化指定的SPI设备。
*/
api_ret_t api_spi_setup(int8_t index, spi_config_t* para)
{
    int8_t (*pList)[4] = spiList;
    api_ret_t ret = OPEN_HAL_INVALID_PARA;
    if(index < EC_SPI_INDEX_START || index >= EC_SPI_INDEX_LIMIT) 
    {
        EPAT_LOG(api_spi_setup_1, P_ERROR, "error index %d [%d,%d]", index, EC_SPI_INDEX_START, EC_SPI_INDEX_LIMIT);
        return OPEN_HAL_INVALID_PARA;
    }
    else if(para == NULL) {
        // TODO: 如果配置参数是NULL则去初始化SPI设备
        pList[index][3] = 0 ;
    }
    else 
    {
        // 启用SPI时钟

        // 配置SPI参数
        if(gSpiDrv[index] != NULL)  // 兼容旧的代码
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
            ret = OPEN_HAL_DONE;
        }
        // 保存运行时参数
        pList[index][0] = index ;
        // pList[index][3] = 0 ;
        EPAT_LOG(api_spi_setup_2, P_INFO, "spi%d:cs %d,mosi %d,miso %d,sclk %d,freq %d,mode %d,poll %d", \
            index, para->cs, para->mosi, para->miso, para->sclk, para->freq, para->mode, para->poll);
    }
    return ret;
}

/**
  \fn          int8_t *api_spi_startup(void* para, int8_t *pad)
  \brief       SPI上电初始化
  \param[in]   para  指向SPI配置参数数组的指针
  \param[in]   pad   指向PAD配置参数数组的指针
  \return      返回指向SPI状态列表的指针
  \details     该函数用于在系统上电时初始化所有SPI设备。
*/
int8_t *api_spi_startup(void* para, int8_t *pad)
{
    // int8_t (*padList)[4] = pad;
    for(int8_t i=EC_SPI_INDEX_START; i<EC_SPI_INDEX_LIMIT; i++)
    {
        spi_set_free(i);
        s_mutex[i] = NULL;
    }
    return (int8_t *)spiList;
}

/* ---------------------------------------------------------------------------------------------- */

/**
  \fn          static uint8_t spiStartSend(uint32_t index, void *data, uint32_t num)
  \brief       启动SPI发送
  \param[in]   index  SPI索引编号
  \param[in]   data   要发送的数据指针
  \param[in]   num    要发送的数据数量
  \return      执行结果，1表示成功，0表示失败
  \details     该函数用于启动SPI数据发送。
*/
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
/**
  \fn          static uint8_t spiStopSend(uint32_t index)
  \brief       停止SPI发送
  \param[in]   index  SPI索引编号
  \return      执行结果，1表示成功，0表示失败
  \details     该函数用于停止SPI数据发送。
*/
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


/* ---------------------------------------------------------------------------------------------- */
/**
  \fn          api_ret_t api_spi_create(uint32_t index,void *cfg, void *out)
  \brief       创建SPI设备实例
  \param[in]   index  SPI索引编号
  \param[in]   cfg    SPI配置参数指针
  \param[out]  out    输出参数，返回创建的SPI设备ID
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于创建一个新的SPI设备实例，分配相关资源并进行初始化。
*/
api_ret_t api_spi_create(uint32_t index,void *cfg, void *out)
{
    api_ret_t ret = OPEN_HAL_INVALID_PARA;
    if(index < EC_SPI_INDEX_START || index >= EC_SPI_INDEX_LIMIT){
        return OPEN_HAL_INVALID_PARA;
    }
    uint32_t usrId = 0; 
    if(s_mutex[index] != NULL){
        // 已经初始化，不再继续执行
        ret = OPEN_HAL_USED;
    }
    else
    {
        s_mutex[index] = osMutexNew(NULL);
        if(cfg != NULL)
        {
            spi_config_t* para = (spi_config_t *)cfg;
            ret = api_spi_checkout(para->mosi, para->miso, para->sclk); 
        }
        if(ret == OPEN_HAL_DONE)
        {
            usrId = spi_set_idle(index); 
        }
    }
    if(out != NULL)
    {
        *(uint32_t *)out = usrId;
    }
    return ret;
}

/**
  \fn          api_ret_t api_spi_delete(uint32_t usrId)
  \brief       删除SPI设备实例
  \param[in]   usrId  SPI设备ID
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于删除指定的SPI设备实例，释放相关资源。
               只有在SPI处于空闲状态时才能被删除。
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
    if(s_mutex[index] != NULL)
    {
        osMutexDelete(s_mutex[index]);
        s_mutex[index] = NULL;
    }
    SYSLOG_INFO("spi%d[0x%08X]:%d\r\n",index,usrId,ret); 
    return ret;
}

/* ---------------------------------------------------------------------------------------------- */
/**
  \fn          api_ret_t api_spi_open(uint32_t usrId,void *cfg,size_t timeout)
  \brief       打开SPI设备
  \param[in]   usrId    SPI设备ID
  \param[in]   cfg      SPI配置参数指针（可为NULL）
  \param[in]   timeout  超时时间（暂未使用）
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于打开SPI设备并根据配置参数进行设置。
               只有在SPI处于空闲状态时才能被打开。
*/
api_ret_t api_spi_open(uint32_t usrId,void *cfg,size_t timeout)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_spi_query(usrId);
    // uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_IDLE)
    {

        if(cfg != NULL)
        {

        } 
        spi_set_used(usrId);
        ret = OPEN_HAL_DONE;
    }
    // SYSLOG_INFO("spi%d[0x%08X]:0x%X\r\n",index,usrId,ret); 
    return ret;
}

/**
  \fn          api_ret_t api_spi_close(uint32_t usrId)
  \brief       关闭SPI设备
  \param[in]   usrId  SPI设备ID
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于关闭指定的SPI设备，将其状态设置为空闲。
               只有在SPI处于使用中状态时才能被关闭。
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

/* ---------------------------------------------------------------------------------------------- */

/**
  \fn          api_ret_t api_spi_ioctl(uint32_t usrId,api_spi_ioctl_t type, void *para)
  \brief       SPI设备控制接口
  \param[in]   usrId  SPI设备ID
  \param[in]   type   控制类型，参考api_spi_ioctl_t枚举
  \param[in]   para   控制参数指针
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于对SPI设备进行各种控制操作。
               只有在SPI处于使用中状态时才能进行控制操作。
*/
api_ret_t api_spi_ioctl(uint32_t usrId,api_spi_ioctl_t type, void *para)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_spi_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    index = usrId;
    uint32_t *data = (uint32_t *)para;
    // if(ret == OPEN_HAL_USED)
    {
        switch (type) {
            case OPEN_SPI_IOCTL_CLK:
                gSpiClk[index] = data[0];
                break;
            case OPEN_SPI_IOCTL_MODE:
                gSpiMode[index] = data[0];
                break;
            case OPEN_SPI_IOCTL_CONFIG:
                api_spi_setup(index, para);
                break;
            case OPEN_SPI_IOCTL_START_SEND:
                spiStartSend(index, (void *)data[0], data[1]);
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
  \fn          api_ret_t api_spi_pmctl(uint32_t usrId, open_hal_pm_t *cfg, size_t count)
  \brief       对SPI设备功耗和模式进行配置
  \param[in]   usrId  SPI设备ID
  \param[in]   cfg    功耗配置参数指针
  \param[in]   count  参数数量（暂未使用）
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于控制SPI设备的功耗模式。
               只有在SPI处于使用中状态时才能进行功耗控制。
*/
api_ret_t api_spi_pmctl(uint32_t usrId, open_hal_pm_t *cfg, size_t count)
{
    if(cfg == NULL || usrId == 0) return OPEN_HAL_INVALID_PARA;
    api_ret_t ret = api_spi_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    uint8_t frst_id[] = SSP_FRST_ID;
    uint8_t fclk_id[] = SSP_FCLK_ID;
    ClockId_e fclk = CONSTRUCT_CLOCK_ID(APB_GPR_APB_MP_FCLK_EN_REG_INDEX, frst_id[index], fclk_id[index]);
    if(ret == OPEN_HAL_USED)
    {
        if(cfg->runtime == RUNTIME_SUSPEND)
        {
            if(cfg->mode == PM_LOWPOW)
            {
                if(GPR_getClockFreq(fclk)>=26000000U)   // 如果时钟频率大于等于26M，则切换时钟降频处理
                {
                    
                }
            }
        }
        EPAT_LOG(api_spi_pmctl_3, P_INFO, "pwm%d fclk=%d", index, GPR_getClockFreq(fclk));
        ret = OPEN_HAL_DONE;
    }
    return ret;
}
/* ---------------------------------------------------------------------------------------------- */
/**
  \fn          static void spiReadBuffer(ARM_DRIVER_SPI *spiDrv, uint8_t *bufferIn, uint32_t length)
  \brief       从SPI设备读取数据到缓冲区
  \param[in]   spiDrv     SPI驱动指针
  \param[out]  bufferIn   读取数据的缓冲区指针
  \param[in]   length     要读取的数据长度
  \return      无
  \details     该函数用于从SPI设备读取指定长度的数据到缓冲区中。
*/
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

/**
  \fn          static void spiWriteBuffer(ARM_DRIVER_SPI *spiDrv, uint8_t *bufferOut, uint32_t length)
  \brief       向SPI设备写入缓冲区数据
  \param[in]   spiDrv      SPI驱动指针
  \param[in]   bufferOut   要写入的数据缓冲区指针
  \param[in]   length      要写入的数据长度
  \return      无
  \details     该函数用于向SPI设备写入指定长度的缓冲区数据。
*/
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

/**
  \fn          static uint8_t spiReadWriteByte(ARM_DRIVER_SPI *spiDrv, uint8_t dataOut)
  \brief       通过SPI读写单个字节
  \param[in]   spiDrv    SPI驱动指针
  \param[in]   dataOut   要写入的字节数据
  \return      读取到的字节数据
  \details     该函数用于通过SPI接口读写单个字节数据。
*/
static uint8_t spiReadWriteByte(ARM_DRIVER_SPI *spiDrv, uint8_t dataOut)
{
    uint8_t dataIn = 0;

    spiDrv->Transfer(&dataOut, &dataIn, 1);

    return dataIn;
}
/* ---------------------------------------------------------------------------------------------- */
/**
  \fn          api_ret_t api_spi_write(uint32_t usrId, void* buf, size_t count)
  \brief       向SPI设备写入数据
  \param[in]   usrId   SPI设备ID
  \param[in]   buf     要写入的数据缓冲区指针
  \param[in]   count   要写入的数据大小（字节数）
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于向SPI设备写入数据。
               只有在SPI处于使用中状态时才能进行写操作。
*/
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

/**
  \fn          api_ret_t api_spi_read(uint32_t usrId, void* buf, size_t count)
  \brief       从SPI设备读取数据
  \param[in]   usrId   SPI设备ID
  \param[out]  buf     读取数据的缓冲区指针
  \param[in]   count   要读取的数据大小（字节数）
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于从SPI设备读取数据。
               只有在SPI处于使用中状态时才能进行读操作。
*/
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
/* ---------------------------------------------------------------------------------------------- */
/**
  \fn          uint8_t api_spi_read_write_byte(uint32_t usrId, uint8_t dataOut)
  \brief       通过SPI读写单个字节
  \param[in]   usrId     SPI设备ID
  \param[in]   dataOut   要写入的字节数据
  \return      读取到的字节数据
  \details     该函数用于通过SPI接口读写单个字节数据。
               只有在SPI处于使用中状态时才能进行读写操作。
*/
uint8_t api_spi_read_write_byte(uint32_t usrId, uint8_t dataOut)
{
    uint8_t dataIn = 0;
    api_ret_t ret = api_spi_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_USED)
    { 
        dataIn = spiReadWriteByte(gSpiDrv[index], dataOut);
        ret = OPEN_HAL_DONE;
    }

    return dataIn;
}
#endif