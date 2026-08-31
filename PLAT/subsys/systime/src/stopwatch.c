#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "cmsis_os2.h"
#include "ps_lib_api.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#include "stopWatch.h"


static osTimerId_t         gOsTimerId = NULL;
static uint32_t           *gTimeList  = NULL;
static uint32_t            gCountMax  = 0;
static uint32_t            gIndex     = 0;
static uint32_t            gBeginTime = 0;
static uint32_t            gInterval  = 0;
static StopWatchCallbackT  gCallback  = NULL;


static uint32_t getTime(void)
{
    return osKernelGetTickCount();
}

static void osTimerFunc(void *argument)
{
    if (gTimeList == NULL)
    {
        SYSLOG_DEBUG("gTimeList is NULL\r\n");
        return;
    }

    gTimeList[gIndex] = getTime() - gBeginTime;
    if (gCallback != NULL)
    {
        gCallback(gTimeList, gIndex + 1);
    }
}

int32_t stopWatchStart(void)
{
    if (gTimeList == NULL)
    {
        SYSLOG_DEBUG("gTimeList is NULL\r\n");
        return -1;
    }

    if ((gOsTimerId != NULL) && (osTimerIsRunning(gOsTimerId) != 0))
    {
        return -1;
    }

    gBeginTime = getTime() - gTimeList[gIndex];

    return osTimerStart(gOsTimerId, gInterval);
}

int32_t stopWatchStop(void)
{
    if (gTimeList == NULL)
    {
        SYSLOG_DEBUG("gTimeList is NULL\r\n");
        return -1;
    }

    if ((gOsTimerId != NULL) && (osTimerIsRunning(gOsTimerId) != 0))
    {
        return osTimerStop(gOsTimerId);
    }

    return -1;
}

int32_t stopWatchCount(void)
{
    if (gIndex < gCountMax - 1)
    {
        gIndex++;
        return 0;
    }
    else
    {
        SYSLOG_DEBUG("Stop watch MAX count is %d\r\n", gCountMax);
        return -1;
    }
}

int32_t stopWatchReset(void)
{
    if (gTimeList == NULL)
    {
        SYSLOG_DEBUG("gTimeList is NULL\r\n");
        return -1;
    }

    if ((gOsTimerId != NULL) && (osTimerIsRunning(gOsTimerId) != 0))
    {
        osTimerStop(gOsTimerId);
    }

    memset(gTimeList, 0, gCountMax * sizeof(uint32_t));
    gCountMax  = 0;
    gIndex     = 0;
    gBeginTime = 0;
    gInterval  = 0;
    gCallback  = NULL;

    return 0;
}

int32_t stopWatchInit(StopWatchCallbackT callback, uint32_t interval, uint32_t countMax)
{
    int32_t  retVal = -1;
    uint32_t size   = countMax * sizeof(uint32_t);

    if ((callback == NULL) || (interval == 0) || (countMax == 0))
    {
        SYSLOG_DEBUG("Param error.\r\n");
        goto labelEnd;
    }

    if (gOsTimerId == NULL)
    {
        gOsTimerId = osTimerNew(osTimerFunc, osTimerPeriodic, NULL, NULL);
        if (gOsTimerId == NULL)
        {
            SYSLOG_DEBUG("Failed to create timer for gOsTimerId.\r\n");
            goto labelEnd;
        }
    }

    if (gTimeList == NULL)
    {
        gTimeList = (uint32_t *)malloc(size);
        if (gTimeList == NULL)
        {
            SYSLOG_DEBUG("Failed to malloc %d bytes for gTimeList\r\n", size);
            goto labelEnd;
        }
        memset(gTimeList, 0, size);
    }

    stopWatchReset();

    gCallback = callback;
    gInterval = interval;
    gCountMax = countMax;

    retVal = 0;

labelEnd:
    if (retVal != 0)
    {
        if (gOsTimerId != NULL)
        {
            osTimerDelete(gOsTimerId);
            gOsTimerId = NULL;
        }
        if (gTimeList != NULL)
        {
            free(gTimeList);
            gTimeList = NULL;
        }
    }
    return retVal;
}

int32_t stopWatchDeinit(void)
{
    stopWatchReset();

    if (gOsTimerId != NULL)
    {
        osTimerDelete(gOsTimerId);
        gOsTimerId = NULL;
    }

    if (gTimeList != NULL)
    {
        free(gTimeList);
        gTimeList = NULL;
    }

    return 0;
}
