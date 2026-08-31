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
#ifndef _ATEC_REF_CR_LBS_H
#define _ATEC_REF_CR_LBS_H

#include "at_util.h"

#define CR_LBS_PRINT_BUF_LEN        256

/* AT+MLBSCFG */
#define MLBSCFG_0_MAX_LEN             32
#define MLBSCFG_0_STR_DEF             NULL


CmsRetId crMLBSCFG(const AtCmdInputContext *pAtCmdReq);
CmsRetId crMLBSLOC(const AtCmdInputContext *pAtCmdReq);

#endif

