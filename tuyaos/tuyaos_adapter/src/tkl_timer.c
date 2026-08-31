#include <stdlib.h>
#include "tkl_timer.h"
#include "cmsis_os2.h"
#include "vlog.h"

#undef LOGD
#define LOGD(fmt, ...) 

typedef struct {
	osTimerId_t id;
	UINT_T us;
} timer_dev_t;

static timer_dev_t s_timer_dev[TUYA_TIMER_NUM_MAX] = {0};

OPERATE_RET tkl_timer_init(TUYA_TIMER_NUM_E timer_id,
                           TUYA_TIMER_BASE_CFG_T* cfg)
{	
	if (timer_id >= TUYA_TIMER_NUM_MAX || !cfg) {
		LOGE("timer init failed, invalid param, timer_id: %d, cfg: %p", timer_id, cfg);
		return OPRT_OS_ADAPTER_INVALID_PARM;
	}
	
	if (NULL == cfg->cb) {
		LOGE("timer init failed, cb is null");
		return OPRT_OS_ADAPTER_INVALID_PARM;
	}

	if (s_timer_dev[timer_id].id) {
		LOGE("timer reinit, timer is already exist, id: %p", s_timer_dev[timer_id].id);
		return OPRT_COM_ERROR;
	}

	osTimerType_t type = (TUYA_TIMER_MODE_ONCE == cfg->mode) ? osTimerOnce : osTimerPeriodic;
	osTimerId_t id = osTimerNew(cfg->cb, type, cfg->args, NULL);
	if (NULL == id) {
		LOGE("timer init failed");
		return OPRT_COM_ERROR;
	}

	s_timer_dev[timer_id].id = id;
	LOGD("init timer succ, id/%d, osid/%d", timer_id, id);
	return OPRT_OK;
}

OPERATE_RET tkl_timer_start(TUYA_TIMER_NUM_E timer_id, UINT_T us)
{
	if (timer_id >= TUYA_TIMER_NUM_MAX || 0 == us) {
		LOGE("timer start failed, invalid param, timer_id: %d, us: %u", timer_id, us);
		return OPRT_OS_ADAPTER_INVALID_PARM;
	}

	if (NULL == s_timer_dev[timer_id].id) {
		LOGE("timer start failed, not inited, timer_id: %d", timer_id);
		return OPRT_OS_ADAPTER_INVALID_PARM;
	}

	uint32_t ms = (us / 1000) + ((us % 1000) ? 1 : 0);
	LOGD("timer %d start, ms: %u", timer_id, ms);
	osStatus_t status = osTimerStart(s_timer_dev[timer_id].id, ms);
	if (osOK != status) {
		LOGE("start timer %d failed, status: %d", timer_id, status);
		return OPRT_COM_ERROR;
	}

	s_timer_dev[timer_id].us = us;
	return OPRT_OK;
}

OPERATE_RET tkl_timer_stop(TUYA_TIMER_NUM_E timer_id)
{
	LOGD("stop timer, id/%d, osid/%d", timer_id, s_timer_dev[timer_id].id);

	if (timer_id >= TUYA_TIMER_NUM_MAX) {
		LOGE("timer stop failed, invalid param, timer_id: %d", timer_id);
		return OPRT_OS_ADAPTER_INVALID_PARM;
	}

	if (NULL == s_timer_dev[timer_id].id) {
		LOGE("timer stop failed, not inited, timer_id: %d", timer_id);
		return OPRT_OS_ADAPTER_INVALID_PARM;
	}

	osStatus_t status = osTimerStop(s_timer_dev[timer_id].id);
	if (osOK != status) {
		LOGE("stop timer %d failed, status: %d", timer_id, status);
		return OPRT_COM_ERROR;
	}
	return OPRT_OK;
}

OPERATE_RET tkl_timer_deinit(TUYA_TIMER_NUM_E timer_id)
{
	if (timer_id >= TUYA_TIMER_NUM_MAX) {
		LOGE("timer deinit failed, invalid param, timer_id: %d", timer_id);
		return OPRT_OS_ADAPTER_INVALID_PARM;
	}
	
	if (NULL == s_timer_dev[timer_id].id) {
		LOGE("timer deinit failed, not inited, timer_id: %d", timer_id);
		return OPRT_OS_ADAPTER_INVALID_PARM;
	}

	osStatus_t status = osTimerDelete(s_timer_dev[timer_id].id);
	if (osOK != status) {
		LOGE("deinit timer %d failed, status: %d", timer_id, status);
		return OPRT_COM_ERROR;
	}
	
	s_timer_dev[timer_id].id = NULL;
	s_timer_dev[timer_id].us = 0;
	return OPRT_OK;
}


/**
 * @brief timer get
 *
 * @param[in] timer_id timer id
 * @param[out] ms timer interval
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_timer_get(TUYA_TIMER_NUM_E timer_id, UINT_T* us)
{	
	if (timer_id >= TUYA_TIMER_NUM_MAX || !us) {
		LOGE("timer get failed, invalid param, timer_id: %d, us: %p", timer_id, us);
		return OPRT_OS_ADAPTER_INVALID_PARM;
	}
	
	if (NULL == s_timer_dev[timer_id].id) {
		LOGE("timer get failed, not inited, timer_id: %d", timer_id);
		return OPRT_OS_ADAPTER_INVALID_PARM;
	}

	*us = s_timer_dev[timer_id].us;
	return OPRT_OK;
}
