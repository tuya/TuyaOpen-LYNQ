#ifndef  __FEATURE_CONTACT_H__
#define  __FEATURE_CONTACT_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "app.h"
#include "feature.h"

int32_t contact_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable);
extern int32_t contact_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
int32_t add_contact_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable);
int32_t add_contact_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
int32_t del_contact_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable);
int32_t del_contact_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
int32_t move_contact_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable);
int32_t move_contact_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
int32_t locate_contact_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable);
int32_t locate_contact_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
int32_t import_contact_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable);
int32_t import_contact_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
int32_t export_contact_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable);
int32_t export_contact_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
int32_t querycapacity_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable);
int32_t querycapacity_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);



#ifdef __cplusplus
}
#endif
#endif 