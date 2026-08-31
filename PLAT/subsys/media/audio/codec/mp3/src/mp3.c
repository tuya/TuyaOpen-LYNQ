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
#if defined(FEATURE_HTTPC_ENABLE) && defined(FEATURE_HTTP_TLS_ENABLE) && \
    defined(FEATURE_SUBSYS_MEDIA_STREAM_ENABLE) &&                       \
    defined(FEATURE_SUBSYS_HTTP_DOWNLOADER_ENABLE)
#include "http_downloader.h"
#endif
#include "media.h"
#include "mp3.h"
#include "medDataHandle.h"
#include "syslog.h"
/*----------------------------------------------------------------------------*
 *					  MACROS												  *
 *----------------------------------------------------------------------------*/
#define INPUT_BUFFER_SIZE                       (2000*2)
#define AUDIO_MP3_DECODE_FRAME_SIZE				1152

//播放网络数据时，需要进行数据缓存，MP3_NETPLAY_BUFFER_PACKET_CNT参数用于控制缓存的数据包数量。
//其中每个数据包为AUDIO_MP3_DECODE_FRAME_SIZE大小。当数据量达到缓存包数后才会开始播放。
//该参数越大，播放开始的时间会越延迟，但是后续的播放会越流畅
#define MP3_NETPLAY_BUFFER_PACKET_CNT			(20)

//播放网络数据时，需要进行数据缓存，MP3_NETPLAY_BUFFER_TIMEOUT参数用于控制缓存的超时时间。
//单位为ms，超过该时间仍未返回足够的数据则停止播放
//该参数越大，由于网络速度较慢引起的播放暂停时间越长。但是后续播放越流畅。
#define MP3_NETPLAY_BUFFER_TIMEOUT (2000)

//当网络中断时，播放会暂停，如果一直等不到数据，会超时结束播放，参数由WAIT_FOR_DATA_TIMEOUT_CNT决定
//单位为计数器到期次数。目前每次计数器到期时间为20ms.
//该参数越大，由于网络中断导致的播放中止的时间越长。
#define WAIT_FOR_DATA_TIMEOUT_CNT				(200)
//当下载速度较慢时，会等待下载数据缓存到达指定数量后，才会继续播放。参数由WAIT_FOR_DATA_PACKET_CNT决定
//单位为数据包数量，目前每个数据包为一个MTU大小，1500Byte。
//该参数越大，由于网络速度较慢引起的播放暂停时间越长。但是后续播放越流畅。
#define WAIT_FOR_DATA_PACKET_CNT				(10)

#define MP3_DATA_CACHE_PRE_FILL_SIZE			(16*1024)
#define MP3_DATA_CACHE_SIZE						(MP3_DATA_CACHE_PRE_FILL_SIZE + AUDIO_MP3_DECODE_FRAME_SIZE*3)


#define FACTOR                                  1
#define MP3_MEDIA_INFO_INPUT_BUFFER				(20 * 1024)
struct mp3_parse_data {
    FILE *mp3_file;
    unsigned char *input_buffer;
    AudioInfo_t *audio_param;
    int has_audio_info;       // 是否已获取到音频信息
	mad_timer_t total_duration;   // 总时长累加器
    int frame_count;          // 帧计数（用于计算平均码率）
};
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
extern DlState_e g_u8AudioDownloadOver;
#if defined(FEATURE_HTTPC_ENABLE) && defined(FEATURE_HTTP_TLS_ENABLE)
#if defined(FEATURE_SUBSYS_HTTP_DOWNLOADER_ENABLE)
#include "http_downloader.h"
extern HttpDownloader_t *gAudioStreamDownloader;
#endif
#endif
#endif
static volatile bool gMp3PlayStopFlag = false;
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
	osStatus_t stat = osOK;
	if(gToneDecodeQueue != NULL)
	{
		if(medDataHandleStateGet() >= MED_DATA_HDL_STA_SUSPEND)
		{
			queue.endFlag = true;
		}
		stat = osMessageQueuePut(gToneDecodeQueue, &queue, 0, AUD_CODEC_SIG_TIMEROUT_MAX);
		if(stat != osOK)
		{
			ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, mp3DecoderTimerFunc_valid, P_ERROR, "message send fail(%d)",stat);
		}
	}
}

static int32_t mp3StringDecoder(uint8_t *string, uint32_t stringLen, audioParamT *audParam)
{
	int32_t  status=AV_RET_PLAY_ERROR, i;
	uint32_t readSize, remaining, stringRemain;
	uint8_t  *readStart = NULL;
	uint8_t *inputPtr = NULL;
	uint8_t  factor 	= 1;
	// uint16_t curVolume = 0;
	// uint32_t debugSize = 0;
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
				gMp3PlayStopFlag = false;
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
					if(EC_audioPlayChkLoop())
					{

						inputPtr = string;
						stringRemain = stringLen;
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

				if(readSize > stringRemain)
					readSize = stringRemain;
				
				memcpy(readStart,inputPtr,readSize);
				inputPtr = inputPtr + readSize;
				stringRemain = stringRemain - readSize;
				
				mad_stream_buffer(&stream, inputBuffer, readSize + remaining);
				// debugSize = readSize + remaining;
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
	// uint16_t 	curVolume = 0;
	// uint32_t 	debugSize = 0;
	INT32 		freeSize = 0;
	// INT32       streamSize = 0;
	// uint32_t ringDateSize = 0;
	rawDataHanderParam_T dataHandleParam = {0};
#if defined(FEATURE_HTTPC_ENABLE) && defined(FEATURE_HTTP_TLS_ENABLE) && defined(FEATURE_SUBSYS_MEDIA_STREAM_ENABLE)
	bool wait_for_data = false;
	uint32_t wait_for_data_cnt = 0;
#endif
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
	uint8_t *zero_buff = NULL;
	zero_buff = malloc(2048);
	memset(zero_buff, 0, 2048);
	uint32_t packet_size = sampleRateConvert(dataHandleParam.field.samplerate,false) * 20 * 16 / 8 / 1000;
	while(1)
	{
		memset(&queue, 0, sizeof(queue));
		if(osMessageQueueGet(gToneDecodeQueue, &queue, 0, osWaitForever) == osOK)
		{
			if(queue.endFlag)
			{
#if defined(FEATURE_HTTPC_ENABLE) && defined(FEATURE_HTTP_TLS_ENABLE) && \
    defined(FEATURE_SUBSYS_MEDIA_STREAM_ENABLE) &&                       \
    defined(FEATURE_SUBSYS_HTTP_DOWNLOADER_ENABLE)
				http_downloader_stop(gAudioStreamDownloader);
#endif
				if(medDataHandleStateGet() >= MED_DATA_HDL_STA_SUSPEND)
				{
#if defined(FEATURE_HTTPC_ENABLE) && defined(FEATURE_HTTP_TLS_ENABLE) && defined(FEATURE_SUBSYS_MEDIA_STREAM_ENABLE)
					status = (g_u8AudioDownloadOver != DLSTATE_COMPLETED) ? AV_RET_PLAY_TERMINATE : AV_RET_PLAY_EOF;
#else
					status = AV_RET_PLAY_EOF;
#endif
				}
				else
				{
					status = AV_RET_PLAY_STOP;
				}
				medDataHandleStop();
#if defined(FEATURE_HTTPC_ENABLE) && defined(FEATURE_HTTP_TLS_ENABLE) && defined(FEATURE_SUBSYS_MEDIA_STREAM_ENABLE)
				wait_for_data = false;
				wait_for_data_cnt = 0;
				gMp3PlayStopFlag = false;
#endif
				ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, mp3StreamDecoder_stop, P_DEBUG, "stop decode mp3 stream");
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
				uint32_t data_size = xEcRingGetOption(gAudStreamRingBuf, E_LRO_DATA_SIZE);
#if defined(FEATURE_HTTPC_ENABLE) && defined(FEATURE_HTTP_TLS_ENABLE) && defined(FEATURE_SUBSYS_MEDIA_STREAM_ENABLE)
				uint32_t packet_size = sampleRateConvert(dataHandleParam.field.samplerate,false) * 20 * 16 / 8 / 1000;
				if(wait_for_data)
				{
					if(data_size < packet_size * WAIT_FOR_DATA_PACKET_CNT)
					{
						wait_for_data_cnt++;
					}
					else
					{
						wait_for_data = false;
						ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, mp3StreamDecoder_continue, P_DEBUG, "data enough, continue playing");
					}

					if(wait_for_data_cnt >= WAIT_FOR_DATA_TIMEOUT_CNT)
					{
						wait_for_data = false;
#if defined(FEATURE_HTTPC_ENABLE) && defined(FEATURE_HTTP_TLS_ENABLE) && \
    defined(FEATURE_SUBSYS_MEDIA_STREAM_ENABLE) &&                       \
    defined(FEATURE_SUBSYS_HTTP_DOWNLOADER_ENABLE)
						http_downloader_stop(gAudioStreamDownloader);
#endif
						ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, mp3StreamDecoder_wait_timeout, P_DEBUG, "wait for download data timeout");
					}
				} 
				else 
				{
					if(data_size == 0)
					{
						continue;
					} 
					else if ((data_size < INPUT_BUFFER_SIZE) && (g_u8AudioDownloadOver == DLSTATE_DOWNLOADING) && (wait_for_data_cnt < WAIT_FOR_DATA_TIMEOUT_CNT))
					{
						wait_for_data_cnt = 0;
						wait_for_data = true;
						ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, mp3StreamDecoder_no_data, P_DEBUG, "download data not enough");
					}
				}
				if(wait_for_data)
				{
					ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, mp3StreamDecoder_wait_for_data, P_DEBUG, "wait for download data, size:%d, count:%d", data_size, wait_for_data_cnt);
					xEcRingWrite(pcmCache, zero_buff, packet_size);
                    continue;
				}
#else
				if(data_size == 0)
				{
					continue;
				} 
#endif
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
				readLen = readLen;
				#endif
				mad_stream_buffer(&stream, inputBuffer, readSize + remaining);
				// debugSize = readSize + remaining;
				stream.error = MAD_ERROR_NONE;
			}
			if (mad_frame_decode(&frame, &stream))
			{
				if (MAD_RECOVERABLE(stream.error))
				{
					xEcRingWrite(pcmCache, zero_buff, packet_size);
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
	if(zero_buff != NULL)
	{
		free(zero_buff);
		zero_buff = NULL;
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
	// uint16_t curVolume = 0;
	// uint32_t debugSize = 0;
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
	if(medDataHandleStateGet() != MED_DATA_HDL_STA_START)
	{
		pcmCache = medInitDataCache(MP3_DATA_CACHE_SIZE,MP3_DATA_CACHE_PRE_FILL_SIZE);	
		
		dataHandleParam.field.samplerate = audParam->rate;
		dataHandleParam.field.envType = (audParam->toneFlag ? MED_DATA_ENV_TYPE_TONE : MED_DATA_ENV_TYPE_LOCAL);
		dataHandleParam.field.bitWidth = 0;//default 16bit
		medDataHandleStart(&dataHandleParam);
	}
	else
		pcmCache = medGetDataCache();
	mp3DecodeTimer = osTimerNew((osTimerFunc_t)mp3DecoderTimerFunc,osTimerPeriodic,NULL,NULL);
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
				// debugSize = readSize + remaining;
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
#ifdef SPEAKER_APP
				halI2sSrcAdjustVolumn((int16_t *)&sample, sizeof(sample), EC_audioGetVolume());
#endif
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

size_t detect_id3v2_tag(const unsigned char *data, size_t data_size)
{
    if(!data || data_size < 10)
    {
        return -1;
    }
    if(data[0] == 'I' && data[1] == 'D' && data[2] == '3')
    {
        return (((data[6] & 0x7F) << 21) | ((data[7] & 0x7F) << 14) |
               ((data[8] & 0x7F) << 7) | (data[9] & 0x7F)) + 10;
    }
    return 0;
}

static int mp3SampleRateGetStreamByMad(uint32_t *sampleRate, uint8_t *mp3Data, uint32_t mp3DataSize)
{
    uint32_t max_attempts = 3;
    int ret = -1;
    if(sampleRate == NULL || mp3Data == NULL || mp3DataSize == 0)
    {
        return -1;
    }
    struct mad_stream stream;
    struct mad_header header;
    mad_stream_init(&stream);
    mad_header_init(&header);
    mad_stream_buffer(&stream, mp3Data, mp3DataSize);

    int attempts = 0;
    while(attempts < max_attempts)
    {
        int decode_result = mad_header_decode(&header, &stream);
        if(decode_result == 0)
        {
            // 成功解码帧头
            *sampleRate = header.samplerate;
            ret = 0;
            break;
        }
        else
        {
			if(stream.error == MAD_ERROR_BUFLEN)
            {
				*sampleRate = header.samplerate;
                ret = -2;
                break;
            }
            if(stream.next_frame && stream.next_frame > stream.this_frame)
            {
                mad_stream_skip(&stream, 1);
            }
            else
            {
				break;
            }
        }

        attempts++;
    }
    mad_header_finish(&header);
    mad_stream_finish(&stream);
    return ret;
}

static uint8_t mp3SampleRateGetbyMad(char *path)
{
    FILE *file = NULL;
    uint32_t sample = 0;
    struct stat f_stat = {0};
	int ret = 0;
	uint8_t *file_data = NULL;
	uint8_t id3_head[10] = {0};
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    file = file_fopen(path, "r");
    if (file == NULL)
    {
        return 0;
    }
	file_fstat((int)file, &f_stat);
	file_fread(id3_head, 10, sizeof(uint8_t), file);
	uint32_t offset = detect_id3v2_tag(id3_head, 10);
	// 文件每次读取长度为1024,320Kbps以下的mp3文件，一般帧长不会超过1024。
	// 如果mp3文件的帧长超过1024，可能会导致读取到的帧头信息不完整，从而导致信息获取失败。
	uint32_t read_size = (f_stat.st_size - 10) > 1024 ? 1024 : (f_stat.st_size - 10);
	file_data = malloc(read_size);
	if(!file_data)
	{
		file_fclose(file);
		return 0;
	}
	file_fseek(file, 0, SEEK_SET);
	file_fseek(file, offset, SEEK_SET);
	file_fread(file_data, read_size, sizeof(uint8_t), file);
	ret = mp3SampleRateGetStreamByMad(&sample, file_data, read_size);
	uint8_t rate = sampleRateConvert(sample, true);
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, mp3SampleRateGetbyMad, P_WARNING, "size: %d, id3_tag_size: %d, rate [%d][%d] ret is %d",read_size, offset, rate, sample, ret);
	free(file_data);
	file_fclose(file);
#endif
    return rate;
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
    rate = mp3SampleRateGetbyMad(path);
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
		return 0;
	
	uint32_t sample = 0;
	size_t id3_tag_size = detect_id3v2_tag(string, stringLen);
	ret = mp3SampleRateGetStreamByMad(&sample, string + id3_tag_size, stringLen - id3_tag_size);	
	rate = sampleRateConvert(sample, true);

	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, mp3StringPlay, P_WARNING, "size: %d, id3_tag_size: %d, rate [%d][%d] ret is %d",stringLen, id3_tag_size, rate, sample, ret);
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
	static 	uint8_t backupRate = 0;
	uint8_t    rate   = 0;
	EC_ASSERT(gAudStreamRingBuf != NULL,0,0,0);
	uint32_t getStreamCnt = 0;
	uint32_t streamDataSize = 0;
	gMp3PlayStopFlag = false;
	while ((streamDataSize <= AUDIO_MP3_DECODE_FRAME_SIZE*MP3_NETPLAY_BUFFER_PACKET_CNT) && 
		   (getStreamCnt < MP3_NETPLAY_BUFFER_TIMEOUT))
	{
		streamDataSize = xEcRingGetOption(gAudStreamRingBuf, E_LRO_DATA_SIZE);
		getStreamCnt++;
		if(gMp3PlayStopFlag)
		{
			gMp3PlayStopFlag = false;
#if defined(FEATURE_HTTPC_ENABLE) && defined(FEATURE_HTTP_TLS_ENABLE) && \
    defined(FEATURE_SUBSYS_MEDIA_STREAM_ENABLE) &&                       \
    defined(FEATURE_SUBSYS_HTTP_DOWNLOADER_ENABLE)
			http_downloader_stop(gAudioStreamDownloader);
#endif
			ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, mp3StreamPlay_stop1, P_INFO, "mp3StreamPlay: stop play flag\r\n");
			return AV_RET_PLAY_STOP;
		
		}
		osDelay(10);
		//如果文件较小，下载缓存完成则直接开始播放
		if(g_u8AudioDownloadOver == DLSTATE_COMPLETED)
		{
			ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, mp3StreamPlay_dlover, P_INFO, "mp3StreamPlay: download over, streamDataSize: %d\r\n", streamDataSize);
			if(streamDataSize == 0)
			{
				//下载完成后，缓存数据仍为0，需要重新更新buffer size
				continue;
			}
			else 
			{
				break;
			}
			break;
		} else if (g_u8AudioDownloadOver == DLSTATE_ERROR)
		{
			ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, mp3StreamPlay_dlerr, P_INFO, "mp3StreamPlay: download error, terminated\r\n");
			return AV_RET_PLAY_TERMINATE;
		}		
	}
	uint32_t sample = 0;
	size_t id3_tag_size = detect_id3v2_tag(gAudStreamRingBuf->pDataBuff, streamDataSize);
	if(id3_tag_size >= 0)
	{	
		ret = mp3SampleRateGetStreamByMad(&sample, gAudStreamRingBuf->pDataBuff + id3_tag_size, streamDataSize - id3_tag_size);	
		
		rate = (ret == 0) ? sampleRateConvert(sample, true) : 0xFF;
	}

	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, mmp3SteamPlay_2, P_WARNING, "size: %d, id3_tag_size: %d, rate [%d][%d] ret is %d",streamDataSize, id3_tag_size, rate, sample, ret);
	if ((rate >= SAMPLERATE_8K) && (rate <= SAMPLERATE_96K))
	{
		audParam->rate = rate;
		backupRate = rate;
		ret = mp3StreamDecoder(audParam);
	}	
	else
	{
		if ((backupRate >= SAMPLERATE_8K) && (backupRate <= SAMPLERATE_96K))
		{
			audParam->rate = backupRate;
			ret = mp3StreamDecoder(audParam);
		}
		else
			return ret;
	}
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
	gMp3PlayStopFlag = true;
	if(gToneDecodeQueue != NULL)
	{
		queue.endFlag = true;
		osMessageQueuePutToFront(gToneDecodeQueue, &queue, 0, osWaitForever);
	}
	if(!puaseFlag)
		memset(&audioStatusInfo,0x00,sizeof(audioPlayStatusInfo));
	return ret;
}

static int skip_id3v2_tag(FILE *fp)
{
    unsigned char id3_header[10];

    if(file_fread(id3_header, 1, 10, fp) != 10)
    {
        SYSLOG_INFO("skip_id3v2_tag: read less than 10 bytes, no ID3 tag\r\n");
        return 0;
    }

    if(memcmp(id3_header, "ID3", 3) != 0)
    {
        SYSLOG_INFO("skip_id3v2_tag: not ID3 tag, rewinding\r\n");
        file_fseek(fp, 0, SEEK_SET);
        return 0;
    }

    int tag_size = ((id3_header[6] & 0x7F) << 21) |
                   ((id3_header[7] & 0x7F) << 14) |
                   ((id3_header[8] & 0x7F) << 7) | (id3_header[9] & 0x7F);

    SYSLOG_INFO("skip_id3v2_tag: found ID3 tag, size=%d\r\n", tag_size);
    
    // 检查是否有扩展头
    int has_extended_header = (id3_header[5] & 0x40);
    if(has_extended_header)
    {
        unsigned char ext_header[4];
        if(file_fread(ext_header, 1, 4, fp) == 4)
        {
            int ext_size = ((ext_header[0] & 0xFF) << 24) |
                           ((ext_header[1] & 0xFF) << 16) |
                           ((ext_header[2] & 0xFF) << 8) |
                           (ext_header[3] & 0xFF);
            tag_size += ext_size;
            SYSLOG_INFO("skip_id3v2_tag: extended header, total size=%d\r\n", tag_size);
        }
    }

    file_fseek(fp, tag_size, SEEK_CUR);
    SYSLOG_INFO("skip_id3v2_tag: skipped ID3 tag\r\n");
    return 0;
}

/**
 * 获取 MP3 文件的音频参数
 * @param filename MP3 文件路径
 * @param audio_param 输出参数结构体指针
 * @return 成功返回 0，失败返回 -1
 */
int32_t mp3GetInfo(char *filename, AudioInfo_t *audio_param)
{
     FILE *fp = NULL;
    struct mad_stream stream;
    struct mad_header header;
    uint8_t *buffer = NULL;  // 8KB 足够解析第一帧
    int result = -1;
    int retry_count = 0;
    const int max_retry = 20;
    
    // 参数检查
    if(filename == NULL || audio_param == NULL)
    {
        SYSLOG_ERR("mp3GetInfoFast: Invalid parameters\r\n");
        return -1;
    }
    
    SYSLOG_INFO("mp3GetInfoFast: start parsing %s\r\n", filename);
    
    fp = file_fopen(filename, "rb");
    if(!fp)
    {
        SYSLOG_ERR("mp3GetInfoFast: Cannot open file %s\r\n", filename);
        return -1;
    }
    
    // 初始化音频信息
    memset(audio_param, 0, sizeof(AudioInfo_t));
    audio_param->codec = AUDIO_PLAY_CODEC_TYPE_IDLE;
    
    // 跳过 ID3v2 标签
    skip_id3v2_tag(fp);
    buffer = malloc(8 * 1024);
    if(!buffer)
    // 读取足够的数据来解析第一帧（通常 4KB 足够）
	{
		SYSLOG_ERR("mp3GetInfoFast: malloc failed\r\n");
		return -1;
	}	
    size_t bytes_read = file_fread(buffer, 1, 8 * 1024, fp);
    if(bytes_read < 4)
    {
        SYSLOG_ERR("mp3GetInfoFast: File too small\r\n");
        file_fclose(fp);
		free(buffer);
        return -1;
    }
    
    SYSLOG_INFO("mp3GetInfoFast: read %d bytes\r\n", bytes_read);
    
    // 初始化解码器流
    mad_stream_init(&stream);
    mad_stream_buffer(&stream, buffer, bytes_read);
    
    // 尝试解析帧头
    while(retry_count < max_retry)
    {
        if(mad_header_decode(&header, &stream) == -1)
        {
            if(MAD_RECOVERABLE(stream.error))
            {
                SYSLOG_WARNING("mp3GetInfoFast: error: %s, retry %d\r\n", 
                           mad_stream_errorstr(&stream), retry_count);
                retry_count++;
                continue;
            }
            else
            {
                SYSLOG_ERR("mp3GetInfoFast: unrecoverable error: %s\r\n", 
                          mad_stream_errorstr(&stream));
                break;
            }
        }
        
        // 成功获取参数
        audio_param->channel = MAD_NCHANNELS(&header);
        audio_param->sample_rate = header.samplerate;
        audio_param->bit_rate = header.bitrate;
        audio_param->bit_depth = 16;
        audio_param->codec = AUDIO_PLAY_CODEC_TYPE_MP3;
        
        // 估算时长（基于文件大小和码率）
        if(audio_param->bit_rate > 0)
        {
            file_fseek(fp, 0, SEEK_END);
            long file_size = file_ftell(fp);
            audio_param->duration = (uint32_t)((file_size * 8) / audio_param->bit_rate);
            SYSLOG_INFO("mp3GetInfoFast: file_size=%ld, bitrate=%d, duration=%d\r\n",
                       file_size, audio_param->bit_rate, audio_param->duration);
        }
        
        result = 0;
        
        SYSLOG_INFO("mp3GetInfoFast: success - channels=%d, samplerate=%d, bitrate=%d, duration=%d\r\n",
                    audio_param->channel, audio_param->sample_rate, 
                    audio_param->bit_rate, audio_param->duration);
        break;
    }
    
    if(result != 0)
    {
        SYSLOG_ERR("mp3GetInfoFast: failed after %d retries\r\n", max_retry);
    }
    
    mad_stream_finish(&stream);
    file_fclose(fp);
    free(buffer);
    return result;
}