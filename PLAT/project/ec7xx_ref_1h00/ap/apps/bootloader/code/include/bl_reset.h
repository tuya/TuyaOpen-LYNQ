
#ifndef BL_RESET_H
#define BL_RESET_H

#include "reset.h"

/**
* @brief   void blResetReasonGet(LastResetState_e *apRstState, LastResetState_e *cpRstState);

* @details get reset reason in bootloader. 
           There are som restrictions: after obtaining the reset reason, the bootloader must start the application and run until
           ResetReasonInit is called. This ensures the reset reason retrieved next time will be accurate. If the bootloader obtains
           the reset reason but fails to start the application(for example, reboots again directly within the bootloader), 
           the subsequently obtained reset reason will be inaccurate.
* @note
*/
void blResetReasonGet(LastResetState_e *apRstState, LastResetState_e *cpRstState);
/**
* @brief   LastResetState_e blResetReasonTranslate(ResetReason_e resetReason);


* @details translate ResetReason_e to LastResetState_e
           blResetReasonTranslate is called in blGetResetReason
           ResetReason_e is a flag written to register before reset start
           the result LastResetState_e is returned and report by blGetResetReason
* @note
*/
LastResetState_e blResetReasonTranslate(ResetReason_e resetReason);

/**
* @brief   void blResetReasonClear(void)


* @details clear reset reason if a reset will happen in bootloader, so ResetReasonInit will not call
           clear reset reason to make sure the next time will obtain an accurate reset reason. 
           !!!! Only call if you reset in bootloader and do not enter app.!!!!!!!
* @note
*/
void blResetReasonClear(void);

#endif

