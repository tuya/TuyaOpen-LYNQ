/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    ctw_mqtt.h
 * Description:  EC618 ctwing mqtt header file
 * History:      Rev1.0   2022-02-17
 *
 ****************************************************************************/
#ifndef _CTW_MQTT_H
#define _CTW_MQTT_H

#include <stdlib.h>
#include <string.h>
#include "ctw_common.h"
#include "cJSON.h"
#include "MQTTClient.h"
#ifdef FEATURE_MQTT_TLS_ENABLE
#include "mqtttls.h"
#endif

#define MQTT_FOTA_STAT_START   1
#define MQTT_FOTA_STAT_ING     2
#define MQTT_FOTA_STAT_END     3
#define MQTT_FOTA_STAT_ERR     4
#define MQTT_REG_URI_MAX_LEN             (128)

enum MQTT_CTW_RET
{
    MQTT_CTW_OK = 200,
    MQTT_CTW_ERR,
    MQTT_CTW_NETWORK_ERR,
    MQTT_CTW_CONTEXT_ERR,
    MQTT_CTW_PARAM_ERR,
    MQTT_CTW_SOCKET_ERR,
    MQTT_CTW_SOCKET_TIME_ERR,
    MQTT_CTW_MQTT_CONN_ERR,
    MQTT_CTW_TASK_ERR,
    MQTT_CTW_RECONNECT,
    MQTT_CTW_CLIENT_ERR,
    MQTT_CTW_BUSY_ERR,
    MQTT_CTW_CONTINUE,

    MQTT_CTW_MAX_ERR,
};

enum MQTT_CTW_TASK_STA
{
    MQTT_CTW_RECV_TASK_NOT_CREATE = 0,
    MQTT_CTW_RECV_TASK_CREATE,
    MQTT_CTW_RECV_TASK_RECONNECT,
    MQTT_CTW_RECV_TASK_DELETE,
    MQTT_CTW_SEND_TASK_NOT_CREATE = 0,
    MQTT_CTW_SEND_TASK_CREATE,
    MQTT_CTW_SEND_TASK_DELETE,

    MQTT_CTW_RECV_TASK_MAX,
};

typedef enum
{
    MQTT_INIT_STATE,
    MQTT_IPREADY_STATE,
    MQTT_REG_STATE,
    MQTT_KEEP_ONLINE_STATE,
    MQTT_EXIT_STATE,
} ctwMqttStateMachine_t;

enum CTW_MQTT_CMD_TYPE
{
    CTW_MQTT_KEEPALIVE_COMMAND = 60,
    CTW_MQTT_RECONNECT_COMMAND,
    CTW_MQTT_PUBLISH_ACK,
    CTW_MQTT_PUBLISH_REC,
};

typedef struct
{
    int mqtt_id;
    char *mqtt_topic; 
    int  mqtt_topic_len; 
    char *mqtt_payload; 
    int  mqtt_payload_len; 
    int tcp_id;
    int msg_id;
    int ret;
    int conn_ret_code;
    int sub_ret_value;
    int pub_ret_value;
    int fota_cmd;
    int fota_percent;
    int fota_ret;    
}ctwMqttMessage;

typedef struct
{
    char host[MQTT_REG_URI_MAX_LEN];
    uint32_t timestamp;
    char signature[65];
    char iccid[CTW_CCID_LEN+1];
    char imsi[CTW_IMSI_LEN+1];
    char imei[CTW_IMEI_LEN+1];
    INT16 rsrp;
    INT8  snr;
    INT8  txPower;
    uint16_t phyCellId;
}ctwMqttRegParam_t;


typedef struct
{
    UINT8   autoRegEnableFlag;   /*autoRegEnableFlag  0:disable    1:enable */
    UINT8   mqttsFlag;           /*mqttsFlag  0:mqtt    1:mqtts */
    UINT8   autoRegStatus;       /*autoRegStatus  0:not regist    1:have regist */
    ctwMqttRegParam_t *mqttRegParam;

    char *mqttUri; 
    unsigned int port;
    char *mqttSendBuf;
    int mqttSendBufLen;
    char *mqttReadBuf;
    int mqttReadBufLen;
    
    int reconnectCount;
    int (*reconnect) (void *reconnClient);
    MQTTPacket_connectData mqttConnectData;
    Network* mqttNetwork;
    MQTTClient *mqttClient;
    messageHandler mqttMsgHandler;
#ifdef FEATURE_MQTT_TLS_ENABLE
    mqttsClientContext* mqttsClient;
#endif
    UINT32  fotaFlag;       /*fotaFlag  0:not fota    1:is fota */
    
}ctwMqttRegContext_t;

void ctwMqttCalcSign(char* outSign, uint32_t* outTimestamp, char* masterKey);
uint8_t ctwMqttAuthReg(ctwMqttRegContext_t* pHttpClient, MWNvmCfgCtwMqttParam* pCtwMqttParam, ctwMqttRegParam_t* pRegParam);
uint8_t ctwMqttSendData(ctwMqttRegContext_t* pHttpClient, MWNvmCfgCtwMqttParam* pCtwMqttParam, ctwMqttRegParam_t* pRegParam, char* topic, char* data);
void ctwMqttRegisterInit(MWNvmCfgCtwMqttParam* pCtwMqttParam);
void ctwMqttStartRegisterRecvTask(void);
int ctwMqttSendPacket(ctwMqttRegContext_t *mqttContext, int length, Timer* timer, int rai, bool exceptdata);


















#endif
