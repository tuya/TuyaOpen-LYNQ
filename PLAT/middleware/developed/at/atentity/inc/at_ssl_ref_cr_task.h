/******************************************************************************

*(C) Copyright 2018 EIGENCOMM International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: at_ssl_task.h
*
*  Description: Process tls client related AT commands
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef _AT_SSL_REF_CR_TASK_H
#define _AT_SSL_REF_CR_TASK_H

#include "at_util.h"
#include "atc_decoder.h"
#include "at_sock_entity.h"

#include "mbedtls/net.h"
#include "mbedtls/ssl.h"
#include "mbedtls/certs.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"

#define MAX_MSSL_CLIENT_INSTANCE 6

typedef enum AT_MSSL_ERROR
{
    MSSLAT_OK = 0,                                           //success
    MSSLAT_CONFIG_ERROR = 2,
    MSSLAT_OPERATION_NOT_SUPPORT = 4,
    MSSLAT_MEM_NOT_ENOUGH = 8,
    MSSLAT_PARAM_ERROR = 50,
    MSSLAT_INTERNAL = 750,
    MSSLAT_MBEDTLS_ERROR = 3,                                //mbedtls error
    MSSLAT_CONN_ERROR = 4,                                   //connect error
    MSSLAT_HAS_CONNECT = 5,                                  //already has a connect
    MSSLAT_NO_CONNECT = 6,                                   //has no open connect
}AtMSslError_e;

enum MSSL_CMD_TYPE
{
    MSSL_OPEN_COMMAND,
    MSSL_SEND_COMMAND,
    MSSL_DISCON_COMMAND,
    MSSL_CLOSE_COMMAND,
    MSSL_CLOSE_TCP_COMMAND,
    MSSL_READBUFF_COMMAND,
    MSSL_BUFFINFO_COMMAND,
    MSSL_SETMODE_COMMAND,
    MSSL_GETSTATE_COMMAND,
    MSSL_CHECK_COMMAND,
    MSSL_CERTWR_COMMAND,
    MSSL_KEYWR_COMMAND,
    MSSL_CERTRD_COMMAND,
};

typedef enum {
    MSSLSTAT_INIT = 0,
    MSSLSTAT_CONNECT,
    MSSLSTAT_DISCONN,
    MSSLSTAT_CLOSE
}MSSLAtStatus_e;

typedef enum
{
    MSSL_EN_SLEEP,
    MSSL_DIS_SLEEP
}MSSLSleep_e;

enum AccessMode_e
{
    ACCESSMODE_DIRECT = 0,
    ACCESSMODE_PASSTH,
    ACCESSMODE_CACHE,
};

enum AbnoState_e
{
    DISCON_BY_PEER = 0,
    DISCON_ABNOR,
    DISCON_PDP_DEACT,
};

enum applMSslPrimId_Enum
{
    APPL_MSSL_PRIM_ID_BASE = 0,

    APPL_MSSL_CHECK_CNF,
    APPL_MSSL_CERTWR_CNF,
    APPL_MSSL_KEYWR_CNF,
    APPL_MSSL_CERTRD_CNF,

    APPL_MSSL_PRIM_ID_END = 0xFF
};

enum MSSLCHECK_VERIFY_TYPE
{
    MSSLCHECK_MD5,
    MSSLCHECK_SHA,
    MSSLCHECK_SHA256,
    MSSLCHECK_CRC,
};

typedef struct
{
    uint8_t cmd_type;
    uint32_t reqHandle;
    uint8_t clientId;
    uint16_t param;
    uint8_t abnorState;
    void* ptr;
}msslCmdMsg_t;

typedef struct sslContextTag
{
    mbedtls_ssl_context       sslContext;
    mbedtls_net_context       netContext;
    mbedtls_ssl_config        sslConfig;
    mbedtls_entropy_context   entropyContext;
    mbedtls_ctr_drbg_context  ctrDrbgContext;
    mbedtls_x509_crt_profile  crtProfile;
    mbedtls_x509_crt          caCert;
    mbedtls_x509_crt          clientCert;
    mbedtls_pk_context        pkContext;
}sslContext;

typedef struct sslDlBufferList_Tag{
    uint16_t totalLen;
    UINT16 reserved;
    UINT16 length;   /*the actual raw data length,*/
    UINT16 offSet;
    CmsSockMgrDataContext *dataContext;
    struct sslDlBufferList_Tag *next;

}sslDlBufferList;

typedef struct
{
    uint32_t reqHandle;
    uint8_t* url;
    uint16_t port;
    uint8_t clientId:4;
    uint8_t pdpId:4;
    uint8_t sslCfgId:4;
    uint8_t accessMode:2;
    uint8_t reserve1:2;
}AtRefCrSslOpenReq;

typedef struct
{
    uint32_t reqHandle;
    uint16_t dataLen;
    uint8_t* data;
    uint8_t clientId;
}AtRefCrSslSendReq;

typedef struct
{
    uint32_t reqHandle;
    uint16_t clientId;
}AtRefCrSslCloseReq;

typedef struct
{
    uint32_t reqHandle;
    uint16_t clientId;
    uint16_t readLen;
}AtRefCrSslReadBuffReq;

typedef struct
{
    uint16_t clientId;
    uint16_t unreadLen;
}AtRefCrSslBuffInfoReq;

typedef struct
{
    uint32_t reqHandle;
    uint16_t clientId;
    uint16_t accessMode;
}AtRefCrSslModeReq;

typedef struct msslClientPriContextTag
{
    UINT32                 dlTotalLen;      /* todo increase in function proc SOCK_EVENT_CONN_DL_DATA event*/
    UINT32                 dlUnreadLen;     /* unread length of DL data*/
    UINT32                 dlUnreadPacktCnt;/* unread packet of DL data*/
    EcSocDlBufferList      *pDlList;
    CHAR    remote[AT_SOC_MAX_REF_URL_IPADDR_LEN +1];
}msslClientPriContext;

typedef struct msslClientContextTag
{
    bool isUsed;
    uint8_t pdpId;
    uint8_t clientId;            /*0~5*/
    uint8_t sslCxtId;
    uint32_t reqhandle;
    int socket;
    char* serverAddr;
    uint16_t serverPort;
    uint8_t status;
    int timeout_s;
    int timeout_r;
    sslContext * ssl;
    char *caCert;
    char *clientCert;
    char *clientPk;
    int32_t caCertLen;
    int32_t clientCertLen;
    int32_t clientPkLen;
    uint8_t seclevel;//0:no verify; 1:verify server; 2:both verify
    int32_t ciphersuite[2];//just like 0x0035 TLS_RSA_WITH_AES_256_CBC_SHA,ciphersuite[1] must NULL
    uint8_t cache;//0:no session resumption; 1:session resumption
    uint8_t accessMode;//0:direct mode; 1:passthrough; 2:cache;
    uint8_t ignoreCertStamp;//0:not ignore; 1:ignore
    uint8_t ignoreCertVerify;//0:not ignore; 1:ignore
    UINT8  priContext[]; //the private context
}msslClientContext;

typedef struct
{
    uint8_t cmd_type;
    uint32_t reqHandle;
    void* filename;
    void* filedata;
    uint16_t filedatalen;
    uint8_t checkType;
}msslFileCmdMsg_t;

typedef struct
{
    char *retStr;
    int  ret;
    uint32_t length;
}msslFileCnfMsg;

void msslSaveCfgList(void);
void msslEngineInit(void);
AtMSslError_e msslCheckRestore();

uint32_t msslSendAsyncRequest(uint8_t reqType, void* req);
uint32_t msslSendRequest(uint8_t reqType, void* req, uint8_t connId);
CmsRetId msslPSTHDataInd(UINT16 reqHandle, void *paras);

CmsRetId msslFileCheck(uint32_t atHandle, char* crtFile, uint8_t type);
CmsRetId msslFileCertWr(uint8_t cmdtype, uint32_t atHandle, char* crtFile, uint8_t* data, uint16_t datalen);
CmsRetId msslFileRead(uint32_t atHandle, char* crtFile);

#endif


