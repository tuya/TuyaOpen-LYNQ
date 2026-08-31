#ifndef __PS_STK_BIP_API_H__
#define __PS_STK_BIP_API_H__
/******************************************************************************
 ******************************************************************************
 Copyright:      - 2017- Copyrights of EigenComm Ltd.
 File name:      - psstk_bip_api.h
 Description:    - API called by psproxy task
 ******************************************************************************
******************************************************************************/
#include <osasys.h>

/******************************************************************************
 *****************************************************************************
 * MARCO
 *****************************************************************************
******************************************************************************/
#if (defined TYPE_EC718S) || (defined TYPE_EC716S)
//#if defined GCF_FEATURE_MODE
#undef     PS_ENABLE_SIM_BIP_APP_FEATURE
//#else
//#define    PS_ENABLE_SIM_BIP_APP_FEATURE
//#endif
#else
#define    PS_ENABLE_SIM_BIP_APP_FEATURE
#endif



//#define FEATURE_PS_STK_RAW_CMD_ENABLE

//#define FEATURE_PS_STK_RAW_CMD_REPORT_ENABLE
/******************************************************************************
 *****************************************************************************
 * ENUM
 *****************************************************************************
******************************************************************************/

/******************************************************************************
 ******************************************************************************
 * External global variable
 ******************************************************************************
******************************************************************************/

/******************************************************************************
 *****************************************************************************
 * Functions
 *****************************************************************************
******************************************************************************/

void psStkBipProcCmiInd(const SignalBuf *indSignalPtr);
void psStkBipProcCmiCnf(const SignalBuf *cnfSignalPtr);

/*
 * processed the signal send to STK CMS, if processed, return TRUE
*/
BOOL psStkBipProcSimBipSig(const SignalBuf *pSig);

void psStkBipProcApplInd(CmsApplInd *pAppInd);

void psStkProcCmiInd(const SignalBuf *indSignalPtr, BOOL bStkDisable);

void psStkProcTimerExpiry(OsaTimerExpiry *pTimerExpiry);

#endif

