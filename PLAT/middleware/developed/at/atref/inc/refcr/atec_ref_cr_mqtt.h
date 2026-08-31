/******************************************************************************

*(C) Copyright 2018 EIGENCOMM International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: atec_mqtt.h
*
*  Description:
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef _ATEC_MQTT_H
#define _ATEC_MQTT_H

#include "at_util.h"

#define CR_MQTT_ID_MIN                 0
#define CR_MQTT_ID_MAX                 3
#define CR_MQTT_ID_DEF                 0xff
#define CR_MQTT_TCP_ID_MIN          0
#define CR_MQTT_TCP_ID_MAX          5 
#define CR_MQTT_TCP_ID_DEF          0

#define CR_MQTTCFG_CFG_QUERY_MAX_LEN      1600

/* AT+MQTTCFG */
#define CR_MQTTCFG_CFG_MAX_LEN            17
#define CR_MQTTCFG_CFG_STR_DEF             NULL
#define CR_MQTTCFG_TCP_ID_MIN          CR_MQTT_TCP_ID_MIN
#define CR_MQTTCFG_TCP_ID_MAX          CR_MQTT_TCP_ID_MAX
#define CR_MQTTCFG_TCP_ID_DEF          CR_MQTT_TCP_ID_DEF

#define CR_MQTTCFG_2_CACHE_MIN              0
#define CR_MQTTCFG_2_CACHE_MAX              1
#define CR_MQTTCFG_2_CACHE_DEF              0

#define CR_MQTTCFG_2_PDP_MIN              1
#define CR_MQTTCFG_2_PDP_MAX              8
#define CR_MQTTCFG_2_PDP_DEF              1

#define CR_MQTTCFG_2_INPUT_FORMAT_MIN              0
#define CR_MQTTCFG_2_INPUT_FORMAT_MAX              2
#define CR_MQTTCFG_2_INPUT_FORMAT_DEF              0
#define CR_MQTTCFG_3_OUTPUT_FORMAT_MIN                0
#define CR_MQTTCFG_3_OUTPUT_FORMAT_MAX                1
#define CR_MQTTCFG_3_OUTPUT_FORMAT_DEF                0

#define CR_MQTTCFG_2_KEEPALIVE_MIN              0
#define CR_MQTTCFG_2_KEEPALIVE_MAX              65535
#define CR_MQTTCFG_2_KEEPALIVE_DEF              120

#define CR_MQTTCFG_2_SSL_ENABLE_MIN              0
#define CR_MQTTCFG_2_SSL_ENABLE_MAX              1
#define CR_MQTTCFG_2_SSL_ENABLE_DEF              0

#define CR_MQTTCFG_3_SSL_ID_MIN              0
#define CR_MQTTCFG_3_SSL_ID_MAX              5
#define CR_MQTTCFG_3_SSL_ID_DEF              0

#define CR_MQTTCFG_2_SESSION_MIN              0
#define CR_MQTTCFG_2_SESSION_MAX              1
#define CR_MQTTCFG_2_SESSION_DEF              0

#define CR_MQTTCFG_2_RETRANS_INTERVAL_MIN              20
#define CR_MQTTCFG_2_RETRANS_INTERVAL_MAX              60
#define CR_MQTTCFG_2_RETRANS_INTERVAL_DEF              20

#define CR_MQTTCFG_2_RETRANS_TIMES_MIN              0
#define CR_MQTTCFG_2_RETRANS_TIMES_MAX              3
#define CR_MQTTCFG_2_RETRANS_TIMES_DEF              0

#define CR_MQTTCFG_2_PKT_MIN              1
#define CR_MQTTCFG_2_PKT_MAX              60
#define CR_MQTTCFG_2_PKT_DEF              10
#define CR_MQTTCFG_3_RETRY_MIN              1
#define CR_MQTTCFG_3_RETRY_MAX              10
#define CR_MQTTCFG_3_RETRY_DEF              3
#define CR_MQTTCFG_4_NOTICE_MIN               0
#define CR_MQTTCFG_4_NOTICE_MAX               1
#define CR_MQTTCFG_4_NOTICE_DEF               0

#define CR_MQTTCFG_2_WILLFLAG_MIN              0
#define CR_MQTTCFG_2_WILLFLAG_MAX              1
#define CR_MQTTCFG_2_WILLFLAG_DEF              0
#define CR_MQTTCFG_3_WILLQOS_MIN             0
#define CR_MQTTCFG_3_WILLQOS_MAX             2
#define CR_MQTTCFG_3_WILLQOS_DEF             0
#define CR_MQTTCFG_4_WILLQRETAIN_MIN            0
#define CR_MQTTCFG_4_WILLQRETAIN_MAX            1
#define CR_MQTTCFG_4_WILLQRETAIN_DEF            0
#define CR_MQTTCFG_5_WILLTOPIC_MAX_LEN               257
#define CR_MQTTCFG_5_WILLTOPIC_STR_DEF               NULL
#define CR_MQTTCFG_6_WILLMSG_MAX_LEN               257
#define CR_MQTTCFG_6_WILLMSG_STR_DEF               NULL

#define CR_MQTTCFG_2_PING_INTERVAL_MIN              60
#define CR_MQTTCFG_2_PING_INTERVAL_MAX              86400
#define CR_MQTTCFG_2_PING_INTERVAL_DEF              120

#define CR_MQTTCFG_2_PING_ACK_MIN              0
#define CR_MQTTCFG_2_PING_ACK_MAX              1
#define CR_MQTTCFG_2_PING_ACK_DEF              0

#define CR_MQTTCFG_2_INPUT_FORMAT_MIN              0
#define CR_MQTTCFG_2_INPUT_FORMAT_MAX              2
#define CR_MQTTCFG_2_INPUT_FORMAT_DEF              0

#define CR_MQTTCFG_3_OUTPUT_FORMAT_MIN              0
#define CR_MQTTCFG_3_OUTPUT_FORMAT_MAX              1
#define CR_MQTTCFG_3_OUTPUT_FORMAT_DEF              0

#define CR_MQTTCFG_2_CACHED_MIN              0
#define CR_MQTTCFG_2_CACHED_MAX              1
#define CR_MQTTCFG_2_CACHED_DEF              0

#define CR_MQTTCFG_2_RECONN_TIMES_MIN              0
#define CR_MQTTCFG_2_RECONN_TIMES_MAX              3
#define CR_MQTTCFG_2_RECONN_TIMES_DEF              3
#define CR_MQTTCFG_3_RECONN_INTERVAL_MIN              20
#define CR_MQTTCFG_3_RECONN_INTERVAL_MAX              60
#define CR_MQTTCFG_3_RECONN_INTERVAL_DEF              20
#define CR_MQTTCFG_4_RECONN_MODE_MIN              0
#define CR_MQTTCFG_4_RECONN_MODE_MAX              1
#define CR_MQTTCFG_4_RECONN_MODE_DEF              0

#define CR_MQTTCFG_2_VERSION_MIN              4
#define CR_MQTTCFG_2_VERSION_MAX              4
#define CR_MQTTCFG_2_VERSION_DEF              4

#define CR_MQTTCFG_2_PRODKEY_MAX_LEN             33
#define CR_MQTTCFG_2_PRODKEY_STR_DEF             NULL
#define CR_MQTTCFG_3_DEVICENAME_MAX_LEN               33
#define CR_MQTTCFG_3_DEVICENAME_STR_DEF               NULL
#define CR_MQTTCFG_4_DEVICESECRET_MAX_LEN             65
#define CR_MQTTCFG_4_DEVICESECRET_STR_DEF             NULL
#define CR_MQTTCFG_5_PRODNAME_MAX_LEN             65
#define CR_MQTTCFG_5_PRODNAME_STR_DEF             NULL
#define CR_MQTTCFG_6_PRODSECRET_MAX_LEN             65
#define CR_MQTTCFG_6_PRODSECRET_STR_DEF             NULL
#define CR_MQTTCFG_7_AUTHTYPE_MAX_LEN             33
#define CR_MQTTCFG_7_AUTHTYPE_STR_DEF             NULL
#define CR_MQTTCFG_8_SIGNMETHOD_MAX_LEN             33
#define CR_MQTTCFG_8_SIGNMETHOD_STR_DEF             NULL
#define CR_MQTTCFG_9_AUTHMODE_MAX_LEN             33
#define CR_MQTTCFG_9_AUTHMODE_STR_DEF             NULL
#define CR_MQTTCFG_10_SECUREMODE_MAX_LEN             9
#define CR_MQTTCFG_10_SECUREMODE_STR_DEF             NULL
#define CR_MQTTCFG_11_INSTANCEID_MAX_LEN          33
#define CR_MQTTCFG_11_INSTANCEID_STR_DEF          NULL
#define CR_MQTTCFG_12_DYNREGUSED_MIN                  0
#define CR_MQTTCFG_12_DYNREGUSED_MAX                  8
#define CR_MQTTCFG_12_DYNREGUSED_DEF                  0

#define CR_MQTTCFG_1_SSL_MAX_LEN             9
#define CR_MQTTCFG_1_SSL_STR_DEF             NULL

#define CR_MQTTCFG_2_PSK_MAX_LEN             129
#define CR_MQTTCFG_2_PSK_STR_DEF             NULL
#define CR_MQTTCFG_3_PSKID_MAX_LEN               256
#define CR_MQTTCFG_3_PSKID_STR_DEF               NULL

#define CR_MQTTCFG_2_ECC_MAX_LEN             4001
#define CR_MQTTCFG_2_ECC_STR_DEF             NULL

#define CR_MQTTCFG_2_CA_MAX_LEN             4001
#define CR_MQTTCFG_2_CA_STR_DEF             NULL

#define CR_MQTTCFG_3_NAME_MAX_LEN             65
#define CR_MQTTCFG_3_NAME_STR_DEF             NULL

#define CR_MQTTCFG_2_CLOUD_MIN              0
#define CR_MQTTCFG_2_CLOUD_MAX              255
#define CR_MQTTCFG_2_CLOUD_DEF              0
#define CR_MQTTCFG_3_PAYLOADTYPE_MIN              0
#define CR_MQTTCFG_3_PAYLOADTYPE_MAX              255
#define CR_MQTTCFG_3_PAYLOADTYPE_DEF              1

/* AT+MQTTCONN*/
#define CR_MQTTCONN_TCP_ID_MIN          CR_MQTT_TCP_ID_MIN
#define CR_MQTTCONN_TCP_ID_MAX          CR_MQTT_TCP_ID_MAX
#define CR_MQTTCONN_TCP_ID_DEF          CR_MQTT_TCP_ID_DEF
#define CR_MQTTCONN_1_HOST_MAX_LEN             128
#define CR_MQTTCONN_1_HOST_STR_DEF             NULL
#define CR_MQTTCONN_2_PORT_MIN          1
#define CR_MQTTCONN_2_PORT_MAX          65535
#define CR_MQTTCONN_2_PORT_DEF          1883
#define CR_MQTTCONN_3_CLIENTID_MAX_LEN             128//256
#define CR_MQTTCONN_3_CLIENTID_STR_DEF             NULL
#define CR_MQTTCONN_4_USERNAME_MAX_LEN           128//256
#define CR_MQTTCONN_4_USERNAME_STR_DEF           NULL
#define CR_MQTTCONN_5_PWD_MAX_LEN             300//256
#define CR_MQTTCONN_5_PWD_STR_DEF             NULL

/* AT+ECMTDISC */
#define CR_MQTTDISC_TCP_ID_MIN          CR_MQTT_TCP_ID_MIN
#define CR_MQTTDISC_TCP_ID_MAX          CR_MQTT_TCP_ID_MAX
#define CR_MQTTDISC_TCP_ID_DEF          CR_MQTT_TCP_ID_DEF

/* AT+ECMTSUB */
#define CR_MQTTSUB_TCP_ID_MIN          CR_MQTT_TCP_ID_MIN
#define CR_MQTTSUB_TCP_ID_MAX          CR_MQTT_TCP_ID_MAX
#define CR_MQTTSUB_TCP_ID_DEF          CR_MQTT_TCP_ID_DEF
#define CR_MQTTSUB_1_MSGID_MIN              1
#define CR_MQTTSUB_1_MSGID_MAX              65535
#define CR_MQTTSUB_1_MSGID_DEF              10
#define CR_MQTTSUB_2_TOPIC_MAX_LEN          257
#define CR_MQTTSUB_2_TOPIC_STR_DEF          NULL
#define CR_MQTTSUB_3_QOS_MIN                    0
#define CR_MQTTSUB_3_QOS_MAX                    2
#define CR_MQTTSUB_3_QOS_DEF                    0

/* AT+ECMTUNSUB */
#define CR_MQTTUNSUB_TCP_ID_MIN          CR_MQTT_TCP_ID_MIN
#define CR_MQTTUNSUB_TCP_ID_MAX          CR_MQTT_TCP_ID_MAX
#define CR_MQTTUNSUB_TCP_ID_DEF          CR_MQTT_TCP_ID_DEF
#define CR_MQTTUNSUB_1_MSGID_MIN              1
#define CR_MQTTUNSUB_1_MSGID_MAX              65535
#define CR_MQTTUNSUB_1_MSGID_DEF              10
#define CR_MQTTUNSUB_2_TOPIC_MAX_LEN          257
#define CR_MQTTUNSUB_2_TOPIC_STR_DEF          NULL

/* AT+ECMTPUB */
#define CR_MQTTPUB_TCP_ID_MIN          CR_MQTT_TCP_ID_MIN
#define CR_MQTTPUB_TCP_ID_MAX          CR_MQTT_TCP_ID_MAX
#define CR_MQTTPUB_TCP_ID_DEF          CR_MQTT_TCP_ID_DEF
#define CR_MQTTPUB_1_TOPIC_MAX_LEN              257
#define CR_MQTTPUB_1_TOPIC_STR_DEF              NULL
#define CR_MQTTPUB_2_QOS_MIN                    0
#define CR_MQTTPUB_2_QOS_MAX                    2
#define CR_MQTTPUB_2_QOS_DEF                    1
#define CR_MQTTPUB_3_RETRAINED_MIN                  0
#define CR_MQTTPUB_3_RETRAINED_MAX                  1
#define CR_MQTTPUB_3_RETRAINED_DEF                  0
#define CR_MQTTPUB_4_DUP_MIN                  0
#define CR_MQTTPUB_4_DUP_MAX                  1
#define CR_MQTTPUB_4_DUP_DEF                  0
#define CR_MQTTPUB_5_LEN_MIN                  0
#define CR_MQTTPUB_5_LEN_MAX                  8192
#define CR_MQTTPUB_5_LEN_DEF                  0
#define CR_MQTTPUB_6_MSG_MAX_LEN                    8192
#define CR_MQTTPUB_6_MSG_STR_DEF                    NULL
#define CR_MQTTPUB_6_DATA_MODE_MSG_MAX_LEN          8192  //6*1024
#define CR_MQTTPUB_6_DATA_MODE_MSG_STR_DEF          NULL

#define CR_MQTT_DATA_FORMAT_DEF                     0
#define CR_MQTT_TLS_CA_SUB_SEQ_LEN                  64

typedef struct
{
    UINT32 reqHandle;
    int tcpId;
    int msgId;
    int qos;
    int retained;
    int rai;
    int dup;
    int inputLen;
    char *mqttTopic;
    
}crMqttPubData_T;

CmsRetId  crMqttCFG(const AtCmdInputContext *AtCmdReqParaPtr);
CmsRetId  crMqttCONN(const AtCmdInputContext *AtCmdReqParaPtr);
CmsRetId  crMqttDISC(const AtCmdInputContext *AtCmdReqParaPtr);
CmsRetId  crMqttSUB(const AtCmdInputContext *AtCmdReqParaPtr);
CmsRetId  crMqttUNS(const AtCmdInputContext *AtCmdReqParaPtr);
CmsRetId  crMqttPUB(const AtCmdInputContext *AtCmdReqParaPtr);
CmsRetId  crMqttPUBInputData(UINT8 chanId, UINT8 *pData, INT16 dataLength);
CmsRetId  crMqttPUBCancel(UINT8 chanId);
CmsRetId  crMqttPUBTimeOut(UINT8 chanId);
CmsRetId  crMqttREAD(const AtCmdInputContext *pAtCmdReq);
CmsRetId  crMqttSTATE(const AtCmdInputContext *pAtCmdReq);

CmsRetId crMqttCONNind(UINT16 primId, UINT16 reqHandle, UINT16 rc, void *paras);
CmsRetId crMqttDISCind(UINT16 primId, UINT16 reqHandle, UINT16 rc, void *paras);
CmsRetId crMqttSUBind(UINT16 primId, UINT16 reqHandle, UINT16 rc, void *paras);
CmsRetId crMqttUNSind(UINT16 primId, UINT16 reqHandle, UINT16 rc, void *paras);
CmsRetId crMqttPUBind(UINT16 primId, UINT16 reqHandle, UINT16 rc, void *paras);

CmsRetId crMqttSTATind(UINT16 primId, UINT16 reqHandle, UINT16 rc, void *paras);
CmsRetId crMqttRECVind(UINT16 primId, UINT16 reqHandle, UINT16 rc, void *paras);


#endif


