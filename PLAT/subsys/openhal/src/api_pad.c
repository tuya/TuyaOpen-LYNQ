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
#ifdef EPAT_HAL_DEBUG
#define EPAT_LOG(subId, debugLevel, format, ...)  \
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, subId, debugLevel, format, ##__VA_ARGS__)
#else
#define EPAT_LOG(subId, debugLevel, format, ...)
#endif
//默认加载的pad配置表，大小限制为芯片pad数量，可通过读固定区域方式获取
//使用中间层数据；1、有效减少存储数据大小，2、灵活增删项目
uint32_t padStartupList[EC_PAD_INDEX_LIMIT] = {
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x4, 0x4, 0x0, 0x0, 0x0, 0x0,         //11-18
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,         //19-26
    0x2E, 0x2E, 0x0, 0x0, 0x07, 0x07, 0x07, 0x07,   //27-34
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,         //35-42
    0x0, 0x0, 0x2E, 0x2E, 0x0, 0x6, 0x6, 0x6,       //43-50
    0x6, 0x6, 0x28,                                 //51-53
};
static uint32_t sPadRuntimeList[EC_PAD_INDEX_LIMIT] = {0xFF};

//sPadUsrIdList管理设备ID，sPadUsrIdSeed管理ID便宜状态
//谁创建谁回收，删除设备需要匹配传入ID进行校验
static uint32_t sPadUsrIdList[EC_PAD_INDEX_LIMIT] = {0};
static uint16_t sPadUsrIdSeed[EC_PAD_INDEX_LIMIT] = {0};

//状态管理方式：NONE->FREE通过sPadUsrIdList赋值情况确认
//IDLE->USED->IDLE通过osPadBitList状态位确认，空闲置位1

#ifdef FEATURE_OPENHAL_RTOS_ENABLE
#define EC_PAD_EVT_INDEX_MID        (EC_PAD_INDEX_START+24)
// static osEventFlagsId_t osPadBitList11_34 = NULL;  //根据具体芯片PAD编号定义，有效24-bit
// static osEventFlagsId_t osPadBitList35_53 = NULL;
#endif
/* ---------------------------------------------------------------------------------------------- */
// 第一项是PAD编号: 如果不通过全局控制则需配置为-1
// 第二项是MUX功能选择：PAD_MUX_ALT0~PAD_MUX_ALT7
// 第三项是上拉/下拉配置：PAD_AUTO_PULL(自动), PAD_INTERNAL_PULL_UP(内部上拉), PAD_INTERNAL_PULL_DOWN(内部下拉)
// 第四项是locked参数：0=可以被参数覆盖且写入寄存器，1=不可以被新参数覆盖，-1=只读不写寄存器
AP_PLAT_COMMON_DATA static int8_t padList[RTE_PAD_NUM_MAX][4] = {
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD11: SWCLK0/SWCLKA/SWCLKC/GPIO16 */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD12: SWDIO0/SWDIOA/SWDIOC/GPIO17 */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD13: SWCLK1/USP2_LRCK/I2C0_SCL/I2C1_SCL/GPIO18/PWM0/KPC_R4/GNSS_PPS_OUT */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD14: SWDIO1/USP2_LSPI_TE/I2C0_SDA/I2C1_SDA/GPIO19/PWM1/KPC_C4 */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD15: GPIO0/KPC_R4 */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD16: GPIO1/UART1_DCDn/UART1_RTSn/PWM1n/PWM0/KPC_R3/GNSS_PPS_OUT */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD17: GPIO2/UART1_DTRn/UART1_CTSn/ONEW/PWM1/KPC_R2/USP2_LSPI_TE */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD18: GPIO3/USP1_MCLK/USP1_WRX/ONEW/PWM2/KPC_C4 */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD19: GPIO4/USP1_BCLK/I2C1_SDA/UART1_RTSn/USIM1_URSTn/KPC_R1 */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD20: GPIO5/USP1_LRCK/I2C1_SCL/UART1_CTSn/USIM1_UCLK/KPC_R0 */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD21: GPIO6/USP1_DIN/UART2_RXD/UART1_RTSn/USIM1_UIO/KPC_C3 */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD22: GPIO7/USP1_DOUT/UART2_TXD/UART1_CTSn/ONEW/KPC_C2 */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD23: GPIO8/SPI0_SSn0/I2C1_SDA/UART2_RTSn/UART0_RTSn */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD24: GPIO9/SPI0_MOSI/I2C1_SCL/UART2_CTSn/UART0_CTSn */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD25: GPIO10/SPI0_MISO/UART2_RXD */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD26: GPIO11/SPI0_SCLK/SPI1_SSn1/UART2_TXD */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD27: GPIO12/SPI1_SSn0/UART1_RTSn/UART2_RXD/USIM1_UIO/UART3_RTSn/KPC_C1/CAN_RXD */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD28: GPIO13/SPI1_MOSI/UART1_CTSn/UART2_TXD/USIM1_URSTn/UART3_CTSn/KPC_C0/CAN_TXD */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD29: GPIO14/SPI1_MISO/I2C0_SDA/UART3_RXD/USIM1_UCLK/PWM0/KPC_C3/CAN_STB */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD30: GPIO15/SPI1_SCLK/I2C0_SCL/UART3_TXD/USP2_MCLK/PWM1/KPC_C2 */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD31: GPIO16/UART0_RXD/I2C0_SDA */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD32: GPIO17/UART0_TXD/I2C0_SCL */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD33: GPIO18/UART1_RXD */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD34: GPIO19/UART1_TXD */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD35: GPIO29/USP0_BCLK/PWM0 */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD36: GPIO30/USP0_LRCK/PWM1 */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD37: GPIO31/USP0_DIN/USP1_MCLK/PWM2 */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD38: GPIO32/USP0_DOUT/PWM3 */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD39: GPIO33/USP0_MCLK/USP0_WRX/PWM4 */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD40: GPIO34/USP2_BCLK/I2C0_SDA/UART3_RXD */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD41: GPIO35/USP2_LRCK/I2C0_SCL/UART3_TXD */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD42: GPIO36/USP2_DIN/I2C1_SCL/UART0_RTSn */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD43: GPIO37/USP2_DOUT/I2C1_SDA/UART0_CTSn */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD44: GPIO38/USP2_MCLK/USP2_WRX */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD45: GPIO20-AGPIOWU0/PWM4n/FEM7/PWM3/KPC_C2 */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD46: GPIO21-AGPIOWU1/PWM3n/FEM6/PWM4/KPC_C3 */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD47: GPIO22-AGPIOWU2/PWM4n/FEM5/PWM5/KPC_C4 */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD48: GPIO23-AGPIO3/PWM1n/FEM4/PWM0/KPC_R4 */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD49: GPIO24-AGPIO4/PWM0n/FEM3/PWM1/KPC_R3 */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD50: GPIO25-AGPIO5/PWM3n/FEM2/PWM2/KPC_R2/CAN_RXD */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD51: GPIO26-AGPIO6/PWM2n/FEM1/PWM3/KPC_R1/CAN_TXD */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1},          /* PAD52: GPIO27-AGPIO7/PWM5n/FEM0/PWM4/KPC_R0/CAN_STB */
    {-1, PAD_MUX_ALT0, PAD_AUTO_PULL, -1}           /* PAD53: GPIO28-AGPIO8/GNSS_PPS_OUT/PWM4n/ONEW/PWM5/CAN_STB/CAN_RXD */
};

/* ---------------------------------------------------------------------------------------------- */
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
  \brief   参数转换，底层配置参数和方便阅读的用户参数转换  
  \return
*/
static uint32_t ec_pad_config_conv(PadConfig_t *oldConfig, pad_config_t *newConfig, uint8_t fmt)
{
    uint32_t *ret = NULL;
    if(oldConfig == NULL && newConfig == NULL)
    {
        EPAT_LOG(ec_pad_config_conv_0, P_WARNING, "invalid para");
    }
    else if(oldConfig == NULL)
    {
        if(fmt != 0){
            ret = (uint32_t *)newConfig ;  // 返回新参数格式
        }
    }
    else if(newConfig == NULL)
    {
        // 设置/重置默认值
        if(fmt == 0){
            oldConfig->inputControl = 0 ;
            oldConfig->inputForceDisable = 0 ;
            oldConfig->outputForceDisable = 0 ;
            oldConfig->outputControl = 0 ;
            oldConfig->swOutputEnable = 0 ;
            oldConfig->swOutputValue = 0 ;
            oldConfig->pullSelect = 0 ;
            oldConfig->pullUpEnable = 0 ;
            oldConfig->pullDownEnable = 0 ;
            ret = (uint32_t *)oldConfig ;  // 返回旧参数格式
        }
        else {
            pad_config_t Config = {0};
            Config.mux = oldConfig->mux ;
            Config.inputControl = oldConfig->inputControl ;
            Config.inputForceDisable = oldConfig->inputForceDisable ;
            Config.outputForceDisable = oldConfig->outputForceDisable ;
            Config.outputControl = oldConfig->outputControl ;
            Config.swOutputEnable = oldConfig->swOutputEnable ;
            Config.swOutputValue = oldConfig->swOutputValue ;
            Config.pullSelect = oldConfig->pullSelect ;
            Config.pullUpEnable = oldConfig->pullUpEnable ;
            Config.pullDownEnable = oldConfig->pullDownEnable ;
            Config.driveStrength = oldConfig->driveStrength ;
            ret = (uint32_t *)&Config;     // 返回新参数格式
        }
    }
    else
    {
        oldConfig->mux = newConfig->mux;
        oldConfig->inputControl = newConfig->inputControl;
        oldConfig->inputForceDisable = newConfig->inputForceDisable ;
        oldConfig->outputForceDisable = newConfig->outputForceDisable ;
        oldConfig->outputControl = newConfig->outputControl ;
        oldConfig->swOutputEnable = newConfig->swOutputEnable ;
        oldConfig->swOutputValue = newConfig->swOutputValue ;
        oldConfig->pullSelect = newConfig->pullSelect ;
        oldConfig->pullUpEnable = newConfig->pullUpEnable ;
        oldConfig->pullDownEnable = newConfig->pullDownEnable ;
        // oldConfig->driveStrength = newConfig->driveStrength ;
        if(fmt == 0){
            ret = (uint32_t *)oldConfig ;  // 返回旧参数格式
        }
        else {
            ret = (uint32_t *)newConfig ;  // 返回新参数格式
        }
    }
    return *ret;
}

/**
  \fn          api_ret_t check_pad_mux(int8_t paddr, int8_t mux)
  \brief       检查PAD的MUX配置
  \param[in]   paddr  PAD物理地址
  \param[in]   mux    MUX功能选择
  \return      检查结果，OPEN_HAL_INVALID_PARA表示失败，其他值表示成功
  \details     该函数用于查询确认底层PAD功能是否配置正确。
*/
api_ret_t check_pad_mux(int8_t paddr, int8_t mux)
{
    api_ret_t ret = OPEN_HAL_INVALID_PARA ;
    if(paddr < EC_PAD_INDEX_START || paddr >= EC_PAD_INDEX_LIMIT) 
    {
        return ret;
    }
    if(mux < 0 || mux >= PAD_MUX_ALT7) 
    {
        return ret;
    }
    volatile PadConfig_t *pad = (volatile PadConfig_t*)(GP_PAD_BASE_ADDR);
    volatile PadConfig_t *reg = (pad+paddr);
    int8_t (*pList)[4] = padList ;
    uint8_t index = paddr - EC_PAD_INDEX_START;
    if(pList[index][0] != paddr)
    {
        ret = OPEN_HAL_INVALID_PARA ;
        EPAT_LOG(check_pad_mux_1, P_WARNING, "pad%d invalid:%d,%d,%d,%d", paddr, pList[index][0], pList[index][1], pList[index][2], pList[index][3]);
    }
    else if(mux == reg->mux) 
    {
        ret = OPEN_HAL_DONE ;
        EPAT_LOG(check_pad_mux_2, P_INFO, "pad%d:0x%X(0x%x),check mux %d=%d(reg)", paddr, (uintptr_t)reg, *reg, mux, reg->mux);
    }
    return ret;
}

/**
  \fn
  \brief   Reset padList to default state
  \param[in] list  Pointer to the PAD list to reset
  \param[in] count Number of elements in the list
  \return  None
*/
void api_pad_default(int8_t (*list)[4], uint8_t count)
{
    for (uint8_t i = 0; i < count; i++) {
        list[i][0] = -1;
        list[i][1] = PAD_MUX_ALT0;
        list[i][2] = PAD_AUTO_PULL;
        list[i][3] = -1;
    }
}

/**
  \fn          int32_t api_pad_parse(char* str, pad_config_t *cfg)
  \brief       解析PAD配置字符串
  \param[in]   str   配置字符串
  \param[out]  cfg   解析后的配置参数结构体指针
  \return      PAD索引编号
  \details     该函数用于解析CSV格式的PAD配置字符串，并填充到配置结构体中。
*/
int32_t api_pad_parse(char* str, pad_config_t *cfg)
{
    if(str == NULL || cfg == NULL){
        EPAT_LOG(api_pad_parse_0, P_ERROR, "invalid para");
        return -1;
    }
    uint8_t num = 0;
    char *item_str[CSV_PAD_ITEM_MAX];
    char *head = strtok(str, ",");
    while(head != NULL && num<CSV_PAD_ITEM_MAX)
    {
        item_str[num] = head;
        head = strtok(NULL, ",");
        num ++;
    }
    if(num < CSV_PAD_ITEM_MAX){
        EPAT_LOG(api_pad_parse_1, P_ERROR, "input items %d < %d", num, CSV_PAD_ITEM_MAX);
        return -1;
    }
    int8_t index = atoi(item_str[0]);   // index
    if(index < EC_PAD_INDEX_START || index >= EC_PAD_INDEX_LIMIT) 
    {
        EPAT_LOG(api_pad_parse_2, P_ERROR, "paddr%d invalid", index);
        return -2;
    }
    int32_t value = atoi(item_str[1]);
    if(value >= 0 && value < 8) // mux
    {
        cfg->mux = value;
    }
    value = atoi(item_str[2]);
    if(value >= 0 && value < 2)
    {
        cfg->pullUpEnable = value;
    }
    value = atoi(item_str[3]);
    if(value >= 0 && value < 2)
    {
        cfg->pullDownEnable = value;
    }
    value = atoi(item_str[4]);
    if(value >= 0 && value < 2)
    {
        cfg->pullSelect = value;
    }
    value = atoi(item_str[5]);
    if(value >= 0 && value < 2)
    {
        cfg->inputControl = value;
    }
    value = atoi(item_str[6]);
    if(value >= 0 && value < 2)
    {
        cfg->outputControl = value;
    }
    EPAT_LOG(api_pad_parse, P_INFO, "pad%d:mux %d,pullUp %d,pullDown %d,Select %d,input %d,output %d", index, \
        cfg->mux, cfg->pullUpEnable, cfg->pullDownEnable, cfg->pullSelect, cfg->inputControl, cfg->outputControl);
    return index;
}

/* ---------------------------------------------------------------------------------------------- */
/**
  \fn          
  \brief       
  \return
*/
static uint8_t pad_set_free(uint8_t paddr)
{
    if(paddr >= EC_PAD_INDEX_START && paddr < EC_PAD_INDEX_LIMIT)
    {
        sPadUsrIdList[paddr] = OPEN_HAL_STAT_UNUSED;
        // if(cfg == NULL) return 0;
        // if(cfg->mux < 0x07)
        // {
        //     PadConfig_t padConfig = {0};
        //     sPadRuntimeList[paddr] = ec_pad_config_conv(&padConfig, cfg, 1);
        //     PAD_setPinConfig(paddr,&padConfig);
        //     return 1;
        // }
        // else sPadRuntimeList[paddr] = 0xFF;
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
    // 添加边界检查以防止数组越界
    if(paddr >= EC_PAD_INDEX_LIMIT) return 0;
    
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
    // 添加边界检查以防止数组越界
    if(paddr >= EC_PAD_INDEX_LIMIT) return 0;
    
    if(sPadRuntimeList[paddr] == 0xFF) return 0;
    if(paddr >= EC_PAD_INDEX_START && paddr < EC_PAD_INDEX_LIMIT)
    {
        if(sPadUsrIdList[paddr] == OPEN_HAL_STAT_UNUSED){
            sPadUsrIdList[paddr] = paddr;
            sPadUsrIdSeed[paddr] = 1;
            sPadUsrIdList[paddr] |= (uint32_t)(sPadUsrIdSeed[paddr] << 16);
        }
        else if(sPadUsrIdList[paddr] & OPEN_HAL_STAT_MUSK){
            sPadUsrIdList[paddr] &= ~(OPEN_HAL_STAT_MUSK);
        }
    }
    return sPadUsrIdList[paddr];
}

/**
  \fn          api_ret_t api_pad_query(uint32_t usrId)
  \brief       查询PAD设备状态
  \param[in]   usrId  PAD设备ID
  \return      PAD设备当前状态
  \details     该函数用于查询指定PAD设备的当前状态（空闲、使用中等）。
*/
api_ret_t api_pad_query(uint32_t usrId)
{
    // ASSERT(usrId > 0);
    uint32_t paddr = usrId_to_paddr(usrId);
    
    // 添加边界检查以防止数组越界
    if(paddr >= EC_PAD_INDEX_LIMIT) return OPEN_HAL_NONE;
    
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
  \brief
  \return
*/
static uint8_t pad_val_get(PadConfig_t *oldConfig, pad_config_t *newConfig)
{
    uint8_t intVal = 0;
    if(newConfig != NULL && oldConfig != NULL)
    {
        newConfig->pullSelect = oldConfig->pullSelect ;
        newConfig->pullUpEnable = oldConfig->pullUpEnable ;
        newConfig->pullDownEnable = oldConfig->pullDownEnable ;
    }
    if(oldConfig != NULL)
    {
        if(oldConfig->pullSelect == PAD_PULL_AUTO){
            intVal = PAD_AUTO_PULL ;
        }
        else if(oldConfig->pullDownEnable != 0){
            intVal = PAD_INTERNAL_PULL_DOWN ;
        }
        else if(oldConfig->pullUpEnable != 0){
            intVal = PAD_INTERNAL_PULL_UP ;
        }
    }
    else if(newConfig != NULL)
    {
        if(newConfig->pullSelect == PAD_PULL_AUTO){
            intVal = PAD_AUTO_PULL ;
        }
        else if(newConfig->pullDownEnable != 0){
            intVal = PAD_INTERNAL_PULL_DOWN ;
        }
        else if(newConfig->pullUpEnable != 0){
            intVal = PAD_INTERNAL_PULL_UP ;
        }
    }
    return intVal;
}

/* ---------------------------------------------------------------------------------------------- */
/**
  \fn          
  \brief    单项初始化，使用HAL统一的参数格式
  \return
*/
api_ret_t api_pad_setup(int8_t paddr, pad_config_t* para)
{
    volatile PadConfig_t *pad = (volatile PadConfig_t*)(GP_PAD_BASE_ADDR);
    // 检查参数有效性
    if(paddr < EC_PAD_INDEX_START || paddr >= EC_PAD_INDEX_LIMIT) 
    {
        EPAT_LOG(api_pad_setup_0, P_ERROR, "pad%d:over range[%d,%d]", paddr, EC_PAD_INDEX_START, EC_PAD_INDEX_LIMIT);
        return OPEN_HAL_INVALID_PARA;
    }
    int8_t (*pList)[4] = padList;
    uint8_t index = paddr - EC_PAD_INDEX_START;
    if(paddr != pList[index][0])
    {
        pList[index][0] = paddr;
    }
    int8_t control = pList[index][3];
    volatile PadConfig_t *reg = (pad+paddr);
    if(control <= 0 && para != NULL && (pad_config_t *)pList[index] != para)    // 更新到运行状态表
    {
        pList[index][1] = para->mux;
        pList[index][2] = pad_val_get(NULL, para);
        pList[index][3] = para->locked;
        control = pList[index][3];
    }
    if(control < 0) // 读取参数
    {
        pList[index][1] = reg->mux ;
        pList[index][2] = pad_val_get((PadConfig_t *)reg, para);
        EPAT_LOG(api_pad_setup_1, P_INFO, "get pad%d,reg 0x%x,mux %d,%d", paddr, *reg, pList[index][1], pList[index][2]);
    }
    else
    {
        PadConfig_t padConfig = {0};
        padConfig.mux = pList[index][1];
        uint8_t intVal = pList[index][2];
        if(intVal == PAD_PULL_AUTO){
            padConfig.pullDownEnable = 0U ; 
            padConfig.pullUpEnable = 0U ;
            padConfig.pullSelect = 0U ;
        }
        else if(intVal == PAD_INTERNAL_PULL_DOWN){
            padConfig.pullDownEnable = 1U ;
            padConfig.pullSelect = 1U ;
        }
        else if(intVal == PAD_INTERNAL_PULL_UP){
            padConfig.pullUpEnable = 1U ;
            padConfig.pullSelect = 1U ;
        }
        CLOCK_clockEnable(PCLK_PAD);
        pad[paddr] = padConfig ;
        CLOCK_clockDisable(PCLK_PAD);
    }
    EPAT_LOG(api_pad_setup_end, P_DEBUG, "pad%d:mux %d=%d", paddr, reg->mux, pList[index][1]);
    return OPEN_HAL_DONE;
}

/**
  \fn          int8_t *api_pad_startup(int8_t *para)
  \brief       全局初始化，传入的参数需要是完整的pad表而非某个pad配置项，便于默认修改参数统一int8_t
  \param[in]   para  指向PAD配置参数数组的指针
  \return      返回指向PAD状态列表的指针
  \details     该函数用于在系统上电时初始化所有PAD设备。它会遍历所有可用的PAD索引，
               并根据传入的配置参数进行初始化设置。如果para不为NULL，则会使用传入的参数更新默认配置。
*/
int8_t *api_pad_startup(int8_t *para)
{
    uint8_t num = 0;
    // uint8_t total = sizeof(padList)/sizeof(padList[0]); // 根据实际表单更新
    if(para != NULL)
    {
        int8_t (*newList)[4] = (int8_t (*)[4])para;
        for(num = 0; num < RTE_PAD_NUM_MAX; num++) // 最多配置所有PAD: 超出部分不处理
        {
            if(newList[num][0] < EC_PAD_INDEX_START || newList[num][0] > EC_PAD_INDEX_LIMIT){
                break;
            }
            if(newList[num][1] > PAD_MUX_ALT7 || newList[num][2] > PAD_AUTO_PULL){
                break;
            }
            // if(newList[num][0] >= EC_PAD_INDEX_START && newList[num][0] < EC_PAD_INDEX_LIMIT)
            {
                uint8_t index = newList[num][0] - EC_PAD_INDEX_START;
                if(padList[index][3] > 0)   // 已锁定不更新
                {
                    // EPAT_LOGD("%d/%d,pad%d,locked %d,%d,%d", num, index, padList[index][0], padList[index][1], padList[index][2], padList[index][3]);
                }
                else {
                    padList[index][0] = newList[num][0] ;
                    padList[index][1] = newList[num][1] ;
                    padList[index][2] = newList[num][2] ;
                    padList[index][3] = newList[num][3] ;
                    // EPAT_LOGD("update pad%d,mux %d,intval %d,control %d", pList[index][0], pList[index][1], pList[index][2], pList[index][3]);
                }
            }
        }
    }
    // uint8_t inited_pad = 0;
    for(uint8_t i = EC_PAD_INDEX_START; i < EC_PAD_INDEX_LIMIT; i++)
    {
        uint8_t index = i - EC_PAD_INDEX_START;
        pad_set_free(i);
        // sPadUsrIdList[i] = OPEN_HAL_STAT_UNUSED;
        if(padList[index][0] == i)   // 有效状态
        {
            api_pad_setup(i, NULL);
            // inited_pad ++;
        }
    }
    return (int8_t *)padList;
}

// 以上为硬件级接口，实现对底层配置管理
/* ---------------------------------------------------------------------------------------------- */
// 如下为系统级接口，通过usrId统一识别
/**
  \fn          api_ret_t api_pad_create(uint32_t paddr, void *out)
  \brief       创建PAD设备实例，分配资源并检查依赖条件
  \param[in]   paddr  PAD物理地址
  \param[out]  out    输出参数，返回创建的PAD设备ID
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于创建一个新的PAD设备实例，分配相关资源并进行初始化。
               如果对应的PAD已经被锁定，则返回锁定状态。
*/
api_ret_t api_pad_create(uint32_t paddr, void *out)
{
    uint32_t usrId = 0;
    api_ret_t ret = OPEN_HAL_INVALID_PARA;
    
    // 添加边界检查以防止数组越界
    if(paddr < EC_PAD_INDEX_START || paddr >= EC_PAD_INDEX_LIMIT) 
    {
        EPAT_LOG(api_pad_create_0, P_ERROR, "invalid paddr%d", paddr);
        return OPEN_HAL_INVALID_PARA;
    }
    
    int8_t (*pList)[4] = padList;
    uint8_t index = paddr - EC_PAD_INDEX_START;
    if(index < RTE_PAD_NUM_MAX)
    {
        if(pList[index][3] > 0)   // 已锁定不更新
        {
            EPAT_LOG(api_pad_create_1, P_INFO, "paddr %d,locked %d", paddr, pList[index][3]);
            ret = OPEN_HAL_LOCK ;
        }
        else {
            pList[index][3] = 0 ;   // 解除读写限制，配置可写状态
            pList[index][0] = paddr ;
            usrId = pad_set_idle(paddr);
            ret = OPEN_HAL_DONE ;
            EPAT_LOG(api_pad_create_2, P_INFO, "paddr %d,usrId 0x%X,val %d-%d", paddr, usrId, pList[index][1], pList[index][2]);
        }
    }
    if(out != NULL) *(uint32_t *)out = usrId;
    return ret;
}

/**
  \fn          api_ret_t api_pad_delete(uint32_t usrId)
  \brief       删除PAD设备实例，释放相关资源
  \param[in]   usrId  PAD设备ID
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于删除指定的PAD设备实例，释放相关资源。
               只有在PAD处于空闲状态时才能被删除。
*/
api_ret_t api_pad_delete(uint32_t usrId)
{
    int8_t (*pList)[4] = padList;
    api_ret_t ret = api_pad_query(usrId);
    uint32_t paddr = usrId_to_paddr(usrId);
    if(ret == OPEN_HAL_IDLE)
    {
        pad_set_free(paddr);
        uint8_t index = paddr - EC_PAD_INDEX_START;
        pList[index][3] = 0 ;
        ret = OPEN_HAL_DONE;
    }
    return ret;
}

/* ---------------------------------------------------------------------------------------------- */
/**
  \fn          api_ret_t api_pad_open(uint32_t usrId, void *cfg, size_t timeout)
  \brief       打开PAD设备
  \param[in]   usrId    PAD设备ID
  \param[in]   cfg      PAD配置参数指针（可为NULL）
  \param[in]   timeout  超时时间（暂未使用）
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于打开PAD设备并根据配置参数进行设置。
               只有在PAD处于空闲状态时才能被打开。
*/
api_ret_t api_pad_open(uint32_t usrId, void *cfg, size_t timeout)
{
    int8_t (*pList)[4] = padList;
    api_ret_t ret = api_pad_query(usrId);
    uint32_t paddr = usrId_to_paddr(usrId);
    uint8_t index = paddr - EC_PAD_INDEX_START;
    if(ret == OPEN_HAL_IDLE)
    {
        if(cfg != NULL)
        {
            PadConfig_t *padConfig = (PadConfig_t *)cfg;
            if(padConfig->mux < 7U && padConfig->outputControl < 2U)
            {
                pList[index][1] = padConfig->mux ;
                pList[index][2] = padConfig->outputControl ;
                // sPadRuntimeList[paddr] = ec_pad_config_conv(padConfig,NULL,1);
                PAD_setPinConfig(paddr, padConfig);
            }
        }
        pad_set_used(paddr);
        ret = OPEN_HAL_DONE;
    }
    // SYSLOG_INFO("[%d][0x%08X]%d\r\n",paddr,usrId,ret);
    return ret;
}

/**
  \fn          api_ret_t api_pad_close(uint32_t usrId)
  \brief       关闭PAD设备
  \param[in]   usrId  PAD设备ID
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于关闭指定的PAD设备，将其状态设置为空闲。
               只有在PAD处于使用中状态时才能被关闭。
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
  \fn          api_ret_t api_pad_apply(uint32_t usrId,void *cb)
  \brief       占用pad并回调，依赖rtos
  \param[in]   usrId  PAD设备ID
  \param[in]   cb     回调函数指针
  \return      执行结果
  \details     系统级接口，非阻塞，获取pad成功后回调，可用于共用外设端口情况
  \note        待实现：需要根据RTOS环境实现具体功能
*/
api_ret_t api_pad_apply(uint32_t usrId,void *cb)
{
    // TODO: 实现PAD申请和回调功能
    return OPEN_HAL_NONE;
}

/**
  \fn          api_ret_t api_pad_ioctl(uint32_t usrId,api_pad_ioctl_t type, void *para)
  \brief       PAD设备控制接口，用于配置设备的各种参数
  \param[in]   usrId  PAD设备ID
  \param[in]   type   控制类型，参考api_pad_ioctl_t枚举
  \param[in]   para   控制参数指针
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于对PAD设备进行各种控制操作，如设置功能、上拉/下拉电阻等。
               只有在PAD处于使用中状态时才能进行控制操作。
*/
api_ret_t api_pad_ioctl(uint32_t usrId,api_pad_ioctl_t type, void *para)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_pad_query(usrId);
    uint32_t paddr = usrId_to_paddr(usrId);
    
    // 添加边界检查以防止数组越界
    if(paddr >= EC_PAD_INDEX_LIMIT) return OPEN_HAL_INVALID_PARA;
    
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
                    ec_pad_config_conv(&padConfig,(pad_config_t *)&sPadRuntimeList[paddr],1);
                    PAD_setPinConfig(paddr,&padConfig);
                }
                break;
            case OPEN_PAD_IOCTL_PULLUP:
                if(val < 2U)    // 使用逻辑上依赖para输入，直接位操作理解难度提高
                {
                    if(val) sPadRuntimeList[paddr] |= 0x08;
                    else sPadRuntimeList[paddr] &= 0xF7;
                    ec_pad_config_conv(&padConfig,(pad_config_t *)&sPadRuntimeList[paddr],1);
                    PAD_setPinConfig(paddr,&padConfig);
                    PAD_setPinPullConfig(paddr, PAD_INTERNAL_PULL_UP);
                }
                break;
            case OPEN_PAD_IOCTL_PULLDOWN:
                if(val < 2U){
                    if(val) sPadRuntimeList[paddr] |= 0x10;
                    else sPadRuntimeList[paddr] &= 0xEF;
                    ec_pad_config_conv(&padConfig,(pad_config_t *)&sPadRuntimeList[paddr],1);
                    PAD_setPinConfig(paddr,&padConfig);
                    PAD_setPinPullConfig(paddr, PAD_INTERNAL_PULL_DOWN);
                }
                break;
            case OPEN_PAD_IOCTL_PULLSELECT:
                if(val < 2U){
                    if(val) sPadRuntimeList[paddr] |= 0x20; //PAD_PULL_INTERNAL
                    else sPadRuntimeList[paddr] &= 0xDF;
                    ec_pad_config_conv(&padConfig,(pad_config_t *)&sPadRuntimeList[paddr],1);
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
  \fn          api_ret_t api_pad_pmctl(uint32_t usrId,open_hal_pm_t *cfg, size_t count)
  \brief       对设备功耗和模式进行配置
  \param[in]   usrId  PAD设备ID
  \param[in]   cfg    功耗配置参数指针
  \param[in]   count  参数数量（暂未使用）
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于控制PAD设备的功耗模式。
               只有在PAD处于使用中状态时才能进行功耗控制。
*/
api_ret_t api_pad_pmctl(uint32_t usrId,open_hal_pm_t *cfg, size_t count)
{
    api_ret_t ret = api_pad_query(usrId);
    // uint32_t paddr = usrId_to_paddr(usrId);
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
    return OPEN_HAL_DONE;
}

/**
  \fn          api_ret_t api_pad_write(uint32_t usrId, void* buf, size_t count)
  \brief       对设备进行写操作  
  \param[in]   usrId   PAD设备ID
  \param[in]   buf     要写入的数据缓冲区指针（暂未使用）
  \param[in]   count   要写入的数据大小（字节数，暂未使用）
  \return      执行结果
  \details     该函数用于向PAD设备写入数据。目前尚未实现具体功能。
  \note        待实现：需要根据具体应用场景实现PAD写操作
*/
api_ret_t api_pad_write(uint32_t usrId, void* buf, size_t count)
{
    // TODO: 实现PAD写操作功能
    return OPEN_HAL_NONE;
}

/**
  \fn          api_ret_t api_pad_read(uint32_t usrId, void* buf, size_t count)
  \brief       对设备进行读操作   
  \param[in]   usrId   PAD设备ID
  \param[out]  buf     读取数据的缓冲区指针（暂未使用）
  \param[in]   count   要读取的数据大小（字节数，暂未使用）
  \return      执行结果
  \details     该函数用于从PAD设备读取数据。目前尚未实现具体功能。
  \note        待实现：需要根据具体应用场景实现PAD读操作
*/
api_ret_t api_pad_read(uint32_t usrId, void* buf, size_t count)
{
    // TODO: 实现PAD读操作功能
    return OPEN_HAL_NONE;
}

/**
  \fn          int api_test_pad(void)
  \brief       PAD设备测试接口
  \return      测试结果
  \details     该函数用于测试PAD设备的基本功能。目前尚未实现具体功能。
  \note        待实现：需要实现PAD设备的完整测试流程
*/
int api_test_pad(void)
{
    // TODO: 实现PAD测试功能

    return 0;
}
