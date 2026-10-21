/****************************************************************************
 *
 ****************************************************************************/
#include <stdio.h>
#include "string.h"
#include "osasys.h"
#include "cmsis_os2.h"
#include "sntp.h"
#include "ol_time_api.h"
#include "ol_log.h"


void test_set_time(void)
{
    int ret = 0;
    ol_ntp_time_t time;

    time.year = 2033;
    time.month = 12;
    time.day = 31;
    time.hour = 15;
    time.minute = 59;
    time.second = 58;
    time.timezone = 32;

    ret = ol_set_time(&time);
    OL_LOG_INFO("ol_set_time ret = %d", ret);
}

void test_get_time(void)
{
    char sign = '+';
    ol_ntp_time_t time;

    if(0 == ol_get_time(&time))
    {
        if(time.timezone < 0)
        {
            sign = '-';
        }

        OL_LOG_INFO("UTC Time = %d-%02d-%02d, %02d:%02d:%02d%c%02d",
                                    time.year, time.month, time.day,\
                                    time.hour, time.minute, time.second,\
                                    sign, time.timezone);
    }
}

void time_demo(void)
{
    int ret = 0;
    time_t time = 0;
    struct tm* gtCurr = NULL;

    OL_LOG_PRINTF("------ system time ------");

    test_get_time();

    test_set_time();
    osDelay(2000);

    test_get_time();

    ret = ol_ntp("ntp.aliyun.com", 123);
    OL_LOG_INFO("ol_ntp = %d",ret);

    osDelay(3000);
    test_get_time();

    time = ol_time(NULL);
    OL_LOG_INFO("ol_time = %d",time);

    gtCurr = ol_localtime(&time);
    if(NULL != gtCurr)
    {
        OL_LOG_INFO("ol_localtime = %d-%02d-%02d, %02d:%02d:%02d\n",
                                   gtCurr->tm_year + 1900,
                                   gtCurr->tm_mon + 1,
                                   gtCurr->tm_mday,
                                   gtCurr->tm_hour,
                                   gtCurr->tm_min,
                                   gtCurr->tm_sec);
    }

    OL_LOG_PRINTF("------ system time end------");
}

