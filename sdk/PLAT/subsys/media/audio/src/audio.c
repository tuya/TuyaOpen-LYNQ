/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    audio.c
 * Description:  EC618 lwm2m demo entry source file
 * History:      Rev1.0   2018-10-12
 *
 ****************************************************************************/
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

#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
#include "flashex.h"
#endif
#include "media.h"
#include "ec_ring.h"

#if defined(FEATURE_HTTPC_ENABLE) && defined(FEATURE_HTTP_TLS_ENABLE)
#include "ps_lib_api.h"
#include "networkmgr.h"
#include "HTTPClient.h"
#endif

#ifdef FEATURE_SUBSYS__MP3_ENABLE
#include "mp3.h"
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

#include "medDataHandle.h"

#define THREAD_STACK_SIZE_AUDIO     		(9 * 1024)
#define THREAD_STACK_SIZE_OPER_AUDIO     	(3 * 1024)
#define THREAD_STACK_SIZE_AUDIO_TEST     	(20 * 1024)
#define QUEUE_SIZE_AUDIO            		100
#define VOLUME_INDEX_MIN            		0
#define VOLUME_INDEX_MAX            		4
#define VOLUME_INDEX_DEFAULT                VOLUME_INDEX_MIN

#define FILE_READ_BUFFER_SIZE        		1500
#define OUTPUT_STRING_LEN 					7000

#define INI_KEY_VOLUME              "volume"

#ifdef THREAD_STATIC
static StaticTask_t       gAudioThreadCbMem                                      = {0};
static StaticTask_t       gStreamMedThreadCbMem                                  = {0};
PLAT_FPSRAM_ZI_CUST static uint8_t            gAudioThreadStackMem[THREAD_STACK_SIZE_AUDIO]          = {0};
PLAT_FPSRAM_ZI_CUST static uint8_t            gStreamMedThreadStackMem[THREAD_STACK_SIZE_AUDIO]      = {0};
#endif
static osMessageQueueId_t gAudioQueue = NULL;
static bool               gAudioReady   = false;
static bool               gAudioBusy    = false;
static bool               gWakeup       = false;
static uint8_t            gVolumeIndex  = VOLUME_INDEX_DEFAULT;
static const uint16_t     gVolumeList[] = {2, 5, 10, 20, 30};
static uint16_t           gVolume       	= gVolumeList[VOLUME_INDEX_DEFAULT];
static uint16_t			  gCustToneVolume 	= gVolumeList[VOLUME_INDEX_DEFAULT];

osMessageQueueId_t gOperateAudioQueue = NULL;
osSemaphoreId_t  gAudioPlayI2sSem = NULL;
//tuya
static osThreadId_t gAudioPlayThread = NULL;	

#ifdef FEATURE_SUBSYS_MEDIA_STREAM_ENABLE
typedef struct
{   
	bool endFlag;
} QueueStreamDlT;

osMessageQueueId_t gStreamDlQueue = NULL;
static osTimerId_t streamDlTimer = NULL;
ecRingT *gAudStreamRingBuf = NULL;
#endif

char 		*gCustToneSrc = NULL;

extern uint8_t mp3String[24600];
extern uint8_t audio16bit_8k[];
extern osSemaphoreId_t  gI2sSemInt;
extern osMessageQueueId_t gToneDecodeQueue;


#if defined (PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST==1)
void CustPsramCodeAndDataInit(void);
#endif

static void restoreCallback(void *p_data, slpManLpState state)
{
    gWakeup = true;
}
static void audioCallback(void)
{
    gAudioReady = true;
}

void audioToneSrcSet(const char *src)
{
	/*to check realloc */
	char *tmpPtr = NULL;
	uint16_t srcLen = 0;
	if(src == NULL)
		return;
	srcLen = strlen(src);
	if(gCustToneSrc == NULL)
	{
		gCustToneSrc = malloc(srcLen+1);
		EC_ASSERT(gCustToneSrc != NULL,0,0,0);
	}
	else
	{
		tmpPtr = realloc(gCustToneSrc, srcLen+1);
		EC_ASSERT(gCustToneSrc == tmpPtr,gCustToneSrc,tmpPtr,0);
	}
	memset(gCustToneSrc,0x00,srcLen+1);
	memcpy(gCustToneSrc,src,srcLen);

	return;
}

char * 	audioToneSrcGet(void)
{
	return (gCustToneSrc == NULL ? ALARM_SOUND : gCustToneSrc);
}

void 	audioToneVolumeSet(uint16_t value)
{
	gCustToneVolume = value;
}
uint16_t audioToneVolumeGet(void)
{
	return gCustToneVolume;
}

void audioStopPlay(void)
{
	medDataHandleStop();
#if defined(FEATURE_HTTPC_ENABLE) && defined(FEATURE_HTTP_TLS_ENABLE)	
	if(gStreamDlQueue != NULL)
	{
		QueueStreamDlT queue = {0};
		queue.endFlag = true;		
		osMessageQueuePut(gStreamDlQueue, &queue, 0, osWaitForever);
	}
#endif
	return;
}

void audioPlayMp3(char *path, AudioCallbackT callback, bool increase, bool dual)
{
    QueueAudioT queue  = {0};
    uint32_t    length = 0;
    if ((path != NULL) && (gAudioQueue != NULL))
    {
        length         = strlen(path);
		if(strcasestr(path,"http://") || strcasestr(path,"https://"))
			queue.audType     = AUDIO_PLAY_STREAM_MP3;
		else
        	queue.audType     = AUDIO_PLAY_MP3;
        queue.buffer   = malloc(length + 1);
        queue.callback = callback;
        queue.audioParam.increase = increase;
        queue.audioParam.dual     = dual;
        if (queue.buffer != NULL)
        {
            memset(queue.buffer, 0,    length + 1);
            memcpy(queue.buffer, path, length);
            osMessageQueuePut(gAudioQueue, &queue, 0, osWaitForever);
        }
    }
}

void audioPlayAmr(char *path, AudioCallbackT callback)
{
    QueueAudioT queue  = {0};
    uint32_t    length = 0;
    if ((path != NULL) && (gAudioQueue != NULL))
    {
        length         = strlen(path);
        queue.audType     = AUDIO_PLAY_AMR;
        queue.buffer   = malloc(length + 1);
        queue.callback = callback;
        if (queue.buffer != NULL)
        {
            memset(queue.buffer, 0,    length + 1);
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
    QueueAudioT queue  = {0};
    uint32_t    length = 0;

    if ((text != NULL) && (gAudioQueue != NULL))
    {
        memset(&queue, 0, sizeof(queue));
        length                   = strlen(text);
        queue.audType          = AUDIO_PLAY_TTS;
        queue.buffer   = malloc(length + 1);
        queue.callback = callback;
        if (queue.buffer != NULL)
        {
            memset(queue.buffer, 0,    length + 1);
            memcpy(queue.buffer, text, length);
            osMessageQueuePut(gAudioQueue, &queue, 0, osWaitForever);
        }
    }
}
#endif

bool audioIsReady(void)
{
    return gAudioReady;
}

bool audioIsBusy(void)
{
    return gAudioBusy;
}

#ifdef SPEAKER_APP
void audioAdjustVolume(uint8_t action)
{
    QueueAudioT     queue       = {0};
    static uint8_t  sAction[32] = {0};
    static uint32_t sIndex      = 0;

    if  (gAudioQueue != NULL)
    {
        memset(&queue, 0, sizeof(queue));
        sAction[sIndex] = action;
        queue.type      = AUDIO_PLAY_VOLUME;
        queue.buffer    = (char *)(&sAction[sIndex]);
        queue.callback  = NULL;
        osMessageQueuePut(gAudioQueue, &queue, 0, 0);
        sIndex = (sIndex + 1) % sizeof(sAction);
    }
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

uint32_t audioPlayTimeGet(void)
{
	return gCurMp3PlayTime;
}

#endif


#if defined(FEATURE_HTTPC_ENABLE) && defined(FEATURE_HTTP_TLS_ENABLE) && defined(FEATURE_SUBSYS_MEDIA_STREAM_ENABLE)
static HttpClientContext        gHttpClient = {0};
#define HTTP_RECV_BUF_SIZE      (1501)
#define HTTP_HEAD_BUF_SIZE      (800)

static const char *testCaCrt = \
{
    \
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIDSjCCAjKgAwIBAgIQRK+wgNajJ7qJMDmGLvhAazANBgkqhkiG9w0BAQUFADA/\r\n" \
    "MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\r\n" \
    "DkRTVCBSb290IENBIFgzMB4XDTAwMDkzMDIxMTIxOVoXDTIxMDkzMDE0MDExNVow\r\n" \
    "PzEkMCIGA1UEChMbRGlnaXRhbCBTaWduYXR1cmUgVHJ1c3QgQ28uMRcwFQYDVQQD\r\n" \
    "Ew5EU1QgUm9vdCBDQSBYMzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\r\n" \
    "AN+v6ZdQCINXtMxiZfaQguzH0yxrMMpb7NnDfcdAwRgUi+DoM3ZJKuM/IUmTrE4O\r\n" \
    "rz5Iy2Xu/NMhD2XSKtkyj4zl93ewEnu1lcCJo6m67XMuegwGMoOifooUMM0RoOEq\r\n" \
    "OLl5CjH9UL2AZd+3UWODyOKIYepLYYHsUmu5ouJLGiifSKOeDNoJjj4XLh7dIN9b\r\n" \
    "xiqKqy69cK3FCxolkHRyxXtqqzTWMIn/5WgTe1QLyNau7Fqckh49ZLOMxt+/yUFw\r\n" \
    "7BZy1SbsOFU5Q9D8/RhcQPGX69Wam40dutolucbY38EVAjqr2m7xPi71XAicPNaD\r\n" \
    "aeQQmxkqtilX4+U9m5/wAl0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNV\r\n" \
    "HQ8BAf8EBAMCAQYwHQYDVR0OBBYEFMSnsaR7LHH62+FLkHX/xBVghYkQMA0GCSqG\r\n" \
    "SIb3DQEBBQUAA4IBAQCjGiybFwBcqR7uKGY3Or+Dxz9LwwmglSBd49lZRNI+DT69\r\n" \
    "ikugdB/OEIKcdBodfpga3csTS7MgROSR6cz8faXbauX+5v3gTt23ADq1cEmv8uXr\r\n" \
    "AvHRAosZy5Q6XkjEGB5YGV8eAlrwDPGxrancWYaLbumR9YbK+rlmM6pZW87ipxZz\r\n" \
    "R8srzJmwN0jP41ZL9c8PDHIyh8bwRLtTcm1D9SZImlJnt1ir/md2cXjbDaJWFBM5\r\n" \
    "JDGFoqgCWjBH4d1QB7wCCZAA62RjYJsWvIjJEubSfZGL+T0yjWW06XyxV3bqxbYo\r\n" \
    "Ob8VZRzI9neWagqNdwvYkQsEjgfbKbYK7p2CNTUQ\r\n" \
    "-----END CERTIFICATE-----"
};

void streamDlTimerCb(void *argument)
{
	QueueStreamDlT queue = {0};
	if(gStreamDlQueue != NULL)
		osMessageQueuePut(gStreamDlQueue, &queue, 0, osWaitForever);
}

static INT32 httpGetData(CHAR *getUrl, CHAR *buf, UINT32 len)
{
    HTTPResult result = HTTP_INTERNAL;
    HttpClientData    clientData = {0};
    UINT32 count = 0;
    uint16_t headerLen = 0;
	
	INT32     freeSize = 0;
	QueueStreamDlT queue = {0};
	
    EC_ASSERT(buf != NULL,0,0,0);

    clientData.headerBuf = malloc(HTTP_HEAD_BUF_SIZE);
    clientData.headerBufLen = HTTP_HEAD_BUF_SIZE;
    clientData.respBuf = buf;
    clientData.respBufLen = len;

    result = httpSendRequest(&gHttpClient, getUrl, HTTP_GET, &clientData);
    if (result != HTTP_OK)
	{
        goto exit;
	}
	audioParamT audParam = {0};
	audParam.dual = false;
	audParam.increase = false;
	audParam.aud_volume = 70;
	//if(streamDlTimer == NULL)
	//{
	//	streamDlTimer = osTimerNew((osTimerFunc_t)streamDlTimerCb,osTimerPeriodic,NULL,NULL);
	//	osTimerStart(streamDlTimer, 10);
	//}
    do {
		if(osMessageQueueGet(gStreamDlQueue, &queue, 0, 20) == osOK)
    	{
    		if(queue.endFlag == true)
			{				
				break;
			}
		}
		freeSize = xEcRingGetOption(gAudStreamRingBuf,E_LRO_FREE_SIZE);
		if(freeSize >= HTTP_RECV_BUF_SIZE)
		{
			memset(clientData.headerBuf, 0, clientData.headerBufLen);
	        memset(clientData.respBuf, 0, clientData.respBufLen);
	        result = httpRecvResponse(&gHttpClient, &clientData);
	        if(result == HTTP_OK || result == HTTP_MOREDATA){
	            headerLen = strlen(clientData.headerBuf);
	            if(headerLen > 0)
	            {
	                //printf("total content length=%d \n",headerLen);
	            }

	            if(clientData.blockContentLen > 0)
	            {
					xEcRingWrite(gAudStreamRingBuf,(uint8_t*)clientData.respBuf,clientData.blockContentLen);
	            }
	            count += clientData.blockContentLen;
	        }
		}

    } while (result == HTTP_MOREDATA || result == HTTP_CONN);
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, httpGetData, P_INFO, "count is %d ",count);
    if (gHttpClient.httpResponseCode < 200 || gHttpClient.httpResponseCode > 404)
    {
        //printf("invalid http response code=%d\n",gHttpClient.httpResponseCode);
    } else if (count == 0 || count != clientData.recvContentLength) {
        //printf("data not receive complete \n");
    } else {
        //printf("receive success \n");
    }
exit:
    free(clientData.headerBuf);
    return result;
}


static void threadStreamMedDlAud(void *argument)
{
	//char *url = "https://cdn9003.iflyos.cn/audio/story/tonghua/281516.mp3";
	EC_ASSERT(argument != NULL,argument,0,0);
	char *url = OsaAllocZeroMemory(strlen((char *)argument) + 1);
	memcpy(url,(char *)argument,strlen((char *)argument));

	HTTPResult result = HTTP_INTERNAL;
	NmAtiNetifInfo netInfo;
	CHAR *recvBuf = malloc(HTTP_RECV_BUF_SIZE);

    gHttpClient.caCert= (char*)testCaCrt;
    gHttpClient.caCertLen= strlen(testCaCrt)+1;
    gHttpClient.timeout_s = 2;
    gHttpClient.timeout_r = 20;
    gHttpClient.ignore = 1;

	gHttpClient.seclevel = 0;
	gHttpClient.sni = 1;
	gHttpClient.ciphersuite[0] = 0xFFFF;
	if(gAudStreamRingBuf == NULL)
	{
		if(recvBuf != NULL)
		{
			free(recvBuf);
			recvBuf = NULL;
		}
		if(url)
			OsaFreeMemory(&url);
		return;
	}
	else
		xEcRingClear(gAudStreamRingBuf);
    while(!((netInfo.netStatus == NM_NETIF_ACTIVATED)&&((netInfo.ipType == NM_NET_TYPE_IPV4)||(netInfo.ipType == NM_NET_TYPE_IPV6)||(netInfo.ipType == NM_NET_TYPE_IPV4V6))))
    {
        appGetNetInfoSync(0, &netInfo);
		osDelay(5);
    }	
    do
    {
       	if (url != NULL)
		{				
			result = httpConnect(&gHttpClient, url);
			if (result == HTTP_OK)
			{
				httpGetData(url, recvBuf, HTTP_RECV_BUF_SIZE);
				httpClose(&gHttpClient);				
			}
		}

		if(url)
			OsaFreeMemory(&url);
		if(recvBuf != NULL)
		{
			free(recvBuf);
			recvBuf = NULL;
		}
		if(osTimerIsRunning(streamDlTimer))
			osTimerStop(streamDlTimer);
		osTimerDelete(streamDlTimer);
		streamDlTimer = NULL;
    }while(0);
	osThreadExit();
}

INT8 threadStreamMedTaskInit(void *argument)
	{
		osThreadAttr_t threadAttr = {0};
		
		QueueStreamDlT queue = {0};
		gStreamDlQueue = osMessageQueueNew(100, sizeof(queue), NULL);
		if(gStreamDlQueue == NULL)
			return -1;
		if(gAudStreamRingBuf == NULL)
		{
			gAudStreamRingBuf = pxEcRingCreate(63 * 1024);
			if(gAudStreamRingBuf == NULL)
			{
				if(gStreamDlQueue != NULL)
				{
					osMessageQueueDelete(gStreamDlQueue);
					gStreamDlQueue = NULL;
				}
				return -1;
			}
		}
		memset(&threadAttr, 0, sizeof(threadAttr));
		threadAttr.name 	  = "threadStreamMed";
		threadAttr.stack_size = THREAD_STACK_SIZE_AUDIO;
		threadAttr.priority   = osPriorityBelowNormal7;
#ifdef THREAD_STATIC
        threadAttr.stack_mem  = gStreamMedThreadStackMem;
        threadAttr.cb_mem     = &gStreamMedThreadCbMem;
        threadAttr.cb_size    = sizeof(StaticTask_t);
#endif
		if (osThreadNew(threadStreamMedDlAud, argument, &threadAttr) == NULL)
		{
			SYSLOG_EMERG("Failed to create thread for audio.\r\n");
			if(gAudStreamRingBuf != NULL)
			{
				xEcRingDelete(gAudStreamRingBuf);
				gAudStreamRingBuf = NULL;
			}
			if(gStreamDlQueue != NULL)
			{
				osMessageQueueDelete(gStreamDlQueue);
				gStreamDlQueue = NULL;
			}
			return -1;
		}
	
		return 0;
	}
#endif


static int32_t audioPlayProcess(QueueAudioT *queue)
{
	int32_t audPlayStatus = AV_RET_PLAY_ERROR;
#ifdef SPEAKER_APP
	char *path = NULL;
#endif

    switch (queue->audType)
    {
#ifdef SPEAKER_APP
        case AUDIO_PLAY_VOLUME:
            if (queue->buffer != NULL)
            {
                adjustVolume(*((uint8_t *)(queue->buffer)), &path);
                audioVolumeWrite(gVolumeIndex);
                if (path != NULL)
                {
#if defined(FEATURE_SUBSYS_MP3_ENABLE)
                    mp3Play(AUDIO_PLAY_MP3, path, queue->audioParam);
#endif
                }
                queue->buffer = NULL;
            }
            break;
#endif

        case AUDIO_PLAY_MP3:
			if (queue->buffer != NULL)
	        {
#if defined(FEATURE_SUBSYS_MED_MP3_ENABLE)
	            audPlayStatus = mp3Play(queue->buffer, &queue->audioParam);
#endif
	            free(queue->buffer);
	            queue->buffer = NULL;
	        }
			break;
			
		case AUDIO_PLAY_STRING_MP3:
			{
#if defined(FEATURE_SUBSYS_MED_MP3_ENABLE)
				audPlayStatus = mp3StringPlay(queue->audString,queue->length, &queue->audioParam);
#endif
			}
			break;
#if defined(FEATURE_HTTPC_ENABLE) && defined(FEATURE_HTTP_TLS_ENABLE) && defined(FEATURE_SUBSYS_MEDIA_STREAM_ENABLE)
		case AUDIO_PLAY_STREAM_MP3:
			{
				if(queue->buffer != NULL)
				{
					if(threadStreamMedTaskInit(queue->buffer) != 0)	
						return audPlayStatus;
				}
				else
				{
					//for cust to fill the stream ringbuffer;
				}
				audPlayStatus = mp3StreamPlay(&queue->audioParam);
			}
			break;
#endif
#ifdef FEATURE_SUBSYS_MED_PCM_ENABLE
		case AUDIO_PLAY_PCM:
			{
				audPlayStatus = audPcmFilePlay(queue->buffer,&queue->audioParam);
			}
			break;
		case AUDIO_PLAY_STRING_PCM:
			{
#ifdef MBTK_OPENCPU_SUPPORT
				audPlayStatus = ol_audPcmStringPlay(queue->audString, queue->length, &queue->audioParam, queue->audioParam.increase);
#else
				audPlayStatus = audPcmStringPlay(queue->audString, queue->length, &queue->audioParam);
#endif
			}
            break;
#endif
#ifdef FEATURE_SUBSYS_MED_WAV_ENABLE
		case AUDIO_PLAY_WAV:
			{
				audPlayStatus = audWavFilePlay(queue->buffer,&queue->audioParam);
			}
			break;
		case AUDIO_PLAY_STRING_WAV:
			{
				audPlayStatus = audWavStringPlay(queue->audString, queue->length, &queue->audioParam);
			}
			break;
#endif

#ifdef FEATURE_SUBSYS_AMR_RECORD_ENABLE
		case AUDIO_PLAY_AMR:
			{
				audPlayStatus = amrPlay(queue->audioParam.toneFlag, queue->buffer);
			}
			break;
		case AUDIO_PLAY_STRING_AMR:
			{
				audPlayStatus = amrPlayString(queue->audioParam.toneFlag, queue->audString,queue->length);
			}
			break;
#if defined(FEATURE_HTTPC_ENABLE) && defined(FEATURE_HTTP_TLS_ENABLE) && defined(FEATURE_SUBSYS_MEDIA_STREAM_ENABLE)
		case AUDIO_PLAY_STREAM_AMR:
			{
				if(queue->buffer != NULL)
				{
					if(threadStreamMedTaskInit(queue->buffer) != 0)	
						return audPlayStatus;
				}
				else
				{
					//for cust to fill the stream ringbuffer;
				}
				audPlayStatus = amrPlayStream(queue->audioParam.toneFlag);
			}
			break;
#endif /* FEATURE_SUBSYS_MEDIA_STREAM_ENABLE */
#endif /* FEATURE_SUBSYS_AMR_RECORD_ENABLE */
#ifdef FEATURE_SUBSYS_TTS_ENABLE
		case AUDIO_PLAY_TTS:
			{
				audPlayStatus = ttsPlay(queue->audioParam.toneFlag,queue->buffer);
			}
			break;	
#endif
        default:
            break;
    }
	return audPlayStatus;
}

uint32_t sampleRateConvert(uint32_t rate, bool toEnum)
{
	if (toEnum == true)
	{
		switch (rate)
		{
			case 8000:	return SAMPLERATE_8K;
			case 16000: return SAMPLERATE_16K;			  
			case 22050: return SAMPLERATE_22_05K;
			case 24000: return SAMPLERATE_24K;
			case 32000: return SAMPLERATE_32K;
			case 44100: return SAMPLERATE_44_1K;
			case 48000: return SAMPLERATE_48K;
			case 96000: return SAMPLERATE_96K;
			default:
				return 0;
		}
	}
	else
	{
		switch (rate)
		{
			case SAMPLERATE_8K: 	return 8000;
			case SAMPLERATE_16K:	return 16000;			
			case SAMPLERATE_22_05K: return 22050;
			case SAMPLERATE_24K:	return 24000;
			case SAMPLERATE_32K:	return 32000;
			case SAMPLERATE_44_1K:	return 44100;
			case SAMPLERATE_48K:	return 48000;
			case SAMPLERATE_96K:	return 96000;
			default:
				return 0;
		}
	}
}


medSrcT *gMedStringSrc = NULL;

medSrcT *medFileToString(char *path)
{
	EC_ASSERT(path != NULL,path,0,0);
	struct stat buf = {0};
	FILE  *file = NULL;
	uint32_t fileSize = 0;
	uint32_t readSize = 0;
	
	file = file_fopen(path, "r");
	if (!file)
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, medFileToString_invalid_1, P_ERROR, "file open fail ");
		return NULL;
	}
	file_fstat((int)file, &buf);
	fileSize = buf.st_size;
	
	if(fileSize > 64 *1024)
	{
		file_fclose(file);
		return NULL;
	}
	if(gMedStringSrc)
	{
		OsaFreeMemory(&gMedStringSrc);
	}
		
	gMedStringSrc = (medSrcT *)OsaAllocZeroMemory( sizeof(medSrcT)+ fileSize * sizeof(uint8_t));
	gMedStringSrc->srcLen = 0;

	do{	
		readSize = file_fread(gMedStringSrc->src + gMedStringSrc->srcLen,1,1152,file);
		//ECPLAT_DUMP(UNILOG_PLA_APP, audioDataInput_dump, P_ERROR, "", readSize, gMedStringSrc->src + gMedStringSrc->srcLen);
		gMedStringSrc->srcLen = gMedStringSrc->srcLen + readSize;
	}while(file_ftell(file) < fileSize);
	file_fclose(file);
	return gMedStringSrc;
}

INT32 EC_audioStartPlayFile(char *file_name,uint8_t audType, audioParamT audParam, 
			AudioCallbackT play_cb_func)
{
	errCodeT ret = EC_RET_OK;
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, EC_AudioStartPlayFile_0, P_INFO, 
		"[audio] file_name:%s,format:%d,play_cb_func:%p",file_name, audType, play_cb_func);
	
	if( file_name == NULL || audParam.aud_volume > 100 )		//modified by Chen Hongshun
    {
    	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, EC_AudioStartPlayFile_1, P_INFO, "[audio] error -> file_name");
        return EC_RET_ERR_PARAM;
    }
	QueueAudioT     queue         = {0};
	if(gAudioQueue != NULL && file_name != NULL)
	{
		queue.callback = play_cb_func;
		queue.audType = audType;
		queue.length = strlen(file_name);
		queue.buffer   = malloc(queue.length + 1);
		queue.audioParam.rate = audParam.rate;
		queue.audioParam.dual = audParam.dual;
		queue.audioParam.BitWidth = audParam.BitWidth;
		queue.audioParam.increase = audParam.increase;
		queue.audioParam.aud_volume = audParam.aud_volume;
		queue.audioParam.toneFlag = audParam.toneFlag;
		if(queue.buffer != NULL)
        {
            memset(queue.buffer, 0,    queue.length + 1);
            memcpy(queue.buffer, file_name, queue.length);
            ret = osMessageQueuePut(gAudioQueue, &queue, 0, osWaitForever);
        }
	}
	return ret;
}

INT32 EC_audioStartPlayStream(UINT8 *url,UINT32 strLen,uint8_t audType,audioParamT audParam, AudioCallbackT play_cb_func)
{
	errCodeT ret = EC_RET_OK;
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, EC_audioStartPlayStream_0, P_INFO, 
		"[audio] format:%d,url:%s,strLen:%d", audType,url,strLen);
	
	QueueAudioT 	queue		  = {0};
	if(gAudioQueue != NULL)
	{
		queue.callback = play_cb_func;
		queue.audType = audType;
		queue.length = strLen;
		if(queue.length > 0 && url)
		{
			queue.buffer  = malloc(queue.length + 1);
			if(!queue.buffer)
				return EC_RET_ERR_RESOURCE;
			memset(queue.buffer, 0,    queue.length + 1);
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

INT32	 EC_audioStartPlayString(UINT8 *String,UINT32 strLen,uint8_t audType,
				audioParamT audParam, AudioCallbackT play_cb_func)
{
	errCodeT ret = EC_RET_OK;
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, EC_audioStartPlayString_0, P_INFO, 
		"[audio] format:%d,play_cb_func:%p,String:%p", audType,play_cb_func,String);
	
	if(audParam.aud_volume > 100 || String == NULL)		//modified by Chen Hongshun
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, EC_audioStartPlayString_1, P_INFO, "[audio] error -> String");
		return EC_RET_ERR_PARAM;
	}

	QueueAudioT 	queue		  = {0};
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

INT32 EC_audioStopPlaying(uint8_t audType,bool pauseFlag)
{
	switch(audType)
	{
		case AUDIO_PLAY_CODEC_TYPE_PCM:
			audPcmPlayStop();
			break;
#if defined(FEATURE_SUBSYS_MED_MP3_ENABLE)
		case AUDIO_PLAY_CODEC_TYPE_MP3:
			mp3PlayStop(pauseFlag);
#endif
			break;
#if defined(FEATURE_SUBSYS_MED_AMR_ENABLE)
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
		default:
			break;
	}
#ifdef FEATURE_SUBSYS_MEDIA_STREAM_ENABLE
	if(gStreamDlQueue != NULL)
	{
		QueueStreamDlT queue = {0};
		queue.endFlag = true;		
		osMessageQueuePut(gStreamDlQueue, &queue, 0, osWaitForever);
	}
#endif

	return 0;
}

bool EC_audioIsBusy(void)
{
    return gAudioBusy;
}

UINT16 EC_audioGetVolume(void)
{
    return gVolume;
}


void audioStopPlayCallback(void *param)
{
	EC_audioStopPlaying(ALARM_SOUND_CODEC,false);
	extern osSemaphoreId_t gI2sSemInt;
	osSemaphoreRelease(gI2sSemInt);
	audioUnRegisterStopPlayCb();
}


void audioToneCallback(void *param)
{
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audioToneCallback, P_INFO, "start play tone ");

	
#if defined(FEATURE_SUBSYS_MED_MP3_ENABLE)
	audioRegisterStopPlayCb(audioStopPlayCallback);
	
	if(gToneDecodeQueue != NULL)
	{
		QueueTonePlayT queue = {0};
		queue.endFlag = true;
		osMessageQueuePut(gToneDecodeQueue, &queue, 0, osWaitForever);
	}
#ifdef FEATURE_SUBSYS_AMR_RECORD_ENABLE
	amrRecordStop(AUDIO_AMR_RECORD_STOP_BY_VOLTE);
#endif
#ifdef FEATURE_SUBSYS_AMR_ENABLE
	amrPlayStop();
#endif
	audioParamT audParam = {0};
	uint8_t audType = (AUDIO_PLAY_FILE << 4 | ALARM_SOUND_CODEC);

	audParam.dual = false;
	audParam.increase = false;
	audParam.aud_volume = 40;
	audParam.toneFlag = true;
	EC_audioStartPlayFile(audioToneSrcGet(), audType, audParam, audioCallback);
#endif	
}

#if defined (PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST==1)
extern UINT32 Load$$LOAD_AP_FIRAM_PSRAM_CUST$$Base;
extern UINT32 Image$$LOAD_AP_FIRAM_PSRAM_CUST$$Base;
extern UINT32 Image$$LOAD_AP_FIRAM_PSRAM_CUST$$Length;
extern UINT32 Load$$LOAD_PSRAM_SHARED_CUST$$Base;
extern UINT32 Image$$LOAD_PSRAM_SHARED_CUST$$Base;
extern UINT32 Image$$LOAD_PSRAM_SHARED_CUST$$Length;
extern UINT32 Image$$LOAD_PSRAM_SHARED_CUST$$ZI$$Base;
extern UINT32 Image$$LOAD_PSRAM_SHARED_CUST$$ZI$$Limit;


void CustPsramCodeAndDataInit(void)
{
    UINT32 *start_addr;
    UINT32 *end_addr  ;
    UINT32 length;
    UINT32 *src;
    UINT32 *dst;

    dst    = &(Image$$LOAD_AP_FIRAM_PSRAM_CUST$$Base);
    src    = &(Load$$LOAD_AP_FIRAM_PSRAM_CUST$$Base);
    length = (unsigned int)&(Image$$LOAD_AP_FIRAM_PSRAM_CUST$$Length);
    length /= sizeof(unsigned int);

    if(dst != src)
    {
        while(length >0)
        {
            dst[length-1]=src[length-1];
            length--;
        }
    }


    dst    = &(Image$$LOAD_PSRAM_SHARED_CUST$$Base);
    src    = &(Load$$LOAD_PSRAM_SHARED_CUST$$Base);
    length = (UINT32)&(Image$$LOAD_PSRAM_SHARED_CUST$$Length);
    length /= sizeof(UINT32);

    if(dst != src)
    {
        while(length >0)
        {
            dst[length-1]=src[length-1];
            length--;
        }
    }

    start_addr = &(Image$$LOAD_PSRAM_SHARED_CUST$$ZI$$Base) ;
    end_addr   = &(Image$$LOAD_PSRAM_SHARED_CUST$$ZI$$Limit);
    length = (UINT32)end_addr - (UINT32)start_addr;
	extern void __fast_memset(UINT32 *dst, UINT32 value, UINT32 length);
    __fast_memset((UINT32 *)start_addr, 0, length);


}
#endif


static void threadAudio(void *argument)
{
    QueueAudioT queue = {0};
    static uint8_t     vote  = 0xFF;
	int32_t status = AV_RET_PLAY_ERROR;
    gAudioQueue = osMessageQueueNew(QUEUE_SIZE_AUDIO, sizeof(queue), NULL);
    if (gAudioQueue == NULL)
    {
        SYSLOG_EMERG("Failed to create queue for gAudioQueue.\r\n");
        goto labelEnd;
    }

#ifdef FEATURE_SUBSYS_VOLUME_ENABLE
    volumeManagerInit();
#endif
#ifdef SPEAKER_APP
    audioPlayMp3(WELCOME_SOUND, audioCallback, false, false);
    if (simGetStatus() == SIM_REMOVED)
    {
        audioPlayMp3(SIM_SOUND_UNREADY, NULL, true, true);
    }
    else if (nwIsReady() == true)
    {
        audioPlayMp3(NW_SOUND_READY, NULL, true, true);
    }
#endif

#if defined (PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST==1)
	//CustPsramCodeAndDataInit();
#endif
	audioRegisterToneCb(audioToneCallback);
    slpManRegisterUsrdefinedRestoreCb(restoreCallback, NULL);
    slpManApplyPlatVoteHandle("audio", &vote);
	// tuya
	if (NULL == gAudioPlayI2sSem) {
		gAudioPlayI2sSem = osSemaphoreNew(1U, 0, PNULL);
	}
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
    while (1)
    {
        memset(&queue, 0, sizeof(queue));
        if (osMessageQueueGet(gAudioQueue, &queue, 0, osWaitForever) == osOK)
        {
        	slpManPlatVoteDisableSleep(vote, SLP_SLP1_STATE);
        	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, threadAudio_0, P_INFO, "start process %e<AudioTypeT>", queue.audType);
            
            if (gWakeup == true)
            {
                gWakeup = false;
#if defined (PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST==1)
				//CustPsramCodeAndDataInit();
#endif
				CLOCK_clockEnable(PSRAM_HCLK);
				CLOCK_clockEnable(PCACHE_HCLK);
				CLOCK_clockEnable(CLK_PSRAM);
                			
                //mediaCodecInit();
            }
			gCurMp3PlayTime = 0;
            gAudioBusy = true;
            status = audioPlayProcess(&queue);
            gAudioBusy = false;
            if (queue.callback != NULL)
            {
            	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, threadAudio_111, P_INFO, "status is %d ",status);
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
			slpManPlatVoteEnableSleep(vote, SLP_SLP1_STATE);
        }
    }

labelEnd:
	// tuya
	if (gAudioPlayI2sSem) {
		osSemaphoreDelete(gAudioPlayI2sSem);
		gAudioPlayI2sSem = NULL;
	}
	gAudioPlayThread = NULL;
    osThreadExit();
}

int32_t audioInit(void)
{
	// tuya
    char *name;
	osThreadAttr_t threadAttr = {0};

	if (   (NULL != gAudioPlayThread)
        && ((name = osThreadGetName(gAudioPlayThread)) != NULL)
        && (!memcmp("threadRecord", name, 12)) ) {
            SYSLOG_EMERG("Thread threadAudio already existed.\r\n");
            return osError;
    }

    memset(&threadAttr, 0, sizeof(threadAttr));
    threadAttr.name       = "threadAudio";
    threadAttr.stack_size = THREAD_STACK_SIZE_AUDIO;
    threadAttr.priority   = osPriorityBelowNormal7;
#ifdef THREAD_STATIC
	threadAttr.stack_mem  = gAudioThreadStackMem;
	threadAttr.cb_mem     = &gAudioThreadCbMem;
	threadAttr.cb_size    = sizeof(StaticTask_t);
#endif
    if ((gAudioPlayThread = osThreadNew(threadAudio, NULL, &threadAttr)) == NULL)
    {
        SYSLOG_EMERG("Failed to create thread for audio.\r\n");
		return osError;
    }

    return 0;
}

