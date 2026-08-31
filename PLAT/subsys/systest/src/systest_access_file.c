/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    test.c
 * Description:  EC618 lwm2m demo entry source file
 * History:      Rev1.0   2018-10-12
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_SYSTEST_ACCESS_FILE_ENABLE
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
#ifdef FEATURE_SUBSYS_MQTT_ONENET_ENABLE
#include "onenet_mqtt.h"
#endif
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "iniparse.h"
#endif
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#include "servicemanager.h"


#define THREAD_STACK_SIZE_SYSTEST_ACCESS_FILE       (5 * 1024)
#define QUEUE_SIZE_SYSTEST_ACCESS_FILE              50
#define ACCESS_ONENET_MQTT_DEV_INFO                 255


typedef enum
{
    ACCESS_TYPE_WRITE = 0,
    ACCESS_TYPE_READ  = 1,
    ACCESS_TYPE_INVALID
} AccessTypeT;


static osThreadId_t       gSystestAccessFileThread = NULL;
static osMessageQueueId_t gSystestAccessFileQueue  = NULL;


#ifdef FEATURE_SUBSYS_MQTT_ONENET_ENABLE
static void *extractStringSplitByComma(char *string, uint32_t index)
{
    char     *posPre  = string;
    char     *posNext = string;
    char     *buffer  = NULL;
    uint32_t length   = 0;

    if (string == NULL)
    {
        SYSLOG_ERR("string is NULL.\r\n");
        goto labelEnd;
    }

    if (index == 0)
    {
        SYSLOG_ERR("\"index\" must be greater than 0.\r\n");
        goto labelEnd;
    }

    for (uint32_t i=0; i<index; i++)
    {
        posNext = strstr(posPre, ",");
        if (posNext == NULL)
        {
            length = strlen(posPre);
        }
        else
        {
            length = posNext - posPre;
        }

        if ((i + 1) == index)
        {
            buffer = malloc(length + 1);
            if (buffer == NULL)
            {
                SYSLOG_ERR("Failed to malloc %d bytes for buffer.\r\n", length + 1);
                goto labelEnd;
            }

            memset(buffer, 0,      length + 1);
            memcpy(buffer, posPre, length);
            goto labelEnd;
        }

        posPre = posNext + 1;
        if ((posNext == NULL) && (posPre == NULL))
        {
            SYSLOG_ERR("Too few parameters.\r\n");
            goto labelEnd;
        }
    }

labelEnd:
    return buffer;
}

static int32_t writeOnenetMqttDevInfo(char *param3)
{
    int32_t            retVal  = -1;
    OnenetMqttDevInfoT devInfo = {0};
    void               *value = NULL;

    memset(&devInfo, 0, sizeof(devInfo));
    for (uint32_t i=0; i<3; i++)
    {
        value = extractStringSplitByComma(param3, i + 1);
        if (value == NULL)
        {
            SYSLOG_ERR("Failed to extract OneNET MQTT device info.\r\n");
            goto labelEnd;
        }
        else
        {
            if (strlen((char *)value) > ITEM_LEN_MAX)
            {
                SYSLOG_ERR("The parameter is too long.\r\n");
                goto labelEnd;
            }
            else
            {
                memcpy(((char *)(&devInfo)) + (i * (ITEM_LEN_MAX + 1)), value, strlen((char *)value));
            }

            free(value);
            value = NULL;
        }
    }

    onenetMqttDevInfoWrite(&devInfo);
    retVal = 0;

labelEnd:
    if (value != NULL)
    {
        free(value);
        value = NULL;
    }
    return retVal;
}
#endif

static void threadSystestAccessFile(void *argument)
{
    SystestParamT param  = {0};
    void          *value = NULL;

    while (1)
    {
        value  = NULL;
        memset(&param, 0, sizeof(param));
        if (osMessageQueueGet(gSystestAccessFileQueue, &param, 0, osWaitForever) == osOK)
        {
            if (param.param3 != NULL)
            {
                switch (param.param1)
                {
                    case ACCESS_TYPE_WRITE:
                        if (param.param2 == ACCESS_ONENET_MQTT_DEV_INFO)
                        {
#ifdef FEATURE_SUBSYS_MQTT_ONENET_ENABLE
                            if (writeOnenetMqttDevInfo(param.param3) != 0)
#endif
                            {
                                SYSLOG_ERR("Failed to write OneNET MQTT device info.\r\n", param.param3);
                            }
                        }
                        else
                        {
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
                            if (iniKeyValueWrite(DEFAULT_INFO, param.param3, INI_VALUE_INT, (void *)(&(param.param2))) != 0)
#endif
                            {
                                SYSLOG_ERR("Failed to write the value of %s\r\n", param.param3);
                            }
                        }
                        break;

                    case ACCESS_TYPE_READ:
                        if (param.param2 == ACCESS_ONENET_MQTT_DEV_INFO)
                        {
#ifdef FEATURE_SUBSYS_MQTT_ONENET_ENABLE
                            value = onenetMqttDevInfoRead(DEV_INFO_RETURN_STRING);
#endif
                            if (value == NULL)
                            {
                                SYSLOG_ERR("Failed to read OneNET MQTT device info.\r\n");
                            }
                            else
                            {
                                SYSLOG_ERR("OneNET MQTT device info: %s\r\n", (char *)value);
                                free(value);
                                value = NULL;
                            }
                        }
                        else
                        {
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
                            value = iniKeyValueRead(DEFAULT_INFO, param.param3, INI_VALUE_INT);
#endif
                            if (value == NULL)
                            {
                                SYSLOG_ERR("Failed to read the value of %s\r\n", param.param3);
                            }
                            else
                            {
                                SYSLOG_ERR("%s=%d\r\n", param.param3, *((int32_t *)value));
                                free(value);
                                value = NULL;
                            }
                        }
                        break;

                    default:
                        SYSLOG_ERR("Unsupported access type: %d\r\n", param.param1);
                        break;
                }
            }
            else
            {
                SYSLOG_ERR("Param error.\r\n");
            }
            systestParamDelete(&param);
        }
    }
}

void systestAccessFile(void *param)
{
    osThreadAttr_t threadAttr = {0};

    if (param == NULL)
    {
        systestCaseDelete(&gSystestAccessFileQueue, &gSystestAccessFileThread);
    }
    else
    {
        if (gSystestAccessFileQueue == NULL)
        {
            gSystestAccessFileQueue = osMessageQueueNew(QUEUE_SIZE_SYSTEST_ACCESS_FILE, sizeof(SystestParamT), NULL);
            SYSTEST_HANDLE_CHECK(gSystestAccessFileQueue);
        }

        if (gSystestAccessFileThread == NULL)
        {
            memset(&threadAttr, 0, sizeof(threadAttr));
            threadAttr.name       = "threadSystestAccessFile";
            threadAttr.stack_size = THREAD_STACK_SIZE_SYSTEST_ACCESS_FILE;
            threadAttr.priority   = osPriorityNormal;
#if 1 // Service Manager
            gSystestAccessFileThread = osThreadNew(threadSystestAccessFile, NULL, &threadAttr);
#else
            char serviceName[32] = {0};
            snprintf(serviceName, sizeof(serviceName), "service:/%s", threadAttr.name);
            Service_reg(serviceName, threadSystestAccessFile, NULL, threadAttr.cb_mem, threadAttr.cb_size, threadAttr.stack_mem, threadAttr.stack_size, threadAttr.priority);
            gSystestAccessFileThread = (osThreadId_t)Service_start(serviceName);
#endif
            SYSTEST_HANDLE_CHECK(gSystestAccessFileThread);
        }

        osMessageQueuePut(gSystestAccessFileQueue, param, 0, osWaitForever);
    }
}
#endif
