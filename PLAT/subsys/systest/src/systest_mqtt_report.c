/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    test.c
 * Description:  EC618 lwm2m demo entry source file
 * History:      Rev1.0   2018-10-12
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_SYSTEST_MQTT_REPORT_ENABLE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "cmsis_os2.h"
#include "systest.h"
#ifdef FEATURE_SUBSYS_MQTT_ONENET_ENABLE
#include "onenet_mqtt.h"
#endif
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#include "servicemanager.h"


#define THREAD_STACK_SIZE_SYSTEST_MQTT_REPORT       (5 * 1024)
#define QUEUE_SIZE_SYSTEST_MQTT_REPORT              50
#define SMALL_INTERNAL_TIMES                        10
#define DURATION_S                                  86400
#define REPORT_FORMAT                               "{\"%s\":{\"name\":\"%s\",\"param1\":0,\"param2\":0,\"param3\":\"%s\"}}"


static osThreadId_t       gSystestMqttReportThread = NULL;
static osMessageQueueId_t gSystestMqttReportQueue  = NULL;
static volatile bool      gTimeout                 = false;


static void s_timer_callback(void *argument)
{
    gTimeout = true;
}

static void threadSystestMqttReport(void *argument)
{
    SystestParamT param       = {0};
    osTimerId_t   timer       = NULL;
    char          buffer[256] = {0};

#if defined(FEATURE_SUBSYS_MQTT_ONENET_ENABLE)
    while (onenetMqttIsReady() != true)
#else
    SYSLOG_ERR("No MQTT cloud platform defined.\r\n");
    goto labelEnd;
#endif
    {
        SYSLOG_INFO("Wait for connection to MQTT.\r\n");
        osDelay(1000);
    }

    timer = osTimerNew(s_timer_callback, osTimerOnce, NULL, NULL);
    if (timer == NULL)
    {
        SYSLOG_ERR("Failed to create timer for timer.\r\n");
        goto labelEnd;
    }

    while (1)
    {
        memset(&param, 0, sizeof(param));
        if (osMessageQueueGet(gSystestMqttReportQueue, &param, 0, osWaitForever) == osOK)
        {
#ifdef FEATURE_SUBSYS_MQTT_ONENET_ENABLE
            onenetMqttUnsub(ONENET_MQTT_TOPIC_SPEAKER);

            if ((memcmp(param.param3+1, ":/", strlen(":/")) == 0) && (strstr(param.param3, ".mp3") != NULL))
            {
                snprintf(buffer, sizeof(buffer), REPORT_FORMAT, ONENET_MQTT_TOPIC_SPEAKER, "systest_play_mp3", param.param3);
            }
            else
            {
                snprintf(buffer, sizeof(buffer), REPORT_FORMAT, ONENET_MQTT_TOPIC_SPEAKER, "systestPlayTts", param.param3);
            }
#endif
            osTimerStart(timer, DURATION_S * osKernelGetTickFreq());
            SYSLOG_INFO("Begin\r\n");
            gTimeout = false;
            while (gTimeout != true)
            {
                for (uint32_t i=0; i<SMALL_INTERNAL_TIMES; i++)
                {
                    if ((param.param1 > 0) && (i > 0))
                    {
                        osDelay(param.param1 * 1000);
                    }

#if defined(FEATURE_SUBSYS_MQTT_ONENET_ENABLE)
                    onenetMqttPub(buffer);
#endif
                    SYSLOG_INFO("Send: %s\r\n", buffer);
                }

                if (param.param2 > 0)
                {
                    osDelay(param.param2 * 1000);
                }
            }
            SYSLOG_INFO("End\r\n");
            systestParamDelete(&param);
#ifdef FEATURE_SUBSYS_MQTT_ONENET_ENABLE
            onenetMqttSub(ONENET_MQTT_TOPIC_SPEAKER);
#endif
        }
    }

labelEnd:
    systestCaseDelete(&gSystestMqttReportQueue, &gSystestMqttReportThread);
}

void systestMqttReport(void *param)
{
    osThreadAttr_t threadAttr = {0};

    if (param == NULL)
    {
        systestCaseDelete(&gSystestMqttReportQueue, &gSystestMqttReportThread);
    }
    else
    {
        if (gSystestMqttReportQueue == NULL)
        {
            gSystestMqttReportQueue = osMessageQueueNew(QUEUE_SIZE_SYSTEST_MQTT_REPORT, sizeof(SystestParamT), NULL);
            SYSTEST_HANDLE_CHECK(gSystestMqttReportQueue);
        }

        if (gSystestMqttReportThread == NULL)
        {
            memset(&threadAttr, 0, sizeof(threadAttr));
            threadAttr.name       = "threadSystestMqttReport";
            threadAttr.stack_size = THREAD_STACK_SIZE_SYSTEST_MQTT_REPORT;
            threadAttr.priority   = osPriorityNormal;
#if 1 // Service Manager
            gSystestMqttReportThread = osThreadNew(threadSystestMqttReport, NULL, &threadAttr);
#else
            char serviceName[32] = {0};
            snprintf(serviceName, sizeof(serviceName), "service:/%s", threadAttr.name);
            Service_reg(serviceName, threadSystestMqttReport, NULL, threadAttr.cb_mem, threadAttr.cb_size, threadAttr.stack_mem, threadAttr.stack_size, threadAttr.priority);
            gSystestMqttReportThread = (osThreadId_t)Service_start(serviceName);
#endif
            SYSTEST_HANDLE_CHECK(gSystestMqttReportThread);
        }

        osMessageQueuePut(gSystestMqttReportQueue, param, 0, osWaitForever);
    }
}
#endif
