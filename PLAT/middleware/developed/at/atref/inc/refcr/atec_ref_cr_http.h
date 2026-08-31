/******************************************************************************

*(C) Copyright 2018 EIGENCOMM International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: atec_ref_cr_http.h
*
*  Description:
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef _ATEC_REF_CR_HTTP_H
#define _ATEC_REF_CR_HTTP_H

#include "at_util.h"

#define MHTTP_MAX_RSP_LEN                        512
/* AT+MHTTPCFG */
#define MHTTPCFG_HTTPID_MIN                   0
#define MHTTPCFG_HTTPID_MAX                   HTTP_AT_CLIENT_MAX_NUM
#define MHTTPCFG_HTTPID_DEF                   0
#define MHTTPCFG_CONTEXTID_MIN                   1
#define MHTTPCFG_CONTEXTID_MAX                   8
#define MHTTPCFG_CONTEXTID_DEF                   1
#define MHTTPCFG_SSLCTXID_MIN                    0
#define MHTTPCFG_SSLCTXID_MAX                   5
#define MHTTPCFG_SSLCTXID_DEF                    0
#define MHTTPCFG_FRAGSIZE_MIN                    0
#define MHTTPCFG_FRAGSIZE_MAX                   1024
#define MHTTPCFG_FRAGSIZE_DEF                    0
#define MHTTPCFG_INTERV_MIN                    0
#define MHTTPCFG_INTERV_MAX                   2000
#define MHTTPCFG_INTERV_DEF                    0
#define MHTTPCFG_CACHELEN_MIN                    1
#define MHTTPCFG_CACHELEN_MAX                   8192
#define MHTTPCFG_CACHELEN_DEF                    4096
#define MHTTPCFG_HEADER_MAX_SIZE                 1024
/* AT+MHTTPCREATE */
#define MHTTPCREATE_HOST_STR_MAX_LEN               128
/* AT+MHTTPHEAD */
#define MHTTPHEADER_MAX_LEN                    1024
#define MHTTPHEADER_TOTAL_LEN                    4096
#define MHTTPHEADER_HTTPID_MIN                   0
#define MHTTPHEADER_HTTPID_MAX                   HTTP_AT_CLIENT_MAX_NUM-1
#define MHTTPHEADER_HTTPID_DEF                   0
#define MHTTPHEADER_EOF_MIN                   0
#define MHTTPHEADER_EOF_MAX                   2
#define MHTTPHEADER_EOF_DEF                   0
#define MHTTPHEADER_LEN_MIN                   0
#define MHTTPHEADER_LEN_MAX                   4096
#define MHTTPHEADER_LEN_DEF                   0
/* AT+MHTTPCONTENT */
#define MHTTPCONTENT_LEN_MIN                   0
#define MHTTPCONTENT_LEN_MAX                   4096
#define MHTTPCONTENT_LEN_DEF                   0

/* AT+MHTTPREQUEST */
#define MHTTPREQUEST_METHOD_MIN                   1
#define MHTTPREQUEST_METHOD_MAX                   5
#define MHTTPREQUEST_METHOD_DEF                   1
#define MHTTPREQUEST_PATHLEN_MAX                  MAXPATH_SIZE
/* AT+MHTTPREAD */
#define MHTTPREAD_TYPE_MIN                   0
#define MHTTPREAD_TYPE_MAX                   1
#define MHTTPREAD_TYPE_DEF                   0
/* AT+MHTTPDLFILE */
#define MHTTPDLFILE_URL_STR_MAX_LEN               388
#define MHTTPDLFILE_LOCAL_PATH_MAX_LEN            63
#define MHTTPDLFILE_RANGE_MAX_LEN            32

enum HTTP_CHANNEL_STATUS
{
    HTTP_HEADER_INPUT,
    HTTP_CONTENT_INPUT,
    HTTP_REQUEST_INPUT
};

void mhttpTimerExpired(void);
CmsRetId  mhttpInputData(uint8_t chanId, uint8_t *pData, uint16_t dataLength);

CmsRetId mhttpCFG(const AtCmdInputContext *pAtCmdReq);
CmsRetId  mhttpHEADER(const AtCmdInputContext *pAtCmdReq);
CmsRetId  mhttpCONTENT(const AtCmdInputContext *pAtCmdReq);
CmsRetId  mhttpCREATE(const AtCmdInputContext *pAtCmdReq);
CmsRetId  mhttpREQUEST(const AtCmdInputContext *pAtCmdReq);
CmsRetId  mhttpTERM(const AtCmdInputContext *pAtCmdReq);
CmsRetId  mhttpDEL(const AtCmdInputContext *pAtCmdReq);
CmsRetId  mhttpREAD(const AtCmdInputContext *pAtCmdReq);
CmsRetId  mhttpDLFILE(const AtCmdInputContext *pAtCmdReq);

#endif

