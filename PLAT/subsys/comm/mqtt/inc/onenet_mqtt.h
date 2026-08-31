/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    test.h
 * Description:  EC618 mqtt demo entry header file
 * History:      Rev1.0   2018-07-12
 *
 ****************************************************************************/
#ifndef __ONENET_MQTT_H__
#define __ONENET_MQTT_H__


#include <stdint.h>


#define ONENET_MQTT_TOPIC_SPEAKER   "speaker"
#ifdef EXTERNAL_TTS_LFS_ENABLE
#define ONENET_MQTT_DEV_INFO        "D:/onenetMqttDev.info"
#else
#define ONENET_MQTT_DEV_INFO        "C:/onenetMqttDev.info"
#endif
#define ITEM_LEN_MAX                32


typedef enum
{
    DEV_INFO_RETURN_STRING = 0,
    DEV_INFO_RETURN_STRUCT = 1,
    DEV_INFO_RETURN_INVALID,
} DevInfoReturnT;

typedef struct
{
    char clientId[ITEM_LEN_MAX + 1];
    char userName[ITEM_LEN_MAX + 1];
    char password[ITEM_LEN_MAX + 1];
} OnenetMqttDevInfoT;


void     onenetMqttInit(void);
bool     onenetMqttIsReady(void);
int32_t  onenetMqttPub(char *data);
int32_t  onenetMqttSub(char *topic);
int32_t  onenetMqttUnsub(char *topic);
int32_t  onenetMqttDevInfoWrite(OnenetMqttDevInfoT *info);
void    *onenetMqttDevInfoRead(uint8_t type);


#endif
