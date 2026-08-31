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
#ifndef _ATEC_REF_CR_LBS_TASK_H
#define _ATEC_REF_CR_LBS_TASK_H

#include "at_util.h"


#define CR_LBS_REQUST_PAYLOAD_LEN         160
#define CR_LBS_REQUST_DATA_LEN            320
#define CR_LBS_RECV_DATA_LEN              800
#define CR_LBS_RECV_POS_LEN               160

#define CR_LBS_REQUST_RETRY_COUNT_MAX     5

/*
 * APPL SGID: APPL_CR_LBS, related PRIM ID
*/
enum applMHttpPrimId_Enum
{
    APPL_CR_LBS_PRIM_ID_BASE = 0,

    APPL_CR_LBS_URC_IND,

    APPL_CR_LBS_PRIM_ID_END = 0xFF
};


bool crLbsTaskInit(int atHandle);

#endif

