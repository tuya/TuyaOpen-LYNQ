#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "ol_log.h"
#include "tkl_output.h"

#define BUFF_SIZE  1024
/* ol_log.h stages unilog payloads in 256 bytes and notes that anything over
 * ~200 bytes may be dropped, so a long line goes out in several pieces rather
 * than being truncated. */
#define LOG_CHUNK_SIZE 192

static char buf[BUFF_SIZE];

/**
* @brief Remove ANSI escape sequences in place
*
* @param[in,out] s: text to clean up
* @param[in] len: text length
*
* @note unilog carries plain text, so the colour sequences TuyaOpen puts around
* each line would show up as literal "[0;32;49m" noise.
*
* @return the new length
*/
static int strip_ansi(char *s, int len)
{
	int in = 0;
	int out = 0;

	while (in < len) {
		if (s[in] == 0x1b) {
			/* CSI ... final byte in @-~ */
			in++;
			if (in < len && s[in] == '[') {
				in++;
				while (in < len && (s[in] < '@' || s[in] > '~')) {
					in++;
				}
			}
			if (in < len) {
				in++;
			}
			continue;
		}
		s[out++] = s[in++];
	}
	s[out] = '\0';
	return out;
}

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
	int off;

	if (NULL == format) {
		return;
	}

	va_start(ap, format);
	len = vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);
	if (len < 0) {
		return;
	}
	/* vsnprintf reports the length it would have written */
	if (len > (int)sizeof(buf) - 1) {
		len = (int)sizeof(buf) - 1;
	}
	buf[len] = '\0';

	len = strip_ansi(buf, len);

	for (off = 0; off < len; off += LOG_CHUNK_SIZE) {
		int n = len - off;
		char save;

		if (n > LOG_CHUNK_SIZE) {
			n = LOG_CHUNK_SIZE;
		}
		save = buf[off + n];
		buf[off + n] = '\0';
		MLOG(UNILOG_TuyaOpen, P_VALUE, "%s", buf + off);
		buf[off + n] = save;
	}
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
