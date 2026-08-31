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
#include "medDataHandle.h"
#include "servicemanager.h"

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
#ifdef MBTK_OPENCPU_SUPPORT
#include "ivTTS_resource_irf.h"
#define TTS_CB_PARAM                        ((ivPointer)mlp_resource)
#else
#define TTS_CB_PARAM                        ((ivPointer)FLASH_TTS_REGION_START)
#endif
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
	bool toneStopFlag;
} QueueTtsPlayT;

typedef struct
{
	uint32_t textLen;
	char *text;
}ttsSyncParaT;

#define AUDIO_TTS_PCM_TRUNCK_SIZE_MAX   1280
#define TTS_PRE_DECODE_FRAME_NUM		1

#define TTS_DATA_CACHE_PRE_FILL_SIZE		(16*1024)	
#define TTS_DATA_CACHE_SIZE 				(TTS_DATA_CACHE_PRE_FILL_SIZE + 3*AUDIO_TTS_PCM_TRUNCK_SIZE_MAX)
#define THREAD_STACK_SIZE_TTS     			(5*1024)
#define TTS_PLAY_STATE_IDLE					(0)
#define TTS_PLAY_STATE_START				(1)
#define TTS_PLAY_STATE_STOP					(2)
#define TTS_START_CHECK_CNT_MAX				(500) //20ms*500

uint8_t 	ttsStaFlag = TTS_PLAY_STATE_IDLE;

osMessageQueueId_t gTtsDecodeQueue = NULL;
static osTimerId_t ttsTimer = NULL;

void ttsTimerCb(void *argument)
{
	BOOL toneFlag = *((BOOL *)argument);
	QueueTtsPlayT queue = {0};
	osStatus_t stat = osOK;
	if(gTtsDecodeQueue != NULL)
	{
		if(toneFlag)
		{
			if(ttsStaFlag == TTS_PLAY_STATE_IDLE)
			{
				queue.endFlag = true;
			}
		}
		else
		{
			if(medDataHandleStateGet() >= MED_DATA_HDL_STA_SUSPEND)
			{
				queue.endFlag = true;			
			}
		}
		
		stat = osMessageQueuePut(gTtsDecodeQueue, &queue, 0, AUD_CODEC_SIG_TIMEROUT_MAX);
		if(stat != osOK)
		{
			ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, ttsTimerCb_valid, P_ERROR, "message send fail(%d)",stat);
		}
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
	// int32_t queueSize = 0;
	int32_t freeCacheSize = 0;
	
	if(ttsStaFlag == TTS_PLAY_STATE_STOP)
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, outputCB, P_WARNING, "ttsStaFlag [%d]",ttsStaFlag);
		return ivTTS_ERR_OK;
	}
	ecRingT *pcmCache = medGetDataCache();
	if(nSize && pcmCache)
	{
		do
		{
			if(ttsStaFlag == TTS_PLAY_STATE_STOP)
			{
				ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, outputCB_1, P_WARNING, "ttsStaFlag [%d]",ttsStaFlag);
				break;
			}
			freeCacheSize = medGetDataCacheAvlbSize();
			if(freeCacheSize > nSize)
			{
				xEcRingWriteEx(pcmCache,(UINT8 *)pcData,nSize);				
				break;
			}
			osDelay(40);
		}while(1);
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

static void threadATTSSync(void *argument)
{
	ttsSyncParaT *ttsSyncParam = (ttsSyncParaT *)argument;
	ivTTSErrID ret = 0;
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, threadATTSSync, P_DEBUG, "text [%s],textLen [%d]",ttsSyncParam->text, ttsSyncParam->textLen);
#if (TTS_RESOURCE_LOCATION == TTS_RESOURCE_LOCATION_FLASH_X)
    gTtsFile = file_fopen(TTS_PATH, "r");
    if (gTtsFile == NULL)
    {
        SYSLOG_DEBUG("Failed to open the file \"%s\"\r\n", TTS_PATH);
        goto labelEnd;
    }
#endif
    if ((gTts != NULL) && (ttsSyncParam->text != NULL))
    {
#ifdef DEBUG_PRINT_TTS_TIME
        gTimeBegin = osKernelGetTickCount();
#endif
		ttsStaFlag = TTS_PLAY_STATE_START;
        ret = ivTTS_SynthText(gTts, ivText(ttsSyncParam->text), ttsSyncParam->textLen);
		ttsStaFlag = TTS_PLAY_STATE_IDLE;
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, threadATTSSync_1, P_DEBUG, "ret [%d]",ret);
#ifdef DEBUG_PRINT_TTS_TIME
		gTimeEnd = osKernelGetTickCount();
#endif
#ifdef DEBUG_PRINT_TTS_TIME
        SYSLOG_DEBUG("BEGIN: %d, END: %d, DIFF: %d\r\n", gTimeBegin, gTimeEnd, gTimeEnd - gTimeBegin);
#endif
    }
#if (TTS_RESOURCE_LOCATION == TTS_RESOURCE_LOCATION_FLASH_X)
labelEnd:
#endif
#if (TTS_RESOURCE_LOCATION == TTS_RESOURCE_LOCATION_FLASH_X)
    if (gTtsFile != NULL)
    {
        file_fclose(gTtsFile);
        gTtsFile = NULL;
    }
#endif
#if 0
	osThreadExit();
#else
	Service_stop("service:/threadTTS");
#endif

}


int32_t ttsPlay(BOOL toneFlag, char *text, uint32_t textLen)
{
	int32_t ret = AV_RET_PLAY_ERROR;
	static int8_t checkCnt = 0;
	ttsSyncParaT ttsSyncParam = {0, NULL};
	while((ttsStaFlag != TTS_PLAY_STATE_IDLE) && (checkCnt < TTS_START_CHECK_CNT_MAX))
	{
		osDelay(20);
	}
	if(gTts == NULL)
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, ttsPlay_invalid_0, P_ERROR, "tts may be not inited");
		return ret;
	}
	if(ttsStaFlag == TTS_PLAY_STATE_START || checkCnt >= TTS_START_CHECK_CNT_MAX)
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, ttsPlay_invalid_1, P_ERROR, "ttsStaFlag is already Running");
		return ret;
	}
	QueueTtsPlayT queue = {0};
	rawDataHanderParam_T dataHandleParam = {0};
	gTtsDecodeQueue = osMessageQueueNew(100, sizeof(queue), NULL);
    if (gTtsDecodeQueue == NULL)
    {
        goto labelEnd;
    }
	ecRingT *pcmCache = NULL;
	pcmCache = pcmCache;
	if(medDataHandleStateGet() != MED_DATA_HDL_STA_START)
	{
		pcmCache = medInitDataCache(TTS_DATA_CACHE_SIZE, TTS_DATA_CACHE_PRE_FILL_SIZE);	
		//medDataHandleInit();
		dataHandleParam.field.samplerate = SAMPLERATE_16K;
		dataHandleParam.field.envType = (toneFlag ? MED_DATA_ENV_TYPE_TONE : MED_DATA_ENV_TYPE_LOCAL);
		dataHandleParam.field.bitWidth = 0;//default 16bit	
		medDataHandleStart(&dataHandleParam);
	}
	else
	{
		pcmCache = medGetDataCache();
	}
	ret = AV_RET_PLAY_START;

	osThreadId_t ttsThread = NULL;
	osThreadAttr_t threadAttr = {0};
    memset(&threadAttr, 0, sizeof(threadAttr));
    threadAttr.name       = "threadTTS";
    threadAttr.stack_size = THREAD_STACK_SIZE_TTS;
    threadAttr.priority   = osPriorityBelowNormal7;
	ttsSyncParam.textLen = textLen;
	ttsSyncParam.text = text;
#if 0
	ttsThread = osThreadNew(threadATTSSync, (void *)&ttsSyncParam, &threadAttr);
#else
    char serviceName[32] = {0};
    snprintf(serviceName, sizeof(serviceName), "service:/%s", threadAttr.name);
    Service_reg(serviceName, threadATTSSync, (void *)&ttsSyncParam, threadAttr.cb_mem, threadAttr.cb_size, threadAttr.stack_mem, threadAttr.stack_size, threadAttr.priority);
    ttsThread = (osThreadId_t)Service_start(serviceName);
#endif
    if (ttsThread == NULL)
    {
    	medDataHandleStop();
        goto labelEnd;
    }
	if(ttsTimer == NULL)
	{
		ttsTimer = osTimerNew((osTimerFunc_t)ttsTimerCb,osTimerPeriodic,(void *)&toneFlag, NULL);
		osTimerStart(ttsTimer, 20);
	}
	while(1)
	{
		memset(&queue, 0, sizeof(queue));		
        if(osMessageQueueGet(gTtsDecodeQueue, &queue, 0, osWaitForever) == osOK)
    	{
    		if(queue.endFlag)
			{
				if(ttsStaFlag == TTS_PLAY_STATE_START)
					ttsStaFlag = TTS_PLAY_STATE_STOP;
				if(medDataHandleStateGet() >= MED_DATA_HDL_STA_SUSPEND)
					ret = AV_RET_PLAY_EOF;
				else
				{
					ret = AV_RET_PLAY_STOP;
				}
				if(!toneFlag || queue.toneStopFlag)
					medDataHandleStop();
				break;
			}			
    	}
	}

labelEnd:

	if(ttsTimer != NULL)
	{
		osTimerStop(ttsTimer);
		osTimerDelete(ttsTimer);
		ttsTimer = NULL;
	}
	if(gTtsDecodeQueue != NULL)
	{
		osMessageQueueDelete(gTtsDecodeQueue);
		gTtsDecodeQueue = NULL;
	}

	return ret;
}

void ttsStopPlay(BOOL toneStopFlag)
{
	QueueTtsPlayT queue = {0};
	if(gTtsDecodeQueue != NULL)
	{
		queue.endFlag = true;
		queue.toneStopFlag = toneStopFlag;
		osMessageQueuePutToFront(gTtsDecodeQueue, &queue, 0, osWaitForever);
	}
	return ;
}

#endif
