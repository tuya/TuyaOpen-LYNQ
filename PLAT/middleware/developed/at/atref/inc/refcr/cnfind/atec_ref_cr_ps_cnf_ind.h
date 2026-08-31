/******************************************************************************

*(C) Copyright 2018 EIGENCOMM International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: atec_ref_cr_ps_cnf_ind.h
*
*  Description:
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef __ATEC_REF_CR_PS_CNF_IND_H__
#define __ATEC_REF_CR_PS_CNF_IND_H__

#include "cmicomm.h"
#include "cms_api.h"

void atRefCrDevProcCmiCnf(CamCmiCnf *pCmiCnf);
void atRefCrSimProcCmiCnf(CamCmiCnf *pCmiCnf);
void atRefCrPsProcCmiCnf(CamCmiCnf *pCmiCnf);
void atRefCrPsProcNmApplInd(CmsApplInd *pApplInd);
CmsRetId refCrPsMWIFISCANInd(UINT32 chanId, void *paras);



#endif

