/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    test.c
 * Description:  EC618 lwm2m demo entry source file
 * History:      Rev1.0   2018-10-12
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_SYSTEST_PLAY_MP3_ENABLE
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


#define THREAD_STACK_SIZE_SYSTEST_PLAY_MP3      (5 * 1024)
#define QUEUE_SIZE_SYSTEST_PLAY_MP3             50


static osThreadId_t       gSystestPlayMp3Thread = NULL;
static osMessageQueueId_t gSystestPlayMp3Queue  = NULL;
static volatile bool      gPlayEnd              = false;


#ifdef FEATURE_SUBSYS_MP3_ENABLE
static void playCallback(int32_t result)
{
    gPlayEnd = true;
}
#endif

static void threadSystestPlayMp3(void *argument)
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
        if (osMessageQueueGet(gSystestPlayMp3Queue, &param, 0, osWaitForever) == osOK)
        {
            if (param.param3 != NULL)
            {
                SYSLOG_INFO("Play %s\r\n", param.param3);
#ifdef FEATURE_SUBSYS_MP3_ENABLE
                gPlayEnd = false;
                audioPlayMp3(param.param3, playCallback, true, true);
                while (gPlayEnd == false)
                {
                    osDelay(5);
                }
#endif
            }
            else
            {
                SYSLOG_ERR("NO MP3 name.\r\n");
            }
            systestParamDelete(&param);
        }
    }
}

void systestPlayMp3(void *param)
{
    osThreadAttr_t threadAttr = {0};

    if (param == NULL)
    {
        systestCaseDelete(&gSystestPlayMp3Queue, &gSystestPlayMp3Thread);
    }
    else
    {
        if (gSystestPlayMp3Queue == NULL)
        {
            gSystestPlayMp3Queue = osMessageQueueNew(QUEUE_SIZE_SYSTEST_PLAY_MP3, sizeof(SystestParamT), NULL);
            SYSTEST_HANDLE_CHECK(gSystestPlayMp3Queue);
        }

        if (gSystestPlayMp3Thread == NULL)
        {
            memset(&threadAttr, 0, sizeof(threadAttr));
            threadAttr.name       = "threadSystestPlayMp3";
            threadAttr.stack_size = THREAD_STACK_SIZE_SYSTEST_PLAY_MP3;
            threadAttr.priority   = osPriorityNormal;
#if 1 // Service Manager
            gSystestPlayMp3Thread = osThreadNew(threadSystestPlayMp3, NULL, &threadAttr);
#else
            char serviceName[32] = {0};
            snprintf(serviceName, sizeof(serviceName), "service:/%s", threadAttr.name);
            Service_reg(serviceName, threadSystestPlayMp3, NULL, threadAttr.cb_mem, threadAttr.cb_size, threadAttr.stack_mem, threadAttr.stack_size, threadAttr.priority);
            gSystestPlayMp3Thread = (osThreadId_t)Service_start(serviceName);
#endif
            SYSTEST_HANDLE_CHECK(gSystestPlayMp3Thread);
        }

        osMessageQueuePut(gSystestPlayMp3Queue, param, 0, osWaitForever);
    }
}
#endif
