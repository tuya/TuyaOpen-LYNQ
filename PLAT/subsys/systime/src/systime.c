#ifdef FEATURE_SUBSYS_SYSTIME_ENABLE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include "cmsis_os2.h"
#include "ps_lib_api.h"
#include "ps_nm_if.h"
#include "mw_aon_info.h"
#include "sntp.h"
#include "systime.h"
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "iniparse.h"
#endif
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#include "sysalarm.h"


#define INI_KEY_SYSTIME                             "systime"
#define SNTP_SERVER                                 "ntp.aliyun.com"
#define SECOND_IN_LEAP_YEAR                         31622400
#define SECOND_IN_COMMON_YEAR                       31536000
#define SECOND_IN_LONG_MONTH                        2678400
#define SECOND_IN_SHORT_MONTH                       2592000
#define SECOND_IN_LEAP_YEAR_FEBRUARY                2505600
#define SECOND_IN_COMMON_YEAR_FEBRUARY              2419200
#define SECOND_IN_DAY                               86400
#define SECOND_IN_HOUR                              3600
#define SECOND_IN_MINUTE                            60


static int8_t      gTimeZone          = 8;
static const char *gWeek[7]           = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static const char *gMonth[12]         = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
static uint32_t    gSecondInMonth[12] = {SECOND_IN_LONG_MONTH,  0,                     SECOND_IN_LONG_MONTH,
                                         SECOND_IN_SHORT_MONTH, SECOND_IN_LONG_MONTH,  SECOND_IN_SHORT_MONTH,
                                         SECOND_IN_LONG_MONTH,  SECOND_IN_LONG_MONTH,  SECOND_IN_SHORT_MONTH,
                                         SECOND_IN_LONG_MONTH,  SECOND_IN_SHORT_MONTH, SECOND_IN_LONG_MONTH};


static bool isLeapYear(uint32_t year)
{
    if ((((year % 4) == 0) && ((year % 100) != 0)) || ((year % 400) == 0))
    {
        return true;
    }
    else
    {
        return false;
    }
}

time_t time_mktime(struct tm *_timeptr)
{
    time_t  time = -1;
    int32_t year = 0;
 
    if (_timeptr == NULL)
    {
        goto labelEnd;
    }

    year = 1900 + _timeptr->tm_year;
    if (year < 1970)
    {
        goto labelEnd;
    }

    time = 0;
    for (uint32_t i=1970; i<year; i++)
    {
        time += (isLeapYear(i) == true) ? SECOND_IN_LEAP_YEAR : SECOND_IN_COMMON_YEAR;
    }

    gSecondInMonth[1] = (isLeapYear(year) == true) ? SECOND_IN_LEAP_YEAR_FEBRUARY : SECOND_IN_COMMON_YEAR_FEBRUARY;
    for (uint32_t i=0; i<_timeptr->tm_mon; i++)
    {
        time += gSecondInMonth[i];
    }

    time += (_timeptr->tm_mday - 1) * SECOND_IN_DAY
         +   _timeptr->tm_hour      * SECOND_IN_HOUR
         +   _timeptr->tm_min       * SECOND_IN_MINUTE
         +   _timeptr->tm_sec;

labelEnd:
    return time;
}

time_t time_time(time_t *_timer)
{
    int32_t           retVal = -1;
    utc_timer_value_t time   = {0};

    if (appGetSystemTimeUtcSync(&time) == CMS_RET_SUCC)
    {
        retVal = time.UTCsecs;
    }

    if (_timer != NULL)
    {
        *_timer = retVal;
    }

    return retVal;
}

struct tm *time_gmtime(const time_t *_timer)
{
    static struct tm tmTime       = {0};
    uint32_t         second       = 0;
    uint32_t         year         = 1970;
    uint32_t         secondInYear = SECOND_IN_COMMON_YEAR;
    bool             leap         = false;

    if ((_timer == NULL) || (*_timer < 0))
    {
        goto labelEnd;
    }

    memset(&tmTime, 0, sizeof(tmTime));
    tmTime.tm_year = 70;

    second = *_timer;

    tmTime.tm_wday = (second / SECOND_IN_DAY + 4) % 7;

    while (1)
    {
        leap         = isLeapYear(year++);
        secondInYear = (leap == true) ? SECOND_IN_LEAP_YEAR : SECOND_IN_COMMON_YEAR;
        if (second >= secondInYear)
        {
            second -= secondInYear;
            tmTime.tm_year++;
        }
        else
        {
            tmTime.tm_yday = second / SECOND_IN_DAY;
            break;
        }
    }

    gSecondInMonth[1] = (leap == true) ? SECOND_IN_LEAP_YEAR_FEBRUARY : SECOND_IN_COMMON_YEAR_FEBRUARY;
    for (uint32_t i=0; i<12; i++)
    {
        if (second >= gSecondInMonth[i])
        {
            second -= gSecondInMonth[i];
            tmTime.tm_mon++;
        }
        else
        {
            break;
        }
    }

    tmTime.tm_mday  = second / SECOND_IN_DAY + 1;
    second         %= SECOND_IN_DAY;

    tmTime.tm_hour  = second / SECOND_IN_HOUR;
    second         %= SECOND_IN_HOUR;

    tmTime.tm_min   = second / SECOND_IN_MINUTE;
    tmTime.tm_sec   = second % SECOND_IN_MINUTE;

labelEnd:
    return &tmTime;
}

struct tm *time_localtime(const time_t *_timer)
{
    time_t timer = *_timer + gTimeZone * SECOND_IN_HOUR;

    return time_gmtime(&timer);
}

char *time_ctime(const time_t *_time)
{
    static char  strTime[] = "Sat Jan 01 00:00:00 2000";
    struct tm   *tmTime    = NULL;

    if (_time == NULL)
    {
        return NULL;
    }

    tmTime = time_gmtime(_time);
    snprintf(strTime, sizeof(strTime), "%s %s %02d %02d:%02d:%02d %04d", gWeek[tmTime->tm_wday], gMonth[tmTime->tm_mon], tmTime->tm_mday, tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec, 1900 + tmTime->tm_year);

    return strTime;
}

double time_difftime(time_t _time2, time_t _time1)
{
    return (_time2 - _time1);
}

void timeZoneSet(int8_t timeZone)
{
    gTimeZone = timeZone;
}

int8_t timeZoneGet(void)
{
    return gTimeZone;
}

int32_t saveTime(time_t time)
{
    int32_t    retVal     = 0;
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    int32_t    valueWrite = time;
#endif
    struct tm *tmTime     = time_gmtime(&time);

    SYSLOG_INFO("%d-%d-%d %d:%d:%d %d\r\n", 1900 + tmTime->tm_year, tmTime->tm_mon + 1, tmTime->tm_mday, tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec, tmTime->tm_wday);

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    retVal = iniKeyValueWrite(DEFAULT_INFO, INI_KEY_SYSTIME, INI_VALUE_INT, (void *)&valueWrite);
#endif

    return retVal;
}

void loadTime(void)
{
    int32_t *value = NULL;

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    value = iniKeyValueRead(DEFAULT_INFO, INI_KEY_SYSTIME, INI_VALUE_INT);
#endif
    if (value != NULL)
    {
        timeSet(time_gmtime((time_t *)value));
        free(value);
        value = NULL;
    }
}

int32_t timeSync(void)
{
    int32_t retVal = psSntpReq(0, SNTP_SERVER, SNTP_DEFAULT_PORT, TRUE, LWIP_PS_INVALID_CID, 0, 0);

    saveTime(time_time(NULL));
    sysAlarmSync();

    return retVal;
}

int32_t timeSet(struct tm *tmTime)
{
    int32_t  retVal = -1;
    uint32_t timer1 = 0;
    uint32_t timer2 = 0;

    if ((tmTime == NULL) || (1900 + tmTime->tm_year < 2000))
    {
        goto labelEnd;
    }

    SYSLOG_INFO("%d-%d-%d %d:%d:%d %d\r\n", 1900 + tmTime->tm_year, tmTime->tm_mon + 1, tmTime->tm_mday, tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec, tmTime->tm_wday);

    timer1 = ((1900 + tmTime->tm_year) << 16) + ((tmTime->tm_mon + 1) << 8) + tmTime->tm_mday;
    timer2 = (tmTime->tm_hour << 24) + (tmTime->tm_min << 16) + (tmTime->tm_sec << 8);
    retVal = appSetSystemTimeUtcSync(timer1, timer2, 0);
    mwAonSetUtcTimeSyncFlag(TRUE);

    saveTime(time_mktime(tmTime));
    sysAlarmSync();

labelEnd:
    return retVal;
}

int32_t _gettimeofday(struct timeval *tv, struct timezone *tz)
{
    return time_gettimeofday(tv, tz);
}

int32_t time_gettimeofday(struct timeval *tv, struct timezone *tz)
{
    int32_t           retVal = 0;
    utc_timer_value_t time   = {0};

    if (tv != NULL)
    {
        if (appGetSystemTimeUtcSync(&time) == CMS_RET_SUCC)
        {
            tv->tv_sec  = time.UTCsecs;
            tv->tv_usec = time.UTCms * 1000;
        }
        else
        {
            retVal = -1;
        }
    }

    if (tz != NULL)
    {
        tz->tz_minuteswest = gTimeZone;
    }

    return retVal;
}

#endif
