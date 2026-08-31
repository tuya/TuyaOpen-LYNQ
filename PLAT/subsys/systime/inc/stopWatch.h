#ifndef  __STOP_WATCH_H__
#define  __STOP_WATCH_H__


#include <stdint.h>


typedef void (*StopWatchCallbackT)(uint32_t *timeList, uint32_t timeCount);


int32_t stopWatchInit(StopWatchCallbackT callback, uint32_t interval, uint32_t countMax);
int32_t stopWatchDeinit(void);
int32_t stopWatchStart(void);
int32_t stopWatchStop(void);
int32_t stopWatchCount(void);
int32_t stopWatchReset(void);

#endif 
