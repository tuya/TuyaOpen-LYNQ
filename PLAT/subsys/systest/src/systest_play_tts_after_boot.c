/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    test.c
 * Description:  EC618 lwm2m demo entry source file
 * History:      Rev1.0   2018-10-12
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_SYSTEST_PLAY_TTS_AFTER_BOOT_ENABLE
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
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#include "servicemanager.h"


#define THREAD_STACK_SIZE_SYSTEST_PLAY_TTS_AFTER_BOOT       (5 * 1024)
#define QUEUE_SIZE_SYSTEST_PLAY_TTS_AFTER_BOOT              50
#define SYSTEST_PLAY_TTS_AFTER_BOOT_FILE                    "C:/systestPlayTtsAfterBoot"


static osThreadId_t       gSystestPlayTtsAfterBootThread = NULL;
static osMessageQueueId_t gSystestPlayTtsAfterBootQueue  = NULL;
static volatile bool      gPlayEnd                       = false;


#ifdef FEATURE_SUBSYS_TTS_ENABLE
static void playCallback(int32_t result)
{
    gPlayEnd = true;
}
#endif

static void threadSystestPlayTtsAfterBoot(void *argument)
{
    SystestParamT  param  = {0};
    FILE          *file   = file;
    struct stat    buf    = {0};
    uint32_t       size   = 0;
    char          *buffer = NULL;

#ifdef FEATURE_SUBSYS_AUDIO_ENABLE
    while (audioIsReady() != true)
    {
        osDelay(5);
    }
#endif

    while (1)
    {
        memset(&param, 0, sizeof(param));
        if (osMessageQueueGet(gSystestPlayTtsAfterBootQueue, &param, 0, osWaitForever) == osOK)
        {
            if (param.param3 != NULL)
            {
                if ((strlen(param.param3) == strlen(SYSTEST_BOOT_START)) && (memcmp(param.param3, SYSTEST_BOOT_START, strlen(SYSTEST_BOOT_START)) == 0))
                {
                    file = file_fopen(SYSTEST_PLAY_TTS_AFTER_BOOT_FILE, "r");
                    if (file != NULL)
                    {
                        file_fstat((int)file, &buf);
                        size = buf.st_size;
                        if (size <= sizeof(uint32_t))
                        {
                            SYSLOG_ERR("The size of the file %s is less than or equal to %d.\r\n", SYSTEST_PLAY_TTS_AFTER_BOOT_FILE, sizeof(uint32_t));
                        }
                        else
                        {
                            buffer = malloc(size + 1);
                            if (buffer == NULL)
                            {
                                SYSLOG_ERR("Failed to malloc %d bytes for buffer.\r\n", size + 1);
                            }
                            else
                            {
                                memset(buffer, 0, size + 1);
                                file_fread(buffer, size, 1, file);
                                for (uint32_t i=0; i<*((uint32_t *)(buffer)); i++)
                                {
                                    SYSLOG_INFO("Play %s\r\n", buffer + sizeof(uint32_t));
#ifdef FEATURE_SUBSYS_TTS_ENABLE
                                    gPlayEnd = false;
                                    audioPlayTts(buffer + sizeof(uint32_t), playCallback);
                                    while (gPlayEnd == false)
                                    {
                                        osDelay(5);
                                    }
#endif
                                }
                                free(buffer);
                                buffer = NULL;
                            }
                        }
                        file_fclose(file);
                        remove(SYSTEST_PLAY_TTS_AFTER_BOOT_FILE);
                    }
                    systestCaseDelete(&gSystestPlayTtsAfterBootQueue, &gSystestPlayTtsAfterBootThread);
                }
                else
                {
                    file = file_fopen(SYSTEST_PLAY_TTS_AFTER_BOOT_FILE, "w");
                    if (file == NULL)
                    {
                        SYSLOG_ERR("Failed to create file %s.\r\n", SYSTEST_PLAY_TTS_AFTER_BOOT_FILE);
                    }
                    else
                    {
                        file_fwrite((void *)(&(param.param2)), sizeof(param.param2), 1, file);
                        file_fwrite((void *)(param.param3), strlen(param.param3), 1, file);
                        file_fclose(file);
                    }

                    systestParamDelete(&param);
                }
            }
        }
    }
}

void systestPlayTtsAfterBoot(void *param)
{
    osThreadAttr_t threadAttr = {0};

    if (param == NULL)
    {
        systestCaseDelete(&gSystestPlayTtsAfterBootQueue, &gSystestPlayTtsAfterBootThread);
    }
    else
    {
        if (gSystestPlayTtsAfterBootQueue == NULL)
        {
            gSystestPlayTtsAfterBootQueue = osMessageQueueNew(QUEUE_SIZE_SYSTEST_PLAY_TTS_AFTER_BOOT, sizeof(SystestParamT), NULL);
            SYSTEST_HANDLE_CHECK(gSystestPlayTtsAfterBootQueue);
        }

        if (gSystestPlayTtsAfterBootThread == NULL)
        {
            memset(&threadAttr, 0, sizeof(threadAttr));
            threadAttr.name       = "threadSystestPlayTtsAfterBoot";
            threadAttr.stack_size = THREAD_STACK_SIZE_SYSTEST_PLAY_TTS_AFTER_BOOT;
            threadAttr.priority   = osPriorityNormal;
#if 1 // Service Manager
            gSystestPlayTtsAfterBootThread = osThreadNew(threadSystestPlayTtsAfterBoot, NULL, &threadAttr);
#else
            char serviceName[32] = {0};
            snprintf(serviceName, sizeof(serviceName), "service:/%s", threadAttr.name);
            Service_reg(serviceName, threadSystestPlayTtsAfterBoot, NULL, threadAttr.cb_mem, threadAttr.cb_size, threadAttr.stack_mem, threadAttr.stack_size, threadAttr.priority);
            gSystestPlayTtsAfterBootThread = (osThreadId_t)Service_start(serviceName);
#endif
            SYSTEST_HANDLE_CHECK(gSystestPlayTtsAfterBootThread);
        }

        osMessageQueuePut(gSystestPlayTtsAfterBootQueue, param, 0, osWaitForever);
    }
}
#endif
