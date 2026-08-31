/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    audio.h
 * Description:  EC718 audio play header file
 * History:      Rev1.0   2018-07-12
 *
 ****************************************************************************/
#ifndef __AUDIO_H__
#define __AUDIO_H__


#include <stdint.h>
#include <stdbool.h>
#include "hal_i2s.h"

typedef enum _EPAT_AudioSrcType
{
    AUDIO_PLAY_IDLE,
    AUDIO_PLAY_FILE    = 1,
    AUDIO_PLAY_STRING,
    AUDIO_PLAY_STREAM,
#ifdef SPEAKER_APP
    AUDIO_PLAY_VOLUME,
#endif
} AudioSrcTypeT;

typedef enum _EPAT_AudioCodecTypeT
{
	AUDIO_PLAY_CODEC_TYPE_IDLE,
	AUDIO_PLAY_CODEC_TYPE_PCM 		= 1,
    AUDIO_PLAY_CODEC_TYPE_MP3,
	AUDIO_PLAY_CODEC_TYPE_AMR,
    AUDIO_PLAY_CODEC_TYPE_WAV,
    AUDIO_PLAY_CODEC_TYPE_AAC,
    AUDIO_PLAY_CODEC_TYPE_G711,
    AUDIO_PLAY_CODEC_TYPE_TTS,
    AUDIO_PLAY_CODEC_TYPE_EOF = 0xff,
}AudioCodecTypeT;

typedef struct AudioInfo_
{
    uint8_t channel;          /**< ??? */
    uint8_t bit_depth;        /**< ?? */
    uint32_t sample_rate;     /**< ??? */
    uint32_t bit_rate;        /**< ?? */
	uint32_t duration;        /**< ?? */
	AudioCodecTypeT codec;
} AudioInfo_t;


#define AUDIO_TX_TRANSFER_SIZE                      (2304)//7000
#define AUDIO_RX_TRANSFER_SIZE                      (8000)

#define ALARM_TONE_NEED_TTS							(0)
#define ALARM_SOUND_CODEC							AUDIO_PLAY_CODEC_TYPE_MP3
#define ALARM_SOUND                                "D:/in.mp3"
#define WELCOME_SOUND                               "D:/Welcome.mp3"
#define POWER_SOUND_OFF                             "D:/PowerOff.mp3"
#define SIM_SOUND_UNREADY                           "D:/SimUnready.mp3"
#define NW_SOUND_READY                              "D:/NwReady.mp3"
#define NW_SOUND_UNREADY                            "D:/NwUnready.mp3"
#define SERVER_SOUND_READY                          "D:/ServerReady.mp3"
#define SERVER_SOUND_UNREADY                        "D:/ServerUnready.mp3"
#define VOLUME_SOUND_MIN                            "D:/VolumeMin.mp3"
#define VOLUME_SOUND_MAX                            "D:/VolumeMax.mp3"
#define VOLUME_SOUND_MINUS                          "D:/VolumeMinus.mp3"
#define VOLUME_SOUND_PLUS                           "D:/VolumePlus.mp3"
#define BATTRY_SOUND_100                            "D:/Battery100.mp3"
#define BATTRY_SOUND_75                             "D:/Battery75.mp3"
#define BATTRY_SOUND_50                             "D:/Battery50.mp3"
#define BATTRY_SOUND_25                             "D:/Battery25.mp3"
#define BATTRY_SOUND_LOW                            "D:/BatteryLow.mp3"
#define CHARGE_SOUND_BEGIN                          "D:/ChargeBegin.mp3"
#define CHARGE_SOUND_END                            "D:/ChargeEnd.mp3"
#define VOLUME_PLUS_VPRESS_SHORT                    "+"
#define VOLUME_PLUS_VPRESS_LONG                     "++"
#define VOLUME_MINUS_VPRESS_SHORT                   "-"
#define VOLUME_MINUS_VPRESS_LONG                    "--"


typedef void (*AudioCallbackT)(int32_t result);
typedef void (*ReadFileCallbackT)(void);


typedef enum DlState_
{
	DLSTATE_NOT_STARTED = -1,
	DLSTATE_DOWNLOADING = 0,
	DLSTATE_COMPLETED = 1,
	DLSTATE_ERROR = 2,
} DlState_e;

typedef struct
{    
    char           *path;
    ReadFileCallbackT callback;
} QueueReadFileT;

typedef struct
{    
    uint8_t           *string;
	int				  length;
    ReadFileCallbackT callback;
} QueueReadStringT;

typedef struct _audioParamT
{
	BOOL  toneFlag;
	UINT8 aud_volume;
	UINT8 rate;
	UINT8 BitWidth;
	I2sChannelSel_e channel;
	bool	increase;
	bool	dual;	
}audioParamT;

#define AUDIO_PLAY_PCM	(AUDIO_PLAY_FILE << 4 | AUDIO_PLAY_CODEC_TYPE_PCM)
#define AUDIO_PLAY_MP3	(AUDIO_PLAY_FILE << 4 | AUDIO_PLAY_CODEC_TYPE_MP3) 
#define AUDIO_PLAY_AMR 	(AUDIO_PLAY_FILE << 4 | AUDIO_PLAY_CODEC_TYPE_AMR) 
#define AUDIO_PLAY_WAV 	(AUDIO_PLAY_FILE << 4 | AUDIO_PLAY_CODEC_TYPE_WAV) 
#define AUDIO_PLAY_AAC 	(AUDIO_PLAY_FILE << 4 | AUDIO_PLAY_CODEC_TYPE_AAC)
#define AUDIO_PLAY_G711 (AUDIO_PLAY_FILE << 4 | AUDIO_PLAY_CODEC_TYPE_G711)


#define AUDIO_PLAY_STRING_PCM	(AUDIO_PLAY_STRING << 4 | AUDIO_PLAY_CODEC_TYPE_PCM)
#define AUDIO_PLAY_STRING_MP3	(AUDIO_PLAY_STRING << 4 | AUDIO_PLAY_CODEC_TYPE_MP3) 
#define AUDIO_PLAY_STRING_AMR	(AUDIO_PLAY_STRING << 4 | AUDIO_PLAY_CODEC_TYPE_AMR) 
#define AUDIO_PLAY_STRING_WAV	(AUDIO_PLAY_STRING << 4 | AUDIO_PLAY_CODEC_TYPE_WAV) 
#define AUDIO_PLAY_STRING_AAC	(AUDIO_PLAY_STRING << 4 | AUDIO_PLAY_CODEC_TYPE_AAC)
#define AUDIO_PLAY_STRING_G711	(AUDIO_PLAY_STRING << 4 | AUDIO_PLAY_CODEC_TYPE_G711)

#define AUDIO_PLAY_STREAM_PCM 	(AUDIO_PLAY_STREAM << 4 | AUDIO_PLAY_CODEC_TYPE_PCM)
#define AUDIO_PLAY_STREAM_MP3 	(AUDIO_PLAY_STREAM << 4 | AUDIO_PLAY_CODEC_TYPE_MP3) 
#define AUDIO_PLAY_STREAM_AMR 	(AUDIO_PLAY_STREAM << 4 | AUDIO_PLAY_CODEC_TYPE_AMR) 
#define AUDIO_PLAY_STREAM_WAV 	(AUDIO_PLAY_STREAM << 4 | AUDIO_PLAY_CODEC_TYPE_WAV) 
#define AUDIO_PLAY_STREAM_AAC 	(AUDIO_PLAY_STREAM << 4 | AUDIO_PLAY_CODEC_TYPE_AAC)
#define AUDIO_PLAY_STREAM_G711 	(AUDIO_PLAY_STREAM << 4 | AUDIO_PLAY_CODEC_TYPE_G711)

#define AUDIO_PLAY_TTS (AUDIO_PLAY_STRING << 4 | AUDIO_PLAY_CODEC_TYPE_TTS)

#ifdef SPEAKER_APP
#define AUDIO_PLAY_VOLUME_MP3	(AUDIO_PLAY_VOLUME << 4 | AUDIO_PLAY_CODEC_TYPE_MP3)
#endif

#define AUD_CODEC_SIG_TIMEROUT_MAX			(3) /*3ms*/
#define AUD_STATUS_SEM_WAIT_TIMEROUT		(20) /*3ms*/

typedef struct
{   
	bool endFlag;

#ifdef MBTK_OPENCPU_SUPPORT
	BOOL continue_play;
	uint32_t length;
	char *play_buff;
#endif

} QueueTonePlayT;


typedef enum
{
	EC_RET_OK,
	EC_RET_ERR_NOMAL 		= -1,///< Unspecified RTOS error: run-time error but no other error message fits.
	EC_RET_ERR_TIMEOUT 		= -2,///< Operation not completed within the timeout period.
	EC_RET_ERR_RESOURCE 	= -3,///< Resource not available.
	EC_RET_ERR_PARAM 		= -4,///< Parameter error.
	EC_RET_ERR_PARAM_NOMEM 	= -5,///< System is out of memory: it was impossible to allocate or reserve memory for the operation.
	EC_RET_ERR_ISR 			= -6,///< Not allowed in ISR context: the function cannot be called from interrupt service routines.
}errCodeT;

typedef enum _avPlayResp{
	AV_RET_PLAY_ERROR		= -1,
	AV_RET_PLAY_START		= 0 ,
	AV_RET_PLAY_STOP,
	AV_RET_PLAY_EOF,
	AV_RET_PLAY_TERMINATE,
}avPlayResp_E;

typedef struct
{
    UINT8      		audType;
	UINT8			toneFlag;
    audioParamT	    audioParam;
    uint32_t        length;
    char           *buffer;
	UINT8          *audString;
    AudioCallbackT callback;
} QueueAudioT;


typedef enum
{
    ACTION_VOLUME_PLUS_SHORT  = 0,
    ACTION_VOLUME_PLUS_LONG   = 1,
    ACTION_VOLUME_MINUS_SHORT = 2,
    ACTION_VOLUME_MINUS_LONG  = 3,
    ACTION_MENU_SHORT         = 4,
    ACTION_MENU_LONG          = 5,
    ACTION_INVALID
} ActionT;

typedef struct _audioPlayStatusInfo{
	char *path;
	uint32_t resumeIndex;
	uint32_t fileSize;
	uint32_t decodeSize;
}audioPlayStatusInfo;

typedef struct _medSrcT {
	uint32_t srcLen;
	uint8_t src[0];
}medSrcT;

medSrcT 	*medFileToString(char *path);

uint32_t 	sampleRateConvert(uint32_t rate, bool toEnum);

INT32   	EC_audioStartPlayFile(char *path,uint8_t audType,audioParamT audParam, 
				AudioCallbackT play_cb_func);

INT32 	 	EC_audioStartPlayStream(UINT8 *url,UINT32 strLen,uint8_t audType,audioParamT audParam, 
				AudioCallbackT play_cb_func);

INT32	 	EC_audioStartPlayString(UINT8 *String,UINT32 strLen,uint8_t type,
					audioParamT audParam, AudioCallbackT play_cb_func);

INT32		EC_audioStopPlaying(uint8_t audType,bool pauseFlag);
bool		EC_audioPlayChkLoop(void);
void		EC_audioPlaySetLoop(bool onoff);

bool     	EC_audioIsBusy(void);
UINT16   	EC_audioGetVolume(void);

void 		audioToneSrcSet(const char *src);
char * 		audioToneSrcGet(void);
void 		audioToneVolumeSet(uint16_t value);
uint16_t 	audioToneVolumeGet(void);


void 		audioPlayMp3(char *src, AudioCallbackT callback, bool increase, bool dual);

void 		audioPlayAmr(char *src, AudioCallbackT callback);

void 		audioStopPlay(void);

uint32_t 	audioPlayTimeGet(void);

int32_t  	audioInit(void);

#ifdef FEATURE_SUBSYS_TTS_ENABLE
void audioPlayTts(char *text, AudioCallbackT callback);
#endif
bool audioIsReady(void);
bool audioIsBusy(void);

#ifdef SPEAKER_APP
void audioAdjustVolume(uint8_t action);
#ifdef FEATURE_SUBSYS_OPENPLAYER_API_ENABLE
#include "openplayer.h"
extern openPlayer gOpenPlayer;
#endif
#endif

#endif
