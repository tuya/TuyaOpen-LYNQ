#ifdef FEATURE_SUBSYS_TTS_ENABLE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "sctdef.h"
#include "cmsis_os2.h"
#include "mem_map.h"
#include DEBUG_LOG_HEADER_FILE
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
#include "flashex.h"
#endif
#include "ivTTS.h"
#include "tts.h"
#include "media.h"
#include "ccio_audio.h"
#include "ec_ring.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif


#define TTS_HEAP_SIZE                       (80 * 1024)
#define TTS_SAMPLE_RATE                     16000
#define BUFFER_SIZE                         3
#ifdef FEATURE_SUBSYS_CHANNEL_1TO2_ENABLE
#define FACTOR                              2
#define CHANNEL                             DUAL_CHANNEL
#else
#define FACTOR                              1
#define CHANNEL                             MONO
#endif
#define TTS_TX_SIZE                         (AUDIO_TX_TRANSFER_SIZE / FACTOR)
#define TTS_RESOURCE_LOCATION_INTERNAL      1
#define TTS_RESOURCE_LOCATION_EXTERNAL      2
#ifdef FLASH_X_ENABLE
#define TTS_RESOURCE_LOCATION_FLASH_X       3
#define TTS_RESOURCE_LOCATION               TTS_RESOURCE_LOCATION_FLASH_X
#else
#define TTS_RESOURCE_LOCATION               TTS_RESOURCE_LOCATION_EXTERNAL
#endif
#if (TTS_RESOURCE_LOCATION == TTS_RESOURCE_LOCATION_INTERNAL)
#define TTS_CB_PARAM                        ((ivPointer)FLASH_TTS_REGION_START)
#elif (TTS_RESOURCE_LOCATION == TTS_RESOURCE_LOCATION_EXTERNAL)
#define TTS_CB_PARAM                        0
#elif (TTS_RESOURCE_LOCATION == TTS_RESOURCE_LOCATION_FLASH_X)
#define TTS_CB_PARAM                        (&gTtsFile)
#include "merged.h"
#if (UI_WATCH_USED == 1)
#define TTS_PATH                            EXT_BIN_00
#else
#define TTS_PATH                            LV_IMAGE_00
#endif
#else
#error "TTS_RESOURCE_LOCATION is out of range."
#endif
#define TTS_FIRST_BUFFER_CONSUME            82


static ivHTTS gTts = NULL;
#if (PSRAM_EXIST == 1)
static PLAT_FPSRAM_ZI_CUST uint8_t gTtsHeap[TTS_HEAP_SIZE] = {0};
#else
static uint8_t gTtsHeap[TTS_HEAP_SIZE] = {0};
#endif
#ifdef DEBUG_PRINT_TTS_TIME
static uint32_t gTimeBegin = 0;
static uint32_t gTimeEnd   = 0;
#endif
#if (TTS_RESOURCE_LOCATION == TTS_RESOURCE_LOCATION_FLASH_X)
static FILE *gTtsFile = NULL;
#endif

typedef struct
{   
	bool endFlag;
} QueueTtsPlayT;

typedef struct _ttsPlayCtrlInfo{
	uint8_t rate;
	uint8_t toneBufIndex;	
	uint8_t toneFlag;
	uint32_t outSrcIndex;
	uint64_t ringBufFillCnt;
	uint64_t dmaBufFillCnt;
}ttsPlayCtrlInfo;

#define AUDIO_TTS_PCM_TRUNCK_SIZE_MAX   1280
#define TTS_PRE_DECODE_FRAME_NUM		1

osMessageQueueId_t gTtsDecodeQueue = NULL;
ecRingT *gTtsRingBuf = NULL;
static osTimerId_t ttsTimer = NULL;

ttsPlayCtrlInfo ttsPlayCntInfo = {0,0,0};
#if (PSRAM_EXIST == 1)
__attribute__((aligned(8))) PLAT_FPSRAM_ZI_CUST uint8_t ttsBuf[AUDIO_TTS_PCM_TRUNCK_SIZE_MAX*3] = {0};/*first byte is SamplteRate*/
#else
__attribute__((aligned(8))) uint8_t ttsBuf[AUDIO_TTS_PCM_TRUNCK_SIZE_MAX*3] = {0};/*first byte is SamplteRate*/
#endif

void ttsTimerCb(void *argument)
{
	QueueTtsPlayT queue = {0};
	UINT8 toneFlag = *((UINT8 *)argument);
	INT32 readSize = 0;
	INT32 dataSize = xEcRingGetOption(gTtsRingBuf,E_LRO_DATA_SIZE);
	INT32 AudioPcmTrunckSize = 0;
	AudioPcmTrunckSize = TTS_SAMPLE_RATE  * 20 * 16 / 8 / 1000;
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, ttsTimerCb, P_DEBUG, "ttsBufIndex is %d ,dataSize is %d type is %d ",ttsPlayCntInfo.toneBufIndex,dataSize,toneFlag);
	if((ttsPlayCntInfo.dmaBufFillCnt >= ttsPlayCntInfo.ringBufFillCnt) && (ttsPlayCntInfo.ringBufFillCnt != 0))
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, ttsTimerCb_1, P_DEBUG, "stop play tts ");
		//osSemaphoreRelease(gTtsSemInt);
		queue.endFlag = true;
		if(gTtsDecodeQueue != NULL)
			osMessageQueuePut(gTtsDecodeQueue, &queue, 0, osWaitForever);
		return;
	}
	if(dataSize > 0)
	{

		readSize = xEcRingRead(gTtsRingBuf, ttsBuf + ttsPlayCntInfo.toneBufIndex*AudioPcmTrunckSize, AudioPcmTrunckSize);
		if((ttsPlayCntInfo.toneFlag == 0) && (ttsPlayCntInfo.toneBufIndex == 1))
		{
			if(toneFlag)
			{
				audioStartPlaySound(PLAY_MULTI_MEDIA, ttsBuf, 3*AudioPcmTrunckSize, TTS_SAMPLE_RATE, 16);
			}
			else 
			{
				audioAppStartPlaySound(PLAY_MULTI_MEDIA, ttsBuf, 3*AudioPcmTrunckSize, TTS_SAMPLE_RATE, 16);
			}
			ttsPlayCntInfo.toneFlag = 1;
		}
		
		ttsPlayCntInfo.toneBufIndex++;
		ttsPlayCntInfo.toneBufIndex = ttsPlayCntInfo.toneBufIndex % 3;
			
		ttsPlayCntInfo.dmaBufFillCnt = ttsPlayCntInfo.dmaBufFillCnt + readSize;
	}
	else
	{
		if(ttsPlayCntInfo.ringBufFillCnt != 0)
		{
			ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, ttsTimerCb_2, P_DEBUG, "stop play tts ");
			queue.endFlag = true;
			if(gTtsDecodeQueue != NULL)
				osMessageQueuePut(gTtsDecodeQueue, &queue, 0, osWaitForever);
			return;
		}
		if((ttsPlayCntInfo.toneFlag == 0) && (ttsPlayCntInfo.toneBufIndex == 1))
		{
			if(toneFlag)
			{
				audioStartPlaySound(PLAY_MULTI_MEDIA, ttsBuf, 3*AudioPcmTrunckSize, TTS_SAMPLE_RATE, 16);
			}
			else 
			{
				audioAppStartPlaySound(PLAY_MULTI_MEDIA, ttsBuf, 3*AudioPcmTrunckSize, TTS_SAMPLE_RATE, 16);
			}
			ttsPlayCntInfo.toneFlag = 1;
		}
		ttsPlayCntInfo.toneBufIndex++;
		ttsPlayCntInfo.toneBufIndex = ttsPlayCntInfo.toneBufIndex % 3;
			
		ttsPlayCntInfo.dmaBufFillCnt = ttsPlayCntInfo.dmaBufFillCnt + readSize;
	}
}


/* read resource callback */
ivBool ivCall readResCB(
		ivPointer		pParameter,		/* [in] user callback parameter */
		ivPointer		pBuffer,		/* [out] read resource buffer */
		ivResAddress	iPos,			/* [in] read start position */
		ivResSize		nSize )			/* [in] read size */
{
#if (TTS_RESOURCE_LOCATION == TTS_RESOURCE_LOCATION_INTERNAL)
    memcpy((uint8_t *)pBuffer, (uint8_t *)pParameter + iPos, nSize);
#elif (TTS_RESOURCE_LOCATION == TTS_RESOURCE_LOCATION_EXTERNAL)
    spiFlashRead((uint32_t)((uint8_t *)pParameter + iPos), (uint8_t *)pBuffer, nSize);
#elif (TTS_RESOURCE_LOCATION == TTS_RESOURCE_LOCATION_FLASH_X)
    file_fseek(*((FILE **)pParameter), iPos, SEEK_SET);
    file_fread((void *)pBuffer, nSize, 1, *((FILE **)pParameter));
#endif
    return ivTrue;
}

/* output callback */
ivTTSErrID ivCall outputCB(
		ivPointer		pParameter,		/* [in] user callback parameter */
		ivUInt16		nCode,			/* [in] output data code */
		ivCPointer		pcData,			/* [in] output data buffer */
		ivSize			nSize )			/* [in] output data size */
{
    /* It is recommended to use the macro AUDIO_TX_TRANSFER_SIZE as the size of this buffer. */
#if (PSRAM_EXIST == 1)
    static __attribute__((aligned(16))) PLAT_FPSRAM_ZI_CUST uint8_t sBuffer[BUFFER_SIZE][AUDIO_TX_TRANSFER_SIZE];
#else
    static __attribute__((aligned(16))) uint8_t sBuffer[BUFFER_SIZE][AUDIO_TX_TRANSFER_SIZE];
#endif
    static volatile uint32_t sLength   = 0;
    static volatile uint32_t sIndex    = 0;
    uint32_t                 length    = 0;
    uint32_t                 remain    = 0;
    uint32_t                 ttsTxSize = TTS_TX_SIZE;
#ifdef FEATURE_SUBSYS_CHANNEL_1TO2_ENABLE
    uint8_t                  buffer[TTS_TX_SIZE];
#endif
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, outputCB, P_DEBUG, "nSize is %d ",nSize);
    if (nSize > 0)
    {
    	if((gTtsRingBuf != NULL) && xEcRingGetOption(gTtsRingBuf,E_LRO_FREE_SIZE))
		{
			if(xEcRingGetOption(gTtsRingBuf,E_LRO_FREE_SIZE) < nSize)
			{
				osDelay(60);
			}

			xEcRingWrite(gTtsRingBuf,pcData, nSize);
			
		}
		ttsPlayCntInfo.ringBufFillCnt += nSize;    
    }   
    return ivTTS_ERR_OK;
}

/* parameter change callback */
ivTTSErrID ivCall paramChangeCB(
		ivPointer       pParameter,		/* [in] user callback parameter */
		ivUInt32		nParamID,		/* [in] parameter id */
		ivUInt32		nParamValue )	/* [in] parameter value */
{
	return ivTTS_ERR_OK;
}

/* progress callback */
ivTTSErrID ivCall progressCB(
		ivPointer       pParameter,		/* [in] user callback parameter */
		ivUInt32		iProcPos,		/* [in] current processing position */
		ivUInt32		nProcLen )		/* [in] current processing length */
{
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, progressCB, P_DEBUG, "iProcPos %d nProcLen %d ",iProcPos,nProcLen);
	if((iProcPos == 0) && (nProcLen == 0))
	{
		if(gTtsDecodeQueue != NULL)
		{
			QueueTtsPlayT queue = {0};
			queue.endFlag = true;
			osMessageQueuePut(gTtsDecodeQueue, &queue, 0, osWaitForever);
		}
	}
	return ivTTS_ERR_OK;
}

int32_t ttsInit(void)
{
    int32_t           retVal       = -1;
    ivTTSErrID        ivReturn     = 0;
    ivTResPackDescExt tResPackDesc = {0};

#if (TTS_RESOURCE_LOCATION == TTS_RESOURCE_LOCATION_FLASH_X)
    gTtsFile = file_fopen(TTS_PATH, "r");
    if (gTtsFile == NULL)
    {
        SYSLOG_DEBUG("Failed to open the file \"%s\"\r\n", TTS_PATH);
        goto labelEnd;
    }
#endif

    memset(&tResPackDesc, 0, sizeof(tResPackDesc));
    tResPackDesc.pCBParam = TTS_CB_PARAM;
    tResPackDesc.pfnRead  = readResCB;

    memset(gTtsHeap, 0, TTS_HEAP_SIZE);
    ivReturn = ivTTS_Create(&gTts, (ivPointer)gTtsHeap, TTS_HEAP_SIZE, ivNull, (ivPResPackDescExt)&tResPackDesc, (ivSize)1, NULL);
    if (ivReturn)
    {
        SYSLOG_EMERG("ivTTS_Create return 0x%X\n", ivReturn);
        goto labelEnd;
    }

    ivReturn = ivTTS_SetParam(gTts, ivTTS_PARAM_OUTPUT_CALLBACK, (ivUInt32)outputCB);
    if (ivReturn)
    {
        SYSLOG_EMERG("ivTTS_SetParam ivTTS_PARAM_OUTPUT_CALLBACK return 0x%x\r\n", ivReturn);
        goto labelEnd;
    }

    ivReturn = ivTTS_SetParam(gTts, ivTTS_PARAM_PARAMCH_CALLBACK, (ivUInt32)paramChangeCB);
    if (ivReturn)
    {
        SYSLOG_EMERG("ivTTS_SetParam ivTTS_PARAM_PARAMCH_CALLBACK return 0x%X\r\n", ivReturn);
        goto labelEnd;
    }

    ivReturn = ivTTS_SetParam(gTts, ivTTS_PARAM_PROGRESS_CALLBACK, (ivUInt32)progressCB);
    if (ivReturn)
    {
        SYSLOG_EMERG("ivTTS_SetParam ivTTS_PARAM_PROGRESS_CALLBACK return 0x%X\r\n", ivReturn);
        goto labelEnd;
    }

    ivReturn = ivTTS_SetParam(gTts, ivTTS_PARAM_INPUT_CODEPAGE, ivTTS_CODEPAGE_UTF8);
    if (ivReturn)
    {
        SYSLOG_EMERG("ivTTS_SetParam ivTTS_PARAM_INPUT_CODEPAGE return 0x%X\r\n", ivReturn);
        goto labelEnd;
    }

    ivReturn = ivTTS_SetParam(gTts, ivTTS_PARAM_LANGUAGE, ivTTS_LANGUAGE_AUTO);
    if (ivReturn)
    {
        SYSLOG_EMERG("ivTTS_SetParam ivTTS_PARAM_LANGUAGE return 0x%X\r\n", ivReturn);
        goto labelEnd;
    }

    ivReturn = ivTTS_SetParam(gTts, ivTTS_PARAM_VOLUME, ivTTS_VOLUME_NORMAL);
    if (ivReturn)
    {
        SYSLOG_EMERG("ivTTS_SetParam ivTTS_PARAM_VOLUME return 0x%X\r\n", ivReturn);
        goto labelEnd;
    }

    ivReturn = ivTTS_SetParam(gTts, ivTTS_PARAM_VOICE_SPEED, ivTTS_SPEED_NORMAL);
    if (ivReturn)
    {
        SYSLOG_EMERG("ivTTS_SetParam ivTTS_PARAM_VOICE_SPEED return 0x%X\r\n", ivReturn);
        goto labelEnd;
    }

    ivReturn = ivTTS_SetParam(gTts, ivTTS_PARAM_ROLE, ivTTS_ROLE_XIAOYAN);
    if (ivReturn)
    {
        SYSLOG_EMERG("ivTTS_SetParam ivTTS_PARAM_ROLE return 0x%X\r\n", ivReturn);
        goto labelEnd;
    }

    retVal = 0;

labelEnd:
#if (TTS_RESOURCE_LOCATION == TTS_RESOURCE_LOCATION_FLASH_X)
    if (gTtsFile != NULL)
    {
        file_fclose(gTtsFile);
        gTtsFile = NULL;
    }
#endif

    return retVal;
}

int32_t ttsPlay(BOOL toneFlag, char *text)
{
	int32_t ret = AV_RET_PLAY_ERROR;
#if (TTS_RESOURCE_LOCATION == TTS_RESOURCE_LOCATION_FLASH_X)
    gTtsFile = file_fopen(TTS_PATH, "r");
    if (gTtsFile == NULL)
    {
        SYSLOG_DEBUG("Failed to open the file \"%s\"\r\n", TTS_PATH);
        goto labelEnd;
    }
#endif

	QueueTtsPlayT queue = {0};
	gTtsDecodeQueue = osMessageQueueNew(100, sizeof(queue), NULL);
    if (gTtsDecodeQueue == NULL)
    {
        goto labelEnd;
    }
	if(gTtsRingBuf == NULL)
		gTtsRingBuf = pxEcRingCreate(1024*24);/*24k ring buff*/
	xEcRingClear(gTtsRingBuf);
	memset(&ttsPlayCntInfo,0,sizeof(ttsPlayCtrlInfo));
	memset(ttsBuf,0,AUDIO_TTS_PCM_TRUNCK_SIZE_MAX*3);
	
	if(ttsTimer == NULL)
	{
		ttsTimer = osTimerNew((osTimerFunc_t)ttsTimerCb,osTimerPeriodic,(void *)&toneFlag, NULL);
		osTimerStart(ttsTimer, 20);
#ifdef DEBUG_PRINT_TTS_TIME
        gTimeEnd = osKernelGetTickCount();
#endif
	}
	osDelay(400);
    if ((gTts != NULL) && (text != NULL))
    {
#ifdef DEBUG_PRINT_TTS_TIME
        gTimeBegin = osKernelGetTickCount();
#endif
#ifdef FEATURE_SUBSYS_VOLUME_ENABLE
        volumeManagerActivate("audio");
#endif
        ivTTS_SynthText(gTts, ivText(text), strlen(text));		
#ifdef DEBUG_PRINT_TTS_TIME
        SYSLOG_DEBUG("BEGIN: %d, END: %d, DIFF: %d\r\n", gTimeBegin, gTimeEnd, gTimeEnd - gTimeBegin);
#endif
    }
	
	while(1)
	{
		memset(&queue, 0, sizeof(queue));
		
        if(osMessageQueueGet(gTtsDecodeQueue, &queue, 0, osWaitForever) == osOK)
    	{
    		if(queue.endFlag == true)
			{
				ret = AV_RET_PLAY_STOP;
				audioAppStopPlaySound(PLAY_MULTI_MEDIA);
				break;
			}
				
    	}
	}

labelEnd:
#if (TTS_RESOURCE_LOCATION == TTS_RESOURCE_LOCATION_FLASH_X)
    if (gTtsFile != NULL)
    {
        file_fclose(gTtsFile);
        gTtsFile = NULL;
    }
#endif

	if(ttsTimer != NULL)
	{
		osTimerStop(ttsTimer);
		osTimerDelete(ttsTimer);
		ttsTimer = NULL;
	}
	if(gTtsRingBuf != NULL)
	{
		xEcRingDelete(gTtsRingBuf);
		gTtsRingBuf = NULL;
	}
	if(gTtsDecodeQueue != NULL)

	{
		osMessageQueueDelete(gTtsDecodeQueue);
		gTtsDecodeQueue = NULL;
	}

	memset(ttsBuf,0,AUDIO_TTS_PCM_TRUNCK_SIZE_MAX*3);
	return ret;
}
#endif
