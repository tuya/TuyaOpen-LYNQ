#ifndef  __FEATURE_TTS_H__
#define  __FEATURE_TTS_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "app.h"
#include "feature.h"

int32_t tts_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable);
int32_t tts_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);

#ifdef __cplusplus
}
#endif
#endif 