#ifndef __ATEC_ATCFG_H__
#define __ATEC_ATCFG_H__
#include "at_util.h"

#if defined(FEATURE_SUBSYS_AI_ENABLE) && defined(FEATURE_SUBSYS_AI_AT_ENABLE)
/******************************************************************************
 *****************************************************************************
 * AT MIFI COMMON MARCO
 *****************************************************************************
******************************************************************************/
#define EC_ECAICFG_CNF_STR_LEN         (512)
#define ATC_ECAICFG_MAX_PARM_STR_LEN   (32)
#define ATC_ECAICFG_MAX_VALUE_STR_LEN  (128)
#define ATC_ECAICFG_MAX_PARM_STR_DEFAULT         NULL
#define ATC_ECAICFG_MAX_VALUE_STR_DEFAULT         NULL

/******************************************************************************
 *****************************************************************************
 * API
 *****************************************************************************
******************************************************************************/

CmsRetId ecAICFG(const AtCmdInputContext *pAtCmdReq);

#endif

#endif/*__ATEC_ATCFG_H__*/

