/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    openplayer.c
 * Description:  EC718 media play source file
 * History:      Rev1.0   2024-03-13
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include DEBUG_LOG_HEADER_FILE
#include <string.h>
#include "cmsis_os2.h"
#include "openplayer.h"
#include "media.h"

/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/
typedef struct _openPlayerCheckSta{
	bool isLoop;
	bool isMute;
	bool isPause;
	bool duration;
	openPlayerPlayStatus playStat;
}openPlayerCheckStaT;

typedef struct _openPlayerHandlerT{
	void *src;
	void *userdata;
	openPlayerConfigT config;
	openPlayerCheckStaT status;
	osSemaphoreId_t openPlayerSemaphore;
	openPlayerCpltCallbackT playCpltcb;
}openPlayerHandlerT;


typedef struct _openPlayerAudioTypeT{
	const char *suffix;
	uint8_t audType;
}openPlayerAudioTypeT;
/*----------------------------------------------------------------------------*
 *                      GLOBAL VARIABLES                                      *
 *----------------------------------------------------------------------------*/

openPlayerCheckStaT gOpenPLayerCheckSta = {0};
static openPlayerHandlerT *gOpenPlayerHandler = NULL;
openPlayerAudioTypeT openPlayerAudTypeCkList[] = 
{
	{".pcm", AUDIO_PLAY_CODEC_TYPE_PCM},
	{".mp3", AUDIO_PLAY_CODEC_TYPE_MP3},
	{".amr", AUDIO_PLAY_CODEC_TYPE_AMR},
	{".wav", AUDIO_PLAY_CODEC_TYPE_WAV},
	{".aac", AUDIO_PLAY_CODEC_TYPE_AAC},
	{".g711a", AUDIO_PLAY_CODEC_TYPE_G711},
	{NULL, AUDIO_PLAY_CODEC_TYPE_IDLE},
};

/*----------------------------------------------------------------------------*
 *                      PRIVATE FUNCTION DECLEARATION                         *
 *----------------------------------------------------------------------------*/
 static void openPlayerInterCallback(int32_t result)
{
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, openPlayerInterCallback, P_INFO, "[openPlayer] internal callback,result [%d]",result);
	if((result == AV_RET_PLAY_STOP) && (gOpenPlayerHandler->status.isPause))
	{
		gOpenPlayerHandler->status.playStat = OPEN_PLAY_STA_PAUSE;
	}
	else
	{
		gOpenPlayerHandler->status.playStat = OPEN_PLAY_STA_IDLE;
	}
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, openPlayerInterCallback_exit, P_INFO, "openplayer state: %d", gOpenPlayerHandler->status.playStat);
	if(gOpenPlayerHandler)
	{
		if(gOpenPlayerHandler->playCpltcb != NULL)
			gOpenPlayerHandler->playCpltcb(gOpenPlayerHandler->userdata,result);
	}
}
#if 0
static openPlayErrCodeT openPlayerLock(void* arg)
{
	EC_ASSERT(arg != NULL,0,0,0);
    osSemaphoreId_t openPlayerLock = (osSemaphoreId_t)arg;

    if (osSemaphoreAcquire(openPlayerLock, 1000) != osOK)
    {
        return OPEN_RET_ERR_TIMEOUT;
    }
    return  OPEN_RET_OK;
}

static openPlayErrCodeT openPlayeUnlock(void* arg)
{
	EC_ASSERT(arg != NULL,0,0,0);
    osSemaphoreId_t openPlayerLock = (osSemaphoreId_t)arg;

    if (osSemaphoreRelease(openPlayerLock) != osOK)
    {
        return OPEN_RET_ERR_TIMEOUT;
    }
    return  OPEN_RET_OK;
}
#endif

static openPlayErrCodeT openPlayeLockDestroy(void *arg)
{
	EC_ASSERT(arg != NULL,0,0,0);
    osSemaphoreId_t codecLock = (osSemaphoreId_t)arg;

    if (osSemaphoreDelete(codecLock) != osOK)
    {
        return OPEN_RET_ERR_TIMEOUT;
    }
    return  OPEN_RET_OK;
}

uint8_t openPlayerCheckSrcTypeFunc(const char *src)
{
	if((src == NULL) || (strlen(src) == 0))
		return AUDIO_PLAY_CODEC_TYPE_IDLE;
	
	uint8_t cnt = 0;
	uint32_t srcLen = 0;
	uint8_t  suffixLen = 0;
	uint8_t srcType = AUDIO_PLAY_CODEC_TYPE_IDLE;
	srcLen = strlen(src);
			
	for(cnt = 0; cnt < sizeof(openPlayerAudTypeCkList) / sizeof(openPlayerAudioTypeT); cnt++)
	{
		suffixLen = strlen(openPlayerAudTypeCkList[cnt].suffix);
		if((srcLen >= suffixLen) && !strncasecmp(src + srcLen - suffixLen, openPlayerAudTypeCkList[cnt].suffix,suffixLen))
		{
			srcType = openPlayerAudTypeCkList[cnt].audType;
			break;
		}
	}
	return srcType;
}


/*----------------------------------------------------------------------------*
 *                      GLOBAL FUNCTIONS                                      *
 *----------------------------------------------------------------------------*/

/**
 \brief    create media player handler
 \param[in] open play param,if NULL,use the default param
 \return	media player handler or NULL
*/
 openPlayer openPlayCreate(openPlayerConfigT *param)
{
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, openPlayCreate, P_INFO, "[openPlay] player Create");
	if(gOpenPlayerHandler != NULL)
	{
		goto INIT_PARAM;
	}
	else
	{
		gOpenPlayerHandler = (openPlayerHandlerT *)malloc(sizeof(openPlayerHandlerT));
		if(gOpenPlayerHandler == NULL)
		{
			goto INIT_EOF;
		}
	
	}

	memset(&gOpenPlayerHandler->config,0x00,sizeof(openPlayerConfigT));
	memset(&gOpenPlayerHandler->status,0x00,sizeof(openPlayerCheckStaT));
	gOpenPlayerHandler->src = NULL;
	gOpenPlayerHandler->playCpltcb = NULL;
	gOpenPlayerHandler->userdata = NULL;
	gOpenPlayerHandler->openPlayerSemaphore = osSemaphoreNew(1U,0,NULL);
INIT_PARAM:
	if(param != NULL)
		memcpy(&gOpenPlayerHandler->config, param,sizeof(openPlayerConfigT));
INIT_EOF:
	return (openPlayer)gOpenPlayerHandler;
}

/**
 \brief    destory media player handler
 \param[in] media player handler
 \return	void
*/
void openPlayDestory(openPlayer player)
{
	if(player == NULL)
		return;
	openPlayerHandlerT *handler = (openPlayerHandlerT *)player;
	openPlayeLockDestroy(handler->openPlayerSemaphore);
	handler->openPlayerSemaphore = NULL;
	handler->playCpltcb = NULL;
	handler->userdata = NULL;
	handler->src = NULL;

	free(handler);
	handler = NULL;
	return;	
	
}

/**
 \brief    set media player completed callback
 \param[in] media player handler
 \param[in] media player completed callback
 \param[in] userdate
 \return	void
*/
void openPlaySetCallback(openPlayer player,openPlayerCpltCallbackT callback,void * userdata)
{
	if(player == NULL)
		return;
	openPlayerHandlerT *handler = (openPlayerHandlerT *)player;
	handler->playCpltcb = callback;
	handler->userdata = userdata;
}

 /**
  \brief    media play API
  \param[in] media player handler
  \param[in] src   	    the media play src,it can be path,filename or url 
  \return    0-----success, < 0-----fail
 */
int8_t openPlay(openPlayer player,void *src)
{
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, openPlay_1, P_INFO, "[openPlay] player enter");
	int8_t ret = OPEN_RET_ERR_NOMAL;
	if(player == NULL)
		return OPEN_RET_ERR_PARAM;
	uint16_t audType = AUDIO_PLAY_CODEC_TYPE_IDLE;
	openPlayerHandlerT *handler = (openPlayerHandlerT *)player;
	if(handler->status.playStat == OPEN_PLAY_STA_PLAYING)
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, openPlay_err1, P_INFO, "[openPlay] player is playing");
		return OPEN_RET_ERR_BUSY;
	}
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, openPlay_2, P_INFO, "[openPlay] sto :%e<AudioSrcTypeT>,codec: %e<AudioCodecTypeT>, state: %d",
		handler->config.playParam.store,handler->config.playParam.codec, handler->status.playStat);
	switch(handler->config.playParam.store)
	{

		case AUDIO_PLAY_IDLE:
		case AUDIO_PLAY_FILE:
		{
			uint8_t type = 0;
			audioParamT audParam = {0};
			if(src == NULL)
			{
				handler->config.playParam.codec = AUDIO_PLAY_CODEC_TYPE_IDLE;
				ret = OPEN_RET_ERR_PARAM;
				goto PLAY_EOF;
			}
			handler->src = src;
			//check codec
			audType = openPlayerCheckSrcTypeFunc(handler->src);
			
			if(audType == AUDIO_PLAY_CODEC_TYPE_IDLE)
			{
				if((handler->config.playParam.codec <= AUDIO_PLAY_CODEC_TYPE_IDLE) || 
				(handler->config.playParam.codec >=  AUDIO_PLAY_CODEC_TYPE_EOF))
				{
					handler->config.playParam.codec = AUDIO_PLAY_CODEC_TYPE_IDLE;
					handler->src = NULL;
					ret = OPEN_RET_ERR_PARAM;
					goto PLAY_EOF;
				}				
			}
			else
			{
				handler->config.playParam.codec = audType;
			}
			handler->status.playStat = OPEN_PLAY_STA_PLAYING;
			ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, openPlay_3, P_INFO, "[openPlay] file aud type: %e<AudioCodecTypeT>",audType);
			memset(&audParam,0x00,sizeof(audioParamT));
			audParam.rate = handler->config.playParam.sampleRate;
			type = ((handler->config.playParam.store << 4) | handler->config.playParam.codec);
			EC_audioStartPlayFile(handler->src, type, audParam, openPlayerInterCallback);
			handler->status.isPause = false;
			ret = OPEN_RET_OK;
		}
		break;
		case AUDIO_PLAY_STRING:
		{
			uint8_t type = 0;
			audioParamT audParam = {0};
			if(src == NULL)
			{
				handler->config.playParam.codec = AUDIO_PLAY_CODEC_TYPE_IDLE;
				ret = OPEN_RET_ERR_PARAM;
				goto PLAY_EOF;
			}
			handler->src = src;

			memset(&audParam,0x00,sizeof(audioParamT));
			if((handler->config.playParam.codec < AUDIO_PLAY_CODEC_TYPE_IDLE) || 
				(handler->config.playParam.codec >= AUDIO_PLAY_CODEC_TYPE_EOF))
			{
				handler->config.playParam.codec = AUDIO_PLAY_CODEC_TYPE_IDLE;
				ret = OPEN_RET_ERR_PARAM;
				goto PLAY_EOF;
			}
			handler->status.playStat = OPEN_PLAY_STA_PLAYING;
			ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, openPlay_4, P_INFO, "[openPlay] string aud type: %e<AudioCodecTypeT>",handler->config.playParam.codec);
			audParam.rate = handler->config.playParam.sampleRate;
			type = ((handler->config.playParam.store << 4) | handler->config.playParam.codec);
			EC_audioStartPlayString(handler->src,handler->config.playParam.srcLen, type, audParam, openPlayerInterCallback);
			handler->status.isPause = false;
			ret = OPEN_RET_OK;
		}
		break;
		case AUDIO_PLAY_STREAM:
		{
			uint8_t type = 0;
			audioParamT audParam = {0};
			handler->src = src;
			//check codec
			audType = openPlayerCheckSrcTypeFunc(handler->src);

			if(audType == AUDIO_PLAY_CODEC_TYPE_IDLE)
			{
				if((handler->config.playParam.codec <= AUDIO_PLAY_CODEC_TYPE_IDLE) || 
				(handler->config.playParam.codec >=  AUDIO_PLAY_CODEC_TYPE_EOF))
				{
					handler->config.playParam.codec = AUDIO_PLAY_CODEC_TYPE_IDLE;

					handler->src = NULL;
					ret = OPEN_RET_ERR_PARAM;
					goto PLAY_EOF;
				}				
			}
			else
			{
				handler->config.playParam.codec = audType;
			}
			handler->status.playStat = OPEN_PLAY_STA_PLAYING;
			ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, openPlay_5, P_INFO, "[openPlay] stream aud type: %e<AudioCodecTypeT>",audType);
			memset(&audParam,0x00,sizeof(audioParamT));
			audParam.rate = handler->config.playParam.sampleRate;
			type = ((handler->config.playParam.store << 4) | handler->config.playParam.codec);
			EC_audioStartPlayStream(handler->src,handler->config.playParam.srcLen, type, audParam, openPlayerInterCallback);
			handler->status.isPause = false;
			ret = OPEN_RET_OK;
		}
		break;	
		default:
		break;
	}
	
PLAY_EOF:
	return ret;
}

/**
 \brief    media pause play API
 \param[in] media player handler
 \param[in] onoff, set pause or not
 \return	void
*/
void openPlayPause(openPlayer player, bool onoff)
{
	// uint8_t type = 0;
	// audioParamT audParam = {0};
	if(player == NULL)
		return;
	bool pauseConfig = false;
	openPlayerHandlerT *handler = (openPlayerHandlerT *)player;
	pauseConfig = onoff;
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, openPlayPause, P_INFO, "[openPlay] pause enter[%d],codec[%d] isPause[%d]",onoff, handler->config.playParam.codec,handler->status.isPause);
	if(pauseConfig != handler->status.isPause)
	{
		if(pauseConfig == true)
		{
			EC_audioStopPlaying(handler->config.playParam.codec,pauseConfig);
			//handler->status.playStat = OPEN_PLAY_STA_PAUSE;
		}
		else
		{
			openPlay(handler, handler->src);
		}
	}
	handler->status.isPause = pauseConfig;
}

/**
 \brief    media stop play API
 \param[in] media player handler
 \return	void
*/
void openPlayStop(openPlayer player)
{
	if(player == NULL)
		return;
	openPlayerHandlerT *handler = (openPlayerHandlerT *)player;
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, openPlayStop, P_INFO, "[openPlay] play stop[state: %d]", handler->status.playStat);
	if(handler->status.playStat >= OPEN_PLAY_STA_PLAYING)
	{
		EC_audioStopPlaying(handler->config.playParam.codec,FALSE);
		if(handler->status.playStat == OPEN_PLAY_STA_PAUSE)
		{
			handler->status.isPause = false;
			openPlayerInterCallback(AV_RET_PLAY_STOP);
		}
		memset(&handler->config,0x00,sizeof(openPlayerConfigT));
		//memset(&handler->status,0x00,sizeof(openPlayerCheckStaT));
		handler->src = NULL;
	}	
}


/**
 \brief    media seek play API
 \param[in] media player handler
 \param[in] seekPercent the seek percentage range <0-99>
 \return
*/
void openPlaySeek(openPlayer player, uint32_t seekPercent)
{
	return;
}

/**
 \brief    media play resume  API
 \param[in] media player handler
 \return
*/
int8_t openPlayResume(openPlayer player)
{
	//TODO
	return OPEN_RET_OK;
}

/**
 \brief    media set  volume API
 \param[in] media player handler
 \param[in] volume    the media volume to be set
 \return 0-----success, < 0-----fail
*/
int8_t openPlaySetVolume(openPlayer player, uint16_t volume)
{
	//TODO
	return OPEN_RET_OK;
}

/**
 \brief    media get  volume API
 \param[in] media player handler
 \return current volume
*/
uint16_t openPlayGetVolume(openPlayer player)
{
	//TODO
	return OPEN_RET_OK;
}

/**
 \brief    media set mute API
 \param[in] media player handler
 \param[in] set mute or note
 \return 0-----success, < 0-----fail
*/
void openPlayMute(openPlayer player, bool onoff)
{
	//TODO
	return;
}

/**
 \brief media get paly time API
 \param[in] media player handler
 \return 0-----success, < 0-----fail
*/
uint32_t openPlayGetTime(openPlayer player)
{
	if(player == NULL)
		return 0;
	uint32_t playTime = 0;
	openPlayerHandlerT *handler = (openPlayerHandlerT *)player;
	switch(handler->config.playParam.codec)
	{
		case AUDIO_PLAY_CODEC_TYPE_MP3:
			playTime = audioPlayTimeGet();
		break;
		case AUDIO_PLAY_CODEC_TYPE_AMR:
		break;
		case AUDIO_PLAY_CODEC_TYPE_AAC:
		break;
		case AUDIO_PLAY_CODEC_TYPE_PCM:
		break;
		default:
		break;
	}
	return playTime;
}

/**
 \brief    media play loop set API
 \param[in] media player handler
 \param[in] loop    set play loop or not
 \return void
*/
void openPlayLoop(openPlayer player, bool onoff)
{
	/*TODO*/
	openPlayerHandlerT *handler = (openPlayerHandlerT *)player;
	if(handler)
		handler->status.isLoop = onoff;
	EC_audioPlaySetLoop(onoff);
	return;
}

/**
 \brief    media play fast forward set API
 \param[in] media player handler
 \param[in] speed fast forward speed
 \return void
*/
void openPlayerFastForward(openPlayer player, float speed)
{
	/*TODO*/
	return;
}

/**
 \brief    media play fast rewind set API
 \param[in] media player handler
 \param[in] speed fast rewind speed
 \return void
*/
void openPlayerFastRewind(openPlayer player,float speed)
{
	/*TODO*/
	return;
}

/**
 \brief    media play get status API
 \param[in] media player handler
 \return openPlayerStatus
*/
openPlayerPlayStatus openPlayerGetStatus(openPlayer player)
{
	if(player == NULL)
		return OPEN_PLAY_STA_IDLE;	
	openPlayerHandlerT *handler = (openPlayerHandlerT *)player;
	return handler->status.playStat;
}

/**
 \brief    media play get loop status API
 \param[in] media player handler
 \return true ---loop,false ---single play
*/
bool openPlayIsLoop(openPlayer player)
{
	if(player == NULL)
		return FALSE;	
	openPlayerHandlerT *handler = (openPlayerHandlerT *)player;
	handler->status.isLoop = EC_audioPlayChkLoop();
	return handler->status.isLoop;
}

/**
 \brief    media play get mute status API
 \param[in] media player handler
 \return true ---mute,false ---unmuted
*/
bool openPlayIsMute(openPlayer player)
{
	if(player == NULL)
		return FALSE;	
	openPlayerHandlerT *handler = (openPlayerHandlerT *)player;
	return handler->status.isMute;
}

/**
 \brief    media play get pause status API
 \param[in] media player handler
 \return true ---pause,false ---unpaused
*/
bool openPlayIsPause(openPlayer player)
{
	if(player == NULL)
		return FALSE;	
	openPlayerHandlerT *handler = (openPlayerHandlerT *)player;
	return handler->status.isPause;
}
