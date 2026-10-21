#ifndef _CTW_TCP_H
#define _CTW_TCP_H

#include "ctw_common.h"

typedef enum
{
	LOGIN_MESSAGE = 0x01,					//!<注册报文标识符
	UPSTREAM_MESSAGE = 0x02,			//!<上行数据报文标识符
	DOWNSTREAM_MESSAGE = 0x03,		//!<下行数据报文标识符
	PING_MESSAGE = 0x04,					//!<心跳报文标识符
	LOGIN_ACK = 0x05,							//!<注册ACK报文标识符
	HEARTBEAT_ACK = 0x06,					//!<心跳ACK报文标识符
}message_type_e;

typedef struct MessagePack
{
	uint16_t msgLen;						//!<数据长度
	uint8_t* message;					//!<数据
}message_pack_t;

typedef struct{
    char iccid[CTW_CCID_LEN+1];
    char imsi[CTW_IMSI_LEN+1];
    char imei[CTW_IMEI_LEN+1];
    INT16 rsrp;
    INT8  snr;
    INT8  txPower;
    uint16_t phyCellId;
}ctwTcpRegParam_t;

typedef struct
{
    int32_t socket;
    uint32_t lastSendExpireTime;
    uint32_t lastRecvExpireTime;
    uint32_t pingAckExpireTime;
    uint8_t waitPingAck; //0:not waiting; 1:in waiting ping ack
    uint8_t regStatus;   //0:no login; 1: logined; 2:logging
    void (*messageHandlerCb)(message_pack_t*);
    uint32_t athandle;
} ctwTcpContext_t;

bool ctwTimeExpired(uint32_t time);
bool ctwTcpSendPacket(ctwTcpContext_t* pContext, uint8_t* buf, int len);
int32_t ctwTcpReadPacket(ctwTcpContext_t* pContext);
bool ctwTcpLogin(MWNvmCfgCtwTcpParam* pCtwTcpParam, ctwTcpContext_t* pContext);
bool ctwTcpUpdata(ctwTcpContext_t* pContext, uint8_t* buf, uint16_t len);


#endif
