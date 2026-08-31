#ifndef _FEATURE_PLAYER_H_
#define _FEATURE_PLAYER_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "app.h"
#include "feature.h"

extern AppT mainApp;
extern int cur_menu_func_id;
extern sub_func sub_func_table[];

int32_t player_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
int32_t player_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable);

#ifdef __cplusplus
}
#endif
#endif