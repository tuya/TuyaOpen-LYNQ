#include DEBUG_LOG_HEADER_FILE
#include "volc_time.h"

#include <stdio.h>
#include <sys/time.h>
#include <string.h>

#include "ps_lib_api.h"

#include "volc_errno.h"
#include "volc_type.h"

uint64_t volc_get_time(void) {
 	utc_timer_value_t time   = {0};
	appGetSystemTimeUtcSync(&time);
	uint64_t timestamp = 0;
	timestamp = (uint64_t)time.UTCsecs * VOLC_HUNDREDS_OF_NANOS_IN_A_SECOND + (uint64_t)time.UTCms * VOLC_HUNDREDS_OF_NANOS_IN_A_MILLISECOND;
	return timestamp;
}

uint32_t volc_timestamp_format_with_ms_and_timezone(char* p_dest_buffer, uint32_t dest_buffer_len, uint64_t timestamp_milliseconds) {
    uint32_t str_len = 0;
    time_t seconds = timestamp_milliseconds / 1000;

	struct tm gtCurr;
	gmtime_ec((time_t *)&seconds, &gtCurr);
	
    str_len = sprintf(p_dest_buffer, "%d-%02d-%02d %02d:%02d:%02d.%03d ", gtCurr.tm_year + 1900, gtCurr.tm_mon, gtCurr.tm_mday,
        gtCurr.tm_hour, gtCurr.tm_min, gtCurr.tm_sec, (timestamp_milliseconds % 1000LL));
    return str_len;
}

uint32_t volc_generate_timestamp_str(uint64_t timestamp, char* format_str, char* p_dest_buffer, uint32_t dest_buffer_len, uint32_t* p_formatted_str_len) {
    time_t timestamp_seconds;
    uint32_t formatted_str_len = 0;
    uint64_t timestamp_millisecond = 0;

    if (NULL == p_dest_buffer || NULL == p_formatted_str_len) {
        return VOLC_FAILED;
    }
    if (strnlen(format_str, VOLC_MAX_TIMESTAMP_FORMAT_STR_LEN + 1) > VOLC_MAX_TIMESTAMP_FORMAT_STR_LEN) {
        return VOLC_FAILED;
    }

    formatted_str_len = 0;
    *p_formatted_str_len = 0;
    // TODO: maybe we should use a universal method to replace it
    if (0 == strncmp(format_str, "%Y-%m-%d %H:%M:%S.xxx ", VOLC_MAX_TIMESTAMP_FORMAT_STR_LEN)) {
        timestamp_millisecond = timestamp / VOLC_HUNDREDS_OF_NANOS_IN_A_MILLISECOND;
        formatted_str_len = volc_timestamp_format_with_ms_and_timezone(p_dest_buffer, dest_buffer_len, timestamp_millisecond);
    } else {
        timestamp_seconds = timestamp / VOLC_HUNDREDS_OF_NANOS_IN_A_SECOND;
        formatted_str_len = (uint32_t) strftime(p_dest_buffer, dest_buffer_len, format_str, gmtime(&timestamp_seconds));
    }
    if (0 == formatted_str_len) {
        return VOLC_FAILED;
    }

    p_dest_buffer[formatted_str_len] = '\0';
    *p_formatted_str_len = formatted_str_len;

    return VOLC_SUCCESS;
}

uint32_t volc_get_time_str(char* time_str, uint32_t maxsize) {
    uint32_t ret = VOLC_STATUS_SUCCESS;
   	utc_timer_value_t timeUtc   = {0};
	appGetSystemTimeUtcSync(&timeUtc);
	snprintf(time_str, maxsize, "%d_%02d_%02d_%02d%02d%02d", timeUtc.UTCtimer1>>16, (timeUtc.UTCtimer1&0xFF00)>>8, (timeUtc.UTCtimer1&0xFF), (timeUtc.UTCtimer2>>24), (timeUtc.UTCtimer2&0xFF0000)>>16, (timeUtc.UTCtimer2&0xFF00)>>8);
    return ret;
}

uint64_t volc_get_montionic_time(void){
	uint32_t tick = osKernelGetTickCount();
	return (uint64_t)tick * VOLC_HUNDREDS_OF_NANOS_IN_A_MILLISECOND;
};