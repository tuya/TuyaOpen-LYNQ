/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    test.c
 * Description:  EC618 lwm2m demo entry source file
 * History:      Rev1.0   2018-10-12
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_MQTT_ONENET_ENABLE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "cmsis_os2.h"
#include "MQTTClient.h"
#include "cJSON.h"
#ifdef FEATURE_SUBSYS_MISC_ENABLE
#include "misc.h"
#endif
#ifdef FEATURE_SUBSYS_STATUS_ENABLE
#include "status.h"
#endif
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#ifdef FEATURE_SUBSYS_CMDPARSE_ENABLE
#include "cmdparse.h"
#endif
#ifdef FEATURE_SUBSYS_AUDIO_ENABLE
#include "audio.h"
#endif
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#include "onenet_mqtt.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#include "servicemanager.h"


extern QueueHandle_t  mqttSendMsgHandle;
extern MQTTClient     mqttClient;
extern Network        mqttNetwork;
extern char          *mqttSendbuf;
extern char          *mqttReadbuf;


extern int  MQTTStartRECVTask(void);
extern void MQTTCloseSession(MQTTClient* c);


#define THREAD_STACK_SIZE_ONENET_MQTT               (5 * 1024)
#define QUEUE_SIZE_ONENET_MQTT                      50
#define KEEP_ALIVE_INTERVAL_S                       (5 * 60)
#define COMMAND_TIMEOUT_MS                          40000
#define PAYLOAD_PREFIX_LEN                          3
#define PAYLOAD_TYPE                                3
#define ITEM_CLIENT_ID                              "clientId"
#define ITEM_USER_NAME                              "userName"
#define ITEM_PASSWORD                               "password"
#define JSON_LEN_MAX                                256
#define CLIENT_TOPIC_LEN_MAX                        64


#ifdef THREAD_STATIC
static StaticTask_t gOnenetMqttThreadCbMem                                   = {0};
static uint8_t      gOnenetMqttThreadStackMem[THREAD_STACK_SIZE_ONENET_MQTT] = {0};
#endif
static osThreadId_t gOnenetMqttThread = NULL;
OnenetMqttDevInfoT  gDevInfo          = {0};


/* JSON Struct
{
  "clientId": "string",
  "userName": "string",
  "password": "string"
}
*/

static void defaultMessageHandler(int unused, MessageData *messageData)
{
    char payload[messageData->message->payloadlen + 1];

    memset(payload, 0, messageData->message->payloadlen + 1);
    memcpy(payload, messageData->message->payload, messageData->message->payloadlen);
    SYSLOG_DEBUG("payloadlen=%d, payload=%s\r\n", messageData->message->payloadlen, payload);

#ifdef FEATURE_SUBSYS_CMDPARSE_ENABLE
    cmdparseQueuePut(messageData->message->payload, messageData->message->payloadlen);
#endif
}

static void threadOnenetMqtt(void *argument)
{
    int32_t                 resVal      = FAILURE;
    mqttSendMsg             sendMessage = {0};
    MQTTPacket_connectData  connectData = MQTTPacket_connectData_initializer;
    OnenetMqttDevInfoT     *devInfo     = NULL;
    uint8_t                 simStatus   = SIM_UNKNOWN;

    do
    {
        osDelay(100);
        simStatus = simGetStatus(0);
        if (simStatus == SIM_REMOVED)
        {
            SYSLOG_DEBUG("No SIM card.\r\n");
            goto labelEnd;
        }
    }
    while (simStatus != SIM_READY);

    while (nwIsReady() != true)
    {
        osDelay(100);
    }

    devInfo = onenetMqttDevInfoRead(DEV_INFO_RETURN_STRUCT);
    if (devInfo == NULL)
    {
        SYSLOG_EMERG("Failed to read device info.\r\n");
        goto labelEnd;
    }

    memset(&gDevInfo, 0,       sizeof(gDevInfo));
    memcpy(&gDevInfo, devInfo, sizeof(gDevInfo));
    free(devInfo);
    devInfo = NULL;

    connectData.clientID.cstring  = gDevInfo.clientId;
    connectData.username.cstring  = gDevInfo.userName;
    connectData.password.cstring  = gDevInfo.password;
    connectData.keepAliveInterval = KEEP_ALIVE_INTERVAL_S;

    mqttSendbuf = malloc(MQTT_SEND_BUFF_LEN);
    if (mqttSendbuf == NULL)
    {
        SYSLOG_EMERG("Failed to malloc %d bytes for mqttSendbuf.\r\n", MQTT_SEND_BUFF_LEN);
        goto labelEnd;
    }

    mqttReadbuf = malloc(MQTT_RECV_BUFF_LEN);
    if (mqttReadbuf == NULL)
    {
        SYSLOG_EMERG("Failed to malloc %d bytes for mqttReadbuf.\r\n", MQTT_RECV_BUFF_LEN);
        goto labelEnd;
    }

    memset(mqttSendbuf, 0, MQTT_SEND_BUFF_LEN);
    memset(mqttReadbuf, 0, MQTT_RECV_BUFF_LEN);

    NetworkInit(&mqttNetwork);
    MQTTClientInit(&mqttClient, &mqttNetwork, COMMAND_TIMEOUT_MS, (unsigned char *)mqttSendbuf, MQTT_SEND_BUFF_LEN, (unsigned char *)mqttReadbuf, MQTT_RECV_BUFF_LEN);
    mqttClient.defaultMessageHandler = defaultMessageHandler;

    if((NetworkSetConnTimeout(&mqttNetwork, MQTT_SEND_TIMEOUT, MQTT_RECV_TIMEOUT)) != 0)
    {
        SYSLOG_EMERG("Failed to set timeout.\r\n");
        goto labelEnd;
    }

    if ((NetworkConnect(&mqttNetwork, MQTT_SERVER_URI, MQTT_SERVER_PORT)) != 0)
    {
        SYSLOG_EMERG("OneNET network connect faild.\r\n");
        goto labelEnd;
    }

    if ((MQTTConnect(&mqttClient, &connectData)) != 0)
    {
        SYSLOG_EMERG("OneNET MQTT connect faild.\r\n");
        goto labelEnd;
    }

#ifdef FEATURE_SUBSYS_STATUS_ENABLE
    statusNotify();
#endif

    mqttSendMsgHandle = xQueueCreate(QUEUE_SIZE_ONENET_MQTT, sizeof(mqttSendMsg));
    if (mqttSendMsgHandle == NULL)
    {
        SYSLOG_EMERG("Failed to create queue for mqttSendMsgHandle.\r\n");
        goto labelEnd;
    }

    if (MQTTStartRECVTask() != SUCCESS)
    {
        SYSLOG_EMERG("Failed to start MQTT RECV task.\r\n");
        goto labelEnd;
    }

    if (onenetMqttSub(ONENET_MQTT_TOPIC_SPEAKER) != SUCCESS)
    {
        SYSLOG_EMERG("Failed to subscribe MQTT topic.\r\n");
        goto labelEnd;
    }

    SYSLOG_INFO("OneNET MQTT is ready.\r\n");
#ifdef FEATURE_SUBSYS_AUDIO_ENABLE
    openPlay(gOpenPlayer, SERVER_SOUND_READY);
#endif

    while(1)
    {
        memset(&sendMessage, 0, sizeof(sendMessage));
        xQueueReceive(mqttSendMsgHandle, &sendMessage, osWaitForever);
        if (sendMessage.cmdType == MQTT_DEMO_MSG_PUBLISH)
        {
            if ((sendMessage.topic != NULL) && (sendMessage.message.payload != NULL))
            {
                memset(mqttSendbuf, 0, MQTT_SEND_BUFF_LEN);
                resVal = MQTTPublish(&mqttClient, sendMessage.topic, &sendMessage.message);
                if(resVal != SUCCESS)
                {
                    SYSLOG_ERR("Publish failed: resVal = %d\r\n", resVal);
                }

                free(sendMessage.topic);
                sendMessage.topic = NULL;

                free(sendMessage.message.payload);
                sendMessage.message.payload = NULL;
            }
        }
    }

labelEnd:
#ifdef FEATURE_SUBSYS_AUDIO_ENABLE
    openPlay(gOpenPlayer, SERVER_SOUND_UNREADY);
#endif
    if (mqttSendbuf != NULL)
    {
        free(mqttSendbuf);
        mqttSendbuf = NULL;
    }
    if (mqttReadbuf != NULL)
    {
        free(mqttReadbuf);
        mqttReadbuf = NULL;
    }
    if (mqttSendMsgHandle != NULL)
    {
        vQueueDelete(mqttSendMsgHandle);
        mqttSendMsgHandle = NULL;
    }
#if 0 // Service Manager
    osThreadExit();
#else
    Service_stop("service:/threadOnenetMqtt");
#endif
}

void onenetMqttInit(void)
{
    osThreadAttr_t threadAttr = {0};

    if (gOnenetMqttThread == NULL)
    {
        memset(&threadAttr, 0, sizeof(threadAttr));
        threadAttr.name       = "threadOnenetMqtt";
        threadAttr.priority   = osPriorityNormal;
        threadAttr.stack_size = THREAD_STACK_SIZE_ONENET_MQTT;
#ifdef THREAD_STATIC
        threadAttr.stack_mem  = gOnenetMqttThreadStackMem;
        threadAttr.cb_mem     = &gOnenetMqttThreadCbMem;
        threadAttr.cb_size    = sizeof(StaticTask_t);
#endif
#if 0
        gOnenetMqttThread = osThreadNew(threadOnenetMqtt, NULL, &threadAttr);
#else
        char serviceName[32] = {0};
        snprintf(serviceName, sizeof(serviceName), "service:/%s", threadAttr.name);
        Service_reg(serviceName, threadOnenetMqtt, NULL, threadAttr.cb_mem, threadAttr.cb_size, threadAttr.stack_mem, threadAttr.stack_size, threadAttr.priority);
        gOnenetMqttThread = (osThreadId_t)Service_start(serviceName);
#endif
        if (gOnenetMqttThread == NULL)
        {
            SYSLOG_EMERG("Failed to create thread for gOnenetMqttThread.\r\n");
        }
    }
}

bool onenetMqttIsReady(void)
{
    return (MQTTIsConnected(&mqttClient) == 1);
}

int32_t onenetMqttPub(char *data)
{
    int32_t     retVal      = -1;
    uint32_t    length      = 0;
    mqttSendMsg sendMessage = {0};

    if ((nwIsReady() != true) && (onenetMqttIsReady() == true))
    {
        SYSLOG_INFO("OneNET MQTT is unready.\r\n");
        MQTTCloseSession(&mqttClient);
        goto labelEnd;
    }

    if (data == NULL)
    {
        SYSLOG_ERR("data is NULL.\r\n");
        goto labelEnd;
    }

    memset(&sendMessage, 0, sizeof(sendMessage));
    length              = strlen(MQTT_SERVER_TOPIC_0);
    sendMessage.cmdType = MQTT_DEMO_MSG_PUBLISH;
    sendMessage.topic   = malloc(length + 1);
    if (sendMessage.topic == NULL)
    {
        SYSLOG_ERR("Failed to malloc %d bytes for sendMessage.topic.\r\n", length + 1);
        goto labelEnd;
    }

    memset(sendMessage.topic, 0,                   length + 1);
    memcpy(sendMessage.topic, MQTT_SERVER_TOPIC_0, length);

    length                         = strlen(data);
    sendMessage.message.qos        = QOS2;
    sendMessage.message.retained   = 0;
    sendMessage.message.id         = 0;
    sendMessage.message.payloadlen = PAYLOAD_PREFIX_LEN + length;
    sendMessage.message.payload    = malloc(sendMessage.message.payloadlen + 1);
    if (sendMessage.message.payload == NULL)
    {
        SYSLOG_ERR("Failed to malloc %d bytes for sendMessage.message.payload.\r\n", sendMessage.message.payloadlen + 1);
        goto labelEnd;
    }

    memset(sendMessage.message.payload, 0, sendMessage.message.payloadlen + 1);
    ((uint8_t *)(sendMessage.message.payload))[0] = PAYLOAD_TYPE;
    ((uint8_t *)(sendMessage.message.payload))[1] = (length >> 8) & 0xFF;
    ((uint8_t *)(sendMessage.message.payload))[2] =  length       & 0xFF;
    memcpy(((uint8_t *)(sendMessage.message.payload)) + PAYLOAD_PREFIX_LEN, data, length);

    if ((mqttSendMsgHandle != NULL) && (xQueueSend(mqttSendMsgHandle, &sendMessage, MQTT_MSG_TIMEOUT) == pdPASS))
    {
        retVal = 0;
    }

labelEnd:
    if (retVal != 0)
    {
        if (sendMessage.topic != NULL)
        {
            free(sendMessage.topic);
            sendMessage.topic = NULL;
        }
        if (sendMessage.message.payload != NULL)
        {
            free(sendMessage.message.payload);
            sendMessage.message.payload = NULL;
        }
    }
    return retVal;
}

int32_t onenetMqttSub(char *topic)
{
    char clientTopic[CLIENT_TOPIC_LEN_MAX] = {0};

    snprintf(clientTopic, sizeof(clientTopic), "/%s/%s", gDevInfo.clientId, topic);
    return MQTTSubscribe(&mqttClient, clientTopic, QOS2, defaultMessageHandler);
}

int32_t onenetMqttUnsub(char *topic)
{
    char clientTopic[CLIENT_TOPIC_LEN_MAX] = {0};

    snprintf(clientTopic, sizeof(clientTopic), "/%s/%s", gDevInfo.clientId, topic);
    return MQTTUnsubscribe(&mqttClient, clientTopic);
}

int32_t onenetMqttDevInfoWrite(OnenetMqttDevInfoT *info)
{
    int32_t     retVal                 = -1;
    char        json[JSON_LEN_MAX + 1] = {0};
    FILE       *file                   = NULL;
    struct stat buf                    = {0};
    bool        opened                 = false;
    uint32_t    size                   = 0;
    char       *buffer                 = NULL;

    if (info == NULL)
    {
        SYSLOG_ERR("info is NULL\r\n");
        goto labelEnd;
    }

    snprintf(json, JSON_LEN_MAX + 1, "{\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\"}",
             ITEM_CLIENT_ID, info->clientId,
             ITEM_USER_NAME, info->userName,
             ITEM_PASSWORD,  info->password);

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    file = file_fopen(ONENET_MQTT_DEV_INFO, "w+");
#endif
    if (file == NULL)
    {
        SYSLOG_ERR("Failed to open the file \"%s\".\r\n", ONENET_MQTT_DEV_INFO);
        goto labelEnd;
    }

    opened = true;
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    file_fstat((int)file, &buf);
    size = buf.st_size;
#endif
    if (size < 0)
    {
        SYSLOG_ERR("Failed to get size of the file \"%s\".\r\n", ONENET_MQTT_DEV_INFO);
        goto labelEnd;
    }

    if (size > 0)
    {
        buffer = malloc(size + 1);
        if (buffer == NULL)
        {
            SYSLOG_ERR("Failed to malloc %d bytes for buffer.\r\n", size + 1);
            goto labelEnd;
        }

        memset(buffer, 0, size + 1);
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
        file_fread(buffer, size, 1, file);
#endif
        if ((strlen(json) == size) && (memcmp(json, buffer, size) == 0))
        {
            goto labelEnd;
        }
        else
        {
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
            file_truncate((int)file, 0);
#endif
        }
    }

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    file_rewind(file);
    file_fwrite(json, strlen(json), 1, file);
#endif

    retVal = 0;

labelEnd:
    if (opened == true)
    {
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
        file_fclose(file);
#endif
    }
    if (buffer != NULL)
    {
        free(buffer);
        buffer = NULL;
    }
    return retVal;
}

void *onenetMqttDevInfoRead(uint8_t type)
{
    FILE               *file   = NULL;
    struct stat         buf    = {0};
    uint32_t            size   = 0;
    char               *buffer = NULL;
    cJSON              *root   = NULL;
    cJSON              *node   = NULL;
    OnenetMqttDevInfoT *info   = NULL;

    if (type >= DEV_INFO_RETURN_INVALID)
    {
        SYSLOG_ERR("type invalied: %d\r\n", type);
        goto labelEnd;
    }

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    file = file_fopen(ONENET_MQTT_DEV_INFO, "r");
#endif
    if (file == NULL)
    {
        SYSLOG_ERR("Failed to open the file \"%s\".\r\n", ONENET_MQTT_DEV_INFO);
        goto labelEnd;
    }

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    file_fstat((int)file, &buf);
    size = buf.st_size;
#endif
    if (size <= 0)
    {
        SYSLOG_ERR("Failed to get size of the file \"%s\" or the file \"%s\" is empty.\r\n", ONENET_MQTT_DEV_INFO, ONENET_MQTT_DEV_INFO);
        goto labelEnd;
    }

    buffer = malloc(size + 1);
    if (buffer == NULL)
    {
        SYSLOG_ERR("Failed to malloc %d bytes for buffer.\r\n", size + 1);
        goto labelEnd;
    }

    memset(buffer, 0, size + 1);
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    file_fread(buffer, size, 1, file);
    file_fclose(file);
#endif

    if (type == DEV_INFO_RETURN_STRING)
    {
        goto labelEnd;
    }

    root = cJSON_Parse(buffer);
    if (root == NULL)
    {
        SYSLOG_ERR("Failed to parse the JSON string.\r\n");
        goto labelEnd;
    }

    node = cJSON_GetObjectItem(root, ITEM_CLIENT_ID);
    if ((node == NULL)
     || (node->type != cJSON_String)
     || (strlen(node->valuestring) == 0)
     || (strlen(node->valuestring) > ITEM_LEN_MAX))
    {
        SYSLOG_ERR("The item \"%s\" is incorrect.\r\n", ITEM_CLIENT_ID);
        goto labelEnd;
    }

    size = sizeof(OnenetMqttDevInfoT);
    info = malloc(size + 1);
    if (info == NULL)
    {
        SYSLOG_ERR("Failed to malloc %d bytes for info.\r\n", size + 1);
        goto labelEnd;
    }

    memset(info, 0, size + 1);
    memcpy(info->clientId, node->valuestring, strlen(node->valuestring));

    node = cJSON_GetObjectItem(root, ITEM_USER_NAME);
    if ((node == NULL)
     || (node->type != cJSON_String)
     || (strlen(node->valuestring) == 0)
     || (strlen(node->valuestring) > ITEM_LEN_MAX))
    {
        SYSLOG_ERR("The item \"%s\" is incorrect.\r\n", ITEM_USER_NAME);
        goto labelEnd;
    }

    memcpy(info->userName, node->valuestring, strlen(node->valuestring));

    node = cJSON_GetObjectItem(root, ITEM_PASSWORD);
    if ((node == NULL)
     || (node->type != cJSON_String)
     || (strlen(node->valuestring) == 0)
     || (strlen(node->valuestring) > ITEM_LEN_MAX))
    {
        SYSLOG_ERR("The item \"%s\" is incorrect.\r\n", ITEM_PASSWORD);
        goto labelEnd;
    }

    memcpy(info->password, node->valuestring, strlen(node->valuestring));

labelEnd:
    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    if (type == DEV_INFO_RETURN_STRING)
    {
        return buffer;
    }
    else if (type == DEV_INFO_RETURN_STRUCT)
    {
        if (buffer != NULL)
        {
            free(buffer);
            buffer = NULL;
        }
        return info;
    }
    else
    {
        return NULL;
    }
}
#endif
