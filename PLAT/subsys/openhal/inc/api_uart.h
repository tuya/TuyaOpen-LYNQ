/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_uart.h
 * Description:  ec7xx openhal uart entry header file
 * History:      Rev1.0   2024-02-23
 *
 ****************************************************************************/
#ifndef  API_UART_H_
#define  API_UART_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "api_def.h"
#define EC_UART_INDEX_START     (2)
#define EC_UART_INDEX_LIMIT     (4)
// 该结构体尽量压缩，4Bytes对齐方便存取
typedef struct {
    uint32_t rxd        : 6;    // paddr < 64
    uint32_t txd        : 6;    // paddr < 64
    uint32_t rts        : 6;    // paddr < 64
    uint32_t cts        : 6;    // paddr < 64
    uint32_t baud       : 4;    // 波特率查表: uart_baudTable
    uint32_t mode       : 4;    // 控制位：Asynchronous Master/Asynchronous Slave/Single-wire/IrDA/Smart Card
    uint32_t bits       : 4;    // 数据位：8(default)/9/-/-/-/5/6/7
    uint32_t parity     : 2;    // 校验位：No(default)/Even/Odd
    uint32_t stop       : 2;    // 停止位：1(default)/2/1.5/0.5
    uint32_t flow       : 2;    // 流控位：No(default)/RTS/CTS/ALL
} uart_config_t;

typedef enum {
    OPEN_UART_IOCTL_ISR_CB = 0,    
    OPEN_UART_IOCTL_BAUDRATE ,    
    OPEN_UART_IOCTL_CONTROL ,  
    GET_RX_COUNT ,   
} api_uart_ioctl_t;

/**
  \fn          int8_t *api_uart_startup(void* para, int8_t *pad)
  \brief       所有UART上电初始化为配置状态
  \param[in]   para  指向UART配置参数数组的指针
  \param[in]   pad   指向PAD配置参数数组的指针
  \return      返回指向UART状态列表的指针
  \details     该函数用于在系统上电时初始化所有UART设备。它会遍历所有可用的UART索引，
               并将它们设置为空闲状态，以便后续使用。
*/
api_ret_t api_uart_query(uint32_t usrId);
int8_t *api_uart_startup(void* para, int8_t *pad);
/**
  \fn          api_ret_t api_uart_setup(int8_t index, uart_config_t* para)
  \brief       单项初始化，使用HAL统一的参数格式
  \param[in]   index  UART索引编号
  \param[in]   para   UART配置参数指针
  \return      执行结果
  \details     该函数用于单项初始化，使用HAL统一的参数格式，会执行硬件配置。
               para参数包含详细的UART配置信息，包括引脚配置、波特率、数据位、校验位、停止位和流控设置。
*/
api_ret_t api_uart_setup(int8_t index, uart_config_t* para);
api_ret_t api_uart_create(int8_t index, uart_config_t *cfg, void *out);
api_ret_t api_uart_delete(uint32_t usrId);
api_ret_t api_uart_open(uint32_t usrId,void *cfg,size_t timeout);
api_ret_t api_uart_close(uint32_t usrId);
api_ret_t api_uart_write(uint32_t usrId, void* buf, size_t count);
api_ret_t api_uart_read(uint32_t usrId, void* buf, size_t count);
api_ret_t api_uart_ioctl(uint32_t usrId,api_uart_ioctl_t type, void *para);
api_ret_t api_uart_pmctl(uint32_t usrId,open_hal_pm_t *cfg, size_t count);
int api_test_uart(void);

#define CSV_CFG_UART_ITEMS  (11)
int32_t api_uart_parse(char* str,uart_config_t *cfg);
#ifdef __cplusplus
}
#endif
#endif /* API_UART_H_ */