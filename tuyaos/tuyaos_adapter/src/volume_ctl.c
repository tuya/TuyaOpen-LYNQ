#include "tuya_iot_config.h"

#if (defined(ENABLE_CATX_CALL) && (ENABLE_CATX_CALL == 1)) || (defined(ENABLE_CATX_PLAYER) && (ENABLE_CATX_PLAYER == 1))

#include "vlog.h"
#include "volume_ctl.h"
#include "ol_gpio_api.h"
#include "tkl_gpio.h"

/**
 * 底层codec 音量范围为 0-255，
 */

typedef struct 
{
    uint8_t volume; 
    bool mute;   // 默认未静音
}volume_ctl_struct;

static volume_ctl_struct volume_controls[VOLUME_TYPE_MAX] = {{0xff, false}, {0xff, false}, {0xff, false}};
static volume_ctl_struct mic_controls[MIC_TYPE_MAX] = {{0xff, false}, {0xff, false}};     
static VOLUME_TYPE_E current_volume_type = VOLUME_TYPE_AUDIO;
static int pa_pin_num = 0xff;

extern int ol_setVolume(int);    
extern int ol_getVolume(int *);
extern int ol_setMicVolume(int);
extern int ol_getMicVolume(int *);

//TODO 初始化 PA控制脚，并返回PA控制脚的编号
int pad_hook(void)
{
    if(pa_pin_num == 0xff) {
        pa_pin_num = 21;                        // 默认使用 PA21 控制 PA
    }
    TUYA_GPIO_BASE_CFG_T cfg = {0};
    cfg.mode = -1;                              // tkl_gpio_init 会设置成 pull_auto
    cfg.direct = TUYA_GPIO_OUTPUT;
    tkl_gpio_init(pa_pin_num, &cfg);
    return pa_pin_num;
}

void tkl_audio_pa_pin_set(int pin, bool enable)
{
    pa_pin_num = pin;
}

static int codec_set_volume(int vol)
{
    LOGI("type:%d set volume:%d", current_volume_type, vol);
    ol_setVolume(vol * 2);
    return 0;
}

static int codec_get_volume(int *vol)
{
    int v;
    ol_getVolume(&v);
    *vol = v / 2;
    return 0;
}

static int codec_set_micvolume(int vol)
{
    ol_setMicVolume(vol * 2);
    return 0;
}

static int codec_get_micvolume(int *vol)
{
    int v;
    ol_getVolume(&v);
    *vol = v / 2;
    return 0;
}

// 设置speak音量，范围0~100
OPERATE_RET codec_volume_set(VOLUME_TYPE_E type, INT_T vol)
{
    if (vol < 0 || vol > 100 || type >= VOLUME_TYPE_MAX) {
        return OPRT_INVALID_PARM;
    }

    volume_controls[type].volume = (uint8_t)vol;
    if(volume_controls[type].mute) {
        // 静音状态下，不设置音量
        return OPRT_OK;
    }

    if(current_volume_type == type) {
        return codec_set_volume(vol);
    }

    return OPRT_OK;
}

// 获取speak音量，范围0~100
OPERATE_RET codec_volume_get(VOLUME_TYPE_E type, INT_T *vol)
{
    if (vol == NULL || type >= VOLUME_TYPE_MAX) {
        return OPRT_INVALID_PARM;
    }

    if(volume_controls[VOLUME_TYPE_AUDIO].volume == 0xff || volume_controls[VOLUME_TYPE_CALL].volume == 0xff || volume_controls[VOLUME_TYPE_TONE].volume == 0xff) {
        // 未设置过音量，获取当前音量
        int current_vol = 0;
        OPERATE_RET ret = codec_get_volume(&current_vol);
        if (ret != OPRT_OK) {
            LOGE("get volume failed, ret: %d", ret);
            return ret;
        }
        LOGI("get current volume:%d", current_vol);
        if(volume_controls[VOLUME_TYPE_AUDIO].volume == 0xff)
            volume_controls[VOLUME_TYPE_AUDIO].volume = (uint8_t)current_vol;
        if(volume_controls[VOLUME_TYPE_CALL].volume == 0xff)
            volume_controls[VOLUME_TYPE_CALL].volume = (uint8_t)current_vol;
        if(volume_controls[VOLUME_TYPE_TONE].volume == 0xff)
            volume_controls[VOLUME_TYPE_TONE].volume = (uint8_t)current_vol;
    }

    *vol = (INT_T)volume_controls[type].volume;
    return OPRT_OK;
}

// 设置speak静音状态
OPERATE_RET codec_volume_set_mute(VOLUME_TYPE_E type, BOOL_T mute)
{
    if (type >= VOLUME_TYPE_MAX) {
        return OPRT_INVALID_PARM;
    }
    
    if(current_volume_type == type) {
        LOGI("set volume mute type:%d, mute:%d", type, mute);
        if(mute) {
            codec_set_volume(0);
        } else {
            codec_set_volume(volume_controls[type].volume);
        }
    }
    volume_controls[type].mute = mute;
    return OPRT_OK;
}

// 获取speak静音状态
OPERATE_RET codec_volume_get_mute(VOLUME_TYPE_E type, BOOL_T *mute)
{
    if (mute == NULL || type >= VOLUME_TYPE_MAX) {
        return OPRT_INVALID_PARM;
    }           
    *mute = volume_controls[type].mute;
    return OPRT_OK;
}

// 设置麦克风音量，范围0~100
OPERATE_RET codec_mic_set_volume(MIC_TYPE_E type, INT_T vol)
{
    if (vol < 0 || vol > 100 || type >= MIC_TYPE_MAX) {
        return OPRT_INVALID_PARM;
    }

    bool set_mic_flag = false;
    if((type == MIC_TYPE_AUDIO && current_volume_type == VOLUME_TYPE_AUDIO) || 
        (type == MIC_TYPE_CALL && (current_volume_type == VOLUME_TYPE_CALL || current_volume_type == VOLUME_TYPE_TONE))) {
        set_mic_flag = true;
    }

    OPERATE_RET ret = OPRT_COM_ERROR;
    int old_vol = 0;
    int set_vol = 0;
    if(set_mic_flag) {
        codec_mic_get_volume(type, &old_vol);
        ret = codec_set_micvolume(set_vol);
    }

    if(ret == OPRT_OK) {
        mic_controls[type].volume = (uint8_t)set_vol;
    }
    LOGI("set mic volume type:%d, vol:%d->%d %d", type, old_vol, set_vol, ret);
    return ret;
}

// 获取麦克风音量，范围0~100
OPERATE_RET codec_mic_get_volume(MIC_TYPE_E type, INT_T *vol)
{
    if (vol == NULL || type >= MIC_TYPE_MAX) {
        return OPRT_INVALID_PARM;
    }           

    if(mic_controls[MIC_TYPE_AUDIO].volume == 0xff || mic_controls[MIC_TYPE_CALL].volume == 0xff) {
        // 未设置过麦克风音量，获取当前麦克风音量  
        int current_vol = 0;
        OPERATE_RET ret = codec_get_micvolume(&current_vol);
        if (ret != OPRT_OK) {
            LOGE("get Micvolume failed, ret: %d", ret);
            return ret;
        }
        LOGI("current Micvolume:%d", current_vol);
        if(mic_controls[MIC_TYPE_AUDIO].volume == 0xff)
            mic_controls[MIC_TYPE_AUDIO].volume = current_vol;
        if(mic_controls[MIC_TYPE_CALL].volume == 0xff)
            mic_controls[MIC_TYPE_CALL].volume = current_vol;
    }

    *vol = (INT_T)mic_controls[type].volume;
    return OPRT_OK;
}

// 设置是否闭麦
OPERATE_RET codec_mic_set_mute(MIC_TYPE_E type, BOOL_T mute)
{
    if (type >= MIC_TYPE_MAX) {
        return OPRT_INVALID_PARM;
    }
    
    bool set_mic_flag = false;
    if((type == MIC_TYPE_AUDIO && current_volume_type == VOLUME_TYPE_AUDIO) || 
        (type == MIC_TYPE_CALL && (current_volume_type == VOLUME_TYPE_CALL || current_volume_type == VOLUME_TYPE_TONE))){
        set_mic_flag = true;
    } 

    OPERATE_RET ret = OPRT_COM_ERROR;
    if(set_mic_flag) {
        if(mute) {
            ret = codec_set_micvolume(0);
        } else {
            ret = codec_set_micvolume(mic_controls[type].volume);
        }
    }
    if(ret == OPRT_OK) {
        mic_controls[type].mute = mute;
    }
    LOGI("set mic mute %d type:%d, mute:%d", ret, type, mute);
    return ret;
}

// 获取是否闭麦
OPERATE_RET codec_mic_get_mute(MIC_TYPE_E type, BOOL_T *mute)
{
    if (mute == NULL || type >= MIC_TYPE_MAX) {
        return OPRT_INVALID_PARM;
    }           
    *mute = mic_controls[type].mute;
    return OPRT_OK;
}

// 获取当前音量类型
OPERATE_RET codec_volume_get_current_type(VOLUME_TYPE_E *type)
{
    if (type == NULL) {
        return OPRT_INVALID_PARM;
    }           

    *type = current_volume_type;
    return OPRT_OK;
}

// 切换音量类型
OPERATE_RET codec_volume_type_switch(VOLUME_TYPE_E type)
{
    if (type >= VOLUME_TYPE_MAX) {
        return OPRT_INVALID_PARM;
    }           

    if(current_volume_type != type) {
        current_volume_type = type;
        LOGI("switch volume type to:%d", type);
        // 切换音量类型时，设置对应的音量和静音状态
        if(volume_controls[type].mute) {
            codec_set_volume(0);
        } else {
            codec_set_volume(volume_controls[type].volume);
        }
        MIC_TYPE_E mic_type = type == VOLUME_TYPE_AUDIO ? MIC_TYPE_AUDIO : MIC_TYPE_CALL;
        codec_set_micvolume(mic_controls[mic_type].volume);
    }
    return OPRT_OK;
}

#endif