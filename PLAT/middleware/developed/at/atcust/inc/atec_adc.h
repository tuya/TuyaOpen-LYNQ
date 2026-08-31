/******************************************************************************

*(C) Copyright 2018 EIGENCOMM International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: atec_adc.h
*
*  Description:
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef __ATEC_ADC_H__
#define __ATEC_ADC_H__

#include "at_util.h"

#define  EC_ADC_STR_LEN_MAX   32
#define  EC_ADC_STR_BUF_SIZE  (EC_ADC_STR_LEN_MAX + 1)
#define  EC_ADC_STR_DEFAULT   NULL

#ifdef FEATURE_PLAT_REF_CR_PER_ADC_ENABLE
#define ATC_MADC_CHANNEL_VAL_MIN        0
#define ATC_MADC_CHANNEL_VAL_MAX        1
#define ATC_MADC_CHANNEL_VAL_DEFAULT    0

CmsRetId pdevMCHIPINFO(const AtCmdInputContext *pAtCmdReq);
CmsRetId pdevMADC(const AtCmdInputContext *pAtCmdReq);
#else
CmsRetId ecADC(const AtCmdInputContext *pAtCmdReq);
#endif

#endif

/* END OF FILE */

