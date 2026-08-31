/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    test.h
 * Description:  EC618 mqtt demo entry header file
 * History:      Rev1.0   2018-07-12
 *
 ****************************************************************************/
#ifndef __MISC_H__
#define __MISC_H__


#include <stdint.h>
#include <stdbool.h>
#include "ps_event_callback.h"


#define SIM_UNKNOWN                 (PS_URC_GROUP_SIM - PS_URC_GROUP_SIM)
#define SIM_READY                   (PS_URC_ID_SIM_READY - PS_URC_GROUP_SIM)
#define SIM_REMOVED                 (PS_URC_ID_SIM_REMOVED - PS_URC_GROUP_SIM)


void    miscInit(void);
bool    nwIsReady(void);
bool    chargeIsOn(void);
uint8_t bootPinValueGet(void);
void    blueLedSetState(bool on);
uint8_t simGetStatus(uint8_t sim);


#endif
