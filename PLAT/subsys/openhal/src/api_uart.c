/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_uart.c
 * Description:  openhal uart entry source file
 * History:      Rev1.0   2024-02-23
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
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
#include "clock.h"
#ifdef EPAT_HAL_DEBUG
#define EPAT_LOG(subId, debugLevel, format, ...)  \
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, subId, debugLevel, format, ##__VA_ARGS__)
#else
#define EPAT_LOG(subId, debugLevel, format, ...) 
#endif

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
#endif
#if (RTE_UART3)
extern ARM_DRIVER_USART Driver_USART3;
#endif
static ARM_DRIVER_USART *uartDrv[EC_UART_INDEX_LIMIT] = {
    &Driver_USART0, 
    &Driver_USART1,
#if (RTE_UART2)
    &Driver_USART2,
#if (RTE_UART3)
    &Driver_USART3,
#endif
#endif
};

/* ---------------------------------------------------------------------------------------------- */
const int8_t uart_RXD_PinTable[][3] = {
    {21, PAD_MUX_ALT2, 2},  // GPIO6    UART2_RXD
    {25, PAD_MUX_ALT3, 2},  // GPIO10   UART2_RXD
    {27, PAD_MUX_ALT3, 2},  // GPIO12   UART2_RXD
    {29, PAD_MUX_ALT3, 3},  // GPIO14   UART3_RXD
    {31, PAD_MUX_ALT1, 0},  // GPIO16   UART0_RXD
    {33, PAD_MUX_ALT1, 1},  // GPIO18   UART1_RXD
    {40, PAD_MUX_ALT3, 3},  // GPIO34   UART3_RXD
};
const int8_t uart_TXD_PinTable[][3] = {
    {22, PAD_MUX_ALT2, 2},  // GPIO7    UART2_TXD
    {26, PAD_MUX_ALT3, 2},  // GPIO11   UART2_TXD
    {28, PAD_MUX_ALT3, 2},  // GPIO13   UART2_TXD
    {30, PAD_MUX_ALT3, 3},  // GPIO15   UART3_TXD
    {32, PAD_MUX_ALT1, 0},  // GPIO17   UART0_TXD
    {34, PAD_MUX_ALT1, 1},  // GPIO19   UART1_TXD
    {41, PAD_MUX_ALT3, 3},  // GPIO35   UART3_TXD
};

/*
 * when uart input clock is 26000000, the supported baudrate is:
 */
const uint32_t uart_baudTable[] = {
    300,600,1200,2400,
    4800,9600,14400,19200,
    28800,38400,56000,57600,
    115200,230400,460800,921600,
    1000000,1500000,2000000,3000000
};

/* ----------------------------------------- 运行状态表 ------------------------------------------ */
AP_PLAT_COMMON_DATA static int8_t uartList[][4] = {
    {-1, 5, UNILOG_MODE, -1},       /* UART0: baudTable[5]/UNILOG_MODE */
    {-1, 5, IRQ_MODE, -1},          /* UART1: baudTable[5]/IRQ */
    {-1, 5, DMA_MODE, -1},          /* UART2: baudTable[5]/DMA_MODE */
    {-1, 5, DMA_MODE, -1}           /* UART3: baudTable[5]/DMA_MODE */
};

/* ---------------------------------------------------------------------------------------------- */
/**
  \fn          
  \brief       检查UART接口的引脚配置是否正确
  \param[in]   rxd   UART接收数据引脚编号
  \param[in]   txd   UART发送数据引脚编号
  \param[in]   rts   UART请求发送引脚编号（暂未使用）
  \param[in]   cts   UART清除发送引脚编号（暂未使用）
  \return      检查结果，OPEN_HAL_DONE表示成功，其他值表示失败
  \details     该函数用于检查指定的UART引脚配置是否正确。它会验证RXD和TXD引脚是否
               在预定义的引脚表中，并且对应的PAD是否已配置为UART功能。
               只有当RXD和TXD引脚都属于同一个UART控制器时，检查才会成功。
*/
api_ret_t api_uart_checkout(int8_t index, int8_t rxd, int8_t txd, int8_t rts, int8_t cts)
{
    api_ret_t ret = OPEN_HAL_INVALID_PARA;
    int8_t select = -1;
    int8_t mux = -1;
    if(rxd > 0){
        for (int8_t i = 0; i < sizeof(uart_RXD_PinTable)/sizeof(uart_RXD_PinTable[0]); i++) {
            if (uart_RXD_PinTable[i][0] == rxd) {
                mux =uart_RXD_PinTable[i][1] ;
                if (check_pad_mux(rxd, mux) == OPEN_HAL_DONE) {
                    select = uart_RXD_PinTable[i][2];
                    if(index != select){
                        ret = OPEN_HAL_NONE ;
                    }
                    else {
                        ret = OPEN_HAL_DONE ;
                    }
                    EPAT_LOG(uart_checkout_rxd, P_INFO, "index %d select UART%d with rxd%d-mux%d", index, select, rxd, mux);
                }
            }
        }
    }
    if(txd > 0 && ret == OPEN_HAL_DONE)
    {
        for (int8_t i = 0; i < sizeof(uart_TXD_PinTable)/sizeof(uart_TXD_PinTable[0]); i++) {
            if (uart_TXD_PinTable[i][0] == txd) {
                mux = uart_TXD_PinTable[i][1] ;
                if (check_pad_mux(txd, mux) == OPEN_HAL_DONE && select == uart_TXD_PinTable[i][2]) 
                {
                    if(index != select){
                        ret = OPEN_HAL_NONE ;
                    }
                    else {
                        ret = OPEN_HAL_DONE ;
                    }
                    EPAT_LOG(uart_checkout_txd, P_INFO, "index %d select UART%d with txd%d-mux%d", index, select, txd, mux);
                }
                else {
                    ret = OPEN_HAL_NONE ;
                }
            }
        }
    }
    EPAT_LOG(uart_checkout_end, P_DEBUG, "index %d: rxd%d,txd%d,ret %d", index, rxd, txd, ret);
    return ret;
}

/**
  \fn          int32_t api_uart_parse(char* str, uart_config_t *cfg)
  \brief       解析UART配置字符串
  \param[in]   str   配置字符串
  \param[out]  cfg   解析后的配置参数结构体指针
  \return      UART索引编号
  \details     该函数用于解析CSV格式的UART配置字符串，并填充到配置结构体中。
               配置字符串应包含11个字段：
               0: 索引
               1: rxd引脚
               2: txd引脚
               3: rts引脚
               4: cts引脚
               5: 波特率表
               6: 工作模式
               7: 数据位 (8/9/-/-/-/5/6/7)
               8: 校验位 (0=None, 1=Odd, 2=Even)
               9: 停止位 (0=1 bit, 1=2 bits)
               10: 流控位 (0=None, 1=RTS, 2=CTS, 3=RTS/CTS)
*/
int32_t api_uart_parse(char* str, uart_config_t *cfg)
{
    if(str == NULL || cfg == NULL){
        EPAT_LOG(api_uart_parse_0, P_ERROR, "invalid para");
        return -1;
    } 
    uint8_t ptr = 0;
    char *item_str[CSV_CFG_UART_ITEMS]; // 使用宏定义值
    char *head = strtok(str, ",");
    while(head != NULL && ptr < CSV_CFG_UART_ITEMS)
    {
        item_str[ptr] = head;
        head = strtok(NULL, ",");
        ptr ++; 
    }
    if(ptr < CSV_CFG_UART_ITEMS){
        EPAT_LOG(api_uart_parse_1, P_ERROR, "input items %d < %d", ptr, CSV_CFG_UART_ITEMS);
        return -1;
    }
    ptr = 0;
    int8_t index = atoi(item_str[ptr++]);
    if(index < EC_UART_INDEX_START || index > EC_UART_INDEX_LIMIT) 
    {
        EPAT_LOG(api_uart_parse_2, P_ERROR, "uart%d invalid", index);
        return -2;
    }
    // 如下为有效参数提取
    int32_t value = atoi(item_str[ptr++]);  //1: rxd pad
    if(value >= 21 && value <= 41)  // 根据实际pad值精确限定
    {
        cfg->rxd = value;
    }
    else {
        cfg->rxd = 0;
        EPAT_LOG(csv_uart_item_2, P_ERROR, "rxd pad%d invalid %d:%s", value, ptr, item_str[ptr-1]);
    }
    value = atoi(item_str[ptr++]);  //2: txd pad
    if(value >= 21 && value <= 41)  // 根据实际pad值精确限定
    {
        cfg->txd = value;
    }
    else {
        cfg->txd = 0;
        EPAT_LOG(csv_uart_item_3, P_ERROR, "txd pad%d invalid %d:%s", value, ptr, item_str[ptr-1]);
    }
    value = atoi(item_str[ptr++]);  //3: rts
    if(value >= EC_PAD_INDEX_START && value < EC_PAD_INDEX_LIMIT)
    {
        cfg->rts = value;
    }
    else {
        cfg->rts = 0;
    }
    value = atoi(item_str[ptr++]);  //4: cts
    if(value >= EC_PAD_INDEX_START && value < EC_PAD_INDEX_LIMIT)
    {
        cfg->cts = value;
    }
    else {
        cfg->cts = 0;
    }
    value = atoi(item_str[ptr++]);  //5: baud
    if(value >= 300 && value <= 3000000)
    {
        int8_t lvl = 0;
        while(uart_baudTable[lvl++] < value)
        {
        }
        cfg->baud = lvl-1;
        // EPAT_LOG(api_uart_parse_baud, "%d.item baud %d -> level[%d]=%d", ptr, value, cfg->baud, uart_baudTable[cfg->baud]);
    }
    else {
        cfg->baud = 12; // 115200
        EPAT_LOG(csv_uart_item_6, P_ERROR, "baud %d invalid %d:%s", value, ptr, item_str[ptr-1]);
    }
    value = atoi(item_str[ptr++]);  //6: mode
    if(value >= 0 && value < 16)
    {
        cfg->mode = value;
    }
    else {
        cfg->mode = 0x01UL;  // 默认 ARM_USART_MODE_ASYNCHRONOUS
        EPAT_LOG(csv_uart_item_7, P_ERROR, "mode %d invalid %d:%s, using default mode", value, ptr, item_str[ptr-1]);
    }
    value = atoi(item_str[ptr++]);  //7: data bits
    if(value >= 8 && value <= 9)
    {
        cfg->bits = value - 8;
    }
    else if(value >= 5 && value <= 7) {
        cfg->bits = value;
    }
    else {
        cfg->bits = 0;  // 默认8位数据
        EPAT_LOG(csv_uart_item_8, P_ERROR, "data bits %d invalid %d:%s, using default 8 bits", value, ptr, item_str[ptr-1]);
    }
    
    value = atoi(item_str[ptr++]);  //8: parity (0=None, 1=Odd, 2=Even)
    if(value >= 0 && value <= 2)
    {
        cfg->parity = value;
    }
    else {
        cfg->parity = 0;  // 默认无校验
        EPAT_LOG(csv_uart_item_9, P_ERROR, "parity %d invalid %d:%s, using default None", value, ptr, item_str[ptr-1]);
    }
    
    value = atoi(item_str[ptr++]);  //9: stop bits (0=1 bit, 1=2 bits)
    if(value >= 0 && value <= 1)
    {
        cfg->stop = value;
    }
    else {
        cfg->stop = 0;  // 默认1位停止位
        EPAT_LOG(csv_uart_item_10, P_ERROR, "stop bits %d invalid %d:%s, using default 1 bit", value, ptr, item_str[ptr-1]);
    }
    
    value = atoi(item_str[ptr++]);  //10: flow control (0=None, 1=RTS, 2=CTS, 3=RTS/CTS)
    if(value >= 0 && value <= 3)
    {
        cfg->flow = value;
    }
    else {
        cfg->flow = 0;  // 默认无流控
        EPAT_LOG(csv_uart_item_11, P_ERROR, "flow control %d invalid %d:%s, using default None", value, ptr, item_str[ptr-1]);
    }
    // EPAT_LOG(api_uart_parse_end, P_INFO, "uart%d:cfg[0] 0x%x,cfg[1] 0x%x", index, *((uint32_t*)cfg), *((uint32_t*)cfg + 1));
    return index;
}


/**
  \fn          static uint32_t usrId_to_index(uint32_t usrId)
  \brief       根据用户ID获取UART索引
  \param[in]   usrId  用户ID
  \return      UART索引编号
  \details     该函数用于从用户ID中提取UART索引编号。
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
  \fn          api_ret_t api_uart_query(uint32_t usrId)
  \brief       查询UART设备状态
  \param[in]   usrId  UART设备ID
  \return      UART设备当前状态
  \details     该函数用于查询指定UART设备的当前状态（空闲、使用中等）。
               返回值可以是OPEN_HAL_FREE（未分配）、OPEN_HAL_IDLE（空闲）、
               OPEN_HAL_USED（使用中）或OPEN_HAL_NONE（无效）。
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
  \fn          static void api_uart2_isr_handle(uint32_t event)
  \brief       UART2中断处理函数
  \param[in]   event  中断事件类型
  \return      无
  \details     该函数用于处理UART2的中断事件，如果已注册了中断回调函数，则调用该回调函数。
*/
static void api_uart2_isr_handle(uint32_t event)
{
    if(sUartIsrFunc[2] != NULL){
        sUartIsrFunc[2](event);
    }
}
/**
  \fn          static void api_uart3_isr_handle(uint32_t event)
  \brief       UART3中断处理函数
  \param[in]   event  中断事件类型
  \return      无
  \details     该函数用于处理UART3的中断事件，如果已注册了中断回调函数，则调用该回调函数。
*/
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
    uint16_t contrbit = sUartRuntimeList[index] >> 16;
    if(uartDrv[index]!=NULL){
        if(contrbit != control){
            uartDrv[index]->Control(control,baudrate);
            sUartRuntimeList[index] &= 0x0000FFFF;
            sUartRuntimeList[index] |= ((uint32_t)control << 16);
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
static uint8_t uart_baudrate_set(uint32_t index, uint32_t baudrate)
{
    uint8_t ret = 0;
    // uint32_t baudrate = 100 * (sUartRuntimeList[index] & 0x0000FFFF);
    // uint16_t contrbit = sUartRuntimeList[index] >> 16;
    if(uartDrv[index] != NULL){
        uartDrv[index]->Control(0x01, baudrate);
        ret = 1;
    }
    SYSLOG_INFO("[%d][x%08X],%d,%d\r\n",index,uartDrv[index],baudrate,ret); 
    return ret;
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
                // uartDrv[index]->Control(contrbit,baudrate);
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
/* ---------------------------------------------------------------------------------------------- */
/**
  \fn          void * api_uart_setup(int8_t index, uart_config_t* para)
  \brief       单项初始化，使用HAL统一的参数格式
  \param[in]   index  UART索引编号
  \param[in]   para   UART配置参数指针
  \return      执行结果
  \details     该函数用于单项初始化，使用HAL统一的参数格式，会执行硬件配置。
*/
#define UART_IRQn_ID    {41, 40, 39, 38}
api_ret_t api_uart_setup(int8_t index, uart_config_t* para)
{
    api_ret_t ret = OPEN_HAL_INVALID_PARA;
    if(index >= EC_UART_INDEX_START && index < EC_UART_INDEX_LIMIT)
    {
        ClockId_e fclk = INVALID_CLK;
        ClockSelect_e clk = MAKE_CLOCK_SEL_VALUE(GET_INDEX_FROM_CLOCK_ID(fclk), 0, CLK_MF_GATED);
        ARM_DRIVER_USART *pUart = NULL;
        ARM_USART_SignalEvent_t uart_isr_cb = NULL;
        // setup单纯进行配置，不再进行校验
        // ret = api_uart_checkout(para->rxd, para->txd, para->rts, para->cts);
        // if(ret == OPEN_HAL_DONE)
        {
            IRQn_Type irqSel[] = UART_IRQn_ID;
            XIC_DisableIRQ(irqSel[index]);
            XIC_ClearPendingIRQ(irqSel[index]);
            if(index == 2){
                #if (RTE_UART2 == 1)
                extern ARM_DRIVER_USART Driver_USART2;
                pUart = &Driver_USART2;
                pUart->Uninitialize();
                fclk = FCLK_UART2;
                clk = FCLK_UART2_SEL_26M;
                GPR_clockDisable(fclk);
                // uart_isr_cb = port_uart2_isr;
                #endif
            }
            #if (RTE_UART3 == 1)
            else if(index == 3)
            {
                extern ARM_DRIVER_USART Driver_USART3;
                pUart = &Driver_USART3;
                pUart->Uninitialize();
                fclk = FCLK_UART3;
                clk = FCLK_UART3_SEL_26M;
                GPR_clockDisable(fclk);
                // uart_isr_cb = port_uart3_isr;
            }
            #endif
            if(pUart)
            {
                if(para)
                {
                    uint32_t baudrate = uart_baudTable[para->baud];
                    pUart->Initialize(uart_isr_cb);
                    if(baudrate > 9600){
                        pUart->PowerControl(ARM_POWER_FULL);
                    }
                    else {
                        pUart->PowerControl(ARM_POWER_LOW);
                    }
                    uint32_t contrbit = (para->mode << ARM_USART_CONTROL_Pos);
                    contrbit |= (para->bits << ARM_USART_DATA_BITS_Pos);
                    contrbit |= (para->parity << ARM_USART_PARITY_Pos);
                    contrbit |= (para->stop << ARM_USART_STOP_BITS_Pos);
                    contrbit |= (para->flow << ARM_USART_FLOW_CONTROL_Pos);
                    pUart->Control(contrbit, baudrate);
                    GPR_setClockSrc(fclk, clk);
                    GPR_clockEnable(fclk);
                }
                else{
                    // deinit其他操作
                }
                ret = OPEN_HAL_DONE;
                EPAT_LOG(api_uart_setup_done, P_INFO, "uart%d:baud %d,mode %d,bits %d,parity %d,stop %d,flow %d", \
                    index, para->baud, para->mode, para->bits, para->parity, para->stop, para->flow);
            }
            else{
                ret = OPEN_HAL_NONE;
            }
        }
    }
    return ret;
}

/**
  \fn          int8_t* api_uart_startup(void* para, int8_t *pad)
  \brief       加载UART默认配置并初始化
  \param[in]   para  UART配置参数数组指针
  \param[in]   pad   PAD配置参数数组指针
  \return      指向UART状态列表的指针
  \details     该函数用于加载UART默认配置并初始化所有可用的UART设备。
               它会初始化状态列表，将所有UART设备设置为空闲状态，以便后续使用。
*/
int8_t* api_uart_startup(void* para, int8_t *pad)
{
    for(uint8_t i=EC_UART_INDEX_START;i<EC_UART_INDEX_LIMIT;i++)
    {
        sUartUsrIdList[i] = OPEN_HAL_STAT_UNUSED;
        sUartUsrIdSeed[i] = 0;
        sUartIsrFunc[i] = NULL;
        sUartRuntimeList[i] = sUartStartupList[i];
        if(uartDrv[i] != NULL)
        {
            // uartDrv[i]->PowerControl(ARM_POWER_OFF);
            // uartDrv[i]->Uninitialize();
            // uint32_t baudrate = 100 * (sUartRuntimeList[i] & 0x0000FFFF);
            // uint16_t contrbit = sUartRuntimeList[i] >> 16;
            // if(baudrate> 1200){
            //     if(i==2) uartDrv[i]->Initialize(api_uart2_isr_handle);
            //     else if(i==3) uartDrv[i]->Initialize(api_uart3_isr_handle);
            //     else uartDrv[i]->Initialize(NULL);
            //     uartDrv[i]->PowerControl(ARM_POWER_FULL);
            //     // uartDrv[i]->Control(contrbit,baudrate);
            // }
            // SYSLOG_INFO("[x%08X]x%04X-%d\r\n",uartDrv[i],contrbit,baudrate);
        }
    }
    return (int8_t *)uartList;
}

/* ---------------------------------------------------------------------------------------------- */
/**
  \fn          api_ret_t api_uart_create(int8_t index, uart_config_t *cfg, void *out)
  \brief       创建UART设备实例
  \param[in]   index  UART索引编号
  \param[in]   cfg    UART配置参数指针（可为NULL）
  \param[out]  out    输出参数，返回创建的UART设备ID
  \return      执行结果
  \details     该函数用于创建UART设备实例，分配资源并检查依赖条件。
               如果配置参数不为NULL，则会检查引脚配置是否正确。
               成功创建后，会返回一个用户ID，用于后续操作。
*/
api_ret_t api_uart_create(int8_t index, uart_config_t *cfg, void *out)
{
    api_ret_t ret = OPEN_HAL_INVALID_PARA;
    if(index < EC_UART_INDEX_START || index >= EC_UART_INDEX_LIMIT) 
    {
        EPAT_LOG(api_uart_create_0, P_ERROR, "uart%d invalid", index);
        return OPEN_HAL_INVALID_PARA;
    }
    ClockId_e uartClocks[] = UART_CLOCK_VECTOR;
    ClockId_e pclk = uartClocks[index*2];
    ClockId_e fclk = uartClocks[index*2+1];
    uint32_t fclk_val = GPR_getClockFreq(fclk);  // 查询时钟是否配置
    // ret = api_uart_checkout(para->rxd, para->txd, para->rts, para->cts);
    if(GPR_clockEnableCheck(pclk) && fclk_val >= 26000000U)  // 不检查 GPR_clockEnableCheck(fclk)
    {

    }
    else 
    {
        EPAT_LOG(api_uart_create_3, P_WARNING, "uart%d check clk fail %d-%d", index, GPR_clockEnableCheck(pclk),fclk_val);
        ret = OPEN_HAL_INVALID_PARA;
    }
    if(cfg != NULL) // 查询是否具备相应依赖条件
    {
        ret = api_uart_checkout(index, cfg->rxd, cfg->txd, cfg->rts, cfg->cts);   // 如果配置的pad不匹配不创建
        if(ret == OPEN_HAL_DONE){
            EPAT_LOG(api_uart_create_1, P_INFO, "checkout uart%d fclk %d", index, fclk_val);
        }
        else {
            EPAT_LOG(api_uart_create_2, P_WARNING, "uart%d check fail ret %d", index, ret);
            ret = OPEN_HAL_INVALID_PARA;
        }
        // sPwmRuntimeList[index] = 0;
    }
    uint32_t usrId = uart_set_idle(index);
    if(out != NULL)
    {
        *(uint32_t *)out = usrId;
    }
    if(usrId)
    {
        ret = OPEN_HAL_DONE;
    }
    return ret;
}

/**
  \fn          api_ret_t api_uart_delete(uint32_t usrId)
  \brief       删除UART设备实例
  \param[in]   usrId  UART设备ID
  \return      执行结果
  \details     该函数用于删除UART设备实例，释放相关资源。
               只有在设备处于空闲状态时才能删除。
*/
api_ret_t api_uart_delete(uint32_t usrId)
{
    api_ret_t ret = api_uart_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_IDLE)
    { 
        if(uart_set_free(index)){
            ret = OPEN_HAL_DONE;
        }
    }
    // SYSLOG_INFO("[x%08X][x%08X],%d\r\n",usrId,uartDrv[index],ret);
    return ret;
}

/* ---------------------------------------------------------------------------------------------- */
/**
  \fn          api_ret_t api_uart_open(uint32_t usrId,void *cfg,size_t timeout)
  \brief       打开和配置UART设备
  \param[in]   usrId     UART设备ID
  \param[in]   cfg       UART配置参数指针（可为NULL）
  \param[in]   timeout   超时时间
  \return      执行结果
  \details     该函数是系统级接口，用于打开和配置UART设备。
               如果在超时时间内，申请的usrId是空闲状态，则获取成功，并以传入的cfg参数初始化;
               如果在超时时间内，申请的usrId是关闭状态，则获取成功，UART的配置参数以cfg传出;
               超时未获取成功，通过open_hal_ack_t枚举返回状态;
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
            api_uart_setup(index, (uart_config_t*)cfg);
        } 
        uart_set_used(usrId);
        ret = OPEN_HAL_DONE;
    }
    SYSLOG_INFO("uart%d[0x%08X]:0x%X\r\n",index,usrId,ret);
    // printf("\r\ni2c%d[0x%08X]open:0x%X",index,usrId,ret); 
    return ret;
}

/**
  \fn          api_ret_t api_uart_close(uint32_t usrId)
  \brief       关闭UART设备
  \param[in]   usrId  UART设备ID
  \return      执行结果
  \details     该函数用于关闭指定的UART设备，将其状态设置为空闲。
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

/* ---------------------------------------------------------------------------------------------- */
/**
  \fn          api_ret_t api_uart_ioctl(uint32_t usrId, api_uart_ioctl_t type, void *para)
  \brief       UART设备控制接口
  \param[in]   usrId  UART设备ID
  \param[in]   type   控制类型，参考api_uart_ioctl_t枚举
  \param[in]   para   控制参数指针
  \return      执行结果
  \details     该函数是UART设备控制接口，用于配置设备的各种参数。
               支持设置中断回调函数、波特率、获取接收计数和发送计数等操作。
*/
api_ret_t api_uart_ioctl(uint32_t usrId, api_uart_ioctl_t type, void *para)
{
    api_ret_t ret = api_uart_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_USED || ret == OPEN_HAL_IDLE)
    { 
        switch (type) {
            case OPEN_UART_IOCTL_ISR_CB:
                sUartIsrFunc[index] = (IsrFunc)para;
                ret = OPEN_HAL_DONE;
                break;
            case OPEN_UART_IOCTL_BAUDRATE:
                uart_baudrate_set(index,*(uint32_t *)para);
                ret = OPEN_HAL_DONE;
                break;
            case OPEN_UART_IOCTL_CONTROL:
                uart_control_set(index,*(uint16_t *)para);
                ret = OPEN_HAL_DONE;
                break;
            case GET_RX_COUNT:
                if (uartDrv[index] != NULL)
                {
                    uint32_t count = uartDrv[index]->GetRxCount();
                    memcpy(para, &count, sizeof(uartDrv[index]->GetRxCount()));
                }
                ret = OPEN_HAL_DONE;
                break;
            default:
                ret = OPEN_HAL_INVALID_PARA;
                break;
        }
    }
    // SYSLOG_INFO("[x%08X][x%08X],%d,%d\r\n",usrId,uartDrv[index],type,ret);
    return ret;
}
/**
  \fn          
  \brief    
  \return
*/
api_ret_t api_uart_pmctl(uint32_t usrId,open_hal_pm_t *cfg, size_t count)
{
    api_ret_t ret = api_uart_query(usrId);
    // uint32_t index = usrId_to_index(usrId);
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
    // SYSLOG_INFO("[x%08X][x%08X],%d,%d\r\n",usrId,uartDrv[index],mode,ret);
    return ret;
}
/**
  \fn          api_ret_t api_uart_write(uint32_t usrId, void* buf, size_t count)
  \brief       向UART设备写入数据
  \param[in]   usrId   UART设备ID
  \param[in]   buf     要写入的数据缓冲区指针
  \param[in]   count   要写入的数据大小（字节数）
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于向UART设备写入指定数量的数据。
               只有在UART处于使用中状态时才能进行写操作。
               目前使用轮询模式发送数据，后续可考虑支持中断或DMA模式。
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
  \fn          api_ret_t api_uart_read(uint32_t usrId, void* buf, size_t count)
  \brief       从UART设备读取数据
  \param[in]   usrId   UART设备ID
  \param[out]  buf     读取数据的缓冲区指针
  \param[in]   count   要读取的数据大小（字节数）
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于从UART设备读取指定数量的数据到缓冲区中。
               只有在UART处于使用中状态时才能进行读操作。
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

/**
  \fn          static void test_uart_cb(uint32_t event)
  \brief       UART测试回调函数
  \param[in]   event  UART事件类型
  \return      无
  \details     该函数用于处理UART的各种事件，包括接收超时、接收完成、发送完成以及各种错误事件。
               这是一个测试用的回调函数，在实际应用中需要根据具体需求实现相应的事件处理逻辑。
*/
#if 0
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
#endif

/**
  \fn          int api_test_uart(void)
  \brief       UART设备测试接口
  \return      无
  \details     该函数用于测试UART设备的基本功能。它会遍历所有可用的UART索引，
               对每个UART执行完整的创建、打开、配置、读写、功耗控制、关闭和删除流程。
               这是一个示例测试函数，展示了如何使用UART API进行基本操作。
*/
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

    return 0;
}
