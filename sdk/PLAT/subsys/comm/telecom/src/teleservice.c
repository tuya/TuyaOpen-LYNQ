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
#include "bsp_custom.h"
#include "cmsis_os2.h"
#include "at_api.h"
#include "slpman.h"
#include "timer.h"
#include "mode_config.h"
#include "lfs_port.h"
#include "ps_sms_if.h"
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#ifdef FEATURE_SUBSYS_STATUS_ENABLE
#include "status.h"
#endif
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_DRIVER_LCD_ENABLE
#include "lcdDrv.h"
#include "ui.h"
#include "ui_helpers.h"
#endif
#include "sysservice.h"
#include "teleservice.h"
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
#define CALLBACK_LIST_SIZE              10

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
PLAT_FPSRAM_ZI_CUST static uint8_t            gTelecomThreadStackMem[THREAD_STACK_SIZE_TELECOM] = {0};
#endif
static osThreadId_t       gTelecomThread = NULL;
static osMessageQueueId_t gTelecomQueue  = NULL;
static volatile bool      gImsReady      = false;
static volatile bool      gSmsReady      = false;
static volatile bool      gRing          = false;
static volatile bool      gCall          = false;
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


bool inCalling(void)
{
    return gCall;
}

bool incomingCall(void)
{
    return gRing;
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
#ifdef FEATURE_SUBSYS_VOLUME_ENABLE
            volumeManagerActivate("call");
#endif
            gCall  = true;
            retVal = 0;
        }
    }
    else
    {
        SYSLOG_INFO("Error.");
    }

    return retVal;
}

int32_t phoneHangUp(void)
{
    int32_t       retVal = -1;
    uint32_t      length = sizeof(ATH);
    QueueTelecomT queue  = {.type = TELE_CMD};
    AppMsgT       msg    = {.msgType = APP_SYSTEM_MSG, .param1 = TELECOME_EVENT_OTHER_HANG_UP};

    if ((gTelecomQueue != NULL) && (gRing == true) || (gCall == true))
    {
        queue.buffer = malloc(length);
        if (queue.buffer != NULL)
        {
            memset(queue.buffer, 0,   length);
            memcpy(queue.buffer, ATH, strlen(ATH));
            osMessageQueuePut(gTelecomQueue, &queue, 0, 0);
            appSendMsg(&msg);
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
    uint32_t      length = sizeof(CLVL) + 3;
    QueueTelecomT queue  = {.type = TELE_CMD};

    if ((gTelecomQueue != NULL) && (percentage <= 100))
    {
        queue.buffer = malloc(length);
        if (queue.buffer != NULL)
        {
            memset(queue.buffer, 0, length);
            snprintf(queue.buffer, length, "%s%d", CLVL, percentage);
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

int32_t smsInputHandle(char *text)
{
    int32_t retVal = -1;
    AppMsgT msg    = {.msgType = APP_SYSTEM_MSG, .param1 = TELECOME_EVENT_INCOMING_SMS};

    if (text == NULL)
    {
        SYSLOG_INFO("Param error.\r\n");
        goto labelEnd;
    }

    memset(&gSmsMemBuffer, 0, sizeof(gSmsMemBuffer));
    ec_sms_parse(text, &gSmsMemBuffer);
    if ((strlen(gSmsMemBuffer.user) == 0) || (strlen(gSmsMemBuffer.date) == 0) || (strlen(gSmsMemBuffer.text) == 0))
    {
        SYSLOG_INFO("Failed to parse sms.\r\n");
        goto labelEnd;
    }

    memset(&gSmsInputData, 0, sizeof(gSmsInputData));
    memcpy(gSmsInputData.number,   gSmsMemBuffer.user, strlen(gSmsMemBuffer.user));
    memcpy(gSmsInputData.datetime, gSmsMemBuffer.date, strlen(gSmsMemBuffer.date));
    memcpy(gSmsInputData.text,     gSmsMemBuffer.text, strlen(gSmsMemBuffer.text));
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


UINT16 Ucs2BeToUcs2Le(UINT8 *ucs2bige,UINT16 size)  //会修改原始数据
{
    unsigned int length = size;
    unsigned short *tmp = ucs2bige;
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
    UINT16 *inbuf = ucs2;
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
            data->length = Ucs2ToUtf8(ucs2hex,smslen/2,data->text,&length);
        } 
        ECOMM_TRACE(UNILOG_MISC, sms_dcs, P_VALUE, 3, "%d,%d,%d", smslen, length, data->length);
        ECPLAT_DUMP(UNILOG_MISC, sms_parse, P_INFO,"Ucs2ToUtf8:",length,data->text);
    } 
    return length;
}


static int32_t atRespCallback(UINT8 chanId, const CHAR *pStr, UINT32 strLen, void *pArg)
{
    SYSLOG_INFO("length=%d, response: %s\r\n", strLen, pStr);
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

static int32_t atUrcCallback(uint8_t chanId, char *pStr, uint32_t strLen)
{
    int32_t      evnet       = -1;
    AppMsgT      msg         = {.msgType = APP_SYSTEM_MSG};
    static char  sNumber[32] = {0};
    char        *number      = NULL;
    char        *tail        = NULL;
    uint32_t     len         = 0;

    SYSLOG_DEBUG("Len: %d, URC: %s\r\n", strLen, pStr);

    if (strstr(pStr, URC_IMS_READY) != NULL)
    {
        SYSLOG_DEBUG("IMS is ready.\r\n");
        evnet     = TELECOME_EVENT_IMS_READY;
        gImsReady = true;
    }
    else if (strstr(pStr, URC_IMS_AND_SMS_READY) != NULL)
    {
        SYSLOG_DEBUG("IMS and SMS are ready.\r\n");
        evnet     = TELECOME_EVENT_IMS_AND_SMS_READY;
        gImsReady = true;
        gSmsReady = true;
    }
    else if(strstr(pStr, URC_SMS_HEAD) != NULL)
    {
        SYSLOG_DEBUG("Incoming SMS.\r\n");
        evnet = TELECOME_EVENT_INCOMING_SMS;
        smsProcess(pStr);
    }
    else if (strstr(pStr, URC_RING) != NULL)
    {
        SYSLOG_DEBUG("Incoming call.\r\n");
        evnet = TELECOME_EVENT_RING;
        gRing = true;
    }
    else if (strstr(pStr, URC_CLIP_HEAD) != NULL)
    {
        evnet  = TELECOME_EVENT_INCOMING_CALL_NUMBER;
        number = strstr(pStr, URC_CLIP_HEAD) + strlen(URC_CLIP_HEAD);
        tail   = strstr(pStr, "\",");
        len    = strlen(number) - strlen(tail);
        if (len >= sizeof(sNumber))
        {
            SYSLOG_DEBUG("Number is too long.\r\n");
            len = sizeof(sNumber) - 1;
        }

        memset(sNumber, 0,      sizeof(sNumber));
        memcpy(sNumber, number, len);
        SYSLOG_DEBUG("Incoming call number: %s\r\n", sNumber);
        msg.param1 = TELECOME_EVENT_INCOMING_CALL_NUMBER;
        msg.param3 = sNumber;
        appSendMsg(&msg);
    }
    else if (strstr(pStr, URC_COLP_HEAD) != NULL)
    {
        SYSLOG_DEBUG("Other answerd.\r\n");
        evnet = TELECOME_EVENT_OTHER_ANSWERED;
    }
    else if (strstr(pStr, URC_OTHER_HANG_UP) != NULL)
    {
        SYSLOG_DEBUG("Other hang up.\r\n");
        evnet      = TELECOME_EVENT_OTHER_HANG_UP;
        gRing      = false;
        gCall      = false;
        msg.param1 = TELECOME_EVENT_OTHER_HANG_UP;
        appSendMsg(&msg);
    }

    if ((evnet != -1) && (gCallbackListCount > 0))
    {
        char *data = (evnet == TELECOME_EVENT_INCOMING_CALL_NUMBER) ? sNumber : NULL;
        for (uint32_t i=0; i<CALLBACK_LIST_SIZE; i++)
        {
            if (gCallbackList[i] != NULL)
            {
                gCallbackList[i](evnet, data);
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

    memset(gCallbackList, 0, sizeof(gCallbackList));
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
                        if (strcmp(queue.buffer, ATA) == 0)
                        {
#ifdef FEATURE_SUBSYS_VOLUME_ENABLE
                            osDelay(100);
                            volumeManagerActivate("call");
#endif
                        }
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
    osThreadExit();
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
        gTelecomThread = osThreadNew(threadTelecom, NULL, &threadAttr);
        if (gTelecomThread == NULL)
        {
            SYSLOG_EMERG("Failed to create thread for gTelecomThread.\r\n");
        }
    }
}
