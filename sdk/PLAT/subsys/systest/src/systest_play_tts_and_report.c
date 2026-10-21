/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    test.c
 * Description:  EC618 lwm2m demo entry source file
 * History:      Rev1.0   2018-10-12
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_SYSTEST_PLAY_TTS_AND_REPORT_ENABLE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "cmsis_os2.h"
#include "systest.h"
#include "status.h"
#ifdef FEATURE_SUBSYS_AUDIO_ENABLE
#include "audio.h"
#endif
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif


#define THREAD_STACK_SIZE_SYSTEST_PLAY_TTS_AND_REPORT      (5 * 1024)
#define QUEUE_SIZE_SYSTEST_PLAY_TTS_AND_REPORT             50


static osThreadId_t       gSystestPlayTtsAndReportThread = NULL;
static osMessageQueueId_t gSystestPlayTtsAndReportQueue  = NULL;
static volatile bool      gPlayEnd                       = false;


#ifdef FEATURE_SUBSYS_TTS_ENABLE
static void playCallback(void)
{
    gPlayEnd = true;
}
#endif

static void threadSystestPlayTtsAndReport(void *argument)
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
        if (osMessageQueueGet(gSystestPlayTtsAndReportQueue, &param, 0, osWaitForever) == osOK)
        {
            if (param.param3 != NULL)
            {
                if (param.param1 > 0)
                {
                    statusUpdateReportInternal(param.param1);
                }
                for (uint32_t i=0; i<param.param2; i++)
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
            }
            else
            {
                SYSLOG_ERR("NO TTS text.\r\n");
            }
            systestParamDelete(&param);
        }
    }
}

void systestPlayRtsAndReport(void *param)
{
    osThreadAttr_t threadAttr = {0};

    if (param == NULL)
    {
        systestCaseDelete(&gSystestPlayTtsAndReportQueue, &gSystestPlayTtsAndReportThread);
    }
    else
    {
        if (gSystestPlayTtsAndReportQueue == NULL)
        {
            gSystestPlayTtsAndReportQueue = osMessageQueueNew(QUEUE_SIZE_SYSTEST_PLAY_TTS_AND_REPORT, sizeof(SystestParamT), NULL);
            SYSTEST_HANDLE_CHECK(gSystestPlayTtsAndReportQueue);
        }

        if (gSystestPlayTtsAndReportThread == NULL)
        {
            memset(&threadAttr, 0, sizeof(threadAttr));
            threadAttr.name       = "threadSystestPlayTtsAndReport";
            threadAttr.stack_size = THREAD_STACK_SIZE_SYSTEST_PLAY_TTS_AND_REPORT;
            threadAttr.priority   = osPriorityNormal;
            gSystestPlayTtsAndReportThread = osThreadNew(threadSystestPlayTtsAndReport, NULL, &threadAttr);
            SYSTEST_HANDLE_CHECK(gSystestPlayTtsAndReportThread);
        }

        osMessageQueuePut(gSystestPlayTtsAndReportQueue, param, 0, osWaitForever);
    }
}
#endif
