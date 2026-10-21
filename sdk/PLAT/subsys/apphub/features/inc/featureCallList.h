#ifndef  __FEATURE_CALL_LIST_H__
#define  __FEATURE_CALL_LIST_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "app.h"
#include "feature.h"


int32_t listCallList_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable);
int32_t listCallList_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
int32_t newCallList_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
int32_t deleteCallList_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable);
int32_t deleteCallList_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
int32_t deleteAllCallList_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable);
int32_t deleteAllCallList_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
int32_t queryCallList_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable);
int32_t queryCallList_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);



#ifdef __cplusplus
}
#endif
#endif 