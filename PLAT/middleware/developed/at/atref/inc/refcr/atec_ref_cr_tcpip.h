/******************************************************************************

*(C) Copyright 2018 EIGENCOMM International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: atec_ref_cr_tcpip.h
*
*  Description: Process TCP/IP related AT CMD
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef _ATEC_REF_CR_TCP_IP_H_
#define _ATEC_REF_CR_TCP_IP_H_

#include "atc_decoder.h"

#define MAX_REF_CR_URL_IPADDR_LEN                    255
#define MAX_REF_CR_IPADDR_LEN                        64

#define REF_CR_MAX_CID    15

#define REF_CR_MAX_CONNECTION_ID    5
#define REF_CR_MAX_CONNECTION_NUM   (REF_CR_MAX_CONNECTION_ID + 1)

#define REF_CR_MIN_SEND_TIMEOUT  1
#define REF_CR_MAX_SEND_TIMEOUT  180
#define REF_CR_DFT_SEND_TIMEOUT  10

#define REF_CR_MIN_SEND_BUF_SIZE 1
#define REF_CR_MAX_SEND_BUF_SIZE 8192
#define REF_CR_DFT_SEND_BUF_SIZE 1460

#define REF_CR_MIN_RECV_BUF_SIZE 1
#define REF_CR_MAX_RECV_BUF_SIZE 65535
#define REF_CR_DFT_RECV_BUF_SIZE 65535

#define REF_CR_MIN_HEARTBEAT_COUNT 1
#define REF_CR_MAX_HEARTBEAT_COUNT 9
#define REF_CR_DFT_HEARTBEAT_COUNT 3

#define REF_CR_MIN_HEARTBEAT_IDLE 30
#define REF_CR_MAX_HEARTBEAT_IDEL 7200
#define REF_CR_DFT_HEARTBEAT_IDEL 90

#define REF_CR_MIN_HEARTBEAT_INTERVAL 30
#define REF_CR_MAX_HEARTBEAT_INTERVAL 600
#define REF_CR_DFT_HEARTBEAT_INTERVAL 75

#define MAX_REF_CR_URL_IPADDR_LEN                    255

/* AT+MIOPEN*/
#define MIPOPEN_0_CONNECT_ID_MIN                  0
#define MIPOPEN_0_CONNECT_ID_MAX                  REF_CR_MAX_CONNECTION_ID
#define MIPOPEN_0_CONNECT_ID_DEF                  0
#define MIPOPEN_1_TYPE_STR_MAX_LEN                3
#define MIPOPEN_1_TYPE_STR_DEF                    NULL
#define MIPOPEN_2_REMOTE_ADDR_STR_MAX_LEN         MAX_REF_CR_URL_IPADDR_LEN
#define MIPOPEN_2_REMOTE_ADDR_STR_DEF             NULL
#define MIPOPEN_3_REMOTE_PORT_MIN                 0
#define MIPOPEN_3_REMOTE_PORT_MAX                 65535
#define MIPOPEN_3_REMOTE_PORT_DEF                 0
#define MIPOPEN_4_TIMEOUT_MIN                     1
#define MIPOPEN_4_TIMEOUT_MAX                     180
#define MIPOPEN_4_TIMEOUT_DEF                     60
#define MIPOPEN_5_ACCESS_MODE_MIN                 0
#define MIPOPEN_5_ACCESS_MODE_MAX                 3
#define MIPOPEN_5_ACCESS_MODE_DEF                 0
#define MIPOPEN_6_LOCAL_PORT_MIN                  0
#define MIPOPEN_6_LOCAL_PORT_MAX                  10096
#define MIPOPEN_6_LOCAL_PORT_DEF                  0

/*AT+MIPSEND parameters defined*/
#define AT_MIPSEND_0_CONNECTID_MIN               (0)
#define AT_MIPSEND_0_CONNECTID_MAX               (REF_CR_MAX_CONNECTION_ID)
#define AT_MIPSEND_0_CONNECTID_DEF               (-1)
#define AT_MIPSEND_1_SENDLEN_MIN                 (0)
#define AT_MIPSEND_1_SENDLEN_MAX                 (1460)
#define AT_MIPSEND_1_SENDLEN_DEF                 (0)
#define AT_MIPSEND_DATAMODE_SENDLEN_MAX          (8192)

/*AT+MIPMODE parameters defined*/
#define AT_MIPMODE_0_CONNECTID_MIN          (0)
#define AT_MIPMODE_0_CONNECTID_MAX          (REF_CR_MAX_CONNECTION_ID)
#define AT_MIPMODE_0_CONNECTID_DEF          (-1)
#define AT_MIPMODE_1_ACCMODE_MIN            (0)
#define AT_MIPMODE_1_ACCMODE_MAX            (3)
#define AT_MIPMODE_1_ACCMODE_DEF            (0)


/* AT+MIPCLOSE*/
#define MIPCLOSE_0_CONNECT_ID_MIN                 0
#define MIPCLOSE_0_CONNECT_ID_MAX                 REF_CR_MAX_CONNECTION_ID
#define MIPCLOSE_0_CONNECT_ID_DEF                 0


#define MIPCFG_0_PARAM_STR_MAX       16
#define MIPCFG_0_PARAM_STR_DEF       NULL

#define MIPCFG_1_CONNECT_ID_MIN      0
#define MIPCFG_1_CONNECT_ID_MAX      REF_CR_MAX_CONNECTION_ID
#define MIPCFG_1_CONNECT_ID_DEF      0

#define MIPCFG_CID_MIN               1
#define MIPCFG_CID_MAX               REF_CR_MAX_CID
#define MIPCFG_CID_DEF               1

#define MIPCFG_ENCODING_SEND_FMT_MIN 0
#define MIPCFG_ENCODING_SEND_FMT_MAX 2
#define MIPCFG_ENCODING_SEND_FMT_DEF 0

#define MIPCFG_ENCODING_RECV_FMT_MIN 0
#define MIPCFG_ENCODING_RECV_FMT_MAX 1
#define MIPCFG_ENCODING_RECV_FMT_DEF 0

#define MIPCFG_SEND_TIMEOUT_MIN      1
#define MIPCFG_SEND_TIMEOUT_MAX      120
#define MIPCFG_SEND_TIMEOUT_DEF      10

#define MIPCFG_AUTOFREE_MIN          0
#define MIPCFG_AUTOFREE_MAX          1
#define MIPCFG_AUTOFREE_DEF          0

#if 0
#define MIPCFG_SENDBUF_MIN           1
#define MIPCFG_SENDBUF_MAX           8192
#define MIPCFG_SENDBUF_DEF           1460
#endif

#define MIPCFG_RECVBUF_MIN           1460
#define MIPCFG_RECVBUF_MAX           65535
#define MIPCFG_RECVBUF_DEF           65535

#define MIPCFG_ACKMODE_MIN           0
#define MIPCFG_ACKMODE_MAX           1
#define MIPCFG_ACKMODE_DEF           0

#define MIPCFG_SSL_ENABLE_MIN           0
#define MIPCFG_SSL_ENABLE_MAX           1
#define MIPCFG_SSL_ENABLE_DEF           0

#define MIPCFG_SSL_ID_MIN           0
#define MIPCFG_SSL_ID_MAX           5
#define MIPCFG_SSL_ID_DEF           0

#define MIPTKA_0_CONNECT_ID_MIN         0
#define MIPTKA_0_CONNECT_ID_MAX         REF_CR_MAX_CONNECTION_ID
#define MIPTKA_0_CONNECT_ID_DEF         0

#define MIPTKA_1_KEEPALIVE_ENABLE_MIN   0
#define MIPTKA_1_KEEPALIVE_ENABLE_MAX   1
#define MIPTKA_1_KEEPALIVE_ENABLE_DEF   0

#define MIPTKA_2_KEEP_IDLE_MIN          30
#define MIPTKA_2_KEEP_IDLE_MAX          7200
#define MIPTKA_2_KEEP_IDLE_DEF          90

#define MIPTKA_3_KEEP_INTERVAL_MIN      30
#define MIPTKA_3_KEEP_INTERVAL_MAX      600
#define MIPTKA_3_KEEP_INTERVAL_DEF      75

#define MIPTKA_4_KEEP_COUNT_MIN         1
#define MIPTKA_4_KEEP_COUNT_MAX         9
#define MIPTKA_4_KEEP_COUNT_DEF         3

#define MIPSTATE_0_CONNECT_ID_MIN         0
#define MIPSTATE_0_CONNECT_ID_MAX         REF_CR_MAX_CONNECTION_ID
#define MIPSTATE_0_CONNECT_ID_DEF         0


#define MIPRD_0_CONNECT_ID_MIN         0
#define MIPRD_0_CONNECT_ID_MAX         REF_CR_MAX_CONNECTION_ID
#define MIPRD_0_CONNECT_ID_DEF         0

#define MIPRD_1_READ_LEN_MIN         0
#define MIPRD_1_READ_LEN_MAX         4096
#define MIPRD_1_READ_LEN_DEF         0

//AT+MDNSCFG
#define MDNCFG_0_PARAM_STR_MAX       16
#define MDNCFG_0_PARAM_STR_DEF       NULL
#define MDNCFG_1_PRIORITY_MIN         0
#define MDNCFG_1_PRIORITY_MAX         1
#define MDNCFG_1_PRIORITY_DEF         1

/* AT+MDNSGIP */
#define MDNSGIP_0_STR_MAX_LEN              MAX_REF_CR_URL_IPADDR_LEN
#define MDNSGIP_0_STR_DEF                  NULL
#define MDNSGIP_1_CID_MIN                  1
#define MDNSGIP_1_CID_MAX                  REF_CR_MAX_CID
#define MDNSGIP_1_CID_DEF                  1

/* AT+MPING */
#define MPING_0_STR_MAX_LEN              MAX_REF_CR_URL_IPADDR_LEN
#define MPING_0_STR_DEF                  NULL
#define MPING_1_TIMEOUT_MIN              1
#define MPING_1_TIMEOUT_MAX              60
#define MPING_1_TIMEOUT_DEF              10
#define MPING_2_PING_NUM_MIN             1
#define MPING_2_PING_NUM_MAX             65535
#define MPING_2_PING_NUM_DEF             4
#define MPING_3_PING_PAYLOAD_LEN_MIN     1
#define MPING_3_PING_PAYLOAD_LEN_MAX     1400
#define MPING_3_PING_PAYLOAD_LEN_DEF     16
#define MPING_4_CID_MIN                  1
#define MPING_4_CID_MAX                  REF_CR_MAX_CID
#define MPING_4_CID_DEF                  1

/*AT+MNTP*/
#define MNTP_0_STR_MAX_LEN              MAX_REF_CR_URL_IPADDR_LEN
#define MNTP_0_STR_DEF                  "ntp1.aliyun.com"
#define MNTP_PORT_1_MIN                  0
#define MNTP_PORT_1_MAX                  AT_SOC_PORT_MAX
#define MNTP_PORT_1_DEF                  123
#define MNTP_SYNC_2_MIN                  0
#define MNTP_SYNC_2_MAX                  1
#define MNTP_SYNC_2_DEF                  1
#define MNTP_TIMEOUT_3_MIN               1
#define MNTP_TIMEOUT_3_MAX               300
#define MNTP_TIMEOUT_3_DEF               20

/* AT+MDIALUPCFG */
#define MDIALUPCFG_0_MAX_PARM_STR_LEN         32
#define MDIALUPCFG_0_MAX_PARM_STR_DEFAULT     NULL
#define MDIALUPCFG_1_MODE_MIN      0
#define MDIALUPCFG_1_MODE_MAX      1
#define MDIALUPCFG_1_MODE_DEF      0
#define MDIALUPCFG_1_AUTO_MIN      0
#define MDIALUPCFG_1_AUTO_MAX      1
#define MDIALUPCFG_1_AUTO_DEF      1
#define MDIALUPCFG_1_DHCPV4_MIN    1
#define MDIALUPCFG_1_DHCPV4_MAX    1
#define MDIALUPCFG_1_DHCPV4_DEF    1
#define MDIALUPCFG_2_DHCPV4_GATEWAY_STR_LEN        MAX_REF_CR_IPADDR_LEN
#define MDIALUPCFG_2_DHCPV4_GATEWAY_STR_DEFAULT     NULL
#define MDIALUPCFG_3_DHCPV4_START_MIN    0
#define MDIALUPCFG_3_DHCPV4_START_MAX    255
#define MDIALUPCFG_3_DHCPV4_START_DEF    100
#define MDIALUPCFG_4_DHCPV4_END_MIN    0
#define MDIALUPCFG_4_DHCPV4_END_MAX    255
#define MDIALUPCFG_4_DHCPV4_END_DEF    100
#define MDIALUPCFG_5_DHCPV4_LEASE_MIN    3600
#define MDIALUPCFG_5_DHCPV4_LEASE_MAX    86400
#define MDIALUPCFG_5_DHCPV4_LEASE_DEF    3600
#define MDIALUPCFG_1_CID_MIN    1
#define MDIALUPCFG_1_CID_MAX    REF_CR_MAX_CID
#define MDIALUPCFG_1_CID_DEF    1

/* AT+MDIALUP */
#define MDIALUP_0_CID_MIN             1
#define MDIALUP_0_CID_MAX             REF_CR_MAX_CID
#define MDIALUP_0_CID_DEF             1
#define MDIALUP_1_OP_MIN              0
#define MDIALUP_1_OP_MAX              1
#define MDIALUP_1_OP_DEF              1


typedef enum RefCrSockCfgItem_Tag
{
    REF_CR_SOCK_CFG_CID = 0,
    REF_CR_SOCK_CFG_ENCODING,
    REF_CR_SOCK_CFG_TIMEOUT,
    REF_CR_SOCK_CFG_SENDBUF,
    REF_CR_SOCK_CFG_RECVBUF,
    REF_CR_SOCK_CFG_AUTOFREE,
    REF_CR_SOCK_CFG_ACKMODE,
    REF_CR_SOCK_CFG_SSL
}RefCrSockCfgItem;

typedef enum RefCrSockAckMode_Tag
{
    REF_CR_SOCK_ACK_MODE_NO_URC = 0,
    REF_CR_SOCK_ACK_MODE_URC = 1
}RefCrSockAckMode;

typedef enum RefCrSockSendFmt_Tag
{
    REF_CR_SOCK_SEND_FMT_ASCII = 0,
    REF_CR_SOCK_SEND_FMT_HEX_STRING = 1,
    REF_CR_SOCK_SEND_FMT_ESCAPE_STRING = 2,
}RefCrSockSendFmt;

typedef enum RefCrSockRecvFmt_Tag
{
    REF_CR_SOCK_RECV_FMT_ASCII = 0,
    REF_CR_SOCK_RECV_FMT_HEX_STRING = 1,
}RefCrSockRecvFmt;


typedef struct RefCrSocketCfg_Tag
{
    UINT32 connectionId :3;   /* 0 ~ 5 */
    UINT32 cid          :8;   /* 1 ~ 15 */
    UINT32 sendTimeout  :8;   /* 1 ~ 180, default: 10 */
    UINT32 isAutoFree   :1;   /* 0 (not need MIPCLOSE to free resource) or 1, default: 0 */
    UINT32 sendFmt      :2;   /* 0 (ASCII), 1(Hex string), 2(escape string), default: 0 */
    UINT32 recvFmt      :1;   /* 0 (ASCII), 1(Hex string), default: 0 */
    UINT32 ackmode      :1;   /* 0 (no urc when receive tcp ack) or 1, default: 0, RefCrSockAckMode*/
    UINT32 sslEnable    :1;   /* 0 ~ 1 */
    UINT32 sslId        :3;   /* 0 ~ 5 */
    UINT32 rsv          :4;

    UINT16 sendBufSize;    /* 1 ~ 8192 , default: 1460  */
    UINT16 rcvBufSize;     /* 1460 ~ 65535, default: 65535 */
}RefCrSocketCfg;//8byte

typedef struct RefCrTcpHeartBeatCfg_Tag
{
    UINT8 connectionId;            /* 0 ~ 5 */
    UINT8 isKeepaliveEnable : 1;   /* 0 (disable), 1(enable), default:0 */
    UINT8 keepCount         : 4;   /* 1-9,  default: 3 */
    UINT8 rsv               : 3;
    UINT16 resv;

    UINT16 keepIdle;               /* 30-7200, default: 90 */
    UINT16 keepInterval;           /* 30-600,  default: 75 */
}RefCrTcpHeartBeatCfg;//8byte

typedef struct RefCrCfg_Tag
{
    RefCrSocketCfg refCrSocketCfg[REF_CR_MAX_CONNECTION_NUM];  //8*6=48bytes
    RefCrTcpHeartBeatCfg refCrTcpHeartBeatCfg[REF_CR_MAX_CONNECTION_NUM]; //8*6=48bytes
}RefCrCfg;//96byte


void refCrSocProcRefCrTimerExpire(UINT16 channelId);
void refCrSocPSTHdataInit(UINT8 connectId,UINT16 reqHander);
CmsRetId refCrSocInputPSTHData(UINT8 chanId, UINT8 *pInput, UINT16 length);
CmsRetId refCrSocInputData(UINT8 chanId, UINT8 *pInput, UINT16 length);
CmsRetId  refCrMIPOPEN(const AtCmdInputContext *pAtCmdReq);
CmsRetId  refCrMIPCFG(const AtCmdInputContext *pAtCmdReq);
CmsRetId  refCrMIPSEND(const AtCmdInputContext *pAtCmdReq);
CmsRetId  refCrMIPMODE(const AtCmdInputContext *pAtCmdReq);
CmsRetId  refCrMIPCLOSE(const AtCmdInputContext *pAtCmdReq);
CmsRetId  refCrMIPTKA(const AtCmdInputContext *pAtCmdReq);
CmsRetId  refCrMIPSTATE(const AtCmdInputContext *pAtCmdReq);
CmsRetId  refCrMIPSACK(const AtCmdInputContext *pAtCmdReq);
CmsRetId  refCrMIPRD(const AtCmdInputContext *pAtCmdReq);
CmsRetId  refCrDNSCFG(const AtCmdInputContext *pAtCmdReq);
CmsRetId  refCrDNSGIP(const AtCmdInputContext *pAtCmdReq);
CmsRetId  refCrSNTP(const AtCmdInputContext *pAtCmdReq);
CmsRetId  refCrMPING(const AtCmdInputContext *pAtCmdReq);
CmsRetId  refCrMDIALUPCFG(const AtCmdInputContext *pAtCmdReq);
CmsRetId  refCrMDIALUP(const AtCmdInputContext *pAtCmdReq);


#endif
