/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    app.h
 * Description:  EC618 at command demo entry header file
 * History:      Rev1.0   2018-07-12
 *
 ****************************************************************************/
 
#ifndef _DM_CTCC_H
#define _DM_CTCC_H

#include "slpman.h"
#include "mw_nvm_config.h"

#define CTCC_AUTO_REG_TASK_STACK_SIZE   3072 
#define CTCC_AUTO_REG_RETRY  0

typedef enum
{
    CTDM_INIT_STATE,
    CTDM_SIM_READY,
    CTDM_WAIT_IPREADY,
    CTDM_NOT_RIGHT_SIM,
    CTDM_IPREADY_STATE,
    CTDM_IDLE_STATE,
    CTDM_TIME_TO_RETRY
} ctdmStateMachine_t;

void ctccAutoRegisterInit(MWNvmCfgDmCtccParam* ctccParam, slpManSlpState_t slpState);
void ctccDeepSleepTimerCbRegister(void);
#endif
