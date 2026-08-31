/******************************************************************************

*(C) Copyright 2018 EIGENCOMM International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename:
*
*  Description:
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef __ATEC_ADC_CNF_IND_H__
#define __ATEC_ADC_CNF_IND_H__

#include "at_util.h"

#ifdef FEATURE_PLAT_REF_CR_PER_ADC_ENABLE
CmsRetId mAdcCnf(UINT16 reqHandle, UINT16 rc, void *paras);
#else
CmsRetId ecAdcCnf(UINT16 reqHandle, UINT16 rc, void *paras);
#endif

void atApplAdcProcCmsCnf(CmsApplCnf *pCmsCnf);

#endif

/* END OF FILE */
