/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    app.c
 * Description:  EC618 lwm2m demo entry source file
 * History:      Rev1.0   2018-10-12
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "ostask.h"
#include "osasys.h"
#include "bsp.h"
#include "bsp_custom.h"

#include "cmsis_os2.h"
#include "timers.h"
#include "cmisim.h"
#include "cmips.h"
#include "cmidev.h"
#include "networkmgr.h"
#include "sntp.h"
#include "ps_nm_if.h"
#include "ps_lib_api.h"
#include "ps_event_callback.h"
#include "slpman.h"
#include "hal_charge.h"
#ifdef FEATURE_SUBSYS_AUDIO_ENABLE
#include "audio.h"
#endif
#ifdef FEATURE_SUBSYS_STATUS_ENABLE
#include "status.h"
#endif
#include "misc.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_SUBSYS_SYSTIME_ENABLE
#include "systime.h"
#endif


#define EPAT_TRACE(subId, argLen, format,  ...)  \
    ECOMM_TRACE(UNILOG_MISC, subId, P_VALUE, argLen, format,  ##__VA_ARGS__) 


#define SNTP_SERVER                 "ntp.aliyun.com"

#if defined CHIP_EC718
#define LED_APWM_PAD                48
#define LED_APWM_GPIO               23
#define LED_APWM_IDX                0
#elif defined CHIP_EC716
#define LED_APWM_PAD                27
#define LED_APWM_GPIO               15
#define LED_APWM_IDX                2
#endif
#define LED_APWM_PORT               ((LED_APWM_GPIO) / 16)
#define LED_APWM_PIN                ((LED_APWM_GPIO) % 16)

#define BOOT_PAD                    15
#define BOOT_GPIO                   0
#define BOOT_PORT                   ((BOOT_GPIO) / 16)
#define BOOT_PIN                    ((BOOT_GPIO) % 16)


static bool     gNwReady   = false;
static bool     gWakeup    = false;
static uint32_t gSimStatus = PS_URC_GROUP_SIM;


#ifdef PHONE_DEVICE
static void GPIO_ISR(void)
{
    //Save current irq mask and diable whole port interrupts to get rid of interrupt overflow
    uint16_t portIrqMask = GPIO_saveAndSetIrqMask(BOOT_PORT);

    if (GPIO_getInterruptFlags(BOOT_PORT) & (1 << BOOT_PIN))
    {
        GPIO_clearInterruptFlags(BOOT_PORT, 1 << BOOT_PIN);
        statusNotify();
    }

    GPIO_restoreIrqMask(BOOT_PORT, portIrqMask);
}
#endif

static void bootPinInit(void)
{
    PadConfig_t     padConfig = {0};
    GpioPinConfig_t pinConfig = {0};

    PAD_getDefaultConfig(&padConfig);
    padConfig.mux = PAD_MUX_ALT0;
#ifndef PHONE_DEVICE
    padConfig.pullSelect = PAD_PULL_INTERNAL;
    padConfig.pullDownEnable = PAD_PULL_DOWN_ENABLE;
#endif
    PAD_setPinConfig(BOOT_PAD, &padConfig);

    pinConfig.pinDirection = GPIO_DIRECTION_INPUT;
#ifdef PHONE_DEVICE
    pinConfig.misc.interruptConfig = GPIO_INTERRUPT_BOTH_EDGE;
#endif
    GPIO_pinConfig(BOOT_PORT, BOOT_PIN, &pinConfig);

#ifdef PHONE_DEVICE
    XIC_SetVector(PXIC1_GPIO_IRQn, GPIO_ISR);
    XIC_EnableIRQ(PXIC1_GPIO_IRQn);
#endif
}

void APWMPostSleep(void *pdata, slpManLpState state)
{
    GpioPinConfig_t pinConfig = {0};
    pinConfig.pinDirection = GPIO_DIRECTION_OUTPUT;
    pinConfig.misc.initOutput = 0;
    GPIO_pinConfig(0, 15, &pinConfig);
}

void APWMPreSleep(void *pdata, slpManLpState state)
{
    slpManAONIOLatchEn(AonIOLatch_Enable);
}

void ledInit(void)
{
    slpManAONIOPowerOn();

    PadConfig_t padConfig = {0};
    PAD_getDefaultConfig(&padConfig);

    padConfig.mux = PAD_MUX_ALT0;
    PAD_setPinConfig(LED_APWM_PAD,  &padConfig);

    GpioPinConfig_t pinConfig = {0};
    pinConfig.pinDirection = GPIO_DIRECTION_OUTPUT;
    pinConfig.misc.initOutput = 0;
    GPIO_pinConfig(LED_APWM_PORT, LED_APWM_PIN, &pinConfig);

    slpManSetAPwmCfg(LED_APWM_IDX, APWM_MAX_PERIOD_4096MS, 0, 50);
    slpManSetAPwmEnable(LED_APWM_IDX, true);

    slpManRegisterUsrdefinedBackupCb(APWMPreSleep, NULL);
    slpManRegisterUsrdefinedRestoreCb(APWMPostSleep, NULL);
}

static int32_t psEventCallback(PsEventID eventID, void *param, UINT32 paramLen)
{
    switch (eventID)
    {
        case PS_URC_ID_PS_NETINFO:
#ifdef FEATURE_SUBSYS_STATUS_ENABLE
            statusNotify();
#endif
            if (((NmAtiNetInfoInd *)param)->netifInfo.netStatus == NM_NETIF_ACTIVATED)
            {
                if (gNwReady == false)
                {
                    SYSLOG_INFO("NW is ready.\r\n");
#ifdef SPEAKER_APP
                    openPlay(gOpenPlayer, NW_SOUND_READY);
#endif
#ifdef FEATURE_SUBSYS_SYSTIME_ENABLE
                    timeSync();
                    timeZoneSet(8);
#endif
                }
                gNwReady = true;
            }
            else
            {
                gNwReady = false;
                if (gSimStatus == PS_URC_ID_SIM_READY)
                {
                    SYSLOG_INFO("NW is unready.\r\n");
#ifdef SPEAKER_APP
                    openPlay(gOpenPlayer, NW_SOUND_UNREADY);
#endif
                }
            }
            break;

        case PS_URC_ID_SIM_READY:
#ifdef FEATURE_SUBSYS_STATUS_ENABLE
            statusNotify();
#endif
            gSimStatus = PS_URC_ID_SIM_READY;
            SYSLOG_INFO("SIM is ready.\r\n");
            break;

        case PS_URC_ID_SIM_REMOVED:
#ifdef FEATURE_SUBSYS_STATUS_ENABLE
            statusNotify();
#endif
            gSimStatus = PS_URC_ID_SIM_REMOVED;
            SYSLOG_INFO("SIM is unready.\r\n");
#ifdef SPEAKER_APP
            openPlay(gOpenPlayer, SIM_SOUND_UNREADY);
#endif
            break;

        default:
            break;
    }

    return 0;
}

static void restoreCallback(void *p_data, slpManLpState state)
{
    gWakeup = true;
    // EPAT_TRACE(restoreCallback, 2, "0x%X,0x%X",state,p_data);
}

void miscInit(void)
{
#ifdef SPEAKER_APP
    ledInit();
#endif
    bootPinInit();
    registerPSEventCallback(PS_GROUP_ALL_MASK, psEventCallback);
    slpManRegisterUsrdefinedRestoreCb(restoreCallback, NULL);
}

uint8_t simGetStatus(void)
{
    // SYSLOG_DEBUG("SIM=%d\r\n", (gSimStatus - PS_URC_GROUP_SIM));

    return (gSimStatus - PS_URC_GROUP_SIM);
}

bool nwIsReady(void)
{
    // SYSLOG_DEBUG("NW=%d\r\n", gNwReady);

    return gNwReady;
}

void blueLedSetState(bool on)
{
    slpManSetAPwmEnable(0, on);
}

bool chargeIsOn(void)
{
    if (gWakeup == true)
    {
        gWakeup = false;
        bootPinInit();
    }

    return (GPIO_pinRead(BOOT_PORT, BOOT_PIN) == 1) ? true : false;
}
