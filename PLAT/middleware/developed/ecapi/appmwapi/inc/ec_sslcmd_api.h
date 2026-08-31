/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    ec_sslcmd_api.h
 * Description:  API interface implementation header file for ChinaNet lwm2m
 * History:      Rev1.0   2021-04-22
 *
 ****************************************************************************/
#ifndef __EC_SSLCMD_API_H__
#define __EC_SSLCMD_API_H__

#include "cms_util.h"

#define SSL_CERTPATH_MAX_SIZE  (48)
#define SSL_URL_MAX_LEN        (255)
#define SSL_SESSION_MAX_LEN    (800)
#define SSL_PSKID_MAX_SIZE     (65)

#define MSSL_CERTNAME_MAX_LEN  (64)

typedef enum
{
    SSL3_0 = 0,
    TLS1_0,
    TLS1_1,
    TLS1_2,
    SSL_VERSION_ALL,
}sslVersion_e;

typedef enum 
{
    SSL_ERR_SUCCESS = 0,
    SSL_ERR_NOSUCHID,
    SSL_ERR_PARAM,
    SSL_ERR_PARAM_ODD, 
    SSL_ERR_OPERATION_NOT_SUPPORT,
    SSL_ERR_SSLID_MAX,
    SSL_ERR_NO_CONNECT
}sslATError_e;

typedef enum 
{
    SSL_NO_VERIFY = 0,
    SSL_VERIFY_SEV,
    SSL_VERIFY_SEV_CLI, 
}sslSecLevel_e;

typedef enum 
{
    SSL_NO_CACHE = 0,
    SSL_CACHE,
}sslSessionCache_e;

#ifdef FEATURE_REF_AT_CR_ENABLE
enum
{
    SSL_INPUT_FORMATE_ASCII = 0,
    SSL_INPUT_FORMATE_HEX,
    SSL_INPUT_FORMATE_TRANS,
};
#endif

typedef struct _sslAtTemplate
{
    struct _sslAtTemplate *next;
    uint8_t id;
}sslAtTemplate_t;

typedef struct _sslPTUlDataNode
{
    struct _sslPTUlDataNode *next;
    uint16_t    dataLen;    /*the send raw datalength*/
    uint8_t*    pData;
}sslPTUlDataNode;

typedef struct _ssl_list_t
{
    struct _ssl_list_t * next;
} ssl_list_t;

typedef struct sslPTUlPendingList_Tag
{
   ssl_list_t *head;
   ssl_list_t *tail;
   uint8_t nodeCount;
   uint8_t maxNodeNum;
   osMutexId_t mut;
}sslPTUlPendingList;

typedef struct _sslAtContext{
    struct _sslAtContext *next;    
    uint8_t sscxtId;
    uint8_t sslversion:4;
    uint8_t sni:1;
    uint8_t ignore:1;
    uint8_t reserve:2;
    uint8_t seclevel;
    uint8_t cache;
    int32_t ciphersuite;
#ifndef FEATURE_REF_AT_CR_ENABLE
    char caCertPath[SSL_CERTPATH_MAX_SIZE];
    char clientCertPath[SSL_CERTPATH_MAX_SIZE];
    char clientKeyPath[SSL_CERTPATH_MAX_SIZE];
#else
    char caCertPath[MSSL_CERTNAME_MAX_LEN+1];
    char clientCertPath[MSSL_CERTNAME_MAX_LEN+1];
    char clientKeyPath[MSSL_CERTNAME_MAX_LEN+1];
    char pskid[SSL_PSKID_MAX_SIZE];
    uint16_t negotime;
    uint8_t inputFormat:2;
    uint8_t ignoreCertVerify:1;
#endif
} sslAtContext_t;

typedef struct sslSessionContextTag
{
    char server[SSL_URL_MAX_LEN+1];
    uint16_t port;
    size_t session_len;
    unsigned char session[SSL_SESSION_MAX_LEN];
}sslSessionContext;

bool atParserCiphersuites(const uint8_t* hexStr, uint32_t *pOutput);
sslAtTemplate_t* sslListFind(sslAtTemplate_t *list, uint8_t id);
void sslListInsert(sslAtTemplate_t **list, sslAtTemplate_t *listNode);
uint8_t sslListSize(sslAtTemplate_t *list);
sslATError_e sslGetSeclevel (sslAtTemplate_t* list, uint8_t sslctxid, uint8_t* authmode);
sslATError_e sslGetSni (sslAtTemplate_t* list, uint8_t sslctxid, uint8_t* sni);
sslATError_e sslGetIgnore (sslAtTemplate_t* list, uint8_t sslctxid, uint8_t* ignore);
sslATError_e sslGetSessioncache (sslAtTemplate_t* list, uint8_t sslctxid, uint8_t* cache);
sslATError_e sslGetCacertpath (sslAtTemplate_t* list, uint8_t sslctxid, char** cacertPath);
sslATError_e sslGetClientcertpath (sslAtTemplate_t* list, uint8_t sslctxid, char** clientcertPath);
sslATError_e sslGetClientkeypath (sslAtTemplate_t* list, uint8_t sslctxid, char** clientkeyPath);
sslATError_e sslGetCiphersuite (sslAtTemplate_t* list, uint8_t sslctxid, int32_t* ciphersuite);
int8_t loadFile2Buff(char* filename, char** buff, int32_t* bufflen);

void sslSaveSession(void);
void sslCheckSession(void);
void sslClearSessionNvm(void);

#endif
