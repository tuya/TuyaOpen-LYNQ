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
#include <stdlib.h>
#include <string.h>
#include "cmsis_os2.h"
#include "at_util.h"
#include "atc_decoder.h"
#include "cmicomm.h"


/******************************************************************************
 ******************************************************************************
 * AT COMMAND PARAMETER ATTRIBUTE DEFINATION
 ******************************************************************************
******************************************************************************/
#define __AT_CMD_PARAM_ATTR_DEFINE__    //just for easy to find this position in SS


/* AT+TESTAT1 */
const static AtValueAttr  attrTESTAT1[] = {AT_PARAM_ATTR_DEF(AT_DEC_VAL, AT_MUST_VAL),
                                             AT_PARAM_ATTR_DEF(AT_STR_VAL, AT_OPT_VAL)};

#if 0
const static AtValueAttr attrlfconf[] = { AT_PARAM_ATTR_DEF(AT_STR_VAL, AT_MUST_VAL),
                                          AT_PARAM_ATTR_DEF(AT_STR_VAL, AT_MUST_VAL)};

const static AtValueAttr attrlfgga[] = { AT_PARAM_ATTR_DEF(AT_STR_VAL, AT_MUST_VAL)};
#endif


extern CmsRetId funcTESTAT1(const AtCmdInputContext *pAtCmdReq);
extern CmsRetId funcTESTAT2(const AtCmdInputContext *pAtCmdReq);

extern CmsRetId lfconf_exec(const AtCmdInputContext* pAtCmdReq);
extern CmsRetId lfinit_exec(const AtCmdInputContext* pAtCmdReq);
extern CmsRetId lfstar_exec(const AtCmdInputContext* pAtCmdReq);
extern CmsRetId lfgga_exec(const AtCmdInputContext* pAtCmdReq);
extern CmsRetId lfrtcm_exec(const AtCmdInputContext* pAtCmdReq);
extern CmsRetId lfstop_exec(const AtCmdInputContext* pAtCmdReq);
extern CmsRetId lffinal_exec(const AtCmdInputContext* pAtCmdReq);

/******************************************************************************
 ******************************************************************************
 * AT COMMAND TABLE
 ******************************************************************************
******************************************************************************/
#define __AT_CUST_CMD_TABLE_DEFINE__    //just for easy to find this position in SS

const AtCmdPreDefInfo  refAtCmdTable[] =
{
    AT_CMD_PRE_DEFINE("+TESTAT1",     funcTESTAT1,  attrTESTAT1,  AT_EXT_PARAM_CMD,   AT_DEFAULT_TIMEOUT_SEC),
//    AT_CMD_PRE_DEFINE("+TESTAT2",     funcTESTAT1,  NULL,         AT_EXT_PARAM_CMD,   AT_DEFAULT_TIMEOUT_SEC),

#if 0
    AT_CMD_PRE_DEFINE("+LFCONF",     lfconf_exec,     attrlfconf,    AT_EXT_PARAM_CMD,    AT_DEFAULT_TIMEOUT_SEC*2),
    AT_CMD_PRE_DEFINE("+LFINIT",     lfinit_exec,     NULL,          AT_EXT_PARAM_CMD,    AT_DEFAULT_TIMEOUT_SEC*2),
    AT_CMD_PRE_DEFINE("+LFSTART",    lfstar_exec,     NULL,          AT_EXT_PARAM_CMD,    AT_DEFAULT_TIMEOUT_SEC*2),
    AT_CMD_PRE_DEFINE("+LFGGA",      lfgga_exec,      attrlfgga,     AT_EXT_PARAM_CMD,    AT_DEFAULT_TIMEOUT_SEC*2),
    AT_CMD_PRE_DEFINE("+LFRTCM",     lfrtcm_exec,     NULL,          AT_EXT_PARAM_CMD,    AT_DEFAULT_TIMEOUT_SEC*2),
    AT_CMD_PRE_DEFINE("+LFSTOP",     lfstop_exec,     NULL,          AT_EXT_PARAM_CMD,    AT_DEFAULT_TIMEOUT_SEC*2),
    AT_CMD_PRE_DEFINE("+LFFINAL",    lffinal_exec,    NULL,          AT_EXT_PARAM_CMD,    AT_DEFAULT_TIMEOUT_SEC*2),
#endif
};

AtCmdPreDefInfoC* atcGetRefAtCmdSeqPointer(void)
{
   return  (AtCmdPreDefInfoC *)refAtCmdTable;
}

UINT32 atcGetRefAtCmdSeqNumb(void)
{
    return AT_NUM_OF_ARRAY(refAtCmdTable);
}


