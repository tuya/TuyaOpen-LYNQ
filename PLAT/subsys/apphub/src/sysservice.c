#include <string.h>
#include "FreeRTOS.h"
#include "ostask.h"
#include "cmsis_os2.h"
#include "charge.h"
#include "pwrkey.h"
#include "slpman.h"
#include "ps_lib_api.h"
#ifdef FEATURE_DRIVER_KEYPAD_ENABLE
#include "keypad.h"
#include "kpc.h"
#endif
#ifdef FEATURE_SUBSYS_STATUS_ENABLE
#include "status.h"
#endif
#ifdef FEATURE_SUBSYS_MODE_ENABLE
#include "mode.h"
#endif
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_SUBSYS_SYSTIME_ENABLE
#include "systime.h"
#endif
#include "bsp_custom.h"
#include DEBUG_LOG_HEADER_FILE
#include "plat_config.h"
#ifdef FEATURE_SUBSYS_APPHUB_ENABLE
#include "app.h"
#include "apphub.h"
#endif
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
#include "jsonDb.h"
#endif
#ifdef FEATURE_SUBSYS_GUI_FPUI_ENABLE
#include "fpui.h"
#include "ui.h"
#include "merged.h"
#endif
#ifdef FEATURE_SUBSYS_TELECOM_ENABLE
#include "teleservice.h"
#endif
#ifdef FEATURE_SUBSYS_POWER_MANAGER_ENABLE
#include "powermanager.h"
#endif
#include "sysservice.h"
#if UI_WATCH_USED
#include "app_hal.h"
#include "api_scr.h"
#endif
#ifdef FEATURE_SUBSYS_FEATURES_ENABLE
#include "featureCall.h"
#endif
#include "hal_alarm.h"

#ifdef FEATURE_SUBSYS_GUI_FPUI_ENABLE
#include "api_scr.h"
extern const char *fpui_bat_arr[];
extern const char *fpui_sig_arr[];

static uint8_t     gBatteryIndex = 0;
static uint8_t     gCsqIndex     = 0;
#endif


extern void show_hangup();
extern uint8_t getOSState(void);
#ifdef FEATURE_SUBSYS_FPUI_ENABLE
extern PhoneUI_call_t call_data;
extern AppT mainApp;
extern int call_num_index;
#endif

static void poweroffHandle(void)
{
    SYSLOG_INFO("Start power Off.\r\n");

    appSetCFUN(0);
#ifdef FEATURE_SUBSYS_FPUI_ENABLE
    fpui_message_t fpuiMessage =
    {
        .context = "正在关机...",
        .width   = 220,
        .height  = 50,
        .color   = WHITE,
    };

    ui_message_set(&fpuiMessage);
    osDelay(1000);
#endif

#ifdef FEATURE_SUBSYS_SYSTIME_ENABLE
    saveTime(time_time(NULL));
#endif

#ifdef FEATURE_SUBSYS_POWER_MANAGER_ENABLE
    powerManagerSetState(powerManagerGetCurrnetPowerSaveMode());
#endif
}

static void poweroffHandle_hightemp(void)
{
    SYSLOG_INFO("Start hightemp power Off.\r\n");

    appSetCFUN(0);
#ifdef FEATURE_SUBSYS_FPUI_ENABLE
    fpui_message_t fpuiMessage =
    {
        .context = "高温关机...",
        .width   = 220,
        .height  = 50,
        .color   = WHITE,
    };

    ui_message_set(&fpuiMessage);
    osDelay(1000);
#endif

#ifdef FEATURE_SUBSYS_SYSTIME_ENABLE
    saveTime(time_time(NULL));
#endif

#ifdef FEATURE_SUBSYS_POWER_MANAGER_ENABLE
    powerManagerSetState(powerManagerGetCurrnetPowerSaveMode());
#endif
}

static void poweroffHandle_lowbat(void)
{
    SYSLOG_INFO("Start lowbat power Off.\r\n");

    appSetCFUN(0);
#ifdef FEATURE_SUBSYS_FPUI_ENABLE
    fpui_message_t fpuiMessage =
    {
        .context = "低电关机...",
        .width   = 220,
        .height  = 50,
        .color   = WHITE,
    };

    ui_message_set(&fpuiMessage);
    osDelay(1000);
#endif

#ifdef FEATURE_SUBSYS_SYSTIME_ENABLE
    saveTime(time_time(NULL));
#endif

#ifdef FEATURE_SUBSYS_POWER_MANAGER_ENABLE
    powerManagerSetState(powerManagerGetCurrnetPowerSaveMode());
#endif
}
void poweroff(void)
{
    AppMsgT msg = {0};

    memset(&msg, 0, sizeof(msg));
    msg.msgType = APP_SYSTEM_MSG;
    msg.param1 = SYSSERVICE_EVENT_POWER_OFF;
    appSendMsg(&msg);
}

void lowBatAlarm(bool isVoltAboveThd)
{
    AppMsgT msg = {0};
    // SYSLOG_INFO("lowBatAlarm isVoltAboveThd = %d\n", isVoltAboveThd);
    if(isVoltAboveThd == ALARM_INFO_DOWNWARD && 1 == getOSState())
    {
        memset(&msg, 0, sizeof(msg));
        msg.msgType = APP_SYSTEM_MSG;
        msg.param1 = SYSSERVICE_EVENT_LOW_BATTERY_ALARM;
        appSendMsg(&msg);
    }
}

void highTempAlarm(bool isTempAboveThd)
{
    AppMsgT msg = {0};
    // SYSLOG_INFO("highTempAlarm isTempAboveThd = %d\n", isTempAboveThd);
    if(isTempAboveThd == ALARM_INFO_UPWARD && 1 == getOSState())
    {        
        memset(&msg, 0, sizeof(msg));
        msg.msgType = APP_SYSTEM_MSG;
        msg.param1 = SYSSERVICE_EVENT_HIGH_TEMP_ALARM;
        appSendMsg(&msg);
    }
}

int32_t sysservice_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
#ifdef FEATURE_SUBSYS_STATUS_ENABLE
    StatusT    status = {0};
#ifdef FEATURE_SUBSYS_SYSTIME_ENABLE
    struct tm *tmTime = NULL;
#endif
#endif
#ifdef FEATURE_SUBSYS_GUI_FPUI_ENABLE
    uint8_t batteryIndex = 0;
    uint8_t csqIndex     = 0;
#endif

    switch(msg->msgType)
    {
        case APP_STAT_MSG:
#ifdef FEATURE_SUBSYS_STATUS_ENABLE
            ((uint32_t *)(&status))[0] = msg->param1;
            ((uint32_t *)(&status))[1] = msg->param2;
            ((uint32_t *)(&status))[2] = (uint32_t)(msg->param3);
            SYSLOG_DEBUG("APP_STAT_MSG: batteryLevel=%d, batteryCharge=%d, simStatus=%d, nwReady=%d, csqLevel=%d\r\n",
                         status.batteryLevel, status.batteryCharge, status.simStatus, status.nwReady, status.csqLevel);
#ifdef FEATURE_SUBSYS_SYSTIME_ENABLE
            tmTime = time_localtime((time_t *)(status.time));
            SYSLOG_DEBUG("APP_STAT_MSG: time=%d-%d-%d %d:%d:%d %d\r\n",
                         1900 + tmTime->tm_year, tmTime->tm_mon + 1, tmTime->tm_mday, tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec, tmTime->tm_wday);
#endif
#ifdef FEATURE_SUBSYS_GUI_FPUI_ENABLE
            batteryIndex = ((status.batteryCharge == true) ? 6 : 0) + (status.batteryLevel / 17);
            if (batteryIndex != gBatteryIndex)
            {
                gBatteryIndex = batteryIndex;
                ui_addr_set(ui_ImageBat, (char *)fpui_bat_arr[gBatteryIndex]);
            }

            csqIndex = status.csqLevel / 21;
            if (csqIndex != gCsqIndex)
            {
                gCsqIndex = csqIndex;
                ui_addr_set(ui_ImageSig, (char *)fpui_sig_arr[gCsqIndex]);
            }
#endif
#endif
            break;

        case APP_SYSTEM_MSG:
            switch(msg->param1)
            {
                case SYSSERVICE_EVENT_POWER_OFF:
                    poweroffHandle();
                    break;
                case SYSSERVICE_EVENT_LOW_BATTERY_ALARM:
                    poweroffHandle_lowbat();
                    break;
                case SYSSERVICE_EVENT_HIGH_TEMP_ALARM:
                    poweroffHandle_hightemp();
                    break;

#ifdef FEATURE_SUBSYS_TELECOM_ENABLE
                case TELECOM_EVENT_INCOMING_CALL_NUMBER:
#ifdef FEATURE_SUBSYS_POWER_MANAGER_ENABLE
                    if (powerManagerGetPowerState() == POWER_STATE_WORKING)
#endif
                    {
#ifdef FEATURE_SUBSYS_FEATURES_ENABLE
                        mainApp.msgProc = callInComing_proc;
                        extern void show_call_incoming(char *num);
                        show_call_incoming((char *)(msg->param3));
#endif
                    }
                    break;

                case TELECOM_EVENT_INCOMING_SMS:
                    SYSLOG_DEBUG("SMS: number=%s\r\n",   ((SmsInputDataT *)(msg->param3))->number);
                    SYSLOG_DEBUG("SMS: datetime=%s\r\n", ((SmsInputDataT *)(msg->param3))->datetime);
                    SYSLOG_DEBUG("SMS: text=%s\r\n",     ((SmsInputDataT *)(msg->param3))->text);
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
                    jsonDbAddItem("D:/smsInboxDb.json", "D:/smsInboxDb.type", msg->param3);
#endif
                    free(msg->param3);
                    msg->param3 = NULL;
                    break;

                case TELECOM_EVENT_OTHER_HANG_UP:
#ifdef FEATURE_SUBSYS_FPUI_ENABLE
                    show_hangup();
                    osDelay(1000);
                    memset(&call_data,0,sizeof(call_data));
                    call_num_index=0;
                    mainApp.msgProc = normalAppMsgProc;
                    show_home();
                    break;
#endif
#endif
                default:
                    break;
            }
            break;

        default:
            break;
    }

    return 0;
}