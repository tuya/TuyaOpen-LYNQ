/****************************************************************************
 *
 ****************************************************************************/
#include "string.h"
#include "osasys.h"
#include "os_common.h"
#include "cmsis_os2.h"
#include "ps_lib_api.h"
#include "slpman.h"
#include "ol_log.h"


static void test_sleep_before_cb(void *pdata, slpManLpState state)
{
    OL_LOG_INFO("before sleep state = %d", state);
    //slpManAONIOLatchEn(true); //AONIO pin keep level during sleep
}

void sleep_demo(void)
{
    slpManSlpState_t slpstate;
    slpManWakeSrc_e wakeupSrc;

    slpstate = slpManGetLastSlpState();
    if(slpstate == SLP_ACTIVE_STATE)  // from power on
    {
        OL_LOG_INFO("Check Wakeup from Power On");
    }
    else if((slpstate == SLP_SLP2_STATE) || (slpstate == SLP_HIB_STATE))  // wakeup from sleep
    {
        wakeupSrc = slpManGetWakeupSrc();
        OL_LOG_INFO("Check Wakeup from state = %d, Wakeup source = %d <slpManWakeSrc_e>", slpstate, wakeupSrc);
    }

    slpManRegisterUsrdefinedBackupCb(test_sleep_before_cb,NULL);

    appSetCFUN(0);

    if(FALSE == slpManDeepSlpTimerIsRunning(DEEPSLP_TIMER_ID0)) //check TIMER0 is not running
    {
        slpManDeepSlpTimerStart(DEEPSLP_TIMER_ID0, 30000); //start timer, wake up after 30 seconds
    }

    slpManSetPmuSleepMode(true,SLP_HIB_STATE,false);
	OL_LOG_INFO("test set sleep HIB state");
    osDelay(1000);
}

