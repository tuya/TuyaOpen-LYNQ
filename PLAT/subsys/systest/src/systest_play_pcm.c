/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    test.c
 * Description:  EC618 lwm2m demo entry source file
 * History:      Rev1.0   2018-10-12
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_SYSTEST_PLAY_PCM_ENABLE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "cmsis_os2.h"
#include "systest.h"
#ifdef FEATURE_SUBSYS_AUDIO_ENABLE
#include "audio.h"
#ifdef FEATURE_SUBSYS_PCM_ENABLE
#include "pcm.h"
#endif
#endif
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#include "servicemanager.h"


#define THREAD_STACK_SIZE_SYSTEST_PLAY_PCM      (5 * 1024)
#define QUEUE_SIZE_SYSTEST_PLAY_PCM             50

#define PCM_PARAM_00H                           "00h"
#define PCM_PARAM_SIN                           "sin"
#define PCM_PARAM_END                           "end"

static osThreadId_t       gSystestPlayPcmThread = NULL;
static osMessageQueueId_t gSystestPlayPcmQueue  = NULL;
static volatile bool      gPlayEnd              = false;


static void threadSystestPlayPcm(void *argument)
{
    SystestParamT param     = {0};
#ifdef FEATURE_SUBSYS_PCM_ENABLE
    int16_t       *dataSin  = NULL;
    uint32_t      lengthSin = 0;
    uint8_t       data00h   = 0;
#endif

#ifdef FEATURE_SUBSYS_AUDIO_ENABLE
    while (audioIsReady() != true)
    {
        osDelay(5);
    }
#endif

    while (1)
    {
        memset(&param, 0, sizeof(param));
        if (osMessageQueueGet(gSystestPlayPcmQueue, &param, 0, osWaitForever) == osOK)
        {
            if (param.param3 != NULL)
            {
                if ((strlen(param.param3) == strlen(PCM_PARAM_00H)) && (memcmp(param.param3, PCM_PARAM_00H, strlen(PCM_PARAM_00H)) == 0))
                {
                    SYSLOG_INFO("Play 00h.\r\n");
#ifdef FEATURE_SUBSYS_PCM_ENABLE
                    audioPlayPcm(&data00h, sizeof(data00h), param.param2);
#endif
                }
                else if ((strlen(param.param3) == strlen(PCM_PARAM_SIN)) && (memcmp(param.param3, PCM_PARAM_SIN, strlen(PCM_PARAM_SIN)) == 0))
                {
                    SYSLOG_INFO("Play sin.\r\n");
#ifdef FEATURE_SUBSYS_PCM_ENABLE
                    lengthSin = pcmSinGet(param.param1, param.param2, &dataSin);
                    if (dataSin != NULL)
                    {
                        audioPlayPcm((uint8_t *)dataSin, lengthSin, param.param2);
                        free(dataSin);
                        dataSin = NULL;
                    }
#endif
                }
                else if ((strlen(param.param3) == strlen(PCM_PARAM_END)) && (memcmp(param.param3, PCM_PARAM_END, strlen(PCM_PARAM_END)) == 0))
                {
                    SYSLOG_INFO("End of play.\r\n");
#ifdef FEATURE_SUBSYS_PCM_ENABLE
                    pcmEndPlay();
#endif
                }
                else
                {
                    SYSLOG_ERR("The value of param.param3 error.\r\n");
                }
            }
            else
            {
                SYSLOG_ERR("param.param3 is NULL.\r\n");
            }
            systestParamDelete(&param);
        }
    }
}

void systestPlayPcm(void *param)
{
    osThreadAttr_t threadAttr = {0};

    if (param == NULL)
    {
        systestCaseDelete(&gSystestPlayPcmQueue, &gSystestPlayPcmThread);
    }
    else
    {
        if (gSystestPlayPcmQueue == NULL)
        {
            gSystestPlayPcmQueue = osMessageQueueNew(QUEUE_SIZE_SYSTEST_PLAY_PCM, sizeof(SystestParamT), NULL);
            SYSTEST_HANDLE_CHECK(gSystestPlayPcmQueue);
        }

        if (gSystestPlayPcmThread == NULL)
        {
            memset(&threadAttr, 0, sizeof(threadAttr));
            threadAttr.name       = "threadSystestPlayPcm";
            threadAttr.stack_size = THREAD_STACK_SIZE_SYSTEST_PLAY_PCM;
            threadAttr.priority   = osPriorityNormal;
#if 1 // Service Manager
            gSystestPlayPcmThread = osThreadNew(threadSystestPlayPcm, NULL, &threadAttr);
#else
            char serviceName[32] = {0};
            snprintf(serviceName, sizeof(serviceName), "service:/%s", threadAttr.name);
            Service_reg(serviceName, threadSystestPlayPcm, NULL, threadAttr.cb_mem, threadAttr.cb_size, threadAttr.stack_mem, threadAttr.stack_size, threadAttr.priority);
            gSystestPlayPcmThread = (osThreadId_t)Service_start(serviceName);
#endif
            SYSTEST_HANDLE_CHECK(gSystestPlayPcmThread);
        }

        osMessageQueuePut(gSystestPlayPcmQueue, param, 0, osWaitForever);
    }
}
#endif
