#ifndef  __FEATURE_PROFILE_H__
#define  __FEATURE_PROFILE_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "app.h"
#include "feature.h"

int32_t profileList_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable);
int32_t profileList_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
int32_t profileNormalList_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable);
int32_t profileNormalList_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);

#ifdef __cplusplus
}
#endif
#endif 