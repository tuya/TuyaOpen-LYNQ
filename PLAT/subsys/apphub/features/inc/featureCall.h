#ifndef  __FEATURE_CALL_H__
#define  __FEATURE_CALL_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "app.h"
#include "feature.h"
#include "phone.h"

int32_t callNumberInput_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
int32_t callInCall_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable);
int32_t callInCall_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
int32_t callInComing_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
int32_t callDialout_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
void    setCallState(uint8_t state);
void    recordCallBeginTime(void);
char    *callTypeToString(uint8_t type);
char    *timeStampToString(uint32_t timeStamp);
void    show_call_incoming(char *num);

#ifdef __cplusplus
}
#endif
#endif 