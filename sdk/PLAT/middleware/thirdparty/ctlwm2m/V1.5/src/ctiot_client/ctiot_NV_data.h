#ifndef __CTIOT_NV_DATA_H__
#define __CTIOT_NV_DATA_H__

#include "internals.h"

#define CTIOT_PARAMETER_VERSION    0
#define CTIOT_CONTEXT_VERSION    0

#define CTIOT_PARAMS_FILE_NAME      "ctiotparmams.nvm"
#define CTIOT_CONTEXT_FILE_NAME      "ctiotcontext.nvm"

#define PSK_LEN    32
#define PSKID_LEN  16
#define NV_ENDPOINT_NAME_SIZE 150
#define SERVER_LEN 256
#define MAX_LOCATION_SIZE 64

typedef struct ctiotNvmHeader_Tag
{
    UINT16 fileBodySize; //file body size, not include size of header;
    UINT8  version;
    UINT8  checkSum;
}ctiotNvmHeader;

typedef struct
{
    uint32_t                       reqhandle;                            
    
    uint32_t                       regFlag        :2;
    uint32_t                       nnmiFlag       :2;
    uint32_t                       natType        :1;                      
    uint32_t                       dtlsType       :1;                        
    uint32_t                       nsmiFlag       :1;                    
    uint32_t                       certificateMode:1;                       
    uint32_t                       endpointMode   :3;
    uint32_t                       idAuthType     :2;
    uint32_t                       onUqMode       :1;                     
    uint32_t                       autoHeartBeat  :1;                     
    uint32_t                       resv1          :11;                   
    uint32_t                       pskLen         :6;                    //8 bytes
    
    char                           psk_id[PSKID_LEN];
    uint8_t                        psk[PSK_LEN];                         //48 bytes
    
    char                           serverIP[SERVER_LEN+1];//server host  
    uint8_t                        resv2[3];                             //260 bytes
    uint32_t                       port;
    uint32_t                       lifetime;                             //8 bytes
    
    
    char                           bsServerIP[SERVER_LEN+1];             
    uint8_t                        resv3[3];                             //260 bytes
    uint32_t                       bsPort;                               //4 bytes
    
    char                           endpoint[NV_ENDPOINT_NAME_SIZE+1];    
    uint8_t                        resv4;                                //152 bytes
    uint32_t                       resvint[3];                           //12 bytes
}ctiot_persist_params_t;                        //size 752

typedef struct /* related to fota upgrade - do not change! */
{
    char                           localIP[40];//serverIp to avoid DNS
    lwm2mObserveBack               obsevInfo[MAX_LWM2M_OBSERVE_NUM];         //56*5 bytes
    char                           hostRestore[64];      //64 bytes
    uint8_t                        resvBytes[2];                             
    uint8_t                        bootFlag;                           
    uint8_t                        loginStatus;
    char                           location[MAX_LOCATION_SIZE];        //64 bytes
#if 0
    char                           publicId[PSKID_LEN];  //32
    uint8_t                        secretKey[PSKID_LEN]; //32
    uint8_t                        secretKeyLen;
#endif
}ctiot_persist_context_t;                        //size 456


int32_t ctiot_restore_params(void);
int32_t ctiot_save_params(void);
int32_t ctiot_restore_context(void);
int32_t ctiot_update_context(void);
int32_t ctiot_update_bootflag(void);
int32_t ctiot_update_localIP(void);
int32_t ctiot_update_loginStatus();
int32_t ctiot_clear_context(void);

#endif

