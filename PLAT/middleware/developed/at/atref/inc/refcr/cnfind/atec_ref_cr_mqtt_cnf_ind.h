/******************************************************************************

*(C) Copyright 2018 EIGENCOMM International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: atec_mqtt_cnf_ind.h
*
*  Description: Process MQTT related AT commands
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef _ATEC_MQTT_CNF_IND_H
#define _ATEC_MQTT_CNF_IND_H

#include "at_util.h"
#include "atec_ref_cr_mqtt.h"


CmsRetId crMqttSubCnf(UINT16 reqHandle, UINT16 rc, void *paras);
CmsRetId crMqttUnSubCnf(UINT16 reqHandle, UINT16 rc, void *paras);
CmsRetId crMqttPubCnf(UINT16 reqHandle, UINT16 rc, void *paras);
CmsRetId crMqttReadCnf(UINT16 reqHandle, UINT16 rc, void *paras);

CmsRetId crMqttConnInd(UINT16 indHandle, void *paras);
CmsRetId crMqttDiscInd(UINT16 indHandle, void *paras);
CmsRetId crMqttSubInd(UINT16 indHandle, void *paras);
CmsRetId crMqttUnSubInd(UINT16 indHandle, void *paras);
CmsRetId crMqttPubInd(UINT16 indHandle, void *paras);
CmsRetId crMqttPubRecInd(UINT16 reqhandle, void *paras);
CmsRetId crMqttPubCompInd(UINT16 reqhandle, void *paras);
CmsRetId crMqttPubRelInd(UINT16 reqhandle, void *paras);
CmsRetId crMqttStatInd(UINT16 indHandle, void *paras);
CmsRetId crMqttRecvInd(UINT16 indHandle, void *paras);
CmsRetId crMqttPingInd(UINT16 reqhandle, void *paras);

void atApplCrMqttProcCmsCnf(CmsApplCnf *pCmsCnf);
void atApplCrMqttProcCmsInd(CmsApplInd *pCmsInd);
#endif

