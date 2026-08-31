/******************************************************************************
 Copyright(C),CEC Huada Electronic Design Co.,Ltd.
 File name: 		port_stm32l433_util.c
 Author:			zhengwd
 Version:			V1.0
 Date:			2020-04-01
 Description:
 History:

******************************************************************************/

/***************************************************************************
 * Include Header Files
 ***************************************************************************/
#include <stdarg.h>
#include "cmsis_os2.h"   
#include DEBUG_LOG_HEADER_FILE

//_weak void hal_printf(const char *format, ...)
/*********************************************************************************
Function:       hal_printf
Description:   打印输出log信息
Input:          format...
Output:         no
Return:         no
Others:         no
*********************************************************************************/
void hal_printf(const char *format, ...)
{
	static char ALIPAY_log_buf[1024];
    memset(ALIPAY_log_buf, 0, sizeof(ALIPAY_log_buf));
    va_list arg;
    va_start(arg, format);
    vsnprintf(ALIPAY_log_buf, sizeof(ALIPAY_log_buf)-1, format, arg);
	ECPLAT_PRINTF(UNILOG_PLAT_ALIPAY, alipay_sc_v2, P_DEBUG, "%s",ALIPAY_log_buf);
    va_end(arg);
}

/*********************************************************************************
Function:       hal_delay
Description:   用于延时
Input:          us, 延时输入值，单位为微秒
Output:         no
Return:         no
Others:         no
*********************************************************************************/
extern void delay_us(uint32_t);
void hal_delay(uint32_t us)
{
    delay_us(us);
}

/*********************************************************************************
Function:       hal_systick
Description:   返回Tick值，用于时间计时
Input:          no
Output:         no
Return:         计时的tick值，单位为ms
Others:         no
*********************************************************************************/
uint32_t hal_systick(void)
{
    return osKernelGetTickCount();
}
