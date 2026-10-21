/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    test.c
 * Description:  EC618 lwm2m demo entry source file
 * History:      Rev1.0   2018-10-12
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_SYSTEST_PLAY_TTS_ENABLE
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


#define THREAD_STACK_SIZE_SYSTEST_PLAY_TTS      (5 * 1024)
#define QUEUE_SIZE_SYSTEST_PLAY_TTS             50


static osThreadId_t       gSystestPlayTtsThread = NULL;
static osMessageQueueId_t gSystestPlayTtsQueue  = NULL;
static volatile bool      gPlayEnd              = false;


#ifdef FEATURE_SUBSYS_TTS_ENABLE
static void playCallback(void)
{
    gPlayEnd = true;
}
#endif

static void threadSystestPlayTts(void *argument)
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
        if (osMessageQueueGet(gSystestPlayTtsQueue, &param, 0, osWaitForever) == osOK)
        {
            if (param.param3 != NULL)
            {
                SYSLOG_INFO("Play %s\r\n", param.param3);
#ifdef FEATURE_SUBSYS_TTS_ENABLE
                gPlayEnd = false;
                audioPlayTts(param.param3, playCallback);
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

void systestPlayTts(void *param)
{
    osThreadAttr_t threadAttr = {0};

    if (param == NULL)
    {
        systestCaseDelete(&gSystestPlayTtsQueue, &gSystestPlayTtsThread);
    }
    else
    {
        if (gSystestPlayTtsQueue == NULL)
        {
            gSystestPlayTtsQueue = osMessageQueueNew(QUEUE_SIZE_SYSTEST_PLAY_TTS, sizeof(SystestParamT), NULL);
            SYSTEST_HANDLE_CHECK(gSystestPlayTtsQueue);
        }

        if (gSystestPlayTtsThread == NULL)
        {
            memset(&threadAttr, 0, sizeof(threadAttr));
            threadAttr.name       = "threadSystestPlayTts";
            threadAttr.stack_size = THREAD_STACK_SIZE_SYSTEST_PLAY_TTS;
            threadAttr.priority   = osPriorityNormal;
            gSystestPlayTtsThread = osThreadNew(threadSystestPlayTts, NULL, &threadAttr);
            SYSTEST_HANDLE_CHECK(gSystestPlayTtsThread);
        }

        osMessageQueuePut(gSystestPlayTtsQueue, param, 0, osWaitForever);
    }
}
#endif
