#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include DEBUG_LOG_HEADER_FILE

#include "alipay_common.h"
#include "vendor_os.h"

int alipay_rand(void)
{
	return (int)rand();
}


int32_t get_Rng_number(uint8_t *data, unsigned size) {
	if(!data || size == 0)
		return 0;
	int cnt = 0;
	for(;cnt < size;cnt++)
	{
		data[cnt] = rand() & 0xff;
	}
		
	return 1;
    hal_rng_get_data(data, size);
    //alipay_log_ext("random bytes[%d]: data[0]:0x%02x, data[1]:0x%02x - data[n-2]:0x%02x, data[n-1]0x%02x", size, data[0], data[1], data[size-2], data[size-1]);
    return 1;
}

uint32_t alipay_get_timestamp(void) {
	utc_timer_value_t utcTime;
	appGetSystemTimeUtcSync(&utcTime);
	//alipay_log_ext("timestamp: %d", utcTime.UTCsecs);
    return utcTime.UTCsecs;
}

uint32_t alipay_get_compile_timestamp() {
    uint32_t build_timestamp = 1706872200;
    //hal_get_compile_timestamp(&build_timestamp);
    //alipay_log_ext("build_timestamp: %d", build_timestamp);
    return build_timestamp;
}

void alipay_set_system_time(PARAM_IN int32_t timestamp_s)
{
	uint32_t timer1 = 0;
    uint32_t timer2 = 0;
	struct tm gtCurr;
	alipay_log_ext("alipay_set_system_time");
	gmtime_ec((time_t *)&timestamp_s, &gtCurr);
	
    timer1 = ((1900 + gtCurr.tm_year) << 16) + ((gtCurr.tm_mon) << 8) + gtCurr.tm_mday;
    timer2 = (gtCurr.tm_hour << 24) + (gtCurr.tm_min << 16) + (gtCurr.tm_sec << 8);
    appSetSystemTimeUtcSync(timer1, timer2, 0);
    mwAonSetUtcTimeSyncFlag(TRUE);
	//alipay_log_ext("alipay_set_system_time: %d", timestamp_s);
}

void alipay_iot_get_local_time(alipay_iot_local_time *time)
{
	utc_timer_value_t timeUtc;
	 
    int32_t ret = appGetSystemTimeUtcSync(&timeUtc);//appCheckSystemTimeSync
    if(ret == 0)
    {
        
        time->year = timeUtc.UTCtimer1>>16;
        time->month = (timeUtc.UTCtimer1&0xFF00)>>8;
        time->day = timeUtc.UTCtimer1&0xFF;
        time->hour = (timeUtc.UTCtimer2>>24)+8;
        time->minute = (timeUtc.UTCtimer2&0xFF0000)>>16;
        time->second = (timeUtc.UTCtimer2&0xFF00)>>8;
	}
	//alipay_log_ext("alipay_iot_get_local_time :%d-%d-%d,%d:%d:%d", time->year,time->month,time->day,time->hour,time->minute,time->second);
}


void* alipay_malloc(uint32_t size) {
    void *ptr = malloc(size);
    if (!ptr) {
        alipay_log_ext("malloc failed");
    }
    memset(ptr, 0, size);
    return ptr;
}

void* alipay_calloc(uint32_t nblock,uint32_t size) {
    void *ptr = calloc(nblock, size);
    if (!ptr) {
        alipay_log_ext("calloc failed");
    }
    return ptr;
}

void alipay_free(void* pt) {
    if (pt) free(pt);
}

void* alipay_realloc(void* ptr, uint32_t size) {
    return realloc(ptr, size);
}

void alipay_log_ext(const char *format, ...) {
    static char ALIPAY_log_buf[1024];
    memset(ALIPAY_log_buf, 0, sizeof(ALIPAY_log_buf));
    va_list arg;
    va_start(arg, format);
    vsnprintf(ALIPAY_log_buf, sizeof(ALIPAY_log_buf)-1, format, arg);
	ECPLAT_PRINTF(UNILOG_PLAT_ALIPAY, alipay_log_ext, P_DEBUG, "%s",ALIPAY_log_buf);
	//#ifdef FEATURE_SUBSYS_CONSOLE_ENABLE
    //extern void ecConsolePutStr(const char* str);
    //ecConsolePutStr(ALIPAY_log_buf);
	//ecConsolePutStr("\r\n");
    //#endif
    va_end(arg);
}
