#ifdef FEATURE_SUBSYS_AMR_ENABLE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "sctdef.h"
#include "cmsis_os2.h"
#include DEBUG_LOG_HEADER_FILE
#include "slpman.h"
#include "hal_i2s.h"
#include "ccio_audio.h"
#include "hal_voice_eng.h"
#include "hal_voice_eng_mem.h"
#include "audAmrCommon.h"
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#include "media.h"
#include "medDataHandle.h"

#define PCM_BUFFER_SIZE              HAL_16K_PCM_FRAME_SIZE

#define DMA_TRUNK_PERIOD            20


#define AMR_DATA_CACHE_PRE_FILL_SIZE			(16*1024)
#define AMR_DATA_CACHE_SIZE						(AMR_DATA_CACHE_PRE_FILL_SIZE + PCM_BUFFER_SIZE * 3)

#ifdef FEATURE_SUBSYS_MEDIA_STREAM_ENABLE
extern ecRingT *gAudStreamRingBuf;
#endif

static osTimerId_t   gAmrTimer        = NULL;
static volatile	bool gPreReadAmrFifo  = false;

osMessageQueueId_t gAudAmrQueue = NULL;

static void amrTimerFunc(void *argument)
{
	QueueTonePlayT queue = {0};
	osStatus_t stat = osOK;
	if(gAudAmrQueue != NULL)
	{
		if(medDataHandleStateGet() >= MED_DATA_HDL_STA_SUSPEND)
		{
			queue.endFlag = true;
		}
		stat = osMessageQueuePut(gAudAmrQueue, &queue, 0, AUD_CODEC_SIG_TIMEROUT_MAX);
		if(stat != osOK)
		{
			ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrTimerFunc_valid, P_ERROR, "message send fail(%d)",stat);
		}
	}
}

int32_t amrPlayStop(void)
{
    ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrPlayStop, P_INFO, "[APP AMR Play] amrPlayStop");
	osStatus_t stat = osOK;
    if(gAudAmrQueue != NULL)
	{
		QueueTonePlayT			queue = {0};
		queue.endFlag = true;
		stat = osMessageQueuePutToFront(gAudAmrQueue,&queue,0,AUD_CODEC_SIG_TIMEROUT_MAX);
		if(stat != osOK)
		{
			ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrPlayStop_valid, P_ERROR, "message send fail(%d)",stat);
		}
	}

    return 0;
}

int32_t amrPlay(BOOL toneFlag, char *path)
{
    int32_t                 retVal      = AV_RET_PLAY_ERROR;
    bool                    engInited   = false;
    bool                    fileOpened  = false;
    FILE                   *file        = NULL;
	struct stat 			buf 		= {0};
    uint8_t                 index       = 0;
    uint8_t                *amrDeBuf    = NULL;
    uint16_t                amrDeBufLen = 0;
    uint8_t                *pcmBuf      = NULL;
	uint16_t				*amrBitlengthPtr	= NULL;
	uint16_t				*amrBytelengthPtr	= NULL;
    uint16_t                pcmBufLen   = 0;
	QueueTonePlayT			queue = {0};
	ecRingT *pcmCache = NULL;
	pcmCache = pcmCache;
	rawDataHanderParam_T dataHandleParam = {0};
	INT32 freeSize = 0;
    HalVoiceCodecConfigReq  codecCfg =
    {
        .encOutBitOffset = AMR_BIT_OFFSET
    };
    HalVoiceDecodeReq decReq =
    {
        .inBitOffset = AMR_BIT_OFFSET,
        .sn          = 0
    };

    if (path == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrPlay_invalid_0, P_ERROR, "[APP AMR Play] Param error");
        goto labelEnd;
    }

    halVEAllocAmrEnFrameBuf(HAL_VE_MEM_APP_CALLER, (void **)(&amrDeBuf), &amrDeBufLen);
    if ((amrDeBuf == NULL) || (amrDeBufLen == 0))
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrPlay_invalid_1, P_ERROR, "[APP AMR Play] Failed to get AMR buffer.");
        goto labelEnd;
    }
    amrDeBuf    += HAL_RTP_HEADER_RSVD_SIZE;
    amrDeBufLen -= HAL_RTP_HEADER_RSVD_SIZE;

    file = file_fopen(path, "r");
    if (file == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrPlay_invalid_4, P_ERROR, "[APP AMR Play] Failed to open file %s: %d", path, retVal);
        goto labelEnd;
    }
	fileOpened = true;
    memset(amrDeBuf, 0, amrDeBufLen);
    file_fread(amrDeBuf, sizeof(char), strlen(AMR_WB_HEAD), file);
    if (memcmp(amrDeBuf, AMR_NB_HEAD, strlen(AMR_NB_HEAD)) == 0)
    {
    	codecCfg.codecType = HAL_VC_AMR_NB;
    	codecCfg.encBitRate = HAL_AMR_NB_FT_7;
		decReq.codecType = HAL_VC_AMR_NB;
		amrBitlengthPtr = (uint16_t *)gAmrNbBitLength;
		amrBytelengthPtr = (uint16_t *)gAmrNbByteLength;
		halVEAllocAppPcmBuf( HAL_VE_8KHZ_16B_PCM, HAL_VE_MEM_APP_CALLER, (void **)(&pcmBuf), &pcmBufLen);
		if ((pcmBuf == NULL) || (pcmBufLen == 0))
	    {
	        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrPlay_invalid_3, P_ERROR, "[APP AMR Play] Failed to get PCM buffer.");
	        goto labelEnd;
	    }
        file_fseek(file, strlen(AMR_NB_HEAD), SEEK_SET);
    }
    else if (memcmp(amrDeBuf, AMR_WB_HEAD, strlen(AMR_WB_HEAD)) == 0)
    {
        codecCfg.codecType = HAL_VC_AMR_WB;
    	codecCfg.encBitRate = HAL_AMR_WB_FT_8;
		decReq.codecType = HAL_VC_AMR_WB;
		amrBitlengthPtr = (uint16_t *)gAmrWbBitLength;
		amrBytelengthPtr = (uint16_t *)gAmrWbByteLength;
		halVEAllocAppPcmBuf( HAL_VE_16KHZ_16B_PCM, HAL_VE_MEM_APP_CALLER, (void **)(&pcmBuf), &pcmBufLen);
		if ((pcmBuf == NULL) || (pcmBufLen == 0))
	    {
	        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrPlay_invalid_3, P_ERROR, "[APP AMR Play] Failed to get PCM buffer.");
	        goto labelEnd;
	    }
        file_fseek(file, strlen(AMR_WB_HEAD), SEEK_SET);
    }
	else
	{
	    goto labelEnd;
	}
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrPlay_0, P_INFO, "[APP AMR Play] amrPlay: %d, %d, %d", codecCfg.codecType, codecCfg.encBitRate, codecCfg.encOutBitOffset);
		

    if(amrEngCommonStart(&codecCfg,AUDIO_AMR_ENG_DECODE_FLAG))
	{
		goto labelEnd;
	}
	
    engInited = true;

    if (gAmrTimer == NULL)
    {
        gAmrTimer = osTimerNew(amrTimerFunc, osTimerPeriodic, NULL, NULL);
        if (gAmrTimer == NULL)
        {
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrPlay_invalid_6, P_ERROR, "[APP AMR Play] Failed to create timer for gAmrTimer");
            goto labelEnd;
        }
    }
	if(medDataHandleStateGet() != MED_DATA_HDL_STA_START)
	{
		pcmCache = medInitDataCache(AMR_DATA_CACHE_SIZE,AMR_DATA_CACHE_PRE_FILL_SIZE);
		dataHandleParam.field.samplerate = (codecCfg.codecType == HAL_VC_AMR_NB ? SAMPLERATE_8K : SAMPLERATE_16K);
		dataHandleParam.field.envType = (toneFlag ? MED_DATA_ENV_TYPE_TONE : MED_DATA_ENV_TYPE_LOCAL);
		dataHandleParam.field.bitWidth = 0;//default 16bit
		medDataHandleStart(&dataHandleParam);
	}
	else
	{
		pcmCache = medGetDataCache();
	}
	gAudAmrQueue = osMessageQueueNew(100, sizeof(queue), NULL);
	if(gAudAmrQueue == NULL)
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrPlay_invalid_7, P_ERROR, "[APP AMR Play] Failed to create gAudAmrQueue");
        goto labelEnd;
	}
	osTimerStart(gAmrTimer, DMA_TRUNK_PERIOD);
    while (1)
    {
    	memset(&queue, 0, sizeof(queue));
        if(osMessageQueueGet(gAudAmrQueue, &queue, 0, osWaitForever) == osOK)
    	{
			if(queue.endFlag == true)
			{
				ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrPlay_1, P_INFO, "[APP AMR Play] queue endflag [%d]",queue.endFlag);
				if(medDataHandleStateGet() >= MED_DATA_HDL_STA_SUSPEND)
					retVal = AV_RET_PLAY_EOF;
				else
					retVal = AV_RET_PLAY_STOP;
				medDataHandleStop();
				break;			
			}
			freeSize = medGetDataCacheAvlbSize();
			if(freeSize <= PCM_BUFFER_SIZE*2)/*16 bit*/
			{
				continue;
			}
	        memset(amrDeBuf, 0, amrDeBufLen);
	        if (file_fread(amrDeBuf, sizeof(char), 1, file) <= 0)
	        {
	        	if(toneFlag)
	    		{
					break;
	    		}
				else
				{
		            continue;
				}
	        }

	        for (index=0; index<AMR_FT_COUNT; index++)
	        {
	            if (amrDeBuf[0] == gAmrNbHead[index])
	            {
	            	if(amrBytelengthPtr[index] != 0)
	                	file_fread(amrDeBuf, sizeof(char), amrBytelengthPtr[index], file);
	                decReq.amrBitLen   = amrBitlengthPtr[index];
	                decReq.pAmrData    = amrDeBuf;
	                decReq.pcmBitWidth = PCM_BIT_WIDTH;
	                decReq.pcmBufLen   = PCM_BUFFER_SIZE;
	                decReq.pPcmData    = pcmBuf;
	                halVoiceDecodeReq(&decReq);
	                break;
	            }
	        }
			file_fstat((int)file, &buf);
            if ((file_ftell(file) == buf.st_size) && EC_audioPlayChkLoop())
            {
            	if(codecCfg.codecType == HAL_VC_AMR_NB)
					file_fseek(file, strlen(AMR_NB_HEAD), SEEK_SET);
				else
					file_fseek(file, strlen(AMR_WB_HEAD), SEEK_SET);
            }
	        if (index >= AMR_FT_COUNT)
	        {
	            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrPlay_3, P_INFO, "[APP AMR Play] Unknown AMR type: 0x%x", amrDeBuf[0]);
	            break;
	        }
	        amrEngWaitCpReply(HAL_VOICE_DECODE_CNF);
    	}
    }

labelEnd:
    if (engInited == true)
    {
        amrEngCommonStop(AUDIO_AMR_ENG_DECODE_FLAG);
    }

	if(gAudAmrQueue != NULL)
	{
		osMessageQueueDelete(gAudAmrQueue);
		gAudAmrQueue = NULL;
    }

    if (amrDeBuf != NULL)
    {
        amrDeBuf -= HAL_RTP_HEADER_RSVD_SIZE;
        halVEFreeAmrEnFrameBuf((void **)(&amrDeBuf));
    }

    if (pcmBuf != NULL)
    {
        halVEFreeAppPcmBuf((void **)(&pcmBuf));
    }

    if (fileOpened == true)
    {
        file_fclose(file);
    }

    if (gAmrTimer != NULL)
    {
        if (osTimerIsRunning(gAmrTimer))
        {
            osTimerStop(gAmrTimer);
        }
        osTimerDelete(gAmrTimer);
        gAmrTimer = NULL;
    }

    return retVal;
}

int32_t amrPlayString(BOOL toneFlag, char *string,uint32_t stringLen)
{
	int32_t 				retVal		= AV_RET_PLAY_ERROR;
	bool					engInited	= false;
	uint8_t 				*inputPtr = NULL;
	uint8_t 				index		= 0;
	uint32_t                 remainSize = 0;
	uint8_t 			   *amrDeBuf	= NULL;
	uint16_t				amrDeBufLen = 0;
	uint8_t 			   *pcmBuf		= NULL;
	uint16_t				*amrBitlengthPtr	= NULL;
	uint16_t				*amrBytelengthPtr	= NULL;
	uint16_t				pcmBufLen	= 0;
	QueueTonePlayT			queue = {0};
	ecRingT *pcmCache = NULL;
	pcmCache = pcmCache;
	rawDataHanderParam_T dataHandleParam = {0};
	INT32 freeSize = 0;
	HalVoiceCodecConfigReq	codecCfg =
	{
		.encOutBitOffset = AMR_BIT_OFFSET
	};
	HalVoiceDecodeReq decReq =
	{
		.inBitOffset = AMR_BIT_OFFSET,
		.sn 		 = 0
	};

	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrPlayString_0, P_INFO, "[APP AMR Play] amrPlay: %d, %d, %d", codecCfg.codecType, codecCfg.encBitRate, codecCfg.encOutBitOffset);

	if (string == NULL || stringLen == 0)
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrPlayString_invalid_0, P_ERROR, "[APP AMR Play] Param error");
		goto labelEnd;
	}
	inputPtr = (uint8_t *)string;
	remainSize = stringLen;
	halVEAllocAmrEnFrameBuf(HAL_VE_MEM_APP_CALLER, (void **)(&amrDeBuf), &amrDeBufLen);
	if ((amrDeBuf == NULL) || (amrDeBufLen == 0))
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrPlayString_invalid_1, P_ERROR, "[APP AMR Play] Failed to get AMR buffer.");
		goto labelEnd;
	}
	amrDeBuf	+= HAL_RTP_HEADER_RSVD_SIZE;
	amrDeBufLen -= HAL_RTP_HEADER_RSVD_SIZE;

	memset(amrDeBuf, 0, amrDeBufLen);
	memcpy(amrDeBuf,inputPtr,strlen(AMR_WB_HEAD));
	if (memcmp(amrDeBuf, AMR_NB_HEAD, strlen(AMR_NB_HEAD)) == 0)
	{
		codecCfg.codecType = HAL_VC_AMR_NB;
    	codecCfg.encBitRate = HAL_AMR_NB_FT_7;
		decReq.codecType = HAL_VC_AMR_NB;
		amrBitlengthPtr = (uint16_t *)gAmrNbBitLength;
		amrBytelengthPtr = (uint16_t *)gAmrNbByteLength;
		halVEAllocAppPcmBuf( HAL_VE_8KHZ_16B_PCM, HAL_VE_MEM_APP_CALLER, (void **)(&pcmBuf), &pcmBufLen);
		if ((pcmBuf == NULL) || (pcmBufLen == 0))
	    {
	        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrPlay_invalid_3, P_ERROR, "[APP AMR Play] Failed to get PCM buffer.");
	        goto labelEnd;
	    }
		inputPtr = inputPtr + strlen(AMR_NB_HEAD);
		remainSize = remainSize - strlen(AMR_NB_HEAD);
	}
	else if (memcmp(amrDeBuf, AMR_WB_HEAD, strlen(AMR_WB_HEAD)) == 0)
	{
		codecCfg.codecType = HAL_VC_AMR_WB;
    	codecCfg.encBitRate = HAL_AMR_WB_FT_8;
		decReq.codecType = HAL_VC_AMR_WB;
		amrBitlengthPtr = (uint16_t *)gAmrWbBitLength;
		amrBytelengthPtr = (uint16_t *)gAmrWbByteLength;
		halVEAllocAppPcmBuf( HAL_VE_16KHZ_16B_PCM, HAL_VE_MEM_APP_CALLER, (void **)(&pcmBuf), &pcmBufLen);
		if ((pcmBuf == NULL) || (pcmBufLen == 0))
	    {
	        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrPlay_invalid_3, P_ERROR, "[APP AMR Play] Failed to get PCM buffer.");
	        goto labelEnd;
	    }
		inputPtr = inputPtr + strlen(AMR_WB_HEAD);
		remainSize = remainSize - strlen(AMR_WB_HEAD);
	}
	else
	{
		 goto labelEnd;
	}	

	if(amrEngCommonStart(&codecCfg,AUDIO_AMR_ENG_DECODE_FLAG))
	{
		goto labelEnd;
	}
	
	engInited = true;

	if (gAmrTimer == NULL)
	{
		gAmrTimer = osTimerNew(amrTimerFunc, osTimerPeriodic, NULL, NULL);
		if (gAmrTimer == NULL)
		{
			ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrPlayString_invalid_4, P_ERROR, "[APP AMR Play] Failed to create timer for gAmrTimer");
			goto labelEnd;
		}
	}
	if(medDataHandleStateGet() != MED_DATA_HDL_STA_START)
	{
		pcmCache = medInitDataCache(AMR_DATA_CACHE_SIZE,AMR_DATA_CACHE_PRE_FILL_SIZE);
		dataHandleParam.field.samplerate = (codecCfg.codecType == HAL_VC_AMR_NB ? SAMPLERATE_8K : SAMPLERATE_16K);
		dataHandleParam.field.envType = (toneFlag ? MED_DATA_ENV_TYPE_TONE : MED_DATA_ENV_TYPE_LOCAL);
		dataHandleParam.field.bitWidth = 0;//default 16bit
		medDataHandleStart(&dataHandleParam);
	}
	else
	{
		pcmCache = medGetDataCache();
	}
	
	gAudAmrQueue = osMessageQueueNew(100, sizeof(queue), NULL);
	if(gAudAmrQueue == NULL)
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrPlayString_invalid_5, P_ERROR, "[APP AMR Play] Failed to create gAudAmrQueue");
		goto labelEnd;
	}
	osTimerStart(gAmrTimer, DMA_TRUNK_PERIOD);
	while (1)
	{
		memset(&queue, 0, sizeof(queue));
		if(osMessageQueueGet(gAudAmrQueue, &queue, 0, osWaitForever) == osOK)
		{
			if(queue.endFlag == true)
			{
				ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrPlayString_1, P_INFO, "[APP AMR Play] queue endflag [%d]",queue.endFlag);
				if(medDataHandleStateGet() >= MED_DATA_HDL_STA_SUSPEND)
					retVal = AV_RET_PLAY_EOF;
				else
					retVal = AV_RET_PLAY_STOP;
				medDataHandleStop();
				break;			
			}
			freeSize = medGetDataCacheAvlbSize();
			if(freeSize <= PCM_BUFFER_SIZE*2)/*16 bit*/
			{
				continue;
			}
			memset(amrDeBuf, 0, amrDeBufLen);
			
			if(remainSize <= 0)
			{
				if(toneFlag || EC_audioPlayChkLoop())
				{
					remainSize = stringLen;
					inputPtr = (uint8_t *)string;
					memcpy(amrDeBuf,inputPtr,strlen(AMR_WB_HEAD));
					if (memcmp(amrDeBuf, AMR_NB_HEAD, strlen(AMR_NB_HEAD)) == 0)
					{
						inputPtr = inputPtr + strlen(AMR_NB_HEAD);
						remainSize = remainSize - strlen(AMR_NB_HEAD);						
					}
					else if (memcmp(amrDeBuf, AMR_WB_HEAD, strlen(AMR_WB_HEAD)) == 0)
					{
						inputPtr = inputPtr + strlen(AMR_WB_HEAD);
						remainSize = remainSize - strlen(AMR_WB_HEAD);
					}
					else
					{
						break;
					}
					memcpy(amrDeBuf,inputPtr,1);
					remainSize = remainSize - 1;
					inputPtr = inputPtr + 1;
				}				
				else
				{
					continue;
				}
			}
			else
			{

				memcpy(amrDeBuf,inputPtr,1);
				remainSize = remainSize - 1;
				inputPtr = inputPtr + 1;
			}

			for (index=0; index<AMR_FT_COUNT; index++)
			{
				if (amrDeBuf[0] == gAmrNbHead[index])
				{
					if(amrBytelengthPtr[index] != 0)
					{
						memcpy(amrDeBuf,inputPtr,amrBytelengthPtr[index]);
						inputPtr = inputPtr + amrBytelengthPtr[index];
						remainSize = remainSize - amrBytelengthPtr[index];
					}
						
					decReq.amrBitLen   = amrBitlengthPtr[index];
					decReq.pAmrData    = amrDeBuf;
					decReq.pcmBitWidth = PCM_BIT_WIDTH;
					decReq.pcmBufLen   = PCM_BUFFER_SIZE;
					decReq.pPcmData    = pcmBuf;
					halVoiceDecodeReq(&decReq);
					break;
				}
			}

			if (index >= AMR_FT_COUNT)
			{
				ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrPlayString_2, P_INFO, "[APP AMR Play] Unknown AMR type: 0x%x", amrDeBuf[0]);
				break;
			}
			amrEngWaitCpReply(HAL_VOICE_DECODE_CNF);
		}
	}

labelEnd:
	if (engInited == true)
	{
		amrEngCommonStop(AUDIO_AMR_ENG_DECODE_FLAG);
	}

	if(gAudAmrQueue != NULL)
	{
		osMessageQueueDelete(gAudAmrQueue);
		gAudAmrQueue = NULL;
	}

	if (amrDeBuf != NULL)
	{
		amrDeBuf -= HAL_RTP_HEADER_RSVD_SIZE;
		halVEFreeAmrEnFrameBuf((void **)(&amrDeBuf));
	}

	if (pcmBuf != NULL)
	{
		halVEFreeAppPcmBuf((void **)(&pcmBuf));
	}

	if (gAmrTimer != NULL)
	{
		if (osTimerIsRunning(gAmrTimer))
		{
			osTimerStop(gAmrTimer);
		}
		osTimerDelete(gAmrTimer);
		gAmrTimer = NULL;
	}

	return retVal;
}


int32_t amrPlayStream(BOOL toneFlag)
{
	int32_t 				retVal		= AV_RET_PLAY_ERROR;
#ifdef FEATURE_SUBSYS_MEDIA_STREAM_ENABLE
	bool					engInited	= false;
	uint8_t 				index		= 0;

	uint8_t 			   *amrDeBuf	= NULL;
	uint16_t				amrDeBufLen = 0;
	uint8_t 			   *pcmBuf		= NULL;
	uint16_t				*amrBitlengthPtr	= NULL;
	uint16_t				*amrBytelengthPtr	= NULL;
	uint16_t				pcmBufLen	= 0;
	QueueTonePlayT			queue = {0};
	ecRingT *pcmCache = NULL;
	pcmCache = pcmCache;
	rawDataHanderParam_T dataHandleParam = {0};
	INT32 freeSize = 0;
	uint32_t getStreamCnt = 0;
	HalVoiceCodecConfigReq	codecCfg =
	{
		.encOutBitOffset = AMR_BIT_OFFSET
	};
	HalVoiceDecodeReq decReq =
	{
		.inBitOffset = AMR_BIT_OFFSET,
		.sn 		 = 0
	};

	EC_ASSERT(gAudStreamRingBuf != NULL,0,0,0);
	while ((xEcRingGetOption(gAudStreamRingBuf, E_LRO_DATA_SIZE) == 0) && getStreamCnt < 500)
	{
		getStreamCnt++;
		osDelay(10);		
	}
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrPlayStream_0, P_INFO, "[APP AMR Play] amrPlay: %d, %d, %d", codecCfg.codecType, codecCfg.encBitRate, codecCfg.encOutBitOffset);

	halVEAllocAmrEnFrameBuf(HAL_VE_MEM_APP_CALLER, (void **)(&amrDeBuf), &amrDeBufLen);
	if ((amrDeBuf == NULL) || (amrDeBufLen == 0))
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrPlayStream_invalid_1, P_ERROR, "[APP AMR Play] Failed to get AMR buffer.");
		goto labelEnd;
	}
	amrDeBuf	+= HAL_RTP_HEADER_RSVD_SIZE;
	amrDeBufLen -= HAL_RTP_HEADER_RSVD_SIZE;

	memset(amrDeBuf, 0, amrDeBufLen);
	xEcRingPreread(gAudStreamRingBuf,amrDeBuf,strlen(AMR_WB_HEAD));
	if (memcmp(amrDeBuf, AMR_NB_HEAD, strlen(AMR_NB_HEAD)) == 0)
	{
		codecCfg.codecType = HAL_VC_AMR_NB;
		codecCfg.encBitRate = HAL_AMR_NB_FT_7;
		decReq.codecType = HAL_VC_AMR_NB;
		amrBitlengthPtr = (uint16_t *)gAmrNbBitLength;
		amrBytelengthPtr = (uint16_t *)gAmrNbByteLength;
		halVEAllocAppPcmBuf( HAL_VE_8KHZ_16B_PCM, HAL_VE_MEM_APP_CALLER, (void **)(&pcmBuf), &pcmBufLen);
		if ((pcmBuf == NULL) || (pcmBufLen == 0))
		{
			ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrPlayStream_invalid_2, P_ERROR, "[APP AMR Play] Failed to get PCM buffer.");
			goto labelEnd;
		}
		xEcRingRead(gAudStreamRingBuf,amrDeBuf,strlen(AMR_NB_HEAD));
		memset(amrDeBuf, 0, amrDeBufLen);
	}
	else if (memcmp(amrDeBuf, AMR_WB_HEAD, strlen(AMR_WB_HEAD)) == 0)
	{
		codecCfg.codecType = HAL_VC_AMR_WB;
		codecCfg.encBitRate = HAL_AMR_WB_FT_8;
		decReq.codecType = HAL_VC_AMR_WB;
		amrBitlengthPtr = (uint16_t *)gAmrWbBitLength;
		amrBytelengthPtr = (uint16_t *)gAmrWbByteLength;
		halVEAllocAppPcmBuf( HAL_VE_16KHZ_16B_PCM, HAL_VE_MEM_APP_CALLER, (void **)(&pcmBuf), &pcmBufLen);
		if ((pcmBuf == NULL) || (pcmBufLen == 0))
		{
			ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrPlayStream_invalid_3, P_ERROR, "[APP AMR Play] Failed to get PCM buffer.");
			goto labelEnd;
		}
		xEcRingRead(gAudStreamRingBuf,amrDeBuf,strlen(AMR_WB_HEAD));
		memset(amrDeBuf, 0, amrDeBufLen);
	}
	else
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrPlayStream_invalid_4, P_ERROR, "[APP AMR Play] invalid amr header [%s]",amrDeBuf);
		 goto labelEnd;
	}

	pcmCache = medInitDataCache(AMR_DATA_CACHE_SIZE,AMR_DATA_CACHE_PRE_FILL_SIZE);	

	if(amrEngCommonStart(&codecCfg,AUDIO_AMR_ENG_DECODE_FLAG))
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrPlayStream_invalid_5, P_ERROR, "[APP AMR Play] amrEngCommonStart fail");
		goto labelEnd;
	}
	
	engInited = true;

	if (gAmrTimer == NULL)
	{
		gAmrTimer = osTimerNew(amrTimerFunc, osTimerPeriodic, NULL, NULL);
		if (gAmrTimer == NULL)
		{
			ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrPlayStream_invalid_6, P_ERROR, "[APP AMR Play] Failed to create timer for gAmrTimer");
			goto labelEnd;
		}
	}
	
	dataHandleParam.field.samplerate = (codecCfg.codecType == HAL_VC_AMR_NB ? SAMPLERATE_8K : SAMPLERATE_16K);
	dataHandleParam.field.envType = (toneFlag ? MED_DATA_ENV_TYPE_TONE : MED_DATA_ENV_TYPE_LOCAL);
	dataHandleParam.field.bitWidth = 0;//default 16bit
	medDataHandleStart(&dataHandleParam);
	
	gAudAmrQueue = osMessageQueueNew(100, sizeof(queue), NULL);
	if(gAudAmrQueue == NULL)
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrPlayStream_invalid_7, P_ERROR, "[APP AMR Play] Failed to create gAudAmrQueue");
		goto labelEnd;
	}
	osTimerStart(gAmrTimer, DMA_TRUNK_PERIOD);
	while (1)
	{
		memset(&queue, 0, sizeof(queue));
		if(osMessageQueueGet(gAudAmrQueue, &queue, 0, osWaitForever) == osOK)
		{
			if(queue.endFlag == true)
			{
				ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrPlayStream_1, P_INFO, "[APP AMR Play] queue endflag [%d]",queue.endFlag);
				if(medDataHandleStateGet() >= MED_DATA_HDL_STA_SUSPEND)
					retVal = AV_RET_PLAY_EOF;
				else
					retVal=  AV_RET_PLAY_STOP;
				medDataHandleStop();
				break;			
			}
			freeSize = medGetDataCacheAvlbSize();
			if(freeSize <= PCM_BUFFER_SIZE*2)/*16 bit*/
			{
				continue;
			}
			memset(amrDeBuf, 0, amrDeBufLen);
			if(xEcRingGetOption(gAudStreamRingBuf, E_LRO_DATA_SIZE) == 0)
			{
				continue;

			}
			else
			{

				xEcRingRead(gAudStreamRingBuf,amrDeBuf,1);
			}

			for (index=0; index<AMR_FT_COUNT; index++)
			{
				if (amrDeBuf[0] == gAmrNbHead[index])
				{
					if(amrBytelengthPtr[index] != 0)
					{
						xEcRingRead(gAudStreamRingBuf,amrDeBuf,amrBytelengthPtr[index]);
					}
						
					decReq.amrBitLen   = amrBitlengthPtr[index];
					decReq.pAmrData    = amrDeBuf;
					decReq.pcmBitWidth = PCM_BIT_WIDTH;
					decReq.pcmBufLen   = PCM_BUFFER_SIZE;
					decReq.pPcmData    = pcmBuf;
					halVoiceDecodeReq(&decReq);
					break;
				}
			}

			if (index >= AMR_FT_COUNT)
			{
				ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrPlayStream_2, P_INFO, "[APP AMR Play] Unknown AMR type: 0x%x", amrDeBuf[0]);
				break;
			}
			amrEngWaitCpReply(HAL_VOICE_DECODE_CNF);
		}
	}

labelEnd:
	if (engInited == true)
	{
		amrEngCommonStop(AUDIO_AMR_ENG_DECODE_FLAG);
	}

	if(gAudAmrQueue != NULL)
	{
		osMessageQueueDelete(gAudAmrQueue);
		gAudAmrQueue = NULL;
	}

	if (amrDeBuf != NULL)
	{
		amrDeBuf -= HAL_RTP_HEADER_RSVD_SIZE;
		halVEFreeAmrEnFrameBuf((void **)(&amrDeBuf));
	}

	if (pcmBuf != NULL)
	{
		halVEFreeAppPcmBuf((void **)(&pcmBuf));
	}

	if (gAmrTimer != NULL)
	{
		if (osTimerIsRunning(gAmrTimer))
		{
			osTimerStop(gAmrTimer);
		}
		osTimerDelete(gAmrTimer);
		gAmrTimer = NULL;
	}
#endif
	return retVal;
}


#endif
