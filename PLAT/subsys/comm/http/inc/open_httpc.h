#ifndef __OPEN_HTTP_H__
#define __OPEN_HTTP_H__
#include <stdbool.h>
#include "HTTPClient.h"

/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/
#define 	THREAD_STACK_SIZE_HTTPC_RECV_TASK    		(5 * 1024)
#define 	THREAD_STACK_SIZE_HTTPC_SEND_TASK      		(5 * 1024)

#define 	OPEN_HTTPC_URL_LEN_MAX						(2048)

#define		OPEN_HTTPC_RSP_HEAD_BUFFER_SIZE 			(1501)
#define 	OPEN_HTTPC_RSP_CONTENT_BUFFER_SIZE  		(1501)


typedef void (* openHttpcActionCb)(int8_t result, int8_t dataType, const void *extra,int32_t len);

typedef enum _e_openHttpcRtrDataType{
	HTTPC_RTEDATA_IDLE,
	HTTPC_RTEDATA_HEADER,
	HTTPC_RTEDATA_CONTEXT,
}e_openHttpRtrDataType;

typedef enum _openHttpcRet_e
{
	E_HTTPC_OK = 0,        	    							///<Success
    E_HTTPC_ERR_PARSE,         								///1<url Parse error
    E_HTTPC_ERR_DNS,           								///2<Could not resolve name
    E_HTTPC_ERR_PRTCL,         								///3<Protocol error
    E_HTTPC_ERR_SOCKET_FAIL,   								///4<create socket fail
    E_HTTPC_ERR_BIND_FAIL,     								///5<bind fail  
    E_HTTPC_ERR_TIMEOUT,       								///6<Connection timeout
    E_HTTPC_ERR_CONN,          								///7<Connection error
    E_HTTPC_ERR_CLOSED,        								///8<Connection was closed by remote host
    E_HTTPC_ERR_MBEDTLS_ERR,   								///9<meet ssl error
    E_HTTPC_MOREDATA,      									///10<Need get more data
    E_HTTPC_ERR_OVERFLOW,      								///11<Buffer overflow
    E_HTTPC_ERR_REQ_TIMEOUT,   								///12<HTTP request timeout waittime is 60s
    E_HTTPC_ERR_NO_MEMORY,     								///13<memory not enough
    E_HTTPC_ERR_INTERNAL,       							///14<Internal error
    E_HTTPC_ERR_PARAM_ERROR,    							///15parameter error
    E_HTTPC_ERR_NETWORK_NOT_ACTICVE,                        ///16network is not active
    E_HTTPC_ERR_REQ_PROCESSING,								///17req on processing
    E_HTTPC_ERR_RECV_COMP = 100,                            ///100receive complete
  
}openHttpcRet_e;

typedef enum _openHttpcAction_e{
	HTTPC_ACTION_IDLE,
	HTTPC_ACTION_SEND,
	HTTPC_ACTION_CLOSE,
	HTTPC_ACTION_STOP,	
}openHttpcAction_e;

typedef enum _openHttpcStatus_e{
	HTTPC_STATUS_CLOSED,
	HTTPC_STATUS_CONNECT,
	HTTPC_STATUS_ACTIVE
}openHttpcStatus_e;

typedef struct _openHttpcQueueMsg_t{
	int8_t 				action;
	int32_t 			method;
	uint32_t			extHdrLen; // >>extern header lenght
	uint32_t			exDataLen;
	char 				*url;
	char	   			*extHeader;
	int8_t	   			*extra;	
	openHttpcActionCb	cb;
}openHttpcQueueMsg_t;

typedef struct _openHttpcClientCtx_t{
	BOOL 					urcFlag;//output to com(uart) or file
	uint8_t					status;
	char 					*hostUrl;
	char					*exData;
	int32_t				exDataSize;
	HttpClientContext 		*clientCtx;
	HttpClientData	  		*clientData;
	openHttpcActionCb		cb;
}openHttpcClientCtx_t;

/*----------------------------------------------------------------------------*
 *					  GLOBAL FUNCTIONS DECLEARATION 						  *
 *----------------------------------------------------------------------------*/
bool openHttpcEngInit(void);
int32_t openHttpcGet(char *url,const char *extHeader,uint32_t extHdrLen,openHttpcActionCb cb);
int32_t openHttpcPost(char *url,const char *extHeader,uint32_t extHdrLen,char *data,uint32_t dataLen,openHttpcActionCb cb);

#endif/*__OPEN_HTTP_H__*/
