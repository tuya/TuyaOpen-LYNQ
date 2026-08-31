/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    ec_ftpclient_api.h
 * Description:  API interface implementation header file for ChinaNet lwm2m
 * History:      Rev1.0   2021-04-22
 *
 ****************************************************************************/
#ifndef __EC_FTPCLIENT_API_H__
#define __EC_FTPCLIENT_API_H__

#include "commontypedef.h"

#include "netdb.h"
#include "sockets.h"

#define FTP_DEFAULT_STRING_SIZE (301)
#define FTP_MAX_USR_CMD_SIZE    (FTP_DEFAULT_STRING_SIZE * 2)
#define FTP_DEFAULT_BUFSIZE     (1024)


#define FTP_DEFAULT_PORT        (21)
#define FTP_MODE_PORT           (0)
#define FTP_MODE_PASV           (1)
#define FTP_TYPE_BINARY         (0)
#define FTP_TYPE_ASCII          (1)
#define FTP_INVALID_SOCKET      (-1)
#define FTP_NEED_REST_CMD       (1)
#define FTP_NO_REST_CMD         (0)
#define FTP_MSS_SIZE            (1400)

#define FTP_MAX_FILE_LIMIT      (2048)

typedef enum {

  FTP_ERR_SUCCESS                          = 0,
  FTP_ERR_OFFSET_BEYOND_FILESIZE           = 3,
  FTP_ERR_LAST_CMD_INPROGRESS              = 4,
  FTP_ERR_SERVER_ERR                       = 10,
  FTP_ERR_CONNECT_FAIL                     = 11,
  FTP_ERR_LIST_EMPTY                       = 14,
  FTP_ERR_INVALID_PARAMETER                = 50,
  FTP_ERR_UNKNOWN_ERROR                    = 800,
  FTP_ERR_SERVICE_NOT_AVAILABLE            = 801,
  FTP_ERR_SERVICE_BUSY                     = 802,
  FTP_ERR_NETWORK_ERROR                    = 803,
  FTP_ERR_DO_NOTHING                       = 804,
  FTP_ERR_CWD_FAIL                         = 805,
  FTP_ERR_PWD_FAIL                         = 806,
  FTP_ERR_DATACONN_CLOSED                  = 807,
  FTP_ERR_LIST_CMD_FAILED                  = 808,
  FTP_ERR_DEL_FAIL                         = 809,
  FTP_ERR_RENAME_FILE_FAIL                 = 810,
  FTP_ERR_FILE_ERROR                       = 811,
  FTP_ERR_STORE_FILE_FAIL                  = 812,
  FTP_ERR_UPLOAD_FILE_FAIL                 = 813,
  FTP_ERR_TRANSFER_ABORTED,
  FTP_ERR_FILE_ACTION_NOT_TAKEN,
  FTP_ERR_ACTION_ABORTED_LOCAL_ERROR,
  FTP_ERR_ACTION_NOT_TAKEN_LACK_STORAGE,
  FTP_ERR_SYNTAX_CMD_UNRECOGNIZED,
  FTP_ERR_SYNTAX_CMD_PARAMETERS,
  FTP_ERR_COMMAND_NOT_IMPLEMENTED,
  FTP_ERR_BAD_SEQUENCE_BAD_CMD,
  FTP_ERR_CMD_PARAMETER_NOT_IMPLEMENTED,
  FTP_ERR_NOT_LOGGED_IN,
  FTP_ERR_NEED_FILE_ACCOUNT,
  FTP_ERR_ACTION_NOT_TAKEN,
  FTP_ERR_ACTION_ABORTED_PAGETYPE_UNKNOWN,
  FTP_ERR_FILE_ACTION_ABORTED,
  FTP_ERR_FILE_NAME_INVALID,
  FTP_ERR_CONNECTION_TIMEOUT,
  FTP_ERR_DNS_PARSE_FAIL,
  FTP_ERR_CONTINUE,
  FTP_ERR_ERROR_END,
}ftpErr_e;

typedef enum {
  FTP_STATE_CLOSED = 0,
  FTP_STATE_CONNECTED,
  FTP_STATE_LOGGED,
  FTP_STATE_XFERING,
  FTP_STATE_XFERING_DL,
  FTP_STATE_XFERING_UL,
} ftpCtrStatus_e;

typedef enum {
    FTP_COMMAND_NULL = 0,
    FTP_COMMAND_CONNECT = 1,
    FTP_COMMAND_RETR,
    FTP_COMMAND_LIST,
    FTP_COMMAND_NLST,
    FTP_COMMAND_CWD,
    FTP_COMMAND_PWD,
    FTP_COMMAND_DEL,
    FTP_COMMAND_MKDIR,
    FTP_COMMAND_RMDIR,
    FTP_COMMAND_MDTM,
    FTP_COMMAND_RENAME,
    FTP_COMMAND_DATA_TRANSFER,
    FTP_COMMAND_STOR,
    FTP_COMMAND_APPE,
    FTP_COMMAND_ABOR,
    FTP_COMMAND_PORT,
    FTP_COMMAND_HELP,
    FTP_COMMAND_QUIT,
}ftpCommand_e;

typedef enum {
    FTP_SUBCOMMAND_SEND_PASV_CMD = 1,
    FTP_SUBCOMMAND_LIST = 2,
    FTP_SUBCOMMAND_GET_FILESIZE = 3,
    FTP_SUBCOMMAND_SET_BINMODE = 4,
    FTP_SUBCOMMAND_SEND_REST = 5,
    FTP_SUBCOMMAND_DOWNLOAD = 6,
    FTP_SUBCOMMAND_SEND_ULCMD = 7,
    FTP_SUBCOMMAND_RNFR = 8,
    FTP_SUBCOMMAND_RNTO = 9,
    FTP_SUBCOMMAND_RMFILE = 10,
    FTP_SUBCOMMAND_RMDIR = 11,
}ftpSubCommand_e;


typedef enum{
    FTP_EVENT_CLOSE,
}ftpEvent_t;

typedef struct
{
    void *pFtpInd;
} ftpIndMsg_t;

typedef struct{
    uint32_t connId:3;//0~1 0:BINARY; 1:ASCII
    uint32_t transType:1;//0~1 0:BINARY; 1:ASCII
    uint32_t timeout:5;//2~30(10)
    uint32_t cId:4;//1~15(0)
    uint32_t reqHandle;
    char* user;
    char* passwd;
} ftpParam_t;

typedef struct _ftpListNode_t
{
    struct _ftpListNode_t * next;
    char  month[4];
    int year;
    int  day;
    int hour;
    int minute;
    char*  name;
    uint16_t nameLen;
    long fileSize;
    char chmod[4];
    uint8_t type;//0:file;1:dir;
}ftpListNode_t;

typedef struct{
      ip_addr_t        localIp;
      //osTimerId_t      timer;

      uint16_t         mode:1; //trans mode, 0:PORT; 1:PASV; now only support PASV default:1;
      uint16_t         type:1; //data trans mode, 0: binary; 1: ASCii; default:0
      uint16_t         uploadMode:1; //0:APPE, 1:STOR
      uint16_t         fsTransType:1; //0:uart; 1:localfile
      uint16_t         rstFlag:1;//0:no need REST cmd; 1:need REST cmd
      uint16_t         cid:4; //pdp context id 1~15
      uint16_t         connectId:3;
      uint16_t         connected:1;
      uint16_t         quit:1;
      uint16_t         execuStatus:2;//0:not executed;1:in progress;2:complete excution
      uint16_t         getFileoffset;
      uint16_t         getFileSize;

      uint32_t         primId; /*assign value to MI_PRIM, store the at config primary msg id*/
      int32_t          timeout;   /*set timeout for cmder execute, unit is second, default set to 90s */
      int32_t          recvTout;  /*set received timeout value for socket*/
      uint32_t         reqHandler; /*match to at cmder config reqHandler*/
      ftpCtrStatus_e   ftpStatus;

      int32_t          setDataOff;
      int32_t          setDataLen;

      char*            dlPayload;
      int32_t          dlPayloadLen;

      char*            ulPayload;
      int32_t          ulPayloadLen;

      char* user;
      char* passwd;
      char* remoteFileName;//newName
      char* localFileName;//oldName
      char* protocolRet; //record the protocol error code return
      char* strUserCmd;

      struct sockaddr_in ftpServerCtrl;
      struct sockaddr_in ftpServerData;
      struct sockaddr_in ftpLocalHost;

      int controlSockFs;
      int dataSockFs;
      uint8_t curCmd;
      uint8_t subCmd;
} ftpContext_t;

ftpErr_e ftpContextInit(ftpContext_t* pContext, ftpParam_t* pParam);
ftpErr_e ftpContextClose(ftpContext_t* pContext);
ftpErr_e ftpContextConnect(ftpContext_t* pContext, char* url);
ftpErr_e ftpContextQuit(ftpContext_t* pContext);
ftpErr_e ftpQuitRsp(ftpContext_t* pContext);
ftpErr_e ftpContextCwd(ftpContext_t* pContext, char *path);
ftpErr_e ftpContextPwd(ftpContext_t* pContext);
ftpErr_e ftpContextMkdir(ftpContext_t* pContext, char * path);
ftpErr_e ftpContextDelFile(ftpContext_t* pContext, char * file);
ftpErr_e ftpContextDelDir(ftpContext_t* pContext, char * dir);
ftpErr_e ftpSetBinMode(ftpContext_t* pContext);
ftpErr_e ftpSetRSTMode(ftpContext_t* pContext);
ftpErr_e ftpDownload(ftpContext_t* pContext);
ftpErr_e ftpListHandle(ftpContext_t* pContext);
ftpErr_e ftpDownloadRsp(ftpContext_t* pContext);
ftpErr_e ftpSendUploadCmd(ftpContext_t* pContext);
ftpErr_e ftpUpload(ftpContext_t* pContext);
ftpErr_e ftpSendListCmd(ftpContext_t* pContext);
ftpErr_e ftpSendRntoCmd(ftpContext_t* pContext);
ftpErr_e ftpSendRetrCmd(ftpContext_t* pContext);
ftpErr_e ftpGetRemoteFileSizeRsp(ftpContext_t* pContext);
ftpErr_e ftpGetRemoteFileSizeCmd(ftpContext_t* pContext);
ftpErr_e ftpSendPasvCmd(ftpContext_t* pContext);
ftpErr_e ftpContextRename(ftpContext_t* pContext);

void ftpSetStatus(ftpContext_t* pContext, ftpCtrStatus_e status);
int ftpReadline(int fd, char* buf, int maxSize);
int ftpCreatePasvDataSock(ftpContext_t* pContext);
void ftpCloseDataSock(ftpContext_t* pContext, int sockfd);

#endif
