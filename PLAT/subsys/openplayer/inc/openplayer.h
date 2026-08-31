#ifndef __OPEN_PLAY_API_H__
#define __OPEN_PLAY_API_H__

/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/
typedef void * openPlayer;
typedef void (*openPlayerCpltCallbackT)(void *userdata,int32_t result);

typedef union _openPlayerConfigT{
	uint8_t config[7];
	struct  {
		uint32_t channel:		2; ///< play channel mono or dual
		uint32_t increase:	2; ///< re-sample,now only support the play schedule without CCIO
		uint32_t bitRate:      2; ///bitRate,used by pcm play,0--->default 16bit,1--->8bit
		uint32_t codec:	    8; ///< the codec to be used,reserved 
		uint32_t store:     2; ///reserved		
		uint32_t srcLen;
		uint8_t sampleRate;
		}playParam;
}openPlayerConfigT;

typedef enum
{
	OPEN_RET_OK,
	OPEN_RET_ERR_NOMAL 			= -1,///< Unspecified RTOS error: run-time error but no other error message fits.
	OPEN_RET_ERR_TIMEOUT 		= -2,///< Operation not completed within the timeout period.
	OPEN_RET_ERR_RESOURCE 		= -3,///< Resource not available.
	OPEN_RET_ERR_PARAM 			= -4,///< Parameter error.
	OPEN_RET_ERR_PARAM_NOMEM 	= -5,///< System is out of memory: it was impossible to allocate or reserve memory for the operation.
	OPEN_RET_ERR_ISR 			= -6,///< Not allowed in ISR context: the function cannot be called from interrupt service routines.
	OPEN_RET_ERR_BUSY			= -7,
}openPlayErrCodeT;

typedef enum {
	OPEN_PLAY_STA_IDLE,				///<< init or idle status
	OPEN_PLAY_STA_PLAYING,			///<< playing status
	OPEN_PLAY_STA_PAUSE,			///<< pause status
	OPEN_PLAY_STA_STOP,				///<< stop status
	OPEN_PLAY_STA_EOF,				//EOF
}openPlayerPlayStatus;

/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/
/**
 \brief    create media player handler
 \param[in] open play param,if NULL,use the default param
 \return	media player handler or NULL
*/
 openPlayer openPlayCreate(openPlayerConfigT *param);

/**
 \brief    destory media player handler
 \param[in] media player handler
 \return	void
*/
void openPlayDestory(openPlayer player);

/**
 \brief    set media player completed callback
 \param[in] media player handler
 \param[in] media player completed callback
 \param[in] userdata
 \return	void
*/
void openPlaySetCallback(openPlayer player,openPlayerCpltCallbackT callback,void * userdata);

 /**
  \brief    media play API
  \param[in] media player handler
  \param[in] src   	    the media play src,it can be path,filename or url 
  \return    0-----success, < 0-----fail
 */
int8_t openPlay(openPlayer player,void *src);

/**
 \brief    media pause play API
 \param[in] media player handler
 \param[in] onoff, set pause or not
 \return	void
*/
void openPlayPause(openPlayer player, bool onoff);

/**
 \brief    media stop play API
 \param[in] media player handler
 \return	void
*/
void openPlayStop(openPlayer player);

/**
 \brief    media seek play API
 \param[in] media player handler
 \param[in] seekPercent the seek percentage range <0-99>
 \return
*/
void openPlaySeek(openPlayer player, uint32_t seekPercent);

/**
 \brief    media play resume  API
 \param[in] media player handler
 \return
*/
int8_t openPlayResume(openPlayer player);

/**
 \brief    media set  volume API
 \param[in] media player handler
 \param[in] volume    the media volume to be set
 \return 0-----success, < 0-----fail
*/
int8_t openPlaySetVolume(openPlayer player, uint16_t volume);

/**
 \brief    media get  volume API
 \param[in] media player handler
 \return current volume
*/
uint16_t openPlayGetVolume(openPlayer player);

/**
 \brief    media set mute API
 \param[in] media player handler
 \param[in] set mute or note
 \return 0-----success, < 0-----fail
*/
void openPlayMute(openPlayer player, bool onoff);

/**
 \brief media get paly time API
 \param[in] media player handler
 \return 0-----success, < 0-----fail
*/
uint32_t openPlayGetTime(openPlayer player);

/**
 \brief    media play loop set API
 \param[in] media player handler
 \param[in] loop    set play loop or not
 \return void
*/
void openPlayLoop(openPlayer player, bool onoff);

/**
 \brief    media play fast forward set API
 \param[in] media player handler
 \param[in] speed fast forward speed
 \return void
*/
void openPlayerFastForward(openPlayer player, float speed);

/**
 \brief    media play fast rewind set API
 \param[in] media player handler
 \param[in] speed fast rewind speed
 \return void
*/
void openPlayerFastRewind(openPlayer player,float speed);

/**
 \brief    media play get status API
 \param[in] media player handler
 \return openPlayerCurStatus
*/
openPlayerPlayStatus openPlayerGetStatus(openPlayer player);

/**
 \brief    media play get loop status API
 \param[in] media player handler
 \return true ---loop,false ---single play
*/
bool openPlayIsLoop(openPlayer player);

/**
 \brief    media play get mute status API
 \param[in] media player handler
 \return true ---mute,false ---unmuted
*/
bool openPlayIsMute(openPlayer player);

/**
 \brief    media play get pause status API
 \param[in] media player handler
 \return true ---pause,false ---unpaused
*/
bool openPlayIsPause(openPlayer player);

#endif
