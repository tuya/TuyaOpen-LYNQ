#ifdef FEATURE_SUBSYS_POWER_MANAGER_ENABLE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "slpman.h"
#include "keypad.h"
#include "reset.h"
#include "pwrkey.h"
#include "hal_charge.h"
#include "ps_lib_api.h"
#include "app.h"
#include DEBUG_LOG_HEADER_FILE
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_SUBSYS_MISC_ENABLE
#include "misc.h"
#endif
#ifdef FEATURE_SUBSYS_STATUS_ENABLE
#include "status.h"
#endif
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#ifdef FEATURE_SUBSYS_TELECOM_ENABLE
#include "teleservice.h"
#endif
#ifdef FEATURE_SUBSYS_OPENHAL_ENABLE
#include "api_scr.h"
#endif
#ifdef FEATURE_SUBSYS_GUI_LVGL_ENABLE
#include "lvgl.h"
#endif
#ifdef FEATURE_SUBSYS_GUI_FPUI_ENABLE
#include "fpui.h"
#include "featurePowerOffCharging.h"
#endif
#ifdef FEATURE_SUBSYS_INPUT_ENABLE
#include "input.h"
#endif
#include "powermanager.h"
#include "servicemanager.h"


#define THREAD_STACK_SIZE_POWER_MANAGER             (5 * 1024)
#define QUEUE_SIZE_POWER_MANAGER                    3
#define POWER_MANAGER_LIST_SIZE                     20
#define HB_POWER_SAVE_LIST_SIZE                     5
#define PROHIBITION_LIST_SIZE                       20
#define SLEEP_DELAY_TIME                            3000
#define CEREG_CHECK_TIME                            1000
#define VBUS_CHANGE                                 20
#define REBOOT_TO_POWER_ON                          21
#define BOOT_INFO_FILE                              "C:/bootInfo.ini"
#define BOOT_INFO_KEY_APP_RESET_REASON              "appResetReason"


typedef struct
{
    uint8_t event;
} QueuePowerManagerT;

typedef struct
{
    osTimerId_t  timerId;
    HbPowerSaveT hbPowerSave;
} HbPowerSaveListT;

typedef enum
{
    BOOT_TYPE_POWER_KEY = 0,
    BOOT_TYPE_CHARGING,
    BOOT_TYPE_OTHER,
    BOOT_TYPE_MAX
} BootTypeT;

typedef enum
{
    APP_RESET_REASON_CHARGING_POWER_OFF = 0,
    APP_RESET_REASON_CHARGING_POWER_ON,
    APP_RESET_REASON_MAX
} AppResetReasonT;


static void hbPowerSaveTimerStop(void);
static void hbPowerSaveSleepCallback(void);


#ifdef THREAD_STATIC
static StaticTask_t          gPowerManagerThreadCbMem                                     = {0};
static uint8_t               gPowerManagerThreadStackMem[THREAD_STACK_SIZE_POWER_MANAGER] = {0};
#endif
static osThreadId_t          gPowerManagerThread                        = NULL;
static osMessageQueueId_t    gPowerManagerQueue                         = NULL;
static PowerManagerCallbackT gPowerManagerList[POWER_MANAGER_LIST_SIZE] = {0};
static HbPowerSaveListT      gHbPowerSaveList[HB_POWER_SAVE_LIST_SIZE]  = {0};
static osTimerId_t           gOsTimerId                                 = NULL;
static osTimerId_t           gSleepDelayTimer                           = NULL;
static osTimerId_t           gCeregTimerId                              = NULL;
static uint32_t              gBacklightDuration                         = 10000;
static bool                  gBacklightOn                               = true;
static uint8_t               gVote                                      = 0xFF;
static uint8_t               gState                                     = POWERMANAGER_FULL_POWER;
static uint8_t               gPowerState                                = POWER_STATE_WORKING;
static uint8_t               gProhibitionList[PROHIBITION_LIST_SIZE]    = {0};
#ifdef FEATURE_SUBSYS_TELECOM_ENABLE
static uint8_t               gProhibitionHandle                         = 0xFF;
#endif
static bool                  gGoToPowerOff                              = false;
static uint8_t               gBootType                                  = BOOT_TYPE_MAX;
static uint8_t               gBootResult                                = BOOT_RESULT_MAX;


int32_t powerManagerProhibitionCreate(uint8_t *handle)
{
    int32_t retVal = -1;

    if (handle == NULL)
    {
        SYSLOG_DEBUG("Param error.\r\n");
        goto labelEnd;
    }

    for (uint32_t i=0; i<PROHIBITION_LIST_SIZE; i++)
    {
        if (gProhibitionList[i] == 0)
        {
            *handle              = i;
             retVal              = 0;
             gProhibitionList[i] = PROHIBIT_NOTHING;
            goto labelEnd;
        }
    }

    SYSLOG_DEBUG("Prohibition list is full: %d\r\n", PROHIBITION_LIST_SIZE);

labelEnd:
    return retVal;
}

int32_t powerManagerProhibitionDelete(uint8_t handle)
{
    int32_t retVal = -1;

    if (handle >= PROHIBITION_LIST_SIZE)
    {
        SYSLOG_DEBUG("Param error.\r\n");
        goto labelEnd;
    }

    gProhibitionList[handle] = 0;

    retVal = 0;

labelEnd:
    return retVal;
}

uint8_t powerManagerProhibitionQuery(void)
{
    uint8_t prohibition = PROHIBIT_NOTHING;

    for (uint32_t i=0; i<PROHIBITION_LIST_SIZE; i++)
    {
        if ((gProhibitionList[i] != 0) && (gProhibitionList[i] > prohibition))
        {
            prohibition = gProhibitionList[i];
        }
    }

    return prohibition;
}

int32_t powerManagerProhibitionSet(uint8_t handle, uint8_t prohibition)
{
    int32_t retVal = -1;
    uint8_t state  = PROHIBIT_NOTHING;

    if ((handle >= PROHIBITION_LIST_SIZE) || (prohibition >= PROHIBIT_MAX))
    {
        SYSLOG_DEBUG("Param error.\r\n");
        goto labelEnd;
    }

    SYSLOG_DEBUG("handle=%d, prohibition=%d\r\n", handle, prohibition);
    state  = powerManagerProhibitionQuery();
    gProhibitionList[handle] = prohibition;

    if (powerManagerProhibitionQuery() < state)
    {
        powerManagerResetBacklightTimer();
    }

    retVal = 0;

labelEnd:
    return retVal;
}

uint8_t powerManagerGetCurrnetPowerSaveMode(void)
{
    for (uint32_t i=0; i<HB_POWER_SAVE_LIST_SIZE; i++)
    {
        if (gHbPowerSaveList[i].timerId != NULL)
        {
            return POWERMANAGER_HEARTBEAT_SAVE_POWER;
        }
    }

    return POWERMANAGER_SAVE_POWER;
}

#ifdef FEATURE_SUBSYS_TELECOM_ENABLE
static void telecomCallback(int32_t event, char *data)
{
    SYSLOG_DEBUG("event=%d\r\n", event);

    switch (event)
    {
        case TELECOM_EVENT_IMS_READY:
        case TELECOM_EVENT_IMS_AND_SMS_READY:
            powerManagerResetBacklightTimer();
            break;

        case TELECOM_EVENT_INCOMING_SMS:
            powerManagerSetState(POWERMANAGER_FULL_POWER);
            break;

        case TELECOM_EVENT_INCOMING_CALL_NUMBER:
            powerManagerSetState(POWERMANAGER_FULL_POWER);
            powerManagerProhibitionSet(gProhibitionHandle, PROHIBIT_TURN_OFF_BACKLIGHT);
            break;

        case TELECOM_EVENT_OTHER_ANSWERED:
            powerManagerProhibitionSet(gProhibitionHandle, PROHIBIT_ENTER_SLEEP);
            break;

        case TELECOM_EVENT_OTHER_HANG_UP:
            powerManagerSetState(POWERMANAGER_FULL_POWER);
            powerManagerProhibitionSet(gProhibitionHandle, PROHIBIT_NOTHING);
            break;

        case TELECOM_EVENT_ANSWER:
            powerManagerProhibitionSet(gProhibitionHandle, PROHIBIT_ENTER_SLEEP);
            break;

        case TELECOM_EVENT_DIALLED:
            powerManagerProhibitionSet(gProhibitionHandle, PROHIBIT_ENTER_SLEEP);
            break;

        case TELECOM_EVENT_HANG_UP:
            powerManagerProhibitionSet(gProhibitionHandle, PROHIBIT_NOTHING);
            break;

        case TELECOM_EVENT_CEREG:
            if (data[0] == '2')
            {
                SYSLOG_DEBUG("Searching\r\n");
                powerManagerResetBacklightTimer();
                osTimerStart(gCeregTimerId, CEREG_CHECK_TIME);
                powerManagerProhibitionSet(gProhibitionHandle, PROHIBIT_ENTER_SLEEP);
            }
            else
            {
                powerManagerProhibitionSet(gProhibitionHandle, PROHIBIT_NOTHING);
            }
            break;

        default:
            break;
    }
}
#endif

#ifdef FEATURE_HAL_SCREEN_ENABLE
#ifdef USED_WATCH_MPLAYER
static void drawActiveScreen(void)
{
    lv_obj_invalidate(lv_scr_act());
    lv_refr_now(NULL);
}
#endif
static void lcdDisplay(void)
{
#ifdef FEATURE_SUBSYS_TELECOM_ENABLE
    if (incomingCall() == true)
    {
        SYSLOG_DEBUG("Show incoming call.\r\n");
        AppMsgT msg =
        {
            .msgType = APP_SYSTEM_MSG,
            .param1  = TELECOM_EVENT_INCOMING_CALL_NUMBER,
            .param3  = (uint32_t *)incomingCallNumberGet(),
        };
        appSendMsg(&msg);
    }
    else
#endif
    {
        SYSLOG_DEBUG("Restore display: gBootResult=%d\r\n", gBootResult);
        if (gBootResult == BOOT_RESULT_POWER_ON)
        {
#ifdef USED_WATCH_MPLAYER
            osDelay(50);
            drawActiveScreen();
#else
#ifdef FEATURE_SUBSYS_GUI_FPUI_ENABLE
            if (mainApp.msgProc == normalAppMsgProc)
            {
                show_home();
            }
            else
#endif
            {
                extern uint32_t scr_dev_UsrId;
                extern lv_color_t s_disp_buf[];
                if(scr_dev_UsrId != 0)
                {
                    ScrWriteParam_t scrWriteParam =
                    {
                        .width  = LCD_WIDTH,
                        .height = LCD_HEIGHT,
                        .data   = (uint8_t *)s_disp_buf,
                        .size   = LCD_WIDTH * LCD_HEIGHT * 2
                    };
                    Device_write_by_index(scr_dev_UsrId, &scrWriteParam, sizeof(scrWriteParam));
                }
            }
#endif
        }
        else if (gBootResult == BOOT_RESULT_POWER_OFF_CHARGING)
        {
#ifdef FEATURE_SUBSYS_GUI_FPUI_ENABLE
            showPowerOffCharging();
#endif
        }
    }
}
#endif

bool powerManagerGetBacklightState(void)
{
    ECPLAT_PRINTF(UNILOG_PLA_APP, powerManagerGetBacklightState, P_DEBUG, "%d", gBacklightOn);
    return gBacklightOn;
}

static void lcdBacklightSet(bool on)
{
    SYSLOG_DEBUG("gBacklightOn=%d, on=%d\r\n", gBacklightOn, on);
#ifdef FEATURE_HAL_SCREEN_ENABLE
    Device_ioctl("dev:/lcd", on ? OPEN_SCREEN_BACKLIGHT_OPEN :OPEN_SCREEN_BACKLIGHT_CLOSE, NULL);
#endif
    gBacklightOn = on;
}

static void sleepDelayTimer(void *argument)
{
    SYSLOG_DEBUG("Sleep timer timeout.\r\n");
    powerManagerSetState(powerManagerGetCurrnetPowerSaveMode());
}

static void stopSleepDelayTimer(void)
{
    if (osTimerIsRunning(gSleepDelayTimer) != 0)
    {
        osTimerStop(gSleepDelayTimer);
    }
}

static void resetSleepDelayTimer(void)
{
    if (osTimerIsRunning(gSleepDelayTimer) != 0)
    {
        osTimerStop(gSleepDelayTimer);
    }
    osTimerStart(gSleepDelayTimer, SLEEP_DELAY_TIME);
}

static void osTimerFunc(void *argument)
{
    uint8_t prohibition = powerManagerProhibitionQuery();
    SYSLOG_DEBUG("prohibition=%d\r\n", prohibition);
    if (prohibition != PROHIBIT_TURN_OFF_BACKLIGHT)
    {
        SYSLOG_DEBUG("Auto turn off LCD backlight.\r\n");
        lcdBacklightSet(false);
    }
    resetSleepDelayTimer();
}

void powerManagerSetBacklightDuration(uint32_t backlightDuration)
{
    SYSLOG_DEBUG("backlightDuration=%d\r\n", backlightDuration);
    gBacklightDuration = backlightDuration * 1000;
    powerManagerResetBacklightTimer();
}

void powerManagerResetBacklightTimer(void)
{
    if (osTimerIsRunning(gOsTimerId) != 0)
    {
        osTimerStop(gOsTimerId);
    }
    osTimerStart(gOsTimerId, gBacklightDuration);
}

int32_t powerManagerRegister(PowerManagerCallbackT *callback)
{
    int32_t retVal = -1;

    if ((callback == NULL) || (strlen(callback->name) == 0))
    {
        SYSLOG_DEBUG("Param error.\r\n");
        goto labelEnd;
    }

    for (uint32_t i=0; i<POWER_MANAGER_LIST_SIZE; i++)
    {
        if (strcmp(gPowerManagerList[i].name, callback->name) == 0)
        {
            SYSLOG_DEBUG("The name has already been used: %s\r\n", callback->name);
            goto labelEnd;
        }
    }

    for (uint32_t i=0; i<POWER_MANAGER_LIST_SIZE; i++)
    {
        if (strlen(gPowerManagerList[i].name) == 0)
        {
            retVal = 0;
            memcpy(&gPowerManagerList[i], callback, sizeof(PowerManagerCallbackT));
            goto labelEnd;
        }
    }

    SYSLOG_DEBUG("Power manager list is full: %d\r\n", POWER_MANAGER_LIST_SIZE);

labelEnd:
    return retVal;
}

int32_t powerManagerUnregister(char *name)
{
    int32_t retVal = -1;

    if ((name == NULL) || (strlen(name) == 0))
    {
        SYSLOG_DEBUG("Param error.\r\n");
        goto labelEnd;
    }

    for (uint32_t i=0; i<POWER_MANAGER_LIST_SIZE; i++)
    {
        if (strcmp(gPowerManagerList[i].name, name) == 0)
        {
            retVal = 0;
            memset(&gPowerManagerList[i], 0, sizeof(PowerManagerCallbackT));
            goto labelEnd;
        }
    }

    SYSLOG_DEBUG("Power manager item don't exist: %s\r\n", name);

labelEnd:
    return retVal;
}

uint8_t powerManagerGetVbusState(void)
{
    uint8_t vbus = 0;

#if (defined(PHONE_DEVICE) && defined(FEATURE_SUBSYS_MISC_ENABLE))
    vbus = ((bootPinValueGet() == 0) && (chargeGetCurStatus() != CHARGE_STATUS_CHARGING)) ? 0 : 1;
#else
    vbus = (slpManGetWakeupPinValue() >> 1) & 1;
#endif

    ECPLAT_PRINTF(UNILOG_PLA_APP, powerManagerGetVbusState, P_DEBUG, "vbus=%d", vbus);

    return vbus;
}

void powerManagerVbusHandler(void)
{
    QueuePowerManagerT queue = {.event = VBUS_CHANGE};

    ECPLAT_PRINTF(UNILOG_PLA_APP, powerManagerVbusHandler, P_DEBUG, "");
    osMessageQueuePut(gPowerManagerQueue, &queue, 0, 0);
}

static int padWakeupHook(uint32_t pad_num)
{
    ECPLAT_PRINTF(UNILOG_PLA_APP, padWakeupHook, P_DEBUG, "pad_num=%d", pad_num);

    if (pad_num == 1)
    {
        powerManagerVbusHandler();
    }

    return 0;
}

static uint8_t getAppResetReason(void)
{
    uint8_t  reason    = APP_RESET_REASON_MAX;
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    int32_t *valueRead = NULL;

    valueRead = iniKeyValueRead(BOOT_INFO_FILE, BOOT_INFO_KEY_APP_RESET_REASON, INI_VALUE_INT);
    if (valueRead != NULL)
    {
        reason = (uint8_t)(*valueRead & 0xFF);
        free(valueRead);
        valueRead = NULL;
    }
    else
    {
        SYSLOG_DEBUG("No matched boot info: %s\r\n", BOOT_INFO_KEY_APP_RESET_REASON);
    }
#endif

    return reason;
}

static int32_t saveAppResetReason(uint8_t reason)
{
    int32_t  retVal     = -1;
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    int32_t  valueWrite = reason;

    retVal = iniKeyValueWrite(BOOT_INFO_FILE, BOOT_INFO_KEY_APP_RESET_REASON, INI_VALUE_INT, &valueWrite);
    if (retVal != 0)
    {
        SYSLOG_DEBUG("Failed to write reset reason: %d\r\n", retVal);
    }
#endif

    return retVal;
}

static int32_t clearAppResetReason(void)
{
    int32_t  retVal     = -1;
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    int32_t  valueWrite = APP_RESET_REASON_MAX;

    retVal = iniKeyValueWrite(BOOT_INFO_FILE, BOOT_INFO_KEY_APP_RESET_REASON, INI_VALUE_INT, &valueWrite);
    if (retVal != 0)
    {
        SYSLOG_DEBUG("Failed to write reset reason: %d\r\n", retVal);
    }
#endif

    return retVal;
}

uint8_t powerManagerBootProcess(void)
{
    LastResetState_e apRstState = 0;
    LastResetState_e cpRstState = 0;
    int32_t          timeout    = 2500;
    uint32_t         interval   = 100;
    slpManWakeSrc_e  wakeupSrc  = slpManGetWakeupSrc();
    uint8_t          vbus       = powerManagerGetVbusState();
    uint8_t          reason     = getAppResetReason();

    ResetStateGet(&apRstState, &cpRstState);
    SYSLOG_DEBUG("apRstState=%d, wakeupSrc=%d, vbus=%d, reason=%d\r\n", apRstState, wakeupSrc, vbus, reason);
    if (apRstState == LAST_RESET_CLEAR)
    {
        if (wakeupSrc == WAKEUP_FROM_PWRKEY)
        {
            do
            {
                osDelay(interval);
                timeout -= interval;
                if (pwrKeyGetPinLevel() == true)
                {
                    SYSLOG_DEBUG("The power key wasn't pressed long enough.\r\n");
                    gBootResult = BOOT_RESULT_POWER_OFF;
                    break;
                }
            } while (timeout > 0);

            if (gBootResult != BOOT_RESULT_POWER_OFF)
            {
                SYSLOG_DEBUG("Power key boot.\r\n");
                gBootType   = BOOT_TYPE_POWER_KEY;
                gBootResult = BOOT_RESULT_POWER_ON;
            }
        }
        else if (wakeupSrc == WAKEUP_FROM_PAD)
        {
            if (vbus == 1)
            {
                SYSLOG_DEBUG("Charging boot.\r\n");
                gBootType   = BOOT_TYPE_CHARGING;
                gBootResult = BOOT_RESULT_POWER_OFF_CHARGING;
            }
            else
            {
                SYSLOG_DEBUG("Other pad wakeup are ignored.\r\n");
                gBootResult = BOOT_RESULT_POWER_OFF;
            }
        }
        else if (wakeupSrc == WAKEUP_FROM_CHARG)
        {
            SYSLOG_DEBUG("Charging boot.\r\n");
            gBootType   = BOOT_TYPE_CHARGING;
            gBootResult = BOOT_RESULT_POWER_OFF_CHARGING;
        }
        else
        {
            SYSLOG_DEBUG("Other wakeup types are ignored.\r\n");
            gBootResult = BOOT_RESULT_POWER_OFF;
        }
    }
    else if (apRstState == LAST_RESET_POR)
    {
        SYSLOG_DEBUG("First power on.\r\n");
        gBootType   = BOOT_TYPE_POWER_KEY;
        gBootResult = BOOT_RESULT_POWER_ON;
    }
    else if ((apRstState == LAST_RESET_SWRESET) && (reason < APP_RESET_REASON_MAX))
    {
        if (reason == APP_RESET_REASON_CHARGING_POWER_ON)
        {
            SYSLOG_DEBUG("Boot up during power off charging.\r\n");
            gBootType   = (pwrKeyGetPinLevel() == false) ? BOOT_TYPE_POWER_KEY : BOOT_TYPE_OTHER;
            gBootResult = BOOT_RESULT_POWER_ON;
        }
        else if (reason == APP_RESET_REASON_CHARGING_POWER_OFF)
        {
            SYSLOG_DEBUG("Shut down during power on charging.\r\n");
            gBootType   = BOOT_TYPE_CHARGING;
            gBootResult = BOOT_RESULT_POWER_OFF_CHARGING;
        }
        clearAppResetReason();
    }
    else
    {
        SYSLOG_DEBUG("Other types of reset.\r\n");
        gBootType   = BOOT_TYPE_OTHER;
        gBootResult = BOOT_RESULT_POWER_ON;
    }

    SYSLOG_DEBUG("gBootType=%d, gBootResult=%d\r\n", gBootType, gBootResult);
    appSetCFUN((gBootResult == BOOT_RESULT_POWER_ON) ? 1 : 0);
    slpManSetPmuSleepMode(true, (gBootResult == BOOT_RESULT_POWER_OFF) ? SLP_HIB_STATE : SLP_SLP1_STATE, false);

    return gBootResult;
}

void powerManagerPowerKeyHandler(pwrKeyPressStatus status)
{
    static bool        sPowerKeyReady = false;
    static bool        sIgnoreEvent   = false;
    QueuePowerManagerT queue          = {.event = status};

    if (sPowerKeyReady == false)
    {
        ECPLAT_PRINTF(UNILOG_PLA_APP, powerManagerPowerKeyHandler5, P_DEBUG, "gBootType=%d, status=%d", gBootType, status);
        if ((gBootType == BOOT_TYPE_CHARGING) || (gBootType == BOOT_TYPE_OTHER))
        {
            ECPLAT_PRINTF(UNILOG_PLA_APP, powerManagerPowerKeyHandler10, P_DEBUG, "Power key ready.");
            sPowerKeyReady = true;
        }
        else if (gBootType == BOOT_TYPE_POWER_KEY)
        {
            if (status == PWRKEY_RELEASE)
            {
                ECPLAT_PRINTF(UNILOG_PLA_APP, powerManagerPowerKeyHandler15, P_DEBUG, "Power key ready.");
                sPowerKeyReady = true;
            }
            return;
        }
        else
        {
            ECPLAT_PRINTF(UNILOG_PLA_APP, powerManagerPowerKeyHandler20, P_DEBUG, "Unknown boot type.");
            return;
        }
    }

    ECPLAT_PRINTF(UNILOG_PLA_APP, powerManagerPowerKeyHandler25, P_DEBUG, "status=%d, sIgnoreEvent=%d, gState=%d, gPowerState=%d, gBootResult=%d", status, sIgnoreEvent, gState, gPowerState, gBootResult);
    if (status == PWRKEY_RELEASE)
    {
        if (sIgnoreEvent == false)
        {
            ECPLAT_PRINTF(UNILOG_PLA_APP, powerManagerPowerKeyHandler30, P_DEBUG, "Forward the event.");
            osMessageQueuePut(gPowerManagerQueue, &queue, 0, 0);
        }
        else
        {
            ECPLAT_PRINTF(UNILOG_PLA_APP, powerManagerPowerKeyHandler35, P_DEBUG, "Ignored the event.");
            sIgnoreEvent = false;
        }
    }
    else if(status == PWRKEY_LONGPRESS)
    {
        sIgnoreEvent = true;
        if (gBootResult == BOOT_RESULT_POWER_ON)
        {
            if (gState == POWERMANAGER_SAVE_POWER)
            {
                ECPLAT_PRINTF(UNILOG_PLA_APP, powerManagerPowerKeyHandler50, P_DEBUG, "Forward the event.");
                queue.event = PWRKEY_RELEASE;
                osMessageQueuePut(gPowerManagerQueue, &queue, 0, 0);
            }
            else
            {
                ECPLAT_PRINTF(UNILOG_PLA_APP, powerManagerPowerKeyHandler55, P_DEBUG, "Go to power off.");
                gGoToPowerOff = true;
                poweroff();
            }
        }
        else if (gBootResult == BOOT_RESULT_POWER_OFF_CHARGING)
        {
            ECPLAT_PRINTF(UNILOG_PLA_APP, powerManagerPowerKeyHandler60, P_DEBUG, "Reboot to power on.");
            queue.event = REBOOT_TO_POWER_ON;
            osMessageQueuePut(gPowerManagerQueue, &queue, 0, 0);
        }
    }
}

int32_t powerManagerSetState(uint8_t state)
{
    int32_t            retVal = -1;
    QueuePowerManagerT queue  = {.event = state};

    if (gPowerManagerQueue == NULL)
    {
        SYSLOG_DEBUG("gPowerManagerQueue is NULL.\r\n");
        goto labelEnd;
    }

    retVal = osMessageQueuePut(gPowerManagerQueue, &queue, 0, osWaitForever);

labelEnd:
    return retVal;
}

uint8_t powerManagerGetPowerState(void)
{
    return gPowerState;
}

static void restoreCallback(void *pdata, slpManLpState state)
{
    bool sleepedFlag = apmuGetSleepedFlag();

    if (sleepedFlag == true)
    {
        gPowerState = POWER_STATE_IN_SLEEP;
    }
    ECPLAT_PRINTF(UNILOG_PLA_APP, restoreCallback, P_DEBUG, "sleepedFlag=%d, gPowerState=%d", sleepedFlag, gPowerState);
}

static void enterPowerFull(void)
{
    int32_t resVal = -1;

    SYSLOG_DEBUG("Begin.\r\n");

    stopSleepDelayTimer();
    hbPowerSaveTimerStop();

    if (gState == POWERMANAGER_FULL_POWER)
    {
        SYSLOG_DEBUG("Already in POWERMANAGER_FULL_POWER.\r\n");
        if (powerManagerGetBacklightState() == false)
        {
            lcdBacklightSet(true);
        }
        powerManagerResetBacklightTimer();
        goto labelEnd;
    }

    slpManPlatVoteDisableSleep(gVote, SLP_SLP1_STATE);

    resVal = Devices_resume(NULL);
    if (resVal != OPEN_HAL_DONE)
    {
        SYSLOG_DEBUG("Failed to resume devices: %d\r\n", resVal);
    }

    resVal = Services_resume(NULL);
    if (resVal < 0)
    {
        SYSLOG_DEBUG("Failed to resume Service: %d\r\n", resVal);
    }

    gPowerState = POWER_STATE_WORKING;
    gState      = POWERMANAGER_FULL_POWER;

#ifdef FEATURE_HAL_SCREEN_ENABLE
    lcdDisplay();
#endif

#ifdef FEATURE_SUBSYS_TELECOM_ENABLE
    if (gBootResult == BOOT_RESULT_POWER_ON)
    {
        ecsimcfgSimPresenceDetect(false);
        osDelay(100);
        ecsimcfgSimPresenceDetect(true);
    }
#endif
    osDelay(100);
    lcdBacklightSet(true);
    powerManagerResetBacklightTimer();

labelEnd:
    SYSLOG_DEBUG("End.\r\n");
}

static void enterPowerNormal(void)
{
    int32_t resVal = -1;

    SYSLOG_DEBUG("Begin.\r\n");

    if (gState == POWERMANAGER_NORMAL_POWER)
    {
        SYSLOG_DEBUG("Already in POWERMANAGER_NORMAL_POWER.\r\n");
        goto labelEnd;
    }

    for (uint32_t i=0; i<POWER_MANAGER_LIST_SIZE; i++)
    {
        if (gPowerManagerList[i].normal != NULL)
        {
            resVal = gPowerManagerList[i].normal();
            if (resVal != 0)
            {
                SYSLOG_DEBUG("Power normal failed: name=%s, resVal=%d\r\n", gPowerManagerList[i].name, resVal);
            }
        }
    }

    gState = POWERMANAGER_NORMAL_POWER;

labelEnd:
    SYSLOG_DEBUG("End.\r\n");
}

static void enterPowerLow(void)
{
    int32_t resVal = -1;

    SYSLOG_DEBUG("Begin.\r\n");

    if (gState == POWERMANAGER_LOW_POWER)
    {
        SYSLOG_DEBUG("Already in POWERMANAGER_LOW_POWER.\r\n");
        goto labelEnd;
    }

    for (uint32_t i=0; i<POWER_MANAGER_LIST_SIZE; i++)
    {
        if (gPowerManagerList[i].low != NULL)
        {
            resVal = gPowerManagerList[i].low();
            if (resVal != 0)
            {
                SYSLOG_DEBUG("Power low failed: name=%s, resVal=%d\r\n", gPowerManagerList[i].name, resVal);
            }
        }
    }

    gState = POWERMANAGER_LOW_POWER;

labelEnd:
    SYSLOG_DEBUG("End.\r\n");
}

static void enterPowerSave(void)
{
    int32_t resVal      = -1;
    uint8_t prohibition = 0;

    SYSLOG_DEBUG("Begin.\r\n");

    if (gState == POWERMANAGER_SAVE_POWER)
    {
        if (gGoToPowerOff == true)
        {
            slpManSetPmuSleepMode(true, SLP_HIB_STATE, false);
            SYSLOG_DEBUG("Power off.\r\n");
        }
        else
        {
            SYSLOG_DEBUG("Already in POWERMANAGER_SAVE_POWER.\r\n");
        }
        goto labelEnd;
    }

    prohibition = powerManagerProhibitionQuery();
    SYSLOG_DEBUG("prohibition=%d\r\n", prohibition);
    if (prohibition == PROHIBIT_TURN_OFF_BACKLIGHT)
    {
        goto labelEnd;
    }
    else if (prohibition == PROHIBIT_ENTER_SLEEP)
    {
        lcdBacklightSet(false);
        goto labelEnd;
    }

    gPowerState = POWER_STATE_GO_TO_SLEEP;

    if (osTimerIsRunning(gOsTimerId) != 0)
    {
        osTimerStop(gOsTimerId);
    }

    resVal = Services_suspend(NULL);
    if (resVal < 0)
    {
        SYSLOG_DEBUG("Failed to suspend service: %d\r\n", resVal);
    }

    resVal = Devices_suspend(NULL);
    if (resVal != OPEN_HAL_DONE)
    {
        SYSLOG_DEBUG("Failed to suspend devices: %d\r\n", resVal);
    }

    slpManPlatVoteEnableSleep(gVote, SLP_SLP1_STATE);
#ifdef FEATURE_SUBSYS_TELECOM_ENABLE
    if (gBootResult == BOOT_RESULT_POWER_ON)
    {
        ecsimcfgSimPresenceDetect(true);
        osDelay(100);
        ecsimcfgSimPresenceDetect(false);
        ecconnrel();
    }
#endif
    osDelay(100);

    if (gGoToPowerOff == true)
    {
        lcdBacklightSet(false);
        if (powerManagerGetVbusState() == 1)
        {
            SYSLOG_DEBUG("Reboot to power off charging.\r\n");
            saveAppResetReason(APP_RESET_REASON_CHARGING_POWER_OFF);
            ResetECSystemReset();
        }
        else
        {
            slpManSetPmuSleepMode(true, SLP_HIB_STATE, false);
            SYSLOG_DEBUG("Power off.\r\n");
        }
    }

    gState = POWERMANAGER_SAVE_POWER;

labelEnd:
    SYSLOG_DEBUG("End.\r\n");
}

static void hbPowerSaveTimerStop(void)
{
    for (uint32_t i=0; i<HB_POWER_SAVE_LIST_SIZE; i++)
    {
        if (gHbPowerSaveList[i].timerId != NULL)
        {
            if (osTimerIsRunning(gHbPowerSaveList[i].timerId) != 0)
            {
                osTimerStop(gHbPowerSaveList[i].timerId);
            }
        }
    }
}

static void hbPowerSaveSleepCallback(void)
{
    for(uint32_t i = 0; i < HB_POWER_SAVE_LIST_SIZE; i++)
    {
        if((gHbPowerSaveList[i].timerId != NULL) &&
           (gHbPowerSaveList[i].hbPowerSave.sleepCallback.callback != NULL))
        {
            gHbPowerSaveList[i].hbPowerSave.sleepCallback.callback(
                gHbPowerSaveList[i].hbPowerSave.sleepCallback.param, 0);
        }
    }
}

static void hbPowerSaveLcdCallback(bool backlight_on)
{
    for(uint32_t i = 0; i < HB_POWER_SAVE_LIST_SIZE; i++)
    {
        if((gHbPowerSaveList[i].timerId != NULL) &&
           (gHbPowerSaveList[i].hbPowerSave.lcdCallback.callback != NULL))
        {
            gHbPowerSaveList[i].hbPowerSave.lcdCallback.callback(
                gHbPowerSaveList[i].hbPowerSave.lcdCallback.param,
                backlight_on ? 1 : 0);
        }
    }
}

static void enterHbPowerSave(void)
{
    SYSLOG_DEBUG("Begin.\r\n");

    if (gGoToPowerOff != true)
    {
        for (uint32_t i=0; i<HB_POWER_SAVE_LIST_SIZE; i++)
        {
            if (gHbPowerSaveList[i].timerId != NULL)
            {
                if (osTimerIsRunning(gHbPowerSaveList[i].timerId) != 0)
                {
                    osTimerStop(gHbPowerSaveList[i].timerId);
                }
                osTimerStart(gHbPowerSaveList[i].timerId, gHbPowerSaveList[i].hbPowerSave.timerTime);
            }
        }
    }

    hbPowerSaveSleepCallback();
    enterPowerSave();

    SYSLOG_DEBUG("End.\r\n");
}

static void hbPowerSaveTimer(void *argument)
{
    gHbPowerSaveList[(uint32_t)argument].hbPowerSave.timerCallback.callback(
        gHbPowerSaveList[(uint32_t)argument].hbPowerSave.timerCallback.param,
        0);
}

int32_t powerManagerHbPowerSaveCreate(HbPowerSaveT *hbPowerSave)
{
    int32_t retVal = -1;

    for (uint32_t i=0; i<HB_POWER_SAVE_LIST_SIZE; i++)
    {
        if (gHbPowerSaveList[i].timerId == NULL)
        {
            gHbPowerSaveList[i].timerId = osTimerNew(hbPowerSaveTimer, osTimerPeriodic, (void *)i, NULL);
            if (gHbPowerSaveList[i].timerId == NULL)
            {
                SYSLOG_DEBUG("Failed to create timer for gHbPowerSaveList[%d].timerId.\r\n", i);
                goto labelEnd;
            }

            if (hbPowerSave != NULL)
            {
                memcpy(&gHbPowerSaveList[i].hbPowerSave, hbPowerSave, sizeof(HbPowerSaveT));
            }
            retVal = i;

            goto labelEnd;
        }
    }

    SYSLOG_DEBUG("Hb Power save list is full: %d\r\n", HB_POWER_SAVE_LIST_SIZE);

labelEnd:
    return retVal;
}

int32_t powerManagerHbPowerSaveDelete(int32_t handle)
{
    int32_t retVal = -1;

    if ((handle < 0) || (handle >= HB_POWER_SAVE_LIST_SIZE) || (gHbPowerSaveList[handle].timerId == NULL))
    {
        SYSLOG_DEBUG("Param error.\r\n");
        goto labelEnd;
    }

    if (osTimerDelete(gHbPowerSaveList[handle].timerId) == osOK)
    {
        memset(&gHbPowerSaveList[handle], 0, sizeof(HbPowerSaveT));
        retVal = 0;
    }
    else
    {
        SYSLOG_DEBUG("Failed to delete timer.\r\n");
    }

labelEnd:
    return retVal;
}

int32_t powerManagerHbPowerSaveIoctl(int32_t handle, int32_t type, void *param)
{
    int32_t retVal = -1;

    if ((handle < 0) || (handle >= HB_POWER_SAVE_LIST_SIZE) || (gHbPowerSaveList[handle].hbPowerSave.timerTime == 0) || (type >= HB_POWER_SAVE_IOCTL_MAX) || (param == NULL))
    {
        SYSLOG_DEBUG("Param error.\r\n");
        goto labelEnd;
    }

    switch (type)
    {
        case HB_POWER_SAVE_IOCTL_TIMER_TIME:
            if (*((uint32_t *)param) <= 0)
            {
                SYSLOG_DEBUG("Param error.\r\n");
                goto labelEnd;
            }
            gHbPowerSaveList[handle].hbPowerSave.timerTime = *((uint32_t *)param);
            if (osTimerIsRunning(gHbPowerSaveList[handle].timerId) != 0)
            {
                osTimerStop(gHbPowerSaveList[handle].timerId);
                osTimerStart(gHbPowerSaveList[handle].timerId, gHbPowerSaveList[handle].hbPowerSave.timerTime);
            }
            break;

        case HB_POWER_SAVE_IOCTL_TIMER_CALLBACK:
            gHbPowerSaveList[handle].hbPowerSave.timerCallback.callback = (HbPowerSaveFuncT)param;
            break;

        case HB_POWER_SAVE_IOCTL_TIMER_CALLBACK_PARAM:
            gHbPowerSaveList[handle].hbPowerSave.timerCallback.param = param;
            break;

        case HB_POWER_SAVE_IOCTL_SLEEP_CALLBACK:
            gHbPowerSaveList[handle].hbPowerSave.sleepCallback.callback = (HbPowerSaveFuncT)param;
            break;

        case HB_POWER_SAVE_IOCTL_SLEEP_CALLBACK_PARAM:
            gHbPowerSaveList[handle].hbPowerSave.sleepCallback.param = param;
            break;

        default:
            SYSLOG_DEBUG("Unknown type.\r\n");
            goto labelEnd;
            break;
    }

    retVal = 0;

labelEnd:
    return retVal;
}

#ifdef HB_POWER_SAVE_TEST
static void hbPowerSaveTestTimerCallback(void *argument)
{
    SYSLOG_DEBUG("%d\r\n", *((uint32_t *)argument));
}

static void hbPowerSaveTestSleepCallback(void *argument)
{
    SYSLOG_DEBUG("%d\r\n", *((uint32_t *)argument));
}

static void hbPowerSaveTest(void)
{
    static uint32_t timerCallbackParam = 67890;
    static uint32_t sleepCallbackParam = 12345;
    HbPowerSaveT hbPowerSave =
    {
        .timerTime              = 1000,
        .timerCallback.callback = hbPowerSaveTestTimerCallback,
        .timerCallback.param    = &timerCallbackParam,
        .sleepCallback.callback = hbPowerSaveTestSleepCallback,
        .sleepCallback.param    = &sleepCallbackParam,
    };

    powerManagerHbPowerSaveCreate(&hbPowerSave);
}
#endif

static int32_t ceregTimerInit(void)
{
    int32_t retVal = -1;

    if (gCeregTimerId == NULL)
    {
        gCeregTimerId = osTimerNew((osTimerFunc_t)sendCommonAt, osTimerOnce, AT_CEREG, NULL);
        if (gCeregTimerId == NULL)
        {
            SYSLOG_DEBUG("Failed to create timer for gCeregTimerId.\r\n");
            goto labelEnd;
        }
        else
        {
            osTimerStart(gCeregTimerId, CEREG_CHECK_TIME);
        }
    }

    retVal = 0;

labelEnd:
    return retVal;
}

static void threadPowerManager(void *argument)
{
    QueuePowerManagerT queue = {0};
    AppMsgT            msg   = {0};

    memset(gPowerManagerList, 0, sizeof(gPowerManagerList));

    if (gPowerManagerQueue == NULL)
    {
        gPowerManagerQueue = osMessageQueueNew(QUEUE_SIZE_POWER_MANAGER, sizeof(queue), NULL);
        if (gPowerManagerQueue == NULL)
        {
            SYSLOG_EMERG("Failed to create queue for gPowerManagerQueue.\r\n");
            goto labelEnd;
        }
    }

    if (gOsTimerId == NULL)
    {
        gOsTimerId = osTimerNew(osTimerFunc, osTimerOnce, NULL, NULL);
        if (gOsTimerId == NULL)
        {
            SYSLOG_DEBUG("Failed to create timer for gOsTimerId.\r\n");
            goto labelEnd;
        }
        else
        {
            SYSLOG_DEBUG("gBacklightDuration=%d\r\n", gBacklightDuration);
            osTimerStart(gOsTimerId, gBacklightDuration);
        }
    }

    if (gSleepDelayTimer == NULL)
    {
        gSleepDelayTimer = osTimerNew(sleepDelayTimer, osTimerOnce, NULL, NULL);
        if (gSleepDelayTimer == NULL)
        {
            SYSLOG_DEBUG("Failed to create timer for gSleepDelayTimer.\r\n");
            goto labelEnd;
        }
    }

    slpManApplyPlatVoteHandle("powerManager", &gVote);
    slpManRegisterUsrdefinedRestoreCb(restoreCallback, NULL);

    typedef int (*pfn_PadWakeupHook)(uint32_t pad_num);
    extern int RegAppPadWakeupIntrHook(pfn_PadWakeupHook pfunc);
    RegAppPadWakeupIntrHook(padWakeupHook);

#ifdef FEATURE_SUBSYS_TELECOM_ENABLE
    if (gBootResult == BOOT_RESULT_POWER_ON)
    {
        powerManagerProhibitionCreate(&gProhibitionHandle);
        SYSLOG_DEBUG("gProhibitionHandle=%d\r\n", gProhibitionHandle);
        telecomCallbackRegister(telecomCallback);
        ceregTimerInit();
    }
#endif

    while (1)
    {
        memset(&queue, 0, sizeof(queue));
        if (osMessageQueueGet(gPowerManagerQueue, &queue, 0, osWaitForever) == osOK)
        {
            SYSLOG_DEBUG("event=%d, gState=%d, gBacklightOn=%d\r\n", queue.event, gState, gBacklightOn);
            switch (queue.event)
            {
                case PWRKEY_RELEASE:
                    if (powerManagerGetBacklightState() == false)
                    {
                        statusNotify();
                        enterPowerFull();
                        hbPowerSaveLcdCallback(true);
                    }
                    else if ((mainApp.msgProc == normalAppMsgProc)
#ifdef FEATURE_SUBSYS_POWEROFF_MODE_ENABLE
                          || (mainApp.msgProc == poweroffAppMsgProc)
#endif
                            )
                    {
                        lcdBacklightSet(false);
                        resetSleepDelayTimer();
                        hbPowerSaveLcdCallback(false);
                    }
                    else
                    {
                        SYSLOG_DEBUG("Forward powerkey event.\r\n");
                        msg.msgType = APP_KEY_MSG;
                        msg.param1  = FUNC_CODE_NO;
                        appSendMsg(&msg);
#ifdef FEATURE_SUBSYS_INPUT_ENABLE
                        inputNotify();
#endif
                    }
                    break;

                case POWERMANAGER_FULL_POWER:
                    enterPowerFull();
                    break;

                case POWERMANAGER_NORMAL_POWER:
                    enterPowerNormal();
                    break;

                case POWERMANAGER_LOW_POWER:
                    enterPowerLow();
                    break;

                case POWERMANAGER_HEARTBEAT_SAVE_POWER:
                    enterHbPowerSave();
                    break;

                case POWERMANAGER_SAVE_POWER:
                    enterPowerSave();
                    break;

                case VBUS_CHANGE:
                    if (gBootResult == BOOT_RESULT_POWER_ON)
                    {
                        statusNotify();
                        enterPowerFull();
                    }
                    else if (gBootResult == BOOT_RESULT_POWER_OFF_CHARGING)
                    {
                        if (powerManagerGetVbusState() == 0)
                        {
                            SYSLOG_DEBUG("Exit power off charging and go to power off.\r\n");
                            gGoToPowerOff = true;
                            powerManagerSetState(powerManagerGetCurrnetPowerSaveMode());
                        }
                    }
                    break;

                case REBOOT_TO_POWER_ON:
                    SYSLOG_DEBUG("Reboot to power on.");
                    saveAppResetReason(APP_RESET_REASON_CHARGING_POWER_ON);
                    ResetECSystemReset();
                    break;

                default:
                    break;
            }
        }
    }

labelEnd:
    osThreadExit();
}

void powerManagerInit(void)
{
    osThreadAttr_t threadAttr = {0};

    if (gPowerManagerThread == NULL)
    {
        memset(&threadAttr, 0, sizeof(threadAttr));
        threadAttr.name       = "threadPowerManager";
        threadAttr.priority   = osPriorityNormal;
        threadAttr.stack_size = THREAD_STACK_SIZE_POWER_MANAGER;
#ifdef THREAD_STATIC
        threadAttr.stack_mem  = gPowerManagerThreadStackMem;
        threadAttr.cb_mem     = &gPowerManagerThreadCbMem;
        threadAttr.cb_size    = sizeof(StaticTask_t);
#endif
        gPowerManagerThread = osThreadNew(threadPowerManager, NULL, &threadAttr);
        if (gPowerManagerThread == NULL)
        {
            SYSLOG_EMERG("Failed to create thread for powerManager.\r\n");
        }
    }
}
#endif
