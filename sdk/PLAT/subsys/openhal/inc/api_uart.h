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

typedef enum {
    OPEN_UART_IOCTL_ISR_CB = 0,    
    OPEN_UART_IOCTL_BAUDRATE ,    
    OPEN_UART_IOCTL_CONTROL ,  
    GET_RX_COUNT ,   
} api_uart_ioctl_t;

api_ret_t api_uart_query(uint32_t usrId);
int api_uart_startup(void* para);
uint32_t api_uart_create(uint32_t index,void *cfg);
api_ret_t api_uart_delete(uint32_t usrId);
api_ret_t api_uart_open(uint32_t usrId,void *cfg,size_t timeout);
api_ret_t api_uart_close(uint32_t usrId);
api_ret_t api_uart_write(uint32_t usrId, void* buf, size_t count);
api_ret_t api_uart_read(uint32_t usrId, void* buf, size_t count);
api_ret_t api_uart_ioctl(uint32_t usrId,api_uart_ioctl_t type, void *para);
api_ret_t api_uart_pmctl(uint32_t usrId,open_hal_pm_t *cfg, size_t count);
int api_test_uart(void);
#ifdef __cplusplus
}
#endif
#endif /* API_UART_H_ */