/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    test.c
 * Description:  EC618 lwm2m demo entry source file
 * History:      Rev1.0   2018-10-12
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_SYSTEST_FLASH_PERFORMANCE_ENABLE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "cmsis_os2.h"
#include "slpman.h"
#include "systest.h"
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#include "servicemanager.h"


#define THREAD_STACK_SIZE_SYSTEST_FLASH_PERFORMANCE (5 * 1024)
#define QUEUE_SIZE_SYSTEST_FLASH_PERFORMANCE        50
#define INTERNAL_TIMERS                             100


static osThreadId_t       gSystestFlashPerformanceThread = NULL;
static osMessageQueueId_t gSystestFlashPerformanceQueue  = NULL;


static void threadSystestFlashPerformance(void *argument)
{
    SystestParamT param                  = {0};
    uint32_t      index                  = 0;
    uint32_t      countErase             = 0;
    uint32_t      countWriteRead         = 0;
    uint32_t      length                 = 0;
    uint8_t       bufferWrite[PAGE_SIZE] = {0};
    uint8_t       bufferRead[PAGE_SIZE]  = {0};
    uint32_t      times                  = 0;
    uint32_t      timeEraseBegin         = 0;
    uint32_t      timeEraseEnd           = 0;
    uint32_t      timeWriteBegin         = 0;
    uint32_t      timeWriteEnd           = 0;
    uint32_t      timeVerifyBegin        = 0;
    uint32_t      timeVerifyEnd          = 0;

    slpManNormalIOVoltSet(IOVOLT_3_30V);
    SYSLOG_INFO("voltage: IOVOLT_3_30V\r\n");
    osDelay(1000);

    while (1)
    {
        memset(&param, 0, sizeof(param));
        if (osMessageQueueGet(gSystestFlashPerformanceQueue, &param, 0, osWaitForever) == osOK)
        {
            if (spiFlashExist() != true)
            {
                SYSLOG_ERR("No external Flash detected.\r\n");
                goto labelEnd;
            }

            SYSLOG_INFO("Flash performance test begin.\r\n");
            times          = atoi(param.param3);
            countErase     = (param.param2 / SECTOR_SIZE) + (((param.param2 % SECTOR_SIZE) == 0) ? 0 : 1);
            countWriteRead = param.param2 / PAGE_SIZE;
            length         = param.param2 % PAGE_SIZE;
            for (uint32_t i=0; i<times; i++)
            {
                memset(bufferWrite, i % 0xFF, PAGE_SIZE);

                timeEraseBegin = osKernelGetTickCount();
                for (index=0; index<countErase; index++)
                {
                    spiFlashEraseSector(param.param1 + index * SECTOR_SIZE);
                }
                timeEraseEnd = osKernelGetTickCount();

                timeWriteBegin = timeEraseEnd;
                for (index=0; index<countWriteRead; index++)
                {
                    spiFlashWritePage(param.param1 + index * PAGE_SIZE, bufferWrite, PAGE_SIZE);
                }
                if (length > 0)
                {
                    spiFlashWritePage(param.param1 + index * PAGE_SIZE, bufferWrite, length);
                }
                timeWriteEnd = osKernelGetTickCount();

                timeVerifyBegin = timeWriteEnd;
                for (index=0; index<countWriteRead; index++)
                {
                    memset(bufferRead, 0, PAGE_SIZE);
                    spiFlashRead(param.param1 + index * PAGE_SIZE, bufferRead, PAGE_SIZE);
                    if (memcmp(bufferWrite, bufferRead, PAGE_SIZE) != 0)
                    {
                        SYSLOG_INFO("Failed info: address = 0x%X. Dump: ", param.param1 + index * PAGE_SIZE);
                        for (uint32_t j=0; j<PAGE_SIZE; j++)
                        {
                            printf("%02X ", bufferRead[j]);
                        }
                        printf("\r\n");
                        goto labelEnd;
                    }
                    if (length > 0)
                    {
                        spiFlashRead(param.param1 + index * PAGE_SIZE, bufferRead, length);
                        if (memcmp(bufferWrite, bufferRead, length) != 0)
                        {
                            SYSLOG_INFO("Failed info: address = 0x%X. Dump: ", param.param1 + index * PAGE_SIZE);
                            for (uint32_t j=0; j<length; j++)
                            {
                                printf("%02X ", bufferRead[j]);
                            }
                            printf("\r\n");
                            goto labelEnd;
                        }
                    }
                }
                timeVerifyEnd = osKernelGetTickCount();

                printf("T=%d\r\nEB=%d, EE=%d, ED=%d\r\nWB=%d, WE=%d, WD=%d\r\nVB=%d, VE=%d, VD=%d\r\n\r\n", i+1,
                       timeEraseBegin,  timeEraseEnd,  timeEraseEnd  - timeEraseBegin,
                       timeWriteBegin,  timeWriteEnd,  timeWriteEnd  - timeWriteBegin,
                       timeVerifyBegin, timeVerifyEnd, timeVerifyEnd - timeVerifyBegin);
            }

            SYSLOG_INFO("Flash performance test end.\r\n");
        }
    }

labelEnd:
    systestCaseDelete(&gSystestFlashPerformanceQueue, &gSystestFlashPerformanceThread);
}

void systestFlashPerformance(void *param)
{
    osThreadAttr_t threadAttr = {0};

    if (param == NULL)
    {
        systestCaseDelete(&gSystestFlashPerformanceQueue, &gSystestFlashPerformanceThread);
    }
    else
    {
        if (gSystestFlashPerformanceQueue == NULL)
        {
            gSystestFlashPerformanceQueue = osMessageQueueNew(QUEUE_SIZE_SYSTEST_FLASH_PERFORMANCE, sizeof(SystestParamT), NULL);
            SYSTEST_HANDLE_CHECK(gSystestFlashPerformanceQueue);
        }

        if (gSystestFlashPerformanceThread == NULL)
        {
            memset(&threadAttr, 0, sizeof(threadAttr));
            threadAttr.name       = "threadSystestFlashPerformance";
            threadAttr.stack_size = THREAD_STACK_SIZE_SYSTEST_FLASH_PERFORMANCE;
            threadAttr.priority   = osPriorityNormal;
#if 1 // Service Manager
            gSystestFlashPerformanceThread = osThreadNew(threadSystestFlashPerformance, NULL, &threadAttr);
#else
            char serviceName[32] = {0};
            snprintf(serviceName, sizeof(serviceName), "service:/%s", threadAttr.name);
            Service_reg(serviceName, threadSystestFlashPerformance, NULL, threadAttr.cb_mem, threadAttr.cb_size, threadAttr.stack_mem, threadAttr.stack_size, threadAttr.priority);
            gSystestFlashPerformanceThread = (osThreadId_t)Service_start(serviceName);
#endif
            SYSTEST_HANDLE_CHECK(gSystestFlashPerformanceThread);
        }

        osMessageQueuePut(gSystestFlashPerformanceQueue, param, 0, osWaitForever);
    }
}

#endif
