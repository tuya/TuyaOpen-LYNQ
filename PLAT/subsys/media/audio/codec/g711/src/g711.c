#include DEBUG_LOG_HEADER_FILE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#ifdef FEATURE_SUBSYS_WAV_ENABLE
#include "wav.h"
#endif
#include "g711.h"

#include "media.h"
#include "audPcm.h"
#include "medDataHandle.h"

#define MAX                 32635
#define BUFFER_SIZE_G711    512
#define BUFFER_SIZE_PCM     (BUFFER_SIZE_G711 * 2)

#define MED_G711_TO_PCM_FRAME_SIZE					(1920)

#define G711_PCM_DATA_CACHE_PRE_FILL_SIZE		MED_G711_TO_PCM_FRAME_SIZE*16	
#define  G711_PCM_DATA_CACHE_SIZE 				( G711_PCM_DATA_CACHE_PRE_FILL_SIZE*2)
osTimerId_t 	   g711PlayTimer = NULL;
osMessageQueueId_t g711PlayQueue = NULL;
extern ecRingT *gAudStreamRingBuf;


typedef struct
{
    char     riffId[4];
    uint32_t riffSize;
    char     riffType[4];
    char     fmtId[4];
    uint32_t fmtSize;
    uint16_t fmtCompressionCode;
    uint16_t fmtChannels;
    uint32_t fmtSampleRate;
    uint32_t fmtBytesPerSec;
    uint16_t fmtBlockAlign;
    uint16_t fmtBitPerSample;
    char     listOrDataId[4];
    uint32_t listOrDataLength;
} WavHeadT;


static unsigned char encode(short pcm)
{
    int sign = (pcm & 0x8000) >> 8;
    if (sign != 0)
        pcm = -pcm;
    if (pcm > MAX) pcm = MAX;
    int exponent = 7;
    int expMask;
    for (expMask = 0x4000; (pcm & expMask) == 0
        && exponent>0; exponent--, expMask >>= 1) { }
    int mantissa = (pcm >> ((exponent == 0) ? 4 : (exponent + 3))) & 0x0f;
    unsigned char alaw = (unsigned char)(sign | exponent << 4 | mantissa);
    return (unsigned char)(alaw^0xD5);
}

static short decode(unsigned char alaw)
{
    alaw ^= 0xD5;
    int sign = alaw & 0x80;
    int exponent = (alaw & 0x70) >> 4;
    int data = alaw & 0x0f;
    data <<= 4;
    data += 8;
    if (exponent != 0)
        data += 0x100;
    if (exponent > 1)
        data <<= (exponent - 1);
    return (short)(sign == 0 ? data : -data);
}

int g711_encode(unsigned char* pCodecBits, const char* pBuffer, int nBufferSize)
{
    short* buffer = (short*)pBuffer;
    int i;
    for(i=0; i<nBufferSize/2; i++)
    {
        pCodecBits[i] = encode(buffer[i]);
    }
    return nBufferSize/2;
}

int g711_decode(char* pRawData, const unsigned char* pBuffer, int nBufferSize)
{
    short *out_data = (short*)pRawData;
    int i;
    for(i=0; i<nBufferSize; i++)
    {
        out_data[i] = decode(pBuffer[i]);
    }
    return nBufferSize*2;
}

int32_t g711Encode(char *pathIn, char *pathOut)
{
    int32_t       retVal                       = -1;
    FILE         *fileSrc                      = NULL;
    FILE         *fileDest                     = NULL;
    struct stat   buf                          = {0};
    int32_t       size                         = 0;
    int32_t       count                        = 0;
    uint32_t      length                       = 0;
    char          bufferSrc[BUFFER_SIZE_PCM]   = {0};
    char          bufferDest[BUFFER_SIZE_G711] = {0};

    if ((pathIn == NULL) || (pathOut == NULL))
    {
        SYSLOG_DEBUG("Param error.\r\n");
        goto labelEnd;
    }

    fileSrc = file_fopen(pathIn, "r");
    if (fileSrc == NULL)
    {
        SYSLOG_DEBUG("Failed to open the file \"%s\".\r\n", pathIn);
        goto labelEnd;
    }

    file_fstat((int)fileSrc, &buf);
    size = buf.st_size - sizeof(WavHeadT);
    SYSLOG_DEBUG("The size of the file %s is %d bytes.\r\n", pathIn, buf.st_size);
    file_fseek(fileSrc, sizeof(WavHeadT), SEEK_SET);

    fileDest = file_fopen(pathOut, "w");
    if (fileDest == NULL)
    {
        SYSLOG_DEBUG("Failed to open the file \"%s\".\r\n", pathOut);
        goto labelEnd;
    }

    count = (size / BUFFER_SIZE_PCM) + (((size % BUFFER_SIZE_PCM) == 0) ? 0 : 1);
    for (uint32_t i=0; i<count; i++)
    {
        memset(bufferSrc,  0, sizeof(bufferSrc));
        memset(bufferDest, 0, sizeof(bufferDest));
        length = file_fread(bufferSrc, 1, BUFFER_SIZE_PCM, fileSrc);
        length = g711_encode((unsigned char *)bufferDest, bufferSrc, length);
        file_fwrite(bufferDest, 1, length, fileDest);
    }

    file_fstat((int)fileDest, &buf);
    SYSLOG_DEBUG("The size of the file %s is %d bytes.\r\n", pathOut, buf.st_size);

    retVal = 0;

labelEnd:
    if (fileSrc != NULL)
    {
        file_fclose(fileSrc);
    }
    if (fileDest != NULL)
    {
        file_fclose(fileDest);
    }

    return retVal;
}

int32_t g711Decode(char *pathIn, char *pathOut)
{
    int32_t       retVal                       = -1;
    FILE         *fileSrc                      = NULL;
    FILE         *fileDest                     = NULL;
    struct stat   buf                          = {0};
    int32_t       size                         = 0;
    int32_t       count                        = 0;
    uint32_t      length                       = 0;
    char          bufferSrc[BUFFER_SIZE_PCM]   = {0};
    char          bufferDest[BUFFER_SIZE_G711] = {0};
    WavHeadT      wavHead   =
    {
        .riffId             = {'R', 'I', 'F', 'F'},
        .riffSize           = sizeof(wavHead) - sizeof(wavHead.riffId) - sizeof(wavHead.riffSize),
        .riffType           = {'W', 'A', 'V', 'E'},
        .fmtId              = {'f', 'm', 't', ' '},
        .fmtSize            = 16,
        .fmtCompressionCode = 1,
        .fmtChannels        = 1,
        .fmtSampleRate      = 8000,
        .fmtBytesPerSec     = 16000,
        .fmtBlockAlign      = 2,
        .fmtBitPerSample    = 16,
        .listOrDataId       = {'d', 'a', 't', 'a'},
        .listOrDataLength   = 0
    };

    if ((pathIn == NULL) || (pathOut == NULL))
    {
        SYSLOG_DEBUG("Param error.\r\n");
        goto labelEnd;
    }

    fileSrc = file_fopen(pathIn, "r");
    if (fileSrc == NULL)
    {
        SYSLOG_DEBUG("Failed to open the file \"%s\".\r\n", pathIn);
        goto labelEnd;
    }

    file_fstat((int)fileSrc, &buf);
    size = buf.st_size;
    SYSLOG_DEBUG("The size of the file %s is %d bytes.\r\n", pathIn, size);

    fileDest = file_fopen(pathOut, "w");
    if (fileDest == NULL)
    {
        SYSLOG_DEBUG("Failed to open the file \"%s\".\r\n", pathOut);
        goto labelEnd;
    }
    file_fwrite(&wavHead, 1, sizeof(wavHead), fileDest);

    count = (size / BUFFER_SIZE_G711) + (((size % BUFFER_SIZE_G711) == 0) ? 0 : 1);
    for (uint32_t i=0; i<count; i++)
    {
        memset(bufferSrc,  0, sizeof(bufferSrc));
        memset(bufferDest, 0, sizeof(bufferDest));
        length = file_fread(bufferSrc, 1, BUFFER_SIZE_G711, fileSrc);
        length = g711_decode(bufferDest, (const unsigned char *)bufferSrc, length);
        file_fwrite(bufferDest, 1, length, fileDest);
    }

    file_fstat((int)fileDest, &buf);
    wavHead.listOrDataLength  = buf.st_size - sizeof(wavHead);
    wavHead.riffSize         += wavHead.listOrDataLength;
    file_fseek(fileDest, 0, SEEK_SET);
    file_fwrite(&wavHead, 1, sizeof(wavHead), fileDest);
    SYSLOG_DEBUG("The size of the file %s is %d bytes.\r\n", pathOut, buf.st_size);

    retVal = 0;

labelEnd:
    if (fileSrc != NULL)
    {
        file_fclose(fileSrc);
    }
    if (fileDest != NULL)
    {
        file_fclose(fileDest);
    }

    return retVal;
}

static void g711PlayTimerFunc(void *argument)
{
	QueueTonePlayT queue = {0};
	osStatus_t stat = osOK;
	int32_t streamBlkSize = 0;
	uint8_t pcmDataBuff[320] = {0};
	streamBlkSize = xEcRingGetOption(gAudStreamRingBuf, E_LRO_DATA_SIZE);
	if(streamBlkSize == 0)
	{
		if(xEcRingGetOption(medGetDataCache(), E_LRO_DATA_SIZE) < MED_G711_TO_PCM_FRAME_SIZE)
		{
			memset(pcmDataBuff,0,320);
			xEcRingWriteEx(medGetDataCache(), pcmDataBuff, 320);
		}
	}

	if(g711PlayQueue != NULL)
	{
		if(medDataHandleStateGet() >= MED_DATA_HDL_STA_SUSPEND)
		{
			queue.endFlag = true;
		}
		stat = osMessageQueuePut(g711PlayQueue, &queue, 0, AUD_CODEC_SIG_TIMEROUT_MAX);
		if(stat != osOK)
		{
			ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, g711PlayTimerFunc_valid, P_ERROR, "message send fail(%d)",stat);
		}
	}

}


int32_t audG711StreamPlay(audioParamT *audParam)
{
	int32_t ret = AV_RET_PLAY_ERROR;

	rawDataHanderParam_T dataHandleParam = {0};
	QueueTonePlayT queue = {0};
	ecRingT *pcmCache = NULL;
	uint32_t cacheFreeSize = 0;
	uint8_t *pcmDataBuff = NULL;
	uint8_t *g711DataBuff = NULL;
	int32_t readLen = 0;
	int32_t streamBlkSize = 0;

	EC_ASSERT(gAudStreamRingBuf != NULL,0,0,0);
	//while ((xEcRingGetOption(gAudStreamRingBuf, E_LRO_DATA_SIZE) == 0) && getStreamCnt < 200)
	//{
	//	getStreamCnt++;
	//	osDelay(10);		
	//}
	pcmDataBuff = malloc(MED_G711_TO_PCM_FRAME_SIZE);
	if (!pcmDataBuff)
	{		
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audG711StreamPlay_invalid_1, P_ERROR, "G711DataBuff alloc fail ");
		ret = AV_RET_PLAY_ERROR;
		goto EXIT;
	}

	g711DataBuff = malloc(MED_G711_TO_PCM_FRAME_SIZE / 2);
	if (!g711DataBuff)
	{		
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audG711StreamPlay_invalid_2, P_ERROR, "G711DataBuff alloc fail ");
		ret = AV_RET_PLAY_ERROR;
		goto EXIT;
	}
	
	pcmCache = medInitDataCache( G711_PCM_DATA_CACHE_SIZE,  G711_PCM_DATA_CACHE_PRE_FILL_SIZE);
	g711PlayQueue = osMessageQueueNew(100, sizeof(queue), NULL);
	if (g711PlayQueue == NULL)
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audG711StreamPlay_invalid_3, P_ERROR, "PCM QUEUE create fail ");
		ret = AV_RET_PLAY_ERROR;
		goto EXIT;
	}
	
	g711PlayTimer = osTimerNew((osTimerFunc_t)g711PlayTimerFunc,osTimerPeriodic,NULL,NULL);
	
	dataHandleParam.field.samplerate = audParam->rate;
	dataHandleParam.field.envType = (audParam->toneFlag ? MED_DATA_ENV_TYPE_TONE : MED_DATA_ENV_TYPE_LOCAL);
	dataHandleParam.field.bitWidth = audParam->BitWidth;//default 16bit
	medDataHandleStart(&dataHandleParam);
	osTimerStart(g711PlayTimer, 20);
	
	while(1)
	{
		memset(&queue, 0, sizeof(queue));
		if(osMessageQueueGet(g711PlayQueue, &queue, 0, osWaitForever) == osOK)
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
			
			if((cacheFreeSize <= MED_G711_TO_PCM_FRAME_SIZE*2))/*16 bit*/
			{
				continue;
			}
			streamBlkSize = xEcRingGetOption(gAudStreamRingBuf, E_LRO_DATA_SIZE);
			ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audG711StreamPlay_DEBUG_0, P_DEBUG, "streamBlkSize(%d)", streamBlkSize);
			if(streamBlkSize > 0)
			{
				readLen = (cacheFreeSize > streamBlkSize ? streamBlkSize : cacheFreeSize);
				readLen = (readLen > (MED_G711_TO_PCM_FRAME_SIZE / 2)  ? (MED_G711_TO_PCM_FRAME_SIZE / 2) : readLen);
				memset(pcmDataBuff,0x00,MED_G711_TO_PCM_FRAME_SIZE);
				memset(g711DataBuff,0x00,MED_G711_TO_PCM_FRAME_SIZE/2);
				readLen = xEcRingRead(gAudStreamRingBuf,g711DataBuff,readLen);
				readLen = g711_decode((char *)pcmDataBuff,g711DataBuff,readLen);
				ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, audG711StreamPlay_DEBUG, P_DEBUG, "readLen(%d)", readLen);
				ECPLAT_DUMP(UNILOG_PLAT_MEDIA, audG711StreamPlay_DUMP, P_DEBUG, "", readLen,pcmDataBuff);
				xEcRingWriteEx(pcmCache, pcmDataBuff, readLen);		
			}

		}
	}

EXIT:
	
	if(pcmDataBuff)
	{
		free(pcmDataBuff);
		pcmDataBuff = NULL;
	}
	if(g711DataBuff)
	{
		free(g711DataBuff);
		g711DataBuff = NULL;
	}
	if(g711PlayTimer != NULL)
	{
		if(osTimerIsRunning(g711PlayTimer))
			osTimerStop(g711PlayTimer);
		osTimerDelete(g711PlayTimer);
		g711PlayTimer = NULL;
	}
	if(g711PlayQueue != NULL)
	{
		osMessageQueueDelete(g711PlayQueue);
		g711PlayQueue = NULL;
	}
	return ret;
}

int32_t audG711PlayStop(void)
{
	QueueTonePlayT queue = {0};
	if(g711PlayQueue != NULL)
	{
		queue.endFlag = true;
		osMessageQueuePutToFront(g711PlayQueue, &queue, 0, osWaitForever);
	}
    return 0;
}	

