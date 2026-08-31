/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    test.h
 * Description:  EC618 mqtt demo entry header file
 * History:      Rev1.0   2018-07-12
 *
 ****************************************************************************/
#ifndef __SYSTIME_H__
#define __SYSTIME_H__


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


time_t     time_mktime(struct tm *_timeptr);
time_t     time_time(time_t *_timer);
struct tm *time_gmtime(const time_t *_timer);
struct tm *time_localtime (const time_t *_timer);
char      *time_ctime(const time_t *_time);
double     time_difftime(time_t _time2, time_t _time1);
void       timeZoneSet(int8_t timeZone);
int8_t     timeZoneGet(void);
int32_t    timeSync(void);
int32_t    timeSet(struct tm *tmTime);
int32_t    saveTime(time_t time);
void       loadTime(void);
int32_t    time_gettimeofday(struct timeval *tv, struct timezone *tz);

#endif
