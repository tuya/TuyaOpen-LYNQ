/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    open_websocket.h
 * Description:  EC718 websocket demo entry header file
 * History:      Rev1.0   2025-05-08
 ****************************************************************************/
#ifndef __OPEN_WEBSOCKET_H__
#define __OPEN_WEBSOCKET_H__
#include <stdbool.h>

/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/
#define 	THREAD_STACK_SIZE_WSS_RECV_TASK    		(10 * 1024)
#define 	THREAD_STACK_SIZE_WSS_ENG_TASK      	(10 * 1024)
#define 	THREAD_STACK_SIZE_WSS_POLLING_TASK     	(1 * 1024)

#define		OPEN_WSS_CLIENT_NUM_MAX					(6) 			//max client num,can be changed
#define 	OPEN_WSS_URL_LEN_MAX					(2048)

#define		OPEN_WSS_RSP_HEAD_BUFFER_SIZE 			(1501)
#define 	OPEN_WSS_RSP_CONTENT_BUFFER_SIZE  		(1501)

#define		OPEN_WSS_KEY_LEN						(24)			//the max key length,can not be change
#define		OPEN_WSS_HEAD_MAX_LEN					(1024)
#define 	OPEN_WSS_VERSION						(13)			//websocket version

//#define		OPEN_WSS_DUMP_ENABLE								//switch of the wss dump info

/**
@brief websocket operate callback
@param socketId,the client index
@actionType refers to openWssAction_e
@result operation result
@Note: !!!do not perform time-consuming operations in callbacks!!!
**/
typedef void (* openWssActionCb)(int8_t socketId,int8_t actionType,int8_t result);
/**
@brief websocket recv data callback
@param socketId,the client index
@frameType websocket frame type 
@data received data
@length received data length
@Note: !!!do not perform time-consuming operations in callbacks!!!
**/
typedef void (* openWssOnMessage)(int8_t socketId,int8_t frameType,int8_t *data,uint32_t length);

typedef enum _openWssRet_e
{
	E_WSS_OK = 0,        	    							///<Success
    E_WSS_ERR_PARSE,         								///1<url Parse error
    E_WSS_ERR_DNS,           								///2<Could not resolve name
    E_WSS_ERR_PRTCL,         								///3<Protocol error
    E_WSS_ERR_SOCKET_FAIL,   								///4<create socket fail
    E_WSS_ERR_BIND_FAIL,     								///5<bind fail  
    E_WSS_ERR_TIMEOUT,       								///6<Connection timeout
    E_WSS_ERR_CONN,          								///7<Connection error
    E_WSS_ERR_CLOSED,        								///8<Connection was closed by remote host
    E_WSS_ERR_MBEDTLS_ERR,   								///9<meet ssl error
    E_WSS_MOREDATA,      									///10<Need get more data
    E_WSS_ERR_OVERFLOW,      								///11<Buffer overflow
    E_WSS_ERR_REQ_TIMEOUT,   								///12<HTTP request timeout waittime is 60s
    E_WSS_ERR_NO_MEMORY,     								///13<memory not enough
    E_WSS_ERR_INTERNAL,       								///14<Internal error
    E_WSS_ERR_PARAM_ERROR,    								///15parameter error
    E_WSS_ERR_NETWORK_NOT_ACTICVE,                        	///16network is not active
    E_WSS_ERR_REQ_PROCESSING,								///17req on processing
	E_WSS_ERR_DECODE,										///18<frame decode err
    E_WSS_ERR_RECV_COMP = 100,                            	///100receive complete
  
}openWssRet_e;

typedef enum _openWssAction_e{
	WSS_ACTION_IDLE,
	WSS_ACTION_INIT,
	WSS_ACTION_CONNECT,
	WSS_ACTION_SEND,
	WSS_ACTION_CLOSE,
}openWssAction_e;

typedef enum _openWssStatus_e{
	WSS_STATUS_IDLE,
	WSS_STATUS_CLOSED = WSS_STATUS_IDLE,
	WSS_STATUS_CLOSING,
	WSS_STATUS_CONNECT,
	WSS_STATUS_HANDSHAKE,
	WSS_STATUS_ACTIVE
}openWssStatus_e;

typedef enum _openWssOpcodeT{
	WSS_OCT_CONT,		//<<0x0 continuation frame
	WSS_OCT_TXT,		//<<0x1 text frame
	WSS_OCT_BIN,		//<<0x2 bynary frame
	WSS_OCT_CLS,		//<<0x8 connection close frame	
	WSS_OCT_PONG,		//<<0xA pong frame
	WSS_OCT_PING,		//<<0x9 ping frame
	WSS_OCT_RSV			//<<0X03~0X07 reserve frame
}openWssOpcodeT;

/*----------------------------------------------------------------------------*
 *					  GLOBAL FUNCTIONS DECLEARATION 						  *
 *----------------------------------------------------------------------------*/
/**
@brief    create websocket engine to process wss connect/close/send
@param[in] NULL
@return	   TRUE---process success, FALSE---process fail
**/
bool openWssEngInit(void);

/**
@brief    create websocket client,get the socketId,less then OPEN_WSS_CLIENT_NUM_MAX
@param[in] NULL
@return	   socketfd,>=0 --- create success, < 0---create fail
**/
int8_t openWssCreate(void);

/**
@brief    connect websocket server,blocked until the handshake complete or connect fail 
@param[in] socketId 	--- client id that return by openWssCreate
		   url 			--- websocket server url
		   actCallback 	--- the callback that be called when operation with action result
		   onMessage    --- if the connect the server success,it will be called when receive message or data
		   extra		--- extra info for websocket handshake if necessary
		   extraLen		--- extra info lenght
@return	   connect result refers to openWssRet_e
**/
int32_t openWssConnect(int8_t socketId, char *url,openWssActionCb actCallback,openWssOnMessage onMessage,char *extra,uint32_t extraLen);

/**
@brief    send data to websocket server,nonblocked 
@param[in] socketId 	--- client id that return by openWssCreate
		   data 		--- the payload in frame
		   dataLen		--- the length of the payload
		   fin			--- indicate whether the data is sharded
		   frameType    --- refers to openWssOpcodeT
		   mask    		--- the data need masked or not
@return	   send result refers to openWssRet_e
**/
int32_t openWssSend(int8_t socketId,char *data,uint32_t dataLen,bool fin,openWssOpcodeT frameType,bool mask);

/**
@brief   close the websocket client,nonblocked
@param[in] socketId 	--- client id that return by openWssCreate
@return	   close result refers to openWssRet_e
**/
int32_t openWssClose(int8_t socketId);


#endif/*__OPEN_WEBSOCKET_H__*/

