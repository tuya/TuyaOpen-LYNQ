/******************************************************************************

*(C) Copyright 2018 EIGENCOMM International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: at_ftp_ref_cr_task.h
*
*  Description: Process ftp(s) client related AT commands
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef _AT_FTP_REF_CR_TASK_H_
#define _AT_FTP_REF_CR_TASK_H_

#include "at_util.h"
#include "ec_ftpclient_api.h"

#define MFTP_AT_CLIENT_MAX_NUM   6

/*
 * APPL SGID: APPL_MFTP, related PRIM ID
*/
enum applMFtpPrimId_Enum
{
    APPL_MFTP_PRIM_ID_BASE = 0,

    APPL_MFTP_URC_IND,
    APPL_MFTP_LIST_RECV_IND,
    APPL_MFTP_RETR_RECV_IND,

    APPL_MFTP_PRIM_ID_END = 0xFF
};

typedef enum
{
    MFTP_EN_SLEEP,
    MFTP_DIS_SLEEP,
    MFTP_EN_SLEEP_ONE,
    MFTP_DIS_SLEEP_ONE,
}MFTPSleep_e;

enum MFTP_ATCMD_TYPE
{
    MFTP_CONN_COMMAND,
    MFTP_CWD_COMMAND,
    MFTP_PWD_COMMAND,
    MFTP_MKD_COMMAND,
    MFTP_LIST_COMMAND,
    MFTP_TERM_COMMAND,
    MFTP_DEL_COMMAND,
    MFTP_RENAME_COMMAND,
    MFTP_RETR_COMMAND,
    MFTP_APPE_COMMAND,
    MFTP_STOR_COMMAND,
};

enum MFTP_UPLOAD_MODE
{
    MFTP_UPLOAD_APPE,
    MFTP_UPLOAD_STOR,
};

typedef struct
{
    uint32_t reqHandle;
    uint8_t connId;
    uint8_t* host;
    uint8_t* username;
    uint8_t* passwd;
    uint16_t port;
}AtRefCrFtpConnReq;

typedef struct
{
    uint32_t cmd_type;
    uint32_t reqhandle;
    void* clientCxt;
    char* url;
    char* path;
} mftpAtCmdMsg_t;

typedef struct
{
    uint32_t reply_code;
    void* clientCxt;
} mftpRspMsg_t;

typedef struct _mftpAtCfgParam{
    uint32_t mftpId:3;//0~5
    uint32_t transType:1;//0~1 0:BINARY; 1:ASCII
    uint32_t transMode:1;//0~1(1)
    uint32_t timeout:5;//2~30(10)
    uint32_t sslId:3;//0~5
    uint32_t sslMode:1;//0~1(only support 0)
    uint32_t sslEn:1;//0~1
    uint32_t fsTransType:1;//0~1
    uint32_t cId:4;//1~15(0)
    uint32_t eof:2;//0~2 0:init; 1:begin appe; 2:last appe
    uint32_t hasCfg:1;//0~1
} mftpAtCfgParam_t;

void mftpSleepVote(MFTPSleep_e sleep);

mftpAtCfgParam_t * mftpGetCfgParam(uint8_t clientId);
void mftpClearCfgParam(uint8_t clientId);

uint32_t mftpATcmdReq(uint8_t reqType, void* req);
int32_t mftpConnReq(ftpContext_t* pContext, char* url);
int32_t mftpDiscReq(UINT32 atHandle, uint8_t connectId);
int32_t mftpCwdReq(UINT32 atHandle, uint8_t connectId, uint8_t* path);
int32_t mftpPwdReq(UINT32 atHandle, uint8_t connectId);
int32_t mftpMkdReq(UINT32 atHandle, uint8_t connectId, uint8_t* dirname);
int32_t mftpListReq(UINT32 atHandle, uint8_t connectId, uint8_t* dirname);
int32_t mftpDelReq(UINT32 atHandle, uint8_t connectId, uint8_t* dirname);
int32_t mftpRnReq(UINT32 atHandle, uint8_t connectId, uint8_t* oldname, uint8_t* newname);
int32_t mftpRetrFileReq(UINT32 atHandle, uint8_t connectId, uint8_t* filename, uint8_t* localfilename, int32_t offset, int32_t getlen);
int32_t mftpUploadFile(uint32_t atHandle, uint8_t connectId, char* filename, char* localFile, uint8_t* data, uint16_t dataLen, uint8_t uploadMode);
uint8_t mftpStateReq(uint8_t connectId);
void mftpErrRetInd(uint16_t reqhandle, ftpErr_e errCode, uint8_t connId, int ftpCmd);

#endif

