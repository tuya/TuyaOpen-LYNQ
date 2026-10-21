/******************************************************************************
*  Filename: atec_lynq.h
*
*  Description:
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef __ATEC_LYNQ_H__
#define __ATEC_LYNQ_H__

#include "at_util.h"
#include "osasys.h"


typedef enum
{
    LYNQ_PARAM_ERROR = 1,
    LYNQ_OPERATION_NOT_SUPPORT = 2,
    LYNQ_TASK_NOT_CREATE = 3,
    LYNQ_CONVERSION_TIMEOUT = 4,
} _AtLYNQCmdError;

extern CmsRetId mftestGPIO(const AtCmdInputContext* pAtCmdReq);
extern CmsRetId mftestcmGPIO(const AtCmdInputContext* pAtCmdReq);
extern CmsRetId mftestCMGPIO(const AtCmdInputContext* pAtCmdReq);
extern CmsRetId mftestXGPIO(const AtCmdInputContext* pAtCmdReq);
extern CmsRetId mftestADC(const AtCmdInputContext* pAtCmdReq);
extern CmsRetId mftestCMADC(const AtCmdInputContext* pAtCmdReq);

extern CmsRetId setMOPENLIGHT(const AtCmdInputContext* pAtCmdReq);
extern CmsRetId mATS24(const AtCmdInputContext *pAtCmdReq);

extern CmsRetId MLHFOTA(const AtCmdInputContext *pAtCmdReq);

#endif /*__ATEC_LYNQ_H__*/
