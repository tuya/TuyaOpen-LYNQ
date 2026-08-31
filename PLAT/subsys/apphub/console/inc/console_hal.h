
#ifndef __CONSOLE_HAL_H__
#define __CONSOLE_HAL_H__
#ifdef __cplusplus
extern "C" {
#endif

#define TASK_SIZE_CONSOLE_HAL               (10 * 1024)
#define TASK_PRIO_CONSOLE_HAL                osPriorityBelowNormal7

#define CMD_RX_SIZE_MAX             (128)
#define FILE_RX_SIZE_MAX            (1024)
#ifdef USB_CONSOLE
#define USB_BUFFER_SIZE             4096
#endif

typedef enum console_hal_event_bits {
    UART_RX_READY = ( 1 << 0 ),
    UART_RX_TIMEOUT = ( 1 << 1 ),
    UART_RX_ERROR = ( 1 << 2 ),
    UART_TX_DONE = ( 1 << 3 ),
#ifdef USB_CONSOLE
    USB_RX_DONE = (1 << 4),
#endif
} ConsoleHalEventBitsT;

int consoleHalInit(void);
int consoleHalDeinit(void);
#ifdef USB_CONSOLE
void usbHalCallback(uint32_t event);
#endif

#ifdef __cplusplus
}
#endif
#endif
