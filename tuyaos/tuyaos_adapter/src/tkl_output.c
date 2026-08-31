#include <stdio.h>
#include <stdarg.h>
#include "ol_log.h"
#include "tkl_output.h"

#define BUFF_SIZE  512
static char buf[BUFF_SIZE];

/**
* @brief Output log information
*
* @param[in] format: log information
*
* @note This API is used for outputing log information
*
* @return
*/
VOID_T tkl_log_output(CONST CHAR_T *format, ...)
{
	va_list ap;
	int len;
	if (NULL == format) {
		return;
	}
	va_start(ap, format);
	len = vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);
	if (len < 0) {
		return;
	}
	buf[sizeof(buf) - 1] = '\0';
	MLOG(UNILOG_API, P_VALUE, "%s", buf);
}

/**
* @brief Close log port
*
* @param VOID
*
* @note This API is used for closing log port.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_log_close(VOID_T)
{
    return OPRT_OK;
}

/**
* @brief Open log port
*
* @param VOID
*
* @note This API is used for openning log port.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_log_open(VOID_T)
{
    return OPRT_OK;
}
