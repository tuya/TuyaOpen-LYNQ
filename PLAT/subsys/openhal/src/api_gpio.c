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
#ifndef ATTRIBUTE_ISR_HEAD
    #ifdef CHIP_EC716
        #define ATTRIBUTE_ISR_HEAD
    #else
        #define ATTRIBUTE_ISR_HEAD  PLAT_FPSRAM_RAMCODE_CUST
    #endif
#endif

#if defined CHIP_EC716
#else   //718
const int8_t list_pad2gpio[] = {
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,       // <11
    16,17,18,19,                            // <15 MUX4
    0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,  // <30
    16,17,18,19,                            // <35 MUX0
    29,30,31,32,33,34,35,36,37,38,
    20,21,22,23,24,25,26,27,28
};
#endif

static uint32_t sGpioUsrIdList[EC_GPIO_INDEX_LIMIT] = {0};
static int8_t sGpioRuntimeConfig[EC_GPIO_INDEX_LIMIT] = {0};

static uint16_t portMusk[3] = {0};
static IsrFunc gpio_isr_cb[3][16] = {NULL};
static uint8_t isr_used_cnt = 0;
#ifdef FEATURE_OPENHAL_RTOS_ENABLE
#define EC_PAD_EVT_INDEX_MID        (EC_GPIO_INDEX_START+24)
// static osEventFlagsId_t osGpioBitList0_23 = NULL;  
// static osEventFlagsId_t osGpioBitList24_38 = NULL;  
#endif
#ifdef EPAT_HAL_DEBUG
#define EPAT_LOG(subId, debugLevel, format, ...)  \
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, subId, debugLevel, format, ##__VA_ARGS__)
#else
#define EPAT_LOG(subId, debugLevel, format, ...)
#endif
/* ---------------------------------------------------------------------------------------------- */
// 查询pad-mux-gpio关系表：>39；支持多mux对应关系
const int8_t gpio_PinTable[][3] = {
    {A4_GPIO16, PAD_MUX_ALT4, 16},
    {A4_GPIO17, PAD_MUX_ALT4, 17},
    {A4_GPIO18, PAD_MUX_ALT4, 18},
    {A4_GPIO19, PAD_MUX_ALT4, 19},
    {A0_GPIO0,  PAD_MUX_ALT0, 0},
    {A0_GPIO1,  PAD_MUX_ALT0, 1},
    {A0_GPIO2,  PAD_MUX_ALT0, 2},
    {A0_GPIO3,  PAD_MUX_ALT0, 3},
    {A0_GPIO4,  PAD_MUX_ALT0, 4},
    {A0_GPIO5,  PAD_MUX_ALT0, 5},
    {A0_GPIO6,  PAD_MUX_ALT0, 6},
    {A0_GPIO7,  PAD_MUX_ALT0, 7},
    {A0_GPIO8,  PAD_MUX_ALT0, 8},
    {A0_GPIO9,  PAD_MUX_ALT0, 9},
    {A0_GPIO10, PAD_MUX_ALT0, 10},
    {A0_GPIO11, PAD_MUX_ALT0, 11},
    {A0_GPIO12, PAD_MUX_ALT0, 12},
    {A0_GPIO13, PAD_MUX_ALT0, 13},
    {A0_GPIO14, PAD_MUX_ALT0, 14},
    {A0_GPIO15, PAD_MUX_ALT0, 15},
    {A0_GPIO16, PAD_MUX_ALT0, 16},
    {A0_GPIO17, PAD_MUX_ALT0, 17},
    {A0_GPIO18, PAD_MUX_ALT0, 18},
    {A0_GPIO19, PAD_MUX_ALT0, 19},
    {A0_GPIO20, PAD_MUX_ALT0, 20},
    {A0_GPIO21, PAD_MUX_ALT0, 21},
    {A0_GPIO22, PAD_MUX_ALT0, 22},
    {A0_GPIO23, PAD_MUX_ALT0, 23},
    {A0_GPIO24, PAD_MUX_ALT0, 24},
    {A0_GPIO25, PAD_MUX_ALT0, 25},
    {A0_GPIO26, PAD_MUX_ALT0, 26},
    {A0_GPIO27, PAD_MUX_ALT0, 27},
    {A0_GPIO28, PAD_MUX_ALT0, 28},
    {A0_GPIO29, PAD_MUX_ALT0, 29},
    {A0_GPIO30, PAD_MUX_ALT0, 30},
    {A0_GPIO31, PAD_MUX_ALT0, 31},
    {A0_GPIO32, PAD_MUX_ALT0, 32},
    {A0_GPIO33, PAD_MUX_ALT0, 33},
    {A0_GPIO34, PAD_MUX_ALT0, 34},
    {A0_GPIO35, PAD_MUX_ALT0, 35},
    {A0_GPIO36, PAD_MUX_ALT0, 36},
    {A0_GPIO37, PAD_MUX_ALT0, 37},
    {A0_GPIO38, PAD_MUX_ALT0, 38}
};
/* ---------------------------------------------------------------------------------------------- */
// 运行状态表
// 第一项是GPIO编号：如果对应的序号没有被配置，则置为-1表示不是gpio功能
// 第二项是GPIO方向：GPIO_DIRECTION_INPUT 或 GPIO_DIRECTION_OUTPUT
// 第三项是初始值配置：对于输出为初始电平(0/1)，对于输入为中断类型(0=无中断, 1=低电平, 2=高电平, 3=下降沿, 4=上升沿, 5=双边沿)
// 第四项是locked参数：0=可以被参数覆盖且写入寄存器，1=不可以被新参数覆盖，-1=只读不写寄存器
AP_PLAT_COMMON_DATA static int8_t gpioList[RTE_GPIO_NUM_MAX][4] = {
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO0: KPC_R4 */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO1: PWM1n/PWM0/KPC_R3/GNSS_PPS_OUT */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO2: UART1_DTRn/UART1_CTSn/ONEW/PWM1/KPC_R2/USP2_LSPI_TE */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO3: USP1_MCLK/USP1_WRX/ONEW/PWM2/KPC_C4 */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO4: USP1_BCLK/I2C1_SDA/UART1_RTSn/USIM1_URSTn/KPC_R1 */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO5: USP1_LRCK/I2C1_SCL/UART1_CTSn/USIM1_UCLK/KPC_R0 */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO6: USP1_DIN/UART2_RXD/UART1_RTSn/USIM1_UIO/KPC_C3 */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO7: USP1_DOUT/UART2_TXD/UART1_CTSn/ONEW/KPC_C2 */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO8: SPI0_SSn0/I2C1_SDA/UART2_RTSn/UART0_RTSn */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO9: SPI0_MOSI/I2C1_SCL/UART2_CTSn/UART0_CTSn */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO10: SPI0_MISO/UART2_RXD */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO11: SPI0_SCLK/SPI1_SSn1/UART2_TXD */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO12: SPI1_SSn0/UART1_RTSn/UART2_RXD/USIM1_UIO/UART3_RTSn/KPC_C1/CAN_RXD */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO13: SPI1_MOSI/UART1_CTSn/UART2_TXD/USIM1_URSTn/UART3_CTSn/KPC_C0/CAN_TXD */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO14: SPI1_MISO/I2C0_SDA/UART3_RXD/USIM1_UCLK/PWM0/KPC_C3/CAN_STB */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO15: SPI1_SCLK/I2C0_SCL/UART3_TXD/USP2_MCLK/PWM1/KPC_C2 */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO16: UART0_RXD/I2C0_SDA/SWCLK1 */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO17: UART0_TXD/I2C0_SCL/SWDIO1 */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO18: UART1_RXD/USP2_LRCK/I2C0_SCL/I2C1_SCL/PWM0/KPC_R4/GNSS_PPS_OUT */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO19: UART1_TXD */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO20: AGPIOWU0/PWM4n/FEM7/PWM3/KPC_C2 */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO21: AGPIOWU1/PWM3n/FEM6/PWM4/KPC_C3 */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO22: AGPIOWU2/PWM4n/FEM5/PWM5/KPC_C4 */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO23: AGPIO3/PWM1n/FEM4/PWM0/KPC_R4 */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO24: AGPIO4/PWM0n/FEM3/PWM1/KPC_R3 */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO25: AGPIO5/PWM3n/FEM2/PWM2/KPC_R2/CAN_RXD */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO26: AGPIO6/PWM2n/FEM1/PWM3/KPC_R1/CAN_TXD */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO27: AGPIO7/PWM5n/FEM0/PWM4/KPC_R0/CAN_STB */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO28: AGPIO8/GNSS_PPS_OUT/PWM4n/ONEW/PWM5/CAN_STB/CAN_RXD */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO29: USP0_BCLK/PWM0 */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO30: USP0_LRCK/PWM1 */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO31: USP0_DIN/USP1_MCLK/PWM2 */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO32: USP0_DOUT/PWM3 */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO33: USP0_MCLK/USP0_WRX/PWM4 */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO34: USP2_BCLK/I2C0_SDA/UART3_RXD */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO35: USP2_LRCK/I2C0_SCL/UART3_TXD */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO36: USP2_DIN/I2C1_SCL/UART0_RTSn */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1},    /* GPIO37: USP2_DOUT/I2C1_SDA/UART0_CTSn */
    {-1, GPIO_DIRECTION_INPUT, GPIO_INTERRUPT_DISABLED, -1}     /* GPIO38: USP2_MCLK/USP2_WRX */
};

/**
  \fn
  \brief
  \return
*/
api_ret_t api_gpio_checkout(int8_t index)
{
    api_ret_t ret = OPEN_HAL_NONE;
    if(index < EC_GPIO_INDEX_START || index >= EC_GPIO_INDEX_LIMIT)
    {
        EPAT_LOG(api_gpio_checkout_0, P_ERROR, "invalid gpio%d", index);
        return OPEN_HAL_INVALID_PARA ;
    }
#ifdef EPAT_HAL_DEBUG
    int8_t mux = -1;
#endif
    uint8_t paddr = 0;
    for (uint8_t i = 0; i < sizeof(gpio_PinTable)/sizeof(gpio_PinTable[0]); i++) 
    {
        if(index == gpio_PinTable[i][2])
        {
            paddr = gpio_PinTable[i][0] ;
            if(check_pad_mux(paddr, gpio_PinTable[i][1]) == OPEN_HAL_DONE)
            {
                ret = OPEN_HAL_FREE ;
#ifdef EPAT_HAL_DEBUG
                mux = gpio_PinTable[i][1] ;
#endif
                EPAT_LOG(api_gpio_checkout_1, P_INFO, "checkout gpio%d with pad%d mux%d", index, paddr, mux);
                break;
            }
        }
    }
    if(paddr >= EC_PAD_INDEX_START && ret == OPEN_HAL_FREE)
    {
        int8_t (*pList)[4] = gpioList;
        if(pList[index][3] > 0)
        {
            ret = OPEN_HAL_LOCK ;
        }
        else if(pList[index][0] >= EC_GPIO_INDEX_START)
        {
            ret = OPEN_HAL_DONE ;
            EPAT_LOG(api_gpio_checkout_ok, P_INFO, "checkout gpio%d status:dir %d,intval %d,control %d", index, pList[index][1], pList[index][2], pList[index][3]);
        }
    }
    else {
        EPAT_LOG(api_gpio_checkout_fail, P_ERROR, "gpio%d,paddr %d,mux %d,fail ret %d", index, paddr, mux, ret);
    }
    return ret;
}

/**
  \fn          void api_gpio_default(int8_t (*list)[4], uint8_t count)
  \brief       重置GPIO列表到默认状态
  \param[in]   list  指向GPIO列表的指针
  \param[in]   count 列表中元素的数量
  \return      无
  \details     该函数用于将GPIO列表重置为默认状态。它会遍历列表中的所有元素，
               将每个GPIO的配置重置为默认值：GPIO编号设为-1（未配置），
               方向设为输入(GPIO_DIRECTION_INPUT)，中断类型设为禁用(GPIO_INTERRUPT_DISABLED)，
               locked参数设为-1（只读不写寄存器）。
*/
void api_gpio_default(int8_t (*list)[4], uint8_t count)
{
    for (uint8_t i = 0; i < count; i++) {
        list[i][0] = -1;
        list[i][1] = GPIO_DIRECTION_INPUT;
        list[i][2] = GPIO_INTERRUPT_DISABLED;
        list[i][3] = -1;
    }
}

/* ---------------------------------------------------------------------------------------------- */
/**
  \fn          static uint32_t usrId_to_gpio(uint32_t usrId)
  \brief       usrID生成方式逆向算法
  \param[in]   usrId  用户ID
  \return      GPIO索引编号
  \details     该函数用于从用户ID中提取GPIO索引编号。通过位运算从usrId中提取低几位作为GPIO索引，
               并验证该索引是否有效以及是否与用户ID匹配。
*/
static uint32_t usrId_to_gpio(uint32_t usrId)
{ 
    // ASSERT(usrId > 0);
    uint32_t gpio = (uint32_t)(usrId & OPEN_HAL_PORT_MUSK); //位提取
    // ASSERT(gpio < EC_GPIO_INDEX_LIMIT);
    if(gpio == (sGpioUsrIdList[gpio] & OPEN_HAL_PORT_MUSK))
    {
        return gpio;
    }
    return EC_GPIO_INDEX_LIMIT;
}

/* ---------------------------------------------------------------------------------------------- */
/**
  \fn          static void gpioIsrSet(int8_t gpio, IsrFunc cb)
  \brief       设置GPIO中断回调函数
  \param[in]   gpio  GPIO索引编号
  \param[in]   cb    中断回调函数指针
  \return      无
  \details     该函数用于设置指定GPIO的中断回调函数。它会根据GPIO编号计算出端口和引脚，
               并在回调函数指针不为NULL时设置相应的位掩码，否则清除位掩码。
*/
static void gpioIsrSet(int8_t gpio, IsrFunc cb)
{
    if(gpio < 0 || gpio >= RTE_GPIO_NUM_MAX) 
    {
        return ;
    }
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

/* ---------------------------------------------------------------------------------------------- */
/**
  \fn          api_ret_t api_gpio_query(uint32_t usrId)
  \brief       查询GPIO设备状态
  \param[in]   usrId  GPIO设备ID
  \return      GPIO设备当前状态
  \details     该函数用于查询指定GPIO设备的当前状态（空闲、使用中等）。
               根据用户ID提取GPIO索引，然后检查该GPIO在状态列表中的状态。
               返回值可以是OPEN_HAL_FREE（未分配）、OPEN_HAL_IDLE（空闲）、
               OPEN_HAL_USED（使用中）或OPEN_HAL_NONE（无效）。
*/
api_ret_t api_gpio_query(uint32_t usrId)
{
    uint32_t index = usrId_to_gpio(usrId);
    // ASSERT(index < EC_GPIO_INDEX_LIMIT);
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
  \fn          static uint32_t gpio_set_free(uint32_t index, gpio_config_t *cfg)
  \brief       将GPIO设备设置为空闲状态
  \param[in]   index  GPIO索引编号
  \param[in]   cfg    GPIO配置参数指针
  \return      设置结果，1表示成功，0表示失败
  \details     该函数用于将指定的GPIO设备设置为空闲状态，释放相关资源。
               它会将GPIO的状态设置为未使用，并清除中断回调函数。
*/
static uint32_t gpio_set_free(uint32_t index, gpio_config_t *cfg)
{
    (void)cfg;
    if(index >= EC_GPIO_INDEX_START && index < EC_GPIO_INDEX_LIMIT)
    {
        sGpioUsrIdList[index] = OPEN_HAL_STAT_UNUSED;
        uint8_t pin=index%16;
        uint8_t port=index/16;
        gpio_isr_cb[port][pin] = NULL; 
        int8_t (*pList)[4] = gpioList;  // 新的内存状态管理，可以管理归属用户
        if(pList[index][3] > 0)         // 已被用户锁定情况下解除锁定
        {
            EPAT_LOG(gpio_set_free_1, P_INFO, "free gpio%d,used by %d",index, pList[index][3]);
            // pList[index][3] = 0 ;    暂时不解除锁定
        }
        return 1;
    }
    return 0;
}

/**
  \fn          
  \brief    在创建时调用配置相应状态
  \return   生成并返回用户ID
*/
static uint32_t gpio_set_idle(uint32_t index)
{
    if(sGpioUsrIdList[index] == OPEN_HAL_STAT_UNUSED)
    {
        sGpioUsrIdList[index] = index;
        uint16_t seed = 1;
        sGpioUsrIdList[index] |= (uint32_t)(seed << 16);
    }
    else if(sGpioUsrIdList[index] & OPEN_HAL_STAT_MUSK)
    {
        sGpioUsrIdList[index] &= ~(OPEN_HAL_STAT_MUSK);
    }
    return sGpioUsrIdList[index];
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
    return 1;
}
/**
  \fn
  \brief    gpio_isr_cb input para active flags
  \return
*/
ATTRIBUTE_ISR_HEAD static void api_gpio_isr_handle(void)
{
    // EPAT_LOG(api_gpio_isr_handle_enter, P_INFO, "api_gpio_isr_handle_enter");
    for(uint8_t port=0; port<GPIO_INSTANCE_NUM; port++) 
    {
        uint16_t actflags = GPIO_saveAndSetIrqMask(port);
        uint16_t gpioFlag = GPIO_getInterruptFlags(port);
        if(gpioFlag & actflags){
            for(uint8_t pin=0; pin<16; pin++) 
            {
                if((gpioFlag & (1 << pin))){
                    if(gpio_isr_cb[port][pin]!=NULL){
                        gpio_isr_cb[port][pin](actflags);
                    }
                }
            }
        }
        GPIO_clearInterruptFlags(port,actflags);
        GPIO_restoreIrqMask(port, actflags);
    }
}

/* ---------------------------------------------------------------------------------------------- */
/**
  \fn          
  \brief     
  \return
*/
int32_t api_gpio_parse(char* str, gpio_config_t *cfg)
{
    if(str == NULL || cfg == NULL){
        EPAT_LOG(api_gpio_parse_0, P_ERROR, "invalid para");
        return -1;
    } 
    uint8_t num = 0, ptr = 0;
    char *item_str[CSV_GPIO_ITEM_MAX];
    char *head = strtok(str, ",");
    while(head != NULL && num<CSV_GPIO_ITEM_MAX)
    {
        item_str[num] = head;
        head = strtok(NULL, ",");
        num ++; 
    }
    if(num < CSV_GPIO_ITEM_MAX){
        EPAT_LOG(api_gpio_parse_1, P_ERROR, "input items %d < %d", num, CSV_GPIO_ITEM_MAX);
        return -1;
    }
    int8_t index = atoi(item_str[ptr++]);   //0:  index
    if(index < EC_GPIO_INDEX_START || index >= EC_GPIO_INDEX_LIMIT) 
    {
        EPAT_LOG(api_gpio_parse_2, P_ERROR, "GPIO%d invalid", index);
        return -2;
    }
    else {
        cfg->port = index/16;
        cfg->pin = index%16;
    }
    int32_t value = atoi(item_str[ptr++]);  //1: locked
    if(value >= 0 && value < 2) 
    {
        cfg->Locked = value;
    }
    value = atoi(item_str[ptr++]);  //2: direction
    if(value >= 0 && value < 2) 
    {
        cfg->Direction = value;
    }
    value = atoi(item_str[ptr++]);  //3: Default value
    if(value >= 0 && value < 6)
    {
        cfg->Default = value;
    }
    value = atoi(item_str[ptr++]);  //4: active
    if(value >= 0 && value < 2)
    {
        cfg->Active = value;
    }
    value = atoi(item_str[ptr++]);
    if(value >= 0 && value < 2) // mode: 包括中断模式
    {
        cfg->mode = value;
    }
    EPAT_LOG(api_gpio_parse, P_INFO, "gpio%d:Locked %d,Direction %d,Default %d,Active %d,mode %d;ptr %d", \
        index, cfg->Locked, cfg->Direction, cfg->Default, cfg->Active, cfg->mode, ptr);
    return index;
}

/* ---------------------------------------------------------------------------------------------- */
/**
  \fn          
  \brief    单项初始化，使用HAL统一的参数格式
  \return
*/
api_ret_t api_gpio_setup(int8_t index, gpio_config_t* para)
{
    api_ret_t ret = OPEN_HAL_INVALID_PARA;
    if(index < 0 || index >= RTE_GPIO_NUM_MAX) 
    {
        return ret;
    }
    int8_t (*pList)[4] = gpioList;  // 方便后续传入指针赋值
    int8_t gpioNum = pList[index][0];
    int8_t control = pList[index][3];
    GpioPinConfig_t pinConfig = {0};
    if(control < 0) // 需要先更新gpioList表的相关状态，否则单独使用该函数会被限制
    {
        if(para != NULL){
            para->Direction = pList[index][1] ;
            para->Default = pList[index][2] ;
            para->Locked = pList[index][3] ;
        }
        ret = OPEN_HAL_DONE;
        EPAT_LOG(api_gpio_setup_1, P_WARNING, "GPIO%d control %d", index, control);
    }
    else    // gpioList中该gpio可设置
    {
        if(para != NULL){
            // 更新数据
            pList[index][0] = index ;
            pList[index][1] = para->Direction ;
            pList[index][2] = para->Default ;
            pList[index][3] = para->Locked ;
        }
        int8_t gpioDir = pList[index][1];
        uint8_t intVal = pList[index][2];
        EPAT_LOG(api_gpio_setup_2, P_INFO, "GPIO%d gpioDir %d,intVal %d", index, gpioDir, intVal);
        pinConfig.pinDirection = gpioDir;
        if(gpioDir == GPIO_DIRECTION_OUTPUT) 
        {
            if(intVal > 0) 
            {
                pinConfig.misc.initOutput = 1U;
            }
            else {
                pinConfig.misc.initOutput = 0U;
            }
        }
        else if(intVal <= GPIO_INTERRUPT_BOTH_EDGE) 
        {
            if(intVal > GPIO_INTERRUPT_DISABLED)
            {
                
            }
            pinConfig.misc.interruptConfig = intVal;
        }
        GPIO_pinConfig(gpioNum/16, gpioNum%16, &pinConfig);
        ret = OPEN_HAL_DONE;
    }
    return ret;
}

/**
  \fn          int8_t *api_gpio_startup(int8_t *pin, int8_t *pad)
  \brief       所有GPIO上电初始化为配置状态  
  \param[in]   pin  指向GPIO配置参数数组的指针
  \param[in]   pad  指向PAD配置参数数组的指针
  \return      返回指向GPIO状态列表的指针
  \details     该函数用于在系统上电时初始化所有GPIO设备。它会遍历所有可用的GPIO索引，
               并根据传入的配置参数进行初始化设置。同时会检查PAD配置，确保GPIO功能正确映射到物理引脚。
*/
int8_t *api_gpio_startup(int8_t *pin, int8_t *pad)
{
    uint8_t num = 0;
    isr_used_cnt = 0;
    int8_t (*pList)[4] = gpioList;  // 内存中的状态管理
    GPIO_driverInit();  // 不开会导致异常RMI
    if(pin != NULL)     // 传入了新的参数
    {
        int8_t (*newList)[4] = (int8_t (*)[4])pin;
        for(num = 0; num < RTE_GPIO_NUM_MAX; num++) // 最多配置所有GPIO: 超出部分不处理
        {
            if(newList[num][0] < 0 || newList[num][0] >= RTE_GPIO_NUM_MAX){
                break;
            }
            // 检查方向和中断值的有效性
            if(newList[num][1] > GPIO_DIRECTION_OUTPUT || newList[num][2] > GPIO_INTERRUPT_BOTH_EDGE){
                break;
            }
            int8_t index = newList[num][0];
            if(pList[index][3] > 0)   // 已锁定不更新
            {
                // EPAT_LOGD("gpio%d locked %d,%d,%d", pList[index][0], pList[index][1], pList[index][2], pList[index][3]);
            }
            else {
                pList[index][0] = newList[num][0] ;
                pList[index][1] = newList[num][1] ;
                pList[index][2] = newList[num][2] ;
                pList[index][3] = newList[num][3] ;
            }
        }
    }
    if(pad != NULL){
        int8_t (*padList)[4] = (int8_t (*)[4])pad;
        uint8_t tbl = sizeof(gpio_PinTable)/sizeof(gpio_PinTable[0]);
        // 用于查询pad mux是否配置为GPIO模式
        for(uint8_t i = 0; i < RTE_PAD_NUM_MAX; i++)
        {
            if(padList[i][0] < EC_PAD_INDEX_START || padList[i][0] > EC_PAD_INDEX_LIMIT){
                break;
            }
            for(uint8_t j = 0; j < tbl; j++)
            {
                if(padList[i][0] == gpio_PinTable[j][0] && padList[i][1] == gpio_PinTable[j][1])
                {
                    uint8_t gpio = gpio_PinTable[j][2] ;
                    pList[gpio][0] = gpio ;
                }
            }
        }
    }
    for(uint8_t i = EC_GPIO_INDEX_START; i < EC_GPIO_INDEX_LIMIT; i++)
    {
        sGpioUsrIdList[i] = OPEN_HAL_STAT_UNUSED;
        if(gpioList[i][0] >= 0)   // 有效状态
        {
            api_gpio_setup(i, NULL) ;   // 不指定参数以默认表进行设置
            if(gpioList[i][1] == GPIO_DIRECTION_INPUT && gpioList[i][2] > GPIO_INTERRUPT_DISABLED)    // 有中断配置
            {
                isr_used_cnt ++;
            }
        }
    }
    if(isr_used_cnt > 0) 
    {
        XIC_SetVector(PXIC1_GPIO_IRQn, api_gpio_isr_handle);
        XIC_EnableIRQ(PXIC1_GPIO_IRQn);
    }
    return (int8_t *)gpioList;
}

// 以上为硬件级接口，实现对底层配置管理
/* ---------------------------------------------------------------------------------------------- */
// 如下为系统级接口，通过usrId统一识别
/**
  \fn          api_ret_t api_gpio_create(uint32_t index, void *cfg, void *out)
  \brief       gpio创建，index 为 gpio0-gpio38 对应编号 0-38
  \param[in]   index  GPIO索引编号 (0-38)
  \param[in]   cfg    GPIO配置参数指针（可为NULL）
  \param[out]  out    输出参数，返回创建的GPIO设备ID
  \return      usrId  GPIO设备ID
  \details     该函数用于创建一个新的GPIO设备实例，分配相关资源并进行初始化。
               如果cfg不为NULL，则会检查GPIO对应PAD的配置情况。
*/
api_ret_t api_gpio_create(uint32_t index, void *cfg, void *out)
{
    if(index < 0 || index >= RTE_GPIO_NUM_MAX) 
    {
        return OPEN_HAL_INVALID_PARA;
    }
    uint32_t usrId = 0;
    int8_t (*pList)[4] = gpioList;
    api_ret_t ret = api_gpio_checkout(index);
    if(ret == OPEN_HAL_DONE)
    {
        if(pList[index][3] > 0)   // 已锁定不更新
        {
            ret = OPEN_HAL_LOCK ;
        }
        else {
            pList[index][0] = index ;
            pList[index][3] = 0 ;
            usrId = gpio_set_idle(index);
            ret = OPEN_HAL_DONE ;
        }

        EPAT_LOG(api_gpio_create_1, P_INFO, "gpio%d,usrId 0x%X,ret %d", index, usrId, ret);
    }
    else {
        EPAT_LOG(api_gpio_create_fail, P_ERROR, "create gpio%d,usrId %d,fail ret %d", index, usrId, ret);
    }
    if(out != NULL)
    {
        *(uint32_t *)out = usrId;
    }
    return ret;
}

/**
  \fn          api_ret_t api_gpio_delete(uint32_t usrId)
  \brief       删除GPIO设备实例
  \param[in]   usrId  GPIO设备ID
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于删除指定的GPIO设备实例，释放相关资源。
               只有在GPIO处于空闲状态时才能被删除。
*/
api_ret_t api_gpio_delete(uint32_t usrId)
{
    int8_t (*pList)[4] = gpioList;
    api_ret_t ret = api_gpio_query(usrId);
    uint32_t index = usrId_to_gpio(usrId);
    if(ret == OPEN_HAL_IDLE)
    {
        if(gpio_set_free(index, NULL))  // 只改内存GPIO状态不改实际的GPIO配置
        {
            
        }
        pList[index][3] = 0 ;
        ret = OPEN_HAL_DONE ;
    }
    // SYSLOG_INFO("[%d][0x%08X]-%d\r\n",index,usrId,ret);
    return ret;
}

/* ---------------------------------------------------------------------------------------------- */
/**
  \fn          api_ret_t api_gpio_open(uint32_t usrId, void *cfg, size_t timeout)
  \brief       打开GPIO设备
  \param[in]   usrId    GPIO设备ID
  \param[in]   cfg      GPIO配置参数指针（可为NULL）
  \param[in]   timeout  超时时间（暂未使用）
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于打开GPIO设备并根据配置参数进行设置。
               只有在GPIO处于空闲状态时才能被打开。
*/
api_ret_t api_gpio_open(uint32_t usrId, void *cfg, size_t timeout)
{
    (void)timeout;
    int8_t (*pList)[4] = gpioList;
    api_ret_t ret = api_gpio_query(usrId);
    uint32_t index = usrId_to_gpio(usrId);
    if(ret == OPEN_HAL_IDLE)
    {
        if(cfg != NULL)
        {
            GpioPinConfig_t *pinConfig = (GpioPinConfig_t *)cfg;
            if(pinConfig->pinDirection<2U && pinConfig->misc.interruptConfig<6U)
            {
                pList[index][1] = pinConfig->pinDirection ;
                pList[index][2] = pinConfig->misc.interruptConfig ;
                GPIO_pinConfig(index/16, index%16, pinConfig);
            }
        }
        gpio_set_used(usrId);
        ret = OPEN_HAL_DONE;
    }
    return ret;
}

/**
  \fn          api_ret_t api_gpio_close(uint32_t usrId)
  \brief       关闭GPIO设备
  \param[in]   usrId  GPIO设备ID
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于关闭指定的GPIO设备，将其状态设置为空闲。
               只有在GPIO处于使用中状态时才能被关闭。
*/
api_ret_t api_gpio_close(uint32_t usrId)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_gpio_query(usrId);
    uint32_t index = usrId_to_gpio(usrId);
    if(ret == OPEN_HAL_USED)
    {  
        gpio_set_idle(index);
        ret = OPEN_HAL_DONE;
    }
    // SYSLOG_INFO("[%d][0x%08X]-%d\r\n",index,usrId,ret);
    return ret;
}

/* ---------------------------------------------------------------------------------------------- */
/**
  \fn          api_ret_t api_gpio_ioctl(uint32_t usrId,api_gpio_ioctl_t type, void *para)
  \brief       GPIO设备控制接口
  \param[in]   usrId  GPIO设备ID
  \param[in]   type   控制类型，参考api_gpio_ioctl_t枚举
  \param[in]   para   控制参数指针
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于对GPIO设备进行各种控制操作，如设置方向、中断回调等。
               只有在GPIO处于使用中状态时才能进行控制操作。
*/
api_ret_t api_gpio_ioctl(uint32_t usrId,api_gpio_ioctl_t type, void *para)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_gpio_query(usrId);
    uint32_t index = usrId_to_gpio(usrId);
    if(ret == OPEN_HAL_USED)
    {
        uint8_t val = 0;
        if(para!=NULL) val = *(uint8_t *)para;
        gpio_config_t newConfig = {0};
        switch (type) 
        {
            case OPEN_GPIO_IOCTL_ISR_CB:
                gpioIsrSet(index, para);
                if(isr_used_cnt == 0 && para != NULL) 
                {
                    XIC_SetVector(PXIC1_GPIO_IRQn, api_gpio_isr_handle);
                    XIC_EnableIRQ(PXIC1_GPIO_IRQn);
                    isr_used_cnt ++;
                }
                break;
            case OPEN_GPIO_IOCTL_DIR_OUT:
                newConfig.Direction = GPIO_DIRECTION_OUTPUT ;
                newConfig.Default = 1U ;
                newConfig.Locked = 0 ;
                ret = api_gpio_setup(index, &newConfig);
                break;

            case OPEN_GPIO_IOCTL_OUT_ACT:
                if(val < 2U){
                    if(val) sGpioRuntimeConfig[index] |= 0x01;
                    else sGpioRuntimeConfig[index] &= ~(0x01);
                }
                break;

            case OPEN_GPIO_IOCTL_DIR_IN:
                newConfig.Direction = GPIO_DIRECTION_INPUT ;
                newConfig.Default = GPIO_INTERRUPT_DISABLED ;
                newConfig.Locked = 0U ;
                ret = api_gpio_setup(index, &newConfig);
                break;
            case OPEN_GPIO_IOCTL_LOCKED:
                if(val) sGpioRuntimeConfig[index] |= 0x40;
                else sGpioRuntimeConfig[index] &= ~(0x40);
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
  \fn          api_ret_t api_gpio_pmctl(uint32_t usrId,open_hal_pm_t *cfg, size_t count)
  \brief       对GPIO设备功耗和模式进行配置
  \param[in]   usrId  GPIO设备ID
  \param[in]   cfg    功耗配置参数指针
  \param[in]   count  参数数量（暂未使用）
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于控制GPIO设备的功耗模式。
               只有在GPIO处于使用中状态时才能进行功耗控制。
*/
api_ret_t api_gpio_pmctl(uint32_t usrId,open_hal_pm_t *cfg, size_t count)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_gpio_query(usrId);
    // uint32_t index = usrId_to_gpio(usrId);
    // ASSERT(index < EC_GPIO_INDEX_LIMIT);
    (void)count;
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
    return OPEN_HAL_DONE;
}

/**
  \fn          api_ret_t api_gpio_write(uint32_t usrId, void* buf, size_t count)
  \brief       对GPIO设备进行写操作
  \param[in]   usrId   GPIO设备ID
  \param[in]   buf     要写入的数据缓冲区指针
  \param[in]   count   要写入的数据大小（字节数）
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于向GPIO输出引脚写入电平值（0或1）。
               只有在GPIO处于使用中状态时才能进行写操作。
*/
api_ret_t api_gpio_write(uint32_t usrId, void* buf, size_t count)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_gpio_query(usrId);
    uint32_t index = usrId_to_gpio(usrId);
    // ASSERT(index < EC_GPIO_INDEX_LIMIT);
    (void)count;
    if(ret == OPEN_HAL_USED)
    {
        uint32_t port = index/16;
        uint16_t pin = index%16;
        uint16_t val = 0;
        if(buf != NULL)
        {
            if(sGpioRuntimeConfig[index] & 0x01)
            {
                if(*(uint16_t *) buf) val = 0U ;
                else val = 1U ;
            }
            else {
                if(*(uint16_t *) buf) val = 1U ;
                else val = 0U ;
            }
            GPIO_pinWrite(port, 1U << pin, val << pin);
        }
        ret = OPEN_HAL_DONE;
    }
    return ret;
}

/**
  \fn          api_ret_t api_gpio_read(uint32_t usrId, void* buf, size_t count)
  \brief       对GPIO设备进行读操作
  \param[in]   usrId   GPIO设备ID
  \param[out]  buf     读取数据的缓冲区指针
  \param[in]   count   要读取的数据大小（字节数）
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于从GPIO输入引脚读取电平值。
               只有在GPIO处于使用中状态时才能进行读操作。
*/
api_ret_t api_gpio_read(uint32_t usrId, void* buf, size_t count)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_gpio_query(usrId);
    uint32_t index = usrId_to_gpio(usrId);
    (void)count;
    if(ret == OPEN_HAL_USED)
    {
        uint32_t port = index/16;
        uint16_t pin = index%16;
        uint32_t val = GPIO_pinRead(port,pin);
        if(buf != NULL)
        {
            if(sGpioRuntimeConfig[index] & 0x01)
            {
                if(val) *(uint8_t *)buf = 0U ;
                else *(uint8_t *)buf = 1U ;
            }
            else {
                if(val) *(uint8_t *)buf = 1U ;
                else *(uint8_t *)buf = 0U ;
            }
        }
        ret = OPEN_HAL_DONE;
    }
    return ret;
}

/**
  \fn          int api_test_gpio(void)
  \brief       GPIO设备测试接口
  \return      测试结果
  \details     该函数用于测试GPIO设备的基本功能，包括创建、打开、配置、读写等操作。
               测试会遍历所有GPIO引脚，对每个引脚执行完整的创建、打开、配置、读写、功耗控制、关闭和删除流程。
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
        api_gpio_create(i,&pinConfig, &test_gpio_id);
        EC_API_CHECK(api_gpio_open(test_gpio_id,NULL,0));
        uint8_t level = 0;
        EC_API_CHECK(api_gpio_ioctl(test_gpio_id,OPEN_GPIO_IOCTL_DIR_OUT,&level));
        uint8_t active = 0;
        EC_API_CHECK(api_gpio_ioctl(test_gpio_id,OPEN_GPIO_IOCTL_OUT_ACT,&active));
        level = 1;
        EC_API_CHECK(api_gpio_write(test_gpio_id,&level,1));
        EC_API_CHECK(api_gpio_read(test_gpio_id,&level,1));
        open_hal_pm_t cfg = {
            .mode = PM_LOWPOW,
            .runtime = RUNTIME_SUSPEND
        };
        EC_API_CHECK(api_gpio_pmctl(test_gpio_id,&cfg,sizeof(cfg)));
        EC_API_CHECK(api_gpio_close(test_gpio_id));
        EC_API_CHECK(api_gpio_delete(test_gpio_id));
    }
    printf("-----gpio test end-----\r\n");

    return 0;
}
