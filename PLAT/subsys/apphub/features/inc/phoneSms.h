#ifndef __PHONE_SMS_H__
#define __PHONE_SMS_H__


#include <stdint.h>
#include "phone.h"


#define SMS_CAPACITY            500
#define SMS_LENGTH_MAX          160


typedef struct
{
    char     number[CONTACT_NUMBER_LENGTH + 1];     //号码，UI自己从电话簿中查询姓名
    char     content[SMS_LENGTH_MAX + 1];           //短信内容
    uint32_t timeStamp;                             //短信时间戳
    uint8_t  type;                                  //SIM1短信，SIM2短信，参考: PhoneSimT
} PhoneSmsT;

typedef struct
{
    char     number[CONTACT_NUMBER_LENGTH + 1];     //号码
    char     content[SMS_LENGTH_MAX + 1];           //草稿内容
    uint32_t timeStamp;                             //草稿最后编辑时间戳
} PhoneDraftBoxSmsT;

typedef struct
{
    char content[SMS_LENGTH_MAX + 1];               //常用短信内容
} PhoneCommonBoxSmsT;

typedef struct
{
    char    sim1CenterNumber[CONTACT_NUMBER_LENGTH + 1];    //SIM1短信中心号码
    char    sim2CenterNumber[CONTACT_NUMBER_LENGTH + 1];    //SIM2短信中心号码
    uint8_t smsExpiryDate;                                  //短信有效期
    bool    sendReport;                                     //发送报告开关
    bool    recoverPath;                                    //回复路径开关
    bool    supportMoreChar;                                //支持多国语音开关
} PhoneSmsConfigT;


int32_t addInboxSms(PhoneSmsT *sms, PhoneLocationT location);
int32_t deleteInboxSms(uint32_t index, PhoneLocationT location);
int32_t getInboxSms(uint32_t index, PhoneSmsT *sms, PhoneLocationT location);
int32_t addOutboxSms(PhoneSmsT *sms, PhoneLocationT location);
int32_t deleteOutboxSms(uint32_t index, PhoneLocationT location);
int32_t getOutboxSms(uint32_t index, PhoneSmsT *sms, PhoneLocationT location);
int32_t addDraftBoxSms(PhoneDraftBoxSmsT *sms, PhoneLocationT location);
int32_t deleteDraftBoxSms(uint32_t index, PhoneLocationT location);
int32_t editDraftBoxSms(uint32_t index, PhoneDraftBoxSmsT *sms, PhoneLocationT location);
int32_t getDraftBoxSms(uint32_t index, PhoneDraftBoxSmsT *sms, PhoneLocationT location);
int32_t addCommonBoxSms(PhoneCommonBoxSmsT *sms, PhoneLocationT location);
int32_t deleteCommonBoxSms(uint32_t index, PhoneLocationT location);
int32_t editCommonBoxSms(uint32_t index, PhoneCommonBoxSmsT *sms, PhoneLocationT location);
int32_t getCommonBoxSms(uint32_t index, PhoneCommonBoxSmsT *sms, PhoneLocationT location);
int32_t getSmsCapacity(uint32_t *capacity, PhoneLocationT location);
int32_t getSmsUsedCount(uint32_t *count, PhoneLocationT location);
int32_t editSmsConfig(PhoneSmsConfigT *config);
int32_t getSmsConfig(PhoneSmsConfigT *config);


#endif
