#ifndef __TELECOM_H__
#define __TELECOM_H__


#include <stdint.h>
#include <stdbool.h>
#include "ps_sms_if.h"
#include "sysservice.h"


typedef enum
{
    TELECOME_EVENT_IMS_READY = SYSSERVICE_EVENT_TELECOME,
    TELECOME_EVENT_IMS_AND_SMS_READY,
    TELECOME_EVENT_INCOMING_SMS,
    TELECOME_EVENT_RING,
    TELECOME_EVENT_INCOMING_CALL_NUMBER,
    TELECOME_EVENT_OTHER_ANSWERED,
    TELECOME_EVENT_OTHER_HANG_UP,
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
bool    incomingCall(void);
bool    inCalling(void);
int32_t telecomCallbackRegister(TelecomCallbackT callback);
int32_t telecomCallbackUnregister(TelecomCallbackT callback);


#endif
