#include "tkl_timer.h"
#include "tkl_cellular_comm.h"
#include "vlog.h"
#include "ol_nw_api.h"

#define TIME_ID   5
static uint8_t rrc_release_time_value = 0;
static bool rcc_release_timer_init = false;

static void tkl_cellular_comm_rrc_release_timer_cb(void *arg)
{
    appTriggerRel();
}

//设置RRC connect切换到idle的延迟时间
OPERATE_RET tkl_cellular_comm_set_rrc_release_time(UINT_T time)
{
    if(time < 4 || time > 20) {
        return OPRT_INVALID_PARM;
    }

    if(rcc_release_timer_init == false) {
        TUYA_TIMER_BASE_CFG_T cfg = {
            .mode = TUYA_TIMER_MODE_ONCE,
            .cb = tkl_cellular_comm_rrc_release_timer_cb,
            .args = NULL
        };
        tkl_timer_init(TIME_ID, &cfg);
        rcc_release_timer_init = true;
    }
    rrc_release_time_value = time;
    LOGD("set rrc release time %d", time);
    return OPRT_OK;
}

//获取RRC connect切换到idle的延迟时间
OPERATE_RET tkl_cellular_comm_get_rrc_release_time(UINT_T *time)
{
    *time = rrc_release_time_value;
    return OPRT_OK;
}

OPERATE_RET rrc_release_timer_reset(void)
{
    if(!rcc_release_timer_init) {
        return OPRT_OK;
    }

    tkl_timer_stop(TIME_ID);
    tkl_timer_start(TIME_ID, rrc_release_time_value * 1000000 / 2);
    return OPRT_OK;
}
