/****************************************************************************
*
* Copy right:   2024-, Copyrigths of EigenComm Ltd.
* File name:    audPcm.c
* Description:  EC718 media play source file
* History:      Rev1.0   2024-08-20
*
****************************************************************************/

/*----------------------------------------------------------------------------*
 *					  INCLUDES												  *
 *----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "cmsis_os2.h"
#include "os_common.h"
#include DEBUG_LOG_HEADER_FILE
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
#include "flashex.h"
#endif

#ifdef FEATURE_SUBSYS_MED_PCM_ENABLE
#include "media.h"
#include "audPcm.h"
#include "medDataHandle.h"

#ifdef MBTK_OPENCPU_SUPPORT
#include "ol_log.h"
#endif

#define MED_PCM_FRAME_SIZE					(1920)

#ifdef FEATURE_SUBSYS_AI_XIAOZHI_ENABLE
#define PCM_DATA_CACHE_PRE_FILL_SIZE		(16*1024)	
#define PCM_DATA_CACHE_SIZE 				(PCM_DATA_CACHE_PRE_FILL_SIZE + MED_PCM_FRAME_SIZE*3)
#else
#define PCM_DATA_CACHE_PRE_FILL_SIZE		(16 * 1024)
#define PCM_DATA_CACHE_SIZE 				(PCM_DATA_CACHE_PRE_FILL_SIZE + MED_PCM_FRAME_SIZE*3)
#endif
osTimerId_t 	   pcmPlayTimer = NULL;
osMessageQueueId_t gPcmPlayQueue = NULL;
extern ecRingT *gAudStreamRingBuf;
static void pcmPlayTimerFunc(void *argument)
{
	QueueTonePlayT queue = {0};
	osStatus_t stat = osOK;
	if(gPcmPlayQueue != NULL)
	{
		if(medDataHandleStateGet() >= MED_DATA_HDL_STA_SUSPEND)
		{
			queue.endFlag = true;
		}
		stat = osMessageQueuePut(gPcmPlayQueue, &queue, 0, AUD_CODEC_SIG_TIMEROUT_MAX);
		if(stat != osOK)
		{
			ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, pcmPlayTimerFunc_valid, P_ERROR, "message send fail(%d)",stat);
		}
	}
}

int32_t audPcmStreamPlay(audioParamT *audParam)
{
	int32_t ret = AV_RET_PLAY_ERROR;

	// struct stat buf = {0};
	rawDataHanderParam_T dataHandleParam = {0};
	QueueTonePlayT queue = {0};
	ecRingT *pcmCache = NULL;
	uint32_t cacheFreeSize = 0;
	uint8_t *pcmDataBuff = NULL;
	int32_t readLen = 0;
	int32_t streamBlkSize = 0;

	// uint32_t getStreamCnt = 0;
	
	EC_ASSERT(gAudStreamRingBuf != NULL,0,0,0);
	//while ((xEcRingGetOption(gAudStreamRingBuf, E_LRO_DATA_SIZE) == 0) && getStreamCnt < 1000)
	//{
	//	getStreamCnt++;
	//	osDelay(10);		
	//}
	pcmDataBuff = malloc(MED_PCM_FRAME_SIZE);
	if (!pcmDataBuff)
	{		
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audPcmStreamPlay_invalid_1, P_ERROR, "pcmDataBuff alloc fail ");
		ret = AV_RET_PLAY_ERROR;
		goto EXIT;
	}
	
	pcmCache = medInitDataCache(PCM_DATA_CACHE_SIZE, PCM_DATA_CACHE_PRE_FILL_SIZE);
	gPcmPlayQueue = osMessageQueueNew(100, sizeof(queue), NULL);
	if (gPcmPlayQueue == NULL)
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audPcmStreamPlay_invalid_2, P_ERROR, "PCM QUEUE create fail ");
		ret = AV_RET_PLAY_ERROR;
		goto EXIT;
	}

	pcmPlayTimer = osTimerNew((osTimerFunc_t)pcmPlayTimerFunc,osTimerPeriodic,NULL,NULL);
	
	dataHandleParam.field.samplerate = audParam->rate;
	dataHandleParam.field.envType = (audParam->toneFlag ? MED_DATA_ENV_TYPE_TONE : MED_DATA_ENV_TYPE_LOCAL);
	dataHandleParam.field.bitWidth = audParam->BitWidth;//default 16bit
	medDataHandleStart(&dataHandleParam);
	osTimerStart(pcmPlayTimer, 20);
		uint32_t packet_size = sampleRateConvert(dataHandleParam.field.samplerate,false) * 20 * 16 / 8 / 1000;
	while(1)
	{
		memset(&queue, 0, sizeof(queue));
		if(osMessageQueueGet(gPcmPlayQueue, &queue, 0, osWaitForever) == osOK)
		{
			if(queue.endFlag)
			{
				if(medDataHandleStateGet() >= MED_DATA_HDL_STA_SUSPEND)
					ret = AV_RET_PLAY_EOF;
				else
					ret = AV_RET_PLAY_STOP;
				medDataHandleStop();
				break;
			}
			cacheFreeSize = medGetDataCacheAvlbSize();
			streamBlkSize = xEcRingGetOption(gAudStreamRingBuf, E_LRO_DATA_SIZE);
			if(cacheFreeSize <= MED_PCM_FRAME_SIZE*2)
			{
				continue;
			}
			streamBlkSize = xEcRingGetOption(gAudStreamRingBuf, E_LRO_DATA_SIZE);
		    if (streamBlkSize <= packet_size)
			{
				ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audPcmStreamPlay_invalid_3, P_DEBUG, "no stream data");
				if(cacheFreeSize >= PCM_DATA_CACHE_PRE_FILL_SIZE)
				{
					memset(pcmDataBuff,0x00,MED_PCM_FRAME_SIZE);
					xEcRingWriteEx(pcmCache, pcmDataBuff, packet_size);
				}
				continue;
			}
			readLen = (cacheFreeSize > streamBlkSize ? streamBlkSize : cacheFreeSize);
			readLen = (readLen > MED_PCM_FRAME_SIZE ? MED_PCM_FRAME_SIZE : readLen);
			memset(pcmDataBuff,0x00,MED_PCM_FRAME_SIZE);
			readLen = xEcRingRead(gAudStreamRingBuf,pcmDataBuff,readLen);
			xEcRingWriteEx(pcmCache, pcmDataBuff, readLen);
		}
	}

EXIT:
	
	if(pcmDataBuff)
	{
		free(pcmDataBuff);
		pcmDataBuff = NULL;
	}
	if(pcmPlayTimer != NULL)
	{
		if(osTimerIsRunning(pcmPlayTimer))
			osTimerStop(pcmPlayTimer);
		osTimerDelete(pcmPlayTimer);
		pcmPlayTimer = NULL;
	}
	if(gPcmPlayQueue != NULL)
	{
		osMessageQueueDelete(gPcmPlayQueue);
		gPcmPlayQueue = NULL;
	}
	return ret;
}


int32_t audPcmFilePlay(uint8_t *path, audioParamT *audParam)
{
	int32_t ret = AV_RET_PLAY_ERROR;
	EC_ASSERT(path != NULL,path,0,0);
	struct stat buf = {0};
	rawDataHanderParam_T dataHandleParam = {0};
	QueueTonePlayT queue = {0};
	ecRingT *pcmCache = NULL;
	uint32_t cacheFreeSize = 0;
	uint8_t *pcmDataBuff = NULL;
	int32_t readLen = 0;
	FILE  *file = NULL;
	uint32_t fileSize = 0;
	file = file_fopen((const char *)path, "r");
	if (!file)
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audPcmFilePlay_invalid_1, P_ERROR, "file open fail ");
		return ret;
	}
	fileSize = file_fstat((int)file, &buf);
	fileSize = buf.st_size;

	pcmDataBuff = malloc(MED_PCM_FRAME_SIZE);
	if (!pcmDataBuff)
	{		
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audPcmFilePlay_invalid_2, P_ERROR, "pcmDataBuff alloc fail ");
		ret = AV_RET_PLAY_ERROR;
		goto EXIT;
	}
	
	
	gPcmPlayQueue = osMessageQueueNew(100, sizeof(queue), NULL);
	if (gPcmPlayQueue == NULL)
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audPcmFilePlay_invalid_3, P_ERROR, "PCM QUEUE create fail ");
		ret = AV_RET_PLAY_ERROR;
		goto EXIT;
	}

	if(medDataHandleStateGet() != MED_DATA_HDL_STA_START)
	{
		pcmCache = medInitDataCache(PCM_DATA_CACHE_SIZE, PCM_DATA_CACHE_PRE_FILL_SIZE);
		
		dataHandleParam.field.samplerate = audParam->rate;
		dataHandleParam.field.envType = (audParam->toneFlag ? MED_DATA_ENV_TYPE_TONE : MED_DATA_ENV_TYPE_LOCAL);
		dataHandleParam.field.bitWidth = audParam->BitWidth;//default 16bit
		medDataHandleStart(&dataHandleParam);
	}
	else
	{
		pcmCache = medGetDataCache();
	}
	
	pcmPlayTimer = osTimerNew((osTimerFunc_t)pcmPlayTimerFunc,osTimerPeriodic,NULL,NULL);
	
	
	osTimerStart(pcmPlayTimer, 20);
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audPcmFilePlay_1, P_DEBUG, "PCM file play start");
	while(1)
	{
		memset(&queue, 0, sizeof(queue));
		if(osMessageQueueGet(gPcmPlayQueue, &queue, 0, osWaitForever) == osOK)
		{
			if(queue.endFlag)
			{
				if(medDataHandleStateGet() >= MED_DATA_HDL_STA_SUSPEND)
					ret = AV_RET_PLAY_EOF;
				else
					ret = AV_RET_PLAY_STOP;
				medDataHandleStop();
				break;
			}
			cacheFreeSize = medGetDataCacheAvlbSize();
			if(cacheFreeSize <= MED_PCM_FRAME_SIZE*2)/*16 bit*/
			{
				continue;
			}

			
            if (file_ftell(file) == fileSize)
            {
				if(audParam->toneFlag)
				{
					break;
				}
				else if(EC_audioPlayChkLoop())
				{
					file_fseek(file,0,SEEK_SET);
				}
				else
				{
					continue;
				}
            }			
			readLen = file_fread(pcmDataBuff, 1, MED_PCM_FRAME_SIZE, file);
			xEcRingWriteEx(pcmCache, pcmDataBuff, readLen);
		}
	}

EXIT:
	if(file)
	{
		file_fclose(file);
		file = NULL;
	}
	if(pcmDataBuff)
	{

		free(pcmDataBuff);
		pcmDataBuff = NULL;
	}
	if(pcmPlayTimer != NULL)
	{
		if(osTimerIsRunning(pcmPlayTimer))
			osTimerStop(pcmPlayTimer);
		osTimerDelete(pcmPlayTimer);
		pcmPlayTimer = NULL;
	}
	if(gPcmPlayQueue != NULL)
	{
		osMessageQueueDelete(gPcmPlayQueue);
		gPcmPlayQueue = NULL;
	}
	return ret;
}



int32_t audPcmStringPlay(uint8_t *src,uint32_t size,audioParamT * audParam)
{
	EC_ASSERT((src != NULL && size > 0),src,size,0);
	int32_t ret = AV_RET_PLAY_ERROR;
	rawDataHanderParam_T dataHandleParam = {0};
	QueueTonePlayT queue = {0};
	ecRingT *pcmCache = NULL;
	uint32_t cacheFreeSize = 0;
	uint32_t pcmPickSize = 0;
	uint32_t pcmFillCnt = 0;
	uint32_t stringRemain = size;
	uint8_t *tmpPtr = src;

	gPcmPlayQueue = osMessageQueueNew(100, sizeof(queue), NULL);
    if (gPcmPlayQueue == NULL)
    {
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audPcmStringPlay_invalid_1, P_ERROR, "file open fail ");
		return ret;
    }

	if(medDataHandleStateGet() != MED_DATA_HDL_STA_START)
	{
		pcmCache = medInitDataCache(PCM_DATA_CACHE_SIZE, PCM_DATA_CACHE_PRE_FILL_SIZE);
		
		dataHandleParam.field.samplerate = audParam->rate;
		dataHandleParam.field.envType = (audParam->toneFlag ? MED_DATA_ENV_TYPE_TONE : MED_DATA_ENV_TYPE_LOCAL);
		dataHandleParam.field.bitWidth = audParam->BitWidth;//default 16bit
		medDataHandleStart(&dataHandleParam);
	}
	else
	{
		pcmCache = medGetDataCache();
	}
	pcmPlayTimer = osTimerNew((osTimerFunc_t)pcmPlayTimerFunc,osTimerPeriodic,NULL,NULL);
	osTimerStart(pcmPlayTimer, 20);

	while(1)
	{
		memset(&queue, 0, sizeof(queue));
		if(osMessageQueueGet(gPcmPlayQueue, &queue, 0, osWaitForever) == osOK)
		{
			if(queue.endFlag)
			{
				if(medDataHandleStateGet() >= MED_DATA_HDL_STA_SUSPEND)
					ret = AV_RET_PLAY_EOF;
				else
					ret = AV_RET_PLAY_STOP;
				medDataHandleStop();
				break;
			}
			cacheFreeSize = medGetDataCacheAvlbSize();
			if(cacheFreeSize <= MED_PCM_FRAME_SIZE*2)/*16 bit*/
			{
				continue;
			}

			if(stringRemain == 0)
			{
				if(audParam->toneFlag)
				{
					break;
				}
				else if(EC_audioPlayChkLoop())
				{
					stringRemain = size;
					pcmFillCnt = 0;;
				}
				else
				{
					continue;
				}
			}
			pcmPickSize = (stringRemain > MED_PCM_FRAME_SIZE ? MED_PCM_FRAME_SIZE : stringRemain);
			pcmPickSize = xEcRingWriteEx(pcmCache, tmpPtr + pcmFillCnt, pcmPickSize);
			pcmFillCnt = pcmFillCnt + pcmPickSize;
			stringRemain = stringRemain - pcmPickSize;
		}
	}
	if(pcmPlayTimer != NULL)
	{
		if(osTimerIsRunning(pcmPlayTimer))
			osTimerStop(pcmPlayTimer);
		osTimerDelete(pcmPlayTimer);
		pcmPlayTimer = NULL;
	}
	if(gPcmPlayQueue != NULL)
	{
		osMessageQueueDelete(gPcmPlayQueue);
		gPcmPlayQueue = NULL;
	}
	return ret;
}

#ifdef MBTK_OPENCPU_SUPPORT
volatile uint8_t gpcmState = 0;
osMessageQueueId_t gPcmDataQueue = NULL;
int ol_audPcmPlayContinue(uint8_t *src,uint32_t size, BOOL continue_p)
{
    QueueTonePlayT queue = {0}; 
    osStatus_t stat = osOK;

    OL_LOG_INFO("PCM play continue gpcmState=%d, continue_p=%d", gpcmState, continue_p);
    if(NULL == src || 0 == size)
    {
        return -1;
    }

    if(1 == gpcmState)
    {
        queue.continue_play = continue_p;
        queue.length = size;
        queue.play_buff = src;
        if(gPcmDataQueue != NULL)
        {
            stat = osMessageQueuePut(gPcmDataQueue,&queue, 0, 3);
            if(stat != osOK)
            {
                OL_LOG_INFO("PCM play continue message send fail(%d)", stat);
                return 0;
            }
        }
        OL_LOG_INFO("add gPcmDataQueue count = %d",osMessageQueueGetCount(gPcmDataQueue));
        return 1;
    }

    return 0;
}

int32_t ol_audPcmStringPlay(uint8_t *src,uint32_t size,audioParamT * audParam, BOOL continue_p)
{
    EC_ASSERT((src != NULL && size > 0),src,size,0);
    int32_t ret = AV_RET_PLAY_ERROR;
    rawDataHanderParam_T dataHandleParam = {0};
    QueueTonePlayT queue = {0};
    ecRingT *pcmCache = NULL;
    uint32_t cacheFreeSize = 0;
    uint32_t pcmPickSize = 0;
    uint32_t pcmFillCnt = 0;
    uint32_t stringRemain = size;
    uint8_t *tmpPtr = src;
    BOOL continue_play = continue_p;

    if(1 == continue_play)
    {
        gpcmState = 1;
        gPcmDataQueue = osMessageQueueNew(20, sizeof(queue), NULL);
        if (gPcmDataQueue == NULL)
        {
            OL_LOG_INFO("gPcmDataQueue new fail");
            return ret;
        }
    }
	pcmCache = medInitDataCache(PCM_DATA_CACHE_SIZE, PCM_DATA_CACHE_PRE_FILL_SIZE);
	gPcmPlayQueue = osMessageQueueNew(100, sizeof(queue), NULL);
    if (gPcmPlayQueue == NULL)
    {
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audPcmStringPlay_invalid_1, P_ERROR, "file open fail ");
		return ret;
    }
	
	pcmPlayTimer = osTimerNew((osTimerFunc_t)pcmPlayTimerFunc,osTimerPeriodic,NULL,NULL);
	
	dataHandleParam.field.samplerate = audParam->rate;
	dataHandleParam.field.envType = (audParam->toneFlag ? MED_DATA_ENV_TYPE_TONE : MED_DATA_ENV_TYPE_LOCAL);
	dataHandleParam.field.bitWidth = audParam->BitWidth;
	medDataHandleStart(&dataHandleParam);
	osTimerStart(pcmPlayTimer, 20);

    while(1)
    {
        memset(&queue, 0, sizeof(queue));
        if(osMessageQueueGet(gPcmPlayQueue, &queue, 0, osWaitForever) == osOK)
        {
            if(queue.endFlag)
            {
                if(1 == continue_play && medDataHandleStateGet() == MED_DATA_HDL_STA_SUSPEND)
                    continue;

                if(medDataHandleStateGet() >= MED_DATA_HDL_STA_SUSPEND)
                    ret = AV_RET_PLAY_EOF;
                else
                    ret = AV_RET_PLAY_STOP;
                medDataHandleStop();
                break;
            }
            cacheFreeSize = medGetDataCacheAvlbSize();
            if(cacheFreeSize <= MED_PCM_FRAME_SIZE*2)/*16 bit*/
            {
                continue;
            }

            if(stringRemain <= 0)
            {
                if(1 == gpcmState && 1 == continue_play)
                {
                    if(osMessageQueueGet(gPcmDataQueue, &queue, 0, osWaitForever) == osOK)
                    {
                        pcmFillCnt = 0;
                        continue_play = queue.continue_play;
                        stringRemain = queue.length;
                        tmpPtr = queue.play_buff;
                        OL_LOG_INFO("PCM play update datalen=%d continue=%d",stringRemain, continue_play);
                    }
                }

                if(audParam->toneFlag)
                {
                    stringRemain = size;
                    pcmFillCnt = 0;
                    break;
                }
                else
                {
                    continue;
                }
            }
            pcmPickSize = (stringRemain > MED_PCM_FRAME_SIZE ? MED_PCM_FRAME_SIZE : stringRemain);
            pcmPickSize = xEcRingWriteEx(pcmCache, tmpPtr + pcmFillCnt, pcmPickSize);
            pcmFillCnt = pcmFillCnt + pcmPickSize;
            stringRemain = stringRemain - pcmPickSize;
            OL_LOG_INFO("PCM play write size=%d Remain=%d",pcmPickSize, stringRemain);
        }
    }

    gpcmState = 0;
    if(pcmPlayTimer != NULL)
    {
        if(osTimerIsRunning(pcmPlayTimer))
            osTimerStop(pcmPlayTimer);
        osTimerDelete(pcmPlayTimer);
        pcmPlayTimer = NULL;
    }
    if(gPcmPlayQueue != NULL)
    {
        osMessageQueueDelete(gPcmPlayQueue);
        gPcmPlayQueue = NULL;
    }
    if(gPcmDataQueue != NULL)
    {
        osMessageQueueDelete(gPcmDataQueue);
        gPcmDataQueue = NULL;
    }
    return ret;
}
#endif //MBTK_OPENCPU_SUPPORT

int32_t audPcmPlayStop(void)
{
	QueueTonePlayT queue = {0};
	if(gPcmPlayQueue != NULL)
	{
		queue.endFlag = true;
		osMessageQueuePutToFront(gPcmPlayQueue, &queue, 0, osWaitForever);
	}
    return 0;
}


#endif/*FEATURE_SUBSYS_MED_PCM_ENABLE*/
