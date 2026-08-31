#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "ccio_opaq.h"
#include "RTE_Device.h"
#include "syslog.h"
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "iniparse.h"
#endif


#define INI_KEY_SYSLOG_LEVEL        "syslogLevel"
#define INI_KEY_SYSLOG_CHANNEL      "syslogChannel"


#if (PSRAM_EXIST == 1)
static PLAT_FPSRAM_ZI_CUST char gBuffer[1024]  = {0};
#else
static char                     gBuffer[1024]  = {0};
#endif
static osMutexId_t              gSyslogMutex   = NULL;
static uint32_t                 gSyslogChannel = 0;
SyslogLevelT                    gSyslogLevel   = SL_NO;


int32_t syslogPrintf(char *fmt, ...)
{
    int32_t       retVal = -1;
#if (RTE_OPAQ_EN == 1)
    DlPduBlock_t *dlpdu  = NULL;
    uint32_t      length = 0;
#endif

    if (fmt == NULL)
    {
        goto labelEnd;
    }

    if(gSyslogMutex == NULL)
    {
        gSyslogMutex = osMutexNew(NULL);
        if(gSyslogMutex == NULL)
        {
            SYSLOG_EMERG("Failed to create mutex for gSyslogMutex\r\n");
            goto labelEnd;
        }
    }

    osMutexAcquire(gSyslogMutex, osWaitForever);

    memset(gBuffer, 0, sizeof(gBuffer));

    va_list args;
    va_start(args, fmt);
    vsnprintf(gBuffer, sizeof(gBuffer), fmt, args);
    va_end(args);

    if ((gSyslogChannel & SC_UART0) != 0)
    {
    }

    if ((gSyslogChannel & SC_UART1) != 0)
    {
        printf("%s", gBuffer);
    }

    if ((gSyslogChannel & SC_UART2) != 0)
    {
    }

    if ((gSyslogChannel & SC_UART3) != 0)
    {
    }
#if (RTE_OPAQ_EN == 1)
    if ((gSyslogChannel & SC_USB) != 0)
    {
        length = strlen(gBuffer);

        dlpdu = (DlPduBlock_t *)OsaDlfcAllocDlPduNonBlocking(length);
        if(dlpdu != NULL)
        {
            memcpy(dlpdu->pPdu, gBuffer, length);
            dlpdu->length = length;

            opaqDataOutputEx(OPAQ_CHAN_CUST1, dlpdu, NULL);
        }
    }
#endif
    if ((gSyslogChannel & SC_EPAT) != 0)
    {
    }

    if ((gSyslogChannel & SC_LCD) != 0)
    {
    }

    if ((gSyslogChannel & SC_FILE) != 0)
    {
    }

    retVal = 0;

    osMutexRelease(gSyslogMutex);

labelEnd:
    return retVal;
}

static int32_t syslogWriteConfig(char *key, int32_t value)
{
    int32_t  retVal    = -1;
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    int32_t *valueRead = NULL;
#endif

    if (key == NULL)
    {
        SYSLOG_DEBUG("Param error.\r\n");
        goto labelEnd;
    }

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    valueRead = iniKeyValueRead(DEFAULT_INFO, key, INI_VALUE_INT);
    if ((valueRead == NULL) || (*valueRead != value))
    {
        retVal = iniKeyValueWrite(DEFAULT_INFO, key, INI_VALUE_INT, (void *)&value);
    }

    if (valueRead != NULL)
    {
        free(valueRead);
        valueRead = NULL;
    }
#endif

labelEnd:
    return retVal;
}

static int32_t syslogReadConfig(char *key, int32_t *value)
{
    int32_t  retVal    = -1;
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    int32_t *valueRead = NULL;
#endif

    if ((key == NULL) || (value == NULL))
    {
        SYSLOG_DEBUG("Param error.\r\n");
        goto labelEnd;
    }

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    valueRead = iniKeyValueRead(DEFAULT_INFO, key, INI_VALUE_INT);
    if (valueRead != NULL)
    {
        *value = *valueRead;
        free(valueRead);
        valueRead = NULL;
        retVal = 0;
    }
#endif

labelEnd:
    return retVal;
}


void syslogSetLevel(uint32_t level)
{
    SYSLOG_DEBUG("gSyslogLevel=%d, level=%d\r\n", gSyslogLevel, level);

    gSyslogLevel = level;
    syslogWriteConfig(INI_KEY_SYSLOG_LEVEL, level);
}

uint32_t syslogGetLevel(void)
{
    uint32_t level = 0;

    syslogReadConfig(INI_KEY_SYSLOG_LEVEL, (int32_t *)&level);
    SYSLOG_DEBUG("gSyslogLevel=%d, level=%d\r\n", gSyslogLevel, level);
    gSyslogLevel = level;

    return level;
}

void syslogSetChannel(uint32_t channel)
{
    SYSLOG_DEBUG("gSyslogChannel=%d, channel=%d\r\n", gSyslogChannel, channel);

    gSyslogChannel = channel;
    syslogWriteConfig(INI_KEY_SYSLOG_CHANNEL, channel);
}

uint32_t syslogGetChannel(void)
{
    uint32_t channel = 0;

    syslogReadConfig(INI_KEY_SYSLOG_CHANNEL, (int32_t *)&channel);
    SYSLOG_DEBUG("gSyslogChannel=%d, channel=%d\r\n", gSyslogChannel, channel);
    gSyslogChannel = channel;

    return channel;
}
