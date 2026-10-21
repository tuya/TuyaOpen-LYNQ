#include "tkl_adc.h"
#include "osasys.h"
#include "cmsis_os2.h"
#include "ol_adc_api.h"
#include "vlog.h"

#define ADC_CHN_MAX    2

/*============================ GLOBAL VARIABLES ==============================*/
OPERATE_RET tkl_adc_init(TUYA_ADC_NUM_E port_num, TUYA_ADC_BASE_CFG_T* cfg)
{
    if (cfg == NULL || port_num >= ADC_CHN_MAX) {
        LOGE("init adc failed, invalid param, port_num/%d, cfg/%p", port_num, cfg);
        return OPRT_INVALID_PARM;
    }

    LOGI("init adc success, port_num/%d", port_num);
    return OPRT_OK;
}

OPERATE_RET tkl_adc_read_data(TUYA_ADC_NUM_E port_num, INT32_T* buff, UINT16_T len)
{
    LOGE("adc read data not supported");
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_adc_deinit(TUYA_ADC_NUM_E port_num)
{
    if (port_num >= ADC_CHN_MAX) {
        LOGE("deinit adc failed, invalid param, port_num/%d", port_num);
    }

    LOGI("deinit adc success, port_num/%d", port_num);
    return OPRT_OK;
}

UINT8_T tkl_adc_width_get(TUYA_ADC_NUM_E port_num)
{
    return 0;
}

UINT32_T tkl_adc_ref_voltage_get(TUYA_ADC_NUM_E port_num)
{
    return OPRT_NOT_SUPPORTED;
}

INT32_T tkl_adc_temperature_get(VOID_T)
{
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_adc_read_single_channel(TUYA_ADC_NUM_E port_num, UINT8_T ch_id,
                                        INT32_T* data)
{
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_adc_read_voltage(TUYA_ADC_NUM_E port_num, INT32_T* buff, UINT16_T len)
{
	UINT32 uV;
    if (port_num >= ADC_CHN_MAX || NULL == buff || 0 == len) {
        LOGE("read voltage failed, port_num/%d, len/%d", port_num, len);
        return OPRT_INVALID_PARM;
    }

    ol_adc_id_enum id = (port_num - TUYA_ADC_NUM_0) + OL_ADC_0;
    for (uint16_t i = 0; i < len; i++) {
        uV = ol_adc_get_vol(id);
		buff[i] = (INT32_T)(uV / 1000);
    }
    return OPRT_OK;
}

/*============================================================================*/

#if 0
#include "tkl_system.h"
extern OPERATE_RET tkl_cellular_vbat_get_voltage(UINT_T* voltage);
void tkl_adc_test(void)
{
    int ret = 0;
    int value = 0;
    while(1) {
        ret = tkl_adc_read_voltage(0, &value, 1);
        LOGI("adc chn 0 get value: %d", value);

        ret = tkl_adc_read_voltage(1, &value, 1);
        LOGI("adc chn 1 get value: %d", value);

        ret = tkl_cellular_vbat_get_voltage(&value);
        LOGI("vbat get value: %d", value);
        tkl_system_sleep(100);
    }

}
#endif