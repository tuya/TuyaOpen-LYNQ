#include <stdlib.h>
#include "tkl_sleep.h"
#include "cmsis_os2.h"
#include "slpman.h"
#include "vlog.h"
#include "bsp_custom.h"

static bool is_lp_enable = false;
static bool is_deep_sleep = false;
static TUYA_SLEEP_CB_T  *ty_sleep_fun = NULL;
static bool enable_wakeup_pin = true;

extern int32_t uartDevNotifySerlDtrEvt(uint32_t uartIdx);
void pin19_irq_handle(void)
{
    if(is_lp_enable && enable_wakeup_pin)
        uartDevNotifySerlDtrEvt(1);             //低功耗下通过拉低pin19，唤醒uart1
}

static int set_wakeup_pin(void)
{
    #define WEKEUP_PIN_NUM 19
    OPERATE_RET ret = OPRT_OK;
    APmuWakeupPadSettings_t wakeupPadSetting;
    wakeupPadSetting.negEdgeEn = true;
    wakeupPadSetting.posEdgeEn = false;
    wakeupPadSetting.pullDownEn = false;
    wakeupPadSetting.pullUpEn = true;
    if(is_lp_enable) {
        slpManSetWakeupPadCfg(WAKEUP_PAD_5, true, &wakeupPadSetting);
        NVIC_EnableIRQ(PadWakeup5_IRQn);
    } else {
        slpManSetWakeupPadCfg(WAKEUP_PAD_5, false, &wakeupPadSetting);
        NVIC_DisableIRQ(PadWakeup5_IRQn);
    }
    return ret;
}

int tuya_device_enable_sleep(BOOL_T enable)
{
    is_lp_enable = enable;
	slpManSetPmuSleepMode(is_lp_enable, SLP_SLP1_STATE, false);
    if(enable_wakeup_pin)
        set_wakeup_pin();
    return 0;
}

void close_wakeup_pin(void)
{
    enable_wakeup_pin = false;
}

/**
 * @brief sleep callback register
 *
 * @param[in] sleep_cb:  sleep callback
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_cpu_sleep_callback_register(TUYA_SLEEP_CB_T *sleep_cb)
{
    ty_sleep_fun = sleep_cb;
    return OPRT_OK;
}

/**
 * @brief allow to sleep
 *
 * @param[in] none
 *
 * @return none
 */
VOID_T tkl_cpu_allow_sleep(VOID_T)
{
    tuya_device_enable_sleep(TRUE);
}

/**
 * @brief force wakeup
 *
 * @param[in] none
 *
 * @return none
 */
VOID_T tkl_cpu_force_wakeup(VOID_T)
{
    tuya_device_enable_sleep(FALSE);
}

/**
* @brief Set the low power mode of CPU
*
* @param[in] enable: enable switch
* @param[in] mode:   cpu sleep mode
*
* @note This API is used for setting the low power mode of CPU.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_cpu_sleep_mode_set(BOOL_T enable, TUYA_CPU_SLEEP_MODE_E mode)
{
    is_lp_enable = enable;

    LOGI("tkl_cpu_sleep_mode_set %s",enable ? "enable" :"disable");
    is_deep_sleep = enable;
    if (enable) {
        tuya_device_enable_sleep(TRUE);
    } else {
        tuya_device_enable_sleep(FALSE);
    }
    return OPRT_OK;
}

/**
 * @brief Set target duration for the next tickless CPU sleep
 * @param[in] num_ms expected sleep window in milliseconds
 * @return OPRT_NOT_SUPPORTED not supported on this platform
 */
OPERATE_RET tkl_cpu_sleep_time_set(CONST UINT_T num_ms)
{
    (VOID_T)num_ms;
    return OPRT_NOT_SUPPORTED;
}
