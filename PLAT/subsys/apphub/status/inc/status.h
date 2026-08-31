/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    status.h
 * Description:  EC718 at command demo entry header file
 * History:      Rev1.0   2018-07-12
 *
 ****************************************************************************/
#ifndef  SUBSYS_STATUS_H
#define  SUBSYS_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "subsys.h"
#include "cmsis_os2.h"
#ifdef FEATURE_SUBSYS_MISC_ENABLE
#include "misc.h"
#endif


typedef struct
{
    uint32_t batteryVoltage : 23;
    uint32_t batteryLevel   : 7;
    uint32_t batteryCharge  : 2;
    uint32_t simStatus      : 2;
    uint32_t nwReady        : 1;
    uint32_t serverReady    : 1;
    uint32_t reserved       : 4;
    uint32_t csqLevel       : 8;
    uint32_t rssi           : 16;
    char     time[6];
} StatusT;



void       subStatusInit(void);
void       statusUpdateReportInternal(uint32_t internal);
osStatus_t statusGet(StatusT *status, uint32_t timeout);
int32_t    statusNotify(void);
uint8_t    getBatteryPercentage(void);
uint8_t    getCsqPercentage(void);


#ifdef __cplusplus
}
#endif

#endif /* SUBSYS_STATUS_H */