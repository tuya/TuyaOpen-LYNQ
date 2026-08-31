/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    test.c
 * Description:  EC618 lwm2m demo entry source file
 * History:      Rev1.0   2018-10-12
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include "cmsis_os2.h"
#include "ps_lib_api.h"
#include "ostask.h"
#include "osasys.h"
#include "bsp.h"
#include "bsp_custom.h"
#include "mode.h"
#include "subsys.h"
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#include "iniparse.h"
#endif
#ifdef FEATURE_SUBSYS_CMDPARSE_ENABLE
#include "cmdparse.h"
#endif
#ifdef FEATURE_SUBSYS_SYSTEST_ENABLE
#include "systest.h"
#endif
#ifdef FEATURE_SUBSYS_CONSOLE_ENABLE
#include "console.h"
#endif
#ifdef FEATURE_SUBSYS_SENSORHUB_ENABLE
#include "sensorhub.h"
#endif
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#include "servicemanager.h"


extern void normalModeMenu(void);
extern void testModeMenu(void);
extern void poweroffModeMenu(void);


#define THREAD_STACK_SIZE_MODE              (2 * 1024)
#define INI_KEY_MODE                        "mode"
#define INI_KEY_DEBUG                       "debug"


#ifdef THREAD_STATIC
static StaticTask_t gModeThreadCbMem                            = {0};
static uint8_t      gModeThreadStackMem[THREAD_STACK_SIZE_MODE] = {0};
#endif
static uint8_t gMode      = 0;
static uint8_t gDebug     = 0;
static uint8_t gDebugType = 0;


static uint8_t modeRead(void)
{
    int32_t  mode  = gMode;
    int32_t *value = NULL;

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    value = iniKeyValueRead(DEFAULT_INFO, INI_KEY_MODE, INI_VALUE_INT);
#endif
    if (value != NULL)
    {
        mode = *value;
        free(value);
        value = NULL;
    }
    else
    {
        iniKeyValueWrite(DEFAULT_INFO, INI_KEY_MODE, INI_VALUE_INT, (void *)&mode);
    }

    return (uint8_t)mode;
}

uint8_t debugRead(void)
{
    int32_t  debug = gDebug;
    int32_t *value = NULL;

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    value = iniKeyValueRead(DEFAULT_INFO, INI_KEY_DEBUG, INI_VALUE_INT);
#endif
    if (value != NULL)
    {
        debug = *value;
        free(value);
        value = NULL;
    }
    else
    {
        iniKeyValueWrite(DEFAULT_INFO, INI_KEY_DEBUG, INI_VALUE_INT, (void *)&debug);
    }

    return (uint8_t)debug;
}

static void threadMode(void *argument)
{
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
	subStorageInit();
#endif

    gMode  = modeRead();
    gDebug = debugRead();

#ifdef FEATURE_SUBSYS_CMDPARSE_ENABLE
    gDebugType = 1;
#elif FEATURE_SUBSYS_CONSOLE_ENABLE
    gDebugType = 2;
#endif

#if (defined FEATURE_SUBSYS_CMDPARSE_ENABLE) && (defined FEATURE_SUBSYS_CONSOLE_ENABLE)
    gDebugType = 3;
#endif
    SYSLOG_INFO("Work  mode: %d\r\n", gMode);
    SYSLOG_INFO("Debug mode: %d\r\n", gDebug);
    SYSLOG_INFO("Debug type: %d\r\n", gDebugType);
#ifdef FEATURE_SUBSYS_CMDPARSE_ENABLE
    if(gDebug == DEBUG_JSON)
    {
        cmdparseInit();
#ifdef FEATURE_SUBSYS_SYSTEST_ENABLE
        systestInit();
#endif
    }
    else
    {
#ifdef FEATURE_SUBSYS_CONSOLE_ENABLE
        consoleTaskInit();
#else
        cmdparseInit();
#ifdef FEATURE_SUBSYS_SYSTEST_ENABLE
        systestInit();
#endif
#endif
    }
#elif FEATURE_SUBSYS_CONSOLE_ENABLE
    consoleTaskInit();  
#endif

    switch(gMode)
    {
        case 0:
            gMode = MODE_NORMAL;
            normalModeMenu();
            break;
#ifdef FEATURE_SUBSYS_TEST_MODE_ENABLE
        case 1:
            gMode = MODE_TEST;
            testModeMenu();
            break;
#endif
#ifdef FEATURE_SUBSYS_POWEROFF_MODE_ENABLE
        case 2:
            gMode = MODE_POWEROFF;
            poweroffModeMenu();
            break;
#endif
        default:
            gMode = MODE_NORMAL;
            normalModeMenu();
            break;
    }

#if 0
    osThreadExit();
#else
    osThreadSetPriority(osThreadGetId(), osPriorityLow);
    Service_stop("service:/threadMode");
#endif
}

void modeSelect(void)
{
    osThreadAttr_t threadAttr = {0};

    memset(&threadAttr, 0, sizeof(threadAttr));
    threadAttr.name       = "threadMode";
    threadAttr.stack_size = THREAD_STACK_SIZE_MODE;
    threadAttr.priority   = osPriorityNormal;
#ifdef THREAD_STATIC
    threadAttr.stack_mem  = gModeThreadStackMem;
    threadAttr.cb_mem     = &gModeThreadCbMem;
    threadAttr.cb_size    = sizeof(StaticTask_t);
#endif
#if 0
    osThreadNew(threadMode, NULL, &threadAttr);
#else
    char serviceName[32] = {0};
    snprintf(serviceName, sizeof(serviceName), "service:/%s", threadAttr.name);
    Service_reg(serviceName, threadMode, NULL, threadAttr.cb_mem, threadAttr.cb_size, threadAttr.stack_mem, threadAttr.stack_size, threadAttr.priority);
    Service_start(serviceName);
#endif
}

int32_t modeSave(uint8_t mode)
{
    int32_t retVal = 0;
    int32_t data   = mode;

    if (modeRead() != mode)
    {
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
        retVal = iniKeyValueWrite(DEFAULT_INFO, INI_KEY_MODE, INI_VALUE_INT, (void *)&data);
#endif
        if (retVal == 0)
        {
            gMode = mode;
        }
    }

    return retVal;
}

uint8_t modeGet(void)
{
    return gMode;
}

void debugSet(uint8_t debug)
{
    gDebug = debug;
}

uint8_t debugGet(void)
{
    return gDebug;
}

uint8_t debugTypeGet(void)
{
    return gDebugType;
}
