#ifndef _PORT_STM32L433_UTIL_H_
#define _PORT_STM32L433_UTIL_H_

#include <stdint.h>
#include "hed_private.h"

/**************************************************************************
 * Global Type Definition
 ***************************************************************************/

#ifdef _DEBUG

#if HAL_UART_PRINTF_ENABLE

#endif // #if HAL_UART_PRINTF_ENABLE

extern int fputc(int ch, FILE *f);

#else
#define fputc(ch, f)
// #define hal_printf(...)

#endif // #ifdef _DEBUG

//extern void hal_printf(const char *format, ...);

extern void hal_delay(uint32_t us);
extern uint32_t hal_systick(void);
void hal_printf(const char *format, ...);

#endif /*_PORT_STM32L433_UTIL_H*/
