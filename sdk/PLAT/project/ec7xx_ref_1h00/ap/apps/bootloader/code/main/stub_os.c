#include <string.h>
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#include "slpman.h"
#include "plat_config.h"
#include "sctdef.h"

#ifndef IS_IRQ_MODE
#define IS_IRQ_MODE()             (__get_IPSR() != 0U)
#endif


PLAT_BL_CIRAM_FLASH_TEXT osStatus_t osSemaphoreAcquire (osSemaphoreId_t semaphore_id, uint32_t timeout) {
    return (osStatus_t)0;
}

PLAT_BL_CIRAM_FLASH_TEXT osStatus_t osSemaphoreDelete (osSemaphoreId_t semaphore_id) {
    return (osStatus_t)0;
}

PLAT_BL_CIRAM_FLASH_TEXT osSemaphoreId_t osSemaphoreNew (uint32_t max_count, uint32_t initial_count, const osSemaphoreAttr_t *attr) {
    return 0;
}

PLAT_BL_CIRAM_FLASH_TEXT osStatus_t osSemaphoreRelease (osSemaphoreId_t semaphore_id) {
    return (osStatus_t)0;
}

PLAT_BL_CIRAM_FLASH_TEXT slpManRet_t slpManRegisterPredefinedBackupCb(slpCbModule_t module, slpManBackupCb_t backup_cb, void *pdata)
{
    return (slpManRet_t)0;
}

PLAT_BL_CIRAM_FLASH_TEXT slpManRet_t slpManRegisterPredefinedRestoreCb(slpCbModule_t module, slpManRestoreCb_t restore_cb, void *pdata)
{
    return (slpManRet_t)0;
}

PLAT_BL_CIRAM_FLASH_TEXT void PhyDebugPrint(uint8_t moduleID, uint8_t subID, uint32_t *pSwLogData, uint32_t swLogLen, uint8_t uniLogLevel, uint8_t ramLogSwitch)

{
    return;
}

PLAT_BL_CIRAM_FLASH_TEXT slpManRet_t slpManDrvVoteSleep(slpDrvVoteModule_t module, slpManSlpState_t status)
{
    return (slpManRet_t)0;
}

PLAT_BL_CIRAM_FLASH_TEXT char *pcTaskGetName( osThreadId_t xTaskToQuery )
{
    return NULL;
}

PLAT_BL_CIRAM_FLASH_TEXT osThreadId_t xTaskGetCurrentTaskHandle( void )
{
    return (osThreadId_t)NULL;
}

PLAT_BL_CIRAM_FLASH_TEXT osStatus_t osDelay (uint32_t ticks)
{
    return (osStatus_t)0;
}

PLAT_BL_CIRAM_FLASH_TEXT osKernelState_t osKernelGetState (void)
{
    return (osKernelState_t)0;
}

PLAT_BL_CIRAM_FLASH_TEXT uint8_t osThreadIsSuspendAll (void)
{
    return 0;
}

PLAT_BL_CIRAM_FLASH_TEXT uint8_t PhyLogLevelGet (void)
{
    return 0;
}

PLAT_BL_UNCOMP_FLASH_TEXT void STUB_WakeupIntHandler(void)
{

}

PLAT_BL_UNCOMP_FLASH_TEXT void PwrKey_WakeupIntHandler()
{

}

PLAT_BL_UNCOMP_FLASH_TEXT void ChrgPad_WakeupIntHandler()
{

}

