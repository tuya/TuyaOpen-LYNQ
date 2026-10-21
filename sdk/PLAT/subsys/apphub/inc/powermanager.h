/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    powermanager.h
 * Description:  EC7xx powermanager header file
 * History:      Rev1.0   2018-07-12
 *
 ****************************************************************************/
#ifndef  SUBSYS_POWERMANAGER_H
#define  SUBSYS_POWERMANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "subsys.h"
#include "devicemanager.h"

typedef enum
{
    POWERMANAGER_NORMAL_POWER = 0,   //normal power mode,all devices are in normal power mode
	POWERMANAGER_FULL_POWER = 1,     //full power mode
    POWERMANAGER_LOW_POWER = 2,     //save power mode, all devices are in low power mode
    POWERMANAGER_SAVE_POWER = 3,     //save power mode, all devices are in power down mode
    POWERMANAGER_MAX_TYPE,
} PowerManagerStrategyType;

typedef void (*PowerManagerFuncT)(void);

typedef struct
{
    PowerManagerFuncT full;
    PowerManagerFuncT normal;
    PowerManagerFuncT low;
    PowerManagerFuncT save;
} PowerManagerFuncArrayT;


void    powerManagerInit(void);
void    powerManagerHandler(int32_t state);
int32_t powerManagerRegister(PowerManagerFuncArrayT *funcArray);
int32_t powerManagerUnregister(PowerManagerFuncArrayT *funcArray);


#ifdef __cplusplus
}
#endif

#endif /* SUBSYS_POWERMANAGER_H */