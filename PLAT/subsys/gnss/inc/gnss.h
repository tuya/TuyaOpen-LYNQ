/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    gnss.h
 * Description:  EC618 gnss demo entry header file
 * History:      Rev1.0   2024-11-11
 *
 ****************************************************************************/
#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "cmsis_os2.h"
#include "subsys.h"
#include "api_gnss.h"

#define GNSS_RX_SIZE_MAX (1024)
// osEventFlagsId_t uartEvtHandle; // 不能使用static声明

typedef struct
{
    uint32_t open;
    uint32_t action;
} QueueGnssT;

typedef enum
{
    POWERCONTROL = 0,
} GnssAction;

#define QUEUE_SIZE_GNSS 100

#define GNSS_DEBUG_PRINTF 0
void subGnssInit();
void gnssStart(void);
void gnssStop(void);
bool gnssIsFix(void);
char *gnssGetLatitude(void);
char *gnssGetLongitude(void);
char *gnssGetAltitude(void);

#endif
