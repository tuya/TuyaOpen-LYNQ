/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    app.c
 * Description:  EC618 lwm2m demo entry source file
 * History:      Rev1.0   2018-10-12
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os2.h"
#include "slpman.h"
#include "charge.h"
#include "codecDrv.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#include "iniparse.h"
#endif
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
#include "flashex.h"
#endif
#include "audio.h"
#ifdef FEATURE_SUBSYS_MISC_ENABLE
#include "misc.h"
#endif
#include "api_codec.h"
#include "servicemanager.h"


#if defined(FEATURE_SUBSYS_MP3_ENABLE)
#define THREAD_STACK_SIZE_AUDIO     (9 * 1024)
#elif defined(FEATURE_SUBSYS_TTS_ENABLE)
#define THREAD_STACK_SIZE_AUDIO     (2 * 1024)
#elif defined(FEATURE_SUBSYS_WAV_ENABLE)
#define THREAD_STACK_SIZE_AUDIO     (1 * 1024)
#endif
#define QUEUE_SIZE_AUDIO            100

#define VOLUME_INDEX_MIN            0
#define VOLUME_INDEX_MAX            4
#if defined(CODEC_ES8374_ENABLE)
#define VOLUME_INDEX_DEFAULT        2
#else
#define VOLUME_INDEX_DEFAULT        VOLUME_INDEX_MIN
#endif
#define INI_KEY_VOLUME              "volume"
#if defined(CODEC_ES8311_ENABLE)
#define API_CODEC_TYPE              API_CODEC_ES8311
#elif defined(CODEC_ES7149_ENABLE)
#define API_CODEC_TYPE              API_CODEC_ES7149
#elif defined(CODEC_ES7111_ENABLE)
#define API_CODEC_TYPE              API_CODEC_ES7111
#elif defined(CODEC_TM8211_ENABLE)
#define API_CODEC_TYPE              API_CODEC_TM8211
#elif defined(PWM_CODEC_ENABLE)
#define API_CODEC_TYPE              API_CODEC_PWM
#elif defined(SPI_PWM_CODEC_ENABLE)
#define API_CODEC_TYPE              API_CODEC_SPI_PWM
#elif defined(SPI_SIGMADELTA_CODEC_ENABLE)
#define API_CODEC_TYPE              API_CODEC_SPI_SIGMADELTA
#endif
#define RECORD_TEST


#ifdef THREAD_STATIC
static StaticTask_t       gAudioThreadCbMem                             = {0};
static uint8_t            gAudioThreadStackMem[THREAD_STACK_SIZE_AUDIO] = {0};
#endif
static osThreadId_t       gAudioThread  = NULL;
static osMessageQueueId_t gAudioQueue   = NULL;
static bool               gAudioReady   = false;
static bool               gAudioBusy    = false;
static bool               gWakeup       = false;
static uint8_t            gVolumeIndex  = VOLUME_INDEX_DEFAULT;
#if defined(CODEC_ES7111_ENABLE) || defined(CODEC_ES7149_ENABLE)
static const uint16_t     gVolumeList[] = {1, 2, 3, 4, 5};
#elif defined(CODEC_ES8311_ENABLE)
static const uint16_t     gVolumeList[] = {1, 3, 6, 10, 20};
#elif defined(PWM_CODEC_ENABLE)
static const uint16_t     gVolumeList[] = {3, 6, 10, 20, 30};
#elif defined(SPI_CODEC_ENABLE)
static const uint16_t     gVolumeList[] = {10, 15, 20, 30, 50};
#else
static const uint16_t     gVolumeList[] = {2, 5, 10, 20, 30};
#endif
static uint16_t           gVolume       = gVolumeList[VOLUME_INDEX_DEFAULT];
static uint8_t            gCodecVolume  = CODEC_VOLUME_PLAY_DEFAULT;
static bool               gPlayIsStop   = true;
openPlayer                gOpenPlayer   = NULL;


static void audioPlayRomData(uint8_t *data, uint32_t length, AudioCallbackT callback, uint8_t audioType)
{
    QueueAudioT queue = {0};

    if ((data != NULL) && (length > 0) && (gAudioQueue != NULL))
    {
        memset(&queue, 0, sizeof(queue));
        queue.audioType          = audioType;
        queue.playParam.buffer   = malloc(sizeof(VirtualFileT));
        queue.playParam.callback = callback;
        if (queue.playParam.buffer != NULL)
        {
            memset(queue.playParam.buffer, 0, sizeof(VirtualFileT));
            memcpy(((VirtualFileT *)(queue.playParam.buffer))->prefix, "R:/", strlen("R:/"));
            ((VirtualFileT *)(queue.playParam.buffer))->address = (uint32_t)data;
            ((VirtualFileT *)(queue.playParam.buffer))->size    = length;
            osMessageQueuePut(gAudioQueue, &queue, 0, osWaitForever);
        }
    }
}

#ifdef FEATURE_SUBSYS_MP3_ENABLE
void audioPlayMp3(char *path, AudioCallbackT callback, bool increase, bool dual)
{
    QueueAudioT queue  = {0};
    uint32_t    length = 0;

    if ((path != NULL) && (gAudioQueue != NULL))
    {
        memset(&queue, 0, sizeof(queue));
        length                   = strlen(path);
        queue.audioType          = AUDIO_PLAY_MP3;
        queue.playParam.buffer   = malloc(length + 1);
        queue.playParam.callback = callback;
        queue.playParam.increase = increase;
        queue.playParam.dual     = dual;
        if (queue.playParam.buffer != NULL)
        {
            memset(queue.playParam.buffer, 0,    length + 1);
            memcpy(queue.playParam.buffer, path, length);
            osMessageQueuePut(gAudioQueue, &queue, 0, osWaitForever);
        }
    }
}

void audioPlayMp3Data(uint8_t *data, uint32_t length)
{
    audioPlayRomData(data, length, NULL, AUDIO_PLAY_MP3);
}
#endif

#ifdef FEATURE_SUBSYS_TTS_ENABLE
void audioPlayTts(char *text, AudioCallbackT callback)
{
    QueueAudioT queue  = {0};
    uint32_t    length = 0;

    if ((text != NULL) && (gAudioQueue != NULL))
    {
        memset(&queue, 0, sizeof(queue));
        length                   = strlen(text);
        queue.audioType          = AUDIO_PLAY_TTS;
        queue.playParam.buffer   = malloc(length + 1);
        queue.playParam.callback = callback;
        if (queue.playParam.buffer != NULL)
        {
            memset(queue.playParam.buffer, 0,    length + 1);
            memcpy(queue.playParam.buffer, text, length);
            osMessageQueuePut(gAudioQueue, &queue, 0, osWaitForever);
        }
    }
}
#endif

#ifdef FEATURE_SUBSYS_AMR_ENABLE
void audioPlayAmr(char *path, AudioCallbackT callback)
{
    QueueAudioT queue  = {0};
    uint32_t    length = 0;

    if ((path != NULL) && (gAudioQueue != NULL))
    {
        memset(&queue, 0, sizeof(queue));
        length                   = strlen(path);
        queue.audioType          = AUDIO_PLAY_AMR;
        queue.playParam.buffer   = malloc(length + 1);
        queue.playParam.callback = callback;
        if (queue.playParam.buffer != NULL)
        {
            memset(queue.playParam.buffer, 0,    length + 1);
            memcpy(queue.playParam.buffer, path, length);
            osMessageQueuePut(gAudioQueue, &queue, 0, osWaitForever);
        }
    }
}

void audioPlayAmrData(uint8_t *data, uint32_t length, AudioCallbackT callback)
{
    audioPlayRomData(data, length, callback, AUDIO_PLAY_AMR);
}
#endif

#ifdef FEATURE_SUBSYS_WAV_ENABLE
void audioPlayWav(char *path, AudioCallbackT callback)
{
    QueueAudioT queue  = {0};
    uint32_t    length = 0;

    if ((path != NULL) && (gAudioQueue != NULL))
    {
        memset(&queue, 0, sizeof(queue));
        length                   = strlen(path);
        queue.audioType          = AUDIO_PLAY_WAV;
        queue.playParam.buffer   = malloc(length + 1);
        queue.playParam.callback = callback;
        if (queue.playParam.buffer != NULL)
        {
            memset(queue.playParam.buffer, 0,    length + 1);
            memcpy(queue.playParam.buffer, path, length);
            osMessageQueuePut(gAudioQueue, &queue, 0, osWaitForever);
        }
    }
}
#endif

#ifdef FEATURE_SUBSYS_PCM_ENABLE
void audioPlayPcm(uint8_t *pcm, uint32_t length, uint32_t rate)
{
    QueueAudioT queue = {0};

    if ((pcm != NULL) && (gAudioQueue != NULL))
    {
        memset(&queue, 0, sizeof(queue));
        queue.audioType        = AUDIO_PLAY_PCM;
        queue.playParam.buffer = malloc(length);
        queue.playParam.length = length;
        queue.playParam.rate   = rate;
        if (queue.playParam.buffer != NULL)
        {
            memset(queue.playParam.buffer, 0,   length);
            memcpy(queue.playParam.buffer, pcm, length);
            osMessageQueuePut(gAudioQueue, &queue, 0, osWaitForever);
        }
    }
}
#endif

void audioAdjustVolume(uint8_t action)
{
    QueueAudioT     queue       = {0};
    static uint8_t  sAction[32] = {0};
    static uint32_t sIndex      = 0;

    if  (gAudioQueue != NULL)
    {
        memset(&queue, 0, sizeof(queue));
        sAction[sIndex]          = action;
        queue.audioType          = AUDIO_PLAY_VOLUME;
        queue.playParam.buffer   = (char *)(&sAction[sIndex]);
        queue.playParam.callback = NULL;
        queue.playParam.dual     = true;
        osMessageQueuePut(gAudioQueue, &queue, 0, 0);
        sIndex = (sIndex + 1) % sizeof(sAction);
    }
}

#ifdef FEATURE_SUBSYS_PCM_RECORD_ENABLE
void audioRecordG726(char *path)
{
    QueueAudioT queue  = {0};
    uint32_t    length = 0;

    if ((path != NULL) && (gAudioQueue != NULL))
    {
        memset(&queue, 0, sizeof(queue));
        length                     = strlen(path);
        queue.audioType            = AUDIO_RECORD_G726;
        queue.recordParam.fileName = malloc(length + 1);
        if (queue.recordParam.fileName != NULL)
        {
            memset(queue.recordParam.fileName, 0,    length + 1);
            memcpy(queue.recordParam.fileName, path, length);
            osMessageQueuePut(gAudioQueue, &queue, 0, osWaitForever);
        }
    }
}

void audioRecordAdpcm(char *path)
{
    QueueAudioT queue  = {0};
    uint32_t    length = 0;

    if ((path != NULL) && (gAudioQueue != NULL))
    {
        memset(&queue, 0, sizeof(queue));
        length                     = strlen(path);
        queue.audioType            = AUDIO_RECORD_ADPCM;
        queue.recordParam.fileName = malloc(length + 1);
        if (queue.recordParam.fileName != NULL)
        {
            memset(queue.recordParam.fileName, 0,    length + 1);
            memcpy(queue.recordParam.fileName, path, length);
            osMessageQueuePut(gAudioQueue, &queue, 0, osWaitForever);
        }
    }
}

void audioRecordG711(char *path)
{
    QueueAudioT queue  = {0};
    uint32_t    length = 0;

    if ((path != NULL) && (gAudioQueue != NULL))
    {
        memset(&queue, 0, sizeof(queue));
        length                     = strlen(path);
        queue.audioType            = AUDIO_RECORD_G711;
        queue.recordParam.fileName = malloc(length + 1);
        if (queue.recordParam.fileName != NULL)
        {
            memset(queue.recordParam.fileName, 0,    length + 1);
            memcpy(queue.recordParam.fileName, path, length);
            osMessageQueuePut(gAudioQueue, &queue, 0, osWaitForever);
        }
    }
}
#endif

#ifdef FEATURE_SUBSYS_AMR_RECORD_ENABLE
int32_t audioRecordAmr(RecordParamT *recordParam)
{
    QueueAudioT queue = {0};

    queue.audioType = AUDIO_RECORD_AMR;
    memcpy(&queue.recordParam, recordParam, sizeof(RecordParamT));

    return osMessageQueuePut(gAudioQueue, &queue, 0, 0);
}
#endif

bool audioIsReady(void)
{
    return gAudioReady;
}

static void adjustVolume(uint8_t action, char **path)
{
    switch (action)
    {
        case ACTION_VOLUME_MINUS_SHORT:
            if (gVolumeIndex == VOLUME_INDEX_MIN)
            {
                *path = VOLUME_SOUND_MIN;
            }
            else
            {
                gVolume = gVolumeList[--gVolumeIndex];
                *path   = VOLUME_SOUND_MINUS;
            }
            break;

        case ACTION_VOLUME_PLUS_SHORT:
            if (gVolumeIndex == VOLUME_INDEX_MAX)
            {
                *path = VOLUME_SOUND_MAX;
            }
            else
            {
                gVolume = gVolumeList[++gVolumeIndex];
                *path   = VOLUME_SOUND_PLUS;
            }
            break;

        case ACTION_VOLUME_MINUS_LONG:
            gVolumeIndex = VOLUME_INDEX_MIN;
            gVolume      = gVolumeList[gVolumeIndex];
            *path        = VOLUME_SOUND_MIN;
            break;

        case ACTION_VOLUME_PLUS_LONG:
            gVolumeIndex = VOLUME_INDEX_MAX;
            gVolume      = gVolumeList[gVolumeIndex];
            *path        = VOLUME_SOUND_MAX;
            break;

        default:
            *path = NULL;
            break;
    }
}

static int32_t audioVolumeWrite(uint16_t volume)
{
    int32_t  retVal     = 0;
    uint16_t volumeRead = 0xFFFF;
    int32_t  *valueRead = NULL;
    int32_t  valueWrite = volume;


#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    valueRead = iniKeyValueRead(DEFAULT_INFO, INI_KEY_VOLUME, INI_VALUE_INT);
#endif
    if (valueRead != NULL)
    {
        volumeRead = (uint16_t)(*valueRead);
        free(valueRead);
        valueRead = NULL;
    }

    if (volumeRead != volume)
    {
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
        retVal = iniKeyValueWrite(DEFAULT_INFO, INI_KEY_VOLUME, INI_VALUE_INT, (void *)&valueWrite);
#endif
    }

    return retVal;
}

static uint16_t audioVolumeRead(void)
{
    int32_t  valueWrite = gVolumeIndex;
    int32_t  *value     = NULL;

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    value = iniKeyValueRead(DEFAULT_INFO, INI_KEY_VOLUME, INI_VALUE_INT);
#endif
    if (value != NULL)
    {
        gVolumeIndex = (uint16_t)(*value);
        free(value);
        value = NULL;
    }
    else
    {
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
        iniKeyValueWrite(DEFAULT_INFO, INI_KEY_VOLUME, INI_VALUE_INT, (void *)&valueWrite);
#endif
    }

    return gVolumeList[gVolumeIndex];
}

static void restoreCallback(void *p_data, slpManLpState state)
{
    gWakeup = true;
}

static void audioCallback(void *userdata,int32_t result)
{
    gAudioReady = true;
    openPlaySetCallback(gOpenPlayer, NULL, NULL);
}

static void audioPlayProcess(uint8_t audioType, PlayParamT *playParam)
{
    char *path = NULL;

    gPlayIsStop = false;

    switch (audioType)
    {
        case AUDIO_PLAY_VOLUME:
            if (playParam->buffer != NULL)
            {
                adjustVolume(*((uint8_t *)(playParam->buffer)), &path);
                audioVolumeWrite(gVolumeIndex);
                if (path != NULL)
                {
#if defined(FEATURE_SUBSYS_MP3_ENABLE)
                    mp3Play(path, playParam->increase, playParam->dual);
#elif defined(FEATURE_SUBSYS_TTS_ENABLE)
                    ttsPlay(path);
#elif defined(FEATURE_SUBSYS_WAV_ENABLE)
                    wavPlay(path);
#endif
                }
            }
            break;

        case AUDIO_PLAY_MP3:
            if (playParam->buffer != NULL)
            {
#ifdef FEATURE_SUBSYS_MP3_ENABLE
                mp3Play(playParam->buffer, playParam->increase, playParam->dual);
#endif
                free(playParam->buffer);
                playParam->buffer = NULL;
            }
            break;

        case AUDIO_PLAY_TTS:
            if (playParam->buffer != NULL)
            {
#ifdef FEATURE_SUBSYS_TTS_ENABLE
                ttsPlay(playParam->buffer);
#endif
                free(playParam->buffer);
                playParam->buffer = NULL;
            }
            break;

        case AUDIO_PLAY_AMR:
            if (playParam->buffer != NULL)
            {
#ifdef FEATURE_SUBSYS_AMR_ENABLE
                SYSLOG_INFO("Start Play AMR: path=%s\r\n", playParam->buffer);
                amrPlay(playParam->buffer);
                SYSLOG_INFO("Play End\r\n");
#endif
                free(playParam->buffer);
                playParam->buffer = NULL;
            }
            break;

        case AUDIO_PLAY_WAV:
            if (playParam->buffer != NULL)
            {
#ifdef FEATURE_SUBSYS_WAV_ENABLE
                wavPlay(playParam->buffer);
#endif
                free(playParam->buffer);
                playParam->buffer = NULL;
            }
            break;

        case AUDIO_PLAY_PCM:
            if (playParam->buffer != NULL)
            {
#ifdef FEATURE_SUBSYS_PCM_ENABLE
                pcmPlay((uint8_t *)(playParam->buffer), playParam->length, playParam->rate);
#endif
                free(playParam->buffer);
                playParam->buffer = NULL;
            }
            break;

        default:
            break;
    }

    gPlayIsStop = true;
    apiCodecStop();

    if (playParam->callback != NULL)
    {
        playParam->callback(0);
    }
}

#ifdef FEATURE_SUBSYS_RECORD_ENABLE
static void audioRecordProcess(uint8_t audioType, RecordParamT *recordParam)
{
    switch (audioType)
    {
#ifdef FEATURE_SUBSYS_PCM_RECORD_ENABLE
        case AUDIO_RECORD_G726:
            if (recordParam->fileName != NULL)
            {
                pcmRecord(recordParam->fileName);
#ifdef RECORD_TEST
                SYSLOG_DEBUG("Play the recording: %s\r\n", recordParam->fileName);
                audioPlayWav(recordParam->fileName, NULL);
#endif
#ifdef FEATURE_SUBSYS_G726_ENABLE
                g726Encode(recordParam->fileName);
#ifdef RECORD_TEST
                SYSLOG_DEBUG("Play the recording: %s\r\n", G726_WAV_FILE);
                g726Decode(WAV_G726_FILE);
                audioPlayWav(G726_WAV_FILE, NULL);
#endif
#endif
                free(recordParam->fileName);
                recordParam->fileName = NULL;
            }
            break;

        case AUDIO_RECORD_ADPCM:
            if (recordParam->fileName != NULL)
            {
                pcmRecord(recordParam->fileName);
#ifdef RECORD_TEST
                SYSLOG_DEBUG("Play the recording: %s\r\n", recordParam->fileName);
                audioPlayWav(recordParam->fileName, NULL);
#endif
#ifdef FEATURE_SUBSYS_ADPCM_ENABLE
                adpcmEncode(recordParam->fileName);
#ifdef RECORD_TEST
                SYSLOG_DEBUG("Play the recording: %s\r\n", ADPCM_WAV_FILE);
                adpcmDecode(WAV_ADPCM_FILE);
                audioPlayWav(ADPCM_WAV_FILE, NULL);
#endif
#endif
                free(recordParam->fileName);
                recordParam->fileName = NULL;
            }
            break;

        case AUDIO_RECORD_G711:
            if (recordParam->fileName != NULL)
            {
                pcmRecord(recordParam->fileName);
#ifdef RECORD_TEST
                SYSLOG_DEBUG("Play the recording: %s\r\n", recordParam->fileName);
                audioPlayWav(recordParam->fileName, NULL);
#endif
#ifdef FEATURE_SUBSYS_G711_ENABLE
                g711Encode(recordParam->fileName, "D:/wav.g711");
#ifdef RECORD_TEST
                SYSLOG_DEBUG("Play the recording: %s\r\n", "D:/g711.wav");
                g711Decode("D:/wav.g711", "D:/g711.wav");
                audioPlayWav("D:/g711.wav", NULL);
#endif
#endif
                free(recordParam->fileName);
                recordParam->fileName = NULL;
            }
            break;

        case AUDIO_RECORD_PCM:
            break;

        case AUDIO_RECORD_PCM_3A:
            break;
#endif

#ifdef FEATURE_SUBSYS_AMR_RECORD_ENABLE
        case AUDIO_RECORD_AMR:
            // SYSLOG_INFO("Start Record AMR: path=%s\r\n", recordParam->fileName);
            amrRecord(recordParam);
            // SYSLOG_INFO("Record End\r\n");
            break;
#endif

        default:
            break;
    }
}
#endif

#ifdef FEATURE_SUBSYS_RECORD_VOLTE_ENABLE
static void audioRecordVolteProcess(uint8_t audioType, RecordVolteParamT *recordVolteParam)
{
    switch (audioType)
    {
        case AUDIO_RECORD_VOLTE_AMR:
            break;

        case AUDIO_RECORD_VOLTE_PCM:
            break;

        case AUDIO_RECORD_VOLTE_PCM_3A:
            break;

        default:
            break;
    }
}
#endif

static void audioProcess(QueueAudioT *queue)
{
    switch (queue->audioType & 0xF0)
    {
        case AUDIO_PLAY:
            audioPlayProcess(queue->audioType, &queue->playParam);
            break;

#ifdef FEATURE_SUBSYS_RECORD_ENABLE
        case AUDIO_RECORD:
            audioRecordProcess(queue->audioType, &queue->recordParam);
            break;
#endif

#ifdef FEATURE_SUBSYS_RECORD_VOLTE_ENABLE
        case AUDIO_RECORD_VOLTE:
            audioRecordVolteProcess(queue->audioType, &queue->recordVolteParam);
            break;
#endif

        default:
            break;
    }
}

static void threadAudio(void *argument)
{
    QueueAudioT queue = {0};
    uint8_t     vote  = 0xFF;

    gAudioQueue = osMessageQueueNew(QUEUE_SIZE_AUDIO, sizeof(queue), NULL);
    if (gAudioQueue == NULL)
    {
        SYSLOG_EMERG("Failed to create queue for gAudioQueue.\r\n");
        goto labelEnd;
    }

    gOpenPlayer = openPlayCreate(NULL);
    openPlaySetCallback(gOpenPlayer, audioCallback, NULL);
    openPlay(gOpenPlayer, WELCOME_SOUND);
    if (simGetStatus(0) == SIM_REMOVED)
    {
        openPlay(gOpenPlayer, SIM_SOUND_UNREADY);
    }
    else if (nwIsReady() == true)
    {
        openPlay(gOpenPlayer, NW_SOUND_READY);
    }
    slpManRegisterUsrdefinedRestoreCb(restoreCallback, NULL);
    slpManApplyPlatVoteHandle("audio", &vote);
    apiCodecInit(API_CODEC_TYPE);
    gVolume = audioVolumeRead();
#ifdef FEATURE_SUBSYS_TTS_ENABLE
    ttsInit();
#endif

    osDelay(3*1000);

    while (1)
    {
        memset(&queue, 0, sizeof(queue));
        slpManPlatVoteEnableSleep(vote, SLP_SLP1_STATE);
        if (osMessageQueueGet(gAudioQueue, &queue, 0, osWaitForever) == osOK)
        {
            slpManPlatVoteDisableSleep(vote, SLP_SLP1_STATE);
            if (gWakeup == true)
            {
                gWakeup = false;
                codecVoltageSet();
                apiCodecPaInit();
            }

            gAudioBusy = true;
            audioProcess(&queue);
            gAudioBusy = false;
        }
    }

labelEnd:
#if 0 // Service Manager
    osThreadExit();
#else
    Service_stop("service:/threadAudio");
#endif
}

void audioInit(void)
{
    osThreadAttr_t threadAttr = {0};

    if (gAudioThread == NULL)
    {
        memset(&threadAttr, 0, sizeof(threadAttr));
        threadAttr.name       = "threadAudio";
        threadAttr.priority   = osPriorityNormal;
        threadAttr.stack_size = THREAD_STACK_SIZE_AUDIO;
#ifdef THREAD_STATIC
        threadAttr.stack_mem  = gAudioThreadStackMem;
        threadAttr.cb_mem     = &gAudioThreadCbMem;
        threadAttr.cb_size    = sizeof(StaticTask_t);
#endif
#if 0 // Service Manager
        gAudioThread = osThreadNew(threadAudio, NULL, &threadAttr);
#else
    char serviceName[32] = {0};
    snprintf(serviceName, sizeof(serviceName), "service:/%s", threadAttr.name);
        Service_reg(serviceName, threadAudio, NULL, threadAttr.cb_mem, threadAttr.cb_size, threadAttr.stack_mem, threadAttr.stack_size, threadAttr.priority);
        gAudioThread = (osThreadId_t)Service_start(serviceName);
#endif
        if (gAudioThread == NULL)
        {
            SYSLOG_EMERG("Failed to create thread for audio.\r\n");
        }
    }
}

bool audioIsBusy(void)
{
    return gAudioBusy;
}

uint16_t audioGetVolume(void)
{
    return gVolume;
}

uint16_t audioGetCodecVolume(void)
{
    return gCodecVolume;
}

void audioSetCodecVolume(uint8_t volume)
{
    gCodecVolume = volume;
}

bool audioPlayIsStop(void)
{
    return gPlayIsStop;
}

void audioPlayStop(void)
{
    gPlayIsStop = true;
}

uint32_t audioPlayTimeGet(void)
{
    return 0;
}

int32_t EC_audioStopPlaying(uint8_t audType,bool pauseFlag)
{
    return 0;
}

int32_t EC_audioStartPlayFile(char *file_name, uint8_t audType, audioParamT audParam, AudioCallbackT play_cb_func)
{
    int32_t retVal = 0;

    switch (audType)
    {
        case AUDIO_PLAY_CODEC_TYPE_MP3:
            audioPlayMp3(file_name, play_cb_func, audParam.increase, audParam.dual);
            break;

        case AUDIO_PLAY_CODEC_TYPE_WAV:
            audioPlayWav(file_name, play_cb_func);
            break;

        default:
            SYSLOG_DEBUG("Not support: audType=%d\r\n", audType);
            retVal = -1;
            break;
    }

    return retVal;
}

int32_t EC_audioStartPlayString(uint8_t *String, uint32_t strLen, uint8_t audType, audioParamT audParam, AudioCallbackT play_cb_func)
{
    int32_t retVal = 0;

    switch (audType)
    {
        case AUDIO_PLAY_CODEC_TYPE_MP3:
            audioPlayRomData(String, strLen, play_cb_func, AUDIO_PLAY_MP3);
            break;

        default:
            SYSLOG_DEBUG("Not support: audType=%d\r\n", audType);
            retVal = -1;
            break;
    }

    return retVal;
}

int32_t EC_audioStartPlayStream(uint8_t *url, uint32_t strLen, uint8_t audType, audioParamT audParam, AudioCallbackT play_cb_func)
{
    SYSLOG_DEBUG("Not support.\r\n");

    return -1;
}

bool EC_audioPlayChkLoop(void)
{
	return false;
}

void EC_audioPlaySetLoop(bool onoff)
{
}
