/******************************************************************************

*(C) Copyright 2018 EIGENCOMM International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename:
*
*  Description:
*
*  History: initiated by xuwang
*
*  Notes:
*
******************************************************************************/

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/
#include "fota_sal.h"
#include "fota_utils.h"
#include "fota_mem.h"
#include "fota_nvm.h"
#include "fota_patch.h"
#include "fota_custom.h"
#include "sctdef.h"

/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/

#if (WDT_FEATURE_ENABLE==1)
#if defined CHIP_EC616 || defined CHIP_EC616_Z0 || defined CHIP_EC616S || defined CHIP_EC626
extern void WDT_Kick(void);
extern void WDT_Start(void);
extern void WDT_Stop(void);

#define FOTA_WDT_KICK()   WDT_Kick()
#define FOTA_WDT_START()  WDT_Start()
#define FOTA_WDT_STOP()   WDT_Stop()
#else
extern void WDT_kick(void);
extern void WDT_start(void);
extern void WDT_stop(void);

#define FOTA_WDT_KICK()   WDT_kick()
#define FOTA_WDT_START()  WDT_start()
#define FOTA_WDT_STOP()   WDT_stop()
#endif
#endif


/*----------------------------------------------------------------------------*
 *                    DATA TYPE DEFINITION                                    *
 *----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*
 *                      PRIVATE FUNCTION DECLEARATION                         *
 *----------------------------------------------------------------------------*/


extern void delay_us(uint32_t us);
extern void SelNormalOrURCPrint(uint32_t Sel);
extern uint32_t VerifyImageBodySignature(uint8_t is224, uint8_t *hash, uint8_t *ecsda);

extern int32_t FOTA_getDfuProgress(FotaDefDfuProgress_t *prog);

/*----------------------------------------------------------------------------*
 *                      GLOBAL VARIABLES                                      *
 *----------------------------------------------------------------------------*/

#ifdef FEATURE_FOTA_SIGN_ENABLE
extern unsigned char fotaKey[FOTA_BUF_SIZE_64];
#endif


/*----------------------------------------------------------------------------*
 *                      PRIVATE FUNCTIONS                                     *
 *----------------------------------------------------------------------------*/
static void fotaGetRamSize(FotaDefGetRamSize_t *ram)
{
    ram->avlbSize = 512 * FOTA_BUF_SIZE_1K;;
}

static void fotaCheckBattery(FotaDefChkBattery_t *batt)
{
    batt->isBattLow = 0;
}

static void fotaCheckDynMem(FotaDefChkDynMem_t *dmem)
{
    dmem->isEnable = fotaIsSuppDynMem();
}

static void fotaCheckTotalProgress(FotaDefDfuProgress_t *prog)
{
    prog->isTotal = 1;
#if defined CHIP_EC718 || defined CHIP_EC716
    prog->rptFreq = FOTA_DPR_FREQ_HIGH;
#else
    prog->rptFreq = FOTA_DPR_FREQ_MEDIUM;
#endif
}

static int32_t fotaReportDfuProgress(FotaDefDfuProgress_t *prog)
{
#ifdef FEATURE_FOTA_ENABLE
    /* LOG PORT */
    FOTA_TRACE(LOG_DEBUG,"FOTA: PKG[%d/%d:%d] upgrading..., %d\n", \
                                    prog->pkgId + 1, prog->pkgNum, \
                                    prog->isTotal, prog->percent);

    /* AT PORT */
    SelNormalOrURCPrint(1);
#ifndef FEATURE_REF_CR_ENABLE
    if(prog->percent == 0)
    {
        FOTA_LOGI(LOG_DEBUG,"+QIND: \"FOTA\",\"START\"\r\n");
    }
    FOTA_LOGI(LOG_DEBUG,"+QIND: \"FOTA\",\"UPDATING\",%d\r\n",prog->percent);
    //FOTA_LOGI(LOG_DEBUG,"+QIND: \"FOTA\",\"UPDATING\",%d%%,1,1\r\n",prog->percent);
#else
    FOTA_LOGI(LOG_DEBUG,"+MFWUPGRADE: 1,%d\r\n",prog->percent);
#endif
    SelNormalOrURCPrint(0);
#endif

    return FOTA_EOK;
}

PLAT_BL_CIRAM_FLASH_TEXT static int32_t fotaReportDfuResult(void)
{
#ifdef FEATURE_FOTA_ENABLE
    int32_t      deltaState = FOTA_DCS_DELTA_UNDEF;
    int32_t       otaResult = fotaNvmGetOtaResult(FOTA_NVM_ZONE_ANY, &deltaState);

#ifndef FEATURE_REF_CR_ENABLE
    uint16_t        errCode = 0xffff;

    switch(otaResult)
    {
        case FOTA_DRC_DFU_SUCC:
            errCode = 0;
            break;
        case FOTA_DRC_DFU_FAIL:
            errCode = 100 + deltaState;
            break;

        default:
            break;
    }

    if(otaResult == FOTA_DRC_DFU_SUCC || otaResult == FOTA_DRC_DFU_FAIL)
    {
        /* AT PORT */
        SelNormalOrURCPrint(1);
        FOTA_LOGI(LOG_DEBUG,"+QIND: \"FOTA\",\"END\",%d\r\n", errCode);
        SelNormalOrURCPrint(0);
    }
#else
    if(otaResult == FOTA_DRC_DFU_SUCC || otaResult == FOTA_DRC_DFU_FAIL)
    {
        /* AT PORT */
        SelNormalOrURCPrint(1);
        FOTA_LOGI(LOG_DEBUG,"+MFWUPGRADE: %d\r\n", (otaResult == FOTA_DRC_DFU_SUCC ? 2 : 3));
        SelNormalOrURCPrint(0);
    }

#endif
#endif

    return FOTA_EOK;
}

static void fotaChkHlsState(FotaDefChkHlsState_t *hlsState)
{
#ifdef FEATURE_FOTA_HLS_ENABLE
    hlsState->isEnable = 1;
#else
    hlsState->isEnable = 0;
#endif
}

static void fotaChkBootState(FotaDefChkBootState_t *bootState)
{
#ifdef CONFIG_PROJ_APP_SECURITY_BOOT
    bootState->isSigned = 1;
#else
    bootState->isSigned = 0;
#endif
}

static int32_t fotaChkBootHdSec(FotaDefChkBootHdSec_t *bootHdSec)
{
#ifdef CONFIG_PROJ_APP_SECURITY_BOOT
    //TODO: ADD IT HERE if the signature of a image needs to be verified!
    if(bootHdSec->fwAttr == FOTA_FA_SYSH)
    {
        //ECPLAT_PRINTF(UNILOG_FOTA, DO_EXTENSION_1, P_ERROR, "Fota chk boot hdsec!\n");
        return !VerifyImageBodySignature(bootHdSec->shaAlgo == FOTA_CA_SHA224SUM,
                                         bootHdSec->hash,
                                         bootHdSec->ecdsa) ? FOTA_EOK : FOTA_EVERIFY;
    }
    else
    {
        //TODO: ADD IT HERE and return OK if the signature of a specific image could be omited!
        //return FOTA_EOK;
    }
#endif

    return FOTA_EVERIFY;
}

static int32_t fotaChkXipZone(FotaDefChkXipZone_t *xipZone)
{
#ifdef FEATURE_FOTA_FS_ENABLE
    if(xipZone->zid == FOTA_NVM_ZONE_DELTA)
    {
        xipZone->isXipEn = 0;
        return FOTA_EOK;
    }
#endif

    if(EF_FLASH_XIP_ADDR == fotaNvmGetExtras(xipZone->zid))
    {
        xipZone->isXipEn = 0;
    }
    else
    {
        xipZone->isXipEn = 1;
    }

    return FOTA_EOK;
}

static void fotaChkSignState(FotaDefChkSignState_t *chkSign)
{
#ifdef FEATURE_FOTA_SIGN_ENABLE
    chkSign->isEnable = 1;
#else
    chkSign->isEnable = 0;
#endif
}

static int32_t fotaChkFullImage(FotaDefChkFullImage_t *chkFullImg)
{
    if(chkFullImg->fwAttr == FOTA_FA_APP || chkFullImg->fwAttr == FOTA_FA_APP2)
    {
        chkFullImg->isSupp = 1;
    }
    else
    {
        chkFullImg->isSupp = 0;
    }

    return FOTA_EOK;
}

/*----------------------------------------------------------------------------*
 *                      GLOBAL FUNCTIONS                                      *
 *----------------------------------------------------------------------------*/
/******************************************************************************
 * @brief : fotaGetDeltaPubKey
 * @author: Xu.Wang
 * @note  :
 ******************************************************************************/
uint8_t* fotaGetDeltaPubKey(void)
{
    uint8_t  *pubKey = NULL;

#ifdef FEATURE_FOTA_SIGN_ENABLE
    pubKey = &fotaKey[0];
#endif

    return pubKey;
}

/******************************************************************************
 * @brief : fotaDoExtension
 * @author: Xu.Wang
 * @note  :
 ******************************************************************************/
PLAT_BL_CIRAM_FLASH_TEXT int32_t fotaDoExtension(uint8_t flags, void *args)
{
    int32_t retCode = FOTA_EOK;

    switch(flags)
    {
        case FOTA_DEF_US_DELAY:
            delay_us(((FotaDefUsDelay_t*)args)->usec);
            break;
    #if (WDT_FEATURE_ENABLE==1)
        case FOTA_DEF_WDT_KICK:
            FOTA_WDT_KICK();
            break;
        case FOTA_DEF_WDT_START:
            FOTA_WDT_START();
            break;
        case FOTA_DEF_WDT_STOP:
            FOTA_WDT_STOP();
            break;
    #else
        case FOTA_DEF_WDT_KICK:
        case FOTA_DEF_WDT_START:
        case FOTA_DEF_WDT_STOP:
            ECPLAT_PRINTF(UNILOG_FOTA, DO_EXTENSION_1, P_ERROR, "FotaDoExt: WDT is disabled!\n");
            break;
    #endif
        case FOTA_DEF_GET_RAM_SIZE:
            fotaGetRamSize((FotaDefGetRamSize_t*)args);
            break;
        case FOTA_DEF_CHK_BATTERY:
            fotaCheckBattery((FotaDefChkBattery_t*)args);
            break;
        case FOTA_DEF_CHK_DYN_MEM:
            fotaCheckDynMem((FotaDefChkDynMem_t*)args);
            break;
        case FOTA_DEF_CHK_TOTAL_PROGRESS:
            fotaCheckTotalProgress((FotaDefDfuProgress_t*)args);
            break;
        case FOTA_DEF_RPT_DFU_PROGRESS:
            retCode = fotaReportDfuProgress((FotaDefDfuProgress_t*)args);
            break;
        case FOTA_DEF_GET_DFU_PROGRESS:
        #ifdef FEATURE_FOTA_ENABLE
            retCode = FOTA_getDfuProgress((FotaDefDfuProgress_t*)args);
        #endif
            break;
        case FOTA_DEF_RPT_DFU_RESULT:
            retCode = fotaReportDfuResult();
            break;
        case FOTA_DEF_CHK_HLS_STATE:
            fotaChkHlsState((FotaDefChkHlsState_t*)args);
            break;
        case FOTA_DEF_CHK_BOOT_STATE:
            fotaChkBootState((FotaDefChkBootState_t*)args);
            break;
        case FOTA_DEF_CHK_BOOT_HDSEC:
            retCode = fotaChkBootHdSec((FotaDefChkBootHdSec_t*)args);
            break;
        case FOTA_DEF_CHK_XIP_ZONE:
            retCode = fotaChkXipZone((FotaDefChkXipZone_t*)args);
            break;
        case FOTA_DEF_CHK_SIGN_STATE:
            fotaChkSignState((FotaDefChkSignState_t*)args);
            break;
        case FOTA_DEF_CHK_FULL_IMAGE:
            fotaChkFullImage((FotaDefChkFullImage_t*)args);
            break;


        case FOTA_DEF_CHK_REMAP_ZONE:
        case FOTA_DEF_CHK_DELTA_STATE:
        case FOTA_DEF_CHK_BASE_IMAGE:
        case FOTA_DEF_IS_IMAGE_IDENTICAL:
        case FOTA_DEF_SET_DFU_RESULT:
        case FOTA_DEF_GET_DFU_RESULT:
        case FOTA_DEF_PREPARE_DFU:
        case FOTA_DEF_CLOSING_DFU:
        case FOTA_DEF_ADJ_ZONE_SIZE:
            retCode = fotaNvmDoExtension(flags, args);
            break;

        default:
            ECPLAT_PRINTF(UNILOG_FOTA, DO_EXTENSION_2, P_ERROR, "FotaDoExt: undef flags(%d)!\n", flags);
            break;
    }

    return retCode;
}

#ifdef FEATURE_FOTA_CORE2_ENABLE
int32_t FOTA_patchImage(FotaPsegCtrlUnit_t *pscu, const FotaPscuCallbacks_t *callbacks)
{
#ifdef FOTA_PATCH_STUB
    return FOTA_patchStub(pscu, callbacks);
#else
    extern int32_t FOTA_patchEntry(FotaPsegCtrlUnit_t *pscu, const FotaPscuCallbacks_t *callbacks);
    return FOTA_patchEntry(pscu, callbacks);
#endif
}

#else
int32_t FOTA_patchImage(FotaPsegCtrlUnit_t *pscu, const FotaPscuCallbacks_t *callbacks)
{
#ifdef FOTA_PATCH_STUB
    return FOTA_patchStub(pscu, callbacks);
#else
    extern int32_t patch_integrate_entry(FotaPsegCtrlUnit_t *pscu, const FotaPscuCallbacks_t *callbacks);
    return patch_integrate_entry(pscu, callbacks);
#endif
}

#if (!defined CHIP_EC618 && !defined CHIP_EC618_Z0 && !defined CHIP_EC626)
uint8_t BSP_QSPI_Write(uint8_t* pData, uint32_t WriteAddr, uint32_t Size)
{
    return FLASH_writeBl(pData, WriteAddr, Size);
}

uint8_t  BSP_QSPI_Erase_Sector(uint32_t SectorAddress)
{
    return FLASH_eraseSectorSafe(SectorAddress);
}
#endif

uint32_t GetDltFileZoneId(void)
{
    return FOTA_NVM_ZONE_DELTA;
}

uint32_t GetBackupZoneId(void)
{
    return FOTA_NVM_ZONE_BKUP;
}
#endif



