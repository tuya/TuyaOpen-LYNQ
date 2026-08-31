#ifndef  __SYSSERVICE_H__
#define  __SYSSERVICE_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "app.h"
#ifdef FEATURE_SUBSYS_FEATURES_ENABLE
#include "feature.h"
#endif


typedef enum
{
    SYSSERVICE_EVENT_POWER_OFF = 0,
    SYSSERVICE_EVENT_LOW_BATTERY_ALARM = 1,
    SYSSERVICE_EVENT_HIGH_TEMP_ALARM = 2,
    SYSSERVICE_EVENT_TELECOM   = 100,
} SysserviceEventT;


int32_t sysservice_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
void    poweroff(void);


#ifdef __cplusplus
}
#endif
#endif 