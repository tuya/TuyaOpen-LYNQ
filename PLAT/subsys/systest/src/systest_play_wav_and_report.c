/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    test.c
 * Description:  EC618 lwm2m demo entry source file
 * History:      Rev1.0   2018-10-12
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_SYSTEST_PLAY_WAV_AND_REPORT_ENABLE
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
#include "servicemanager.h"


#define THREAD_STACK_SIZE_SYSTEST_PLAY_WAV_AND_REPORT      (5 * 1024)
#define QUEUE_SIZE_SYSTEST_PLAY_WAV_AND_REPORT             50


static osThreadId_t       gSystestPlayWavAndReportThread = NULL;
static osMessageQueueId_t gSystestPlayWavAndReportQueue  = NULL;
static volatile bool      gPlayEnd                       = false;


#ifdef FEATURE_SUBSYS_WAV_ENABLE
static void playCallback(int32_t result)
{
    gPlayEnd = true;
}
#endif

static void threadSystestPlayWavAndReport(void *argument)
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
        if (osMessageQueueGet(gSystestPlayWavAndReportQueue, &param, 0, osWaitForever) == osOK)
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
#ifdef FEATURE_SUBSYS_WAV_ENABLE
                    gPlayEnd = false;
                    audioPlayWav(param.param3, playCallback);
                    while (gPlayEnd == false)
                    {
                        osDelay(5);
                    }
#endif
                }
            }
            else
            {
                SYSLOG_ERR("NO WAV name.\r\n");
            }
            systestParamDelete(&param);
        }
    }
}

void systestPlayWavAndReport(void *param)
{
    osThreadAttr_t threadAttr = {0};

    if (param == NULL)
    {
        systestCaseDelete(&gSystestPlayWavAndReportQueue, &gSystestPlayWavAndReportThread);
    }
    else
    {
        if (gSystestPlayWavAndReportQueue == NULL)
        {
            gSystestPlayWavAndReportQueue = osMessageQueueNew(QUEUE_SIZE_SYSTEST_PLAY_WAV_AND_REPORT, sizeof(SystestParamT), NULL);
            SYSTEST_HANDLE_CHECK(gSystestPlayWavAndReportQueue);
        }

        if (gSystestPlayWavAndReportThread == NULL)
        {
            memset(&threadAttr, 0, sizeof(threadAttr));
            threadAttr.name       = "threadSystestPlayWavAndReport";
            threadAttr.stack_size = THREAD_STACK_SIZE_SYSTEST_PLAY_WAV_AND_REPORT;
            threadAttr.priority   = osPriorityNormal;
#if 1 // Service Manager
            gSystestPlayWavAndReportThread = osThreadNew(threadSystestPlayWavAndReport, NULL, &threadAttr);
#else
            char serviceName[32] = {0};
            snprintf(serviceName, sizeof(serviceName), "service:/%s", threadAttr.name);
            Service_reg(serviceName, threadSystestPlayWavAndReport, NULL, threadAttr.cb_mem, threadAttr.cb_size, threadAttr.stack_mem, threadAttr.stack_size, threadAttr.priority);
            gSystestPlayWavAndReportThread = (osThreadId_t)Service_start(serviceName);
#endif
            SYSTEST_HANDLE_CHECK(gSystestPlayWavAndReportThread);
        }

        osMessageQueuePut(gSystestPlayWavAndReportQueue, param, 0, osWaitForever);
    }
}
#endif
