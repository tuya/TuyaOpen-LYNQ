#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "codecDrv.h"
#include "mw_nvm_audio.h"
#include "at_api.h"
#include "ccio_audio.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#include "volumeManager.h"
#include "servicemanager.h"

#define VOLUME_MANAGER_FILE                         "D:/volumeManager.ini"
#define VOLUME_NAME_LEN_MAX                         32
#define THREAD_STACK_SIZE_VOLUME_MANAGER            (5 * 1024)
#define QUEUE_SIZE_VOLUME_MANAGER                   20


typedef enum
{
    VOLUME_MANAGER_GET = 0,
    VOLUME_MANAGER_SET,
    VOLUME_MANAGER_MUTE,
    VOLUME_MANAGER_UNMUTE,
    VOLUME_MANAGER_IS_MUTE,
    VOLUME_MANAGER_ACTIVATE,
} VolumeManagerControlT;

typedef struct
{
    char    name[VOLUME_NAME_LEN_MAX + 1];
    uint8_t volume;
    uint8_t type;
} QueueVolumeManagerT;


#ifdef THREAD_STATIC
static StaticTask_t       gVolumeManagerThreadCbMem                                      = {0};
static uint8_t            gVolumeManagerThreadStackMem[THREAD_STACK_SIZE_VOLUME_MANAGER] = {0};
#endif
static osThreadId_t       gVolumeManagerThread        = NULL;
static osMessageQueueId_t gVolumeManagerRequestQueue  = NULL;
static osMessageQueueId_t gVolumeManagerResponseQueue = NULL;
static osMutexId_t        gVolumeManagerMutex         = NULL;
static char               gCurrentVolumeName[VOLUME_NAME_LEN_MAX + 1] = {0};
static volatile int8_t    gClvlValue[AUDIO_CFG_DEVICE_MAX]            = {0};


static int32_t sendMessageQueue(char *name, uint8_t volume, uint8_t type)
{
    int32_t             retVal = -1;
    QueueVolumeManagerT queue  = {.volume = volume, .type = type};

    if (gVolumeManagerRequestQueue == NULL)
    {
        if (osIsInISRContext() != true)
        {
            SYSLOG_DEBUG("gVolumeManagerRequestQueue is NULL.\r\n");
        }
        goto labelEnd;
    }

    if ((name == NULL) || (strlen(name) > VOLUME_NAME_LEN_MAX))
    {
        if (osIsInISRContext() != true)
        {
            SYSLOG_DEBUG("param error.\r\n");
        }
        goto labelEnd;
    }

    if (osIsInISRContext() != true)
    {
        SYSLOG_DEBUG("name=%s, volume=%d, type=%d\r\n", name, volume, type);
    }
    memset(queue.name, 0, sizeof(queue.name));
    memcpy(queue.name, name, strlen(name));
    retVal = osMessageQueuePut(gVolumeManagerRequestQueue, &queue, 0, (osIsInISRContext() == true) ? 0 : osWaitForever);

labelEnd:
    return retVal;
}

static int32_t sendMessageQueueAndWaitResponse(char *name, uint8_t *response, uint8_t type)
{
    int32_t retVal = -1;

    if (gVolumeManagerMutex == NULL)
    {
        SYSLOG_DEBUG("gVolumeManagerMutex is NULL.\r\n");
        goto labelEnd;
    }

    osMutexAcquire(gVolumeManagerMutex, osWaitForever);

    retVal = sendMessageQueue(name, 0, type);
    if (retVal == 0)
    {
        retVal = osMessageQueueGet(gVolumeManagerResponseQueue, response, 0, osWaitForever);
    }

    osMutexRelease(gVolumeManagerMutex);

labelEnd:
    return retVal;
}

int32_t volumeManagerGet(char *name, uint8_t *volume)
{
    return sendMessageQueueAndWaitResponse(name, volume, VOLUME_MANAGER_GET);
}

int32_t volumeManagerSet(char *name, uint8_t volume)
{
    return sendMessageQueue(name, volume, VOLUME_MANAGER_SET);
}

int32_t volumeManagerMute(char *name)
{
    return sendMessageQueue(name, 0, VOLUME_MANAGER_MUTE);
}

int32_t volumeManagerUnmute(char *name)
{
    return sendMessageQueue(name, 0, VOLUME_MANAGER_UNMUTE);
}

int32_t volumeManagerIsMute(char *name, bool *isMute)
{
    return sendMessageQueueAndWaitResponse(name, (uint8_t *)isMute, VOLUME_MANAGER_IS_MUTE);
}

int32_t volumeManagerActivate(char *name)
{
    return sendMessageQueue(name, 0, VOLUME_MANAGER_ACTIVATE);
}

static int32_t volumeGet(char *name, uint8_t *value)
{
    int32_t  retVal    = -1;
    int32_t *valueRead = NULL;

    if ((name == NULL) || (value == NULL))
    {
        SYSLOG_DEBUG("param error.\r\n");
        goto labelEnd;
    }

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    valueRead = iniKeyValueRead(VOLUME_MANAGER_FILE, name, INI_VALUE_INT);
#endif
    if (valueRead != NULL)
    {
        retVal = 0;
        *value = (uint8_t)(*valueRead & 0xFF);
        free(valueRead);
        valueRead = NULL;
    }
    else
    {
        SYSLOG_DEBUG("No matched volume: %s\r\n", name);
    }

labelEnd:
    return retVal;
}

static int32_t atRespCallback(UINT8 chanId, const CHAR *pStr, UINT32 strLen, void *pArg)
{
    if (pStr != NULL)
    {
        SYSLOG_DEBUG("pStr=%s\r\n", pStr);

        char *posBegin = strstr((char *)pStr, "+CLVL: ");
        if (posBegin != NULL)
        {
            posBegin += strlen("+CLVL: ");
            gClvlValue[0] = atoi(posBegin);

            for (uint32_t i=1 ;i<AUDIO_CFG_DEVICE_MAX; i++)
            {
                posBegin = strchr(posBegin, ',');
                if (posBegin == NULL)
                {
                    break;
                }

                posBegin++;
                gClvlValue[i] = atoi(posBegin);
            }
        }
    }

    return 0;
}

static int32_t setVolumeToDevice(uint8_t volume)
{
    int32_t retVal        = -1;
    char    bufferSet[16] = {0};
    char    bufferGet[16] = "AT+CLVL?";
    int8_t  count         = 3;

    memset(bufferSet, 0, sizeof(bufferSet));
    snprintf(bufferSet, sizeof(bufferSet), "AT+CLVL=%d,%d", AUDIO_CFG_DEVICE_HAND_FREE, volume);
    SYSLOG_DEBUG("RIL AT: %s\r\n", bufferSet);

    while (count-- > 0)
    {
        atRilAtCmdReq(bufferSet, strlen(bufferSet), atRespCallback, NULL, 0);
        osDelay(1);
        memset((void *)gClvlValue, -1, sizeof(gClvlValue));
        atRilAtCmdReq(bufferGet, strlen(bufferGet), atRespCallback, NULL, 0);
        osDelay(1);
        while (gClvlValue[0] == -1)
        {
            osDelay(1);
        }

        if (gClvlValue[AUDIO_CFG_DEVICE_HAND_FREE] == volume)
        {
            retVal = 0;
            break;
        }
    }

    return retVal;
}

static int32_t volumeSet(char *name, uint8_t volume)
{
    int32_t  retVal     = -1;
    int32_t *valueRead  = NULL;
    int32_t  valueWrite = 0;

    if (name == NULL)
    {
        SYSLOG_DEBUG("param error.\r\n");
        goto labelEnd;
    }

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    valueRead = iniKeyValueRead(VOLUME_MANAGER_FILE, name, INI_VALUE_INT);
#endif
    if ((valueRead == NULL) || ((*valueRead & 0xFF) != volume))
    {
        valueWrite = (valueRead == NULL) ? volume : ((*valueRead & (~0xFF)) | volume);
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
        retVal = iniKeyValueWrite(VOLUME_MANAGER_FILE, name, INI_VALUE_INT, &valueWrite);
        if (retVal != 0)
        {
            SYSLOG_DEBUG("Failed to write volume: %s, %d\r\n", name, volume);
        }
        else if (((*valueRead & 0xFF00) == 0) && (strcmp(gCurrentVolumeName, name) == 0))
        {
            setVolumeToDevice(volume);
        }
#endif
    }

labelEnd:
    if (valueRead != NULL)
    {
        free(valueRead);
        valueRead = NULL;
    }

    return retVal;
}

static int32_t volumeMute(char *name)
{
    int32_t  retVal     = -1;
    int32_t *valueRead  = NULL;
    int32_t  valueWrite = 0;

    if (name == NULL)
    {
        SYSLOG_DEBUG("param error.\r\n");
        goto labelEnd;
    }

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    valueRead = iniKeyValueRead(VOLUME_MANAGER_FILE, name, INI_VALUE_INT);
#endif
    if ((valueRead != NULL) && ((*valueRead & 0xFF00) == 0))
    {
        valueWrite = *valueRead | (1 << 8);
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
        retVal = iniKeyValueWrite(VOLUME_MANAGER_FILE, name, INI_VALUE_INT, &valueWrite);
        if (retVal != 0)
        {
            SYSLOG_DEBUG("Failed to write mute: %s\r\n", name);
        }
        else if (strcmp(gCurrentVolumeName, name) == 0)
        {
            setVolumeToDevice(0);
        }
#endif
    }

labelEnd:
    if (valueRead != NULL)
    {
        free(valueRead);
        valueRead = NULL;
    }

    return retVal;
}

static int32_t volumeUnmute(char *name)
{
    int32_t  retVal     = -1;
    int32_t *valueRead  = NULL;
    int32_t  valueWrite = 0;

    if (name == NULL)
    {
        SYSLOG_DEBUG("param error.\r\n");
        goto labelEnd;
    }

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    valueRead = iniKeyValueRead(VOLUME_MANAGER_FILE, name, INI_VALUE_INT);
#endif
    if ((valueRead != NULL) && ((*valueRead & 0xFF00) == (1 << 8)))
    {
        valueWrite = *valueRead & (~(1 << 8));
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
        retVal = iniKeyValueWrite(VOLUME_MANAGER_FILE, name, INI_VALUE_INT, &valueWrite);
        if (retVal != 0)
        {
            SYSLOG_DEBUG("Failed to write unmute: %s\r\n", name);
        }
        else if (strcmp(gCurrentVolumeName, name) == 0)
        {
            setVolumeToDevice((uint8_t)(*valueRead & 0xFF));
        }
#endif
    }

labelEnd:
    if (valueRead != NULL)
    {
        free(valueRead);
        valueRead = NULL;
    }

    return retVal;
}

static int32_t volumeIsMute(char *name, bool *mute)
{
    int32_t  retVal    = -1;
    int32_t *valueRead = NULL;
    uint8_t  muteValue = 0;

    if ((name == NULL) || (mute == NULL))
    {
        SYSLOG_DEBUG("param error.\r\n");
        goto labelEnd;
    }

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    valueRead = iniKeyValueRead(VOLUME_MANAGER_FILE, name, INI_VALUE_INT);
#endif
    if (valueRead != NULL)
    {
        muteValue = (*valueRead >> 8) & 0xFF;
        if (muteValue == 0)
        {
            *mute = false;
        }
        else if (muteValue == 1)
        {
            *mute = true;
        }
        else
        {
            SYSLOG_DEBUG("mute value error: %s, %d\r\n", name, muteValue);
            goto labelEnd;
        }

        retVal = 0;
    }
    else
    {
        SYSLOG_DEBUG("Failed to read mute status: %s\r\n", name);
    }

labelEnd:
    if (valueRead != NULL)
    {
        free(valueRead);
        valueRead = NULL;
    }
    return retVal;
}

static int32_t volumeActivate(char *name)
{
    int32_t retVal = -1;
    uint8_t volume = 0;

    if ((volumeGet(name, &volume) == 0) && (setVolumeToDevice(volume) == 0))
    {
        memset(gCurrentVolumeName, 0, sizeof(gCurrentVolumeName));
        memcpy(gCurrentVolumeName, name, strlen(name));
        retVal = 0;
    }

    return retVal;
}

static void threadVolumeManager(void *argument)
{
    QueueVolumeManagerT queue    = {0};
    uint8_t             response = 0;

    if (gVolumeManagerRequestQueue == NULL)
    {
        gVolumeManagerRequestQueue = osMessageQueueNew(QUEUE_SIZE_VOLUME_MANAGER, sizeof(queue), NULL);
        if (gVolumeManagerRequestQueue == NULL)
        {
            SYSLOG_EMERG("Failed to create queue for gVolumeManagerRequestQueue.\r\n");
            goto labelEnd;
        }
    }

    if (gVolumeManagerResponseQueue == NULL)
    {
        gVolumeManagerResponseQueue = osMessageQueueNew(1, sizeof(uint8_t), NULL);
        if (gVolumeManagerResponseQueue == NULL)
        {
            SYSLOG_EMERG("Failed to create queue for gVolumeManagerResponseQueue.\r\n");
            goto labelEnd;
        }
    }

    if (gVolumeManagerMutex == NULL)
    {
        gVolumeManagerMutex = osMutexNew(NULL);
        if(gVolumeManagerMutex == NULL)
        {
            SYSLOG_EMERG("Failed to create mutex for gVolumeManagerMutex\r\n");
            goto labelEnd;
        }
    }

    audioSetCurrentMode(AUDIO_CFG_DEVICE_HAND_FREE);

    while (1)
    {
        memset(&queue, 0, sizeof(queue));
        if (osMessageQueueGet(gVolumeManagerRequestQueue, &queue, 0, osWaitForever) == osOK)
        {
            switch (queue.type)
            {
                case VOLUME_MANAGER_GET:
                    volumeGet(queue.name, &response);
                    osMessageQueuePut(gVolumeManagerResponseQueue, &response, 0, osWaitForever);
                    break;

                case VOLUME_MANAGER_SET:
                    volumeSet(queue.name, queue.volume);
                    break;

                case VOLUME_MANAGER_MUTE:
                    volumeMute(queue.name);
                    break;

                case VOLUME_MANAGER_UNMUTE:
                    volumeUnmute(queue.name);
                    break;

                case VOLUME_MANAGER_IS_MUTE:
                    volumeIsMute(queue.name, (bool *)&response);
                    osMessageQueuePut(gVolumeManagerResponseQueue, &response, 0, osWaitForever);
                    break;

                case VOLUME_MANAGER_ACTIVATE:
                    volumeActivate(queue.name);
                    break;

                default:
                    break;
            }
        }
    }

labelEnd:
#if 0 // Service Manager
    osThreadExit();
#else
    Service_stop("service:/threadVolumeManager");
#endif
}

void volumeManagerInit(void)
{
    osThreadAttr_t threadAttr = {0};

    if (gVolumeManagerThread == NULL)
    {
        memset(&threadAttr, 0, sizeof(threadAttr));
        threadAttr.name       = "threadVolumeManager";
        threadAttr.priority   = osPriorityNormal;
        threadAttr.stack_size = THREAD_STACK_SIZE_VOLUME_MANAGER;
#ifdef THREAD_STATIC
        threadAttr.stack_mem  = gVolumeManagerThreadStackMem;
        threadAttr.cb_mem     = &gVolumeManagerThreadCbMem;
        threadAttr.cb_size    = sizeof(StaticTask_t);
#endif
#if 0
        gVolumeManagerThread = osThreadNew(threadVolumeManager, NULL, &threadAttr);
#else
        char serviceName[32] = {0};
        snprintf(serviceName, sizeof(serviceName), "service:/%s", threadAttr.name);
        Service_reg(serviceName, threadVolumeManager, NULL, threadAttr.cb_mem, threadAttr.cb_size, threadAttr.stack_mem, threadAttr.stack_size, threadAttr.priority);
        gVolumeManagerThread = (osThreadId_t)Service_start(serviceName);
#endif
        if (gVolumeManagerThread == NULL)
        {
            SYSLOG_EMERG("Failed to create thread for volumeManager.\r\n");
        }
    }
}
