#ifndef __MAIN_TCPIP_H__
#define __MAIN_TCPIP_H__
#include "sockets.h"

/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/

#define 	THREAD_STACK_SIZE_TCPIP     		(10 * 1024)
#define 	THREAD_STACK_SIZE_TCPIP_STA     	(1 * 1024)

#define 	OPEN_TCPIP_RX_BUF_SIZE				(1500)

#define 	QUEUE_SIZE_TCPIP					(100)
#define 	OPEN_TCPIP_SOCKET_TYPE_TCPC			(0)
#define 	OPEN_TCPIP_SOCKET_TYPE_TCPS			(1)
#define 	OPEN_TCPIP_SOCKET_TYPE_UDPC			(2)
#define 	OPEN_TCPIP_SOCKET_TYPE_MAX			(3)
#define 	OPEN_TCPIP_SOCKET_RXTIMEOUT_MAX	    (3*60)//3 min

#define		OPEN_TCPIP_SOCKET_RXBUF_SIZE_MAX 	(64*1024)
#define		OPEN_TCPIP_SOCKET_PORT_INDEX_MAX 	(65535)

#define  	OPEN_TCPIP_SOCKLET_ID_MAX 		(6)

typedef void (* tcpipSocketActionCb)(uint8_t socketId, uint8_t action, int8_t result,const void *extra);
typedef void (* tcpipSocketStateCb)(uint8_t socketId, int32_t value);

typedef int32_t (* openSocketStateCallBackFunc)(void * param);
typedef int32_t (* openSocketStateCallRelease)(void * param);

typedef enum _socketState_e{
	E_SOCKET_STATE_IDLE,
	E_SOCKET_STATE_SET,
	E_SOCKET_STATE_CONNECT,
	E_SOCKET_STATE_ACTIVE,
	E_SOCKET_STATE_SENDING,
	E_SOCKET_STATE_CLOSING,
}socketState_e;

typedef enum _socketAction_e{
	E_SOCKET_ACTION_INIT,
	E_SOCKET_ACTION_OPEN,
	E_SOCKET_ACTION_SEND,
	E_SOCKET_ACTION_READ,
	E_SOCKET_ACTION_CLOSE,
	E_SOCKET_ACTION_MAX,
}socketAction_e;

typedef enum _socketRet_e{
	E_SOCKET_OK,
	E_SOCKET_ERR_NETWORK_NO_ACTIVE 	= -1,
	E_SOCKET_ERR_SIM_FAIL		   	= -2,
	E_SOCKET_ERR_CONNET    		   	= -3,
	E_SOCKET_ERR_NO_CONNET 			= -4,
	E_SOCKET_ERR_TIMEOUT		   	= -5,
	E_SOCKET_ERR_BUFFER_FULL	   	= -6,
	E_SOCKET_ERR_CONNECT_REJECT	   	= -7,
	E_SOCKET_ERR_CONNECT_TIMEOUT   	= -8,
	E_SOCKET_ERR_DNS_FAIL		   	= -9,
	E_SOCKET_ERR_RARAM				= -10,
	E_SOCKET_ERR_DEINIT				= -11,
	E_SOCKET_ERR_GET_ADDR_INFO_FAIL = -12,
	E_SOCKET_ERR_SOCKET_FAIL		= -13,
	E_SOCKET_ERR_SOCKET_SND_FAIL	= -14,
	E_SOCKET_ERR_NET_CONN_RESET		= -15,
	E_SOCKET_ERR_RECV_FAIL			= -16,
	E_SOCKET_ERR_EWOULDBLOCK 		= -17,
	E_SOCKET_ERR_MAX			   	= -127
}socketRet_e;

typedef struct _openSocketNode_t
{
	int32_t               param;
	openSocketStateCallBackFunc   Function;
	openSocketStateCallRelease    Release;
	struct _openSocketNode_t 	*next;
}openSocketNode_t;


typedef struct _socketConfig_t{
	uint8_t					socketType;
	uint8_t					revTimeOut;
	uint16_t				port;
	uint32_t				rxBufSize;			/*the size of receive buffer*/
	int8_t *				hostUrl;
	tcpipSocketActionCb 	saCb;
	tcpipSocketStateCb 		ssCb;
}socketConfig_t;

typedef struct _socketHandler_t {
	uint8_t					socketState;
	uint8_t					urcFlag;
	int8_t					sockfd;
	uint16_t				localPort;			/*the port of local-host*/
	uint16_t				peerPort;			/*the port of remote-host*/	
	uint32_t				cid;				/*channel id */				
	uint32_t				rxDataLen;			/*the length of receive*/
	int32_t               	ai_family;
	ip_addr_t				remoteIp;			/*the ip address of local-host*/
	uint8_t *				localIp;			/*the ip address of remote-host*/	
	int8_t *				rxBuff;				/*pointer of receive buffer*/
	socketConfig_t			appConfigInfo;
	openSocketNode_t 		*stateNode;
}socketHandler_t;

typedef struct _queueTcpipSocket_t{
	uint8_t 				socketId;
	uint8_t 				action;
	union {
	socketConfig_t			config;
	int8_t					*txData;
	uint32_t				readLen;
	}value;		
}queueTcpipSocket_t;


/*----------------------------------------------------------------------------*
 *					  GLOBAL FUNCTIONS DECLEARATION 						  *
 *----------------------------------------------------------------------------*/
int 	openSocketTaskinit(void);
int 	openSocketClientTracker(uint8_t socketId, uint8_t action, void *value);
int8_t 	openSocketConnect(uint8_t socketId);
int8_t 	openSocketClose(uint8_t socketId);
int32_t openSocketSend(uint8_t socketId, int8_t * sndbuffer, uint32_t sndLength);
int32_t openSocketRead(uint8_t socketId, int8_t *buffer,uint32_t rdLength);

#endif/*__MAIN_TCPIP_H__*/
