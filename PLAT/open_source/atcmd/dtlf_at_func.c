#include <stdlib.h>
#include <string.h>
#include "cmsis_os2.h"
#include "at_util.h"
#include "ctype.h"
#include "cmicomm.h"

#define AT_DTLF_CONF_RESP_STR \
"+LFCONF: \"keyType\":%d \r\n\
+LFCONF: \"appKey\":%s \r\n\
+LFCONF: \"appSecret\":%s \r\n\
+LFCONF: \"deviceID\":%s \r\n\
+LFCONF: \"deviceType\":%s \r\n\
+LFCONF: \"mountPoint\":%s \r\n"

#define ATC_DTLF_CONF_RESP_VAL(rcfg) \
(rcfg)->keytype,\
(rcfg)->appkey,\
(rcfg)->appsecret,\
(rcfg)->deviceid,\
(rcfg)->devicetype,\
(rcfg)->mountpoint

typedef struct
{
    int keytype;
    char appkey[16];
    char appsecret[70];
    char deviceid[16];
    char devicetype[16];
    char mountpoint[16];
}dtlf_conf;

dtlf_conf cfg;
char gga_data[128];

CmsRetId lfconf_exec(const AtCmdInputContext* pAtCmdReq)
{
    CmsRetId rc = CMS_FAIL;
    UINT32 reqHandle = AT_SET_SRC_HANDLER(pAtCmdReq->tid, CMS_DEFAULT_SUB_AT_ID, pAtCmdReq->chanId);
    UINT32 operaType = (UINT32)pAtCmdReq->operaType;
    CHAR rspbuf[256] = {0};

    UINT8 param1[32] = {0};
    UINT8 param2[128] = {0};
    UINT16 param1_len = 0;
    UINT16 param2_len = 0;

    int keytype = 0;

    switch(operaType)
    {
        case AT_SET_REQ:
        {
            if(atGetStrValue(pAtCmdReq->pParamList, 0, param1, 32, &param1_len, (char*)NULL) == AT_PARA_ERR)
            {
                rc = atcReply(reqHandle, AT_RC_CME_ERROR, CME_INCORRECT_PARAM, NULL);
                break;
            }
            if(atGetStrValue(pAtCmdReq->pParamList, 1, param2, 128, &param2_len, (char*)NULL) == AT_PARA_ERR)
            {
                rc = atcReply(reqHandle, AT_RC_CME_ERROR, CME_INCORRECT_PARAM, NULL);
                break;
            }
            if(strncasecmp((const CHAR*)param1, "keyType", strlen("keyType")) == 0)
            {
                keytype = atoi((char *)param2);
                if((keytype == 1) || (keytype == 2))
                {
                    cfg.keytype = keytype;
                }
                else
                {
                    rc = atcReply(reqHandle, AT_RC_CME_ERROR, CME_INCORRECT_PARAM, NULL);
                    break;
                }
            }
            else if(strncasecmp((const CHAR*)param1, "appKey", strlen("appKey")) == 0)
            {
                memcpy(cfg.appkey,(char*)param2,param2_len);
            }
            else if(strncasecmp((const CHAR*)param1, "appSecret", strlen("appSecret")) == 0)
            {
                memcpy(cfg.appsecret,(char*)param2,param2_len);
            }
            else if(strncasecmp((const CHAR*)param1, "deviceID", strlen("deviceID")) == 0)
            {
                memcpy(cfg.deviceid,(char*)param2,param2_len);
            }
            else if(strncasecmp((const CHAR*)param1, "deviceType", strlen("deviceType")) == 0)
            {
                memcpy(cfg.devicetype,(char*)param2,param2_len);
            }
            else if(strncasecmp((const CHAR*)param1, "mountPoint", strlen("mountPoint")) == 0)
            {
                memcpy(cfg.mountpoint,(char*)param2,param2_len);
            }
            else
            {
                rc = atcReply(reqHandle, AT_RC_CME_ERROR, CME_INCORRECT_PARAM, NULL);
                break;
            }
            rc = atcReply(reqHandle, AT_RC_OK, ATC_SUCC_CODE, NULL);
            break;
        }
        case AT_READ_REQ:
        {
            snprintf((char*)rspbuf, 256, AT_DTLF_CONF_RESP_STR,
                         ATC_DTLF_CONF_RESP_VAL(&cfg));
            rc = atcReply(reqHandle, AT_RC_OK, ATC_SUCC_CODE, (CHAR*)rspbuf);
            break;
        }
        default:
        {
            rc = atcReply(reqHandle, AT_RC_ERROR, CME_OPERATION_NOT_SUPPORT, NULL);
            break;
        }
    }

    return rc;
}

CmsRetId lfinit_exec(const AtCmdInputContext* pAtCmdReq)
{
    CmsRetId rc = CMS_FAIL;
    UINT32 reqHandle = AT_SET_SRC_HANDLER(pAtCmdReq->tid, CMS_DEFAULT_SUB_AT_ID, pAtCmdReq->chanId);
    UINT32 operaType = (UINT32)pAtCmdReq->operaType;

    switch(operaType)
    {
        case AT_SET_REQ:
        {
            rc = atcReply(reqHandle, AT_RC_OK, ATC_SUCC_CODE, NULL);
            break;
        }
        case AT_EXEC_REQ:
        {
            rc = atcReply(reqHandle, AT_RC_OK, ATC_SUCC_CODE, NULL);
            break;
        }
        case AT_TEST_REQ:
        {
            rc = atcReply(reqHandle, AT_RC_OK, ATC_SUCC_CODE, NULL);
            break;
        }
        case AT_READ_REQ:
        {
            rc = atcReply(reqHandle, AT_RC_OK, ATC_SUCC_CODE, NULL);
            break;
        }
        default:
        {
            rc = atcReply(reqHandle, AT_RC_ERROR, CME_OPERATION_NOT_SUPPORT, NULL);
            break;
        }
    }

    return rc;
}

CmsRetId lfstar_exec(const AtCmdInputContext* pAtCmdReq)
{
    CmsRetId rc = CMS_FAIL;
    UINT32 reqHandle = AT_SET_SRC_HANDLER(pAtCmdReq->tid, CMS_DEFAULT_SUB_AT_ID, pAtCmdReq->chanId);
    UINT32 operaType = (UINT32)pAtCmdReq->operaType;

    switch(operaType)
    {
        case AT_SET_REQ:
        {
            rc = atcReply(reqHandle, AT_RC_OK, ATC_SUCC_CODE, NULL);
            break;
        }
        case AT_EXEC_REQ:
        {
            rc = atcReply(reqHandle, AT_RC_OK, ATC_SUCC_CODE, NULL);
            break;
        }
        case AT_TEST_REQ:
        {
            rc = atcReply(reqHandle, AT_RC_OK, ATC_SUCC_CODE, NULL);
            break;
        }
        case AT_READ_REQ:
        {
            rc = atcReply(reqHandle, AT_RC_OK, ATC_SUCC_CODE, NULL);
            break;
        }
        default:
        {
            rc = atcReply(reqHandle, AT_RC_ERROR, CME_OPERATION_NOT_SUPPORT, NULL);
            break;
        }
    }

    return rc;
}

CmsRetId lfgga_exec(const AtCmdInputContext* pAtCmdReq)
{
    CmsRetId rc = CMS_FAIL;
    UINT32 reqHandle = AT_SET_SRC_HANDLER(pAtCmdReq->tid, CMS_DEFAULT_SUB_AT_ID, pAtCmdReq->chanId);
    UINT32 operaType = (UINT32)pAtCmdReq->operaType;

    UINT8 param1[128] = {0};
    UINT16 param1_len = 0;

    switch(operaType)
    {
        case AT_SET_REQ:
        {
            if(atGetStrValue(pAtCmdReq->pParamList, 0, param1, 128, &param1_len, (char*)NULL) == AT_PARA_ERR)
            {
                rc = atcReply(reqHandle, AT_RC_CME_ERROR, CME_INCORRECT_PARAM, NULL);
                break;
            }
            memset(gga_data,0,128);
            memcpy(gga_data,(char*)param1,param1_len);
            rc = atcReply(reqHandle, AT_RC_OK, ATC_SUCC_CODE, NULL);
            break;
        }
        case AT_EXEC_REQ:
        {
            rc = atcReply(reqHandle, AT_RC_OK, ATC_SUCC_CODE, NULL);
            break;
        }
        case AT_TEST_REQ:
        {
            rc = atcReply(reqHandle, AT_RC_OK, ATC_SUCC_CODE, NULL);
            break;
        }
        case AT_READ_REQ:
        {
            rc = atcReply(reqHandle, AT_RC_OK, ATC_SUCC_CODE, (CHAR*)gga_data);
            break;
        }
        default:
        {
            rc = atcReply(reqHandle, AT_RC_ERROR, CME_OPERATION_NOT_SUPPORT, NULL);
            break;
        }
    }

    return rc;
}

CmsRetId lfrtcm_exec(const AtCmdInputContext* pAtCmdReq)
{
    CmsRetId rc = CMS_FAIL;
    UINT32 reqHandle = AT_SET_SRC_HANDLER(pAtCmdReq->tid, CMS_DEFAULT_SUB_AT_ID, pAtCmdReq->chanId);
    UINT32 operaType = (UINT32)pAtCmdReq->operaType;

    switch(operaType)
    {
        case AT_SET_REQ:
        {
            rc = atcReply(reqHandle, AT_RC_OK, ATC_SUCC_CODE, NULL);
            break;
        }
        case AT_EXEC_REQ:
        {
            rc = atcReply(reqHandle, AT_RC_OK, ATC_SUCC_CODE, NULL);
            break;
        }
        case AT_TEST_REQ:
        {
            rc = atcReply(reqHandle, AT_RC_OK, ATC_SUCC_CODE, NULL);
            break;
        }
        case AT_READ_REQ:
        {
            rc = atcReply(reqHandle, AT_RC_OK, ATC_SUCC_CODE, NULL);
            break;
        }
        default:
        {
            rc = atcReply(reqHandle, AT_RC_ERROR, CME_OPERATION_NOT_SUPPORT, NULL);
            break;
        }
    }

    return rc;
}

CmsRetId lfstop_exec(const AtCmdInputContext* pAtCmdReq)
{
    CmsRetId rc = CMS_FAIL;
    UINT32 reqHandle = AT_SET_SRC_HANDLER(pAtCmdReq->tid, CMS_DEFAULT_SUB_AT_ID, pAtCmdReq->chanId);
    UINT32 operaType = (UINT32)pAtCmdReq->operaType;

    switch(operaType)
    {
        case AT_SET_REQ:
        {
            rc = atcReply(reqHandle, AT_RC_OK, ATC_SUCC_CODE, NULL);
            break;
        }
        case AT_EXEC_REQ:
        {
            rc = atcReply(reqHandle, AT_RC_OK, ATC_SUCC_CODE, NULL);
            break;
        }
        case AT_TEST_REQ:
        {
            rc = atcReply(reqHandle, AT_RC_OK, ATC_SUCC_CODE, NULL);
            break;
        }
        case AT_READ_REQ:
        {
            rc = atcReply(reqHandle, AT_RC_OK, ATC_SUCC_CODE, NULL);
            break;
        }
        default:
        {
            rc = atcReply(reqHandle, AT_RC_ERROR, CME_OPERATION_NOT_SUPPORT, NULL);
            break;
        }
    }

    return rc;
}

CmsRetId lffinal_exec(const AtCmdInputContext* pAtCmdReq)
{
    CmsRetId rc = CMS_FAIL;
    UINT32 reqHandle = AT_SET_SRC_HANDLER(pAtCmdReq->tid, CMS_DEFAULT_SUB_AT_ID, pAtCmdReq->chanId);
    UINT32 operaType = (UINT32)pAtCmdReq->operaType;

    switch(operaType)
    {
        case AT_SET_REQ:
        {
            rc = atcReply(reqHandle, AT_RC_OK, ATC_SUCC_CODE, NULL);
            break;
        }
        case AT_EXEC_REQ:
        {
            rc = atcReply(reqHandle, AT_RC_OK, ATC_SUCC_CODE, NULL);
            break;
        }
        case AT_TEST_REQ:
        {
            rc = atcReply(reqHandle, AT_RC_OK, ATC_SUCC_CODE, NULL);
            break;
        }
        case AT_READ_REQ:
        {
            rc = atcReply(reqHandle, AT_RC_OK, ATC_SUCC_CODE, NULL);
            break;
        }
        default:
        {
            rc = atcReply(reqHandle, AT_RC_ERROR, CME_OPERATION_NOT_SUPPORT, NULL);
            break;
        }
    }

    return rc;
}

