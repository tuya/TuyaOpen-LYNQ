#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "ps_lib_api.h"
#include "hal_charge.h"
#include "hal_adc.h"
#include "adc.h"

#include "ostask.h"
#include "osasys.h"
#include "bsp.h"
#include "bsp_custom.h"
#include "servicemanager.h"
#include "status.h"
#ifdef FEATURE_SUBSYS_MQTT_ONENET_ENABLE
#include "onenet_mqtt.h"
#endif
#ifdef FEATURE_SUBSYS_MISC_ENABLE
#include "misc.h"
#endif
#ifdef FEATURE_SUBSYS_INPUT_ENABLE
#include "input.h"
#endif
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_SUBSYS_SYSTIME_ENABLE
#include "systime.h"
#endif
#ifdef FEATURE_SUBSYS_POWER_MANAGER_ENABLE
#include "powermanager.h"
#endif


#define QUEUE_SIZE_STATUS           100
#define VBAT_TIMEOUT_MS             5
#define REPORT_INTERNAL_MS          (5 * 60 * 1000)
#define REPORT_FORMAT               "{\"%s\":{\"cellId\":%d,\"rssi\":%d,\"timestamp\":\"%d\",\"vBat\":%d}}"


typedef struct
{
    uint32_t cellId;
    int16_t  rssi;
    uint32_t timestamp;
    uint32_t vBat;
} ReportT;


utc_timer_value_t         gUtcTime         = {0};
StatusT                   gStatus          = {0};
#ifdef STATUS_REPORT
static ReportT            gReport          = {0};
#endif
static uint32_t           gStatusBarExist  = 0;
static osMessageQueueId_t gStatusQueue     = NULL;
static osSemaphoreId_t    gStatusSemaphore = NULL;
static volatile uint32_t  gVBatValue       = 0;
static volatile bool      gVBatDone        = false;
static volatile bool      gNeedReport      = false;
static TimerHandle_t      gReportTimer     = NULL;
static volatile uint8_t   gBatteryCharge   = CHARGE_STATUS_DISCONNECT;
static uint8_t            gCsq             = 99;


uint32_t getStatusBarExist(void)
{
	return gStatusBarExist;
}

void setStatusBarExist(int32_t flag)
{
	gStatusBarExist = flag;
}

#ifdef STATUS_REPORT
static void reportTimerCallback(TimerHandle_t xTimer)
{
    gNeedReport = true;
    statusNotify();
}

static int32_t statusReport(ReportT *report)
{
    int32_t retVal      = -1;
#if defined(FEATURE_SUBSYS_MQTT_ONENET_ENABLE)
    char    buffer[256] = {0};

    if (onenetMqttIsReady() != true)
    {
        gNeedReport = true;
        goto labelEnd;
    }

    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), REPORT_FORMAT, ONENET_MQTT_TOPIC_SPEAKER, report->cellId, report->rssi, report->timestamp, report->vBat);
    retVal = onenetMqttPub(buffer);
    SYSLOG_DEBUG("Status report%s %s\r\n", ((retVal == 0) ? ":" : " failed:"), buffer);
#endif

labelEnd:
    return retVal;
}

static int32_t reportInit(void)
{
    int32_t retVal = 0;

    gReportTimer = xTimerCreate("reportTimer", (REPORT_INTERNAL_MS / portTICK_PERIOD_MS), pdTRUE, NULL, reportTimerCallback);
    if (gReportTimer == NULL)
    {
        SYSLOG_EMERG("Failed to create timer for gReportTimer.\r\n");
        retVal = -1;
    }
    else
    {
        xTimerStart(gReportTimer, portMAX_DELAY);
        gNeedReport = true;
    }

    return retVal;
}

static uint8_t batteryVoltageToLevel(uint32_t voltage)
{
         if (voltage >= 4000000)    {return 100;}
    else if (voltage >= 3850000)    {return 75;}
    else if (voltage >= 3600000)    {return 50;}
    else if (voltage >= 3300000)    {return 25;}
    else                            {return 0;}
}
#endif

static void chargeStateUpdate(void)
{
#ifdef FEATURE_SUBSYS_MISC_ENABLE
#if defined(SPEAKER_DEVICE)
    if (gBatteryCharge == CHARGE_STATUS_DISCONNECT)
    {
        if (chargeIsOn() == true)
        {
            gBatteryCharge = CHARGE_STATUS_FINISH;
        }
        else
        {
            blueLedSetState(true);
        }
    }
    else if (gBatteryCharge == CHARGE_STATUS_CHARGING)
    {
        blueLedSetState(false);
    }
    else if ((gBatteryCharge == CHARGE_STATUS_FINISH) && (chargeIsOn() == false))
    {
        gBatteryCharge = CHARGE_STATUS_DISCONNECT;
        blueLedSetState(true);
    }
#elif defined(FEATURE_SUBSYS_POWER_MANAGER_ENABLE)
    gBatteryCharge = (powerManagerGetVbusState() == 0) ? CHARGE_STATUS_DISCONNECT : CHARGE_STATUS_CHARGING;
#else
    osDelay(500);
    if((gBatteryCharge == CHARGE_STATUS_DISCONNECT) && chargeIsOn())
    {
        gBatteryCharge = CHARGE_STATUS_FINISH;
    }
    else if((gBatteryCharge == CHARGE_STATUS_FINISH) && (!chargeIsOn()))
    {
        gBatteryCharge = CHARGE_STATUS_DISCONNECT;
    }
#endif
#endif
}

#ifndef SPEAKER_APP
static void chargeEvent(chargeStatus_e status)
{
    gBatteryCharge = status;
    statusNotify();
#ifdef FEATURE_SUBSYS_POWER_MANAGER_ENABLE
    powerManagerVbusHandler();
#endif
}
#endif

static void vBatCallback(uint32_t result)
{
    gVBatDone  = true;
    gVBatValue = result;
}

static void vBatInit(void)
{
    AdcConfig_t adcConfig = {0};

    memset(&adcConfig, 0, sizeof(adcConfig));
    ADC_getDefaultConfig(&adcConfig);
    adcConfig.channelConfig.vbatResDiv = ADC_VBAT_RESDIV_RATIO_8OVER32;
    ADC_channelInit(ADC_CHANNEL_VBAT, ADC_USER_APP, &adcConfig, vBatCallback);
}

static int32_t vBatGet(void)
{
    int32_t  retVal = -1;
    uint32_t time   = 0;

    ADC_startConversion(ADC_CHANNEL_VBAT, ADC_USER_APP);

    while ((gVBatDone != true) && (time++ < VBAT_TIMEOUT_MS))
    {
        osDelay(1);
    }
    if (gVBatDone == true)
    {
        gVBatDone = false;
        retVal = HAL_ADC_CalibrateRawCode(gVBatValue) * 4;
    }
    else
    {
        SYSLOG_ERR("Get vBat timeout: %dms\r\n", VBAT_TIMEOUT_MS);
    }

    return retVal;
}

static uint8_t csqGet(void)
{
    uint8_t csq  = 99;
    int8_t  snr  = 0;
    int8_t  rsrp = 0;
    int8_t  rsrq = 0;

#ifdef FEATURE_SUBSYS_MISC_ENABLE
    if (simGetStatus(0) == SIM_READY)
#endif
    {
        if (appGetSignalQualitySync(&csq, &snr, &rsrp, &rsrq) != CMS_RET_SUCC)
        {
            csq = 99;
        }
    }

    return csq;
}

#ifdef STATUS_REPORT
static int16_t rssiGet(void)
{
    CmiDevGetExtStatusCnf status = {0};

    memset(&status, 0, sizeof(status));
    appGetUeExtStatusInfoSync(UE_EXT_STATUS_PHY, &status);

    return (status.phyStatus.rssi / 100);
}

static uint32_t cellIdGet(void)
{
    CmiDevGetExtStatusCnf status = {0};

    memset(&status, 0, sizeof(status));
    appGetUeExtStatusInfoSync(UE_EXT_STATUS_ERRC, &status);
    return status.rrcStatus.cellId;
}

static int32_t timeGet(utc_timer_value_t *time, char *timeStr, uint32_t size)
{
    int32_t retVal = -1;

    memset(time, 0, sizeof(utc_timer_value_t));
    retVal = appGetSystemTimeUtcSync(time);
    if (retVal != CMS_RET_SUCC)
    {
        SYSLOG_ERR("Failed to get time: %d\r\n", retVal);
    }
    else
    {
        memset(timeStr, 0, size);
        snprintf(timeStr, size, "%02u:%02u", (((time->UTCtimer2 >> 24) & 0xFF) + 8) % 24, ((time->UTCtimer2 >> 16) & 0xFF));
    }

    return retVal;
}
#endif

void subStatusTask(void *argument)
{
    gStatusQueue = osMessageQueueNew(QUEUE_SIZE_STATUS, sizeof(StatusT), NULL);
    if (gStatusQueue == NULL)
    {
        SYSLOG_EMERG("Failed to create queue for gStatusQueue.\r\n");
        goto labelEnd;
    }

    gStatusSemaphore = osSemaphoreNew(1, 1, NULL);
    if (gStatusSemaphore == NULL)
    {
        SYSLOG_EMERG("Failed to create semaphore for gStatusSemaphore.\r\n");
        goto labelEnd;
    }

#ifndef SPEAKER_APP
    chargeDetectInit(chargeEvent, false, 0);
#endif
    vBatInit();
#ifdef STATUS_REPORT
    reportInit();
#endif

    while(1)
    {
        osSemaphoreAcquire(gStatusSemaphore, osWaitForever);
        chargeStateUpdate();
        memset(&gStatus, 0, sizeof(StatusT));
#ifdef FEATURE_SUBSYS_MISC_ENABLE
        gStatus.simStatus      = simGetStatus(0);
        gStatus.nwReady        = nwIsReady();
#endif
        gStatus.batteryCharge  = gBatteryCharge;
        gStatus.batteryVoltage = vBatGet();
        gCsq                   = csqGet();

#ifdef STATUS_REPORT
        gStatus.batteryLevel   = batteryVoltageToLevel(gStatus.batteryVoltage);
#ifdef FEATURE_SUBSYS_MQTT_ONENET_ENABLE
        gStatus.serverReady    = onenetMqttIsReady();
#endif
        gStatus.rssi           = (gStatus.simStatus == SIM_READY) ? rssiGet() : 0x8000;
        timeGet(&gUtcTime, gStatus.time, sizeof(gStatus.time));
#else
        gStatus.batteryLevel   = getBatteryPercentage();
        gStatus.csqLevel       = getCsqPercentage();
#ifdef FEATURE_SUBSYS_SYSTIME_ENABLE
        time_time((time_t *)(gStatus.time));
#endif
#endif

        osMessageQueuePut(gStatusQueue, &gStatus, 0, 0);
#ifdef FEATURE_SUBSYS_INPUT_ENABLE
        inputNotify();
#endif

#ifdef STATUS_REPORT
        if (gNeedReport == true)
        {
            gNeedReport        = false;
            gReport.cellId    = cellIdGet();
            gReport.rssi      = gStatus.rssi;
            gReport.timestamp = gUtcTime.UTCsecs;
            gReport.vBat      = gStatus.batteryVoltage;
            statusReport(&gReport);
        }
#endif
    }

labelEnd:
#if 0 // Service Manager
    osThreadExit();
#else
    Service_stop("service:/status");
#endif
}

void subStatusInit()
{
    osThreadAttr_t taskAttr;

    memset(&taskAttr,0,sizeof(taskAttr));
    memset(subsys_status_task_stack, 0xA5,SUBSYS_STATUS_TASK_STACK_SIZE);
    taskAttr.name = "status";
    taskAttr.stack_mem = subsys_status_task_stack;
    taskAttr.stack_size = SUBSYS_STATUS_TASK_STACK_SIZE;
    taskAttr.priority = osPriorityNormal;
    taskAttr.cb_mem = &subsysStatusTask;//task control block
    taskAttr.cb_size = sizeof(StaticTask_t);//size of task control block

#if 0
    osThreadNew(subStatusTask, NULL, &taskAttr);
#else
    char serviceName[32] = {0};
    snprintf(serviceName, sizeof(serviceName), "service:/%s", taskAttr.name);
    Service_reg(serviceName, subStatusTask, NULL, taskAttr.cb_mem, taskAttr.cb_size, taskAttr.stack_mem, taskAttr.stack_size, taskAttr.priority);
    Service_start(serviceName);
#endif
}

osStatus_t statusGet(StatusT *p_status, uint32_t timeout)
{
    return osMessageQueueGet(gStatusQueue, p_status, 0, timeout);
}

void statusUpdateReportInternal(uint32_t internal)
{
    if (gReportTimer != NULL)
    {
        xTimerChangePeriod(gReportTimer, internal * 1000 / portTICK_PERIOD_MS, portMAX_DELAY);
    }
    else
    {
        SYSLOG_ERR("Status report timer is invalid.\r\n");
    }
}

int32_t statusNotify(void)
{
    int32_t retVal = -1;

    if (gStatusSemaphore != NULL)
    {
        osSemaphoreRelease(gStatusSemaphore);
        retVal = 0;
    }

    return retVal;
}

uint8_t getBatteryPercentage(void)
{
    uint8_t retVal = 0;

         if (gStatus.batteryVoltage > 4000000)      {retVal = 100;}
    else if (gStatus.batteryVoltage > 3870000)      {retVal = 80;}
    else if (gStatus.batteryVoltage > 3790000)      {retVal = 60;}
    else if (gStatus.batteryVoltage > 3730000)      {retVal = 40;}
    else if (gStatus.batteryVoltage > 3500000)      {retVal = 20;}
    else                                            {retVal = 0;}

    // SYSLOG_DEBUG("Battery Percentage: %d\r\n", retVal);

    return retVal;
}

uint8_t getCsqPercentage(void)
{
    uint8_t retVal = 0;

    if (gCsq != 99)
    {
        retVal = (gCsq / 8 + 1) * 25;
    }

    // SYSLOG_DEBUG("CSQ Percentage: %d, %d\r\n", gCsq, retVal);

    return retVal;
}
