#include <stdlib.h>
#include <string.h>
#include "cmsis_os2.h"
#include "at_util.h"
#include "ctype.h"
#include "cmicomm.h"
#include "ol_log.h"


#define  TEST_VAL_MIN   0
#define  TEST_VAL_MAX   3
#define  TEST_VAL_DEF   1
#define  TEST_STR_LEN_MAX   32


INT32 test_int_val = 0;
UINT16 test_str_len = 0;
char test_str_val[10] = {0};

CmsRetId funcTESTAT1(const AtCmdInputContext *pAtCmdReq)
{
    CmsRetId    rc = CMS_FAIL;
    UINT32      reqHandle = AT_SET_SRC_HANDLER(pAtCmdReq->tid, CMS_DEFAULT_SUB_AT_ID, pAtCmdReq->chanId);
    UINT32      operaType = (UINT32)pAtCmdReq->operaType;
    char rspBuf[256] = {0};

    switch(operaType)
    {
        case AT_EXEC_REQ:             // AT+TESTAT1
        {
			test_int_val = 0;
			memset(test_str_val, 0, sizeof(test_str_val));
            rc = atcReply(reqHandle, AT_RC_OK, ATC_SUCC_CODE, (CHAR *)"+TESTAT1: ACTION");
            break;
        }
        case AT_TEST_REQ:             // AT+TESTAT1=?
        {
            rc = atcReply(reqHandle, AT_RC_OK, ATC_SUCC_CODE, (CHAR *)"+TESTAT1: (0-3), \"string\"");
            break;
        }
        case AT_READ_REQ:             // AT+TESTAT1?
        {
            sprintf(rspBuf, "+TESTAT1: %d, %s", test_int_val, test_str_val);

            rc = atcReply(reqHandle, AT_RC_OK, 0, rspBuf);
            break;
        }
        case AT_SET_REQ:              // AT+TESTAT1=
        {
            if (atGetNumValue(pAtCmdReq->pParamList, 0, &test_int_val, TEST_VAL_MIN, TEST_VAL_MAX, TEST_VAL_DEF) == AT_PARA_ERR)
            {
                rc = atcReply(reqHandle, AT_RC_ERROR, 1, NULL);
                break;
            }
            if(atGetStrValue(pAtCmdReq->pParamList, 1, (uint8_t*)test_str_val, TEST_STR_LEN_MAX,  &test_str_len, (char *)NULL) == AT_PARA_ERR)
            {
                rc = atcReply(reqHandle, AT_RC_ERROR, 1, NULL);
                break;
            }

            OL_LOG_INFO("intput: num=%d, strlen=%d, string=%s",test_int_val, test_str_len, test_str_val);

            rc = atcReply(reqHandle, AT_RC_OK, ATC_SUCC_CODE, NULL);
            break;
        }
        default:
        {
            rc = atcReply(reqHandle, AT_RC_ERROR, 2, NULL);
            break;
        }
    }

    return rc;
}
/*
CmsRetId funcTESTAT2(const AtCmdInputContext *pAtCmdReq)
{
    CmsRetId rc = CMS_FAIL;
    UINT32 reqHandle = AT_SET_SRC_HANDLER(pAtCmdReq->tid, CMS_DEFAULT_SUB_AT_ID, pAtCmdReq->chanId);
    UINT32 operaType = (UINT32)pAtCmdReq->operaType;

    switch(operaType)
    {
        case AT_EXEC_REQ:             // AT+TESTAT2
        {
            OL_LOG_INFO("test AT do nothing");

            rc = atcReply(reqHandle, AT_RC_OK, ATC_SUCC_CODE, NULL);
            break;
        }
        case AT_TEST_REQ:             // AT+TESTAT2=?
        {
            rc = atcReply(reqHandle, AT_RC_OK, ATC_SUCC_CODE, NULL);
            break;
        }
        default:
        {
            rc = atcReply(reqHandle, AT_RC_ERROR, 2, NULL);
            break;
        }
    }

    return rc;
}
*/
