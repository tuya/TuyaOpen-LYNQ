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

#ifdef FEATURE_SUBSYS_MED_WAV_ENABLE
#include "media.h"
#include "audWav.h"
#include "medDataHandle.h"

#define MED_WAV_FRAME_SIZE					(1920)

#define WAV_DATA_CACHE_PRE_FILL_SIZE		(16*1024)	
#define WAV_DATA_CACHE_SIZE 				(WAV_DATA_CACHE_PRE_FILL_SIZE + MED_WAV_FRAME_SIZE*3)
osTimerId_t 	   wavPlayTimer = NULL;
osMessageQueueId_t gWavPlayQueue = NULL;

static void wavPlayTimerFunc(void *argument)
{
	QueueTonePlayT queue = {0};
	osStatus_t stat = osOK;
	if(gWavPlayQueue != NULL)
	{
		if(medDataHandleStateGet() >= MED_DATA_HDL_STA_SUSPEND)
		{
			queue.endFlag = true;
		}
		stat = osMessageQueuePut(gWavPlayQueue, &queue, 0, AUD_CODEC_SIG_TIMEROUT_MAX);
		if(stat != osOK)
		{
			ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, wavPlayTimerFunc_valid, P_ERROR, "message send fail(%d)",stat);
		}
	}
}


typedef enum _medWavChunkId_E{
	MED_WAV_CHUNK_ID_IDLE = 0,
	MED_WAV_CHUNK_ID_RIFF,
	MED_WAV_CHUNK_ID_FMT,
	MED_WAV_CHUNK_ID_LIST,
	MED_WAV_CHUNK_ID_DATA,
	MED_WAV_CHUNK_ID_WAV,
	MED_WAV_CHUNK_ID_AVI,
	
	MED_WAV_CHUNK_ID_MAX = 0xff,
}medWavChunkId_E;
#define RIFF_CHUNK_ID 			0x46464952 // 'F' 'F' 'I' 'R'
#define FMT_SUB_CHUNK_ID  		0x20746D66 // 'f' 'm' 't' ' '
#define LIST_SUB_CHUNK_ID  		0x5453494C // 'T' 'S' 'I' 'L'
#define DATA_SUB_CHUNK_ID  		0x61746164 // 'a' 't' 'a' 'd'
#define WAV_SUB_CHUNK_ID  		0x20564157 // ' ' 'V' 'A' 'W'
#define AVI_SUB_CHUNK_ID  		0x20495641 // ' ' 'I' 'V' 'A'

medWavChunkT * createChunkBlock(uint32_t size)
{
	medWavChunkT *chunkBlock = OsaAllocZeroMemory(sizeof(medWavChunkT) + size *sizeof(int8_t));
	return chunkBlock;
}

void destroyChunkBlock(medWavChunkT * chunkBlock)
{
	OsaFreeMemory(&chunkBlock);
}

medWavChunkId_E getChunkId(medWavChunkT *src)
{
	int8_t chunkId = MED_WAV_CHUNK_ID_IDLE;
	EC_ASSERT(src,src,0,0);
	medWavChunkT *chunkBlock = src;
	switch(chunkBlock->chunkId)
	{
		case RIFF_CHUNK_ID:
			chunkId = MED_WAV_CHUNK_ID_RIFF;
			break;
		case FMT_SUB_CHUNK_ID:
			chunkId = MED_WAV_CHUNK_ID_FMT;
			break;
		case LIST_SUB_CHUNK_ID:
			chunkId = MED_WAV_CHUNK_ID_LIST;
			break;
		case DATA_SUB_CHUNK_ID:
			chunkId = MED_WAV_CHUNK_ID_DATA;
			break;
		case WAV_SUB_CHUNK_ID:
			chunkId = MED_WAV_CHUNK_ID_WAV;
			break;
		case AVI_SUB_CHUNK_ID:
			chunkId = MED_WAV_CHUNK_ID_AVI;
		default:
		{
			ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, getChunkId_invalid, P_ERROR, "get chunkId fail");
			chunkId = MED_WAV_CHUNK_ID_MAX;
		}
			break;
	}
	return  chunkId;
}

uint32_t getChunkSize(medWavChunkT *src)
{
	uint32_t chunkSize = 0;
	medWavChunkT *chunkBlock = src;
	if(getChunkId(chunkBlock) != MED_WAV_CHUNK_ID_MAX)
	{
		chunkSize = chunkBlock->dataSize;
	}
	return chunkSize;
}

int32_t audWavFilePlay(uint8_t *path, audioParamT *audParam)
{
	int32_t ret = AV_RET_PLAY_ERROR;
	EC_ASSERT(path != NULL,path,0,0);
	struct stat buf = {0};
	rawDataHanderParam_T dataHandleParam = {0};
	QueueTonePlayT queue = {0};
	ecRingT *wavCache = NULL;
	uint32_t cacheFreeSize = 0;
	uint8_t *wavDataBuff = NULL;
	medWavChunkT *wavChunk = NULL;
	medWavFmt wavFmt = {0};
	uint8_t subChunkId = MED_WAV_CHUNK_ID_IDLE;
	uint32_t chunkSize = 0;
	uint32_t wavHeaderSize = 0;
	uint32_t riffChunkSize = 0;
	int32_t readLen = 0;
	FILE  *file = NULL;
	uint32_t fileSize = 0;
	file = file_fopen((const char *)path, "r");
	if (!file)
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audWavFilePlay_invalid_1, P_ERROR, "file open fail ");
		return ret;
	}
	//step 1:check RIFF
	wavChunk = createChunkBlock(0);
	file_fread((void *)wavChunk, 1, sizeof(medWavChunkT), file);
	if(getChunkId(wavChunk) != MED_WAV_CHUNK_ID_RIFF)
	{
		ret = AV_RET_PLAY_ERROR;
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audWavFilePlay_invalid_2, P_ERROR, "RIFF chunk check fail");
		goto EXIT;
	}
	riffChunkSize = getChunkSize(wavChunk);
	
	file_fseek(file,4,SEEK_CUR);//seek "WAVE"

	//step 2:get fmt
	file_fread((void *)wavChunk, 1, sizeof(medWavChunkT), file);
	subChunkId = getChunkId(wavChunk);
	if(subChunkId != MED_WAV_CHUNK_ID_FMT)
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audWavFilePlay_invalid_3, P_ERROR, "fmt subchunk check fail");
		ret = AV_RET_PLAY_ERROR;
		goto EXIT;
	}
	else
	{
		chunkSize = getChunkSize(wavChunk);
		EC_ASSERT(chunkSize == sizeof(medWavFmt),chunkSize,sizeof(medWavFmt),0);
		file_fread((void *)&wavFmt, 1, chunkSize, file);
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audWavFilePlay_1, P_DEBUG, "af[%d],CH[%d],sr[%d],br[%d],ba[%d],bps[%d]",wavFmt.audioFmt,wavFmt.numChannels,wavFmt.samplerate,wavFmt.byteRate,wavFmt.blockAlign,wavFmt.bitsPerSample);
	}

	//step 3:get data
	chunkSize = 0;
	do{	
		file_fseek(file,chunkSize,SEEK_CUR);
		file_fread((void *)wavChunk, 1, sizeof(medWavChunkT), file);		
		subChunkId = getChunkId(wavChunk);
		chunkSize = getChunkSize(wavChunk);
		if((subChunkId == MED_WAV_CHUNK_ID_MAX) || (chunkSize == 0))
		{
			break;
		}		
	}while(subChunkId != MED_WAV_CHUNK_ID_DATA);
	if(subChunkId != MED_WAV_CHUNK_ID_DATA)
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audWavFilePlay_invalid_4, P_ERROR, "data subchunk check fail");
		ret = AV_RET_PLAY_ERROR;
		goto EXIT;
	}
	else
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audWavFilePlay_2, P_DEBUG, "data Size [0x%x]",chunkSize);
		wavHeaderSize = riffChunkSize - chunkSize + 8; /*"RIFF" and next 4 byte*/
	}	
	
	file_fstat((int)file, &buf);
	fileSize = buf.st_size;

	wavDataBuff = OsaAllocZeroMemory(MED_WAV_FRAME_SIZE);
	if (!wavDataBuff)
	{		
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audWavFilePlay_invalid_5, P_ERROR, "wavDataBuff alloc fail ");
		ret = AV_RET_PLAY_ERROR;
		goto EXIT;
	}
	
	gWavPlayQueue = osMessageQueueNew(100, sizeof(queue), NULL);
	if (gWavPlayQueue == NULL)
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audWavFilePlay_invalid_6, P_ERROR, "Wav QUEUE create fail ");
		ret = AV_RET_PLAY_ERROR;
		goto EXIT;
	}

	if(medDataHandleStateGet() != MED_DATA_HDL_STA_START)
	{
		wavCache = medInitDataCache(WAV_DATA_CACHE_SIZE, WAV_DATA_CACHE_PRE_FILL_SIZE);
		
		dataHandleParam.field.samplerate = sampleRateConvert(wavFmt.samplerate,true);
		dataHandleParam.field.envType = (audParam->toneFlag ? MED_DATA_ENV_TYPE_TONE : MED_DATA_ENV_TYPE_LOCAL);
		dataHandleParam.field.bitWidth = (wavFmt.bitsPerSample == 0x10 ? 0 : 1);//default 16bit
		medDataHandleStart(&dataHandleParam);
	}
	else
	{
		wavCache = medGetDataCache();
	}	
	wavPlayTimer = osTimerNew((osTimerFunc_t)wavPlayTimerFunc,osTimerPeriodic,NULL,NULL);
	osTimerStart(wavPlayTimer, 20);

	while(1)
	{
		memset(&queue, 0, sizeof(queue));
		if(osMessageQueueGet(gWavPlayQueue, &queue, 0, osWaitForever) == osOK)
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
			if(cacheFreeSize <= MED_WAV_FRAME_SIZE*2)/*16 bit*/
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
					file_fseek(file,wavHeaderSize,SEEK_SET);
				}
				else
				{
					continue;
				}
			}			
			readLen = file_fread(wavDataBuff, 1, MED_WAV_FRAME_SIZE, file);
			xEcRingWriteEx(wavCache, wavDataBuff, readLen);
		}
	}

EXIT:
	if(wavChunk)
	{
		destroyChunkBlock(wavChunk);
	}
	if(file)
	{
		file_fclose(file);
		file = NULL;
	}
	if(wavDataBuff)
	{
		OsaFreeMemory(&wavDataBuff);
		wavDataBuff = NULL;
	}

	if(wavPlayTimer != NULL)
	{
		if(osTimerIsRunning(wavPlayTimer))
			osTimerStop(wavPlayTimer);
		osTimerDelete(wavPlayTimer);
		wavPlayTimer = NULL;
	}
	if(gWavPlayQueue != NULL)
	{
		osMessageQueueDelete(gWavPlayQueue);
		gWavPlayQueue = NULL;
	}

	return ret;
}

int32_t audWavStringPlay(const uint8_t *src,uint32_t size,audioParamT * audParam)
{
	int32_t ret = AV_RET_PLAY_ERROR;
	EC_ASSERT(src != NULL,src,0,0);

	rawDataHanderParam_T dataHandleParam = {0};
	QueueTonePlayT queue = {0};
	ecRingT *wavCache = NULL;
	uint32_t cacheFreeSize = 0;
	uint8_t *wavDataBuff = NULL;
	medWavChunkT *wavChunk = NULL;
	medWavFmt wavFmt = {0};
	uint8_t subChunkId = MED_WAV_CHUNK_ID_IDLE;
	uint32_t chunkSize = 0;
	// uint32_t wavHeaderSize = 0;
	uint32_t riffChunkSize = 0;
	// int32_t readLen = 0;

	uint32_t wavPickSize = 0;
	uint32_t wavFillCnt = 0;
	uint32_t stringRemain = 0;
	uint8_t *mvPtr = (uint8_t *)src;
	//step 1:check RIFF
	wavChunk = createChunkBlock(0);

	memcpy(wavChunk,mvPtr,sizeof(medWavChunkT));
	mvPtr = mvPtr + sizeof(medWavChunkT);	
	if(getChunkId(wavChunk) != MED_WAV_CHUNK_ID_RIFF)
	{
		ret = AV_RET_PLAY_ERROR;
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audWavStringPlay_invalid_1, P_ERROR, "RIFF chunk check fail");
		goto EXIT;
	}
	riffChunkSize = getChunkSize(wavChunk);

	if((riffChunkSize + 8) != size)
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audWavStringPlay_invalid_2, P_ERROR, "size[%d]_riffSize[riffChunkSize]",size,riffChunkSize);
	}
	
	mvPtr = mvPtr + 4; // seek "WAVE"
	//step 2:get fmt
	memcpy(wavChunk,mvPtr,sizeof(medWavChunkT));
	mvPtr = mvPtr + sizeof(medWavChunkT);
	
	subChunkId = getChunkId(wavChunk);
	if(subChunkId != MED_WAV_CHUNK_ID_FMT)
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audWavStringPlay_invalid_3, P_ERROR, "fmt subchunk check fail");
		ret = AV_RET_PLAY_ERROR;
		goto EXIT;
	}

	chunkSize = getChunkSize(wavChunk);
	EC_ASSERT(chunkSize == sizeof(medWavFmt),chunkSize,sizeof(medWavFmt),0);

	memcpy(&wavFmt,mvPtr,chunkSize);
	mvPtr = mvPtr + chunkSize;
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audWavStringPlay_1, P_DEBUG, "af[%d],CH[%d],sr[%d],br[%d],ba[%d],bps[%d]",wavFmt.audioFmt,wavFmt.numChannels,wavFmt.samplerate,wavFmt.byteRate,wavFmt.blockAlign,wavFmt.bitsPerSample);

	//step 3:get data
	chunkSize = 0;
	do{ 
		mvPtr = mvPtr + chunkSize;
		memcpy(wavChunk,mvPtr,sizeof(medWavChunkT));
		mvPtr = mvPtr + sizeof(medWavChunkT);
		subChunkId = getChunkId(wavChunk);
		chunkSize = getChunkSize(wavChunk);
		if((subChunkId == MED_WAV_CHUNK_ID_MAX) || (chunkSize == 0))
		{
			break;
		}		
	}while(subChunkId != MED_WAV_CHUNK_ID_DATA);
	if(subChunkId != MED_WAV_CHUNK_ID_DATA)
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audWavStringPlay_invalid_4, P_ERROR, "data subchunk check fail");
		ret = AV_RET_PLAY_ERROR;
		goto EXIT;
	}

	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audWavStringPlay_2, P_DEBUG, "data Size [0x%x] ",chunkSize);
	// wavHeaderSize = riffChunkSize - chunkSize + 8; /*"RIFF" and next 4 byte*/
	mvPtr = mvPtr + sizeof(medWavChunkT);
	stringRemain = chunkSize;

	wavDataBuff = OsaAllocZeroMemory(MED_WAV_FRAME_SIZE);
	if (!wavDataBuff)
	{		
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audWavStringPlay_invalid_5, P_ERROR, "wavDataBuff alloc fail ");
		ret = AV_RET_PLAY_ERROR;
		goto EXIT;
	}
	
	gWavPlayQueue = osMessageQueueNew(100, sizeof(queue), NULL);
	if (gWavPlayQueue == NULL)
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audWavStringPlay_invalid_6, P_ERROR, "Wav QUEUE create fail ");
		ret = AV_RET_PLAY_ERROR;
		goto EXIT;
	}

	if(medDataHandleStateGet() != MED_DATA_HDL_STA_START)
	{
		wavCache = medInitDataCache(WAV_DATA_CACHE_SIZE, WAV_DATA_CACHE_PRE_FILL_SIZE);
		dataHandleParam.field.samplerate = sampleRateConvert(wavFmt.samplerate,true);
		dataHandleParam.field.envType = (audParam->toneFlag ? MED_DATA_ENV_TYPE_TONE : MED_DATA_ENV_TYPE_LOCAL);
		dataHandleParam.field.bitWidth = (wavFmt.bitsPerSample == 0x10 ? 0 : 1);//default 16bit
		medDataHandleStart(&dataHandleParam);
	}
	else
	{
		wavCache = medGetDataCache();
	}
	wavPlayTimer = osTimerNew((osTimerFunc_t)wavPlayTimerFunc,osTimerPeriodic,NULL,NULL);
	osTimerStart(wavPlayTimer, 20);

	while(1)
	{
		memset(&queue, 0, sizeof(queue));
		if(osMessageQueueGet(gWavPlayQueue, &queue, 0, osWaitForever) == osOK)
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
			if(cacheFreeSize <= MED_WAV_FRAME_SIZE*2)/*16 bit*/
			{
				continue;
			}

			
			if (stringRemain == 0)
			{
				if(audParam->toneFlag)
				{
					break;
				}
				else if(EC_audioPlayChkLoop())
				{
					stringRemain = chunkSize;
					wavFillCnt = 0;
				}
				else
				{
					continue;
				}
			}			
			wavPickSize = (stringRemain > MED_WAV_FRAME_SIZE ? MED_WAV_FRAME_SIZE : stringRemain);
			wavPickSize = xEcRingWriteEx(wavCache, mvPtr + wavFillCnt, wavPickSize);
			wavFillCnt = wavFillCnt + wavPickSize;
			stringRemain = stringRemain - wavPickSize;
		}
	}

EXIT:
	if(wavChunk)
	{
		destroyChunkBlock(wavChunk);
	}
	
	if(wavDataBuff)
	{
		OsaFreeMemory(&wavDataBuff);
		wavDataBuff = NULL;
	}

	if(wavPlayTimer != NULL)
	{
		if(osTimerIsRunning(wavPlayTimer))
			osTimerStop(wavPlayTimer);
		osTimerDelete(wavPlayTimer);
		wavPlayTimer = NULL;
	}
	if(gWavPlayQueue != NULL)
	{
		osMessageQueueDelete(gWavPlayQueue);
		gWavPlayQueue = NULL;
	}

	return ret;
}

uint32_t audWavPlayStop(void)
{
	QueueTonePlayT queue = {0};
	if(gWavPlayQueue != NULL)
	{
		queue.endFlag = true;
		osMessageQueuePutToFront(gWavPlayQueue, &queue, 0, osWaitForever);
	}

	return 0;
}
#endif/*FEATURE_SUBSYS_MED_WAV_ENABLE*/
