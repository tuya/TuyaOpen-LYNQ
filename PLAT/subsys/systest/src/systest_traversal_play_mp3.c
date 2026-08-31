/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    test.c
 * Description:  EC618 lwm2m demo entry source file
 * History:      Rev1.0   2018-10-12
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_SYSTEST_TRAVERSAL_PLAY_MP3_ENABLE
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


#define THREAD_STACK_SIZE_SYSTEST_TRAVERSAL_PLAY_MP3      (5 * 1024)
#define QUEUE_SIZE_SYSTEST_TRAVERSAL_PLAY_MP3             50


typedef struct
{
    char *path;
    bool increase;
} mp3ListT;


static osThreadId_t       gSystestTraversalPlayMp3Thread = NULL;
static osMessageQueueId_t gSystestTraversalPlayMp3Queue  = NULL;
static volatile bool      gPlayEnd                       = false;
static mp3ListT           gMp3List[]                     =
{
    {"C:/NwReadyS8kB32kF576.mp3",    false},
    {"C:/NwReady.mp3",               false},
    {"C:/NwReadyS22k05B32kF576.mp3", false},
    {"C:/NwReadyS44k1B32kF576.mp3",  false},
    {"C:/NwReadyS48kB32kF1152.mp3",  false},
    {"C:/NwReadyS8kB32kF576.mp3",    true},
    {"C:/NwReady.mp3",               true}
};


#ifdef FEATURE_SUBSYS_MP3_ENABLE
static void playCallback(int32_t result)
{
    gPlayEnd = true;
}
#endif

static void threadSystestTraversalPlayMp3(void *argument)
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
        if (osMessageQueueGet(gSystestTraversalPlayMp3Queue, &param, 0, osWaitForever) == osOK)
        {
            for (uint32_t i=0; i<sizeof(gMp3List)/sizeof(gMp3List[0]); i++)
            {
                SYSLOG_INFO("Play %s%s\r\n", gMp3List[i].path, (gMp3List[i].increase == true) ? " ->48k" : " ");
#ifdef FEATURE_SUBSYS_MP3_ENABLE
                gPlayEnd = false;
                audioPlayMp3(gMp3List[i].path, playCallback, gMp3List[i].increase, true);
                while (gPlayEnd == false)
                {
                    osDelay(5);
                }
#endif
            }
            systestParamDelete(&param);
        }
    }
}

void systestTraversalPlayMp3(void *param)
{
    osThreadAttr_t threadAttr = {0};

    if (param == NULL)
    {
        systestCaseDelete(&gSystestTraversalPlayMp3Queue, &gSystestTraversalPlayMp3Thread);
    }
    else
    {
        if (gSystestTraversalPlayMp3Queue == NULL)
        {
            gSystestTraversalPlayMp3Queue = osMessageQueueNew(QUEUE_SIZE_SYSTEST_TRAVERSAL_PLAY_MP3, sizeof(SystestParamT), NULL);
            SYSTEST_HANDLE_CHECK(gSystestTraversalPlayMp3Queue);
        }

        if (gSystestTraversalPlayMp3Thread == NULL)
        {
            memset(&threadAttr, 0, sizeof(threadAttr));
            threadAttr.name       = "threadSystestTraversalPlayMp3";
            threadAttr.stack_size = THREAD_STACK_SIZE_SYSTEST_TRAVERSAL_PLAY_MP3;
            threadAttr.priority   = osPriorityNormal;
#if 1 // Service Manager
            gSystestTraversalPlayMp3Thread = osThreadNew(threadSystestTraversalPlayMp3, NULL, &threadAttr);
#else
            char serviceName[32] = {0};
            snprintf(serviceName, sizeof(serviceName), "service:/%s", threadAttr.name);
            Service_reg(serviceName, threadSystestTraversalPlayMp3, NULL, threadAttr.cb_mem, threadAttr.cb_size, threadAttr.stack_mem, threadAttr.stack_size, threadAttr.priority);
            gSystestTraversalPlayMp3Thread = (osThreadId_t)Service_start(serviceName);
#endif
            SYSTEST_HANDLE_CHECK(gSystestTraversalPlayMp3Thread);
        }

        osMessageQueuePut(gSystestTraversalPlayMp3Queue, param, 0, osWaitForever);
    }
}
#endif
