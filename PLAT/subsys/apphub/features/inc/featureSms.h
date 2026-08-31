#ifndef  __FEATURE_SMS_H__
#define  __FEATURE_SMS_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "app.h"
#include "feature.h"

int32_t newSms_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable);
int32_t newSms_proc(AppInfoT *appInfo, AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
int32_t presetSms_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable);
int32_t presetSms_proc(AppInfoT *appInfo, AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
int32_t deleteSms_proc(AppInfoT *appInfo, AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
int32_t editSms_proc(AppInfoT *appInfo, AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
int32_t smsOutboxList_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable);
int32_t smsOutboxList_proc(AppInfoT *appInfo, AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
int32_t smsInboxList_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable);
int32_t smsInboxList_proc(AppInfoT *appInfo, AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);



#ifdef __cplusplus
}
#endif
#endif 