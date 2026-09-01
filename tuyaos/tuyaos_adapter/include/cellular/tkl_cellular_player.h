/**
 * @file tkl_cellular_player.h
 * @brief Cellular module audio player interface.
 *
 * The OEM package ships tkl_cellular_player.c but no matching header -- in the
 * TuyaOS 3.x tree it came from the SDK's own adapter directory. The types below
 * are the ones that file and tkl_init_cellular.h's TKL_CELL_PLAYER_INTF_T use;
 * every entry point on this module currently answers OPRT_NOT_SUPPORTED, so
 * they exist to describe the interface, not to drive the hardware. Replace them
 * with the vendor definitions when the audio player is actually implemented.
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */

#ifndef __TKL_CELLULAR_PLAYER_H__
#define __TKL_CELLULAR_PLAYER_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
***********************typedef define***********************
***********************************************************/

/**
 * @brief audio player type
 */
typedef enum {
    TKL_AUDEV_PLAY_TYPE_VOICE = 0, /* 8K sample rate, mono, voice channel */
    TKL_AUDEV_PLAY_TYPE_MEDIA,     /* media channel */
    TKL_AUDEV_PLAY_TYPE_MAX,
} TKL_AUDIO_PLAYER_TYPE_E;

/**
 * @brief audio stream format
 */
typedef enum {
    TKL_AUDIO_STREAM_FORMAT_PCM = 0,
    TKL_AUDIO_STREAM_FORMAT_WAV,
    TKL_AUDIO_STREAM_FORMAT_MP3,
    TKL_AUDIO_STREAM_FORMAT_AMR,
    TKL_AUDIO_STREAM_FORMAT_MAX,
} TKL_AUDIO_STREAM_FORMAT_E;

/**
 * @brief audio player status
 */
typedef enum {
    TKL_AUDIO_PLAYER_STATUS_IDLE = 0,
    TKL_AUDIO_PLAYER_STATUS_PLAYING,
    TKL_AUDIO_PLAYER_STATUS_PAUSED,
    TKL_AUDIO_PLAYER_STATUS_STOPPED,
} TKL_AUDIO_PLAYER_STATUS_E;

/**
 * @brief audio output channel
 */
typedef enum {
    TKL_AUDEV_OUTPUT_TYPE_SPEAKER = 0,
    TKL_AUDEV_OUTPUT_TYPE_HEADPHONE,
    TKL_AUDEV_OUTPUT_TYPE_RECEIVER,
    TKL_AUDEV_OUTPUT_TYPE_MAX,
} TKL_AUDEV_OUTPUT_TYPE;

/**
 * @brief audio input channel
 */
typedef enum {
    TKL_AUDEV_INPUT_TYPE_MAIN_MIC = 0,
    TKL_AUDEV_INPUT_TYPE_AUX_MIC,
    TKL_AUDEV_INPUT_TYPE_HEADSET_MIC,
    TKL_AUDEV_INPUT_TYPE_MAX,
} TKL_AUDEV_INPUT_TYPE;

/**
 * @brief player event reported to the caller
 */
typedef enum {
    TKL_AUDIO_PLAYER_EVENT_START = 0,
    TKL_AUDIO_PLAYER_EVENT_FINISH,
    TKL_AUDIO_PLAYER_EVENT_ERROR,
} TKL_AUDIO_PLAYER_EVENT_E;

/**
 * @brief audio player event callback
 *
 * @param[in] event  what happened, see TKL_AUDIO_PLAYER_EVENT_E
 * @param[in] cb_ctx the context handed to the play call
 */
typedef VOID (*TKL_AUDIO_PLAYER_CB)(TKL_AUDIO_PLAYER_EVENT_E event,
                                    PVOID_T cb_ctx);

/**
 * @brief stream playing operations, supplied by the caller
 */
typedef struct {
    /* fill buff with at most size bytes, return the byte count or < 0 */
    INT_T (*get_data)(PVOID_T play_ctx, PBYTE_T buff, UINT_T size);
    /* stream playing finished */
    VOID (*finish)(PVOID_T play_ctx);
} TKL_AUDIO_PLAY_OPS_T;

/**
 * @brief audio frame description, only the stream info is used
 */
typedef struct {
    TKL_AUDIO_STREAM_FORMAT_E format;
    UINT_T sample_rate;
    UINT_T channels;
    UINT_T bits_per_sample;
} TKL_AUDIO_FRAME_T;

/***********************************************************
********************function declaration********************
***********************************************************/

/**
 * @brief play an audio file
 *
 * @param[in] type      player type
 * @param[in] format    file format
 * @param[in] file_path full path of the audio file
 * @param[in] cb        event callback, may be NULL
 * @param[in] cb_ctx    callback context
 *
 * @return OPRT_OK on success. Others on error, please refer to
 * tuya_error_code.h
 */
OPERATE_RET tkl_cellular_player_start(TKL_AUDIO_PLAYER_TYPE_E type,
                                      TKL_AUDIO_STREAM_FORMAT_E format,
                                      PCHAR_T file_path,
                                      TKL_AUDIO_PLAYER_CB cb, PVOID_T cb_ctx);

/**
 * @brief pause the audio being played
 *
 * @return OPRT_OK on success. Others on error.
 */
OPERATE_RET tkl_cellular_player_pause(VOID);

/**
 * @brief resume the paused audio
 *
 * @return OPRT_OK on success. Others on error.
 */
OPERATE_RET tkl_cellular_player_resume(VOID);

/**
 * @brief stop the audio being played
 *
 * @return OPRT_OK on success. Others on error.
 */
OPERATE_RET tkl_cellular_player_stop(VOID);

/**
 * @brief set the playing volume
 *
 * @param[in] vol volume
 *
 * @return OPRT_OK on success. Others on error.
 */
OPERATE_RET tkl_cellular_player_set_vol(INT_T vol);

/**
 * @brief get the playing volume
 *
 * @param[out] vol volume
 *
 * @return OPRT_OK on success. Others on error.
 */
OPERATE_RET tkl_cellular_player_get_vol(PINT_T vol);

/**
 * @brief mute or unmute the player
 *
 * @param[in] mute TRUE to mute
 *
 * @return OPRT_OK on success. Others on error.
 */
OPERATE_RET tkl_cellular_player_set_mute(BOOL_T mute);

/**
 * @brief get the mute state
 *
 * @param[out] mute TRUE when muted
 *
 * @return OPRT_OK on success. Others on error.
 */
OPERATE_RET tkl_cellular_player_get_mute(PBOOL_T mute);

/**
 * @brief play an audio clip held in memory
 *
 * @param[in] type   player type
 * @param[in] format stream format
 * @param[in] buff   audio data
 * @param[in] size   audio data length
 * @param[in] cb     event callback, may be NULL
 * @param[in] cb_ctx callback context
 *
 * @return OPRT_OK on success. Others on error.
 */
OPERATE_RET tkl_cellular_player_mem_start(TKL_AUDIO_PLAYER_TYPE_E type,
                                          TKL_AUDIO_STREAM_FORMAT_E format,
                                          PBYTE_T buff, UINT_T size,
                                          TKL_AUDIO_PLAYER_CB cb,
                                          PVOID_T cb_ctx);

/**
 * @brief select the audio output channel
 *
 * @param[in] chan output channel
 *
 * @return OPRT_OK on success. Others on error.
 */
OPERATE_RET tkl_cellular_player_set_audio_output(TKL_AUDEV_OUTPUT_TYPE chan);

/**
 * @brief get the audio output channel
 *
 * @param[out] chan output channel
 *
 * @return OPRT_OK on success. Others on error.
 */
OPERATE_RET tkl_cellular_player_get_audio_output(TKL_AUDEV_OUTPUT_TYPE *chan);

/**
 * @brief select the audio input channel
 *
 * @param[in] chan input channel
 *
 * @return OPRT_OK on success. Others on error.
 */
OPERATE_RET tkl_cellular_player_set_audio_input(TKL_AUDEV_INPUT_TYPE chan);

/**
 * @brief get the audio input channel
 *
 * @param[out] chan input channel
 *
 * @return OPRT_OK on success. Others on error.
 */
OPERATE_RET tkl_cellular_player_get_audio_input(TKL_AUDEV_INPUT_TYPE *chan);

/**
 * @brief get the player status
 *
 * @param[out] status player status
 *
 * @return OPRT_OK on success. Others on error.
 */
OPERATE_RET tkl_cellular_player_get_status(TKL_AUDIO_PLAYER_STATUS_E *status);

/**
 * @brief start playing a stream fed by the caller
 *
 * @param[in] type    player type
 * @param[in] playOps stream callbacks
 * @param[in] playCtx context handed back to those callbacks
 * @param[in] frame   stream description
 *
 * @return OPRT_OK on success. Others on error.
 */
OPERATE_RET tkl_cellular_player_start_stream(TKL_AUDIO_PLAYER_TYPE_E type,
                                             CONST TKL_AUDIO_PLAY_OPS_T *playOps,
                                             PVOID_T playCtx,
                                             CONST TKL_AUDIO_FRAME_T *frame);

/**
 * @brief stop the stream being played
 *
 * @return OPRT_OK on success. Others on error.
 */
OPERATE_RET tkl_cellular_player_stop_stream(VOID);

#ifdef __cplusplus
}
#endif

#endif /* __TKL_CELLULAR_PLAYER_H__ */
