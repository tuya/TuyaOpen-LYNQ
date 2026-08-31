#ifndef  __FEATURE_CALL_LIST_H__
#define  __FEATURE_CALL_LIST_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "app.h"
#include "feature.h"


int32_t callList_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable);
int32_t callList_proc(AppInfoT *appInfo, AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
int32_t callListDialled_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable);
int32_t callListDialled_proc(AppInfoT *appInfo, AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
int32_t callListNewSms_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable);
int32_t callListNewSm_proc(AppInfoT *appInfo, AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
int32_t callListDetail_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable);
int32_t callListDetail_proc(AppInfoT *appInfo, AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
int32_t callListAddToContact_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable);
int32_t callListAddToContact_proc(AppInfoT *appInfo, AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
int32_t callListAddToBlockList_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable);
int32_t callListAddToBlockList_proc(AppInfoT *appInfo, AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
int32_t callListDelete_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable);
int32_t callListDelete_proc(AppInfoT *appInfo, AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
int32_t callListDeleteAll_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable);
int32_t callListDeleteAll_proc(AppInfoT *appInfo, AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);


#ifdef __cplusplus
}
#endif
#endif 