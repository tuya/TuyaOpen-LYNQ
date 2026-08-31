/******************************************************************************

*(C) Copyright 2018 EIGENCOMM International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: atec_ref_cr_http_cnf_ind.h
*
*  Description: Process http(s) client related AT commands
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef _ATEC_REF_CR_HTTP_CNF_IND_H_
#define _ATEC_REF_CR_HTTP_CNF_IND_H_

#include "at_util.h"

void atApplMHttpProcCmsCnf(CmsApplCnf *pCmsCnf);
void atApplMHttpProcCmsInd(CmsApplInd *pCmsInd);

#endif

