/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    test.c
 * Description:  EC618 lwm2m demo entry source file
 * History:      Rev1.0   2018-10-12
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_SYSTEST_ENABLE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "cmsis_os2.h"
#include "systest.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif


#define CASE_NAME_LEN_MAX                           100
#define SYSTEST_CASE(testCase, bootStart)           \
do {                                                \
    extern void testCase(void *param);              \
    systestRegister(#testCase, testCase, bootStart);\
} while (0)


typedef void (*CaseHandlerT)(void *param);
typedef struct CaseListS
{
    struct CaseListS *next;
    char             name[CASE_NAME_LEN_MAX + 1];
    bool             running;
    bool             bootStart;
    CaseHandlerT     handler;
} CaseListT;


static CaseListT *gCmdListHead = NULL;
static CaseListT *gCmdListTail = NULL;


static void systestBootStart(void)
{
    CaseListT     *testCase = gCmdListHead;
    SystestParamT param     = {.param3 = SYSTEST_BOOT_START};

    while (testCase != NULL)
    {
        if (testCase->bootStart == true)
        {
            testCase->handler(&param);
            testCase->running = true;
        }
        testCase = testCase->next;
    }
}

static int32_t systestRegister(char *name, CaseHandlerT handler, bool bootStart)
{
    int32_t   retVal    = -1;
    CaseListT *testCase = NULL;

    if ((name == NULL) || (handler == NULL))
    {
        SYSLOG_EMERG("%s is NULL.\r\n", (name == NULL) ? "name" : "handler");
        goto labelEnd;
    }

    testCase = malloc(sizeof(CaseListT));
    if (testCase == NULL)
    {
        SYSLOG_EMERG("Failed to malloc %d bytes for testCase.\r\n", sizeof(CaseListT));
        goto labelEnd;
    }

    memset(testCase, 0, sizeof(CaseListT));
    memcpy(testCase->name, name, strlen(name));
    testCase->handler   = handler;
    testCase->bootStart = bootStart;

    if ((gCmdListHead == NULL) || (gCmdListTail == NULL))
    {
        gCmdListHead = testCase;
        gCmdListTail = testCase;
    }
    else
    {
        gCmdListTail->next = testCase;
        gCmdListTail       = testCase;
    }

    retVal = 0;

labelEnd:
    return retVal;
}

void systestInit(void)
{
    static bool firstBoot = true;

    if (gCmdListHead != NULL)
    {
        return;
    }

    SYSTEST_CASE(systestExit, false);
#ifdef FEATURE_SUBSYS_SYSTEST_PLAY_MP3_ENABLE
    SYSTEST_CASE(systestPlayMp3, false);
#endif
#ifdef FEATURE_SUBSYS_SYSTEST_PLAY_WAV_ENABLE
    SYSTEST_CASE(systestPlayWav, false);
#endif
#ifdef FEATURE_SUBSYS_SYSTEST_PLAY_TTS_ENABLE
    SYSTEST_CASE(systestPlayTts, false);
#endif
#ifdef FEATURE_SUBSYS_SYSTEST_PLAY_PCM_ENABLE
    SYSTEST_CASE(systestPlayPcm, false);
#endif
#ifdef FEATURE_SUBSYS_SYSTEST_CHANGE_VOLUME_ENABLE
    SYSTEST_CASE(systestChangeVolume, false);
#endif
#ifdef FEATURE_SUBSYS_SYSTEST_TRAVERSAL_PLAY_MP3_ENABLE
    SYSTEST_CASE(systestTraversalPlayMp3, false);
#endif
#ifdef FEATURE_SUBSYS_SYSTEST_TRAVERSAL_PLAY_WAV_ENABLE
    SYSTEST_CASE(systestTraversalPlayWav, false);
#endif
#ifdef FEATURE_SUBSYS_SYSTEST_ACCESS_FILE_ENABLE
    SYSTEST_CASE(systestAccessFile, false);
#endif
#ifdef FEATURE_SUBSYS_SYSTEST_PLAY_MP3_AFTER_BOOT_ENABLE
    SYSTEST_CASE(systestPlayMp3AfterBoot, true);
#endif
#ifdef FEATURE_SUBSYS_SYSTEST_PLAY_TTS_AFTER_BOOT_ENABLE
    SYSTEST_CASE(systestPlayTtsAfterBoot, true);
#endif
#ifdef FEATURE_SUBSYS_SYSTEST_PLAY_MP3_AND_REPORT_ENABLE
    SYSTEST_CASE(systestPlayMp3AndReport, false);
#endif
#ifdef FEATURE_SUBSYS_SYSTEST_PLAY_TTS_AND_REPORT_ENABLE
    SYSTEST_CASE(systestPlayRtsAndReport, false);
#endif
#ifdef FEATURE_SUBSYS_SYSTEST_PLAY_WAV_AND_REPORT_ENABLE
    SYSTEST_CASE(systestPlayWavAndReport, false);
#endif
#ifdef FEATURE_SUBSYS_SYSTEST_MQTT_REPORT_ENABLE
    SYSTEST_CASE(systestMqttReport, false);
#endif
#ifdef FEATURE_SUBSYS_SYSTEST_FLASH_LFS_ENABLE
    SYSTEST_CASE(systestFlashLfs, false);
#endif
#ifdef FEATURE_SUBSYS_SYSTEST_FLASH_PERFORMANCE_ENABLE
    SYSTEST_CASE(systestFlashPerformance, false);
#endif

    if (firstBoot == true)
    {
        firstBoot = false;
        systestBootStart();
    }
}

int32_t systestProcess(char *name, SystestParamT *param)
{
    int32_t   retVal    = -1;
    CaseListT *testCase = gCmdListHead;

    while (testCase != NULL)
    {
        if ((strlen(testCase->name) == strlen(name))
         && (memcmp(testCase->name, name, strlen(name)) == 0))
        {
            SYSLOG_INFO("%s: %d, %d, %s\r\n", name, param->param1, param->param2, (param->param3 != NULL) ? param->param3 : "NULL");
            testCase->handler((void *)param);
            testCase->running = true;
            retVal = 0;
            break;
        }
        testCase = testCase->next;
    }

    return retVal;
}

void systestParamDelete(SystestParamT *param)
{
    if ((param != NULL) && (param->param3 != NULL))
    {
        free(param->param3);
        param->param3 = NULL;
    }
}

void systestExit(void *param)
{
    CaseListT *testCase = NULL;

    testCase = gCmdListHead;
    while (testCase != NULL)
    {
        if (testCase->running == true)
        {
            SYSLOG_INFO("Terminate %s\r\n", testCase->name);
            testCase->handler(NULL);
            testCase->running = false;
        }
        testCase = testCase->next;

        free(gCmdListHead);
        gCmdListHead = testCase;
    }
}

void systestCaseDelete(osMessageQueueId_t *queue, osThreadId_t *thread)
{
    if ((queue != NULL) && (*queue != NULL))
    {
        osMessageQueueDelete(*queue);
        *queue = NULL;
    }

    if ((thread != NULL) && (*thread != NULL))
    {
        if (osThreadGetId() != *thread)
        {
            osThreadTerminate(*thread);
            *thread = NULL;
        }
        else
        {
            *thread = NULL;
            osThreadExit();
        }
    }
}
#endif
