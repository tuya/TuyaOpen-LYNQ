/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    openRecorder.c
 * Description:  EC718 media play source file
 * History:      Rev1.0   2024-03-13
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include DEBUG_LOG_HEADER_FILE
#include <time.h>
#include <string.h>
#include "osasys.h"
#include "ps_lib_api.h"
#include "os_common.h"
#include "openrecorder.h"
#include "record.h"
#include "audAmrRecord.h"
#include "media.h"

/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/
#define OPEN_RECORDER_DST_DISK 			"D:/"
#define OPEN_RECORDER_DST_PATH_LEN		(30)

typedef struct _openRecorderCheckSta{
	bool					isPause;
	time_t 					beginTime;
	time_t 					durationTime;
	openRecorderRecStatus 	recStat;
}openRecorderCheckStaT;

typedef struct _openRecorderHandlerT{
	void *dst;
	void *userdata;
	openRecorderConfigT config;
	openRecorderCheckStaT status;
	osSemaphoreId_t openRecorderSemaphore;
	openRecorderCpltCallbackT recCpltcb;
	openRecorderGetDataCallbackT recDataCb;
}openRecorderHandlerT;


typedef struct _openRecorderMedTypeT{
	const char *suffix;
	openRecorderCodecT RecCodec;
}openRecorderMedTypeT;
/*----------------------------------------------------------------------------*
 *                      GLOBAL VARIABLES                                      *
 *----------------------------------------------------------------------------*/

openRecorderCheckStaT gOpenRecorderCheckSta = {0};
static openRecorderHandlerT *gOpenRecorderHandler = NULL;
openRecorderMedTypeT openRecoderMedTypeCkList[] = 
{
	{".pcm", AUDIO_RECORD_CODEC_PCM_3A},
	{".amr", AUDIO_RECORD_CODEC_AMR},
	{NULL, 	 AUDIO_RECORD_CODEC_IDLE},
};

/*----------------------------------------------------------------------------*
 *                      PRIVATE FUNCTION DECLEARATION                         *
 *----------------------------------------------------------------------------*/
static void openRecorderInterCallback(int32_t result)
{
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, openRecorderInterCallback, P_INFO, "[openRecord] stop callback");
	time_t currTime = 0;
	utc_timer_value_t time = {0};
	appGetSystemTimeUtcSync(&time);
	currTime = time.UTCsecs;
    
	gOpenRecorderHandler->status.durationTime = gOpenRecorderHandler->status.durationTime + (currTime - gOpenRecorderHandler->status.beginTime);
	gOpenRecorderHandler->status.beginTime = 0;

	if(gOpenRecorderHandler)
	{
		if(gOpenRecorderHandler->recCpltcb != NULL)
			gOpenRecorderHandler->recCpltcb(gOpenRecorderHandler->userdata,result);
	}
	
	if(gOpenRecorderHandler->status.recStat ==  OPEN_RECORD_STA_START)
	{
		if(gOpenRecorderHandler->status.isPause)
		{
			if(gOpenRecorderHandler->config.recordParam.recordTime > 0)
			{
				if(gOpenRecorderHandler->status.durationTime < gOpenRecorderHandler->config.recordParam.recordTime)
				{
					gOpenRecorderHandler->config.recordParam.recordTime = gOpenRecorderHandler->config.recordParam.recordTime - gOpenRecorderHandler->status.durationTime;
					gOpenRecorderHandler->status.recStat = OPEN_RECORD_STA_PAUSE;
				}
				else
				{
					gOpenRecorderHandler->config.recordParam.recordTime = 0;
					if(gOpenRecorderHandler->dst != NULL)
					{
						free(gOpenRecorderHandler->dst);			
					}
					gOpenRecorderHandler->status.recStat = OPEN_RECORD_STA_IDLE;
					gOpenRecorderHandler->status.isPause = FALSE;
					gOpenRecorderHandler->recCpltcb = NULL;
					gOpenRecorderHandler->recDataCb = NULL;
				}
			}
			else
				gOpenRecorderHandler->status.recStat = OPEN_RECORD_STA_PAUSE;
		}			
		else
		{
			gOpenRecorderHandler->recCpltcb = NULL;
			gOpenRecorderHandler->recDataCb = NULL;
			if(gOpenRecorderHandler->dst != NULL){
				free(gOpenRecorderHandler->dst);
				gOpenRecorderHandler->dst = NULL;
			}
			gOpenRecorderHandler->status.recStat = OPEN_RECORD_STA_IDLE;
		}
			
	}
}

static openRecorderErrCodeT __attribute__((used)) openRecorderLock(void* arg)
{
	EC_ASSERT(arg != NULL,0,0,0);
    osSemaphoreId_t openRecorderLock = (osSemaphoreId_t)arg;

    if (osSemaphoreAcquire(openRecorderLock, 1000) != osOK)
    {
        return OPEN_REC_RET_ERR_TIMEOUT;
    }
    return  OPEN_REC_RET_OK;
}

static openRecorderErrCodeT __attribute__((used)) openRecorderUnlock(void* arg)
{
	EC_ASSERT(arg != NULL,0,0,0);
    osSemaphoreId_t openRecorderLock = (osSemaphoreId_t)arg;

    if (osSemaphoreRelease(openRecorderLock) != osOK)
    {
        return OPEN_REC_RET_ERR_TIMEOUT;
    }
    return  OPEN_REC_RET_OK;
}

static openRecorderErrCodeT openRecorderLockDestroy(void *arg)
{
	EC_ASSERT(arg != NULL,0,0,0);
    osSemaphoreId_t codecLock = (osSemaphoreId_t)arg;

    if (osSemaphoreDelete(codecLock) != osOK)
    {
        return OPEN_REC_RET_ERR_TIMEOUT;
    }
    return  OPEN_REC_RET_OK;
}

openRecorderCodecT openRecorderCheckSrcTypeFunc(const char *dst)
{
	if((dst == NULL) || (strlen(dst) == 0))
		return AUDIO_RECORD_CODEC_IDLE;
	
	uint8_t cnt = 0;
	uint32_t srcLen = 0;
	uint8_t  suffixLen = 0;
	uint8_t srcType = AUDIO_RECORD_CODEC_IDLE;
	srcLen = strlen(dst);
			
	for(cnt = 0; cnt < sizeof(openRecoderMedTypeCkList) / sizeof(openRecorderMedTypeT); cnt++)
	{
		suffixLen = strlen(openRecoderMedTypeCkList[cnt].suffix);
		if((srcLen >= suffixLen) && !strncasecmp(dst + srcLen - suffixLen, openRecoderMedTypeCkList[cnt].suffix,suffixLen))
		{
			srcType = openRecoderMedTypeCkList[cnt].RecCodec;
			break;
		}
	}
	return srcType;
}


/*----------------------------------------------------------------------------*
 *                      GLOBAL FUNCTIONS                                      *
 *----------------------------------------------------------------------------*/

 /**
  \brief	create recorder handler
  \param[in] open recorder param,if NULL,use the default param
  \return	 recorder handler or NULL
 */
 openRecorder openRecorderCreate(openRecorderConfigT *param)
{
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, openRecorderCreate, P_INFO, "[openRecord] recorder Create");
	if(gOpenRecorderHandler != NULL)
	{
		goto INIT_PARAM;
	}
	else
	{
		gOpenRecorderHandler = (openRecorderHandlerT *)malloc(sizeof(openRecorderHandlerT));
		if(gOpenRecorderHandler == NULL)
		{
			goto INIT_EOF;
		}
	
	}

	memset(&gOpenRecorderHandler->config,0x00,sizeof(openRecorderConfigT));
	memset(&gOpenRecorderHandler->status,0x00,sizeof(openRecorderCheckStaT));
	gOpenRecorderHandler->dst = NULL;
	gOpenRecorderHandler->recCpltcb = NULL;
	gOpenRecorderHandler->recDataCb = NULL;
	gOpenRecorderHandler->openRecorderSemaphore = osSemaphoreNew(1U,0,NULL);
INIT_PARAM:
	if(param != NULL)
		memcpy(&gOpenRecorderHandler->config, param,sizeof(openRecorderConfigT));
INIT_EOF:
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, openRecorderCreate_1, P_INFO, "[openRecord] recorder time:%d",gOpenRecorderHandler->config.recordParam.recordTime);

	return (openRecorder)gOpenRecorderHandler;
}

 /**
  \brief	destory recorder handler
  \param[in] recorder handler
  \return	 void
 */
void openRecorderDestory(openRecorder recorder)
{
	if(recorder == NULL)
		return;
	openRecorderHandlerT *handler = (openRecorderHandlerT *)recorder;
	openRecorderLockDestroy(handler->openRecorderSemaphore);
	handler->openRecorderSemaphore = NULL;
	handler->recCpltcb = NULL;
	handler->recDataCb = NULL;
	if(handler->dst != NULL){
		free(handler->dst);
		handler->dst = NULL;
		}
	free(handler);
	handler = NULL;
	gOpenRecorderHandler = NULL;
	return;	
	
}

/**
 \brief    set recorder completed callback
 \param[in] recorder handler
 \param[in] recorder completed callback
 \param[in] userdate
 \return	void
*/
void openRecorderSetCallback(openRecorder recorder,openRecorderCpltCallbackT callback, void * userdata)
{

	if(recorder == NULL)
		return;
	openRecorderHandlerT *handler = (openRecorderHandlerT *)recorder;
	handler->recCpltcb = callback;
	handler->userdata = userdata;
}

 /**
  \brief	set recorder get data callback.The callback will be called every 20ms,
  \			 the record data will be sent to the callback and not be saved to the file. 
  \param[in] recorder handler
  \param[in] recorder completed callback
  \param[in] userdate
  \return	 void
 */
 void openRecorderGetDataCallback(openRecorder recorder,openRecorderGetDataCallbackT callback)
 {
 	 if(recorder == NULL)
		 return;
	 openRecorderHandlerT *handler = (openRecorderHandlerT *)recorder;
	 handler->recDataCb = callback;
 }


 /**
  \brief   start record
  \param[in] recorder handler
  \param[in] dst,the destination to save the record source,it can be path or filename 
  \return    0-----success, < 0-----fail
 */
int8_t openRecorderStart(openRecorder recorder,void *dst)
{
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, openRecorderStart, P_INFO, "[openRecord] recorder start");
	int8_t ret = OPEN_REC_RET_OK;
	if(recorder == NULL)
		return OPEN_REC_RET_ERR_PARAM;
	//get current time
    utc_timer_value_t timeUtc;

	uint16_t recCodec = AUDIO_RECORD_CODEC_IDLE;
	openRecorderHandlerT *handler = (openRecorderHandlerT *)recorder;
	if(handler->status.recStat != OPEN_RECORD_STA_IDLE)
		return OPEN_REC_RET_ERR_NOMAL;
	if(!dst)
	{
		if(handler->recDataCb == NULL)
		{
			static uint32_t recCnt = 0;
			if(!handler->dst)
				handler->dst = malloc(OPEN_RECORDER_DST_PATH_LEN);
			else
				handler->dst = realloc(handler->dst,OPEN_RECORDER_DST_PATH_LEN);
			EC_ASSERT(handler->dst,handler->dst,0,0);
			memset(handler->dst,0x00,OPEN_RECORDER_DST_PATH_LEN);
		    if(!appGetSystemTimeUtcSync(&timeUtc))
		    {
		    	switch(handler->config.recordParam.codec)
	    		{
	    			case AUDIO_RECORD_CODEC_PCM:
					case AUDIO_RECORD_CODEC_PCM_3A:
						snprintf(handler->dst, OPEN_RECORDER_DST_PATH_LEN, "%s%d%02d%02d_%02d%02d%02d.pcm", OPEN_RECORDER_DST_DISK,timeUtc.UTCtimer1>>16, (timeUtc.UTCtimer1&0xFF00)>>8, (timeUtc.UTCtimer1&0xFF), (timeUtc.UTCtimer2>>24)+8, (timeUtc.UTCtimer2&0xFF0000)>>16, (timeUtc.UTCtimer2&0xFF00)>>8);
						break;
					default:
						snprintf(handler->dst, OPEN_RECORDER_DST_PATH_LEN, "%s%d%02d%02d_%02d%02d%02d.amr", OPEN_RECORDER_DST_DISK,timeUtc.UTCtimer1>>16, (timeUtc.UTCtimer1&0xFF00)>>8, (timeUtc.UTCtimer1&0xFF), (timeUtc.UTCtimer2>>24)+8, (timeUtc.UTCtimer2&0xFF0000)>>16, (timeUtc.UTCtimer2&0xFF00)>>8);
						break;						
	    		}
		    }
		    else
		    {
		    	switch(handler->config.recordParam.codec)
	    		{
	    			case AUDIO_RECORD_CODEC_PCM:
					case AUDIO_RECORD_CODEC_PCM_3A:
						snprintf(handler->dst, OPEN_RECORDER_DST_PATH_LEN, "%srecord%d.pcm",OPEN_RECORDER_DST_DISK,recCnt++);
						break;
					default:
						snprintf(handler->dst, OPEN_RECORDER_DST_PATH_LEN, "%srecord%d.amr",OPEN_RECORDER_DST_DISK,recCnt++);
						break;						
	    		}
		        snprintf(handler->dst, OPEN_RECORDER_DST_PATH_LEN, "%srecord%d.amr",OPEN_RECORDER_DST_DISK,recCnt++);
		    }
		}		
	}
	else
	{
		if(!handler->dst)
			handler->dst = malloc(strlen((char *)dst) + 1);
		else
			handler->dst = realloc(handler->dst,strlen((char *)dst) + 1);
		EC_ASSERT(handler->dst,handler->dst,0,0);
		memset(handler->dst,0x00,strlen((char *)dst) + 1);
		memcpy(handler->dst,dst,strlen(dst));
	}

	//check codec
	recCodec = openRecorderCheckSrcTypeFunc(handler->dst);
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, openRecorderStart_2, P_INFO, "[openRecord] recorder audio type is %d", recCodec);
	if(recCodec == AUDIO_RECORD_CODEC_IDLE)
	{
		if((handler->config.recordParam.codec <= AUDIO_RECORD_CODEC_IDLE) || 
		(handler->config.recordParam.codec >=  AUDIO_RECORD_CODEC_EOF))
		{
			memset(&handler->config,0x00,sizeof(handler->config));
			if(handler->dst)
				free(handler->dst);
			handler->dst = NULL;
			ret = OPEN_REC_RET_ERR_PARAM;
			goto PLAY_EOF;
		}		
	}
	else
	{
		handler->config.recordParam.codec = recCodec;
	}

	RecordParamT recordParam = {0};
	memset(&recordParam,0x00,sizeof(RecordParamT));
	recordParam.time = handler->config.recordParam.recordTime;
	recordParam.codec = handler->config.recordParam.codec;
	recordParam.samplerate = handler->config.recordParam.samplerate;
	recordParam.channel = handler->config.recordParam.channel;
	recordParam.dataCb =  handler->recDataCb;
	if(!EC_audioRecord(handler->dst,recordParam,openRecorderInterCallback))
	{
		handler->status.durationTime = 0;
        utc_timer_value_t time = {0};
	    appGetSystemTimeUtcSync(&time);
	    handler->status.beginTime  = time.UTCsecs;
		handler->status.isPause = FALSE;
		handler->status.recStat = OPEN_RECORD_STA_START;
	}
	else
	{
		handler->config.recordParam.codec = AUDIO_RECORD_CODEC_IDLE;
		if(handler->dst)
			free(handler->dst);
		handler->dst = NULL;
		ret = OPEN_REC_RET_ERR_NOMAL;
	}
PLAY_EOF:
	return ret;
}

/**
 \brief     pause recorder API
 \param[in] recorder handler
 \return	void
*/
void openRecorderPause(openRecorder recorder)
{
	if(recorder == NULL)
		return ;
	openRecorderHandlerT *handler = (openRecorderHandlerT *)recorder;

	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, openRecorderPause, P_INFO, "[openRecord] recorder pause");
	handler->status.isPause = TRUE;
	
	if(handler->status.recStat == OPEN_RECORD_STA_START)
	{
		switch(handler->config.recordParam.codec)
		{
			case AUDIO_RECORD_CODEC_AMR:
			case AUDIO_RECORD_CODEC_PCM:
			case AUDIO_RECORD_CODEC_PCM_3A:
			{
				amrRecordStop(0);
			}
			break;
			default:
			break;
		}
		//handler->status.recStat = OPEN_RECORD_STA_PAUSE;
	}
}

/**
 \brief    stop recorder API
 \param[in] recorder handler
 \return	void
*/
void openRecorderStop(openRecorder recorder)
{
	if(recorder == NULL)
		return ;
	openRecorderHandlerT *handler = (openRecorderHandlerT *)recorder;
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, openRecorderStop, P_INFO, "[openRecord] recorder stop");
	switch (handler->status.recStat)
	{
		case OPEN_RECORD_STA_START:
		{
			switch(handler->config.recordParam.codec)
			{
				case AUDIO_RECORD_CODEC_AMR:
				case AUDIO_RECORD_CODEC_PCM:
				case AUDIO_RECORD_CODEC_PCM_3A:
				{
					amrRecordStop(0);
				}
				break;
				default:
				break;
			}
		}
		break;
		case OPEN_RECORD_STA_PAUSE:
		{
			if(handler->dst != NULL)
			{
				free(handler->dst);			
			}
			handler->status.recStat = OPEN_RECORD_STA_IDLE;
			handler->status.isPause = FALSE;
			handler->recCpltcb = NULL;
			handler->recDataCb = NULL;
		}
		break;
		case OPEN_RECORD_STA_IDLE:
		default:
			break;
	}
	return;
}


/**
 \brief    recorder resume  API
 \param[in] recorder handler
 \return    0-----success, < 0-----fail
*/
int8_t openRecorderResume(openRecorder recorder)
{
	int8_t ret = OPEN_REC_RET_OK;
	if(recorder == NULL)
		return OPEN_REC_RET_ERR_PARAM;
	openRecorderHandlerT *handler = (openRecorderHandlerT *)recorder;
	if(handler->status.recStat == OPEN_RECORD_STA_PAUSE)
	{
		switch(handler->config.recordParam.codec)
		{
			case AUDIO_RECORD_CODEC_AMR:
			case AUDIO_RECORD_CODEC_PCM:
			case AUDIO_RECORD_CODEC_PCM_3A:
			{
				RecordParamT recordParam = {0};
				memset(&recordParam,0x00,sizeof(RecordParamT));
				recordParam.time = handler->config.recordParam.recordTime;
				recordParam.codec = handler->config.recordParam.codec;
				recordParam.samplerate = handler->config.recordParam.samplerate;
				recordParam.channel = handler->config.recordParam.channel;
				recordParam.bContinue = TRUE;
				recordParam.dataCb = handler->recDataCb;
				if(!EC_audioRecord(handler->dst,recordParam,openRecorderInterCallback))
				{
				     utc_timer_value_t time = {0};
	                 appGetSystemTimeUtcSync(&time);
					handler->status.beginTime = time.UTCsecs;
					handler->status.recStat = OPEN_RECORD_STA_START;
					handler->status.isPause = FALSE;
				}
				else
				{
					memset(&handler->config,0x00,sizeof(handler->config));
					if(handler->dst)
						free(handler->dst);
					handler->dst = NULL;
					ret = OPEN_REC_RET_ERR_NOMAL;
				}
			break;
			}
			default:
			break;
		}
	}
	else
	{
		ret = OPEN_REC_RET_ERR_NOMAL;
	}
	return ret;
}

/**
 \brief    get recoder time API
 \param[in] recorder handler
 \return:recorder time
*/
uint32_t openRecorderGetRecTime(openRecorder recorder)
{
	if(recorder == NULL)
		return 0;
	uint32_t playTime = 0;
	openRecorderHandlerT *handler = (openRecorderHandlerT *)recorder;
	if(handler->status.recStat == OPEN_RECORD_STA_START)
	{
	    utc_timer_value_t time = {0};
	    appGetSystemTimeUtcSync(&time);
	    time_t currTime  = time.UTCsecs;

		playTime =currTime - handler->status.beginTime + handler->status.durationTime;  
	}
	else
		playTime = handler->status.durationTime;
	return playTime;
}



/**
 \brief    recorder get pause status API
 \param[in] recorder handler
 \return true ---pause,false ---unpaused
*/
bool openRecorderIsPause(openRecorder recorder)
{
	if(recorder == NULL)
		return FALSE;	
	openRecorderHandlerT *handler = (openRecorderHandlerT *)recorder;
	return (handler->status.recStat == OPEN_RECORD_STA_PAUSE ? TRUE : FALSE);
}
