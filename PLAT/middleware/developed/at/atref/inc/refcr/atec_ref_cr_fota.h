/******************************************************************************

*(C) Copyright 2018 EIGENCOMM International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: atec_ref_cr_fota.h
*
*  Description:
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef __ATEC_REF_CR_FOTA_H__
#define __ATEC_REF_CR_FOTA_H__

#include "at_util.h"

#define   REF_CR_CFG_OPTION_MAXSIZE            32

#define   REF_CR_CFG_URL_MAXSIZE               256
#define   REF_CR_CFG_USER_MAXSIZE              48
#define   REF_CR_CFG_PASSWD_MAXSIZE            48

#define   REF_CR_CFG_TIMEOUT_MIN               30
#define   REF_CR_CFG_TIMEOUT_MAX               180
#define   REF_CR_CFG_TIMEOUT_DFT               60

#define   REF_CR_CFG_PATMODE_MIN               0
#define   REF_CR_CFG_PATMODE_MAX               1
#define   REF_CR_CFG_PATMODE_DFT               0

#define   REF_CR_CFG_PROGIND_MIN               0
#define   REF_CR_CFG_PROGIND_MAX               0x7fffffff
#define   REF_CR_CFG_PROGIND_DFT               0

#define   REF_CR_CFG_SSLID_MIN                 0
#define   REF_CR_CFG_SSLID_MAX                 4
#define   REF_CR_CFG_SSLID_DFT                 0

#define   REF_CR_CFG_PKGSIZE_MIN               512
#define   REF_CR_CFG_PKGSIZE_MAX               3072
#define   REF_CR_CFG_PKGSIZE_DFT               512

#define   REF_CR_CHK_ALGO_MIN                  0
#define   REF_CR_CHK_ALGO_MAX                  2
#define   REF_CR_CHK_ALGO_DFT                  2

#define   REF_CR_DLOAD_PKGOFFS_MIN             0
#define   REF_CR_DLOAD_PKGOFFS_MAX             0x7fffffff
#define   REF_CR_DLOAD_PKGOFFS_DFT             0

#define   REF_CR_DLOAD_PKGLEN_MIN              1
#define   REF_CR_DLOAD_PKGLEN_MAX              512
#define   REF_CR_DLOAD_PKGLEN_DFT              1

CmsRetId  refCrFwCfg(const AtCmdInputContext *pAtCmdReq);
CmsRetId  refCrFwErase(const AtCmdInputContext *pAtCmdReq);
CmsRetId  refCrFwDload(const AtCmdInputContext *pAtCmdReq);
CmsRetId  refCrFwCheck(const AtCmdInputContext *pAtCmdReq);
CmsRetId  refCrFwUpgrade(const AtCmdInputContext *pAtCmdReq);

#endif

/* END OF FILE */

