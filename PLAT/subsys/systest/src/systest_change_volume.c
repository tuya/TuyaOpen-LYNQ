/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    test.c
 * Description:  EC618 lwm2m demo entry source file
 * History:      Rev1.0   2018-10-12
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_SYSTEST_CHANGE_VOLUME_ENABLE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "cmsis_os2.h"
#include "systest.h"
#ifdef FEATURE_SUBSYS_AUDIO_ENABLE
#include "audio.h"
#endif
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#include "servicemanager.h"


#define THREAD_STACK_SIZE_SYSTEST_CHANGE_VOLUME     (5 * 1024)
#define QUEUE_SIZE_SYSTEST_CHANGE_VOLUME            50


static osThreadId_t       gSystestChangeVolumeThread = NULL;
static osMessageQueueId_t gSystestChangeVolumeQueue  = NULL;
static volatile bool      gPlayEnd                   = false;


static void vPressProcess(char *vPress)
{
#ifdef FEATURE_SUBSYS_AUDIO_ENABLE
#ifdef SPEAKER_APP
    if ((strlen(vPress) == strlen(VOLUME_PLUS_VPRESS_SHORT))
     && (memcmp(vPress, VOLUME_PLUS_VPRESS_SHORT, strlen(VOLUME_PLUS_VPRESS_SHORT)) == 0))
    {
        SYSLOG_INFO("Volume +\r\n");
        audioAdjustVolume(ACTION_VOLUME_PLUS_SHORT);
    }
    else if ((strlen(vPress) == strlen(VOLUME_PLUS_VPRESS_LONG))
          && (memcmp(vPress, VOLUME_PLUS_VPRESS_LONG, strlen(VOLUME_PLUS_VPRESS_LONG)) == 0))
    {
        SYSLOG_INFO("Volume ++\r\n");
        audioAdjustVolume(ACTION_VOLUME_PLUS_LONG);
    }
    else if ((strlen(vPress) == strlen(VOLUME_MINUS_VPRESS_SHORT))
          && (memcmp(vPress, VOLUME_MINUS_VPRESS_SHORT, strlen(VOLUME_MINUS_VPRESS_SHORT)) == 0))
    {
        SYSLOG_INFO("Volume -\r\n");
        audioAdjustVolume(ACTION_VOLUME_MINUS_SHORT);
    }
    else if ((strlen(vPress) == strlen(VOLUME_MINUS_VPRESS_LONG))
          && (memcmp(vPress, VOLUME_MINUS_VPRESS_LONG, strlen(VOLUME_MINUS_VPRESS_LONG)) == 0))
    {
        SYSLOG_INFO("Volume --\r\n");
        audioAdjustVolume(ACTION_VOLUME_MINUS_LONG);
    }
    else
    {
        SYSLOG_ERR("Unsupported virtual volume.\r\n");
    }
#endif
#endif
}

static void threadSystestChangeVolume(void *argument)
{
    SystestParamT param = {0};

#ifdef FEATURE_SUBSYS_AUDIO_ENABLE
    while (audioIsReady() != true)
    {
        osDelay(5);
    }
#endif

    while (1)
    {
        memset(&param, 0, sizeof(param));
        if (osMessageQueueGet(gSystestChangeVolumeQueue, &param, 0, osWaitForever) == osOK)
        {
            if (param.param3 != NULL)
            {
                vPressProcess(param.param3);
            }
            else
            {
                SYSLOG_ERR("NO volume data.\r\n");
            }
            systestParamDelete(&param);
        }
    }
}

void systestChangeVolume(void *param)
{
    osThreadAttr_t threadAttr = {0};

    if (param == NULL)
    {
        systestCaseDelete(&gSystestChangeVolumeQueue, &gSystestChangeVolumeThread);
    }
    else
    {
        if (gSystestChangeVolumeQueue == NULL)
        {
            gSystestChangeVolumeQueue = osMessageQueueNew(QUEUE_SIZE_SYSTEST_CHANGE_VOLUME, sizeof(SystestParamT), NULL);
            SYSTEST_HANDLE_CHECK(gSystestChangeVolumeQueue);
        }

        if (gSystestChangeVolumeThread == NULL)
        {
            memset(&threadAttr, 0, sizeof(threadAttr));
            threadAttr.name       = "threadSystestChangeVolume";
            threadAttr.stack_size = THREAD_STACK_SIZE_SYSTEST_CHANGE_VOLUME;
            threadAttr.priority   = osPriorityNormal;
#if 1 // Service Manager
            gSystestChangeVolumeThread = osThreadNew(threadSystestChangeVolume, NULL, &threadAttr);
#else
            char serviceName[32] = {0};
            snprintf(serviceName, sizeof(serviceName), "service:/%s", threadAttr.name);
            Service_reg(serviceName, threadSystestChangeVolume, NULL, threadAttr.cb_mem, threadAttr.cb_size, threadAttr.stack_mem, threadAttr.stack_size, threadAttr.priority);
            gSystestChangeVolumeThread = (osThreadId_t)Service_start(serviceName);
#endif
            SYSTEST_HANDLE_CHECK(gSystestChangeVolumeThread);
        }

        osMessageQueuePut(gSystestChangeVolumeQueue, param, 0, osWaitForever);
    }
}
#endif
