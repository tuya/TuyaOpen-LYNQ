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
#include "pwrkey.h"


#define POWER_MANAGER_CALLBACK_NAME_LENGTH_MAX      32


typedef enum
{
    POWERMANAGER_NORMAL_POWER = 10,     //normal power mode,all devices are in normal power mode
    POWERMANAGER_FULL_POWER,            //full power mode
    POWERMANAGER_LOW_POWER,             //save power mode, all devices are in low power mode
    POWERMANAGER_HEARTBEAT_SAVE_POWER,  //heartbeat save power mode, all devices are in power down mode
    POWERMANAGER_SAVE_POWER,            //save power mode, all devices are in power down mode
    POWERMANAGER_MAX_TYPE,
} PowerManagerStrategyType;

typedef int32_t (*PowerManagerFuncT)(void);

typedef struct
{
    char name[POWER_MANAGER_CALLBACK_NAME_LENGTH_MAX + 1];
    PowerManagerFuncT full;
    PowerManagerFuncT normal;
    PowerManagerFuncT low;
    PowerManagerFuncT save;
} PowerManagerCallbackT;

typedef enum
{
    PROHIBIT_NOTHING = 1,
    PROHIBIT_ENTER_SLEEP,
    PROHIBIT_TURN_OFF_BACKLIGHT,
    PROHIBIT_MAX
} ProhibitionStateT;

typedef enum
{
    POWER_STATE_WORKING = 0,
    POWER_STATE_GO_TO_SLEEP,
    POWER_STATE_IN_SLEEP,
} PowerStateT;

typedef enum
{
    BOOT_RESULT_POWER_ON = 0,
    BOOT_RESULT_POWER_OFF,
    BOOT_RESULT_POWER_OFF_CHARGING,
    BOOT_RESULT_MAX
} BootResultT;


typedef void (*HbPowerSaveFuncT)(void *param, int32_t state);

typedef struct
{
    HbPowerSaveFuncT  callback;
    void             *param;
} HbPowerSaveCallbackT;

typedef struct
{
    uint32_t             timerTime;
    HbPowerSaveCallbackT timerCallback;
    HbPowerSaveCallbackT sleepCallback;
    HbPowerSaveCallbackT lcdCallback;
} HbPowerSaveT;

typedef enum
{
    HB_POWER_SAVE_IOCTL_TIMER_TIME,
    HB_POWER_SAVE_IOCTL_TIMER_CALLBACK,
    HB_POWER_SAVE_IOCTL_TIMER_CALLBACK_PARAM,
    HB_POWER_SAVE_IOCTL_SLEEP_CALLBACK,
    HB_POWER_SAVE_IOCTL_SLEEP_CALLBACK_PARAM,
    HB_POWER_SAVE_IOCTL_MAX
} HbPowerSaveIoctlT;


void    powerManagerInit(void);
int32_t powerManagerSetState(uint8_t state);
uint8_t powerManagerGetCurrnetPowerSaveMode(void);
int32_t powerManagerRegister(PowerManagerCallbackT *callback);
int32_t powerManagerUnregister(char *name);
uint8_t powerManagerGetVbusState(void);
void    powerManagerVbusHandler(void);
uint8_t powerManagerBootProcess(void);
void    powerManagerPowerKeyHandler(pwrKeyPressStatus status);
bool    powerManagerGetBacklightState(void);
void    powerManagerSetBacklightDuration(uint32_t backlightDuration);
void    powerManagerResetBacklightTimer(void);
uint8_t powerManagerGetPowerState(void);
int32_t powerManagerProhibitionCreate(uint8_t *handle);
int32_t powerManagerProhibitionDelete(uint8_t handle);
uint8_t powerManagerProhibitionQuery(void);
int32_t powerManagerProhibitionSet(uint8_t handle, uint8_t prohibition);
int32_t powerManagerHbPowerSaveCreate(HbPowerSaveT *hbPowerSave);
int32_t powerManagerHbPowerSaveDelete(int32_t handle);
int32_t powerManagerHbPowerSaveIoctl(int32_t handle, int32_t type, void *param);


#ifdef __cplusplus
}
#endif

#endif /* SUBSYS_POWERMANAGER_H */