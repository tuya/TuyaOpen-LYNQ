#include "tkl_cellular_vbat.h"
#include "vlog.h"
#include "ol_adc_api.h"

/**
 * @brief 获取电池剩余电量百分比
 *
 * @param rsoc 剩余电量百分比
 *
 * @return OPRT_OK 获取成功 其它 获取失败
 */
OPERATE_RET tkl_cellular_vbat_get_rsoc(UINT8_T* rsoc)
{
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief 获取电池电压
 *
 * @param voltage 当前电池电压，单位mV
 *
 * @return OPRT_OK 获取成功 其它 获取失败
 */
OPERATE_RET tkl_cellular_vbat_get_voltage(UINT_T* voltage)
{
    int ret = ol_adc_get_vol(OL_ADC_VBAT) / 1000;
    if (ret < 0) {
        LOGE("vbat get voltage failed, ret: %d", ret);
        return OPRT_COM_ERROR;
    }

    *voltage = ret;
    return OPRT_OK;
}

/**
 * @brief 设置是否开启NTC检测电池温度
 *
 * @param enable NTC检测电池温度开/关
 *
 * @return OPRT_OK 设置成功 其它 设置失败
 */
OPERATE_RET tkl_cellular_vbat_ntc_enable(BOOL_T enable)
{
    LOGE("set ntc enable not support");
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief 设置恒流充电阶段，电池充电电流
 *
 * @param current 充电电流，单位毫安（mA)
 *
 * @return OPRT_OK 设置成功 其它 设置失败
 */
OPERATE_RET tkl_cellular_vbat_set_charge_current(UINT_T current)
{
    LOGE("set charge current not support");
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief 获取充电器状态
 *
 * @param 无
 *
 * @return 充电器状态，查看 @TKL_CELLULAR_VBAT_CHG_STATE_E定义
 */
TKL_CELLULAR_VBAT_CHG_STATE_E tkl_cellular_vbat_get_charger_state(VOID)
{
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief 注册电池及充电器消息回调处理函数
 *
 * @param callback 回调函数
 *
 * @return 0 注册成功 其它 注册失败
 */
OPERATE_RET tkl_cellular_vbat_cb_register(TKL_CELLULAR_VBAT_CHARGE_CB callback)
{
    return OPRT_OK;
}

/**
 * @brief 是否开启电池低电压关机功能
 *
 * @param enable TRUE 开启，FLASE关闭
 *
 * @return OPRT_OK 设置成功 其它 设置失败
 */
OPERATE_RET tkl_cellular_vbat_low_volt_poweroff_enable(BOOL_T enable)
{
    LOGE("low volt poweroff not support");
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_cellular_vbat_init(void)
{
    LOGE("vbat init not support");
    return OPRT_NOT_SUPPORTED;
}
