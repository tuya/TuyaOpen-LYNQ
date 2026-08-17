/*
 * atec_mqtt_task.h
 *
 *  Created on: 
 *      Author:
 */
#ifndef _MQTT_TASK_H
#define _MQTT_TASK_H

#include "MQTTClient.h"
#ifdef FEATURE_MQTT_TLS_ENABLE
#include "mqtttls.h"
#endif

#define CR_MQTT_SEND_Q_LENGTH      7

#define CR_MQTT_SEND_TIMEOUT       2000
#define CR_MQTT_RECV_TIMEOUT       5000
#define CR_MQTT_TASK_CREATE   1 
#define CR_MQTT_TASK_DELETE   0 

#define CR_MQTT_SEMPHR_NOT_CREATE   0 
#define CR_MQTT_SEMPHR_HAVE_CREATE   2 

#define CR_MQTT_SEMPHR_MAX_NUMB   6 

#define CR_MQTT_RECV_LOOP_TIMEOUT       2000

#define CM_EC_TOPIC_LENGTH                     128
#define CM_EC_BUFF_LENGTH                      128

#define CR_MQTT_CONTEXT_NUMB_MAX      6
#define CR_MQTT_ID_DEFAULT            0xff
#define CR_MQTT_CLOUD_DEFAULT         0xff
#define CR_MQTT_TCP_ID_DEFAULT        0xff
#define CR_MQTT_PDP_ID_DEFAULT        1

#define CR_MQTT_CMD_TIMEOUT_DEFAULT        10000
#define CR_MQTT_CMD_TIMEOUT_TX_DEFAULT        2
#define CR_MQTT_CMD_TIMEOUT_RX_DEFAULT        20

#define CR_MQTT_KEEPALIVE_DEFAULT        120

#define CR_MQTT_TX_BUF_DEFAULT        (4+128+6*1024)    /*header+topic+payload*/
#define CR_MQTT_RX_BUF_DEFAULT        (4+128+1024)    /*header+topic+payload*/

#define CR_MQTT_PORT_DEFAULT        1883

#define CR_MQTT_MSG_TIMEOUT        1000

#define CR_MQTT_ERR_ABRT          (-13)
#define CR_MQTT_ERR_RST           (-14)
#define CR_MQTT_ERR_CLSD          (-15)
#define CR_MQTT_ERR_BADE          (9)

#define CR_MQTT_TAG_LEN          8
#define CM_KEEPALIVE_RETRY_MAX    6
#define CR_MQTT_RECONN_MAX       0

#define ALI_DYNAMIC_REGISTER_IS_NOT_USED       0
#define ALI_DYNAMIC_REGISTER_IS_USED           1

#define CR_MQTT_SLP_NAME_LEN        12
#define CR_MQTT_READ_CACHE_LEN_MAX      (8*1024)

/*
 * APPL SGID: APPL_MQTT, related PRIM ID
*/


enum CR_MQTT_IND_TYPE
{
    CR_MQTT_IND_SEND     = 1,
    CR_MQTT_IND_RECV_ACK = 2,
    CR_MQTT_IND_CACHED_DROP_DATA = 3,
    CR_MQTT_IND_CACHED_RECV_DATA = 4,
};

enum CR_MQTT_CLIENT_CHANNEL_TYPE
{
    CR_MQTT_CLIENT_CHANNEL_0 = 0,
    CR_MQTT_CLIENT_CHANNEL_1 = 1,
    CR_MQTT_CLIENT_CHANNEL_2 = 2,
    CR_MQTT_CLIENT_CHANNEL_3 = 3,
    CR_MQTT_CLIENT_CHANNEL_4 = 4,
    CR_MQTT_CLIENT_CHANNEL_5 = 5,
    CR_MQTT_CLIENT_CHANNEL_MAX
};

enum CR_MQTT_PUB_TYPE
{
    CR_MQTT_PUB_CTRLZ = 0,
    CR_MQTT_PUB_AT = 1,
};

enum CR_MQTT_INT_RET 
{ 
    CM_MQTTSUCCESS = 0, 
    CM_MQTTKEEPALIVE = 1 
};

enum CR_MQTT_RET
{
    CR_MQTT_OK = 200,
    CR_MQTT_ERR,
    CR_MQTT_NETWORK_ERR,
    CR_MQTT_CONTEXT_ERR,
    CR_MQTT_PARAM_ERR,
    CR_MQTT_SOCKET_ERR,
    CR_MQTT_SOCKET_TIME_ERR,
    CR_MQTT_MQTT_CONN_ERR,
    CR_MQTT_TASK_ERR,
    CR_MQTT_RECONNECT,
    CR_MQTT_CLIENT_ERR,
    CR_MQTT_ALI_ENCRYP_ERR,
    CR_MQTT_BUSY_ERR,
    CR_MQTT_CONTINUE,

    MQTT_MAX_ERR,
};

enum CR_MQTT_CONTEXT
{
    CR_MQTT_CONTEXT_NOT_USED,
    CR_MQTT_CONTEXT_USED,  
    CR_MQTT_CONTEXT_IS_CREATING,
    CR_MQTT_CONTEXT_OPENED,
    CR_MQTT_CONTEXT_CONFIGED,
    
};

enum CR_MQTT_CONNECT
{
    CR_MQTT_CONN_DEFAULT,
    
    CR_MQTT_CONN_IS_CONNECTING,
    CR_MQTT_CONN_CONNECTED,
    CR_MQTT_CONN_CONNECT_FAIL,
    
    CR_MQTT_CONN_IS_CLOSING,
    CR_MQTT_CONN_CLOSED,
    CR_MQTT_CONN_CLOSED_FAIL,
    
    CR_MQTT_CONN_IS_DISCONNECTING,
    CR_MQTT_CONN_DISCONNECTED,
    CR_MQTT_CONN_DISCONNECTED_FAIL,
    
    CR_MQTT_CONN_RECONNECTING,
    CR_MQTT_CONN_RECONNECTING_FAIL,
    
};

enum CR_MQTT_MSG_CMD
{
    CM_MSG_RESERVE,
    CR_MQTT_MSG_OPEN,
    CR_MQTT_MSG_CONNECT,
    CR_MQTT_MSG_CLOSE,
    CR_MQTT_MSG_DISCONNECT,
    CR_MQTT_MSG_CREATE_CLINET,
    CR_MQTT_MSG_KEEPALIVE, 
    CR_MQTT_MSG_RECONNECT, 
    CR_MQTT_MSG_DELETE_CLINET, 
    CR_MQTT_MSG_PUBLISH, 
    CR_MQTT_MSG_PUBLISH_REC, 
    CR_MQTT_MSG_PUBLISH_REL, 
    CR_MQTT_MSG_PUBLISH_ACK,
    CR_MQTT_MSG_PUBLISH_COMP, 
    CR_MQTT_MSG_SUB, 
    CR_MQTT_MSG_UNSUB, 
    CR_MQTT_MSG_READ, 
    CR_MQTT_MSG_STATE, 
    CR_MQTT_MSG_CONNECT_DOWN, 
    CR_MQTT_MSG_ALI_DYN_CONNECT,
    
};

enum CR_MQTT_CMD
{
    ONENET_CMD,
    ALI_CMD,        
};

enum CR_MQTT_INPUT_FORMAT_TYPE
{
    CR_MQTT_INPUT_FORMAT_STR = 0,
    CR_MQTT_INPUT_FORMAT_HEX = 1,
    CR_MQTT_INPUT_FORMAT_ESCAPED_STR = 2,
    
    CR_MQTT_INPUT_FORMAT_DEFAULT = 0xFF,
};
    
enum CR_MQTT_OUTPUT_FORMAT_TYPE
{
    CR_MQTT_OUTPUT_FORMAT_STR = 0,
    CR_MQTT_OUTPUT_FORMAT_HEX = 1,
    CR_MQTT_OUTPUT_FORMAT_ESCAPED_STR = 2,
    
    CR_MQTT_OUTPUT_FORMAT_DEFAULT = 0xFF,
};

enum CR_MQTT_DATA_TYPE
{
    CR_MQTT_DATA_JSON = 0,
    CR_MQTT_DATA_STR = 1,
    CR_MQTT_DATA_HEX = 2,
    
    ONENET_DATA_TYPE1 = 1,
    ONENET_DATA_TYPE2 = 2,
    ONENET_DATA_TYPE3 = 3,
    ONENET_DATA_TYPE4 = 4,
    ONENET_DATA_TYPE5 = 5,
    ONENET_DATA_TYPE6 = 6,
    ONENET_DATA_TYPE7 = 7,
    ONENET_DATA_TYPE8 = 8,  //customer define data type
    ONENET_DATA_TYPE9 = 9,  //customer define data type

    CR_MQTT_DATA_DEFAULT = 0xFF,
};

enum CR_MQTT_CLOUD_TYPE
{
    CLOUD_TYPE_ONENET = 1,
    CLOUD_TYPE_ALI,
    CLOUD_TYPE_ECLIPSE,  /*not use client id, user name, passwd*/
    CLOUD_TYPE_NORMAL,   /*need client id, user name, passwd*/
    CLOUD_TYPE_MAX
};

enum CR_MQTT_CONFIG_TYPE
{
    CR_MQTT_CONFIG_BASE = 0,
    CR_MQTT_CONFIG_VERSION,
    CR_MQTT_CONFIG_CID,
    CR_MQTT_CONFIG_SSL,
    CR_MQTT_CONFIG_KEEPALIVE,
    CR_MQTT_CONFIG_CLEAN,
    CR_MQTT_CONFIG_RETRANS,
    CR_MQTT_CONFIG_WILLOPTION,
    CR_MQTT_CONFIG_WILLPAYLOAD,
    CR_MQTT_CONFIG_PINGREQ,
    CR_MQTT_CONFIG_PINGRESP,
    CR_MQTT_CONFIG_ENCODING,
    CR_MQTT_CONFIG_CACHED,
    CR_MQTT_CONFIG_RECONN,
    CR_MQTT_CONFIG_QUERY,

    
    CR_MQTT_CONFIG_ECHOMODE,
    CR_MQTT_CONFIG_DATAFORMAT,
    CR_MQTT_CONFIG_SEESION,
    CR_MQTT_CONFIG_KEEPALIVE_CMD_TIMEOUT,
    CR_MQTT_CONFIG_KEEPALIVE_CONN_TIMEOUT,
    CR_MQTT_CONFIG_TIMEOUT,
    CR_MQTT_CONFIG_CMD_TIMEOUT,
    CR_MQTT_CONFIG_CONN_TIMEOUT,
    CR_MQTT_CONFIG_WILL,
    CR_MQTT_CONFIG_ALIAUTH,
    CR_MQTT_CONFIG_OPEN,
    CR_MQTT_CONFIG_CLOUD,
    
    MQTT_CONFIG_MAX
};

enum CR_MQTT_SSL_TYPE
{
    CR_MQTT_SSL_NONE = 0,
    CR_MQTT_SSL_HAVE = 1,
    CR_MQTT_SSL_PSK = 2,
    CR_MQTT_SSL_ECC,
    CR_MQTT_SSL_CA,  
};

enum CR_MQTT_RECV_CACHE_TYPE
{
    CR_MQTT_READ_CACHE_NONE = 0,
    CR_MQTT_READ_CACHE_ENABLE = 1,
};

typedef struct
{
    int decParam1;
    int decParam2;
    int decParam3;
    char *strParam1;
    char *strParam2;
    char *strParam3;
    char *strParam4;
    char *strParam5;
    char *strParam6;
    char *strParam7;
    char *strParam8;
    char *strParam9;
    char *strParam10;
}crMqttCfgData_T;

typedef struct
{
    int crMqttRecvNodeTotalNumb;
    int crMqttRecvNodeTotalLen;
    char *crMqttRecvNode; 
}crMqttReadNodeInfo_T;

typedef struct
{
    int crMqttRecvNodeIndex;
    UINT32 crMqttRecvNodeMsgId;
    char *crMqttRecvNodeTopic; 
    int crMqttRecvNodeTopicLen; 
    char *crMqttRecvNodePayload; 
    int crMqttRecvNodePayloadLen; 
    char *crMqttRecvPrevNode; 
    char *crMqttRecvNextNode; 
}crMqttReadNode_T;

typedef struct
{
    int    cmd_type;
    unsigned int reqhandle;
    void * context_ptr;
    void * client_ptr;
    int tcp_id;
    int msg_id;
    int msgCount;
    int pub_mode;
    int server_ack_mode;
    char *sub_topic;
    char *unsub_topic;
    char *topic;
    int qos;
    int rai;
    int dup;
    MQTTMessage message;
}crMqttSendMsg_T;

typedef struct
{
    unsigned int reqhandle;
    int mqtt_id;
    char *mqtt_topic; 
    int  mqtt_topic_len; 
    char *mqtt_payload; 
    int  mqtt_payload_len; 
    int tcp_id;
    int msg_id;
    int dup;
    int ret;    
    int conn_ret_code;
    int sub_ret_value;
    char sub_ret_qos[4];
    int sub_ret_qos_count;
    int pub_ret_value;
    int indType;
    int outputDataFormat;
    crMqttReadNodeInfo_T mqttReadCacheInfo;
    crMqttReadNode_T *mqttReadCacheNode;
}crMqttMessage_T;

typedef struct
{
    char *product_name;
    char *product_key; 
    char *product_secret; 
    char *device_name; 
    char *device_secret;
    char *device_token;
    char *auth_type;       /*register, regnwl*/  
    char *sign_method;     /*hmac_sha1, hmac_sha256, hmac_md5*/  
    char *auth_mode;       /*tls-psk, tls-ca*/
    char *secure_mode;     /*-2, 2*/
    char *instance_id;     /* */
    char *client_id;       /*for regnwl */
    int  dynamic_register_used;
}ali_auth;

typedef struct
{
    UINT16 is_used;
    UINT16 is_connected;
    UINT16 is_mqtts;
    UINT16 cloud_type;
    UINT16 tcp_id;
    UINT16 mqtt_id;
    UINT32 reqHandle;
    char *mqtt_uri; 
    UINT32 port;
    CHAR *mqtt_send_buf;
    INT32 mqtt_send_buf_len;
    CHAR *mqtt_read_buf;
    INT32 mqtt_read_buf_len;
    INT32 mqttReadCacheFlag;
    crMqttReadNodeInfo_T mqttReadCacheInfo;
    crMqttReadNode_T *mqttReadCacheNode;
    
    INT32 reconnect_count;
    int (*reconnect) (void *c);
    MQTTPacket_connectData mqtt_connect_data;
    Network* mqtt_network;
    MQTTClient *mqtt_client;
    messageHandler mqtt_msg_handler;
#ifdef FEATURE_MQTT_TLS_ENABLE
    mqttsClientContext* mqtts_client;
#endif
    UINT16 echomode;
    UINT16 send_data_format;
    UINT16 recv_data_format;
    UINT32 keepalive;
    UINT16 keepaliveAckMode;
    UINT16 session;
    INT32 version;
    INT32 pkt_timeout;
    CHAR subCountQos[4];
    INT32 subCountMax;
    INT32 subCountCurr;
    ali_auth aliAuth;
    UINT32 retransInterval;
    UINT32 retransTimes;
    INT32  retransTimesCurr;
    
    UINT16 inputFormat;
    UINT16 outputFormat;
    UINT32 reconnTimes;
    UINT32 reconnInterval;
    UINT32 reconnMode;
    UINT32 reconnTimesCurr;
    UINT32 pdpId;
    
    CHAR *sub_topic;
    CHAR *unsub_topic;
    int payloadType;
    int ssl_type;
    CHAR *ecc_key;
    CHAR *ca_key;
    CHAR *host_name;

    int keepalive_retry_time;
    int mqtt_task_recv_status_flag;
    int mqtt_task_send_status_flag;
    int mqtt_sempr_status_flag;
    
    uint8_t mqttSlpHandler;
    
    void *mqtt_recv_task_handle;
    void *mqtt_send_task_handle;
    void *mqtt_send_msg_handle;
    
    Mutex mqtt_mutex;
}crMqttContext_T;

typedef struct
{
    unsigned char cleansession;
    unsigned char willFlag;
    MQTTPacket_willOptions will;
}crMqttOption_T;

typedef struct
{
    int dec_param1; /**/
    int dec_param2; /**/
    int dec_param3; /**/
    int dec_param4; /**/
    char *str_param1; /**/
    char *str_param2; /**/
    char *str_param3; /**/
    char *str_param4; /**/
    
}MQTT_CNF_STRUCT;

typedef struct
{
    int       channel;
    uint8_t   slpHandler;
    uint8_t   slpName[CR_MQTT_SLP_NAME_LEN];
    uint8_t   resv0;
    uint16_t  resv1;
}crMqttTaskPara_T;

void check_tcpip_ready(void);
int crMqttReconnect(void *v);
void MQTT_messageArrived(MessageData* data);
crMqttContext_T *crMqttFindContext(int tcpId);

int crMqttClientConfig(int cnfType, int tcpId, crMqttCfgData_T *cfgData);
int crMqttClientConnect(UINT32 reqHandle, int tcpId, char *mqttUri, int mqttPort, char *clientId, char *userName, char *passWord);
int crMqttClientDisconnect(UINT32 reqHandle, int tcpId);
int crMqttClientSub(UINT32 reqHandle, int tcpId, char *mqttSubTopic, int qos);
int crMqttClientUnsub(UINT32 reqHandle, int tcpId, char *mqttSubTopic);
int crMqttClientPub(UINT32 reqHandle, int tcpId, int dup, int qos, int retained, int pubMode, char *mqttPubTopic, int msgLen, char *mqttMessage, int rai);
int crMqttClientRead(UINT32 reqHandle, int tcpId, int msgCount);
int crMqttClientState(UINT32 reqHandle, int tcpId, int *state);
int crMqttReconnectTimerCreate(int tcpId);
int crMqttReconnectTimerDelete(int tcpId);
int crMqttReTransTimerCreate(crMqttSendMsg_T *mqttMsg);
int crMqttReTransTimerDelete(int tcpId);
int *crMqttReTransTimerDataGet(int tcp_id);
crMqttContext_T *crMqttCreateContext(int tcpId, char *mqttUri, int mqttPort, int txBufLen, int rxBufLen, int mode);

int crMqttRecvTaskInit(crMqttContext_T *mqttContext);
int crMqttCycle(crMqttContext_T* context, Timer* timer);
void crMqttCloseSession(MQTTClient* c);


#endif



