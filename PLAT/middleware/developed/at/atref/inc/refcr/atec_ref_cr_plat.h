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


/* AT+MEMINFO */
#define MEMINFO_0_MAX_LEN             16
#define MEMINFO_0_STR_DEF             NULL


CmsRetId crMSWVER(const AtCmdInputContext *pAtCmdReq);
CmsRetId crMBSVER(const AtCmdInputContext *pAtCmdReq);
CmsRetId crMHWVER(const AtCmdInputContext *pAtCmdReq);
CmsRetId crMWHWVER(const AtCmdInputContext *pAtCmdReq);
CmsRetId crMCGSNW(const AtCmdInputContext *pAtCmdReq);
CmsRetId crMTSETID(const AtCmdInputContext *pAtCmdReq);
CmsRetId crMEMINFO(const AtCmdInputContext *pAtCmdReq);

#endif

