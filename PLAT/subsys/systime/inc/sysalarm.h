#ifndef  __SYS_ALARM_H__
#define  __SYS_ALARM_H__


#include <stdint.h>
#include <stdbool.h>


typedef struct
{
    char    name[32];
    char    tone[32];
    bool    vibrate;
    bool    on;
    uint8_t hour;
    uint8_t minute;
    uint8_t repeat;
} SysAlarmInfoT;
typedef void (*SysAlarmCallbackT)(SysAlarmInfoT *sysAlarmInfo);


int32_t        sysAlarmInit(SysAlarmCallbackT callback, uint32_t countMax);
int32_t        sysAlarmAdd(SysAlarmInfoT *sysAlarmInfo);
int32_t        sysAlarmDelete(uint32_t id);
int32_t        sysAlarmEdit(uint32_t id, SysAlarmInfoT *sysAlarmInfo);
int32_t        sysAlarmSync(void);
int32_t        sysAlarmSetState(uint32_t id, bool on);
int32_t        sysAlarmGetState(uint32_t id, bool *on);
uint32_t       sysAlarmGetCount(void);
SysAlarmInfoT *sysAlarmGetList(void);


#endif 
