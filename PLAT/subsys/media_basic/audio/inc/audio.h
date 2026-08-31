/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    audio.h
 * Description:  EC618 mqtt demo entry header file
 * History:      Rev1.0   2018-07-12
 *
 ****************************************************************************/
#ifndef __AUDIO_H__
#define __AUDIO_H__


#include <stdint.h>
#include <stdbool.h>
#include "api_codec.h"
#include "openplayer.h"
#ifdef FEATURE_SUBSYS_MP3_ENABLE
#include "mp3.h"
#endif
#ifdef FEATURE_SUBSYS_TTS_ENABLE
#include "tts.h"
#endif
#ifdef FEATURE_SUBSYS_WAV_ENABLE
#include "wav.h"
#endif
#ifdef FEATURE_SUBSYS_AMR_ENABLE
#include "amr.h"
#endif
#ifdef FEATURE_SUBSYS_PCM_ENABLE
#include "pcm.h"
#endif
#ifdef FEATURE_SUBSYS_RECORD_ENABLE
#include "record.h"
#endif


#ifdef SPI_CODEC_ENABLE
#define AUDIO_TX_TRANSFER_SIZE                      640
#else
#define AUDIO_TX_TRANSFER_SIZE                      8000
#endif
#define AUDIO_RX_TRANSFER_SIZE                      8000
#ifdef EXTERNAL_TTS_LFS_ENABLE
#if defined(FEATURE_SUBSYS_MP3_ENABLE)
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
#elif defined(FEATURE_SUBSYS_TTS_ENABLE)
#define WELCOME_SOUND                               "欢迎使用移芯收款播报设备"
#define POWER_SOUND_OFF                             "正在关机"
#define SIM_SOUND_UNREADY                           "无SIM卡"
#define NW_SOUND_READY                              "网络连接成功"
#define NW_SOUND_UNREADY                            "网络连接失败"
#define SERVER_SOUND_READY                          "服务器连接成功"
#define SERVER_SOUND_UNREADY                        "服务器连接失败"
#define VOLUME_SOUND_MIN                            "音量最小"
#define VOLUME_SOUND_MAX                            "音量最大"
#define VOLUME_SOUND_MINUS                          "音量减小"
#define VOLUME_SOUND_PLUS                           "音量加大"
#define BATTRY_SOUND_100                            "电量百分之一百"
#define BATTRY_SOUND_75                             "电量百分之七十五"
#define BATTRY_SOUND_50                             "电量百分之五十"
#define BATTRY_SOUND_25                             "电量百分之二十五"
#define BATTRY_SOUND_LOW                            "电量过低"
#define CHARGE_SOUND_BEGIN                          "充电开始"
#define CHARGE_SOUND_END                            "充电结束"
#elif defined(FEATURE_SUBSYS_WAV_ENABLE)
#define WELCOME_SOUND                               "D:/Welcome.wav"
#define POWER_SOUND_OFF                             "D:/PowerOff.wav"
#define SIM_SOUND_UNREADY                           "D:/SimUnready.wav"
#define NW_SOUND_READY                              "D:/NwReady.wav"
#define NW_SOUND_UNREADY                            "D:/NwUnready.wav"
#define SERVER_SOUND_READY                          "D:/ServerReady.wav"
#define SERVER_SOUND_UNREADY                        "D:/ServerUnready.wav"
#define VOLUME_SOUND_MIN                            "D:/VolumeMin.wav"
#define VOLUME_SOUND_MAX                            "D:/VolumeMax.wav"
#define VOLUME_SOUND_MINUS                          "D:/VolumeMinus.wav"
#define VOLUME_SOUND_PLUS                           "D:/VolumePlus.wav"
#define BATTRY_SOUND_100                            "D:/Battery100.wav"
#define BATTRY_SOUND_75                             "D:/Battery75.wav"
#define BATTRY_SOUND_50                             "D:/Battery50.wav"
#define BATTRY_SOUND_25                             "D:/Battery25.wav"
#define BATTRY_SOUND_LOW                            "D:/BatteryLow.wav"
#define CHARGE_SOUND_BEGIN                          "D:/ChargeBegin.wav"
#define CHARGE_SOUND_END                            "D:/ChargeEnd.wav"
#else
#error "No defined audio types, such as MP3, TTS and WAV."
#endif

#else

#if defined(FEATURE_SUBSYS_MP3_ENABLE)
#define WELCOME_SOUND                               "C:/Welcome.mp3"
#define POWER_SOUND_OFF                             "C:/PowerOff.mp3"
#define SIM_SOUND_UNREADY                           "C:/SimUnready.mp3"
#define NW_SOUND_READY                              "C:/NwReady.mp3"
#define NW_SOUND_UNREADY                            "C:/NwUnready.mp3"
#define SERVER_SOUND_READY                          "C:/ServerReady.mp3"
#define SERVER_SOUND_UNREADY                        "C:/ServerUnready.mp3"
#define VOLUME_SOUND_MIN                            "C:/VolumeMin.mp3"
#define VOLUME_SOUND_MAX                            "C:/VolumeMax.mp3"
#define VOLUME_SOUND_MINUS                          "C:/VolumeMinus.mp3"
#define VOLUME_SOUND_PLUS                           "C:/VolumePlus.mp3"
#define BATTRY_SOUND_100                            "C:/Battery100.mp3"
#define BATTRY_SOUND_75                             "C:/Battery75.mp3"
#define BATTRY_SOUND_50                             "C:/Battery50.mp3"
#define BATTRY_SOUND_25                             "C:/Battery25.mp3"
#define BATTRY_SOUND_LOW                            "C:/BatteryLow.mp3"
#define CHARGE_SOUND_BEGIN                          "C:/ChargeBegin.mp3"
#define CHARGE_SOUND_END                            "C:/ChargeEnd.mp3"
#elif defined(FEATURE_SUBSYS_TTS_ENABLE)
#define WELCOME_SOUND                               "欢迎使用移芯收款播报设备"
#define POWER_SOUND_OFF                             "正在关机"
#define SIM_SOUND_UNREADY                           "无SIM卡"
#define NW_SOUND_READY                              "网络连接成功"
#define NW_SOUND_UNREADY                            "网络连接失败"
#define SERVER_SOUND_READY                          "服务器连接成功"
#define SERVER_SOUND_UNREADY                        "服务器连接失败"
#define VOLUME_SOUND_MIN                            "音量最小"
#define VOLUME_SOUND_MAX                            "音量最大"
#define VOLUME_SOUND_MINUS                          "音量减小"
#define VOLUME_SOUND_PLUS                           "音量加大"
#define BATTRY_SOUND_100                            "电量百分之一百"
#define BATTRY_SOUND_75                             "电量百分之七十五"
#define BATTRY_SOUND_50                             "电量百分之五十"
#define BATTRY_SOUND_25                             "电量百分之二十五"
#define BATTRY_SOUND_LOW                            "电量过低"
#define CHARGE_SOUND_BEGIN                          "充电开始"
#define CHARGE_SOUND_END                            "充电结束"
#elif defined(FEATURE_SUBSYS_WAV_ENABLE)
#define WELCOME_SOUND                               "C:/Welcome.wav"
#define POWER_SOUND_OFF                             "C:/PowerOff.wav"
#define SIM_SOUND_UNREADY                           "C:/SimUnready.wav"
#define NW_SOUND_READY                              "C:/NwReady.wav"
#define NW_SOUND_UNREADY                            "C:/NwUnready.wav"
#define SERVER_SOUND_READY                          "C:/ServerReady.wav"
#define SERVER_SOUND_UNREADY                        "C:/ServerUnready.wav"
#define VOLUME_SOUND_MIN                            "C:/VolumeMin.wav"
#define VOLUME_SOUND_MAX                            "C:/VolumeMax.wav"
#define VOLUME_SOUND_MINUS                          "C:/VolumeMinus.wav"
#define VOLUME_SOUND_PLUS                           "C:/VolumePlus.wav"
#define BATTRY_SOUND_100                            "C:/Battery100.wav"
#define BATTRY_SOUND_75                             "C:/Battery75.wav"
#define BATTRY_SOUND_50                             "C:/Battery50.wav"
#define BATTRY_SOUND_25                             "C:/Battery25.wav"
#define BATTRY_SOUND_LOW                            "C:/BatteryLow.wav"
#define CHARGE_SOUND_BEGIN                          "C:/ChargeBegin.wav"
#define CHARGE_SOUND_END                            "C:/ChargeEnd.wav"
#else
#error "No defined audio types, such as MP3, TTS and WAV."
#endif

#endif

#ifdef FEATURE_SUBSYS_PCM_RECORD_ENABLE
#define RECORD_SOUND                                "D:/Record.wav"
#endif

#define VOLUME_PLUS_VPRESS_SHORT                    "+"
#define VOLUME_PLUS_VPRESS_LONG                     "++"
#define VOLUME_MINUS_VPRESS_SHORT                   "-"
#define VOLUME_MINUS_VPRESS_LONG                    "--"

#define BOOT_TIME_PA_LTK5313                        180
#define BOOT_TIME_PA                                BOOT_TIME_PA_LTK5313
#define BOOT_TIME_CODEC_ES7148_ES7149_ES8311        70
#define BOOT_TIME_CODEC                             BOOT_TIME_CODEC_ES7148_ES7149_ES8311

#if defined(CODEC_ES8374_ENABLE)
#define CODEC_VOLUME_PLAY_DEFAULT                   100
#define CODEC_VOLUME_MIC_DEFAULT                    100
#elif defined(CODEC_ES8311_ENABLE)
#define CODEC_VOLUME_PLAY_DEFAULT                   (HAL_CODEC_VOL_SPEAKER_DEFAULT + 10)
#define CODEC_VOLUME_MIC_DEFAULT                    HAL_CODEC_VOL_MIC_DEFAULT
#elif defined(CODEC_TM8211_ENABLE)
#define CODEC_VOLUME_PLAY_DEFAULT                   100
#elif defined(CODEC_ES7111_ENABLE)
#define CODEC_VOLUME_PLAY_DEFAULT                   100
#elif defined(CODEC_ES7149_ENABLE)
#define CODEC_VOLUME_PLAY_DEFAULT                   100
#elif (defined(PWM_CODEC_ENABLE) || defined(SPI_CODEC_ENABLE))
#define CODEC_VOLUME_PLAY_DEFAULT                   100
#endif


typedef void (*AudioCallbackT)(int32_t result);

typedef struct
{
    bool            increase;
    bool            dual;
    uint32_t        rate;
    uint32_t        length;
    char           *buffer;
    AudioCallbackT  callback;
} PlayParamT;

typedef struct
{
} RecordVolteParamT;

typedef struct
{
    uint8_t           audioType;
    PlayParamT        playParam;
#ifdef FEATURE_SUBSYS_RECORD_ENABLE
    RecordParamT      recordParam;
#endif
    RecordVolteParamT recordVolteParam;
} QueueAudioT;

typedef enum
{
    AUDIO_PLAY         = 0x00,
#ifdef FEATURE_SUBSYS_RECORD_ENABLE
    AUDIO_RECORD       = 0x10,
    AUDIO_RECORD_VOLTE = 0x20
#endif
} AudioT;

typedef enum
{
    AUDIO_PLAY_MP3 = AUDIO_PLAY,
    AUDIO_PLAY_TTS,
    AUDIO_PLAY_AMR,
    AUDIO_PLAY_WAV,
    AUDIO_PLAY_PCM,
    AUDIO_PLAY_VOLUME
} AudioPlayT;

#ifdef FEATURE_SUBSYS_RECORD_ENABLE
typedef enum
{
    AUDIO_RECORD_G726 = AUDIO_RECORD,
    AUDIO_RECORD_ADPCM,
    AUDIO_RECORD_G711,
    AUDIO_RECORD_PCM,
    AUDIO_RECORD_PCM_3A,
    AUDIO_RECORD_AMR
} AudioRecordT;

typedef enum
{
    AUDIO_RECORD_VOLTE_AMR = AUDIO_RECORD_VOLTE,
    AUDIO_RECORD_VOLTE_PCM,
    AUDIO_RECORD_VOLTE_PCM_3A
} AudioRecordVolteT;
#endif

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

typedef enum
{
    AUDIO_PLAY_IDLE,
    AUDIO_PLAY_FILE    = 1,
    AUDIO_PLAY_STRING,
    AUDIO_PLAY_STREAM,
} AudioSrcTypeT;

typedef enum
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

typedef enum _avPlayResp{
	AV_RET_PLAY_ERROR		= -1,
	AV_RET_PLAY_START		= 0 ,
	AV_RET_PLAY_STOP,
	AV_RET_PLAY_EOF,
	AV_RET_PLAY_TERMINATE,
}avPlayResp_E;

extern openPlayer gOpenPlayer;


void     audioInit(void);
void     audioPlayMp3(char *path, AudioCallbackT callback, bool increase, bool dual);
void     audioPlayTts(char *text, AudioCallbackT callback);
void     audioPlayAmr(char *path, AudioCallbackT callback);
void     audioPlayAmrData(uint8_t *data, uint32_t length, AudioCallbackT callback);
void     audioPlayWav(char *path, AudioCallbackT callback);
void     audioPlayPcm(uint8_t *pcm, uint32_t length, uint32_t rate);
#ifdef FEATURE_SUBSYS_PCM_RECORD_ENABLE
void     audioRecordG726(char *path);
void     audioRecordAdpcm(char *path);
void     audioRecordG711(char *path);
#endif
#ifdef FEATURE_SUBSYS_AMR_RECORD_ENABLE
int32_t  audioRecordAmr(RecordParamT *recordParam);
#endif
void     audioAdjustVolume(uint8_t action);
bool     audioIsReady(void);
bool     audioIsBusy(void);
uint16_t audioGetVolume(void);
uint16_t audioGetCodecVolume(void);
bool     audioPlayIsStop(void);
void     audioPlayStop(void);
void     audioPlayMp3Data(uint8_t *data, uint32_t length);

uint32_t audioPlayTimeGet(void);
int32_t  EC_audioStopPlaying(uint8_t audType,bool pauseFlag);
int32_t  EC_audioStartPlayFile(char *file_name, uint8_t audType, audioParamT audParam, AudioCallbackT play_cb_func);
int32_t  EC_audioStartPlayString(uint8_t *String, uint32_t strLen, uint8_t audType, audioParamT audParam, AudioCallbackT play_cb_func);
int32_t  EC_audioStartPlayStream(uint8_t *url, uint32_t strLen, uint8_t audType, audioParamT audParam, AudioCallbackT play_cb_func);
bool     EC_audioPlayChkLoop(void);
void     EC_audioPlaySetLoop(bool onoff);

#endif
