#ifdef FEATURE_SUBSYS_AMR_RECORD_ENABLE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "cmsis_os2.h"
#include DEBUG_LOG_HEADER_FILE
#include "slpman.h"
#include "i2s_device.h"
#include "ccio_audio.h"
#include "hal_voice_eng.h"
#include "hal_voice_eng_mem.h"
#include "record.h"
#include "audAmrCommon.h"
#include "storage.h"
#include "media.h"

#ifdef MBTK_OPENCPU_SUPPORT
#include "ol_log.h"
#endif

#define RECORD_TIMER_LOOP_MS    20

volatile uint8_t gRecordState = AMR_RECORD_IDLE;
static osTimerId_t   gAmrRecordTimer = NULL;
osMessageQueueId_t gAudAmrRecordQueue = NULL;


void audioStopRecordCallback(void *param)
{	
	QueueTonePlayT			queue = {0}; 
	gRecordState = AMR_RECORD_ENDING_BY_VOLTE;
	queue.endFlag = true;
	if(gAudAmrRecordQueue != NULL)
		osMessageQueuePut(gAudAmrRecordQueue,&queue, 0, osWaitForever);
}

static int32_t stopRecord(void)
{    
    ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, stopRecord, P_INFO, "[APP AMR Record] stopRecord: gRecordState=%d", gRecordState);
	if(gRecordState != AMR_RECORD_ENDING_BY_VOLTE)
	{
    	audioAppStopRecord(AMR_CODEC_TYPE);
		gRecordState = AMR_RECORD_STOP;
	}
   // halVoiceEngStopReq();
    //amrEngWaitCpReply(HAL_VOICE_ENG_STOP_CNF);
    amrEngCommonStop(AUDIO_AMR_ENG_ENCODE_FLAG);

    return 0;
}

int32_t amrRecordStop(uint8_t stopType)
{
	QueueTonePlayT			queue = {0}; 
	if(stopType == AUDIO_AMR_RECORD_STOP_NORMAL)
    	gRecordState = AMR_RECORD_ENDING_NOMAL;
	else
		gRecordState = AMR_RECORD_ENDING_BY_VOLTE;
    ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrRecordStop, P_INFO, "[APP AMR Record] amrRecordStop: gRecordState=%d", gRecordState);
	queue.endFlag = true;
	if(gAudAmrRecordQueue != NULL)
		osMessageQueuePut(gAudAmrRecordQueue,&queue, 0, osWaitForever);

    return 0;
}

static void amrRecordTimerFunc(void *argument)
{
	QueueTonePlayT			queue = {0};    
	if(gAudAmrRecordQueue != NULL)
		osMessageQueuePut(gAudAmrRecordQueue,&queue, 0, osWaitForever);
}


int32_t amrRecord(char *path,RecordParamT *recordParam)
{
    int32_t                 retVal  = AV_RET_RECORD_ERROR;
    FILE                   *file    = NULL;
    uint32_t                count   = 0;
    uint32_t                timeEnd = 0;
    int32_t                 length  = 0;
    uint8_t                 buffer[HAL_16K_PCM_FRAME_SIZE] = {0};
	uint32_t				readLen = HAL_RTP_ONE_AMR_FRAME_MAX_SIZE;
	QueueTonePlayT			queue = {0};
    char                   *amrHead    = (AMR_CODEC_TYPE == HAL_VC_AMR_NB) ? AMR_NB_HEAD : AMR_WB_HEAD;
    HalVoiceCodecConfigReq  codecCfg   =
    {
        .codecType       = AMR_CODEC_TYPE,
        .encBitRate      = (AMR_CODEC_TYPE == HAL_VC_AMR_NB) ? HAL_AMR_NB_FT_7 : HAL_AMR_WB_FT_8,
        .encOutBitOffset = AMR_BIT_OFFSET
    };

    ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrRecord_1, P_INFO, "[APP AMR Record] amrRecord: %s, %d, %d, %d", amrHead, codecCfg.codecType, codecCfg.encBitRate, codecCfg.encOutBitOffset);

    if ((recordParam == NULL) || (path == NULL))
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrRecord_invalid_1, P_ERROR, "[APP AMR Record] Param error");
        return retVal;
    }
	if (gAmrRecordTimer == NULL)
    {
        gAmrRecordTimer = osTimerNew(amrRecordTimerFunc, osTimerPeriodic, NULL, NULL);
        if (gAmrRecordTimer == NULL)
        {
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrRecord_invalid_2, P_ERROR, "[[APP AMR Record] Failed to create timer for gAmrTimer");
            return retVal;
        }
    }
	gAudAmrRecordQueue = osMessageQueueNew(100, sizeof(queue), NULL);
	if(gAudAmrRecordQueue == NULL)
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrRecord_invalid_3, P_ERROR, "[[APP AMR Record] Failed to create gAudAmrQueue");
		if(osTimerIsRunning(gAmrRecordTimer))
		{
			osTimerStop(gAmrRecordTimer);
		}
		osTimerDelete(gAmrRecordTimer);
	  	gAmrRecordTimer = NULL;
        return retVal;
	}
	if(!recordParam->bContinue)
	{
    	file_remove(path);
    	file = file_fopen(path, "w");
	}
	else
	{
		file = file_fopen(path, "ab+");
		file_ftell(file);
	}
    if (file == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrRecord_invalid_4, P_ERROR, "[APP AMR Record] Failed to open file %s: %d", path, retVal);
		if(gAudAmrRecordQueue != NULL)
		{
			osMessageQueueDelete(gAudAmrRecordQueue);
			gAudAmrRecordQueue = NULL;
		}
    
	    if(osTimerIsRunning(gAmrRecordTimer))
		{
			osTimerStop(gAmrRecordTimer);
		}
		osTimerDelete(gAmrRecordTimer);
	  	gAmrRecordTimer = NULL;
        return retVal;
    }
	 //halSetVoiceEngRetCallback(amrEngCallback);
    //halVoiceEngStartReq();
   // amrEngWaitCpReply(HAL_VOICE_ENG_START_CNF);
    //halVoiceCodecConfigReq(&codecCfg);
    //amrEngWaitCpReply(HAL_VOICE_CODEC_CONFIG_CNF);
	if(amrEngCommonStart(&codecCfg,AUDIO_AMR_ENG_ENCODE_FLAG))
	{
		goto EXIT;
	}
	if(recordParam->codec == AUDIO_RECORD_CODEC_PCM || recordParam->codec == AUDIO_RECORD_CODEC_PCM_3A)
	{
		amrEngSetRecPcmFlag(TRUE);
		readLen =  (codecCfg.codecType == HAL_VC_AMR_NB ? HAL_8K_PCM_FRAME_SIZE : HAL_16K_PCM_FRAME_SIZE);
	}
	if(!recordParam->bContinue)
    	file_fwrite(amrHead, strlen(amrHead), 1, file);
    amrFifoInit();
	audioRegisterStopRecordCb(audioStopRecordCallback);
   
    audioAppStartRecord(codecCfg.codecType);

    gTimeBegin   = 0;
    gRecordState = AMR_RECORD_START;
	
	osTimerStart(gAmrRecordTimer, RECORD_TIMER_LOOP_MS);
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrRecord_2, P_INFO, "[APP AMR Record] recordParam->time=%d", recordParam->time);
    while (1)
    {
    	if(osMessageQueueGet(gAudAmrRecordQueue, &queue, 0, osWaitForever) == osOK)
		{
			if((queue.endFlag == true) || (gRecordState != AMR_RECORD_START))
			{
				retVal = AV_RET_RECORD_STOP;
				break;			
			}
        length = amrFifoRead(buffer, readLen);
        if (length > 0)
        {
            	file_fwrite(buffer, 1, length, file);
	            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrRecord_3, P_INFO, "[APP AMR Record] count=%d, length=%d", count++, length);
        }
        timeEnd = osKernelGetTickCount();
		if(recordParam->time)
		{
	        if ((gTimeBegin != 0) && ((timeEnd - gTimeBegin) > (recordParam->time * 1000)))
	        {
		        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrRecord_4, P_INFO, "[APP AMR Record] timeEnd=%d", timeEnd);
				retVal = AV_RET_RECORD_EOF;
	            break;
	        }
		}
    }
    }
    stopRecord();
EXIT:
	if(gAudAmrRecordQueue != NULL)
	{
		osMessageQueueDelete(gAudAmrRecordQueue);
		gAudAmrRecordQueue = NULL;
	}
    
    if(osTimerIsRunning(gAmrRecordTimer))
	{
		osTimerStop(gAmrRecordTimer);
	}
	osTimerDelete(gAmrRecordTimer);
  	gAmrRecordTimer = NULL;
	
    file_fclose(file);

    return retVal;
}

#ifdef MBTK_OPENCPU_SUPPORT
int32_t ol_amrRecord(RecordCallbackT callback,RecordParamT *recordParam)
{
    int32_t                 retVal  = -1;
    ol_RecordCallbackT      callback_ptr = (ol_RecordCallbackT)callback;
    uint32_t                count   = 0;
    uint32_t                timeEnd = 0;
    int32_t                 length  = 0;
    uint8_t                 buffer[HAL_16K_PCM_FRAME_SIZE] = {0};
    uint32_t                readLen = HAL_RTP_ONE_AMR_FRAME_MAX_SIZE;
    QueueTonePlayT          queue = {0};
    char                   *amrHead    = (AMR_CODEC_TYPE == HAL_VC_AMR_NB) ? AMR_NB_HEAD : AMR_WB_HEAD;
    HalVoiceCodecConfigReq  codecCfg   =
    {
        .codecType       = AMR_CODEC_TYPE,
        .encBitRate      = (AMR_CODEC_TYPE == HAL_VC_AMR_NB) ? HAL_AMR_NB_FT_7 : HAL_AMR_WB_FT_8,
        .encOutBitOffset = AMR_BIT_OFFSET
    };

    if ((recordParam == NULL) || (callback_ptr == NULL))
    {
        OL_LOG_PRINTF("[AMR Record] Param error");
        return retVal;
    }

    if(recordParam->samplerate > SAMPLERATE_8K)
    {
        codecCfg.codecType = HAL_VC_AMR_WB;
        codecCfg.encBitRate = HAL_AMR_WB_FT_8;
        amrHead = AMR_WB_HEAD;
    }

    OL_LOG_PRINTF("[AMR Record] amrRecord: %s, %d, %d, %d", amrHead, codecCfg.codecType, codecCfg.encBitRate, codecCfg.encOutBitOffset);

    if (gAmrRecordTimer == NULL)
    {
        gAmrRecordTimer = osTimerNew(amrRecordTimerFunc, osTimerPeriodic, NULL, NULL);
        if (gAmrRecordTimer == NULL)
        {
            OL_LOG_PRINTF("[AMR Record] Failed to create timer for gAmrTimer");
            return retVal;
        }
    }

    gAudAmrRecordQueue = osMessageQueueNew(100, sizeof(queue), NULL);
    if(gAudAmrRecordQueue == NULL)
    {
        OL_LOG_PRINTF("[AMR Record] Failed to create gAudAmrQueue");
        if(osTimerIsRunning(gAmrRecordTimer))
        {
            osTimerStop(gAmrRecordTimer);
        }
        osTimerDelete(gAmrRecordTimer);
        gAmrRecordTimer = NULL;
        return retVal;
    }

    //halSetVoiceEngRetCallback(amrEngCallback);
    //halVoiceEngStartReq();
    // amrEngWaitCpReply(HAL_VOICE_ENG_START_CNF);
    //halVoiceCodecConfigReq(&codecCfg);
    //amrEngWaitCpReply(HAL_VOICE_CODEC_CONFIG_CNF);
    if(amrEngCommonStart(&codecCfg,AUDIO_AMR_ENG_ENCODE_FLAG))
    {
        goto EXIT;
    }
    if(recordParam->codec == AUDIO_RECORD_CODEC_PCM || recordParam->codec == AUDIO_RECORD_CODEC_PCM_3A)
    {
        amrEngSetRecPcmFlag(TRUE);
        readLen =  (codecCfg.codecType == HAL_VC_AMR_NB ? HAL_8K_PCM_FRAME_SIZE : HAL_16K_PCM_FRAME_SIZE);
    }
    else
    {
        if(!recordParam->bContinue)
        {
            callback_ptr(amrHead, strlen(amrHead));
        }
    }

    amrFifoInit();
    audioRegisterStopRecordCb(audioStopRecordCallback);
    audioAppStartRecord(codecCfg.codecType);

    gTimeBegin   = 0;
    gRecordState = AMR_RECORD_START;

    osTimerStart(gAmrRecordTimer, RECORD_TIMER_LOOP_MS);
    OL_LOG_PRINTF("[AMR Record] recordParam->time=%d", recordParam->time);
    while (1)
    {
        if(osMessageQueueGet(gAudAmrRecordQueue, &queue, 0, osWaitForever) == osOK)
        {
            if((queue.endFlag == true) || (gRecordState != AMR_RECORD_START))
            {
                break;
            }
            length = amrFifoRead(buffer, readLen);
            if (length > 0)
            {
                OL_LOG_PRINTF("[AMR Record] count=%d, length=%d", count++, length);
                callback_ptr(buffer, length);
            }
            timeEnd = osKernelGetTickCount();
            if(recordParam->time)
            {
                if ((gTimeBegin != 0) && ((timeEnd - gTimeBegin) > (recordParam->time * 1000)))
                {
                    OL_LOG_PRINTF("[AMR Record] timeEnd=%d", timeEnd);
                    break;
                }
            }
        }
    }

    stopRecord();

EXIT:
    if(gAudAmrRecordQueue != NULL)
    {
        osMessageQueueDelete(gAudAmrRecordQueue);
        gAudAmrRecordQueue = NULL;
    }

    if(osTimerIsRunning(gAmrRecordTimer))
    {
        osTimerStop(gAmrRecordTimer);
    }
    osTimerDelete(gAmrRecordTimer);
    gAmrRecordTimer = NULL;

    return 0;
}
#endif
#endif

