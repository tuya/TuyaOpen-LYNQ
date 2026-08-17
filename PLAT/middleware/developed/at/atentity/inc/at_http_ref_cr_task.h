/******************************************************************************

*(C) Copyright 2018 EIGENCOMM International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: at_http_ref_cr_task.h
*
*  Description: Process http(s) client related AT commands
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef _AT_HTTP_REF_CR_TASK_H_
#define _AT_HTTP_REF_CR_TASK_H_

#include "at_util.h"
#include "HTTPClient.h"
#ifdef FEATURE_FOTAPAR_ENABLE
#include "ec_xota_api.h"
#endif

#define HTTP_RSP_HEAD_BUFFER_SIZE 800
#define HTTP_RSP_CONTENT_BUFFER_SIZE  CHUNK_SIZE

#define HTTP_AT_CLIENT_MAX_NUM   4
#define HTTP_AT_DLFILE_CLIENT_ID 4

#define MHTTP_HAEDER_MAX_SIZE 1024
/*
 * APPL SGID: APPL_HTTP, related PRIM ID
*/
enum applMHttpPrimId_Enum
{
    APPL_MHTTP_PRIM_ID_BASE = 0,

    APPL_MHTTP_CREATE_CNF,
    APPL_MHTTP_REQUEST_CNF,
    APPL_MHTTP_TERM_CNF,
    APPL_MHTTP_DEL_CNF,
    APPL_MHTTP_READ_CNF,
    APPL_MHTTP_DLFILE_CNF,
    APPL_HTTP_URC_IND,
    APPL_HTTP_RECV_IND,

    APPL_HTTP_PRIM_ID_END = 0xFF
};

typedef enum {
    MHTTPSTAT_DEFAULT = 0,
    MHTTPSTAT_CREATED,
    MHTTPSTAT_CONNECTED,
    MHTTPSTAT_REQHANDL,
    MHTTPSTAT_TERM,
}MHTTPAtStatus_e;

typedef enum AT_MHTTP_ERROR
{
    MHTTPAT_OK = 0,                                           //success
    MHTTPAT_OPERATION_NOT_SUPPORT = 3,                        //operation not support
    MHTTPAT_MEMORY_NOT_ENOUGH = 23,                           //memory not enough
    MHTTPAT_PARAM_ERROR = 50,                                 //parameter error
    MHTTPAT_INTERNAL = 650,                                   //unknown error
    MHTTPAT_NO_FREE_CLIENT = 651,                             //no free client
    MHTTPAT_NO_CREAT_CLIENT = 652,                            //no create client
    MHTTPAT_CLIENT_BUSY = 653,                                //client busy
    MHTTPAT_URL_PARSE_ERROR = 654,                            //url parse error
    MHTTPAT_NOT_SUPPORT_TLS = 655,                            //tls is not supported
    MHTTPAT_CONNECT_ERROR = 656,                              //connect error
}atMHttpError_e;

typedef enum AT_MHTTP_URC_ERROR
{
    MHTTPURC_DNS_FAILED = 1,
    MHTTPURC_CONNECT_FAILED = 2,
    MHTTPURC_CONNECT_TIMEOUT = 3,
    MHTTPURC_TLS_HS_FAILED = 4,
    MHTTPURC_CONNECT_ABORT = 5,
    MHTTPURC_RESPONSE_TIMEOUT = 6,
    MHTTPURC_DATA_PARSE_FAILED = 7,
    MHTTPURC_MEMORY_NOT_ENOUGH = 8,
    MHTTPURC_DATA_LOSS = 9,
    MHTTPURC_WRITE_FILE_FAILED = 10,
    MHTTPURC_UNKNOWN = 255,
}atMHttpUrcError_e;

typedef enum
{
    HTTPFOTA_BEGIN = 0,                                    //FOTA begin http download
    HTTPFOTA_DL_PROC = 1,                                  //FOTA download progress
    HTTPFOTA_ERR = 2,                                      //FOTA http error
    HTTPFOTA_DOWNLOADEND = 3,                              //FOTA http download end
    HTTPFOTA_PACKAGE_MISMATCH = 4                          //FOTA http package mismatch
}HTTPFotaUrc_e;

typedef enum
{
    MHTTP_EN_SLEEP,
    MHTTP_DIS_SLEEP
}MHTTPSleep_e;


enum MHTTP_ATCMD_TYPE
{
    MHTTP_CREATE_COMMAND,
    MHTTP_REQUEST_COMMAND,
    MHTTP_CHUNKED_REQUEST_COMMAND,
    MHTTP_CLOSE_TCP_COMMAND,
    MHTTP_DEL_COMMAND,
    MHTTP_TERM_COMMAND,
    MHTTP_DLFILE_COMMAND,
    MHTTP_FWDL_COMMAND
};

enum MHTTP_URC_TYPE
{
    MHTTP_URC_ERR,
    MHTTP_URC_IND,
    MHTTP_URC_RECV,
    MHTTP_URC_FILEDOWN,
};

typedef struct{
    char* respfilename;
    void* fp;
    uint32_t startPos;
    uint32_t endPos;
    uint16_t progind;
} httpDlfileParam_t;

typedef struct {
    uint8_t isCreated:1;
    uint8_t isFota:1;
    uint8_t isReceiving:1;// only support one receive ongoing for a connection
    uint8_t clientId:2;
    uint8_t cmdStat:3;
    uint8_t netStat:1;
    uint8_t eof:1;
    uint8_t isRange:1;
    uint8_t resve:5;
    CHAR* host;
    CHAR* url;
    HttpClientContext* clientContext;
    HttpClientData* clientData;
    UINT32 reqhandle;
} httpAtClient_t;

typedef struct
{
    uint32_t cmd_type;
    char* host;
    char* path;
    void* clientCxt;
    uint32_t reqhandle;
    char* url;
    char* local_path;
    uint8_t sslid;
    uint8_t timeout;
} httpAtCmdMsg_t;

typedef struct _mhttpHeadNode_t
{
    struct _mhttpHeadNode_t * next;
    char   key[64];
    char*  value;
    uint16_t keyLen;
    uint16_t valueLen;
}mhttpHeadNode_t;

typedef struct _mhttpAtCfgParam{
    uint8_t mhttpId:2;//0~3
    uint8_t chunked:1;//0~1
    uint8_t cached:1;//0~1
    uint8_t inputFormat:2;//0~2
    uint8_t outputFormat:1;//0~1
    uint8_t sslEn:1;//0~1                    1 byte
    uint8_t connTimeout;//0~180
    uint8_t rspTimeout;//0~80
    uint8_t inputTimeout;//0~180             3 byte
    uint32_t sslId:3;//0~5
    uint32_t cId:4;//1~15
    uint32_t cacheLen:14;//1~8192
    uint32_t fragSize:11;//0~1024            4 byte
    uint32_t urlenc:1;//0~1
    uint32_t interv:11;//0~2000ms
    uint32_t isCreated:1;
    uint32_t contentEof:1;
    uint32_t uniheadEof:1;
    uint32_t method:3;
    uint32_t reserve:14;//                   4 byte
    char* host;//                            4 byte
    uint8_t* header;//                       4 byte
    uint8_t* uniHeader;//                    4 byte
    uint8_t* content;//                      4 byte
    uint8_t* cachedHeadBuf;//                4 byte
    uint8_t* cachedBodyBuf;//                4 byte
    mhttpHeadNode_t *    cfgHeadList;
    mhttpHeadNode_t *    uniHeadList;
    uint16_t cachedHeadLen;
    uint16_t cachedBodyLen;
    uint16_t cachedHeadunReadLen;
    uint16_t cachedBodyunReadLen;
    uint16_t headerLen;
    uint16_t uniHeaderLen;//                 4 byte
    uint16_t contentLen;
} mhttpAtCfgParam_t;

typedef struct
{
    uint8_t cmd_type;
} mhttpCmdMsg_t;

typedef struct
{
    uint16_t ret;
    uint16_t clientId;
    uint8_t  *data;
} mhttpCnfCmdMsg_t;

typedef struct
{
    void *pHttpInd;
} mhttpIndMsg_t;

typedef int32_t (*mhttp_fwdl_callback)(uint8_t *buf, uint8_t state, uint32_t totalSize, uint32_t dloadSize);

mhttpAtCfgParam_t * mhttpGetCfgParam(uint8_t clientId);

bool mhttpCheckClientBusy(void);
CmsRetId mhttpShowRead(uint8_t httpid, uint32_t reqhandle, uint8_t type, uint16_t readLen);

CmsRetId mhttpCreateReq(UINT32 atHandle, uint8_t* host);
bool mhttpRequestReq(UINT32 atHandle, uint8_t clientId, uint8_t* path);
bool mhttpCheckRequest(uint8_t clientId);
CmsRetId mhttpChunkedRequestReq(UINT32 atHandle, uint8_t clientId);
CmsRetId mhttpTermReq(UINT32 atHandle, uint8_t clientId);
CmsRetId mhttpDeleteReq(UINT32 atHandle, uint8_t clientId);
CmsRetId mhttpDlfileReq(UINT32 atHandle, char* url, char* local_path, uint16_t progind, char* range, int eof, uint8_t sslid);
int mhttpFwdlReq(uint32_t reqhandle, AtXotaDownloadReq_t* downloadReq, mhttp_fwdl_callback fw_dl_cb);
mhttpHeadNode_t * mhttpHeadListAdd(mhttpHeadNode_t * head, mhttpHeadNode_t * node);
bool mhttpAssemHeader(mhttpHeadNode_t* nodeList, char* newHeader, uint16_t* newHeaderLen);

#endif

