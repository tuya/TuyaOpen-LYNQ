#ifndef _OL_SSL_API_H_
#define _OL_SSL_API_H_

/*
 * SSL version enumeration
*/
typedef enum
{
    SSL_VSN_SSLV30,
    SSL_VSN_TLSV10,
    SSL_VSN_TLSV11,
    SSL_VSN_TLSV12,
    SSL_VSN_ALL
} SSLVersion;

/*
 * SSL verification enumeration
*/

typedef enum
{
    SSL_VERIFY_MODE_NONE = 0,        // don't verify peer's identification
    SSL_VERIFY_MODE_OPTIONAL = 1,    // verify peer's identification, but continue handshaking when verifies fail
    SSL_VERIFY_MODE_REQUIRED = 2,    // verify peer's identification, but stop handshaking when verifies fail
    SSL_VERIFY_MODE_UNSET = 3         // Used only for sni_authmode
} SSLVerifyMode;

/*
 * SSL certificate structure
*/

typedef char* SSLCertPathPtr;

typedef struct
{
    SSLCertPathPtr rootCA;
    SSLCertPathPtr clientKey;
    SSLCertPathPtr clientCert;
}SSLCertPath;

typedef enum
{
    SSL_CERT_ROOTCA,
    SSL_CERT_CLIENTKEY,
    SSL_CERT_CLIENTCERT
}SSLCertType;

typedef enum
{
    SSL_CERT_FROM_BUF,    // certificates come from buffer
    SSL_CERT_FROM_FS    // certificates come from file system
}SSLCertFrom;

typedef struct
{
    unsigned char *data;
    int len;
} clientKeyPassword;

typedef struct
{
    SSLCertFrom from;    // specify where certificates come from
    SSLCertPath path;    // specify certificates path
    clientKeyPassword clientKeyPwd;    // client key password
}SSLCert;

/*
 * SSL configuration structure
*/

typedef struct
{
    unsigned char* data;
    int len;
} SSLCTRDRBGSeed;

typedef char* SSLCipherListPtr;

typedef struct
{
    unsigned char profileIdx;        // profile idx, range:0-15
    char *serverName;                // server name
    unsigned short serverPort;        // server port
    unsigned char protocol;            // 0:SSL/TLS
    unsigned char dbgLevel;            // 0:no debug, 1:error, 2:state change, 3:informational, 4:verbose
    unsigned char sessionReuseEn;    // whether to reuse previous ssl session on next connection, 0:disable, 1:enable
    SSLVersion vsn;                    // ssl version
    SSLVerifyMode verify;            // verify mode
    SSLCert cert;                    // certificate info
    SSLCipherListPtr cipherList;    // cipher list pointer
    SSLCTRDRBGSeed CTRDRBGSeed;        // CTR-DRBG seed
} SSLConfig;

/*
 * SSL context structure
*/

typedef struct
{
    SSLConfig config;
    void *SSL;
} SSLCtx;


/*****************************************************************************
 * DESCRIPTION
 *    This API is used to set ssl configuration
 *
 * PARAMETERS
 *      sslCtx          : [IN] the ssl context structure
 *      config          : [IN] the set configuration
 *
 * RETURN
 *    NULL
*****************************************************************************/
void ol_ssl_set_config(SSLCtx * sslCtx, SSLConfig* config);


/*****************************************************************************
 * DESCRIPTION
 *    This API is used to init ssl context structure
 *
 * PARAMETERS
 *      sslCtx          : [IN] the ssl context structure
 *
 * RETURN
 *      0 : success
 *      <0: fail
*****************************************************************************/
int ol_ssl_ctx_init(SSLCtx * sslCtx);


/*****************************************************************************
 * DESCRIPTION
 *    This API is used to deinit ssl context structure
 *
 * PARAMETERS
 *      sslCtx          : [IN] the ssl context structure
 *
 * RETURN
 *      NULL
*****************************************************************************/
void ol_ssl_ctx_deinit(SSLCtx * sslCtx);


/*****************************************************************************
 * DESCRIPTION
 *    This API is used to perform the ssl handshake
 *
 * PARAMETERS
 *      sslCtx          : [IN] the ssl context structure
 *      timeout_ms      : [IN] the handshake timeout
 *
 * RETURN
 *      0 : success
 *      <0: fail
*****************************************************************************/
int ol_ssl_handshake(SSLCtx * sslCtx, int timeout_ms);


/*****************************************************************************
 * DESCRIPTION
 *    This API is used to write data to server
 *
 * PARAMETERS
 *      sslCtx          : [IN] the ssl context structure
 *      data            : [IN] the write data
 *      sz              : [IN] the write data size
 * RETURN
 *      >0 : write data length
 *      <0 : error
*****************************************************************************/
int ol_ssl_write(SSLCtx * sslCtx, const void* data, int sz);


/*****************************************************************************
 * DESCRIPTION
 *    This API is used to read data to server
 *
 * PARAMETERS
 *      sslCtx          : [IN] the ssl context structure
 *      data            : [OUT] pointer to the recieve data
 *      sz              : [IN]  the data size want to read
 * RETURN
 *      >0 : read data length
 *      <0 : error
*****************************************************************************/
int ol_ssl_read(SSLCtx * sslCtx, void* data, int sz);


/*****************************************************************************
 * DESCRIPTION
 *    This API is used to shutdown the ssl connect
 *
 * PARAMETERS
 *      sslCtx          : [IN] the ssl context structure
 * RETURN
 *      0 : success
 *      <0: fail
*****************************************************************************/
int ol_ssl_shutdown(SSLCtx * sslCtx);

#endif


