/******************************************************************************

*(C) Copyright 2018 EIGENCOMM International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: atec_ref_cr_tcpip_cnf_ind.h
*
*  Description: Process packet switched service related AT commands
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef __ATEC_REF_CR_TCPIP_IND_H__
#define __ATEC_REF_CR_TCPIP_IND_H__

#include "at_util.h"

#ifdef FEATURE_PS_REF_CR_TCPIP_SOCK_AT_ENABLE

CmsRetId refCrSockPSTHCallback(UINT8 chanId, UINT8 curChanState, AtDataAndOnlineCmdSEvt eventId, void *pArg);

void atTcpIpProcRefCrSoclCnf(CmsApplCnf *pApplCnf);
void atTcpIpProcRefCrSoclInd(CmsApplInd *pApplInd);

#endif
void atTcpIpProcRefCrNmApplCnf(CmsApplCnf *pApplCnf);
void atTcpIpProcRefCrNmApplInd(CmsApplInd *pApplInd);

#endif
