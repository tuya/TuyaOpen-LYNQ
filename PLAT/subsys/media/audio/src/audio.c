/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    audio.c
 * Description:  EC618 lwm2m demo entry source file
 * History:      Rev1.0   2018-10-12
 *
 ****************************************************************************/
#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include DEBUG_LOG_HEADER_FILE
#include <string.h>

#include "cmsis_os2.h"
#include "slpman.h"
#include "osasys.h"

#include "charge.h"
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#include "iniparse.h"
#endif
#ifdef FEATURE_SUBSYS_AUDIO_ENABLE
#include "audio.h"
#endif
#ifdef FEATURE_SUBSYS_MISC_ENABLE
#include "misc.h"
#endif
#include "servicemanager.h"
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
#include "flashex.h"
#endif
#include "media.h"
#include "ec_ring.h"

#if defined(FEATURE_HTTPC_ENABLE) && defined(FEATURE_HTTP_TLS_ENABLE)
#if defined(FEATURE_SUBSYS_HTTP_DOWNLOADER_ENABLE)
#include "http_downloader.h"
#endif
#endif

#ifdef FEATURE_SUBSYS_MED_PCM_ENABLE
#include "audPcm.h"
#endif

#ifdef FEATURE_SUBSYS_MED_WAV_ENABLE
#include "audWav.h"
#endif

#ifdef FEATURE_SUBSYS_AMR_RECORD_ENABLE
#include "audAmr.h"
#include "audAmrCommon.h"
#endif
#include "ccio_audio.h"

#ifdef FEATURE_SUBSYS_TTS_ENABLE
#include "tts.h"
#endif
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_SUBSYS_POWER_MANAGER_ENABLE
#include "powermanager.h"
#endif
#ifdef FEATURE_SUBSYS_VOLUME_ENABLE
#include "volumeManager.h"
#endif
#ifdef FEATURE_SUBSYS_AMR_RECORD_ENABLE
#include "audAmrRecord.h"
#endif
#ifdef FEATURE_SUBSYS_MP3_ENABLE
#include "mp3.h"
#endif
#include "medDataHandle.h"
#ifdef FEATURE_SUBSYS_G711_ENABLE
#include "g711.h"
#endif

#define THREAD_STACK_SIZE_AUDIO (9 * 1024)
#define THREAD_STACK_SIZE_OPER_AUDIO (3 * 1024)
#define THREAD_STACK_SIZE_AUDIO_TEST (20 * 1024)
#define QUEUE_SIZE_AUDIO 100
#define VOLUME_INDEX_MIN 0
#define VOLUME_INDEX_MAX 4
#define VOLUME_INDEX_DEFAULT VOLUME_INDEX_MIN

#define FILE_READ_BUFFER_SIZE 1500
#define OUTPUT_STRING_LEN 7000
#define DOWNLOAD_SECTION_CNT (6)

#define INI_KEY_VOLUME "volume"

#ifdef THREAD_STATIC
static StaticTask_t gAudioThreadCbMem = {0};
PLAT_FPSRAM_ZI_CUST static uint8_t
    gAudioThreadStackMem[THREAD_STACK_SIZE_AUDIO] = {0};
#if defined(FEATURE_HTTPC_ENABLE) && defined(FEATURE_HTTP_TLS_ENABLE) && \
    defined(FEATURE_SUBSYS_MEDIA_STREAM_ENABLE)
static StaticTask_t gStreamMedThreadCbMem = {0};
PLAT_FPSRAM_ZI_CUST static uint8_t
    gStreamMedThreadStackMem[THREAD_STACK_SIZE_AUDIO] = {0};
#endif
#endif
osMessageQueueId_t gAudioQueue = NULL;
static bool gAudioReady = false;
static bool gAudioBusy = false;
static bool gWakeup = false;
static const uint16_t gVolumeList[] = {2, 5, 10, 20, 30};
static uint16_t gVolume = gVolumeList[VOLUME_INDEX_DEFAULT];
static bool gLocalPlayLoop = false;
osSemaphoreId_t gAudioPlayI2sSem = NULL;

#ifdef FEATURE_SUBSYS_MEDIA_STREAM_ENABLE
typedef struct
{
    bool endFlag;
} QueueStreamDlT;
DlState_e g_u8AudioDownloadOver = DLSTATE_NOT_STARTED;
ecRingT *gAudStreamRingBuf = NULL;
static BOOL gHttpPauseStatus = false;
uint32_t gDownloadLastPos = 0;
#if (defined(FEATURE_HTTPC_ENABLE) && defined(FEATURE_HTTP_TLS_ENABLE) && defined(FEATURE_SUBSYS_HTTP_DOWNLOADER_ENABLE))
static char* gDownloadLastUrl = NULL;
#endif
#endif

char *gCustToneSrc = NULL;
uint8_t gCurPlayAudType = AUDIO_PLAY_CODEC_TYPE_IDLE;

static void restoreCallback(void *p_data, slpManLpState state)
{
    gWakeup = true;
}

void audioToneSrcSet(const char *src)
{
    /*to check realloc */
    char *tmpPtr = NULL;
    uint16_t srcLen = 0;
    if(src == NULL) return;
    srcLen = strlen(src);
    if(gCustToneSrc == NULL)
    {
        gCustToneSrc = malloc(srcLen + 1);
        EC_ASSERT(gCustToneSrc != NULL, 0, 0, 0);
    }
    else
    {
        tmpPtr = realloc(gCustToneSrc, srcLen + 1);
        EC_ASSERT(gCustToneSrc == tmpPtr, gCustToneSrc, tmpPtr, 0);
    }
    memset(gCustToneSrc, 0x00, srcLen + 1);
    memcpy(gCustToneSrc, src, srcLen);

    return;
}

char *audioToneSrcGet(void)
{
    return (gCustToneSrc == NULL ? ALARM_SOUND : gCustToneSrc);
}

void audioStopPlay(void)
{
    medDataHandleStop();
    return;
}

void audioPlayMp3(char *path, AudioCallbackT callback, bool increase, bool dual)
{
    QueueAudioT queue = {0};
    uint32_t length = 0;
    if((path != NULL) && (gAudioQueue != NULL))
    {
        length = strlen(path);
        if(strcasestr(path, "http://") || strcasestr(path, "https://"))
            queue.audType = AUDIO_PLAY_STREAM_MP3;
        else
            queue.audType = AUDIO_PLAY_MP3;
        queue.buffer = malloc(length + 1);
        queue.callback = callback;
        queue.audioParam.increase = increase;
        queue.audioParam.dual = dual;
        if(queue.buffer != NULL)
        {
            memset(queue.buffer, 0, length + 1);
            memcpy(queue.buffer, path, length);
            osMessageQueuePut(gAudioQueue, &queue, 0, osWaitForever);
        }
    }
}

void audioPlayAmr(char *path, AudioCallbackT callback)
{
    QueueAudioT queue = {0};
    uint32_t length = 0;
    if((path != NULL) && (gAudioQueue != NULL))
    {
        length = strlen(path);
        queue.audType = AUDIO_PLAY_AMR;
        queue.buffer = malloc(length + 1);
        queue.callback = callback;
        if(queue.buffer != NULL)
        {
            memset(queue.buffer, 0, length + 1);
            memcpy(queue.buffer, path, length);
            osMessageQueuePut(gAudioQueue, &queue, 0, osWaitForever);
        }
    }
}

#ifdef MBTK_OPENCPU_SUPPORT
void ol_audioPlayMp3Data(UINT8 *data,UINT32 datalen, AudioCallbackT callback)
{
    QueueAudioT queue  = {0};

    if ((data != NULL) && (datalen > 0) && (gAudioQueue != NULL))
    {
        memset(&queue, 0, sizeof(queue));
        queue.audType          = AUDIO_PLAY_MP3;
        queue.buffer   = malloc(sizeof(VirtualFileT));
        queue.callback = callback;
        if (queue.buffer != NULL)
        {
            memset(queue.buffer, 0, sizeof(VirtualFileT));
            memcpy(((VirtualFileT *)(queue.buffer))->prefix, "R:/", strlen("R:/"));
            ((VirtualFileT *)(queue.buffer))->address = data;
            ((VirtualFileT *)(queue.buffer))->size    = datalen;
            osMessageQueuePut(gAudioQueue, &queue, 0, osWaitForever);
        }
    }
}

void ol_audioPlayAmrData(UINT8 *data,UINT32 datalen, AudioCallbackT callback)
{
    QueueAudioT queue  = {0};

    if ((data != NULL) && (datalen > 0) && (gAudioQueue != NULL))
    {
        memset(&queue, 0, sizeof(queue));
        queue.audType          = AUDIO_PLAY_AMR;
        queue.buffer   = malloc(sizeof(VirtualFileT));
        queue.callback = callback;
        if (queue.buffer != NULL)
        {
            memset(queue.buffer, 0, sizeof(VirtualFileT));
            memcpy(((VirtualFileT *)(queue.buffer))->prefix, "R:/", strlen("R:/"));
            ((VirtualFileT *)(queue.buffer))->address = data;
            ((VirtualFileT *)(queue.buffer))->size    = datalen;
            osMessageQueuePut(gAudioQueue, &queue, 0, osWaitForever);
        }
    }
}

void ol_audioPlayPcmData(UINT8 *data, uint32_t datalen, uint32_t rate,AudioCallbackT callback, BOOL continue_p)
{
    QueueAudioT queue = {0};

    if(0 == ol_audPcmPlayContinue(data, datalen, continue_p))
    {
        if ((data != NULL) && (gAudioQueue != NULL) && (datalen > 0))
        {
            memset(&queue, 0, sizeof(queue));
            queue.audType        = AUDIO_PLAY_STRING_PCM;
            queue.audString = data;
            queue.length = datalen;
            queue.callback = callback;
            queue.audioParam.rate = rate;
            queue.audioParam.increase = continue_p;

            osMessageQueuePut(gAudioQueue, &queue, 0, osWaitForever);

            if(1 == continue_p)
            {
                osDelay(100);
            }
        }
    }
}

void ol_audioPlayPcm(UINT8 *path,uint32_t rate, AudioCallbackT callback)
{
    QueueAudioT queue  = {0};
    uint32_t    length = 0;
    if ((path != NULL) && (gAudioQueue != NULL))
    {
        length         = strlen(path);
        queue.audType     = AUDIO_PLAY_PCM;
        queue.buffer   = malloc(length + 1);
        queue.audioParam.rate = rate;
        queue.callback = callback;
        if (queue.buffer != NULL)
        {
            memset(queue.buffer, 0,    length + 1);
            memcpy(queue.buffer, path, length);
            osMessageQueuePut(gAudioQueue, &queue, 0, osWaitForever);
        }
    }
}

#endif //MBTK_OPENCPU_SUPPORT
#ifdef FEATURE_SUBSYS_TTS_ENABLE
void audioPlayTts(char *text, AudioCallbackT callback)
{
    QueueAudioT queue = {0};
    uint32_t length = 0;

    if((text != NULL) && (gAudioQueue != NULL))
    {
        memset(&queue, 0, sizeof(queue));
        length = strlen(text);
        queue.audType = AUDIO_PLAY_TTS;
        queue.buffer = malloc(length + 1);
        queue.callback = callback;
        queue.length = length;
        if(queue.buffer != NULL)
        {
            memset(queue.buffer, 0, length + 1);
            memcpy(queue.buffer, text, length);
            queue.audString = (UINT8 *)queue.buffer;
            osMessageQueuePut(gAudioQueue, &queue, 0, osWaitForever);
        }
    }
}
#endif

bool audioIsReady(void) { return gAudioReady; }

bool audioIsBusy(void) { return gAudioBusy; }

#ifdef SPEAKER_APP
static uint8_t gVolumeIndex = VOLUME_INDEX_DEFAULT;
#ifdef FEATURE_SUBSYS_OPENPLAYER_API_ENABLE
openPlayer     gOpenPlayer  = NULL;
#endif

static void audioCallback(void *userdata,int32_t result)
{
    gAudioReady = true;
#ifdef FEATURE_SUBSYS_OPENPLAYER_API_ENABLE
    openPlaySetCallback(gOpenPlayer, NULL, NULL);
#endif
}

static void playSpeakerWelcome(void)
{
#ifdef FEATURE_SUBSYS_OPENPLAYER_API_ENABLE
    if (gOpenPlayer == NULL)
    {
        openPlayerConfigT opParam =
        {
            .playParam.sampleRate = SAMPLERATE_16K,
            .playParam.store = AUDIO_PLAY_FILE,
        };
        gOpenPlayer = openPlayCreate(&opParam);
    }
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
#else
    audioPlayMp3(WELCOME_SOUND, NULL, false, false);
    if (simGetStatus(0) == SIM_REMOVED)
    {
        audioPlayMp3(SIM_SOUND_UNREADY, NULL, false, false);
    }
    else if (nwIsReady() == true)
    {
        audioPlayMp3(NW_SOUND_READY, NULL, false, false);
    }
#endif
}

void audioAdjustVolume(uint8_t action)
{
    QueueAudioT queue = {0};
    static uint8_t sAction[32] = {0};
    static uint32_t sIndex = 0;

    if(gAudioQueue != NULL)
    {
        memset(&queue, 0, sizeof(queue));
        sAction[sIndex] = action;
        queue.audType = AUDIO_PLAY_VOLUME_MP3;
        queue.buffer = (char *)(&sAction[sIndex]);
        queue.callback = NULL;
        osMessageQueuePut(gAudioQueue, &queue, 0, 0);
        sIndex = (sIndex + 1) % sizeof(sAction);
    }
}

static void adjustVolume(uint8_t action, char **path)
{
    switch(action)
    {
        case ACTION_VOLUME_MINUS_SHORT:
            if(gVolumeIndex == VOLUME_INDEX_MIN)
            {
                *path = VOLUME_SOUND_MIN;
            }
            else
            {
                gVolume = gVolumeList[--gVolumeIndex];
                *path = VOLUME_SOUND_MINUS;
            }
            break;

        case ACTION_VOLUME_PLUS_SHORT:
            if(gVolumeIndex == VOLUME_INDEX_MAX)
            {
                *path = VOLUME_SOUND_MAX;
            }
            else
            {
                gVolume = gVolumeList[++gVolumeIndex];
                *path = VOLUME_SOUND_PLUS;
            }
            break;

        case ACTION_VOLUME_MINUS_LONG:
            gVolumeIndex = VOLUME_INDEX_MIN;
            gVolume = gVolumeList[gVolumeIndex];
            *path = VOLUME_SOUND_MIN;
            break;

        case ACTION_VOLUME_PLUS_LONG:
            gVolumeIndex = VOLUME_INDEX_MAX;
            gVolume = gVolumeList[gVolumeIndex];
            *path = VOLUME_SOUND_MAX;
            break;

        default:
            *path = NULL;
            break;
    }
}

static int32_t audioVolumeWrite(uint16_t volume)
{
    int32_t retVal = 0;
    uint16_t volumeRead = 0xFFFF;
    int32_t *valueRead = NULL;
    int32_t valueWrite = volume;

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    valueRead = iniKeyValueRead(DEFAULT_INFO, INI_KEY_VOLUME, INI_VALUE_INT);
#endif
    if(valueRead != NULL)
    {
        volumeRead = (uint16_t)(*valueRead);
        free(valueRead);
        valueRead = NULL;
    }

    if(volumeRead != volume)
    {
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
        retVal = iniKeyValueWrite(DEFAULT_INFO, INI_KEY_VOLUME, INI_VALUE_INT,
                                  (void *)&valueWrite);
#endif
    }

    return retVal;
}

static uint16_t audioVolumeRead(void)
{
    int32_t valueWrite = gVolumeIndex;
    int32_t *value = NULL;

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    value = iniKeyValueRead(DEFAULT_INFO, INI_KEY_VOLUME, INI_VALUE_INT);
#endif
    if(value != NULL)
    {
        gVolumeIndex = (uint16_t)(*value);
        free(value);
        value = NULL;
    }
    else
    {
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
        iniKeyValueWrite(DEFAULT_INFO, INI_KEY_VOLUME, INI_VALUE_INT,
                         (void *)&valueWrite);
#endif
    }

    return gVolumeList[gVolumeIndex];
}

static int32_t playSpeakerVolume(uint8_t *action, audioParamT *audioParam)
{
    int32_t  retVal = -1;
    char    *path   = NULL;

    if ((action != NULL) && (audioParam != NULL))
    {
        adjustVolume(*action, &path);
        audioVolumeWrite(gVolumeIndex);
        if(path != NULL)
        {
#ifdef FEATURE_SUBSYS_MP3_ENABLE
            retVal = mp3Play(path, audioParam);
#endif
        }
    }

    return retVal;
}
#endif

#if 0
uint32_t audioPlayTimeGet(char *path)
{
	FILE     *file       = NULL;
	uint8_t   buffer[10] = {0};
	uint32_t  length     = 0;
	uint8_t   version    = 0;

	uint16_t	bitrate = 0;
	uint8_t 	bitrateIndex = 0;
	uint32_t	headSize = 0;
	uint32_t	playTime = 0;//ms
	uint32_t	fileSize = 0;
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
	file = file_fopen(path, "r");
#endif
	if (file == NULL)
	{
		//printf("Failed to open the file \"%s\".\r\n", path);
		goto labelEnd;
	}

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
	file_fread(buffer, 10, 1, file);
	if (memcmp(buffer, "ID3", strlen("ID3")) == 0)
	{
		length = ((buffer[6] & 0x7F) << 21) + ((buffer[7] & 0x7F) << 14) + ((buffer[8] & 0x7F) << 7) + (buffer[9] & 0x7F);
		file_fseek(file, length, SEEK_CUR);
		file_fread(buffer, 4, 1, file);
		headSize = length + 10;
	}
	file_fstat((int)file, &buf);
    fileSize = buf->st_size;
	file_fclose(file);
#endif

	version = (buffer[1] >> 3) & 0x03;

	bitrateIndex = (buffer[2] >> 4) & 0x0f;
		
	if (version == 0x00) // MPEG2.5
	{
		bitrate = bitrateTable[2][bitrateIndex - 1];
	}
	else if (version == 0x2) // MPEG2
	{
		bitrate = bitrateTable[1][bitrateIndex - 1];
	}
	else if (version == 0x3) // MPEG1
	{
		bitrate = bitrateTable[0][bitrateIndex - 1];
	}
	else
	{
		bitrate = 128;
	}
	ECPLAT_PRINTF(UNILOG_PLAT_MP3, mp3PlayTimeGet, P_INFO, "bitrateIndex is %d ",bitrateIndex);
	playTime = (fileSize - headSize)  * 8 / bitrate;
	ECPLAT_PRINTF(UNILOG_PLAT_MP3, mp3PlayTimeGet_1, P_INFO, "playTime is %d ",playTime);

labelEnd:
	return playTime;
}

#else
uint32_t gCurMp3PlayTime = 0;

uint32_t audioPlayTimeGet(void) { return gCurMp3PlayTime; }

#endif

#if defined(FEATURE_HTTPC_ENABLE) && defined(FEATURE_HTTP_TLS_ENABLE) && \
    defined(FEATURE_SUBSYS_MEDIA_STREAM_ENABLE) &&                       \
    defined(FEATURE_SUBSYS_HTTP_DOWNLOADER_ENABLE)
HttpDownloader_t *gAudioStreamDownloader = NULL;
int stream_downloader_cb(DownloadInfo_t *info, uint8_t *data, uint32_t len,
                         void *userData)
{
    if(!gAudStreamRingBuf)
    {
        return 0;
    }
    if(info->status == HTTP_DOWNLOADER_STATUS_DOWNLOADING)
    {
        uint32_t free_size = 0;
        free_size = xEcRingGetOption(gAudStreamRingBuf, E_LRO_FREE_SIZE);
        if(free_size < len)
        {
            // ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, stream_downloader_cb_no_free,
            //               P_INFO, "free size is not enough");
            return -1;
        }
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, stream_downloader_cb_downloading,
                      P_INFO, "free %d, downloading: %d|%d", free_size,
                      info->cur_pos, info->total_pos);
        gDownloadLastPos = info->cur_pos;
        xEcRingWrite(gAudStreamRingBuf, data, len);
    }
    else if(info->status == HTTP_DOWNLOADER_STATUS_ERROR)
    {
        g_u8AudioDownloadOver = DLSTATE_ERROR;
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, stream_downloader_cb_error, P_INFO,
                      "download error");
    }
    else if(info->status == HTTP_DOWNLOADER_STATUS_STOPPED)
    {
        g_u8AudioDownloadOver = DLSTATE_COMPLETED;
        if(!gHttpPauseStatus)
        {
            gDownloadLastPos = 0;
        }
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, stream_downloader_cb_stopped, P_INFO,
                      "download stop");
    }
    else if(info->status == HTTP_DOWNLOADER_STATUS_COMPLETED)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, stream_downloader_cb_completed, P_INFO,
                      "download completed");
        gDownloadLastPos = 0;
        g_u8AudioDownloadOver = DLSTATE_COMPLETED;
    }

    return 0;
}

static int stream_downloader_start(char *url)
{
    if(gAudStreamRingBuf == NULL)
    {
        gAudStreamRingBuf = pxEcRingCreate(64 * 1024 * 4);
        if(gAudStreamRingBuf == NULL)
        {
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, stream_downloader_start_failed,
                          P_INFO, "gAudStreamRingBuf ecring created failed");
            return -1;
        }
    }

	if(gDownloadLastUrl)
	{
		if((strcmp(gDownloadLastUrl, url) != 0) && (gDownloadLastPos != 0))
		{
			ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, stream_downloader_url_not_match,
                          P_INFO, "url changed, play from 0");
			gDownloadLastPos = 0;
		}
		free(gDownloadLastUrl);
	}
	gDownloadLastUrl = malloc(strlen(url) + 1);
	memset(gDownloadLastUrl, 0, strlen(url) + 1);
	memcpy(gDownloadLastUrl, url, strlen(url));
    HttpDownloaderCfg_t cfg = {0};
    cfg.cb = stream_downloader_cb;
    cfg.section_cnt = DOWNLOAD_SECTION_CNT;
    cfg.start_pos = gDownloadLastPos;
    cfg.url = url;
    if(gDownloadLastPos != 0)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, stream_downloader_pause_resume, P_INFO,
                      "HTTP downloader resume, pos: %d, url: %s",
                      gDownloadLastPos, url);
    }

    cfg.user_data = NULL;
    gAudioStreamDownloader = http_downloader_create(&cfg);
    http_downloader_start(gAudioStreamDownloader);
    g_u8AudioDownloadOver = DLSTATE_DOWNLOADING;
    return 0;
}

#endif

static int32_t audioPlayProcess(QueueAudioT *queue)
{
    int32_t audPlayStatus = AV_RET_PLAY_ERROR;

    osSemaphoreAcquire(gAudioPlayI2sSem, AUD_STATUS_SEM_WAIT_TIMEROUT); /*20ms*/
    gCurPlayAudType = (queue->audType & 0x0f);
    osSemaphoreRelease(gAudioPlayI2sSem);
    printf("audioPlayProcess audType = %d\r\n", queue->audType);
    switch(queue->audType)
    {
#ifdef SPEAKER_APP
        case AUDIO_PLAY_VOLUME_MP3:
            audPlayStatus = playSpeakerVolume((uint8_t *)queue->buffer, &queue->audioParam);
            queue->buffer = NULL;
            break;
#endif

        case AUDIO_PLAY_MP3:
            if(queue->buffer != NULL)
            {
#if defined(FEATURE_SUBSYS_MP3_ENABLE)
                audPlayStatus = mp3Play(queue->buffer, &queue->audioParam);
#endif
                free(queue->buffer);
                queue->buffer = NULL;
            }
            break;

        case AUDIO_PLAY_STRING_MP3: {
#if defined(FEATURE_SUBSYS_MP3_ENABLE)
            audPlayStatus = mp3StringPlay(queue->audString, queue->length,
                                          &queue->audioParam);
#endif
        }
        break;

        case AUDIO_PLAY_STREAM_MP3: {
#if defined(FEATURE_HTTPC_ENABLE) && defined(FEATURE_HTTP_TLS_ENABLE) && \
    defined(FEATURE_SUBSYS_MEDIA_STREAM_ENABLE) &&                       \
    defined(FEATURE_SUBSYS_HTTP_DOWNLOADER_ENABLE)
            if(queue->buffer != NULL)
            {
                if(stream_downloader_start(queue->buffer) != 0)
                    return audPlayStatus;
            }
            else
            {
                // for cust to fill the stream ringbuffer;
            }
#endif
#if defined(FEATURE_SUBSYS_MP3_ENABLE)
            audPlayStatus = mp3StreamPlay(&queue->audioParam);
#endif
        }
        break;

#ifdef FEATURE_SUBSYS_MED_PCM_ENABLE
        case AUDIO_PLAY_PCM: {
            audPlayStatus =
                audPcmFilePlay((uint8_t *)queue->buffer, &queue->audioParam);
        }
        break;
        case AUDIO_PLAY_STRING_PCM: {
#ifdef MBTK_OPENCPU_SUPPORT
				audPlayStatus = ol_audPcmStringPlay(queue->audString, queue->length, &queue->audioParam, queue->audioParam.increase);
#else
				audPlayStatus = audPcmStringPlay(queue->audString, queue->length, &queue->audioParam);
#endif
        }
        break;
        case AUDIO_PLAY_STREAM_PCM: {
#if defined(FEATURE_HTTPC_ENABLE) && defined(FEATURE_HTTP_TLS_ENABLE) && \
    defined(FEATURE_SUBSYS_MEDIA_STREAM_ENABLE) &&                       \
    defined(FEATURE_SUBSYS_HTTP_DOWNLOADER_ENABLE)
            if(queue->buffer != NULL)
            {
                if(stream_downloader_start(queue->buffer) != 0)
                    return audPlayStatus;
            }
            else
            {
                // for cust to fill the stream ringbuffer;
            }
#endif
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audioPlayProcess_pcm_streamp_play,
                          P_INFO, "AUDIO_PLAY_STREAM_PCM");
            printf("audPcmStreamPlay\r\n");
            audPlayStatus = audPcmStreamPlay(&queue->audioParam);
        }
        break;

#endif
#ifdef FEATURE_SUBSYS_G711_ENABLE
        case AUDIO_PLAY_STREAM_G711: {
#if defined(FEATURE_HTTPC_ENABLE) && defined(FEATURE_HTTP_TLS_ENABLE) && \
    defined(FEATURE_SUBSYS_MEDIA_STREAM_ENABLE) &&                       \
    defined(FEATURE_SUBSYS_HTTP_DOWNLOADER_ENABLE)
            if(queue->buffer != NULL)
            {
                if(stream_downloader_start(queue->buffer) != 0)
                    return audPlayStatus;
            }
            else
            {
                // for cust to fill the stream ringbuffer;
            }
#endif
            audPlayStatus = audG711StreamPlay(&queue->audioParam);
        }
        break;
#endif
#ifdef FEATURE_SUBSYS_MED_WAV_ENABLE
        case AUDIO_PLAY_WAV: {
            audPlayStatus =
                audWavFilePlay((uint8_t *)queue->buffer, &queue->audioParam);
        }
        break;
        case AUDIO_PLAY_STRING_WAV: {
            audPlayStatus = audWavStringPlay(queue->audString, queue->length,
                                             &queue->audioParam);
        }
        break;
#endif

#ifdef FEATURE_SUBSYS_AMR_ENABLE
        case AUDIO_PLAY_AMR: {
            audPlayStatus = amrPlay(queue->audioParam.toneFlag, queue->buffer);
        }
        break;
        case AUDIO_PLAY_STRING_AMR: {
            audPlayStatus =
                amrPlayString(queue->audioParam.toneFlag,
                              (char *)queue->audString, queue->length);
        }
        break;

        case AUDIO_PLAY_STREAM_AMR: {

#if defined(FEATURE_HTTPC_ENABLE) && defined(FEATURE_HTTP_TLS_ENABLE) && \
    defined(FEATURE_SUBSYS_MEDIA_STREAM_ENABLE) &&                       \
    defined(FEATURE_SUBSYS_HTTP_DOWNLOADER_ENABLE)
            if(queue->buffer != NULL)
            {
                if(stream_downloader_start(queue->buffer) != 0)
                    return audPlayStatus;
            }
            else
            {
                // for cust to fill the stream ringbuffer;
            }
#endif /* FEATURE_SUBSYS_HTTP_DOWNLOADER_ENABLE */
            audPlayStatus = amrPlayStream(queue->audioParam.toneFlag);
        }
        break;

#endif /* FEATURE_SUBSYS_AMR_ENABLE */
#ifdef FEATURE_SUBSYS_TTS_ENABLE
        case AUDIO_PLAY_TTS: {
            audPlayStatus = ttsPlay(queue->audioParam.toneFlag,
                                    (char *)queue->audString, queue->length);
        }
        break;
#endif
        default:
            break;
    }
    osSemaphoreAcquire(gAudioPlayI2sSem, AUD_STATUS_SEM_WAIT_TIMEROUT); /*20ms*/
    gCurPlayAudType = AUDIO_PLAY_CODEC_TYPE_IDLE;
    osSemaphoreRelease(gAudioPlayI2sSem);
    return audPlayStatus;
}

uint32_t sampleRateConvert(uint32_t rate, bool toEnum)
{
    if(toEnum == true)
    {
        switch(rate)
        {
            case 8000:
                return SAMPLERATE_8K;
            case 16000:
                return SAMPLERATE_16K;
            case 22050:
                return SAMPLERATE_22_05K;
            case 24000:
                return SAMPLERATE_24K;
            case 32000:
                return SAMPLERATE_32K;
            case 44100:
                return SAMPLERATE_44_1K;
            case 48000:
                return SAMPLERATE_48K;
            case 96000:
                return SAMPLERATE_96K;
            default:
                return 0;
        }
    }
    else
    {
        switch(rate)
        {
            case SAMPLERATE_8K:
                return 8000;
            case SAMPLERATE_16K:
                return 16000;
            case SAMPLERATE_22_05K:
                return 22050;
            case SAMPLERATE_24K:
                return 24000;
            case SAMPLERATE_32K:
                return 32000;
            case SAMPLERATE_44_1K:
                return 44100;
            case SAMPLERATE_48K:
                return 48000;
            case SAMPLERATE_96K:
                return 96000;
            default:
                return 0;
        }
    }
}

medSrcT *gMedStringSrc = NULL;

medSrcT *medFileToString(char *path)
{
    EC_ASSERT(path != NULL, path, 0, 0);
    struct stat buf = {0};
    FILE *file = NULL;
    uint32_t fileSize = 0;
    uint32_t readSize = 0;

    file = file_fopen(path, "r");
    if(!file)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, medFileToString_invalid_1, P_ERROR,
                      "file open fail ");
        return NULL;
    }
    file_fstat((int)file, &buf);
    fileSize = buf.st_size;

    if(fileSize > 64 * 1024)
    {
        file_fclose(file);
        return NULL;
    }
    if(gMedStringSrc)
    {
        OsaFreeMemory(&gMedStringSrc);
    }

    gMedStringSrc = (medSrcT *)OsaAllocZeroMemory(sizeof(medSrcT) +
                                                  fileSize * sizeof(uint8_t));
    gMedStringSrc->srcLen = 0;

    do
    {
        readSize = file_fread(gMedStringSrc->src + gMedStringSrc->srcLen, 1,
                              1152, file);
        // ECPLAT_DUMP(UNILOG_PLA_APP, audioDataInput_dump, P_ERROR, "",
        // readSize, gMedStringSrc->src + gMedStringSrc->srcLen);
        gMedStringSrc->srcLen = gMedStringSrc->srcLen + readSize;
    } while(file_ftell(file) < fileSize);
    file_fclose(file);
    return gMedStringSrc;
}

INT32 EC_audioStartPlayFile(char *file_name, uint8_t audType,
                            audioParamT audParam, AudioCallbackT play_cb_func)
{
    errCodeT ret = EC_RET_OK;
    ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, EC_AudioStartPlayFile_0, P_INFO,
                  "[audio] file_name:%s,format:%d,play_cb_func:%p", file_name,
                  audType, play_cb_func);

    if(file_name == NULL ||
       audParam.aud_volume > 100)  // modified by Chen Hongshun
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, EC_AudioStartPlayFile_1, P_INFO,
                      "[audio] error -> file_name");
        return EC_RET_ERR_PARAM;
    }
    QueueAudioT queue = {0};
    if(gAudioQueue != NULL && file_name != NULL)
    {
        queue.callback = play_cb_func;
        queue.audType = audType;
        queue.length = strlen(file_name);
        queue.buffer = malloc(queue.length + 1);
        queue.audioParam.rate = audParam.rate;
        queue.audioParam.dual = audParam.dual;
        queue.audioParam.BitWidth = audParam.BitWidth;
        queue.audioParam.increase = audParam.increase;
        queue.audioParam.aud_volume = audParam.aud_volume;
        queue.audioParam.toneFlag = audParam.toneFlag;
        if(queue.buffer != NULL)
        {
            memset(queue.buffer, 0, queue.length + 1);
            memcpy(queue.buffer, file_name, queue.length);
            ret = osMessageQueuePut(gAudioQueue, &queue, 0, osWaitForever);
        }
    }
    return ret;
}

INT32 EC_audioStartPlayStream(UINT8 *url, UINT32 strLen, uint8_t audType,
                              audioParamT audParam, AudioCallbackT play_cb_func)
{
    errCodeT ret = EC_RET_OK;
    ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, EC_audioStartPlayStream_0, P_INFO,
                  "[audio] format:%d,url:%s,strLen:%d", audType, url, strLen);

    QueueAudioT queue = {0};
    if(gAudioQueue != NULL)
    {
        queue.callback = play_cb_func;
        queue.audType = audType;
        queue.length = strLen;
        if(queue.length > 0 && url)
        {
            queue.buffer = malloc(queue.length + 1);
            if(!queue.buffer) return EC_RET_ERR_RESOURCE;
            memset(queue.buffer, 0, queue.length + 1);
            memcpy(queue.buffer, url, queue.length);
        }
        queue.audioParam.rate = audParam.rate;
        queue.audioParam.BitWidth = audParam.BitWidth;
        queue.audioParam.dual = audParam.dual;
        queue.audioParam.increase = audParam.increase;
        queue.audioParam.aud_volume = audParam.aud_volume;
        queue.audioParam.toneFlag = audParam.toneFlag;
        ret = osMessageQueuePut(gAudioQueue, &queue, 0, osWaitForever);
    }
    return ret;
}

INT32 EC_audioStartPlayString(UINT8 *String, UINT32 strLen, uint8_t audType,
                              audioParamT audParam, AudioCallbackT play_cb_func)
{
    errCodeT ret = EC_RET_OK;
    ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, EC_audioStartPlayString_0, P_INFO,
                  "[audio] format:%d,play_cb_func:%p,String:%p", audType,
                  play_cb_func, String);

    if(audParam.aud_volume > 100 || String == NULL)  // modified by Chen
                                                     // Hongshun
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, EC_audioStartPlayString_1, P_INFO,
                      "[audio] error -> String");
        return EC_RET_ERR_PARAM;
    }

    if(strLen == 0)
    {
        if(play_cb_func != NULL)
        {
            play_cb_func(0);
        }
        return EC_RET_ERR_PARAM;
    }

    QueueAudioT queue = {0};
    if(gAudioQueue != NULL)
    {
        queue.callback = play_cb_func;
        queue.audType = audType;
        queue.length = strLen;
        queue.audString = String;
        queue.audioParam.rate = audParam.rate;
        queue.audioParam.BitWidth = audParam.BitWidth;
        queue.audioParam.dual = audParam.dual;
        queue.audioParam.increase = audParam.increase;
        queue.audioParam.aud_volume = audParam.aud_volume;
        queue.audioParam.toneFlag = audParam.toneFlag;

        ret = osMessageQueuePut(gAudioQueue, &queue, 0, osWaitForever);
    }

    return ret;
}

INT32 EC_audioStopPlaying(uint8_t audType, bool pauseFlag)
{
    switch(audType)
    {
#if defined(FEATURE_SUBSYS_MED_PCM_ENABLE)
        case AUDIO_PLAY_CODEC_TYPE_PCM:
            audPcmPlayStop();
#endif
            break;
#if defined(FEATURE_SUBSYS_MP3_ENABLE)
        case AUDIO_PLAY_CODEC_TYPE_MP3:
            mp3PlayStop(pauseFlag);
#endif
            break;
#if defined(FEATURE_SUBSYS_AMR_ENABLE)
        case AUDIO_PLAY_CODEC_TYPE_AMR:
            amrPlayStop();
#endif
            break;
#if defined(FEATURE_SUBSYS_MED_WAV_ENABLE)
        case AUDIO_PLAY_CODEC_TYPE_WAV:
            audWavPlayStop();
#endif
            break;
        case AUDIO_PLAY_CODEC_TYPE_AAC:
            break;
#ifdef FEATURE_SUBSYS_TTS_ENABLE
        case AUDIO_PLAY_CODEC_TYPE_TTS:
            ttsStopPlay(false);
#endif
            break;
#ifdef FEATURE_SUBSYS_G711_ENABLE
        case AUDIO_PLAY_CODEC_TYPE_G711:
            audG711PlayStop();
            break;
#endif
        default:
            break;
    }
#ifdef FEATURE_SUBSYS_MEDIA_STREAM_ENABLE
    gHttpPauseStatus = pauseFlag;
    if(!pauseFlag)
    {
        gDownloadLastPos = 0;
    }
#endif

    return 0;
}

bool EC_audioIsBusy(void) { return gAudioBusy; }

UINT16 EC_audioGetVolume(void) { return gVolume; }

bool EC_audioPlayChkLoop(void) { return gLocalPlayLoop; }
void EC_audioPlaySetLoop(bool onoff) { gLocalPlayLoop = onoff; }

uint32_t toneStopFlag = 0;

void audioStopPlayCallback(void *param)
{
    uint8_t audType = ALARM_SOUND_CODEC;
    ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audioStopPlayCallback, P_INFO,
                  "stop play tone ");
     toneStopFlag = 1;
    if(!EC_audioIsBusy())
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audioStopPlayCallback_1, P_INFO,"send msg to ccio ");
        audioMsgApp2Ccio(0);
    }
   
#ifdef FEATURE_SUBSYS_TTS_ENABLE
    ttsStopPlay(true);
#endif
    EC_audioStopPlaying(audType, false);
    audioUnRegisterStopPlayCb();
}

void toneAudCallBack(int32_t result)
{
    char ttsToneString[128] = {0};
    uint32_t srcLen = 0;
    if(toneStopFlag == 1)
    {
        toneStopFlag = 0;
        return;
    }

    audioParamT audParam = {0};
    uint8_t audType = 0;
    audParam.toneFlag = true;
    memset(ttsToneString, 0x00, 128 * sizeof(char));
#if ALARM_TONE_NEED_TTS
    ListCurrCallInfo pListCurrCallInfo;
    appListCurrentCallSync(&pListCurrCallInfo);
    ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, toneAudCallBack_1, P_INFO,
                  "call number[%s]",
                  pListCurrCallInfo.callInfoList[0].dialNumStr);
    sprintf(ttsToneString, "%s来电话了",
            pListCurrCallInfo.callInfoList[0].dialNumStr);
    srcLen = pListCurrCallInfo.callInfoList[0].dialNumStrLen + 4 * 3 /*UTF8*/;
#endif
    audType = (AUDIO_PLAY_STRING << 4 | AUDIO_PLAY_CODEC_TYPE_TTS);
    extern void toneTTSCallback(int32_t result);
    EC_audioStartPlayString((UINT8 *)ttsToneString, srcLen, audType, audParam,
                            toneTTSCallback);
}

void toneTTSCallback(int32_t result)
{
    if(toneStopFlag == 1)
    {
        toneStopFlag = 0;
        return;
    }
    audioParamT audParam = {0};
    uint8_t audType = 0;
    audParam.toneFlag = true;
    audType = (AUDIO_PLAY_FILE << 4 | ALARM_SOUND_CODEC);
    audParam.dual = false;
    audParam.increase = false;
    audParam.aud_volume = 40;

    EC_audioStartPlayFile(audioToneSrcGet(), audType, audParam,
                          toneAudCallBack);
}
void audioToneCallback(void *param)
{
    char ttsToneString[128] = {0};
    uint32_t srcLen = 0;
    uint8_t curAudType = AUDIO_PLAY_CODEC_TYPE_IDLE;
    ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audioToneCallback, P_INFO,
                  "start play tone ");
    toneStopFlag = 0;
    /*clean audio queue*/
    osMessageQueueReset(gAudioQueue);
    /*get current play type add stop it*/
    osSemaphoreAcquire(gAudioPlayI2sSem, AUD_STATUS_SEM_WAIT_TIMEROUT); /*20ms*/
    curAudType = gCurPlayAudType;
    osSemaphoreRelease(gAudioPlayI2sSem);

    EC_audioStopPlaying(curAudType, true);

    /*stop record*/
#ifdef FEATURE_SUBSYS_AMR_RECORD_ENABLE
    amrRecordStop(AUDIO_AMR_RECORD_STOP_BY_VOLTE);
#endif

    /*tone play*/
    audioRegisterStopPlayCb(audioStopPlayCallback);
    memset(ttsToneString, 0x00, 128 * sizeof(char));
#if ALARM_TONE_NEED_TTS
    ListCurrCallInfo pListCurrCallInfo;
    appListCurrentCallSync(&pListCurrCallInfo);
    ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audioToneCallback_1, P_INFO,
                  "call number[%s]",
                  pListCurrCallInfo.callInfoList[0].dialNumStr);
    sprintf(ttsToneString, "%s来电话了",
            pListCurrCallInfo.callInfoList[0].dialNumStr);
    srcLen = pListCurrCallInfo.callInfoList[0].dialNumStrLen + 4 * 3 /*UTF8*/;
#endif
    audioParamT audParam = {0};
    uint8_t audType = 0;
    audParam.toneFlag = true;

    audType = (AUDIO_PLAY_STRING << 4 | AUDIO_PLAY_CODEC_TYPE_TTS);
    EC_audioStartPlayString((UINT8 *)ttsToneString, srcLen, audType, audParam,
                            toneTTSCallback);
}

static void threadAudio(void *argument)
{
    QueueAudioT queue = {0};
    static uint8_t vote = 0xFF;
#ifdef FEATURE_SUBSYS_POWER_MANAGER_ENABLE
    uint8_t prohibitionHandle = 0xFF;
#endif
    int32_t status = AV_RET_PLAY_ERROR;
    gAudioQueue = osMessageQueueNew(QUEUE_SIZE_AUDIO, sizeof(queue), NULL);
    if(gAudioQueue == NULL)
    {
        SYSLOG_EMERG("Failed to create queue for gAudioQueue.\r\n");
        goto labelEnd;
    }

#ifdef FEATURE_SUBSYS_VOLUME_ENABLE
    volumeManagerInit();
#endif
#ifdef SPEAKER_APP
    playSpeakerWelcome();
#endif

    audioRegisterToneCb(audioToneCallback);
    slpManRegisterUsrdefinedRestoreCb(restoreCallback, NULL);
    slpManApplyPlatVoteHandle("audio", &vote);
#ifdef FEATURE_SUBSYS_POWER_MANAGER_ENABLE
    powerManagerProhibitionCreate(&prohibitionHandle);
#endif
    gAudioPlayI2sSem = osSemaphoreNew(1U, 0, PNULL);
    EC_ASSERT(gAudioPlayI2sSem, gAudioPlayI2sSem, 0, 0);
#ifdef FEATURE_SUBSYS_TTS_ENABLE
    ttsInit();
#endif

#ifdef SPEAKER_APP
    gVolume = audioVolumeRead();
    osDelay(3000);
#else
    gAudioReady = true;
#endif

    ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, threadAudio, P_INFO, "threadAudio start ");
    while(1)
    {
        memset(&queue, 0, sizeof(queue));
        if(osMessageQueueGet(gAudioQueue, &queue, 0, osWaitForever) == osOK)
        {
            slpManPlatVoteDisableSleep(vote, SLP_SLP1_STATE);
#ifdef FEATURE_SUBSYS_POWER_MANAGER_ENABLE
            powerManagerProhibitionSet(prohibitionHandle, PROHIBIT_ENTER_SLEEP);
#endif
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, threadAudio_0, P_INFO,
                          "start process 0x%x,toneFlag [%d]", queue.audType,
                          queue.audioParam.toneFlag);

            if(gWakeup == true)
            {
                gWakeup = false;
#ifndef TYPE_EC718M
                CLOCK_clockEnable(PSRAM_HCLK);
                CLOCK_clockEnable(PCACHE_HCLK);
                CLOCK_clockEnable(CLK_PSRAM);
#endif
            }
            gCurMp3PlayTime = 0;
            gAudioBusy = true;
            status = audioPlayProcess(&queue);
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, threadAudio_1, P_INFO,
                          "toneStopFlag [%d]", toneStopFlag);
            if(queue.audioParam.toneFlag && (toneStopFlag == 1))
            {
                audioMsgApp2Ccio(0);
            }
            gAudioBusy = false;
#if defined(FEATURE_HTTPC_ENABLE) && defined(FEATURE_HTTP_TLS_ENABLE) && \
    defined(FEATURE_SUBSYS_MEDIA_STREAM_ENABLE) && defined(FEATURE_SUBSYS_HTTP_DOWNLOADER_ENABLE)
            g_u8AudioDownloadOver = DLSTATE_NOT_STARTED;
            if(gAudioStreamDownloader != NULL)
            {
                http_downloader_stop(gAudioStreamDownloader);
                http_downloader_destroy(gAudioStreamDownloader);
                gAudioStreamDownloader = NULL;
            }
#endif

            if(queue.callback != NULL)
            {
                queue.callback(status);
            }
            if(queue.buffer != NULL)
            {
                free(queue.buffer);
                queue.buffer = NULL;
            }
#ifdef FEATURE_SUBSYS_MEDIA_STREAM_ENABLE
            if(gAudStreamRingBuf != NULL)
            {
                xEcRingClear(gAudStreamRingBuf);
            }
#endif
#ifdef FEATURE_SUBSYS_POWER_MANAGER_ENABLE
            powerManagerProhibitionSet(prohibitionHandle, PROHIBIT_NOTHING);
            SYSLOG_DEBUG("prohibitionHandle=%d\r\n", prohibitionHandle);
#endif
            slpManPlatVoteEnableSleep(vote, SLP_SLP1_STATE);
        }
    }

labelEnd:
#if 0 // Service Manager
    osThreadExit();
#else
    Service_stop("service:/threadAudio");
#endif
}

int32_t audioInit(void)
{
    osThreadAttr_t threadAttr = {0};

    memset(&threadAttr, 0, sizeof(threadAttr));
    threadAttr.name = "threadAudio";
    threadAttr.stack_size = THREAD_STACK_SIZE_AUDIO;
    threadAttr.priority = osPriorityBelowNormal7;
#ifdef THREAD_STATIC
    threadAttr.stack_mem = gAudioThreadStackMem;
    threadAttr.cb_mem = &gAudioThreadCbMem;
    threadAttr.cb_size = sizeof(StaticTask_t);
#endif
#if 1
    if(osThreadNew(threadAudio, NULL, &threadAttr) == NULL)
#else
    char serviceName[32] = {0};
    snprintf(serviceName, sizeof(serviceName), "service:/%s", threadAttr.name);
    Service_reg(serviceName, threadAudio, NULL, threadAttr.cb_mem,
                threadAttr.cb_size, threadAttr.stack_mem, threadAttr.stack_size,
                threadAttr.priority);
    if((osThreadId_t)Service_start(serviceName) == NULL)
#endif
    {
        SYSLOG_EMERG("Failed to create thread for audio.\r\n");
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audioInit, P_ERROR,
                      "Failed to create thread for audio ");
    }
    return 0;
}
