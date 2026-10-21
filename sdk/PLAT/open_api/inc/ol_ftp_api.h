#ifndef _OL_FTP_API_H_
#define _OL_FTP_API_H_

typedef enum
{
    OL_FTP_OK = 0,                  /*Success*/
    OL_FTP_ERROR = -1,              /*Non specific error*/
    OL_FTP_PARAM_ERR = -2,          /*Input parameter error, including ftp_config*/
    OL_FTP_CON_SERVER_FAIL = -3,    /*Connect to remote FTP server error*/
    OL_FTP_SSL_CON_FAIL = -4,       /*SSL connect error*/
}ftp_error_e;

typedef struct ftp_config
{
    int     is_ipv6;    /*0: ipv4; 1: ipv6; default: ipv4*/
    int     ftp_type;   /*0: binary mode; 1: ascii mode; default: binary mode*/
    int     port ;      /*FTP server port*/
    int     sslmode;    /*0: plain; 1: implict; 2: explict*/
    int     seclevel;   /*0: no verify; 1: verify server; 2: both verify*/
    char*   hostname;   /*FTP server address*/
    char*   username;   /*FTP username*/
    char*   password;   /*FTP username password*/
    char*   ca_crt;     /*CA certificate*/
    char*   client_crt; /*client certificate*/
    char*   client_key; /*client public key*/
} ftp_config_t;

/*****************************************************************************
 * DESCRIPTION
 *    This API is used to download file
 *
 * PARAMETERS
 *      remote_file         : [IN] The file name from the remote FTP server
 *      remote_path         : [IN] The file path from the remote FTP server
 *      rest                : [IN] Download file offset from specified location
 *      ftp_config          : [IN] FTP configuration information
 * RETURN
 *      0 : success
 *      <0: fail
*****************************************************************************/
int ol_ftp_getfile(char* remote_file, char* remote_path, int rest, ftp_config_t* ftp_config);


/*****************************************************************************
 * DESCRIPTION
 *    This API is used to upload file
 *
 * PARAMETERS
 *      local_file          : [IN] The local file name
 *      local_path          : [IN] The local file path
 *      rest                : [IN] Upload file offset from specified location
 *      ftp_config          : [IN] FTP configuration information
 * RETURN
 *      0 : success
 *      <0: fail
*****************************************************************************/
int ol_ftp_putfile(char* local_file, char* local_path, int rest, ftp_config_t* ftp_config);


/*****************************************************************************
 * DESCRIPTION
 *    This API is used to get file size
 *
 * PARAMETERS
 *      remote_file         : [IN] The file name from the remote FTP server
 *      remote_path         : [IN] The file path from the remote FTP server
 *      ftp_config          : [IN] FTP configuration information
 * RETURN
 *      0 : success
 *      <0: fail
*****************************************************************************/
int ol_ftp_size(char* remote_file, char* remote_path, ftp_config_t* ftp_config);


/*****************************************************************************
 * DESCRIPTION
 *    This API is used to delete file
 *
 * PARAMETERS
 *      remote_file         : [IN] The file name from the remote FTP server
 *      remote_path         : [IN] The file path from the remote FTP server
 *      ftp_config          : [IN] FTP configuration information
 * RETURN
 *      0 : success
 *      <0: fail
*****************************************************************************/
int ol_ftp_delete(char* remote_file, char* remote_path, ftp_config_t* ftp_config);


/*****************************************************************************
 * DESCRIPTION
 *    This API is used to get list info
 *
 * PARAMETERS
 *      remote_path         : [IN] The path of the list on the FTP server
 *      list                : [OUT] Buffer for saving list information
 *      list_len            : [IN] The length of the buffer
 *      ftp_config          : [IN] FTP configuration information
 * RETURN
 *      0 : success
 *      <0: fail
 *      >0: return list information length
*****************************************************************************/
int ol_ftp_list(char* remote_path, char* list, int list_len, ftp_config_t* ftp_config);


/*****************************************************************************
 * DESCRIPTION
 *    This API is used to create folder
 *
 * PARAMETERS
 *      remote_dir          : [IN] The name of the directory on the FTP server
 *      ftp_config          : [IN] FTP configuration information
 * RETURN
 *      0 : success
 *      <0: fail
*****************************************************************************/
int ol_ftp_mkdir(char* remote_dir, ftp_config_t* ftp_config);


/*****************************************************************************
 * DESCRIPTION
 *    This API is used to delete folder
 *
 * PARAMETERS
 *      remote_dir          : [IN] The name of the directory on the FTP server
 *      ftp_config          : [IN] FTP configuration information
 * RETURN
 *      0 : success
 *      <0: fail
*****************************************************************************/
int ol_ftp_rmdir(char* remote_dir, ftp_config_t* ftp_config);


#endif //OL_FTP_API_H_
