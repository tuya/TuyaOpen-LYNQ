#ifndef __VOLUME_CTL_H__
#define __VOLUME_CTL_H__

#include <stdint.h>
#include <stdbool.h>
#include "tuya_cloud_types.h"

typedef enum {
    VOLUME_TYPE_AUDIO = 0,          // 音频播放音量
    VOLUME_TYPE_TONE,               // 铃声音量
    VOLUME_TYPE_CALL,               // 通话音量
    VOLUME_TYPE_MAX,
} VOLUME_TYPE_E;

typedef enum
{
    MIC_TYPE_AUDIO = 0,          // 音频播放麦克风
    MIC_TYPE_CALL,               // 通话麦克风
    MIC_TYPE_MAX,
}MIC_TYPE_E;

// 设置speak音量，范围0~100
OPERATE_RET codec_volume_set(VOLUME_TYPE_E type, INT_T vol);
// 获取speak音量，范围0~100
OPERATE_RET codec_volume_get(VOLUME_TYPE_E type, INT_T *vol);
// 设置speak静音状态
OPERATE_RET codec_volume_set_mute(VOLUME_TYPE_E type, BOOL_T mute);
// 获取speak静音状态
OPERATE_RET codec_volume_get_mute(VOLUME_TYPE_E type, BOOL_T *mute);
// 设置麦克风音量，范围0~100
OPERATE_RET codec_mic_set_volume(MIC_TYPE_E type, INT_T vol);
// 获取麦克风音量，范围0~100
OPERATE_RET codec_mic_get_volume(MIC_TYPE_E type, INT_T *vol);
// 设置是否闭麦
OPERATE_RET codec_mic_set_mute(MIC_TYPE_E type, BOOL_T mute);
// 获取是否闭麦
OPERATE_RET codec_mic_get_mute(MIC_TYPE_E type, BOOL_T *mute);
// 获取当前音量类型
OPERATE_RET codec_volume_get_current_type(VOLUME_TYPE_E *type);
// 切换音量类型
OPERATE_RET codec_volume_type_switch(VOLUME_TYPE_E type);
// 设置PA控制脚，及使能时电平
void tkl_audio_pa_pin_set(int pin, bool enable);

#endif
