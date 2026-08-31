/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    test.c
 * Description:  EC618 lwm2m demo entry source file
 * History:      Rev1.0   2018-10-12
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_SYSTEST_FLASH_LFS_ENABLE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "cmsis_os2.h"
#include "systest.h"
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#include "servicemanager.h"


#define THREAD_STACK_SIZE_SYSTEST_FLASH_LFS         (5 * 1024)
#define QUEUE_SIZE_SYSTEST_FLASH_LFS                50
#define FILE_PATH                                   "D:/flashLfs"
#define BUFF_SIZE                                   1024
#define INTERNAL_TIMERS                             100


static osThreadId_t       gSystestFlashLfsThread = NULL;
static osMessageQueueId_t gSystestFlashLfsQueue  = NULL;


static void threadSystestFlashLfs(void *argument)
{
    SystestParamT  param                = {0};
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    FILE          *file                 = NULL;
    uint32_t       count                = 0;
    uint32_t       length               = 0;
    uint8_t        buffWrite[BUFF_SIZE] = {0};
    uint8_t        buffRead[BUFF_SIZE]  = {0};
    bool           print                = false;
#endif

    while (1)
    {
        memset(&param, 0, sizeof(param));
        if (osMessageQueueGet(gSystestFlashLfsQueue, &param, 0, osWaitForever) == osOK)
        {
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
            if (spiFlashExist() != true)
            {
                SYSLOG_ERR("No external Flash detected.\r\n");
                goto labelEnd;
            }

            remove(FILE_PATH);
            SYSLOG_INFO("Flash LFS start.\r\n");
            count  = param.param1 / BUFF_SIZE;
            length = param.param1 % BUFF_SIZE;

            for (uint32_t i=0; i<param.param2; i++)
            {
                if ((i != 0) && ((i % 20) == 0))
                {
                    osDelay(2);
                }
                memset(buffWrite, i % 0xFF, BUFF_SIZE);
                print = ((i == 0) || (((i + 1) % INTERNAL_TIMERS) == 0) || ((i + 1) == param.param2)) ? true : false;

                if (print == true)
                {
                    SYSLOG_INFO("\r\nWrite(%d)\r\n", i+1);
                }
                file = file_fopen(FILE_PATH, "w");
                if (file == NULL)
                {
                    SYSLOG_INFO("Failed to open the file \"%s\".\r\n", FILE_PATH);
                    goto labelEnd;
                }
                for (uint32_t j=0; j<count; j++)
                {
                    if ((j != 0) && ((j % 20) == 0))
                    {
                        osDelay(2);
                    }
                    file_fwrite(buffWrite, BUFF_SIZE, 1, file);
                }
                if (length != 0)
                {
                    file_fwrite(buffWrite, length, 1, file);
                }
                file_fclose(file);

                if (print == true)
                {
                    SYSLOG_INFO("Verify\r\n");
                }
                file = file_fopen(FILE_PATH, "r");
                if (file == NULL)
                {
                    SYSLOG_INFO("Failed to open the file \"%s\".\r\n", FILE_PATH);
                    goto labelEnd;
                }
                for (uint32_t j=0; j<count; j++)
                {
                    memset(buffRead, 0, BUFF_SIZE);
                    file_fread(buffRead, BUFF_SIZE, 1, file);
                    if (memcmp(buffWrite, buffRead, BUFF_SIZE) != 0)
                    {
                        SYSLOG_INFO("Failed info: offset = %d, Dump: ", j * BUFF_SIZE);
                        for (uint32_t k=0; k<BUFF_SIZE; k++)
                        {
                            printf("%02X ", buffRead[k]);
                        }
                        printf("\r\n");
                    }
                }
                if (length != 0)
                {
                    memset(buffRead, 0, BUFF_SIZE);
                    file_fread(buffRead, length, 1, file);
                    if (memcmp(buffWrite, buffRead, length) != 0)
                    {
                        SYSLOG_INFO("Failed info: offset = %d, Dump: ", count * BUFF_SIZE);
                        for (uint32_t k=0; k<length; k++)
                        {
                            printf("%02X ", buffRead[k]);
                        }
                        printf("\r\n");
                    }
                }
                file_fclose(file);
                if (print == true)
                {
                    SYSLOG_INFO("End\r\n");
                }
            }

            SYSLOG_INFO("Flash LFS stop.\r\n");
            remove(FILE_PATH);
#endif

            systestParamDelete(&param);
        }
    }

labelEnd:
    systestCaseDelete(&gSystestFlashLfsQueue, &gSystestFlashLfsThread);
}

void systestFlashLfs(void *param)
{
    osThreadAttr_t threadAttr = {0};

    if (param == NULL)
    {
        systestCaseDelete(&gSystestFlashLfsQueue, &gSystestFlashLfsThread);
    }
    else
    {
        if (gSystestFlashLfsQueue == NULL)
        {
            gSystestFlashLfsQueue = osMessageQueueNew(QUEUE_SIZE_SYSTEST_FLASH_LFS, sizeof(SystestParamT), NULL);
            SYSTEST_HANDLE_CHECK(gSystestFlashLfsQueue);
        }

        if (gSystestFlashLfsThread == NULL)
        {
            memset(&threadAttr, 0, sizeof(threadAttr));
            threadAttr.name       = "threadSystestFlashLfs";
            threadAttr.stack_size = THREAD_STACK_SIZE_SYSTEST_FLASH_LFS;
            threadAttr.priority   = osPriorityNormal;
#if 1 // Service Manager
            gSystestFlashLfsThread = osThreadNew(threadSystestFlashLfs, NULL, &threadAttr);
#else
            char serviceName[32] = {0};
            snprintf(serviceName, sizeof(serviceName), "service:/%s", threadAttr.name);
            Service_reg(serviceName, threadSystestFlashLfs, NULL, threadAttr.cb_mem, threadAttr.cb_size, threadAttr.stack_mem, threadAttr.stack_size, threadAttr.priority);
            gSystestFlashLfsThread = (osThreadId_t)Service_start(serviceName);
#endif
            SYSTEST_HANDLE_CHECK(gSystestFlashLfsThread);
        }

        osMessageQueuePut(gSystestFlashLfsQueue, param, 0, osWaitForever);
    }
}

#endif
