#ifndef __TELECOM_H__
#define __TELECOM_H__


#include <stdint.h>
#include <stdbool.h>
#include "ps_sms_if.h"
#include "sysservice.h"


#define AT_CEREG                        "AT+CEREG?"


typedef enum
{
    TELECOM_EVENT_IMS_READY = SYSSERVICE_EVENT_TELECOM,
    TELECOM_EVENT_IMS_AND_SMS_READY,
    TELECOM_EVENT_INCOMING_SMS,
    TELECOM_EVENT_RING,
    TELECOM_EVENT_INCOMING_CALL_NUMBER,
    TELECOM_EVENT_OTHER_ANSWERED,
    TELECOM_EVENT_OTHER_HANG_UP,
    TELECOM_EVENT_ANSWER,
    TELECOM_EVENT_DIALLED,
    TELECOM_EVENT_HANG_UP,
    TELECOM_EVENT_CEREG,
} TelecomEventT;

typedef struct
{
    char number[40];
    char datetime[40];
    char text[PSIL_SMS_MAX_TXT_SIZE + 1];
} SmsInputDataT;


typedef void (*TelecomCallbackT)(int32_t event, char *data);


void    teleServiceInit(void);
int32_t phoneAnswer(void);
int32_t phoneDialled(char *number);
int32_t phoneHangUp(void);
int32_t smsSendText(char *number, char *text);
int32_t setVolume(uint8_t percentage);
#ifdef VOLTE_DEMO
int32_t setVolumeInIsr(uint8_t percentage);
#endif
int32_t vtsSend(char *character);
bool    imsIsReady(void);
bool    incomingCall(void);
bool    inCalling(void);
char   *incomingCallNumberGet(void);
int32_t telecomCallbackRegister(TelecomCallbackT callback);
int32_t telecomCallbackUnregister(TelecomCallbackT callback);
int32_t sendCommonAt(char *at);
int32_t ecconnrel(void);
int32_t ecsimcfgSimPresenceDetect(bool enable);
int32_t ecSetLogCtrl(uint8_t logCtrl);
int32_t ecSetAdjacentCell(bool enable);
bool    npiRfCaliIsDone(void);
bool    npiRfNstIsDone(void);
bool    npiRfCtIsDone(void);

bool    teleHasSimCard(void);
int32_t teleConnect(uint8_t sim);
int32_t teleDisconnect(uint8_t sim);
int32_t teleGetSignalQuality(int32_t *rssi, int32_t *rsrp, int32_t *rsrq);
int32_t teleGetCsqInfo(int32_t *rssi, int32_t *ber);
int32_t teleGetNetworkStatus(void);
int32_t teleGetBand(void);
int32_t teleGetImei(char *imei);
int32_t teleGetIccid(char *iccid);
int32_t teleGetImsi(char *imsi);
int32_t teleGetApn(char *apn);
int32_t teleGetLocalIpaddr(char *ip);
int32_t teleSetAirplaneMode(bool enable);
int32_t teleGetAirplaneMode(bool *enable);

#endif
