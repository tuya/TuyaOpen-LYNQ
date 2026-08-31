#include <stdlib.h>
#include "tkl_system.h"
#include "cmsis_os2.h"
#include "reset.h"
#include "ol_open_api.h"
#include "vlog.h"

extern void ol_power_reset(void);

/**
* @brief system reset
*
* @param none
*
* @return none
*/
VOID_T tkl_system_reset(VOID_T)
{
	LOGD("system reset");
	tkl_system_sleep(500);
	ol_power_reset();
}

/**
* @brief Get system tick count
*
* @param none
*
* @return system tick count
*/
SYS_TICK_T tkl_system_get_tick_count(VOID_T)
{
    return osKernelGetTickCount();
}

/**
* @brief Get system millisecond
*
* @param none
*
* @return system millisecond
*/
SYS_TIME_T tkl_system_get_millisecond(VOID_T)
{
    return TICKS_TO_MILLISECONDS(osKernelGetTickCount());
}

UINT_T tkl_system_enter_critical(VOID_T)
{
	int32_t ret = osKernelLock();
    return (ret <= 0) ? 0 : ret;
}

VOID_T tkl_system_exit_critical(UINT_T irq_mask)
{
    osKernelUnlock();
}

/**
 * @brief tuya_os_adapt_get_random_data用于获取指定条件下的随机数
 *
 * @param[in] range
 * @return 随机值
 */
int tkl_system_get_random(const unsigned int range)
{
    static bool seed_flag = false;
	if(seed_flag == false) {
		seed_flag = true;
		srand(osKernelGetTickCount());
	}

	int data = rand();
	return (int)((unsigned int)data % range);
}

/**
* @brief Get system reset reason
*
* @param[in] describe: point to reset reason describe
*
* @return reset reason
*/
TUYA_RESET_REASON_E tkl_system_get_reset_reason(CHAR_T** describe)
{	
	LastResetState_e ap_state, cp_state;
	ResetStateGet(&ap_state, &cp_state);
	
	TUYA_RESET_REASON_E reason = TUYA_RESET_REASON_UNKNOWN;
	switch (ap_state) {
		case LAST_RESET_POR:
		case LAST_RESET_PAD:
			reason = TUYA_RESET_REASON_POWERON; break;
		case LAST_RESET_SWRESET:
			reason = TUYA_RESET_REASON_SOFTWARE; break;
		case LAST_RESET_HARDFAULT:
			reason = TUYA_RESET_REASON_FAULT; break;
		case LAST_RESET_ASSERT:
		case LAST_RESET_LOCKUP:
			reason = TUYA_RESET_REASON_CRASH; break;
		case LAST_RESET_WDTSW:
			reason = TUYA_RESET_REASON_SW_WDOG; break;
		case LAST_RESET_WDTHW:
		case LAST_RESET_AONWDT:
			reason = TUYA_RESET_REASON_HW_WDOG; break;
		case LAST_RESET_BATLOW:
		case LAST_RESET_TEMPHI:
			reason = TUYA_RESET_REASON_EXTERNAL; break;
		case LAST_RESET_FOTA:
			reason = TUYA_RESET_REASON_BOOTLOADER; break;
		case LAST_RESET_UNKNOWN:
			reason = TUYA_RESET_REASON_UNKNOWN; break;
		case LAST_RESET_MAXNUM:
		default:
			reason = TUYA_RESET_REASON_UNSUPPORT; break;
	}

	LOGI("tuya boot cause, ap/%d, cp/%d, ty_cause: %d", ap_state, cp_state, reason);
	return reason;
}

/**
* @brief  system sleep
*
* @param[in] describe: num ms
*
* @return none
*/
VOID_T tkl_system_sleep(UINT_T num_ms)
{
    osDelay(num_ms);
}

/**
* @brief system delay
*
* @param[in] msTime: time in MS
*
* @note This API is used for system sleep.
*
* @return VOID
*/
VOID_T tkl_system_delay(UINT_T num_ms)
{
    osDelay(num_ms);
}

OPERATE_RET tkl_system_get_cpu_info(TUYA_CPU_INFO_T **cpu_ary, INT_T *cpu_cnt)
{
    return OPRT_OK;
}

/**
 * @brief Get the device hardware-intrinsic unique id
 * @param[out] id caller-provided buffer
 * @param[inout] len in: buffer capacity; out: bytes written
 * @return OPRT_NOT_SUPPORTED not supported on this platform
 */
OPERATE_RET tkl_system_get_hw_unique_id(UINT8_T *id, UINT8_T *len)
{
    (VOID_T)id;
    (VOID_T)len;
    return OPRT_NOT_SUPPORTED;
}
