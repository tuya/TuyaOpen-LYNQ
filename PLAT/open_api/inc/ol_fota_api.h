#ifndef __OL_FOTA_API__
#define __OL_FOTA_API__


#ifdef MBTK_OPENCPU_SUPPORT_SSL
#include "ssl.h"
#include "ctr_drbg.h"
#include "entropy.h"
#endif


#define MBTK_FOTA_SERVER_CONTEXT_STRLEN    128


typedef enum
{
    OL_FOTA_BEGIN = 0,              //FOTA begin http download
    OL_FOTA_DL_PROC = 1,            //FOTA download progress
    OL_FOTA_ERR = 2,                //FOTA  error
    OL_FOTA_DOWNLOADEND = 3,        //FOTA download end
    OL_FOTA_PACKAGE_MISMATCH = 4,   //FOTA package mismatch
    OL_FOTA_PARAM_ERRR = 5          //FOTA param error
} ol_fota_status;

typedef enum 
{
    MBTK_FTP_SSL_TYPE_NONE,
    MBTK_FTP_SSL_TYPE_PBSZ,
    MBTK_FTP_SSL_TYPE_AUTH,
}mbtk_ftp_ssl_type_enum;

typedef struct
{
    char host[MBTK_FOTA_SERVER_CONTEXT_STRLEN];
    int port;
    char username[MBTK_FOTA_SERVER_CONTEXT_STRLEN];
    char password[MBTK_FOTA_SERVER_CONTEXT_STRLEN];
    int ssl_type;       //enum mbtk_ftp_ssl_type_enum
    int is_ipv6;
    char path[MBTK_FOTA_SERVER_CONTEXT_STRLEN];
}mbtk_fota_server_info;

#ifdef MBTK_OPENCPU_SUPPORT_SSL
struct ssl_client
{
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config config;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_entropy_context entropy;
    mbedtls_x509_crt ca_chain;
    mbedtls_x509_crt client_chain;
    mbedtls_pk_context client_rsa;
    int fd;
};
#endif


typedef void (*ol_fota_callback)(ol_fota_status status, int percent);


/*****************************************************************************
 * DESCRIPTION
 *    This API is used to download fota package from http network
 *
 * PARAMETERS
 *    url         :[OUT]  the http url
 *    callback    :[IN]   the callback function report while fota process
 *
 * RETURN
 *    =0 : success
 *    -1 : error
 *****************************************************************************/
extern int ol_fota_http_api(char* url, ol_fota_callback callback);


/*****************************************************************************
 * DESCRIPTION
 *    This API is used to download fota package through ftp server
 *
 * PARAMETERS
 *    server_info         :[IN]  the information about server,struct mbtk_fota_server_info
 *    callback            :[IN]   the callback function report while fota process
 *
 * RETURN
 *    =0 : success
 *    -1 : error
 *****************************************************************************/
extern int ol_fota_ftp_api(mbtk_fota_server_info server_info, ol_fota_callback callback);


/*****************************************************************************
 * DESCRIPTION
 *    This API is used to init fota through nvm mode
 *
 * PARAMETERS
 *    NONE
 *
 * RETURN
 *    =0 : success
 *    -1 : error
 *****************************************************************************/
extern int ol_fota_nvm_init(void);


/*****************************************************************************
 * DESCRIPTION
 *    This API is used to deinit fota through nvm mode
 *
 * PARAMETERS
 *    NONE
 *
 * RETURN
 *    =0 : success
 *    -1 : error
 *****************************************************************************/
extern int ol_fota_nvm_deinit(void);


/*****************************************************************************
 * DESCRIPTION
 *    This API is used to write fota package to the fota partition through nvm mode
 *
 * PARAMETERS
 *    offset          :[IN]   offset of the write address
 *    data            :[IN]   the data packet to be written
 *    data_len        :[IN]   the data packet length
 *
 * RETURN
 *    =0 : success
 *    -1 : error
 *****************************************************************************/
extern int ol_fota_nvm_write(unsigned long offset, unsigned char* data, unsigned long data_len);


/*****************************************************************************
 * DESCRIPTION
 *    This API is used to clear the fota nvm
 *
 * PARAMETERS
 *    offset          :[IN]   offset of the clear address
 *    data_len        :[IN]   the data length clear
 *
 * RETURN
 *    =0 : success
 *    -1 : error
 *****************************************************************************/
extern int ol_fota_nvm_clear(unsigned long offset, unsigned long data_len);


/*****************************************************************************
 * DESCRIPTION
 *    This API is used to get the fota packet size
 *
 * PARAMETERS
 *    NONE
 *
 * RETURN
 *    >=0  : the size of packet
 *****************************************************************************/
extern unsigned long ol_fota_nvm_size(void);


/*****************************************************************************
 * DESCRIPTION
 *  This API is used to check the fota packet
 *
 * PARAMETERS
 *      NONE
 *
 * RETURN
 *    =0 : success
 *    -1 : error
 *****************************************************************************/
extern int ol_fota_nvm_check(void);


/*****************************************************************************
 * DESCRIPTION
 *    This API is used to reboot module and start ota upgrade
 *
 * PARAMETERS
 *    NONE
 *
 * RETURN
 *    VOID
 *****************************************************************************/
extern void ol_fota_reboot_upgrade(void);


/*****************************************************************************
 * DESCRIPTION
 *    This API is used to set the https fota ssl config
 *
 * PARAMETERS
 *    seclevel              :[IN]  the security level /0:no verify; 1:verify server; 2:both verify
 *    ca                    :[IN]  the ca cert
 *    client_cert           :[IN]  the client cert
 *    clientPk              :[IN]   the client key
 *    ignore                :[IN]   1:ignore the cert 0:not ignore
 *
 * RETURN
 *    =0 : success
 *    -1 : error
 *****************************************************************************/
extern int ol_http_fota_set_ssl(int seclevel, char *ca, char* client_cert, char * clientPk,int ignore);


/*****************************************************************************
 * DESCRIPTION
 *    This API is used to set the ftps fota ssl config
 *
 * PARAMETERS
 *    ca                    :[IN]  the ca cert
 *    cert                  :[IN]  the client cert
 *    key                   :[IN]   the client key
 *
 * RETURN
 *    =0 : success
 *    -1 : error
 *****************************************************************************/
extern int ol_ftp_fota_set_ssl(char *ca, char * cert, char *key);


#ifdef MBTK_OPENCPU_SUPPORT_SSL
/*****************************************************************************
 * DESCRIPTION
 *    This API is used to download fota package through mbtk server
 *
 * PARAMETERS
 *    imei              :[IN]  the imei registered in server
 *    version           :[IN]  the current version
 *    target_verson     :[IN]  the target version name
 *    callback          :[IN]   the callback function report while fota process
 *
 * RETURN
 *    =0 : success
 *    -1 : error
 *****************************************************************************/
extern int ol_fota_mbtk_api(char *imei, char * version, char * target_version, ol_fota_callback callback);
#endif

#endif
