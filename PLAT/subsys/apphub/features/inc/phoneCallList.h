#ifndef __PHONE_CALL_LIST_H__
#define __PHONE_CALL_LIST_H__


#include <stdint.h>
#include "phone.h"


typedef struct
{
    char     number[CONTACT_NUMBER_LENGTH + 1];     //号码，UI自己去电话簿里查询姓名
    uint8_t  type;                                  //参考: PhoneCallTypeT
    uint8_t  sim;                                   //参考: PhoneSimT
    uint32_t time;                                  //通话时长，单位：秒
    uint32_t timeStamp;                             //通话时间戳
} PhoneCallListT;

typedef struct
{
    char number[CONTACT_NUMBER_LENGTH + 1];         //号码，UI自己去电话簿里查询姓名
    char type[2];                                   //参考: PhoneCallTypeT
    char sim[2];                                    //参考: PhoneSimT
    char time[11];                                  //通话时长，单位：秒
    char timeStamp[11];                             //通话时间戳
} PhoneCallListStrT;


int32_t addCallList(PhoneCallListT *callList);
int32_t deleteCallList(uint32_t index);
int32_t getCallList(uint32_t index, PhoneCallListT *callList);
int32_t getCallListCount(uint32_t *count);
int32_t getCallListOneItemCount(char *number, uint32_t *count);
int32_t getCallListOneItemIds(char *number, uint32_t *ids, uint32_t *length);


#endif
