/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    test.c
 * Description:  EC618 lwm2m demo entry source file
 * History:      Rev1.0   2018-10-12
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_SYSTEST_TRAVERSAL_PLAY_WAV_ENABLE
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


#define THREAD_STACK_SIZE_SYSTEST_TRAVERSAL_PLAY_WAV      (5 * 1024)
#define QUEUE_SIZE_SYSTEST_TRAVERSAL_PLAY_WAV             50


static osThreadId_t       gSystestTraversalPlayWavThread = NULL;
static osMessageQueueId_t gSystestTraversalPlayWavQueue  = NULL;
static volatile bool      gPlayEnd                       = false;
static char               *gWavList[]                    =
{
    "C:/NwReadyS8kB128k.wav",
    "C:/NwReady.wav",
    "C:/NwReadyS22k05B352k8.wav",
    "C:/NwReadyS44k1B705k6.wav",
    "C:/NwReadyS48kB768k.wav"
};


#ifdef FEATURE_SUBSYS_WAV_ENABLE
static void playCallback(void)
{
    gPlayEnd = true;
}
#endif

static void threadSystestTraversalPlayWav(void *argument)
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
        if (osMessageQueueGet(gSystestTraversalPlayWavQueue, &param, 0, osWaitForever) == osOK)
        {
            for (uint32_t i=0; i<sizeof(gWavList)/sizeof(gWavList[0]); i++)
            {
                SYSLOG_INFO("Play %s\r\n", gWavList[i]);
#ifdef FEATURE_SUBSYS_WAV_ENABLE
                gPlayEnd = false;
                audioPlayWav(gWavList[i], playCallback);
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

void systestTraversalPlayWav(void *param)
{
    osThreadAttr_t threadAttr = {0};

    if (param == NULL)
    {
        systestCaseDelete(&gSystestTraversalPlayWavQueue, &gSystestTraversalPlayWavThread);
    }
    else
    {
        if (gSystestTraversalPlayWavQueue == NULL)
        {
            gSystestTraversalPlayWavQueue = osMessageQueueNew(QUEUE_SIZE_SYSTEST_TRAVERSAL_PLAY_WAV, sizeof(SystestParamT), NULL);
            SYSTEST_HANDLE_CHECK(gSystestTraversalPlayWavQueue);
        }

        if (gSystestTraversalPlayWavThread == NULL)
        {
            memset(&threadAttr, 0, sizeof(threadAttr));
            threadAttr.name       = "threadSystestTraversalPlayWav";
            threadAttr.stack_size = THREAD_STACK_SIZE_SYSTEST_TRAVERSAL_PLAY_WAV;
            threadAttr.priority   = osPriorityNormal;
            gSystestTraversalPlayWavThread = osThreadNew(threadSystestTraversalPlayWav, NULL, &threadAttr);
            SYSTEST_HANDLE_CHECK(gSystestTraversalPlayWavThread);
        }

        osMessageQueuePut(gSystestTraversalPlayWavQueue, param, 0, osWaitForever);
    }
}
#endif
