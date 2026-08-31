#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "cmsis_os2.h"
#include "ps_lib_api.h"
#include "slpman.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_SUBSYS_SYSTIME_ENABLE
#include "systime.h"
#endif
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#include "sysalarm.h"


#define SYS_ALARM_FILE          "D:/sysAlarm.dat"


static SysAlarmInfoT     *gSysAlarmList = NULL;
static uint32_t           gCountMax  = 0;
static uint32_t           gCount     = 0;
static uint32_t           gIndex     = 0;
static SysAlarmCallbackT  gCallback  = NULL;


static void sysAlarmDeepSlpTimerCallback(uint8_t id)
{
    if ((id == DEEPSLP_TIMER_ID2) && (gCallback != NULL) && (gSysAlarmList != NULL) && (gIndex < gCount) && (gSysAlarmList[gIndex].on == true))
    {
        gCallback(&gSysAlarmList[gIndex]);
        sysAlarmSync();
    }
}

static int32_t loadSysAlarm(SysAlarmInfoT *sysAlarmList, uint32_t countMax)
{
    int32_t   retVal  = 0;
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    FILE     *file    = NULL;
    uint32_t  sizeMax = sizeof(SysAlarmInfoT) * countMax;
#endif

    if ((sysAlarmList == NULL) || (countMax == 0))
    {
        SYSLOG_DEBUG("param error.\r\n");
        goto labelEnd;
    }

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    file = file_fopen(SYS_ALARM_FILE, "r");
    if (file == NULL)
    {
        SYSLOG_DEBUG("Failed to open the file \"%s\".\r\n", SYS_ALARM_FILE);
        goto labelEnd;
    }

    memset(sysAlarmList, 0, sizeMax);
    retVal = file_fread((void *)sysAlarmList, 1, sizeMax, file) / sizeof(SysAlarmInfoT);
    SYSLOG_DEBUG("alarm count: %d\r\n", retVal);
#endif

labelEnd:
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    if (file != NULL)
    {
        file_fclose(file);
    }
#endif
    return retVal;
}

static int32_t saveSysAlarm(SysAlarmInfoT *sysAlarmList, uint32_t count)
{
    int32_t  retVal = 0;
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    FILE    *file   = NULL;
#endif

    if ((sysAlarmList == NULL) || (count == 0))
    {
        SYSLOG_DEBUG("param error.\r\n");
        goto labelEnd;
    }

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    file = file_fopen(SYS_ALARM_FILE, "w+");
    if (file == NULL)
    {
        SYSLOG_DEBUG("Failed to open the file \"%s\".\r\n", SYS_ALARM_FILE);
        goto labelEnd;
    }

    retVal = file_fwrite((void *)sysAlarmList, 1, sizeof(SysAlarmInfoT) * count, file) / sizeof(SysAlarmInfoT);
    SYSLOG_DEBUG("alarm count: %d, %d\r\n", count, retVal);
#endif

    SYSLOG_DEBUG("");
    for (uint32_t i=0; i<count; i++)
    {
        printf("%ssysAlarmList[%d]=%d:%d\r\n", ((i == 0) ? "" : "                   "), i, sysAlarmList[i].hour, sysAlarmList[i].minute);
    }

labelEnd:
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    if (file != NULL)
    {
        file_fclose(file);
    }
#endif
    return retVal;
}

int32_t sysAlarmInit(SysAlarmCallbackT callback, uint32_t countMax)
{
    int32_t  retVal = -1;
    uint32_t size   = countMax * sizeof(SysAlarmInfoT);

    if ((gSysAlarmList != NULL) || (countMax == 0))
    {
        SYSLOG_DEBUG("Param error.\r\n");
        goto labelEnd;
    }

    gSysAlarmList = (SysAlarmInfoT *)malloc(size);
    if (gSysAlarmList == NULL)
    {
        SYSLOG_DEBUG("Failed to malloc %d bytes for gSysAlarmList\r\n", size);
        goto labelEnd;
    }
    memset(gSysAlarmList, 0, size);

    gCallback = callback;
    gCountMax = countMax;
    gCount    = loadSysAlarm(gSysAlarmList, countMax);

    slpManDeepSlpTimerRegisterExpCb(DEEPSLP_TIMER_ID2, sysAlarmDeepSlpTimerCallback);
    sysAlarmSync();

    retVal = 0;

labelEnd:
    if (retVal != 0)
    {
        if (gSysAlarmList != NULL)
        {
            free(gSysAlarmList);
            gSysAlarmList = NULL;
        }
    }
    return retVal;
}

static int32_t addAlarm(SysAlarmInfoT *sysAlarmInfo)
{
    int32_t  retVal = -1;
    uint32_t index  = 0;

    if ((gSysAlarmList == NULL) || (gCount >= gCountMax) || (sysAlarmInfo == NULL) || (sysAlarmInfo->hour > 23) || (sysAlarmInfo->minute > 59))
    {
        SYSLOG_DEBUG("Param error.\r\n");
        goto labelEnd;
    }

    for (index=0; index<gCount; index++)
    {
        if (((sysAlarmInfo->hour * 60) + sysAlarmInfo->minute) < ((gSysAlarmList[index].hour * 60) + gSysAlarmList[index].minute))
        {
            memmove(&gSysAlarmList[index + 1], &gSysAlarmList[index], (gCount - index) * sizeof(SysAlarmInfoT));
            break;
        }
    }
    memcpy(&gSysAlarmList[index], sysAlarmInfo, sizeof(SysAlarmInfoT));
    gCount++;

    retVal = 0;

labelEnd:
    return retVal;
}

int32_t sysAlarmAdd(SysAlarmInfoT *sysAlarmInfo)
{
    if ((addAlarm(sysAlarmInfo) != 0) || (sysAlarmSync() != 0) || (saveSysAlarm(gSysAlarmList, gCount) != 0))
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

static int32_t deleteAlarm(uint32_t id)
{
    int32_t  retVal = -1;
    uint32_t lastId = gCount - 1;

    if ((gSysAlarmList == NULL) || (id >= gCount))
    {
        SYSLOG_DEBUG("Param error.\r\n");
        goto labelEnd;
    }

    if (id < lastId)
    {
        memmove(&gSysAlarmList[id], &gSysAlarmList[id + 1], (lastId - id) * sizeof(SysAlarmInfoT));
    }
    memset(&gSysAlarmList[lastId], 0, sizeof(SysAlarmInfoT));
    gCount--;

    retVal = 0;

labelEnd:
    return retVal;
}

int32_t sysAlarmDelete(uint32_t id)
{
    if ((deleteAlarm(id) != 0) || (sysAlarmSync() != 0) || (saveSysAlarm(gSysAlarmList, gCount) != 0))
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

int32_t sysAlarmEdit(uint32_t id, SysAlarmInfoT *sysAlarmInfo)
{
    int32_t retVal = -1;

    if ((gSysAlarmList == NULL) || (id >= gCount) || (sysAlarmInfo == NULL) || (sysAlarmInfo->hour > 23) || (sysAlarmInfo->minute > 59))
    {
        SYSLOG_DEBUG("Param error.\r\n");
        goto labelEnd;
    }

    if ((deleteAlarm(id) != 0) || (addAlarm(sysAlarmInfo) != 0))
    {
        SYSLOG_DEBUG("Failed to edit alarm.\r\n");
        gCount = loadSysAlarm(gSysAlarmList, gCountMax);
        goto labelEnd;
    }

    sysAlarmSync();
    saveSysAlarm(gSysAlarmList, gCount);

    retVal = 0;

labelEnd:
    return retVal;
}

int32_t sysAlarmSync(void)
{
    int32_t    retVal       = -1;
    uint32_t   index        = 0;
    time_t     time         = time_time(NULL);
    struct tm *localTime    = time_localtime(&time);
    uint8_t    wday         = (localTime->tm_wday == 0) ? 6 : (localTime->tm_wday - 1);
    uint32_t   currentTime  = localTime->tm_hour * 3600 + localTime->tm_min * 60 + localTime->tm_sec;
    uint32_t   sysAlarmTime = 0;

    if (gSysAlarmList == NULL)
    {
        SYSLOG_DEBUG("gSysAlarmList is NULL.\r\n");
        goto labelEnd;
    }

    for (index=0; index<gCount; index++)
    {
        if (((gSysAlarmList[index].repeat & (1 << wday)) != 0) && (gSysAlarmList[index].on == true))
        {
            sysAlarmTime = gSysAlarmList[index].hour * 3600 + gSysAlarmList[index].minute * 60;
            if (currentTime < sysAlarmTime)
            {
                retVal = 0;
                goto labelEnd;
            }
        }
    }

    time      += 86400;
    localTime  = time_localtime(&time);
    wday       = (localTime->tm_wday == 0) ? 6 : (localTime->tm_wday - 1);
    for (index=0; index<gCount; index++)
    {
        if (((gSysAlarmList[index].repeat & (1 << wday)) != 0) && (gSysAlarmList[index].on == true))
        {
            sysAlarmTime = gSysAlarmList[index].hour * 3600 + gSysAlarmList[index].minute * 60;
            if (currentTime >= sysAlarmTime)
            {
                sysAlarmTime += 86400;
                retVal = 0;
                goto labelEnd;
            }
        }
    }

    slpManDeepSlpTimerDel(DEEPSLP_TIMER_ID2);
    gIndex = 0xFFFFFFFF;

labelEnd:
    if (retVal == 0)
    {
        slpManDeepSlpTimerStart(DEEPSLP_TIMER_ID2, (sysAlarmTime - currentTime) * 1000);
        gIndex = index;
    }
    if (gIndex == 0xFFFFFFFF)
    {
        retVal = 0;
    }
    return retVal;
}


int32_t sysAlarmSetState(uint32_t id, bool on)
{
    int32_t retVal = -1;

    if ((gSysAlarmList == NULL) || (id >= gCount))
    {
        SYSLOG_DEBUG("Param error.\r\n");
        goto labelEnd;
    }

    gSysAlarmList[id].on = on;
    saveSysAlarm(gSysAlarmList, gCount);

    retVal = 0;

labelEnd:
    return retVal;
}

int32_t sysAlarmGetState(uint32_t id, bool *on)
{
    int32_t retVal = -1;

    if ((gSysAlarmList == NULL) || (id >= gCount) || (on == NULL))
    {
        SYSLOG_DEBUG("Param error.\r\n");
        goto labelEnd;
    }

    *on = gSysAlarmList[id].on;

    retVal = 0;

labelEnd:
    return retVal;
}

uint32_t sysAlarmGetCount(void)
{
    return gCount;
}

SysAlarmInfoT *sysAlarmGetList(void)
{
    return gSysAlarmList;
}
