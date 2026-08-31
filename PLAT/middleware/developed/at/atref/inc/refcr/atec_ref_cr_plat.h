/******************************************************************************

*(C) Copyright 2018 EIGENCOMM International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: atec_http.h
*
*  Description:
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef _ATEC_REF_CR_PLAT_H
#define _ATEC_REF_CR_PLAT_H

#include "at_util.h"


#define EC_CR_PRINT_BUF_LEN        256

#define CR_MCGSNW_TYPE_MIN          0
#define CR_MCGSNW_TYPE_MAX          0
#define CR_MCGSNW_TYPE_DEF          0

/* AT+MEMINFO */
#define CR_MTSETID_TYPE_MIN          0
#define CR_MTSETID_TYPE_MAX          1
#define CR_MTSETID_TYPE_DEF          0

#define CR_MTSETID_1_SN_MAX_LEN             32
#define CR_MTSETID_1_SN_STR_DEF             NULL

#define CR_MTSETID_1_IMEI_MAX_LEN             16
#define CR_MTSETID_1_IMEI_STR_DEF             NULL



/* AT+MEMINFO */
#define MEMINFO_0_MAX_LEN             33
#define MEMINFO_0_STR_DEF             NULL

CmsRetId crMSWVER(const AtCmdInputContext *pAtCmdReq);
CmsRetId crMBSVER(const AtCmdInputContext *pAtCmdReq);
CmsRetId crMHWVER(const AtCmdInputContext *pAtCmdReq);
CmsRetId crMWHWVER(const AtCmdInputContext *pAtCmdReq);
CmsRetId crMCGSNW(const AtCmdInputContext *pAtCmdReq);
CmsRetId crMTSETID(const AtCmdInputContext *pAtCmdReq);
CmsRetId crMEMINFO(const AtCmdInputContext *pAtCmdReq);
CmsRetId crMFSINFO(const AtCmdInputContext *pAtCmdReq);
CmsRetId crMTEST(const AtCmdInputContext *pAtCmdReq);

#endif

