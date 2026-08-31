/******************************************************************************

*(C) Copyright 2018 EIGENCOMM International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: atec_onenet.h
*
*  Description:
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef _ATEC_ONENET_H
#define _ATEC_ONENET_H

#include "at_util.h"
#include "at_onenet_task.h"

#ifdef FEATURE_REF_AT_CR_ENABLE
/* AT+MIPLCFG */
#define ATC_MIPLCFG_3_PATTERN_VAL_MIN                   ENDPOINT_PATTERN_NO
#define ATC_MIPLCFG_3_PATTERN_VAL_MAX                   ENDPOINT_PATTERN_URN_IMEI
#define ATC_MIPLCFG_3_PATTERN_VAL_DEFAULT               ENDPOINT_PATTERN_NO
#define ATC_MIPLCFG_3_EPNAME_MAX_SIZE                   63

#endif

#ifndef FEATURE_REF_AT_CR_ENABLE
CmsRetId  onenetCONFIG(const AtCmdInputContext *pAtCmdReq);
#endif
CmsRetId  onenetCREATE(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetDELETE(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetOPEN(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetCLOSE(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetADDOBJ(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetDELOBJ(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetNOTIFY(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetREADRSP(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetWRITERSP(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetEXECUTERSP(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetOBSERVERSP(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetDISCOVERRSP(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetPARAMETERRSP(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetUPDATE(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetVERSION(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetSETIMSI(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetSETIMEI(const AtCmdInputContext *pAtCmdReq);
#if 0
CmsRetId  onenetOTASTART(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetOTASTATE(const AtCmdInputContext *pAtCmdReq);
#endif
#ifndef FEATURE_REF_AT_CR_ENABLE
CmsRetId  onenetMIPLRD(const AtCmdInputContext *pAtCmdReq);
#else
CmsRetId  onenetCFG(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetNMI(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetMGR(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetQMGR(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetCREATEEX(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetDEVINFO(const AtCmdInputContext *pAtCmdReq);
CmsRetId onenetDTLSNAT(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetUPDATESET(const AtCmdInputContext *pAtCmdReq);
#endif
#endif
