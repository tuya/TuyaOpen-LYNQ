/******************************************************************************

*(C) Copyright 2018 EIGENCOMM International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: atec_http.h
*
*  Description:
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef _ATEC_REF_CR_SSL_H
#define _ATEC_REF_CR_SSL_H

#include "at_util.h"
#include "ec_sslcmd_api.h"

#define MSSL_MAX_RSP_LEN                        512
/* AT+MSSLCFG */
#define MSSLCFG_ID_MIN                          0
#define MSSLCFG_ID_MAX                          5
#define MSSLCFG_ID_DEF                          0
#define MSSLCFG_CERTNAME_MAX_LEN                MSSL_CERTNAME_MAX_LEN
#define MSSLCFG_NEGOTIME_MIN                    10
#define MSSLCFG_NEGOTIME_MAX                    300
#define MSSLCFG_NEGOTIME_DEF                    300
#define MSSLCFG_CIPHSUIT_MAX_SIZE                7

/* AT+MSSLCHECK */
#define MSSLCHECK_VERIFY_MIN                          0
#define MSSLCHECK_VERIFY_MAX                          3
#define MSSLCHECK_VERIFY_DEF                          0

/* AT+MSSLCERTWR */
#define MSSLCERTWR_MAX_LEN                    8192

/* AT+MSSLLIST */
#define MSSLLIST_MODE_MIN                          1
#define MSSLLIST_MODE_MAX                          3
#define MSSLLIST_MODE_DEF                          0
#define MSSLLIST_MODE_1                            1
#define MSSLLIST_MODE_2                            2
#define MSSLLIST_MODE_3                            3

enum MSSL_CHANNEL_STATUS
{
    MSSL_CERT_INPUT,
    MSSL_KEY_INPUT,
};

CmsRetId  msslInputData(uint8_t chanId, uint8_t *pData, uint16_t dataLength);

CmsRetId msslCFG(const AtCmdInputContext *pAtCmdReq);
CmsRetId msslCHECK(const AtCmdInputContext *pAtCmdReq);
CmsRetId msslCIPHER(const AtCmdInputContext *pAtCmdReq);
CmsRetId  msslCERTWR(const AtCmdInputContext *pAtCmdReq);
CmsRetId  msslKEYWR(const AtCmdInputContext *pAtCmdReq);
CmsRetId msslCERTRD(const AtCmdInputContext *pAtCmdReq);
CmsRetId msslLIST(const AtCmdInputContext *pAtCmdReq);
CmsRetId msslRM(const AtCmdInputContext *pAtCmdReq);

#endif

