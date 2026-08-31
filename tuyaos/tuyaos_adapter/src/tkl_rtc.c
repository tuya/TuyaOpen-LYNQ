#include <stdlib.h>
#include <time.h>
#include "tkl_rtc.h"
#include "cmsis_os2.h"
#include "ol_time_api.h"
#include "vlog.h"

/**
 * @brief rtc init
 *
 * @param[in] none
 *
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_rtc_init(VOID_T)
{
    return OPRT_OK;
}

/**
 * @brief rtc deinit
 * @param[in] none
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_rtc_deinit(VOID_T)
{
    return OPRT_OK;
}

/**
 * @brief rtc time set
 *
 * @param[in] time_sec: rtc time seconds
 *
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_rtc_time_set(TIME_T time_sec)
{
	time_t t = time_sec;
	struct tm* gmtime = ol_gmtime(&t);
	ol_ntp_time_t time;
	
	if (!gmtime) {
		LOGE("convert into gmtime failed");
		return OPRT_COM_ERROR;
	}
	
	time.year = gmtime->tm_year + 1900;
	time.month = gmtime->tm_mon + 1;
	time.day = gmtime->tm_mday;
	time.hour = gmtime->tm_hour;
	time.minute = gmtime->tm_min;
	time.second = gmtime->tm_sec;
	time.timezone = 0;
	
	int ret = ol_set_time(&time);
	if (0 != ret) {
		LOGE("rtc set time failed, ret: %d", ret);
		return OPRT_COM_ERROR;
	}

	return OPRT_OK;
}

/**
 * @brief rtc time get
 *
 * @param[in] time_sec:rtc time seconds
 *
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_rtc_time_get(TIME_T *time_sec)
{
	time_t time;
	
	time = ol_time(NULL);
	*time_sec = (TIME_T)time;
	return OPRT_OK;
}

