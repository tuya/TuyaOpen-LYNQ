/****************************************************************************
*
* Copy right:   2024-, Copyrigths of EigenComm Ltd.
* File name:    mp3.c
* Description:  EC718 media play source file
* History:      Rev1.0   2024-03-13
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

#include "mad.h"
#include DEBUG_LOG_HEADER_FILE
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
#include "flashex.h"
#endif

#include "media.h"
#include "mp3.h"
#include "medDataHandle.h"

/*----------------------------------------------------------------------------*
 *					  MACROS												  *
 *----------------------------------------------------------------------------*/
#define INPUT_BUFFER_SIZE                       2000
#define AUDIO_MP3_DECODE_FRAME_SIZE				1152


#define MP3_DATA_CACHE_PRE_FILL_SIZE			(16*1024)
#define MP3_DATA_CACHE_SIZE						(MP3_DATA_CACHE_PRE_FILL_SIZE + AUDIO_MP3_DECODE_FRAME_SIZE*3)


#define FACTOR                                  1

/*----------------------------------------------------------------------------*
 * 					 GLOBAL VARIABLES									  	  *
 *----------------------------------------------------------------------------*/

static audioPlayStatusInfo audioStatusInfo = {NULL,0,0,0};

static struct mad_stream stream = {0};
static struct mad_frame  frame = {0};
static struct mad_synth  synth = {0};
#if (PSRAM_EXIST == 1)
static PLAT_FPSRAM_ZI_CUST uint8_t           inputBuffer[INPUT_BUFFER_SIZE + MAD_BUFFER_GUARD]; // Huffman decoding needs guard buffer
#else
static uint8_t           inputBuffer[INPUT_BUFFER_SIZE + MAD_BUFFER_GUARD]; // Huffman decoding needs guard buffer
#endif

uint16_t bitrateTable[3][14] = {
								{32,40,48,56,64,80,96,112,128,160,192,224,256,320},
							  	{8,16,24,32,64,80,56,64,128,160,112,128,256,320},							
							  	{8,16,24,32,40,48,56,64,80,96,112,128,144,160}
							};


osTimerId_t 	   mp3DecodeTimer = NULL;
osMessageQueueId_t gToneDecodeQueue = NULL;

#ifdef FEATURE_SUBSYS_MEDIA_STREAM_ENABLE
extern ecRingT *gAudStreamRingBuf;
#endif

extern uint32_t	gCurMp3PlayTime;//ms

/*----------------------------------------------------------------------------*
 * 				     PRIVATE FUNCTION					   	   	  			  *
 *----------------------------------------------------------------------------*/

static inline signed short madFixedToSignedShort(mad_fixed_t sample)
{
    // round
    sample += (1L << (MAD_F_FRACBITS - 16));

    // clip
    if (sample >= MAD_F_ONE)
        sample = MAD_F_ONE - 1;
    else if (sample < -MAD_F_ONE)
        sample = -MAD_F_ONE;

    // quantize 
    return sample >> (MAD_F_FRACBITS + 1 - 16);
}

static void mp3DecoderTimerFunc(void *argument)
{
	QueueTonePlayT queue = {0};
	if(gToneDecodeQueue != NULL)
	{
		if(medDataHandleStateGet() >= MED_DATA_HDL_STA_SUSPEND)
		{
			queue.endFlag = true;
		}
		osMessageQueuePut(gToneDecodeQueue, &queue, 0, osWaitForever);
	}
}

static int32_t mp3StringDecoder(uint8_t *string, uint32_t stringLen, audioParamT *audParam)
{
	int32_t  status=AV_RET_PLAY_ERROR, i;
	uint32_t readSize, remaining, stringRemain;
	uint8_t  *readStart = NULL;
	uint8_t *inputPtr = NULL;
	uint8_t  factor 	= 1;
	uint16_t curVolume = 0;
	uint32_t debugSize = 0;
	INT32 freeSize = 0;
	ecRingT *pcmCache = NULL;
	rawDataHanderParam_T dataHandleParam = {0};
	
	mad_stream_init(&stream);
	mad_frame_init(&frame);
	mad_synth_init(&synth);

	inputPtr = string;
	stringRemain = stringLen;
	QueueTonePlayT queue = {0};
	
	gToneDecodeQueue = osMessageQueueNew(100, sizeof(queue), NULL);
	if (gToneDecodeQueue == NULL)
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, mp3StringDecoder_invalid_1, P_ERROR, "create tone queue fail");
		return status;
	}
	pcmCache = medInitDataCache(MP3_DATA_CACHE_SIZE,MP3_DATA_CACHE_PRE_FILL_SIZE);	
	mp3DecodeTimer = osTimerNew((osTimerFunc_t)mp3DecoderTimerFunc,osTimerPeriodic,NULL,NULL);
	
	dataHandleParam.field.samplerate = audParam->rate;
	dataHandleParam.field.envType = MED_DATA_ENV_TYPE_LOCAL;
	dataHandleParam.field.bitWidth = 0;//default 16bit
	medDataHandleStart(&dataHandleParam);
	osTimerStart(mp3DecodeTimer, 20);
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, mp3StringDecoder_1, P_DEBUG, "start decode mp3 string");
	while(1)
	{
		memset(&queue, 0, sizeof(queue));
		if(osMessageQueueGet(gToneDecodeQueue, &queue, 0, osWaitForever) == osOK)
		{
			if(queue.endFlag)
			{
				if(medDataHandleStateGet() >= MED_DATA_HDL_STA_SUSPEND)
					status = AV_RET_PLAY_EOF;
				else
					status = AV_RET_PLAY_STOP;
				medDataHandleStop();
				break;
			}
			freeSize = xEcRingGetOption(pcmCache,E_LRO_FREE_SIZE);
			if(freeSize <= AUDIO_MP3_DECODE_FRAME_SIZE*2)/*16 bit*/
			{
				continue;
			}
			if(stream.buffer == NULL || stream.error == MAD_ERROR_BUFLEN)
			{
				if (stringRemain == 0)
				{
					continue;
				}
				if(stream.next_frame != NULL)
				{
					remaining = stream.bufend - stream.next_frame;
					memmove(inputBuffer, stream.next_frame, remaining);
					readStart = inputBuffer + remaining;
					readSize = INPUT_BUFFER_SIZE - remaining;
				}
				else // First frame
				{
					readSize  = INPUT_BUFFER_SIZE,
					readStart = inputBuffer,
					remaining = 0;
				}
				memset(readStart, 0, readSize);

				if(readSize > stringRemain)
					readSize = stringRemain;
				
				memcpy(readStart,inputPtr,readSize);
				inputPtr = inputPtr + readSize;
				stringRemain = stringRemain - readSize;
				
				mad_stream_buffer(&stream, inputBuffer, readSize + remaining);
				debugSize = readSize + remaining;
				stream.error = MAD_ERROR_NONE;
			}
			
			if (mad_frame_decode(&frame, &stream))
			{
				if (MAD_RECOVERABLE(stream.error))
				{
						continue;
				}
				else
				{
					if(stream.error == MAD_ERROR_BUFLEN)
					{
							continue;
					}
					else
					{
						ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, mp3StringDecoder_2, P_ERROR, "stream.error is %d  ",stream.error);
						status = AV_RET_PLAY_ERROR;
						break;
					}
				}
			}
			mad_synth_frame(&synth, &frame);

			for(i = 0; i < synth.pcm.length; i++)
			{
				signed short sample = madFixedToSignedShort(synth.pcm.samples[0][i]);
				
				for (uint8_t j=0; j<factor; j++)
				{
					xEcRingWriteEx(pcmCache,(UINT8 *)&sample, 2);
				}
			}
		}
	}
	//medDataHandleStop(&dataHandleParam);
	mad_synth_finish(&synth);
	mad_frame_finish(&frame);
	mad_stream_finish(&stream);

	if(mp3DecodeTimer != NULL)
	{
		if(osTimerIsRunning(mp3DecodeTimer))
			osTimerStop(mp3DecodeTimer);
		osTimerDelete(mp3DecodeTimer);
		mp3DecodeTimer = NULL;
	}
	if(gToneDecodeQueue != NULL)
	{
		osMessageQueueDelete(gToneDecodeQueue);
		gToneDecodeQueue = NULL;
	}
	return status;
}


static int32_t mp3StreamDecoder(audioParamT *audParam)
{
	int32_t		status = AV_RET_PLAY_ERROR,i;
	uint32_t 	readSize, remaining, readLen;
	uint8_t *	readStart = NULL;
	uint8_t  	factor 	= 1;
	uint16_t 	curVolume = 0;
	uint32_t 	debugSize = 0;
	INT32 		freeSize = 0;
	INT32       streamSize = 0;
	uint32_t ringDateSize = 0;
	rawDataHanderParam_T dataHandleParam = {0};
	
	mad_stream_init(&stream);
	mad_frame_init(&frame);
	mad_synth_init(&synth);
	
	QueueTonePlayT queue = {0};
	ecRingT *pcmCache = NULL;

	gToneDecodeQueue = osMessageQueueNew(100, sizeof(queue), NULL);
	if (gToneDecodeQueue == NULL)
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, mp3StreamDecoder_invalid_1, P_ERROR, "create tone queue fail");
		return status;
	}
	pcmCache = medInitDataCache(MP3_DATA_CACHE_SIZE,MP3_DATA_CACHE_PRE_FILL_SIZE);	
	mp3DecodeTimer = osTimerNew((osTimerFunc_t)mp3DecoderTimerFunc,osTimerPeriodic,NULL,NULL);
	
	dataHandleParam.field.samplerate = audParam->rate;
	dataHandleParam.field.envType = MED_DATA_ENV_TYPE_LOCAL;
	dataHandleParam.field.bitWidth = 0;//default 16bit
	medDataHandleStart(&dataHandleParam);
	osTimerStart(mp3DecodeTimer, 20);
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, mp3StreamDecoder_1, P_DEBUG, "start decode mp3 stream");

	while(1)
	{
		memset(&queue, 0, sizeof(queue));
		if(osMessageQueueGet(gToneDecodeQueue, &queue, 0, osWaitForever) == osOK)
		{
			if(queue.endFlag)
			{
				if(medDataHandleStateGet() >= MED_DATA_HDL_STA_SUSPEND)
					status = AV_RET_PLAY_EOF;
				else
					status = AV_RET_PLAY_STOP;
				medDataHandleStop();
				break;
			}

			freeSize = xEcRingGetOption(pcmCache,E_LRO_FREE_SIZE);			
			if(freeSize <= AUDIO_MP3_DECODE_FRAME_SIZE*2)/*16 bit*/
			{
				continue;
			}
			if(stream.buffer == NULL || stream.error == MAD_ERROR_BUFLEN)
			{
				#ifdef FEATURE_SUBSYS_MEDIA_STREAM_ENABLE
				if(xEcRingGetOption(gAudStreamRingBuf, E_LRO_DATA_SIZE) == 0)
				{					
					continue;
				}
				#endif
				if(stream.next_frame != NULL)
				{
					remaining = stream.bufend - stream.next_frame;
					memmove(inputBuffer, stream.next_frame, remaining);
					readStart = inputBuffer + remaining;
					readSize = INPUT_BUFFER_SIZE - remaining;
				}
				else // First frame
				{
					readSize  = INPUT_BUFFER_SIZE,
					readStart = inputBuffer,
					remaining = 0;
				}
				memset(readStart, 0, readSize);
				#ifdef FEATURE_SUBSYS_MEDIA_STREAM_ENABLE
				readLen = xEcRingRead(gAudStreamRingBuf, readStart, readSize);
				#endif
				mad_stream_buffer(&stream, inputBuffer, readSize + remaining);
				debugSize = readSize + remaining;
				stream.error = MAD_ERROR_NONE;
			}
			
			if (mad_frame_decode(&frame, &stream))
			{
				if (MAD_RECOVERABLE(stream.error))
				{
						continue;
				}
				else
				{
					if(stream.error == MAD_ERROR_BUFLEN)
					{
							continue;
					}
					else
					{
						ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, mp3StreamDecoder_3, P_ERROR, "stream.error is %d  ",stream.error);
						status = AV_RET_PLAY_ERROR;
						break;
					}
				}
			}
			mad_synth_frame(&synth, &frame);

			for(i = 0; i < synth.pcm.length; i++)
			{
				signed short sample = madFixedToSignedShort(synth.pcm.samples[0][i]);
				
				for (uint8_t j=0; j<factor; j++)
				{
					xEcRingWriteEx(pcmCache,(UINT8 *)&sample, 2);
				}
			}
		}
	}
	//medDataHandleStop(&dataHandleParam);
	mad_synth_finish(&synth);
	mad_frame_finish(&frame);
	mad_stream_finish(&stream);

	if(mp3DecodeTimer != NULL)
	{
		if(osTimerIsRunning(mp3DecodeTimer))
			osTimerStop(mp3DecodeTimer);
		osTimerDelete(mp3DecodeTimer);
		mp3DecodeTimer = NULL;
	}
	if(gToneDecodeQueue != NULL)
	{
		osMessageQueueDelete(gToneDecodeQueue);
		gToneDecodeQueue = NULL;
	}
	return status;
}


static int32_t mp3FileDecoder(FILE *file, audioParamT *audParam)
{
    struct stat buf = {0};
    int32_t  status=AV_RET_PLAY_ERROR, i;
    uint32_t readSize, remaining, readLen;
    uint8_t  *readStart = NULL;
    uint8_t  factor     = 1;
	uint16_t curVolume = 0;
	uint32_t debugSize = 0;
	INT32 freeSize = 0;
	rawDataHanderParam_T dataHandleParam = {0};
	
	mad_stream_init(&stream);
	mad_frame_init(&frame);
	mad_synth_init(&synth);
	
	QueueTonePlayT queue = {0};
	ecRingT *pcmCache = NULL;

	gToneDecodeQueue = osMessageQueueNew(100, sizeof(queue), NULL);
    if (gToneDecodeQueue == NULL)
    {
		return -1;
    }
	pcmCache = medInitDataCache(MP3_DATA_CACHE_SIZE,MP3_DATA_CACHE_PRE_FILL_SIZE);	
	mp3DecodeTimer = osTimerNew((osTimerFunc_t)mp3DecoderTimerFunc,osTimerPeriodic,NULL,NULL);
	
	dataHandleParam.field.samplerate = audParam->rate;
	dataHandleParam.field.envType = (audParam->toneFlag ? MED_DATA_ENV_TYPE_TONE : MED_DATA_ENV_TYPE_LOCAL);
	dataHandleParam.field.bitWidth = 0;//default 16bit
	medDataHandleStart(&dataHandleParam);
	osTimerStart(mp3DecodeTimer, 20);
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, mp3FileDecoder_1, P_DEBUG, "start decode mp3 file");
	while(1)
	{
		memset(&queue, 0, sizeof(queue));
        if(osMessageQueueGet(gToneDecodeQueue, &queue, 0, osWaitForever) == osOK)
    	{
    		if(queue.endFlag)
			{
				if(mp3DecodeTimer != NULL)
				{
					if(osTimerIsRunning(mp3DecodeTimer))
						osTimerStop(mp3DecodeTimer);
					osTimerDelete(mp3DecodeTimer);
					mp3DecodeTimer = NULL;
				}
				if(medDataHandleStateGet() >= MED_DATA_HDL_STA_SUSPEND)
					status = AV_RET_PLAY_EOF;
				else
					status = AV_RET_PLAY_STOP;
				medDataHandleStop();
				break;
			}
			freeSize = medGetDataCacheAvlbSize();
			if(freeSize <= AUDIO_MP3_DECODE_FRAME_SIZE*2)/*16 bit*/
			{
				continue;
			}
			if(stream.buffer == NULL || stream.error == MAD_ERROR_BUFLEN)
			{
                file_fstat((int)file, &buf);
	            if (file_ftell(file) == buf.st_size)
	            {
					if(audParam->toneFlag)
					{
						file_fseek(file,0,0);
						mad_synth_finish(&synth);
						mad_frame_finish(&frame);
						mad_stream_finish(&stream);

						mad_stream_init(&stream);
						mad_frame_init(&frame);
						mad_synth_init(&synth);
						continue;
					}
					else
					{
						continue;
					}
	            }
				if(stream.next_frame != NULL)
				{
					remaining = stream.bufend - stream.next_frame;
					memmove(inputBuffer, stream.next_frame, remaining);
					readStart = inputBuffer + remaining;
					readSize = INPUT_BUFFER_SIZE - remaining;
				}
				else // First frame
				{
					readSize  = INPUT_BUFFER_SIZE,
				   	readStart = inputBuffer,
					remaining = 0;
				}
	            memset(readStart, 0, readSize);

				readLen = file_fread(readStart, 1, readSize, file);
				if(audioStatusInfo.path != NULL)
					audioStatusInfo.decodeSize = audioStatusInfo.decodeSize + readLen;
				
				mad_stream_buffer(&stream, inputBuffer, readSize + remaining);
				debugSize = readSize + remaining;
				ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, mp3FileDecoder_2, P_DEBUG, "decode mp3 read file Size [%d] debugSize [%d]",readSize, debugSize);
				stream.error = MAD_ERROR_NONE;
			}
			
			if (mad_frame_decode(&frame, &stream))
			{
				if (MAD_RECOVERABLE(stream.error))
				{
						continue;
				}
				else
				{
					if(stream.error == MAD_ERROR_BUFLEN)
					{
							continue;
					}
					else
					{
						ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, mp3FileDecoder_4, P_ERROR, "stream.error is %d  ",stream.error);
						status = AV_RET_PLAY_ERROR;
						break;
					}
				}
			}
			mad_synth_frame(&synth, &frame);

			for(i = 0; i < synth.pcm.length; i++)
			{
	 			signed short sample = madFixedToSignedShort(synth.pcm.samples[0][i]);
				
	            for (uint8_t j=0; j<factor; j++)
	            {
                	xEcRingWriteEx(pcmCache,(UINT8 *)&sample, 2);
	            }
			}
		}
	}
	//medDataHandleStop(&dataHandleParam);
	mad_synth_finish(&synth);
	mad_frame_finish(&frame);
	mad_stream_finish(&stream);

	if(mp3DecodeTimer != NULL)
	{
		if(osTimerIsRunning(mp3DecodeTimer))
			osTimerStop(mp3DecodeTimer);
		osTimerDelete(mp3DecodeTimer);
		mp3DecodeTimer = NULL;
	}
	if(gToneDecodeQueue != NULL)
	{
		osMessageQueueDelete(gToneDecodeQueue);
		gToneDecodeQueue = NULL;
	}
	return status;
}

static uint8_t mp3SampleRateGet(char *path)
{
    FILE       *file       = NULL;
    struct stat buf        = {0};
    uint8_t     buffer[10] = {0};
    uint32_t    length     = 0;
    uint8_t     version    = 0;
    uint8_t     index      = 0;
    uint32_t    sample     = 0;
    uint32_t    table[3]   = {44100, 48000, 32000};

	uint16_t	bitrate = 0;
	uint8_t 	bitrateIndex = 0;
	uint32_t	headSize = 0;
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
	fileSize = buf.st_size;
    file_fclose(file);
#endif

    version = (buffer[1] >> 3) & 0x03;
    index   = (buffer[2] >> 2) & 0x03;
	bitrateIndex = (buffer[2] >> 4) & 0x0f;
	
    if (version == 0x00) // MPEG2.5
    {
        sample = table[index] / 4;
		bitrate = bitrateTable[2][bitrateIndex - 1];
    }
    else if (version == 0x2) // MPEG2
    {
        sample = table[index] / 2;
		bitrate = bitrateTable[1][bitrateIndex - 1];
    }
    else if (version == 0x3) // MPEG1
    {
        sample = table[index];
		bitrate = bitrateTable[0][bitrateIndex - 1];
    }
    else
    {
        sample = 0;
		bitrate = 128;
    }
	gCurMp3PlayTime = (fileSize - headSize)  * 8 / bitrate;
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, mp3SampleRateGet, P_INFO, "playTime is %d,sample is ",gCurMp3PlayTime, sample);
    sample = sampleRateConvert(sample, true);
	

labelEnd:
    return sample;
}

static uint8_t mp3SampleRateGetStream(uint8_t *string)
{

    uint8_t    buffer[10] = {0};
	uint8_t    *preReadBuf = NULL;
    uint32_t   length     = 0;
    uint8_t    version    = 0;
    uint8_t    index      = 0;
    uint32_t   sample     = 0xff;
    uint32_t   table[3]   = {44100, 48000, 32000};

	if(string != NULL)
	{	
		memcpy(buffer, string, 10);		
	}
	else
	
{
		goto labelEnd;
	}

    if (memcmp(buffer, "ID3", strlen("ID3")) == 0)
    {
        length = ((buffer[6] & 0x7F) << 21) + ((buffer[7] & 0x7F) << 14) + ((buffer[8] & 0x7F) << 7) + (buffer[9] & 0x7F);
		preReadBuf = malloc(length + 10 + 3);
		memcpy(preReadBuf, string, length + 10 + 3);
		//xEcRingPreread(gEcRingBuf,preReadBuf,length + 10 + 3);
		version = (preReadBuf[length + 10 + 1] >> 3) & 0x03;
    	index   = (preReadBuf[length + 10 + 2] >> 2) & 0x03;
    }
	else
	{
		version = (buffer[1] >> 3) & 0x03;
    	index   = (buffer[2] >> 2) & 0x03;
	}
    

    if (version == 0x00) // MPEG2.5
    {
        sample = table[index] / 4;
    }
    else if (version == 0x2) // MPEG2
    {
        sample = table[index] / 2;
    }
    else if (version == 0x3) // MPEG1
    {
        sample = table[index];
    }
    else
    {
        sample = 0;
    }

    sample = sampleRateConvert(sample, true);
	free(preReadBuf);
labelEnd:
    return sample;
}

/*----------------------------------------------------------------------------*
 * 				     GLOBAL FUNCTION					   	   	  			  *
 *----------------------------------------------------------------------------*/
int32_t mp3Play(char *path, audioParamT *audParam)
{
	int32_t ret = AV_RET_PLAY_ERROR;
    uint8_t     rate = 0;
	FILE       *file = NULL;
    struct stat buf  = {0};

	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, mp3Play_0, P_DEBUG, "mp3Play enter");
    rate = mp3SampleRateGet(path);
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, mp3Play_1, P_DEBUG, "rate is %d ",rate);

    if ((rate >= SAMPLERATE_8K) && (rate <= SAMPLERATE_96K))
    {	
    	audParam->rate = rate;
    	file = file_fopen(path, "r");
		if (file != NULL)
		{
            file_fstat((int)file, &buf);
			if(!audParam->toneFlag)
			{
				if((audioStatusInfo.path != NULL) && (audioStatusInfo.resumeIndex != 0))
				{
					if(!memcmp(audioStatusInfo.path,path,strlen(audioStatusInfo.path)))
					{
						file_fseek(file, audioStatusInfo.resumeIndex, SEEK_SET);
					}
					else
					{
						memset(audioStatusInfo.path,0,strlen(audioStatusInfo.path));
						memcpy(audioStatusInfo.path,path,strlen(path));
						audioStatusInfo.resumeIndex = 0;
						audioStatusInfo.decodeSize = 0;
						audioStatusInfo.fileSize = buf.st_size;
					}
				}
				else
				{
					audioStatusInfo.path = (char *)malloc(strlen(path) + 1);
					if(audioStatusInfo.path != NULL)
					{
						memset(audioStatusInfo.path,0,strlen(path) + 1);
						memcpy(audioStatusInfo.path,path,strlen(path));
						audioStatusInfo.resumeIndex = 0;
						audioStatusInfo.decodeSize = 0;
						audioStatusInfo.fileSize = buf.st_size;
					}				
				}
			}
        	ret = mp3FileDecoder(file,audParam);
			if(!audParam->toneFlag)
			{
				if(file_ftell(file) != buf.st_size)
				{
					audioStatusInfo.resumeIndex = file_ftell(file);
				}
				else
				{
					if(audioStatusInfo.path != NULL)
					{
						free(audioStatusInfo.path);
						audioStatusInfo.path = NULL;
					}
					audioStatusInfo.resumeIndex = 0;
					audioStatusInfo.fileSize = 0;
					audioStatusInfo.decodeSize = 0;
				}
			}
			file_fclose(file);
		}
    }
	else
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, mp3Play_2, P_DEBUG, "wrong samplerate[%d] ",rate);
	}
	return ret;
}


int32_t mp3StringPlay(uint8_t *string,UINT32 stringLen,audioParamT *audParam)
{
	int32_t ret = AV_RET_PLAY_ERROR;
	uint8_t    rate   = 0;
	if(string == NULL || stringLen < 4)
		return;
	rate = mp3SampleRateGetStream(string);
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, mp3StringPlay, P_WARNING, "rate is %d ",rate);

	if ((rate >= SAMPLERATE_8K) && (rate <= SAMPLERATE_96K))
	{
		audParam->rate = rate;
		ret = mp3StringDecoder(string,stringLen,audParam);
	}
	return ret;
}

int32_t mp3StreamPlay(audioParamT *audParam)
{
	int32_t ret = AV_RET_PLAY_START;
#ifdef FEATURE_SUBSYS_MEDIA_STREAM_ENABLE
	uint8_t    rate   = 0;
	EC_ASSERT(gAudStreamRingBuf != NULL,0,0,0);
	uint32_t getStreamCnt = 0;
	uint32_t streamDataSize = 0;

	while ((streamDataSize <= AUDIO_MP3_DECODE_FRAME_SIZE*2) && (getStreamCnt < 500))
	{
		streamDataSize = xEcRingGetOption(gAudStreamRingBuf, E_LRO_DATA_SIZE);
		getStreamCnt++;
		osDelay(10);		
	}

	rate = mp3SampleRateGetStream(gAudStreamRingBuf->pDataBuff);
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, mmp3SteamPlay_2, P_WARNING, "rate is %d cnt is %d",rate, getStreamCnt);
	if ((rate >= SAMPLERATE_8K) && (rate <= SAMPLERATE_96K))
	{
		audParam->rate = rate;
		ret = mp3StreamDecoder(audParam);
	}	
	else
		return ret;
#endif
	return ret;
}


uint32_t getMp3PlaySchedule(void)
{
	uint32_t playSechedule = 0;
	if(audioStatusInfo.path == NULL || audioStatusInfo.fileSize == 0)
		return 0;
	playSechedule = audioStatusInfo.decodeSize * 100 / audioStatusInfo.fileSize ;
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, getMp3PlaySchedule, P_DEBUG, "play Schedule is %d% ",playSechedule);
	//printf("play Schedule is %d%\r\n",playSechedule);
	return playSechedule;
}

uint32_t mp3PlayStop(bool puaseFlag)
{
	QueueTonePlayT queue = {0};
	int32_t ret = 0;
	if(gToneDecodeQueue != NULL)
	{
		queue.endFlag = true;
		osMessageQueuePutToFront(gToneDecodeQueue, &queue, 0, osWaitForever);
	}
	if(!puaseFlag)
		memset(&audioStatusInfo,0x00,sizeof(audioPlayStatusInfo));
	return ret;
}
