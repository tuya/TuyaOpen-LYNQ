/****************************************************************************
 *
 * File name:    app.c
 * Description:  open app demo entry source file
 *
 ****************************************************************************/
#include "string.h"
#include "bsp.h"
#include "bsp_custom.h"
#include "os_common.h"
#include "ostask.h"
#include "cmsis_os2.h"
#include "plat_config.h"
#include "ps_lib_api.h"
#include "ps_event_callback.h"
#include "slpman.h"
#include "reset.h"
#include "ol_sys_api.h"
#include "plat_config.h"
#include "ol_fs_api.h"
#include "uart_device.h"
#include "vlog.h"

/*
  App init entry function. Customer do init or create tasks based on their needs
*/
static void app_init(void *arg)
{
    LastResetState_e ap_state;
    LastResetState_e cp_state;
    slpManSlpState_t slpstate;
    ol_dump_mode dump;

    osDelay(1000);
    LOGD("App demo init: %s", ol_get_sw_version());

    ResetStateGet(&ap_state, &cp_state);
    LOGD("Check last reset reason: ap_state = %d, cp_state = %d",ap_state, cp_state);

    //check wakeup from deep sleep or power on
    slpstate = slpManGetLastSlpState();
    LOGD("Check last sleep state = %d",slpstate);

    uint32_t slpwaittime = BSP_GetPlatConfigItemValue(PLAT_CONFIG_ITEM_WAIT_SLEEP);
    if(slpwaittime != 4000) {
        LOGD("Set slpwaittime");
        //设置睡眠等待时间，低功耗下，唤醒后，等待4s后进入睡眠，等同于 AT+ECPCFG="slpWaitTime",4000
        slpwaittime = apmuBuildWaitSlpCfg(4000);
        BSP_SetPlatConfigItemValue(PLAT_CONFIG_ITEM_WAIT_SLEEP, slpwaittime);
    }

    // ol_set_dump_flag(OL_DUMP_PRINT_RESET);
	// ol_set_dump_flag(OL_DUMP_FLASH_EPAT_LOOP);
    slpManSetPmuSleepMode(false, SLP_SLP1_STATE, true);

    char *list_buf = malloc(1024);
    if(list_buf) {
        LOGD("list_buf = %d",list_buf);
        memset(list_buf, 0, 1024);
        ol_fs_list(list_buf, 1024);

        LOGD("list_buf = %s",list_buf);
        char *token = strtok(list_buf, "\r\n");
        while (token != NULL) {
            LOGD("ol_fs_list: %s", token);
            token = strtok(NULL, "\n");
        }
        free(list_buf);
    }
    
	int t_size = ol_fs_get_totalspacesize();
    int u_size = ol_fs_get_usedspacesize();
    int f_size = ol_fs_get_freespacesize();
    LOGD("check fs: total/%d, used/%d, free/%d", t_size, u_size, f_size);
    LOGD("Build time %s %s", __DATE__, __TIME__);

#if 1
    extern int main(void);
    main();
#else
    // extern void uart_demo(void);
    // uart_demo();

    // extern void tkl_semaphore_test(void);
    // tkl_semaphore_test();                    //ok
    // extern void tkl_mutex_test(void);
    // tkl_mutex_test();                        //ok
    // extern void tkl_queue_test(void);
    // tkl_queue_test();                        //ok

    // extern void tkl_uart_test(void);         
    // tkl_uart_test();                         //ok
    // extern void tkl_gpio_test(void);
    // tkl_gpio_test();

    // extern void tkl_adc_test(void);
    // tkl_adc_test();

    // extern void tkl_pwm_test(void);
    // tkl_pwm_test();

    // extern void tkl_i2c_test(void);
    // tkl_i2c_test();

    while(1) 
    {
        osDelay(1000);
    }
#endif

}

static void test_sleep_before_cb(void *pdata, slpManLpState state)
{
    LOGD("before sleep state:%d", state);
    slpManAONIOLatchEn(AonIOLatch_Enable);
}

static void test_sleep_after_cb(void *pdata, slpManLpState state)
{
    LOGD("after sleep state:%d", state);
    mbtkInitENGpio();
    extern void keep_pin_output_in_lowpower(void);
    keep_pin_output_in_lowpower();
}

/**
  \fn          int main_entry(void)
  \brief       main entry function.
  \return
*/
void main_entry(void)
{
    BSP_CommonInit();

    slpManAONIOPowerOn();
    mbtkInitENGpio();
    slpManRegisterUsrdefinedBackupCb(test_sleep_before_cb, NULL);
    slpManRegisterUsrdefinedRestoreCb(test_sleep_after_cb, NULL);
    // if(0)
    // {
    //     //IO voltage default is 1.8V
    //     //Modify the IO voltage to 3.3V or other voltage define in IOVoltageSel_t as follows
    //     slpManAONIOVoltSet(IOVOLT_3_30V);
    //     slpManNormalIOVoltSet(IOVOLT_3_30V);
    // }
    osKernelInitialize();
    registerAppEntry(app_init, NULL);
    if (osKernelGetState() == osKernelReady)
    {
        osKernelStart();
    }
    while(1);

}
