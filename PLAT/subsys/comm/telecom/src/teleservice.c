/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    
 * Description:  
 * History:      Rev1.0   
 *
 ****************************************************************************/
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "ostask.h"
#include "osasys.h"
#include "bsp.h"
#include "ps_lib_api.h"
#include "ps_sim_if.h"
#include "ps_mm_if.h"
#include "cms_api.h"
#include "npi_config.h"
#include "bsp_custom.h"
#include "cmsis_os2.h"
#include "at_api.h"
#include "slpman.h"
#include "timer.h"
#include "mode_config.h"
#include "lfs_port.h"
#include "ps_sms_if.h"
#include "mw_nvm_audio.h"
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#ifdef FEATURE_SUBSYS_STATUS_ENABLE
#include "status.h"
#endif
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_SUBSYS_SYSINFO_ENABLE
#include "sysinfo.h"
#endif
#ifdef FEATURE_SUBSYS_FEATURES_ENABLE
#include "phone.h"
#endif
#ifdef FEATURE_SUBSYS_VOLUME_ENABLE
#include "volumeManager.h"
#endif
#ifdef FEATURE_SUBSYS_FEATURES_ENABLE
#include "featureCall.h"
#endif
#include "sysservice.h"
#include "teleservice.h"
#include "servicemanager.h"
#include DEBUG_LOG_HEADER_FILE
#define EPAT_LOG(subId, debugLevel, format, ...)  \
    ECPLAT_PRINTF(UNILOG_VoLTE, subId, debugLevel, format, ##__VA_ARGS__)


#define THREAD_STACK_SIZE_TELECOM       (2 * 1024)
#define QUEUE_SIZE_TELECOM              10
#define URC_IMS_AND_SMS_READY           "+CIREGU: 1,5"
#define URC_IMS_READY                   "+CIREGU: 1,1"
#define URC_SMS_HEAD                    "+CMT: \""
#define URC_CLIP_HEAD                   "+CLIP: \""
#define URC_COLP_HEAD                   "+COLP: \""
#define URC_OTHER_HANG_UP               "NO CARRIER"
#define URC_RING                        "RING"
#define ATA                             "ATA"
#define ATD                             "ATD"
#define ATH                             "ATH"
#define ECSMSSEND                       "AT+ECSMSSEND=1,"
#define CMGF                            "AT+CMGF="
#define CLVL                            "AT+CLVL="
#define VTS                             "AT+VTS="
#define ECCONNREL                       "AT+ECCONNREL"
#define ECSIMCFG                        "AT+ECSIMCFG=\"SimPresenceDetect\","
#define ECLOGCTRLCFG                    "AT+ECPCFG=\"logCtrl\","
#define RESP_CEREG_HEAD                 "+CEREG: "
#define CALLBACK_LIST_SIZE              10
#define PDP_CID                         1

#ifndef SMS_SIZE_MAX
#define SMS_SIZE_MAX                    (2*164U)
typedef struct
{
    int8_t index;
    uint8_t length;
    char user[25];
    char date[25];
    char number[25];
    uint8_t text[SMS_SIZE_MAX];
} sms_data_t;
#endif

enum
{
    TELE_CMD = 0,
    TELE_URC_SMS,
#ifdef VOLTE_DEMO
    TELE_ADJUST_VOLUME_IN_ISR
#endif
};

typedef struct
{
    uint8_t  type;
    char    *buffer;
} QueueTelecomT;


#ifdef THREAD_STATIC
static StaticTask_t       gTelecomThreadCbMem                               = {0};
static uint8_t            gTelecomThreadStackMem[THREAD_STACK_SIZE_TELECOM] = {0};
#endif
static osThreadId_t       gTelecomThread = NULL;
static osMessageQueueId_t gTelecomQueue  = NULL;
static volatile bool      gImsReady      = false;
static volatile bool      gSmsReady      = false;
static volatile bool      gRing          = false;
static volatile bool      gCall          = false;
static char               gNumber[32]    = {0};
#if (PSRAM_EXIST == 1)
static PLAT_FPSRAM_ZI_CUST sms_data_t    gSmsMemBuffer = {0};
static PLAT_FPSRAM_ZI_CUST SmsInputDataT gSmsInputData = {0};
#else
static sms_data_t         gSmsMemBuffer  = {0};
static SmsInputDataT      gSmsInputData  = {0};
#endif
#ifdef VOLTE_DEMO
static uint8_t            gVolume        = 0;
#endif
static uint32_t           gCallbackListCount = 0;
static TelecomCallbackT   gCallbackList[CALLBACK_LIST_SIZE] = {0};


extern void add_call_list();
extern CmsRetId appPsCmiReq(AppPsCmiReqData *pReqData, UINT32 timeOutMs);
extern CmsRetId appGetEpsBeaerParamSync(const UINT8  cid,GetPsBearerParams *pGetEpsBrParams);


bool teleHasSimCard(void)
{
    bool    retVal   = false;
    uint8_t pinState = CMI_SIM_PIN_STATE_UNKNOWN;

    retVal = simGetPinStateSync(&pinState);
    if ((retVal == CME_SUCC) && (pinState == CMI_SIM_PIN_STATE_READY))
    {
        retVal = true;
    }
    else
    {
        SYSLOG_DEBUG("retVal=%d, pinState=%d\r\n", retVal, pinState);
    }

    return retVal;
}

int32_t teleConnect(uint8_t sim)
{
    int32_t                 retVal      = -1;
    EcSimCfgGetParams       simCfgGet   = {0};
    EcSimCfgSetParams       simCfgSet   = {0};
    SetEpsBearerStateParams bearerState = {0};

    if (sim > 2)
    {
        SYSLOG_DEBUG("sim=%d\r\n", sim);
        goto labelEnd;
    }

    memset(&simCfgGet, 0, sizeof(simCfgGet));
    appGetECSIMCFGSync(&simCfgGet);
    if(simCfgGet.simSlot != sim)
    {
        appSetCFUN(0);
        memset(&simCfgSet, 0, sizeof(simCfgSet));
        simCfgSet.simSlotPresent = true;
        simCfgSet.simSlot        = sim;
        appSetECSIMCFGSync(&simCfgSet);
        appSetCFUN(1);
    }

    if (teleHasSimCard() != true)
    {
        goto labelEnd;
    }

    memset(&bearerState, 0, sizeof(bearerState));
    bearerState.cid   = PDP_CID;
    bearerState.state = 1;
    retVal = appSetEpsBeaerStateSync(&bearerState); //block max 42s
    if (retVal != CMS_RET_SUCC)
    {
        SYSLOG_DEBUG("retVal=%d\r\n", retVal);
        goto labelEnd;
    }

labelEnd:
    return retVal;
}

int32_t teleDisconnect(uint8_t sim)
{
    int32_t                 retVal      = -1;
    EcSimCfgGetParams       simCfgGet   = {0};
    SetEpsBearerStateParams bearerState = {0};

    if (sim > 2)
    {
        SYSLOG_DEBUG("sim=%d\r\n", sim);
        goto labelEnd;
    }

    memset(&simCfgGet, 0, sizeof(simCfgGet));
    appGetECSIMCFGSync(&simCfgGet);
    if(simCfgGet.simSlot != sim)
    {
        SYSLOG_DEBUG("sim=%d, simCfgGet.simSlot=%d\r\n", sim, simCfgGet.simSlot);
        goto labelEnd;
    }

    if (teleHasSimCard() != true)
    {
        goto labelEnd;
    }

    memset(&bearerState, 0, sizeof(bearerState));
    bearerState.cid   = PDP_CID;
    bearerState.state = 0;
    retVal = appSetEpsBeaerStateSync(&bearerState);
    if (retVal != CMS_RET_SUCC)
    {
        SYSLOG_DEBUG("retVal=%d\r\n", retVal);
        goto labelEnd;
    }

labelEnd:
    return retVal;
}

int32_t teleGetSignalQuality(int32_t *rssi, int32_t *rsrp, int32_t *rsrq)
{
    int32_t retVal = CMS_RET_SUCC;
    int8_t  snr    = 0;

    if ((rssi != NULL) && (rsrp != NULL) && (rsrq != NULL))
    {
        retVal = appGetSignalQualitySync((uint8_t *)rssi, &snr, (int8_t *)rsrp, (int8_t *)rsrq);
        if (retVal != CMS_RET_SUCC)
        {
            SYSLOG_DEBUG("retVal=%d\r\n", retVal);
        }
    }

    return retVal;
}

int32_t teleGetCsqInfo(int32_t *rssi, int32_t *ber)
{
    int32_t         retVal     = -1;
    AppPsCmiReqData cmiReqData = {0};
    CamCmiEmptySig  cmiReq     = {0};
    CmiMmGetCesqCnf cmiCnf     = {0};

    if ((rssi != NULL) && (ber != NULL))
    {
        memset(&cmiReqData, 0, sizeof(cmiReqData));
        memset(&cmiReq,     0, sizeof(cmiReq));
        memset(&cmiCnf,     0, sizeof(cmiCnf));
        cmiReqData.sgId        = CAM_MM;
        cmiReqData.reqPrimId   = CMI_MM_GET_EXTENDED_SIGNAL_QUALITY_REQ;
        cmiReqData.cnfPrimId   = CMI_MM_GET_EXTENDED_SIGNAL_QUALITY_CNF;
        cmiReqData.reqParamLen = sizeof(cmiReq);
        cmiReqData.pReqParam   = &cmiReq;
        cmiReqData.cnfBufLen   = sizeof(cmiCnf);
        cmiReqData.pCnfBuf     = &cmiCnf;
        retVal = appPsCmiReq(&cmiReqData, CMS_MAX_DELAY_MS);
        if ((retVal == CMS_RET_SUCC) && (cmiReqData.cnfRc == CME_SUCC))
        {
            *ber  = cmiCnf.dlBer;
            *rssi = mmGetCsqRssiFromCesq(cmiCnf.rsrp, cmiCnf.rsrq, cmiCnf.rssiCompensation);
        }
        else
        {
            SYSLOG_DEBUG("retVal=%d, cmiReqData.cnfRc=%d\r\n", retVal, cmiReqData.cnfRc);
        }
    }

    return retVal;
}

int32_t teleGetNetworkStatus(void)
{
    int32_t        retVal    = -1;
    NmAtiNetifInfo netifInfo = {0};

    retVal = appGetNetInfoSync(PDP_CID, &netifInfo);
    if (retVal == CMS_RET_SUCC)
    {
        retVal = netifInfo.netStatus;
    }
    else
    {
        SYSLOG_DEBUG("retVal=%d\r\n", retVal);
    }

    return retVal;
}

int32_t teleGetBand(void)
{
    int32_t         retVal     = -1;
    UeExtStatusInfo statusInfo = {0};

    retVal = appGetUeExtStatusInfoSync(CMI_DEV_GET_ECSTATUS_PHY, &statusInfo);
    if (retVal == CMS_RET_SUCC)
    {
        retVal = statusInfo.phyStatus.band;
    }
    else
    {
        SYSLOG_DEBUG("retVal=%d\r\n", retVal);
    }

    return retVal;
}

int32_t teleGetImei(char *imei)
{
#ifdef FEATURE_SUBSYS_SYSINFO_ENABLE
    return getSysinfo(SYSINFO_IMEI, imei);
#else
    return -1;
#endif
}

int32_t teleGetIccid(char *iccid)
{
#ifdef FEATURE_SUBSYS_SYSINFO_ENABLE
    return getSysinfo(SYSINFO_ICCID, iccid);
#else
    return -1;
#endif
}

int32_t teleGetImsi(char *imsi)
{
    int32_t retVal = -1;

    if (imsi != NULL)
    {
        retVal = appGetImsiNumSync(imsi);
        if (retVal != CMS_RET_SUCC)
        {
            SYSLOG_DEBUG("retVal=%d\r\n", retVal);
        }
    }

    return retVal;
}

int32_t teleGetApn(char *apn)
{
    int32_t           retVal       = -1;
    GetPsBearerParams bearerParams = {0};

    if (apn != NULL)
    {
        retVal = appGetEpsBeaerParamSync(PDP_CID, &bearerParams);
        if (retVal == CMS_RET_SUCC)
        {
            memcpy(apn, bearerParams.pdpDynParam.apnStr, CMI_PS_MAX_APN_LEN);
        }
        else
        {
            SYSLOG_DEBUG("retVal=%d\r\n", retVal);
        }
    }

    return retVal;
}

int32_t teleGetLocalIpaddr(char *ip)
{
#ifdef FEATURE_SUBSYS_SYSINFO_ENABLE
    return getSysinfo(SYSINFO_IP, ip);
#else
    return -1;
#endif
}

int32_t teleSetAirplaneMode(bool enable)
{
    int32_t retVal = -1;

    retVal = appSetCFUN((enable == true) ? 0 : 1);
    if (retVal != CMS_RET_SUCC)
    {
        SYSLOG_DEBUG("retVal=%d\r\n", retVal);
    }

    return retVal;
}

int32_t teleGetAirplaneMode(bool *enable)
{
    int32_t retVal = -1;
    uint8_t cfun   = 0;

    if (enable != NULL)
    {
        retVal = appGetCFUN(&cfun);
        if (retVal == CMS_RET_SUCC)
        {
            *enable = (cfun == 0) ? true : false;
        }
        else
        {
            SYSLOG_DEBUG("retVal=%d\r\n", retVal);
        }
    }

    return retVal;
}

bool imsIsReady(void)
{
    return gImsReady;
}

bool inCalling(void)
{
    return gCall;
}

bool incomingCall(void)
{
    return gRing;
}

char *incomingCallNumberGet(void)
{
    return gNumber;
}

bool npiRfCaliIsDone(void)
{
    return (npiGetProcessStatusItemValue(NPI_PROCESS_STATUS_ITEM_RFCALI) != 0) ? true : false;
}

bool npiRfNstIsDone(void)
{
    return (npiGetProcessStatusItemValue(NPI_PROCESS_STATUS_ITEM_RFNST) != 0) ? true : false;
}

bool npiRfCtIsDone(void)
{
    return (npiGetProcessStatusItemValue(NPI_PROCESS_STATUS_ITEM_RFCT) != 0) ? true : false;
}

int32_t ecsimcfgSimPresenceDetect(bool enable)
{
    int32_t       retVal = -1;
    uint32_t      length = sizeof(ECSIMCFG) + 1;
    QueueTelecomT queue  = {.type = TELE_CMD};

    if (gTelecomQueue != NULL)
    {
        queue.buffer = malloc(length);
        if (queue.buffer != NULL)
        {
            memset(queue.buffer, 0, length);
            snprintf(queue.buffer, length, "%s%s", ECSIMCFG, ((enable == true) ? "1" : "0"));
            osMessageQueuePut(gTelecomQueue, &queue, 0, 0);
            retVal = 0;
        }
    }
    else
    {
        SYSLOG_INFO("Error.\r\n");
    }

    return retVal;
}

int32_t ecSetLogCtrl(uint8_t logCtrl)
{
    int32_t retVal = -1;
    uint32_t length = sizeof(ECSIMCFG) + 1;
    QueueTelecomT queue = {.type = TELE_CMD};

    if(gTelecomQueue != NULL)
    {
        queue.buffer = malloc(length);
        if(queue.buffer != NULL)
        {
            memset(queue.buffer, 0, length);
            snprintf(queue.buffer, length, "%s%d", ECLOGCTRLCFG,
                     logCtrl);
            osMessageQueuePut(gTelecomQueue, &queue, 0, 0);
            retVal = 0;
        }
    }
    else
    {
        SYSLOG_INFO("Error.\r\n");
    }

    return retVal;
}

int32_t sendCommonAt(char *at)
{
    int32_t       retVal = -1;
    uint32_t      length = strlen(at) + 1;
    QueueTelecomT queue  = {.type = TELE_CMD};

    if (gTelecomQueue != NULL)
    {
        queue.buffer = malloc(length);
        if (queue.buffer != NULL)
        {
            memset(queue.buffer, 0,  length);
            memcpy(queue.buffer, at, length - 1);
            osMessageQueuePut(gTelecomQueue, &queue, 0, 0);
            retVal = 0;
        }
        else
        {
            SYSLOG_DEBUG("Failed to malloc %d bytes for queue.buffer.\r\n", length);
        }
    }
    else
    {
        SYSLOG_DEBUG("gTelecomQueue is NULL.\r\n");
    }

    return retVal;
}

int32_t ecconnrel(void)
{
    int32_t       retVal = -1;
    uint32_t      length = sizeof(ECCONNREL);
    QueueTelecomT queue  = {.type = TELE_CMD};

    if (gTelecomQueue != NULL)
    {
        queue.buffer = malloc(length);
        if (queue.buffer != NULL)
        {
            memset(queue.buffer, 0, length);
            memcpy(queue.buffer, ECCONNREL, strlen(ECCONNREL));
            osMessageQueuePut(gTelecomQueue, &queue, 0, 0);
            retVal = 0;
        }
    }
    else
    {
        SYSLOG_INFO("Error.\r\n");
    }

    return retVal;
}

int32_t vtsSend(char *character)
{
    int32_t       retVal = -1;
    uint32_t      length = 0;
    QueueTelecomT queue  = {.type = TELE_CMD};

    if ((gTelecomQueue != NULL) && (gCall == true) && (character != NULL))
    {
        length = sizeof(VTS) + strlen(character);
        queue.buffer = malloc(length);
        if (queue.buffer != NULL)
        {
            memset(queue.buffer, 0, length);
            snprintf(queue.buffer, length, "%s%s", VTS, character);
            osMessageQueuePut(gTelecomQueue, &queue, 0, 0);
            retVal = 0;
        }
    }
    else
    {
        SYSLOG_INFO("Error.\r\n");
    }

    return retVal;
}

static void telecomActionEventProcess(uint8_t event)
{
    if (gCallbackListCount > 0)
    {
        for (uint32_t i=0; i<CALLBACK_LIST_SIZE; i++)
        {
            if (gCallbackList[i] != NULL)
            {
                gCallbackList[i](event, NULL);
            }
        }
    }
}

int32_t phoneAnswer(void)
{
    int32_t       retVal = -1;
    uint32_t      length = sizeof(ATA);
    QueueTelecomT queue  = {.type = TELE_CMD};

    if ((gTelecomQueue != NULL) && (gRing == true))
    {
        queue.buffer = malloc(length);
        if (queue.buffer != NULL)
        {
            memset(queue.buffer, 0,   length);
            memcpy(queue.buffer, ATA, strlen(ATA));
            osMessageQueuePut(gTelecomQueue, &queue, 0, 0);
#ifdef FEATURE_SUBSYS_FEATURES_ENABLE
            setCallState(PHONE_CALL_STATE_ATA);
            recordCallBeginTime();
#endif
#ifdef FEATURE_SUBSYS_VOLUME_ENABLE
            volumeManagerActivate("call");
#endif
            telecomActionEventProcess(TELECOM_EVENT_ANSWER);
            gRing = false;
            gCall = true;
            retVal = 0;
        }
    }
    else
    {
        SYSLOG_INFO("Error.\r\n");
    }

    return retVal;
}

int32_t phoneDialled(char *number)
{
    int32_t       retVal = -1;
    uint32_t      length = 0;
    QueueTelecomT queue  = {.type = TELE_CMD};

    if ((gTelecomQueue != NULL) && (gImsReady == true) && (gRing != true) && (gCall != true) && (number != NULL))
    {
        length = sizeof(ATD) + strlen(number);
        queue.buffer = malloc(length);
        if (queue.buffer != NULL)
        {
            memset(queue.buffer, 0, length);
            snprintf(queue.buffer, length, "%s%s", ATD, number);
            osMessageQueuePut(gTelecomQueue, &queue, 0, 0);
#ifdef FEATURE_SUBSYS_FEATURES_ENABLE
            setCallState(PHONE_CALL_STATE_ATD);
            recordCallBeginTime();
#endif
#ifdef FEATURE_SUBSYS_VOLUME_ENABLE
            volumeManagerActivate("call");
#endif
            telecomActionEventProcess(TELECOM_EVENT_DIALLED);
            gCall  = true;
            retVal = 0;
        }
    }
    else
    {
        SYSLOG_INFO("Error.\r\n");
    }

    return retVal;
}

int32_t phoneHangUp(void)
{
    int32_t       retVal = -1;
    uint32_t      length = sizeof(ATH);
    QueueTelecomT queue  = {.type = TELE_CMD};

    if ((gTelecomQueue != NULL) && ((gRing == true) || (gCall == true)))
    {
        queue.buffer = malloc(length);
        if (queue.buffer != NULL)
        {
            memset(queue.buffer, 0,   length);
            memcpy(queue.buffer, ATH, strlen(ATH));
            osMessageQueuePut(gTelecomQueue, &queue, 0, 0);
#ifdef FEATURE_SUBSYS_FEATURES_ENABLE
            add_call_list();
#endif
            telecomActionEventProcess(TELECOM_EVENT_HANG_UP);
            gRing  = false;
            gCall  = false;
            retVal = 0;
            #if (UI_WATCH_USED == 1)
            SYSLOG_INFO("phoneHangUp.\r\n");
            #endif
        }
    }
    else
    {
        SYSLOG_INFO("Error.\r\n");
    }

    return retVal;
}

int32_t smsConfigMessageFormat(uint8_t mode)
{
    int32_t       retVal = -1;
    uint32_t      length = sizeof(CMGF) + 1;
    QueueTelecomT queue  = {.type = TELE_CMD};

    if (gTelecomQueue != NULL)
    {
        queue.buffer = malloc(length);
        if (queue.buffer != NULL)
        {
            memset(queue.buffer, 0, length);
            snprintf(queue.buffer, length, "%s%d", CMGF, mode);
            osMessageQueuePut(gTelecomQueue, &queue, 0, 0);
            retVal = 0;
        }
    }
    else
    {
        SYSLOG_INFO("Error.\r\n");
    }

    return retVal;
}

int32_t smsSendText(char *number, char *text)
{
    int32_t       retVal = -1;
    uint32_t      length = 0;
    QueueTelecomT queue  = {.type = TELE_CMD};

    if ((gTelecomQueue != NULL) && (number != NULL) && (text != NULL)  && (gSmsReady == true))
    {
        length = strlen(ECSMSSEND) + strlen(number) + strlen(text) + 5;
        queue.buffer = malloc(length);
        if (queue.buffer != NULL)
        {
            memset(queue.buffer, 0, length);
            snprintf(queue.buffer, length, "%s%s,,\"%s\"", ECSMSSEND, number, text);
            osMessageQueuePut(gTelecomQueue, &queue, 0, 0);
            retVal = 0;
        }
    }
    else
    {
        SYSLOG_INFO("Error.\r\n");
    }

    return retVal;
}

int32_t smsProcess(char *payload)
{
    int32_t       retVal = -1;
    uint32_t      length = 0;
    QueueTelecomT queue  = {.type = TELE_URC_SMS};

    if ((gTelecomQueue != NULL) && (payload != NULL))
    {
        length = strlen(payload) + 1;
        queue.buffer = malloc(length);
        if (queue.buffer != NULL)
        {
            memset(queue.buffer, 0,       length);
            memcpy(queue.buffer, payload, strlen(payload));
            osMessageQueuePut(gTelecomQueue, &queue, 0, 0);
            retVal = 0;
        }
    }
    else
    {
        SYSLOG_INFO("Error.\r\n");
    }

    return retVal;
}

int32_t setVolume(uint8_t percentage)
{
    int32_t       retVal = -1;
    uint32_t      length = sizeof(CLVL) + 5;
    QueueTelecomT queue  = {.type = TELE_CMD};

    if ((gTelecomQueue != NULL) && (percentage <= 100))
    {
        queue.buffer = malloc(length);
        if (queue.buffer != NULL)
        {
            memset(queue.buffer, 0, length);
            snprintf(queue.buffer, length, "%s%d,%d", CLVL, AUDIO_CFG_DEVICE_HAND_FREE, percentage);
            osMessageQueuePut(gTelecomQueue, &queue, 0, 0);
            retVal = 0;
        }
    }
    else
    {
        SYSLOG_INFO("Error.\r\n");
    }

    return retVal;
}

#ifdef VOLTE_DEMO
int32_t setVolumeInIsr(uint8_t percentage)
{
    int32_t       retVal = -1;
    QueueTelecomT queue  = {.type = TELE_ADJUST_VOLUME_IN_ISR, .buffer = NULL};

    if ((gTelecomQueue != NULL) && (percentage <= 100))
    {
        gVolume = percentage;
        osMessageQueuePut(gTelecomQueue, &queue, 0, 0);
        retVal = 0;
    }
    else
    {
        SYSLOG_INFO("Error.\r\n");
    }

    return retVal;
}
#endif

UINT16 Ucs2BeToUcs2Le(UINT8 *ucs2bige,UINT16 size)  //会修改原始数据
{
    unsigned int length = size;
    unsigned short *tmp = (unsigned short *)ucs2bige;
    while (*tmp && length) {
        length--;
        unsigned char val_high = *tmp >> 8;
        unsigned char val_low = (unsigned char)*tmp;
        *tmp = val_low << 8 | val_high;
        tmp++;
    }
    return size - length;
}

// Convert Ucs-2 to Utf-8
UINT16 Ucs2ToUtf8(UINT8 *ucs2,UINT16 ucs2_size,UINT8 *utf8, UINT16 *utf8_size)
{
    UINT16 length = 0;
    if (!ucs2) return length;
    ECPLAT_DUMP(UNILOG_MISC, Ucs2ToUtf8_1, P_INFO,"input:",ucs2_size,ucs2);
    UINT16 *inbuf = (UINT16 *)ucs2;
    UINT8 *outbuf = utf8;
    
    UINT16 insize = ucs2_size;
    UINT16 total = 0;
    if (!utf8) 
    {
        while (*inbuf && insize) {
            insize-=sizeof(*inbuf);
            if (0x0080 > *inbuf) {
                length++;
                total++;
            } else if (0x0800 > *inbuf) {
                length += 2;
                total++;
            } else {
                length += 3;
                total++;
            }
            inbuf++;
        }
    } 
    else 
    {
        while((*inbuf) && (insize) && length < *utf8_size) 
        {
            if (*inbuf == 0xFFFE) {
                inbuf++;
                continue;
            }
            if (0x0080 > *inbuf) {
                /* 1 byte UTF-8 Character.*/
                *outbuf++ = (unsigned char)(*inbuf);
                length++;
                total++;
            } else if (0x0800 > *inbuf) {
                /*2 bytes UTF-8 Character.*/
                *outbuf++ = 0xc0 | ((unsigned char)(*inbuf >> 6));
                *outbuf++ = 0x80 | ((unsigned char)(*inbuf & 0x3F));
                length += 2;
                total++;
            } else {
                /* 3 bytes UTF-8 Character .*/
                *outbuf++ = 0xE0 | ((unsigned char)(*inbuf >> 12));
                *outbuf++ = 0x80 | ((unsigned char)((*inbuf >> 6) & 0x3F));
                *outbuf++ = 0x80 | ((unsigned char)(*inbuf & 0x3F));
                length += 3;
                total++;
            }
            ECOMM_TRACE(UNILOG_MISC, Ucs2ToUtf8, P_VALUE, 5, "%d,%d,%d,0x%X,0x%X",insize, length, total, *inbuf,*outbuf);
            inbuf++;
            insize-=sizeof(*inbuf);
        }
        *utf8_size = length;
        ECPLAT_DUMP(UNILOG_MISC, Ucs2ToUtf8_2, P_INFO,"output:",length,utf8);
        return total;
    }

    return 0;
}

extern INT32 cmsHexStrToHex(UINT8 *pOutput, UINT32 outBufSize, const CHAR *pHexStr, UINT32 strMaxLen);
int ec_sms_parse(char* str, sms_data_t *data)
{
    if(str == NULL || strlen(str) < 50) return 0;
    char  *head   = NULL;
    char  *tail   = NULL;
    int  length     = 0;
    uint8_t sms_dcs = 0;
    uint16_t smslen = 0;
    uint8_t buffer[4] = {0};
    head = strstr(str,URC_SMS_HEAD);
    if (head != NULL)
    {
        tail = strstr(head,"\",");
        length  = strlen(head)- strlen(tail) - strlen(URC_SMS_HEAD);
        memcpy(data->user,head+strlen(URC_SMS_HEAD),length);
        // smslen += length;
    }
    //,,"23/12/07,15:10:35 +32",160,36,0,8,"+8613800210541",128,236
    head = strstr(tail,",\"");
    if (head != NULL)
    {
        tail = strstr(head," +32\"");   //tz=32,tzSign=+
        length  = strlen(head) - strlen(tail) - 2;
        memcpy(data->date,head+2,length);
    } 
    head = strstr(tail,",\"");
    if (head != NULL)//+8613800210541
    {
        tail = strstr(head,"\",");
        length  = strlen(head) - strlen(tail) - 2;
        memcpy(data->number,head+2,length);
        sms_dcs = *(uint8_t*)(head - 1) - '0';
    } 
    head = strstr(tail,"\r\n"); //解析获得UCS2数据
    if (head != NULL)
    {
        for (int i = 0; i < 3;i++){
            if(*(head-1-i)!=',' && *(head-1-i)>='0' && *(head-1-i)<='9') {
                buffer[i] = *(head - 1 -i) - '0';
            }
            else break;
        }
        smslen  = buffer[0]+ 10*buffer[1]+ 100*buffer[2];    // strlen(head) - 2;
        if(sms_dcs == 0) memcpy(data->text,head+2,smslen);
        else{
            uint8_t ucs2hex[SMS_SIZE_MAX] = {0};
            length = SMS_SIZE_MAX;   
            cmsHexStrToHex(ucs2hex, smslen/2, head+2, smslen); 
            ECPLAT_DUMP(UNILOG_MISC, ucs2, P_INFO,"input:",smslen/2,ucs2hex);
            Ucs2BeToUcs2Le(ucs2hex, smslen/2);
            data->length = Ucs2ToUtf8(ucs2hex,smslen/2,data->text,(UINT16 *)&length);
        } 
        ECOMM_TRACE(UNILOG_MISC, sms_dcs, P_VALUE, 3, "%d,%d,%d", smslen, length, data->length);
        ECPLAT_DUMP(UNILOG_MISC, sms_parse, P_INFO,"Ucs2ToUtf8:",length,data->text);
    } 
    return length;
}

int32_t smsInputHandle(char *text)
{
    int32_t retVal = -1;
    AppMsgT msg    = {.msgType = APP_SYSTEM_MSG, .param1 = TELECOM_EVENT_INCOMING_SMS};

    if (text == NULL)
    {
        SYSLOG_INFO("Param error.\r\n");
        goto labelEnd;
    }

    memset(&gSmsMemBuffer, 0, sizeof(gSmsMemBuffer));
    ec_sms_parse(text, &gSmsMemBuffer);
    if ((strlen(gSmsMemBuffer.user) == 0) || (strlen(gSmsMemBuffer.date) == 0) || (strlen((char *)gSmsMemBuffer.text) == 0))
    {
        SYSLOG_INFO("Failed to parse sms.\r\n");
        goto labelEnd;
    }

    memset(&gSmsInputData, 0, sizeof(gSmsInputData));
    memcpy(gSmsInputData.number,   gSmsMemBuffer.user, strlen(gSmsMemBuffer.user));
    memcpy(gSmsInputData.datetime, gSmsMemBuffer.date, strlen(gSmsMemBuffer.date));
    memcpy(gSmsInputData.text,     gSmsMemBuffer.text, strlen((char *)gSmsMemBuffer.text));
    msg.param3 = malloc(sizeof(gSmsInputData));
    if (msg.param3 != NULL)
    {
        memset(msg.param3, 0,             sizeof(gSmsInputData));
        memcpy(msg.param3, &gSmsInputData, sizeof(gSmsInputData));
        appSendMsg(&msg);
    }

labelEnd:
    return retVal;
}

static int32_t atRespCallback(UINT8 chanId, const CHAR *pStr, UINT32 strLen, void *pArg)
{
    int32_t  event = -1;
    char    *param = NULL;

    SYSLOG_INFO("length=%d, response: %s\r\n", strLen, pStr);

    if (strstr(pStr, RESP_CEREG_HEAD) != NULL)
    {
        event = TELECOM_EVENT_CEREG;
        param = strstr(pStr, RESP_CEREG_HEAD) + strlen(RESP_CEREG_HEAD) + 2;
    }

    if ((event != -1) && (gCallbackListCount > 0))
    {
        for (uint32_t i=0; i<CALLBACK_LIST_SIZE; i++)
        {
            if (gCallbackList[i] != NULL)
            {
                gCallbackList[i](event, param);
            }
        }
    }

    return 0;
}

int32_t telecomCallbackRegister(TelecomCallbackT callback)
{
    int32_t retVal = -1;

    if (callback == NULL)
    {
        SYSLOG_DEBUG("callback is NULL\r\n");
        goto labelEnd;
    }

    for (uint32_t i=0; i<CALLBACK_LIST_SIZE; i++)
    {
        if (gCallbackList[i] == callback)
        {
            retVal = 0;
            goto labelEnd;
        }
    }

    for (uint32_t i=0; i<CALLBACK_LIST_SIZE; i++)
    {
        if (gCallbackList[i] == NULL)
        {
            retVal           = 0;
            gCallbackList[i] = callback;
            gCallbackListCount++;
            goto labelEnd;
        }
    }

    SYSLOG_DEBUG("Telecom callback list is full: %d\r\n", CALLBACK_LIST_SIZE);

labelEnd:
    return retVal;
}

int32_t telecomCallbackUnregister(TelecomCallbackT callback)
{
    int32_t retVal = -1;

    if (callback == NULL)
    {
        SYSLOG_DEBUG("callback is NULL\r\n");
        goto labelEnd;
    }

    for (uint32_t i=0; i<CALLBACK_LIST_SIZE; i++)
    {
        if (gCallbackList[i] == callback)
        {
            retVal           = 0;
            gCallbackList[i] = NULL;
            gCallbackListCount--;
            goto labelEnd;
        }
    }

    SYSLOG_DEBUG("Callback don't exist.\r\n");

labelEnd:
    return retVal;
}

static INT32 atUrcCallback(UINT8 chanId, const CHAR *pStr, UINT32 strLen)
{
    int32_t   event  = -1;
    AppMsgT   msg    = {.msgType = APP_SYSTEM_MSG};
    char     *number = NULL;
    char     *tail   = NULL;
    uint32_t  len    = 0;

    SYSLOG_DEBUG("Len: %d, URC: %s\r\n", strLen, pStr);

    if (strstr(pStr, URC_IMS_READY) != NULL)
    {
        SYSLOG_DEBUG("IMS is ready.\r\n");
        event     = TELECOM_EVENT_IMS_READY;
        gImsReady = true;
    }
    else if (strstr(pStr, URC_IMS_AND_SMS_READY) != NULL)
    {
        SYSLOG_DEBUG("IMS and SMS are ready.\r\n");
        event     = TELECOM_EVENT_IMS_AND_SMS_READY;
        gImsReady = true;
        gSmsReady = true;
    }
    else if(strstr(pStr, URC_SMS_HEAD) != NULL)
    {
        SYSLOG_DEBUG("Incoming SMS.\r\n");
        event = TELECOM_EVENT_INCOMING_SMS;
        smsProcess((char *)pStr);
    }
    else if (strstr(pStr, URC_RING) != NULL)
    {
        SYSLOG_DEBUG("Incoming call.\r\n");
        event = TELECOM_EVENT_RING;
        gRing = true;
    }
    else if (strstr(pStr, URC_CLIP_HEAD) != NULL)
    {
        event  = TELECOM_EVENT_INCOMING_CALL_NUMBER;
        number = strstr(pStr, URC_CLIP_HEAD) + strlen(URC_CLIP_HEAD);
        tail   = strstr(pStr, "\",");
        len    = strlen(number) - strlen(tail);
        if (len >= sizeof(gNumber))
        {
            SYSLOG_DEBUG("Number is too long.\r\n");
            len = sizeof(gNumber) - 1;
        }

        memset(gNumber, 0,      sizeof(gNumber));
        memcpy(gNumber, number, len);
        SYSLOG_DEBUG("Incoming call number: %s\r\n", gNumber);
        msg.param1 = TELECOM_EVENT_INCOMING_CALL_NUMBER;
        msg.param3 = (uint32_t *)gNumber;
        appSendMsg(&msg);
#ifdef FEATURE_SUBSYS_FEATURES_ENABLE
        setCallState(PHONE_CALL_STATE_RING);
        recordCallBeginTime();
#endif
    }
    else if (strstr(pStr, URC_COLP_HEAD) != NULL)
    {
        SYSLOG_DEBUG("Other answerd.\r\n");
        event = TELECOM_EVENT_OTHER_ANSWERED;
#ifdef FEATURE_SUBSYS_FEATURES_ENABLE
        setCallState(PHONE_CALL_STATE_COLP);
        recordCallBeginTime();
#endif
    }
    else if (strstr(pStr, URC_OTHER_HANG_UP) != NULL)
    {
        SYSLOG_DEBUG("Other hang up.\r\n");
        event      = TELECOM_EVENT_OTHER_HANG_UP;
        gRing      = false;
        gCall      = false;
        msg.param1 = TELECOM_EVENT_OTHER_HANG_UP;
        appSendMsg(&msg);
#ifdef FEATURE_SUBSYS_FEATURES_ENABLE
        add_call_list();
#endif
    }

    if ((event != -1) && (gCallbackListCount > 0))
    {
        char *data = (event == TELECOM_EVENT_INCOMING_CALL_NUMBER) ? gNumber : NULL;
        for (uint32_t i=0; i<CALLBACK_LIST_SIZE; i++)
        {
            if (gCallbackList[i] != NULL)
            {
                gCallbackList[i](event, data);
            }
        }
    }

    return 0;
}

static void threadTelecom(void *argument)
{
    QueueTelecomT queue = {0};

    gTelecomQueue = osMessageQueueNew(QUEUE_SIZE_TELECOM, sizeof(queue), NULL);
    if (gTelecomQueue == NULL)
    {
        SYSLOG_EMERG("Failed to create queue for gTelecomQueue.\r\n");
        goto labelEnd;
    }

    atRilRegisterUrcCallback(atUrcCallback);
    smsConfigMessageFormat(1);
    osDelay(2000);

    while (1)
    {
        memset(&queue, 0, sizeof(queue));
        if (osMessageQueueGet(gTelecomQueue, &queue, 0, osWaitForever) == osOK)
        {
            switch (queue.type)
            {
                case TELE_CMD:
                    if (queue.buffer != NULL)
                    {
                        SYSLOG_NOTICE("Send RIL AT: %s\r\n", queue.buffer);
                        atRilAtCmdReq(queue.buffer, strlen(queue.buffer), atRespCallback, NULL, 0);
                    }
                    break;

                case TELE_URC_SMS:
                    if (queue.buffer != NULL)
                    {
                        smsInputHandle(queue.buffer);
                    }
                    break;

#ifdef VOLTE_DEMO
                case TELE_ADJUST_VOLUME_IN_ISR:
                    setVolume(gVolume);
                    break;
#endif

                default:
                    break;
            }

            if (queue.buffer != NULL)
            {
                EPAT_LOG(threadTelecom_1, P_INFO, "%s",queue.buffer);
                free(queue.buffer);
                queue.buffer = NULL;
            }
        }
    }

labelEnd:
#if 0 // Service Manager
    osThreadExit();
#else
    Service_stop("service:/threadTelecom");
#endif
}

void teleServiceInit(void)
{
    osThreadAttr_t threadAttr = {0};

    if (gTelecomThread == NULL)
    {
        memset(&threadAttr, 0, sizeof(threadAttr));
        threadAttr.name       = "threadTelecom";
        threadAttr.priority   = osPriorityNormal;
        threadAttr.stack_size = THREAD_STACK_SIZE_TELECOM;
#ifdef THREAD_STATIC
        threadAttr.stack_mem  = gTelecomThreadStackMem;
        threadAttr.cb_mem     = &gTelecomThreadCbMem;
        threadAttr.cb_size    = sizeof(StaticTask_t);
#endif
#if 0
        gTelecomThread = osThreadNew(threadTelecom, NULL, &threadAttr);
#else
        char serviceName[32] = {0};
        snprintf(serviceName, sizeof(serviceName), "service:/%s", threadAttr.name);
        Service_reg(serviceName, threadTelecom, NULL, threadAttr.cb_mem, threadAttr.cb_size, threadAttr.stack_mem, threadAttr.stack_size, threadAttr.priority);
        gTelecomThread = (osThreadId_t)Service_start(serviceName);
#endif
        if (gTelecomThread == NULL)
        {
            SYSLOG_EMERG("Failed to create thread for gTelecomThread.\r\n");
        }
    }
}
