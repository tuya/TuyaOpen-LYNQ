/******************************************************************************

*(C) Copyright 2018 EIGENCOMM International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: atec_ref_cr_ftp.h
*
*  Description:
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef _ATEC_REF_CR_FTP_H
#define _ATEC_REF_CR_FTP_H

#include "at_util.h"

#define MFTP_MAX_RSP_LEN                        512
/* AT+MFTPCFG */
#define MFTPCFG_FTPID_MIN                   0
#define MFTPCFG_FTPID_MAX                   MFTP_AT_CLIENT_MAX_NUM-1
#define MFTPCFG_FTPID_DEF                   0
#define MFTPCFG_CONTEXTID_MIN                   1
#define MFTPCFG_CONTEXTID_MAX                   15
#define MFTPCFG_CONTEXTID_DEF                   1
#define MFTPCFG_SSLCTXID_MIN                    0
#define MFTPCFG_SSLCTXID_MAX                   5
#define MFTPCFG_SSLCTXID_DEF                    0
#define MFTPCFG_RSPTIMEOUT_MIN                    2
#define MFTPCFG_RSPTIMEOUT_MAX                   30
#define MFTPCFG_RSPTIMEOUT_DEF                   10
/* AT+MHTTPCONN */
#define MFTPCONN_ITEM_STR_MAX_LEN               255
/* AT+MFTPAPPE */
#define MFTPAPPE_FILENAME_MAX_LEN               255
#define MFTPAPPE_EOF_MIN                   0
#define MFTPAPPE_EOF_MAX                   1
#define MFTPAPPE_EOF_DEF                   0
#define MFTPAPPEDATA_LEN_MIN                   1
#define MFTPAPPEDATA_LEN_MAX                   4096
#define MFTPAPPEDATA_LEN_DEF                   0
/* AT+MFTPCMD */
#define MFTPCMD_PATH_MAX_LEN               255


enum FTP_CHANNEL_STATUS
{
    FTP_CONTENT_INPUT,
    FTP_APPE_INPUT,
    FTP_STOR_INPUT,
};

void mftpTimerExpired(void);
CmsRetId  mftpInputData(uint8_t chanId, uint8_t *pData, uint16_t dataLength);

CmsRetId mftpCFG(const AtCmdInputContext *pAtCmdReq);
CmsRetId  mftpCONN(const AtCmdInputContext *pAtCmdReq);
CmsRetId  mftpCWD(const AtCmdInputContext *pAtCmdReq);
CmsRetId  mftpPWD(const AtCmdInputContext *pAtCmdReq);
CmsRetId  mftpAPPE(const AtCmdInputContext *pAtCmdReq);
CmsRetId  mftpSTOR(const AtCmdInputContext *pAtCmdReq);
CmsRetId  mftpDISC(const AtCmdInputContext *pAtCmdReq);
CmsRetId  mftpMKD(const AtCmdInputContext *pAtCmdReq);
CmsRetId  mftpLIST(const AtCmdInputContext *pAtCmdReq);
CmsRetId  mftpDEL(const AtCmdInputContext *pAtCmdReq);
CmsRetId  mftpRN(const AtCmdInputContext *pAtCmdReq);
CmsRetId  mftpRETR(const AtCmdInputContext *pAtCmdReq);
CmsRetId  mftpSTATE(const AtCmdInputContext *pAtCmdReq);

#endif

