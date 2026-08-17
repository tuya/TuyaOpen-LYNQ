#ifndef _AT_FILE_TASK_
#define _AT_FILE_TASK_

#define FILE_SEMPHR_MAX_NUMB   4 
#define FILE_TASK_STACK_SIZE   1600
#define FILE_INFO_MAX_NUMB     16
#define FILE_MSG_TIMEOUT       2000
#define FILE_TASK_CREATE   1 
#define FILE_TASK_DELETE   2 
#define FILE_READ_MAX_COUNT   4 
#define FILE_CRC_LEN          2 

#define FILE_READ_DEFAULT_LEN        0xffff

enum FILE_RET
{
    CR_FILE_OK = 200,
    CR_FILE_ERR,
    CR_FILE_CTRL_Z_OK,

    CR_FILE_MAX_ERR,
};
    
enum FILE_CFG_FORMAT_MODE
{
    CR_FILE_CFG_INPUT_FORMAT_ASCII = 0,
    CR_FILE_CFG_INPUT_FORMAT_HEX   = 1,
    CR_FILE_CFG_INPUT_FORMAT_ECSP  = 2,
    CR_FILE_CFG_OUTPUT_FORMAT_ASCII = 0,
    CR_FILE_CFG_OUTPUT_FORMAT_HEX   = 1,
};
    
enum FILE_CFG_MODE
{
    CR_FILE_CFG_MODE_0 = 0,
    CR_FILE_CFG_MODE_1,
    CR_FILE_CFG_MODE_2,
    CR_FILE_CFG_MODE_3,
    CR_FILE_CFG_MODE_4,
};
        
enum FILE_CHECK_MODE
{
    CR_FILE_CHECK_MODE_MD5 = 0,
    CR_FILE_CHECK_MODE_SHA,
    CR_FILE_CHECK_MODE_SHA256,
    CR_FILE_CHECK_MODE_CRC,
};

enum FILE_MODE
{
    CR_FILE_MODE_0 = 0,
    CR_FILE_MODE_1,
    CR_FILE_MODE_2,
    CR_FILE_MODE_3,
    CR_FILE_MODE_4,
};

enum FILE_STATUS
{
    CR_FILE_STATUS_RESERVE,
    CR_FILE_STATUS_OPEN,
    CR_FILE_STATUS_READ,
    CR_FILE_STATUS_WRITE,
    CR_FILE_STATUS_SEEK,
    CR_FILE_STATUS_POSITION,
    CR_FILE_STATUS_TUCAT, 
    CR_FILE_STATUS_CLOSE, 
    CR_FILE_STATUS_RENAME,
    CR_FILE_STATUS_DELETE, 
    CR_FILE_STATUS_ERASE, 
    CR_FILE_STATUS_DELETE_ALL, 
    
    CR_FILE_STATUS_MSLDS,     /*memory space LDS*/
    CR_FILE_STATUS_MSLST,     /*memory space LST*/
    CR_FILE_STATUS_MSDEL,     /*memory space DEL*/
    CR_FILE_STATUS_MSUPL,     /*memory space UPL*/
    CR_FILE_STATUS_MSDWL,     /*memory space DWL*/
    CR_FILE_STATUS_MOV, 

};

enum FILE_MSG_CMD
{
    CR_FILE_MSG_RESERVE,
    CR_FILE_MSG_CFG,
    CR_FILE_MSG_INFO,
    CR_FILE_MSG_LIST,
    CR_FILE_MSG_SIZE,
    CR_FILE_MSG_PUT,
    CR_FILE_MSG_GET,
    CR_FILE_MSG_OPEN,
    CR_FILE_MSG_READ,
    CR_FILE_MSG_WRITE,
    CR_FILE_MSG_SYNC,
    CR_FILE_MSG_SEEK,
    CR_FILE_MSG_TRUNC, 
    CR_FILE_MSG_CLOSE, 
    CR_FILE_MSG_DELETE, 
    CR_FILE_MSG_MOVE,
    CR_FILE_MSG_CHECK,
    CR_FILE_MSG_DELETE_ALL, 
};

typedef enum applCrFilePrimId_Enum
{
    APPL_CR_FILE_PRIM_ID_BASE = 0,
        
    APPL_CR_FILE_CFG_REQ, //
    APPL_CR_FILE_CFG_CNF,
    APPL_CR_FILE_CFG_IND,
    
    APPL_CR_FILE_INFO_REQ, //
    APPL_CR_FILE_INFO_CNF,
    APPL_CR_FILE_INFO_IND,
    
    APPL_CR_FILE_LIST_REQ, //
    APPL_CR_FILE_LIST_CNF,
    APPL_CR_FILE_LIST_IND,
    
    APPL_CR_FILE_SIZE_REQ, //
    APPL_CR_FILE_SIZE_CNF,
    APPL_CR_FILE_SIZE_IND,
    
    APPL_CR_FILE_PUT_REQ, //
    APPL_CR_FILE_PUT_CNF,
    APPL_CR_FILE_PUT_IND,
    
    APPL_CR_FILE_GET_REQ, //
    APPL_CR_FILE_GET_CNF,
    APPL_CR_FILE_GET_IND,

    APPL_CR_FILE_OPEN_REQ, //
    APPL_CR_FILE_OPEN_CNF,
    APPL_CR_FILE_OPEN_IND,

    APPL_CR_FILE_READ_REQ, //
    APPL_CR_FILE_READ_CNF,
    APPL_CR_FILE_READ_IND,

    APPL_CR_FILE_WRITE_REQ, //
    APPL_CR_FILE_WRITE_CNF,
    APPL_CR_FILE_WRITE_IND,
        
    APPL_CR_FILE_SYNC_REQ, //
    APPL_CR_FILE_SYNC_CNF,
    APPL_CR_FILE_SYNC_IND,

    APPL_CR_FILE_SEEK_REQ, //
    APPL_CR_FILE_SEEK_CNF,
    APPL_CR_FILE_SEEK_IND,

    APPL_CR_FILE_TRUNC_REQ, //
    APPL_CR_FILE_TRUNC_CNF,
    APPL_CR_FILE_TRUNC_IND,

    APPL_CR_FILE_CLOSE_REQ, //
    APPL_CR_FILE_CLOSE_CNF,
    APPL_CR_FILE_CLOSE_IND,
     
    APPL_CR_FILE_DELETE_REQ, //
    APPL_CR_FILE_DELETE_CNF,
    APPL_CR_FILE_DELETE_IND,   
        
    APPL_CR_FILE_MOVE_REQ, //
    APPL_CR_FILE_MOVE_CNF,
    APPL_CR_FILE_MOVE_IND,
    
    APPL_CR_FILE_CHECK_REQ, //
    APPL_CR_FILE_CHECK_CNF,
    APPL_CR_FILE_CHECK_IND,
    
    APPL_CR_FILE_ERASE_REQ, //
    APPL_CR_FILE_ERASE_CNF,
    APPL_CR_FILE_ERASE_IND,

    APPL_CR_FILE_PRIM_ID_END = 0xFF
}applCrFilePrimId;

typedef struct
{
    uint32_t timeout;
    uint32_t fragmentSize;
    uint32_t fragmentInterval;
    uint32_t encodingInputFormat;
    uint32_t encodingOutputFormat;
}crFileGlobalInfo_t;

typedef struct
{
    char *fileName;
    char *fileSysHandle;
    int32_t  flags;
    uint32_t fileHandler;
    uint32_t fileMode;
    uint32_t fileLen;
    uint32_t fileStatus;
}crFileInfo_t;

typedef struct
{
    uint32_t reqhandle;
    uint32_t cmdType;
    int32_t  flags;
    char *fileName;
    char *filePattern;
    char *fileSysHandle;
    char *fileBuff;
    char *fileNameNew;
    uint32_t fileHandler;    
    uint32_t mode;
    uint32_t length;
    uint32_t totalLength;
    uint32_t timeout;
    uint32_t offset;
    uint32_t position;
    uint32_t fileStatus;
    uint32_t truncLength;
    uint32_t cfgData1;
    uint32_t cfgData2;

}crFileInfoPara_t;


typedef struct
{
    uint32_t reqhandle;
    uint32_t cmdType;
    char *fileName;
    char *filePattern;
    char *fileNameNew;
    crFileInfo_t *fileInfoPtr;
    uint32_t fileHandle;
    uint32_t mode;
    uint32_t length;
    uint32_t totalLength;
    uint32_t timeout;
    uint32_t offset;
    uint32_t position;
    uint32_t truncLength;
    uint32_t cfgData1;
    uint32_t cfgData2;
    char *reqBuff;
}crFileReqMsg_t;

typedef struct
{
    uint32_t reqhandle;
    char *fileName;
    char *filePattern;
    uint32_t fileHandle;
    uint32_t result;
    uint32_t ack;
    char *retBuff;
    INT32 retLen;
    INT32 retTotalLen;
    
}crFileCnfMsg_t;

void crFileGetGlobalInfo(crFileGlobalInfo_t *info);
crFileInfo_t *crFileFindExistFileByName(crFileInfoPara_t fileInfo);
crFileInfo_t *crFileFindExistFileByHandler(crFileInfoPara_t fileInfo);
int crFileClientInterface(crFileInfoPara_t fileReq);
crFileInfo_t *crFileGetExistFile(uint32_t index);

#endif

