/****************************************************************************

            (c) Copyright 2019 by 天翼物联科技有限公司. All rights reserved.

****************************************************************************/

#ifndef _CTIOT_LWM2M_SDK_H
#define _CTIOT_LWM2M_SDK_H

#include "slpman.h"

//#include "ctiot_common_type.h"
#include "../port/ct_platform.h"
#include "common/ctiot_aep_msg_queue.h"
#include "er-coap-13.h"
#ifdef WITH_MBEDTLS
#include "../port/dtlsconnection.h"
#else
#include "../port/connection.h"
#endif


#ifdef __cplusplus
extern "C"
{
#endif

#define CTIOT_DEFAULT_LOCAL_PORT "56830"
#define CTIOT_DEFAULT_SERVER_ID 123
#define CTIOT_DEREG_WAIT_TIME 20000
#define CTIOT_MAX_PACKET_SIZE 1080
#define CTIOT_MAX_QUEUE_SIZE 20
#define CTIOT_THREAD_TIMEOUT 200*1000
#define CTIOT_MAX_URI_LEN     272

//#ifdef  FEATURE_REF_AT_ENABLE
#define CTIOT_MAX_RECV_QUEUE_SIZE 8
//#endif

typedef enum
{
    REG_SUCCESS,
    REG_FAILED_TIMEOUT,
    REG_FAILED_AUTHORIZE_FAIL,
    REG_FAILED_ERROR_ENDPOINTNAME,
    REG_FAILED_PROTOCOL_NOT_SUPPORT,
    REG_FAILED_OTHER,//5
    OBSERVE_UNSUBSCRIBE,
    OBSERVE_SUBSCRIBE,
    OBSERVE_ACK_ERROR,
    SEND_NON_DONE,
    SEND_CON_DONE,//10
    SEND_UPDATE_DONE,
    SEND_FAILED,
    RECV_DATA_MSG,
    DEREG_SUCCESS,
    DEREG_FAILED_TIMEOUT,//15
    DEREG_FAILED_SEND,
    DEREG_FAILED_SERVER_FORBIDDEN,
    DEREG_FAILED_URI_NOT_FOUND,
    DEREG_FAILED_UNKNOWN_CAUSE,
    UPDATE_SUCCESS,//20
    UPDATE_FAILED_TIMEOUT,
    UPDATE_FAILED_SEND,
    UPDATE_FAILED_SERVER_FORBIDDEN,
    UPDATE_FAILED_WRONG_PARAM,
    UPDATE_FAILED_UE_OFFLINE,//25
    PING_SUCCESS,
    PING_FAILED_TIMEOUT,
    PING_FAILED_SEND,
    PING_FAILED_SERVER_FORBIDDEN,
    PING_FAILED_WRONG_PARAM,//30
    PING_FAILED_UE_OFFLINE,
    RECV_RST_CMD,
    FOTA_BEGIN,
    FOTA_DOWNLOAD_SUCCESS,
    FOTA_DOWNLOAD_FAIL,//35
    FOTA_UPDATE_FAIL,
    FOTA_OVER,
    OBSV_CMD
} evnet_code_t;

typedef enum
{
    ENDPOINT_URN_IMEI,
    ENDPOINT_IMEI,
    //ENDPOINT_AUTH_SM9,
    ENDPOINT_IMEI_IMSI=6,
    ENDPOINT_MAX,
}ctiot_endpoint_mode_e;

typedef enum
{
    DISABLE_NAT=0,
    ENABLE_NAT
}ctiot_nat_type_e;     //NAT type

typedef enum
{
    U_MODE=0,
    UQ_MODE
}ctiot_uq_mode_e;     //手动阻塞功能

typedef enum
{
    NO_CERTIFICATE_MODE = 0,
    CERTIFICATE_MODE
}ctiot_certificate_mode_e;//

typedef enum
{
    AUTO_HEARTBEAT=0,     //自动心跳
    NO_AUTO_HEARTBEAT //不自动心�?
}ctiot_auto_heartbeat_e;//模组自动心跳

typedef enum
{
    PROTOCOL_MODE_NORMAL = 1,
    PROTOCOL_MODE_ENHANCE
}ctiot_funcv1_protocol_mode_e;//

typedef enum
{
    NETWORK_UNCONNECTED = 0,//未连�?
    NETWORK_CONNECTED,      //已连�?
    NETWORK_UNAVAILABLE,    //连接不可�?
    NETWORK_JAM,            //网络拥塞
}ctiot_funcv1_wireless_status_e;     //无线连接状�?
typedef enum
{
    SIGNAL_LEVEL_0 = 1,
    SIGNAL_LEVEL_1,
    SIGNAL_LEVEL_2
}ctiot_funcv1_signal_level_e;//无线连接状态下的网络覆盖等�?

typedef enum
{
    IP_V4 = 1,
    IP_V6,
    IP_V4V6
}ctiot_funcv1_ip_type_e;

typedef enum
{
    UE_NOT_LOGINED,
    UE_LOGINED,
    UE_LOGINING,
    UE_LOG_FAILED,
    UE_NO_IP,
    UE_LOG_OUT
}ctiot_login_status_e;     //登录状�?

typedef enum
{
    BOOT_NOT_LOAD = 0,
    BOOT_LOCAL_BOOTUP,
    BOOT_FOTA_REBOOT,
    BOOT_FOTA_REBOOT_OK,
    BOOT_FLAG_MAX_VALUE
}ctiot_boot_flag_e;     //加载方式

typedef enum
{
    HANDSHAKE_INIT,   //未握�?
    HANDSHAKE_OVER,   //握手完成
    HANDSHAKE_FAIL,   //握手失败
    HANDSHAKE_ING     //正在握手
}ctiot_funcv1_handshake_result_e;

typedef enum
{
    SENDMODE_CON,
    SENDMODE_NON,
    SENDMODE_NON_REL,
    SENDMODE_CON_REL,
}ctiot_send_mode_e;

typedef enum
{
    HAVE_NOT_SEND,       //haven't been send
    SENT_WAIT_RESP,      //sent, wait for response
    SENT_FAIL,           //sent failed
    SEND_TIMEOUT,        //timeout
    SEND_SUC,            //success
    GET_RST              //get reset command
}ctiot_funcv1_condata_status_e;//for *16*

/*AT status definiton*/
#define SR0 "reg,0"                 //登录成功
#define SR1 "reg,1"                 //登录失败，超时或其他原因
#define SR2 "reg,13"                //鉴权失败，Server拒绝接入
#define SR3 "reg,22"                //IOT Protocol或LWM2M版本不支�?
#define SR4 "reg,10"                //Endpoint Name无法识别或参数错�?
#define SR5 "reg,2"                 //platform no such device

#define SU0 "update,0"              //操作成功
#define SU1 "update,14"             //platform no such device
#define SU2 "update,1"              //超时无响�?
#define SU3 "update,10"             //参数错误
#define SU4 "update,13"             //鉴权失败，Server拒绝接入

#define SD0 "dereg,0"               //登出成功

#define SL1 "lwstatus,25"           //会话加载失败
#define SL2 "lwstatus,26"           //engine异常

#define SE3 "lwstatus,29"         //模组退出休眠�?

#define SO1 "obsrv,0"                //object19传送通道已建�?
#define SO2 "obsrv,2"                //object19传输通道取消

#define SS2 "send,31"               //数据发送状态，已发�?
#define SS3 "send,0"                //数据发送状态，已送达
#define SS4 "send,1"                //数据发送状态，发送超�?
#define SS5 "send,9"                //数据发送状态，平台RST响应
#define SS6 "send,32"               //数据发送状态，传送通道失败



#define FOTA0 "FIRMWARE DOWNLOADING"
#define FOTA1 "FIRMWARE DOWNLOAD FAILED"
#define FOTA2 "FIRMWARE DOWNLOAD SUCCESS"
#define FOTA2_BC28 "FIRMWARE DOWNLOADED"
#define FOTA3 "FIRMWARE UPDATING"
#define FOTA4 "FIRMWARE UPDATE SUCCESS"
#define FOTA5 "FIRMWARE UPDATE FAILED"
#define FOTA6 "FIRMWARE UPDATE OVER"

#define HS0 "+QDTLSSTAT: 0"
#define HS3 "+QDTLSSTAT: 3"
#define REGNOTIFY "REGISTERNOTIFY"
#define REGERROR "ERROR"

/*AT ERRORs*/
typedef enum
{
    CTIOT_SUCCESS,
    CTIOT_EB_OTHER = 1,                         //其它错误
    CTIOT_PARAMETER_ERROR = 2,
    CTIOT_OPERATOR_NOT_SUPPORTED = 3,
    CTIOT_EB_NO_IP  = 4,                  
    CTIOT_LIFETIME_ERROR = 5,
    CTIOT_DATA_LENGTH_OVERRRUN = 6,            
    CTIOT_DATA_LENGTH_NOT_EVEN = 7,        
    CTIOT_NO_RECV_DATA  = 8,             
    CTIOT_NO_SET_REG_PARAM = 9,   
    CTIOT_ALREADY_LOGIN = 10,          
    CTIOT_NO_NETWORK = 11,
    CTIOT_IN_LOGING = 12, 
    CTIOT_NO_SESSION = 13,
    CTIOT_NO_AUTHSTR = 14,        
    CTIOT_NOT_LOGIN = 15,           
    CTIOT_OBJECT_NOT_OBSERVED = 16,      
    CTIOT_QUEUE_OVERRUN = 17,               
    CTIOT_UPLINK_BUSY = 159,                    //Uplink busy/flow control
    CTIOT_SM9_ERROR = 20,                       //SM9 error
    CTIOT_SM9_NO_SUCH_INDEX_ERROR = 21,         //SM9 no such index
    CTIOT_SM9_INIT_FAIL = 22,                   //SM9 init fail
    CTIOT_SM9_N0_INIT = 23,                     //SM9 no init 
    CTIOT_SM9_ENC_FAIL = 24,                    //SM9 enc fail
    CTIOT_SM9_DEC_FAIL = 25,                    //SM9 dec fail
    CTIOT_SM9_SIGN_FAIL = 26,                   //SM9 sign fail
    CTIOT_SM9_VERIFY_FAIL = 27,                 //SM9 veify fail
    CTIOT_SM9_GETONLINE_FAIL = 28              //SM9 get online key fail
}CtiotErrors;

typedef struct _ctiot_updata_list
{
    struct _ctiot_updata_list *next;
    //COAP
    uint16_t  msgid;
    uint8_t   token[8];
    uint8_t   tokenLen;
    ctiot_send_mode_e mode;
    //LWM2M
    uint8_t* uri;

    uint8_t *data;
    size_t datalength;
    uint8_t   seqnum;
}ctiot_updata_list_t;

typedef struct _ctiot_recvdata_list
{
    struct _ctiot_recvdata_list *next;
    uint16_t  msgid;
    uint16_t recvdataLen;
    uint8_t* recvdata;
}ctiot_recvdata_list_t;

typedef struct
{
    //ctiot_funcv1_status_type_e statusType;
    char* baseInfo;
    void* extraInfo;
}ctiot_funcv1_status_t;

typedef struct
{
    char *                   cChipType;
    char                     cApn[100];
    char                     cImsi[20];
    char                     cImei[20];
    char                     cIccid[21];
    uint32_t                 cCellID;
    char*                    cSoftVersion;//模组软件版本
    char*                    cModule;//模组型号
    int8_t                   cRsrp;//信号强度
    int8_t                   cSinr;//信噪�?
    int8_t                   cTxpower;//发射功率
    /*无线连接*/
    ctiot_funcv1_wireless_status_e  cState;
    ctiot_funcv1_signal_level_e     cSignalLevel;
    ctiot_funcv1_ip_type_e          cIptype;
}ctiot_funcv1_chip_info,*ctiot_funcv1_chip_info_ptr;

typedef struct
{
    lwm2m_object_t *securityObjP;
    lwm2m_object_t *serverObject;
    int sock;
#ifdef WITH_MBEDTLS
    mbedtls_connection_t * connList;
#ifdef  FEATURE_REF_AT_ENABLE
    uint8_t   handshakeResult;//EC add
    bool      resetHandshake;//EC add
#endif
    uint8_t   natType;//EC add
#else
    connection_t * connList;
#endif
    lwm2m_context_t * lwm2mH;
    int addressFamily;
    bool avoidDns;
} ctiot_client_data_t;

typedef enum{
    AT_TO_MCU_RECEIVE,
    AT_TO_MCU_STATUS,
    AT_TO_MCU_QUERY_STATUS,
    AT_TO_MCU_SENDSTATUS,
#ifdef  FEATURE_REF_AT_ENABLE
    AT_TO_MCU_QLWEVTIND,
    AT_TO_MCU_NSMI,
    AT_TO_MCU_HSSTATUS
#endif
}ctiot_at_to_mcu_type_e;


typedef struct
{

    /*lwm2m_context*/
    lwm2m_context_t *        lwm2mContext;

    /*****************************CLIENT*****************************/
    /*模组信息*/
    ctiot_funcv1_chip_info_ptr      chipInfo;
    ctiot_login_status_e     loginStatus;//登录状态，处理时要注意和lwm2m的state同步
    /*平台参数*/
    char *                   serverIP;
    uint32_t                 port;
#ifdef  FEATURE_REF_AT_ENABLE
    char *                   bsServerIP;
    uint32_t                 bsPort;
    char *                   endpoint;
    uint8_t                  dtlsType;
#else //mbtk change
    char *                   bsServerIP;
    char *                   endpoint;
#endif
	//bool                     ipReadyRereg;
    char                     localIP[40];
    uint32_t                 lifetime;
    ctiot_client_data_t      clientInfo;
    /*加载方式*/
    ctiot_boot_flag_e        bootFlag;//临时数据*/
    uint8_t                  endpointMode;
    uint8_t                  natType;
    uint8_t                  uqMode;
    uint8_t                  certificateMode;

    uint8_t                         idAuthType;
    /*FOTA*/
    uint8_t                  fotaMode;
    uint8_t                  regFlag;
    uint8_t                  restoreFlag;
    char *                   pskid;
    uint8_t *                psk;
    uint16_t                 pskLen;

    /*上下行list*/
    ctiot_msg_list_head *      updataList;
    ctiot_msg_list_head *    recvdataList;

    /*线程*/
    bool send_thread_status; //false:no need task; true:need task
    bool send_thread_run;       //false:task not running; true:task is running
    bool main_thread_status; //false:no need task; true:need task
    bool main_thread_run;       //false:task not running; true:task is running
    uint32_t                      reqhandle;  //EC add
    
    uint8_t                  nnmiFlag;
#ifdef  FEATURE_REF_AT_ENABLE
    uint8_t                  nsmiFlag;
    /*con data status*/
    ctiot_funcv1_condata_status_e conDataStatus;
    uint8_t                       seqNum;    
#endif
    uint8_t                  paramRestore;
    char*                    hostRestore;
    char*                    location;
    bool                     autoHeartBeat;
}ctiot_context_t;

typedef void(*ctlwm2m_event_handler_t)(INT32 code);
typedef void(*ctlwm2m_ind_handler_t)(uint32_t reqhandle, char* at_str);
typedef void(*ctiot_notify_ind_t)(ctiot_at_to_mcu_type_e infoType,void* params,uint16_t paramLen);

void ctiot_register_event_handler(ctlwm2m_event_handler_t callback);
void ctiot_notify_event(int code);
void ctiot_register_ind_handler(ctlwm2m_ind_handler_t callback);
void ctiot_call_ind(uint32_t reqhandle, char* msg);

ctiot_funcv1_chip_info* ctiot_get_chip_instance(void);
ctiot_context_t* ctiot_get_context(void);
void ctiot_notify_ind(ctiot_at_to_mcu_type_e infoType,void* params,uint16_t paramLen );
void prv_set_uri_option(coap_packet_t* messageP,lwm2m_uri_t* uriP);
uint16_t ctiot_set_boot_flag(uint8_t bootFlag);
int prv_extend_query_len(void);
char* prv_extend_query(int querylen);

int resource_value_changed(char* uri);
void ctiot_setCoapAckTimeout(ctiot_funcv1_signal_level_e celevel);

char* prv_at_encode(uint8_t* data,int datalen);

void ctiot_recvlist_slp_set(bool slp_enable);


#ifdef __cplusplus
}
#endif
#endif//_CTIOT_LWM2M_SDK_H



