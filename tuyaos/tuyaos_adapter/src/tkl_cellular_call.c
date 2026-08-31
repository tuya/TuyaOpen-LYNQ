#include "tuya_iot_config.h"

#if defined(ENABLE_CATX_CALL) && (ENABLE_CATX_CALL == 1)

#include <stdio.h>
#include <string.h>
#include "tkl_cellular_call.h"
#include "osasys.h"
#include "cmsis_os2.h"
#include "ol_call_api.h"
#include "vlog.h"
#include "volume_ctl.h"

static TKL_CELLULAR_CALL_CB call_event_func = NULL;
static CELLULAR_CALL_STATE_E g_cellular_call_state = CELLULAR_CALL_STATE_IDLE;

static void set_call_state(CELLULAR_CALL_STATE_E state)
{
    LOGD("set call state from %d to %d", g_cellular_call_state, state);
    g_cellular_call_state = state;
    if(state == CELLULAR_CALL_STATE_DIALING || state == CELLULAR_CALL_STATE_RINGING || state == CELLULAR_CALL_STATE_IDLE) {
        codec_volume_type_switch(VOLUME_TYPE_TONE);
    } else if(state == CELLULAR_CALL_STATE_ANSWERING) {
        codec_volume_type_switch(VOLUME_TYPE_CALL);
    }
}   

/**
 * @brief 查询呼叫服务是否可用
 * @note 该函数用于查询呼叫服务是否可用，只有在呼叫服务可用时，才能使用使用
 *       其它呼叫服务接口。
 *
 * @param simId sim卡ID
 *
 * @return 1 服务可用 0 服务不可用
 */
OPERATE_RET tkl_cellular_call_service_available(UINT8_T sim_id)
{
    return OPRT_OK;
}

/**
 * @brief 查询当前电话状态
 *
 * @param simId sim卡ID号
 * @param state 电话状态
 *
 * @return  0 成功 其它 失败
 */
OPERATE_RET tkl_cellular_call_get_status(UINT8_T sim_id, CELLULAR_CALL_STATE_E *state)
{
    if(state == NULL) {
        return OPRT_INVALID_PARM;
    }
    *state = g_cellular_call_state;
    return OPRT_OK;
}

/**
 * @brief 呼叫拨号接口函数
 *
 * @param simId sim卡ID号
 * @param callNUm 呼叫号码，字符串形式
 *
 * @return  0 发起呼叫成功 其它 失败
 */
OPERATE_RET tkl_cellular_call(UINT8_T sim_id, CHAR_T callNum[TKL_CELLULAR_CALLNUM_LEN_MAX])
{   
    if(CELLULAR_CALL_STATE_IDLE != g_cellular_call_state)
        return OPRT_COM_ERROR;
    OPERATE_RET ret = ol_call_dial(callNum);
    if(ret) {
        LOGE("call dial failed, ret=%d num:%s", ret, callNum);
        call_event_func(TUYA_CALL_IND_CALL_RSP_FAIL, 0, NULL);
        return ret;
    }
    set_call_state(CELLULAR_CALL_STATE_DIALING);
    return ret;
}

/**
 * @brief 外部呼叫到来时，应答接听接口函数
 *
 * @param 无
 *
 * @return   0 应答成功 其它 失败
 */
OPERATE_RET tkl_cellular_call_answer(UINT8_T sim_id)
{
    if(CELLULAR_CALL_STATE_RINGING != g_cellular_call_state)
        return OPRT_COM_ERROR;
    OPERATE_RET ret = ol_call_answer();
    if (ret) {
        LOGE("call answer failed, ret=%d", ret);
        call_event_func(TUYA_CALL_IND_ACCEPT_CALL_FAIL, 0, NULL);
        return ret;
    }
    set_call_state(CELLULAR_CALL_STATE_ANSWERING);
    call_event_func(TUYA_CALL_IND_ACCEPT_CALL_OK, 0, NULL);
    return ret;
}

/**
 * @brief 呼叫通话后，挂机接口函数
 *
 * @param 无
 *
 * @return  0 挂机成功 其它 失败
 */
OPERATE_RET tkl_cellular_call_hungup(UINT8_T sim_id)
{
    if(CELLULAR_CALL_STATE_IDLE == g_cellular_call_state)
        return OPRT_COM_ERROR;
    OPERATE_RET ret = ol_call_hangup();
    if(ret) {
        LOGE("call hangup failed, ret=%d", ret);
        return ret;
    }
    set_call_state(CELLULAR_CALL_STATE_IDLE);
    call_event_func(TUYA_CALL_IND_RELEASE, 0, NULL);
    return ret;
}

/**
 * @brief 注册用户定义的呼叫回调处理函数
 *
 * @param callback 呼叫回调处理函数
 *
 * @return 0 注册成功 其它 注册失败
 */
OPERATE_RET tkl_cellular_call_cb_register(TKL_CELLULAR_CALL_CB callback)
{
    call_event_func = callback;
    return OPRT_OK;
}

/**
 * @brief 设置呼入时铃声静音
 * @param mute TRUE 静音 FALSE 非静音
 * @return 0 设置成功 其它 设置失败
 */
OPERATE_RET tkl_cellular_call_set_callin_mute(BOOL_T mute)
{
    return codec_volume_set_mute(VOLUME_TYPE_TONE, mute);
}

/**
 * @brief 设置呼入时铃声音量
 * @param vol 铃声音量（0~100）
 * @return 0 设置成功 其它 设置失败
 */
OPERATE_RET tkl_cellular_call_set_callin_vol(INT_T vol)
{
    return codec_volume_set(VOLUME_TYPE_TONE, vol);
}

/**
 * @brief 设置呼入时铃声音量
 * @param vol 铃声音量（0~100）
 * @return 0 设置成功 其它 设置失败 
 */
OPERATE_RET tkl_cellular_call_get_callin_vol(PINT_T vol)
{
    return codec_volume_get(VOLUME_TYPE_TONE, vol);
}

/**
 * @brief 启动或者关闭volte功能
 *
 * @param enable TRUE启用volte,false关闭volte
 *
 * @return 0 设置成功 其它 设置失败
 */
OPERATE_RET tkl_cellular_call_set_volte(UINT8_T sim_id, BOOL_T enable)
{
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief 播放电话的拨号音
 *
 * @param tone 拨号音的特征值
 * @param duration 播放音的持续时间
 *
 * @return 0 成功，其他失败
 */
OPERATE_RET tkl_cellular_call_play_tone(TUYA_TONE_TYPE_E tone, INT_T duration)
{
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief 停止电话的拨号音
 *
 * @param 无
 *
 * @return 0 成功，其他失败
 */
OPERATE_RET tkl_cellular_call_stop_tone(VOID)
{
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief 将DTMF数字按键音频发送到语音通道
 * @note 1、该函数用于实现10086之类的语音交互，语音提示后，用户操作按键，调用该函数
 *       将按键音频发送到语音通道，该函数只能在通话状态后调用，否则将引起系统异常。
 *       2、dtmfTone只能是TKL_TONE_DTMF_0 ~ TKL_TONE_DTMF_STAR中的一种。
 *
 * @param dtmfTone 拨号按键
 * @param duration 音频持续时长
 *
 * @return 0 成功
 *        -1 dtmfTone 类型错误
 *        -2 分配内存失败
 *    OPRT_TIMEOUT 获取信号量超时
 *    OPRT_BASE_OS_ADAPTER_REG_NULL_ERROR 函数未适配
 */
OPERATE_RET tkl_cellular_call_dtmf2voice(TUYA_TONE_TYPE_E dtmfTone, UINT_T duration)
{
	return OPRT_NOT_SUPPORTED;
}

/**
 * @brief 设置语音通话音频的音量
 *
 * @param vol （0~100）
 *
 * @return 0 成功，其他失败
 */
OPERATE_RET tkl_cellular_call_set_voice_vol(INT_T vol)
{
    return codec_volume_set(VOLUME_TYPE_CALL, vol);
}

/**
 * @brief 获取语音通话音频的音量
 *
 * @param vol （0~100）
 *
 * @return 0 成功，其他失败
 */
OPERATE_RET tkl_cellular_call_get_voice_vol(PINT_T vol)
{
    return codec_volume_get(VOLUME_TYPE_CALL, vol);
}

/**
 * @brief 通话过程中，上行音频是否设置静音
 * 当前的语音呼叫结束后，这个属性会被保存下来。下次再进行语音通话的过程中，如果
 * 上次被设置为静音，则继续静音。但是重启后，会被默认恢复成非静音
 *
 * @param TRUE for mute uplink of voice call
 *
 * @return 0 成功，其他失败
 */
OPERATE_RET tkl_cellular_call_set_voice_mute(BOOL_T mute)
{
    return codec_mic_set_mute(MIC_TYPE_CALL, mute);
}

/**
 * @brief 获取通话过程中的上行声音的静音状态。
 *
 * @param mute : TRUE 静音
 *
 * @return 0 成功，其他失败
 */
OPERATE_RET tkl_cellular_call_get_voice_mute(PBOOL_T mute)
{
    return codec_mic_get_mute(MIC_TYPE_CALL, mute);
}

/**
 * @brief 注册DMTF侦测回调函数
 *
 * @param cb :  回调函数
 *
 * @return 0 成功，其他失败
 */
OPERATE_RET tkl_cellular_call_reg_KTDetect(TKL_CELLULAR_CALL_KTDETECH_CB cb)
{
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief 控制DTMF侦测功能是否使能，系统默认未使能。（开启这个功能，音频相关业务过程中，会增加系统负荷）
 *
 * @param enable : TRUE 使能 FALSE 禁止
 *
 * @return 0 成功，其他失败
 */
OPERATE_RET tkl_cellular_call_ctrl_KTDetect(BOOL_T enable)
{
    return OPRT_NOT_SUPPORTED;
}

// 设置录音模式麦克风增益
OPERATE_RET tkl_cellular_call_set_mic_gain(INT_T gain)
{
    return codec_mic_set_volume(MIC_TYPE_CALL, gain);
}

OPERATE_RET tkl_cellular_call_get_mic_gain(INT_T *gain)
{
    return codec_mic_get_volume(MIC_TYPE_CALL, gain);
}

OPERATE_RET tkl_cellular_ctrl_call_sideton_gain(BOOL_T set,INOUT INT_T *gain)
{
    return OPRT_NOT_SUPPORTED;
}

static void call_urc(int event_id,void *msg)
{
    LOGI("call urc event_id:%d %s",event_id, msg);
    switch (event_id)
    {
        case VCALL_NO_CARRIER:
            set_call_state(CELLULAR_CALL_STATE_IDLE);
            call_event_func(TUYA_CALL_IND_RELEASE, 0, NULL);  
        break;
        case VCALL_NO_ANSWER:
        break;
        case VCALL_RING:
            
        break;
        case VCALL_CLIP:
        {
            set_call_state(CELLULAR_CALL_STATE_RINGING);
            //+CLIP: "xxxxxx",129,,0,,0
            char callNum[TKL_CELLULAR_CALLNUM_LEN_MAX];
            memset(callNum,0,sizeof(callNum));
            char *num_start = strchr(msg, '"');
            if(num_start == NULL) {
                return ;
            }
            char *num_end = strchr(num_start+1, '"');
            if(num_end == NULL) {
                return ;
            }
            strncpy(callNum,num_start+1,num_end-num_start-1);
            call_event_func(TUYA_CALL_IND_CALLIN, 0, (CHAR_T *)callNum);
        }
        break;
        case VCALL_COLP:
            set_call_state(CELLULAR_CALL_STATE_ANSWERING);
            call_event_func(TUYA_CALL_IND_CALL_RSP_OK, 0, NULL); 
        break;
        default:
        break;
    }
}

OPERATE_RET tkl_cellular_call_init(TKL_CELLULAR_CALL_CB callback)
{
    static bool init = false;
    if(init) {
        return OPRT_OK;
    }
    ol_call_task_Init();
    ol_call_event_register(call_urc);
    call_event_func = callback;
    codec_volume_set(VOLUME_TYPE_TONE, 35);
    codec_volume_set(VOLUME_TYPE_CALL, 35);
    init = true;
    return OPRT_OK;
}
#endif