/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    cucc_dm.h
 * Description:  EC618 at command demo entry header file
 * History:      Rev1.0   2018-07-12
 *
 ****************************************************************************/
 
#ifndef _DM_CUCC_H
#define _DM_CUCC_H

#include "slpman.h"
#include "dm_task.h"

#define AR_RETRY_TIME_MAX        3
#define AR_RETRY_TIME_OUT        10 /*secs*/

#define AR_REGISTER_TIME_OUT        1
#define AR_REGISTER_TIME_WAIT       0

#define CUCC_HAS_REGISTER           1
#define CUCC_NOT_REGISTER           0

#define CUCC_AUTO_REG_PERIOD_DISABLE           0
#define CUCC_AUTO_REG_PERIOD_TYPE1             (-1)
#define CUCC_AUTO_REG_PERIOD_TYPE2             (-2)
#define CUCC_AUTO_REG_PERIOD_TYPE3             (1)

#define AR_RESEND_COUNT_MAX        5
#define CUCC_AUTO_REG_TASK_STACK_SIZE   4096 
#define CUCC_MQTT_SERVER_PORT            1883
#define CUCC_MQTT_SERVER_URL             "dmp-mqtt.cuiot.cn"

#define DM_MSG_NETWORK_READY   1 
#define DM_MSG_SIM_READY       2 
#define DM_MSG_MAX_NUMB        6 
#define DM_MSG_TIMEOUT         1000
#define DM_MSG_RECV_TIMEOUT    4000

#define CUCC_MQTT_SEND_BUFF_LEN   2048
#define CUCC_MQTT_RECV_BUFF_LEN   2048

#define CUCC_DM_TOPIC_LEN   72
#define CUCC_MQTT_SEND_PAYLOAD_LEN   3000
#define CUCC_DM_UTC_TIME_LEN   32
#define CUCC_DM_APN_LEN            100
#define CUCC_DM_MSG_ID_LEN   32

typedef struct
{
    INT32 cmdType;
    UINT8 imsi[IMSI_LEN+1];
}dmInterMsg;

void cuccAutoRegPreInitTask(void *argument);
void cuccAutoRegisterInit(void *argument);
#endif
