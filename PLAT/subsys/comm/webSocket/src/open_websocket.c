/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    open_websocket.c
 * Description:  EC718 websocket demo entry source file
 * History:      Rev1.0   2025-05-08
 ****************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include DEBUG_LOG_HEADER_FILE
#include <string.h>
#include "freeRTOS.h"
#include "cmsis_os2.h"
#include "slpman.h"
#include "charge.h"

#include "ps_lib_api.h"
#include "ps_sim_if.h"
#include "networkmgr.h"
#include "netdb.h"
#include "HTTPClient.h"

#include "open_websocket.h"
#include "servicemanager.h"
/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/
#define 	OPEN_WSS_CREATE_SUCC					(0x01)
#define		OPEN_WSS_CREATE_FAIL					(0x02)
#define 	OPEN_WSS_CREATE_COMPLT					(OPEN_WSS_CREATE_SUCC|OPEN_WSS_CREATE_FAIL)
#define 	OPEN_WSS_CONNECT_SUCC					(0x04)
#define		OPEN_WSS_CONNECT_FAIL					(0x08)
#define 	OPEN_WSS_CONNECT_COMPLT					(OPEN_WSS_CONNECT_SUCC|OPEN_WSS_CONNECT_FAIL)
#define 	OPEN_WSS_CLOSE_SUCC						(0x10)
#define		OPEN_WSS_CLOSE_FAIL						(0x20)
#define 	OPEN_WSS_CLOSE_COMPLT					(OPEN_WSS_CLOSE_SUCC|OPEN_WSS_CLOSE_FAIL)
#define 	OPEN_WSS_SEND_INPROGRESS				(0x40)
#define		OPEN_WSS_SEND_FAIL						(0x80)
#define 	OPEN_WSS_SEND_COMPLT					(OPEN_WSS_SEND_INPROGRESS|OPEN_WSS_SEND_FAIL)

#define 	_api_output
#define 	_api_input

typedef struct _openWssQueueMsg_t{
	uint8_t				socketId;	
	int8_t 				action;
	void *				threadId;
	int32_t 			method;		 //<< HTTP_GET/HTTP HEAD
	char 				*url;
	int8_t				*extention;
	uint32_t			extentionLen;
	uint8_t				frameType;
	int8_t	   			*payload;
	uint32_t			payloadLen;
	bool				fin;
	bool				mask;
	openWssActionCb	    actCb;
	openWssOnMessage	onMessage;
}openWssQueueMsg_t;

typedef struct _openWssClientCtx_t{	
	int8_t					socketId;
	uint8_t					status;
	char 					*hostUrl;
	char					*exData;
	int32_t					exDataSize;
	HttpClientContext 		*clientCtx;
	HttpClientData	  		*clientData;
	bool 					isRecvRun;
	osTimerId_t 			timer;
	osMutexId_t    			mutex;
	openWssActionCb	    	actCb;
	openWssOnMessage		onMessage;
}openWssClientCtx_t;


/*----------------------------------------------------------------------------*
 *						GLOBAL VARIABLES									  *
 *----------------------------------------------------------------------------*/

static openWssClientCtx_t gOpenWssCtx[OPEN_WSS_CLIENT_NUM_MAX] = {{0,WSS_STATUS_CLOSED,NULL,NULL,0,NULL,NULL,NULL,NULL,NULL},
																  {1,WSS_STATUS_CLOSED,NULL,NULL,0,NULL,NULL,NULL,NULL,NULL},
																  {2,WSS_STATUS_CLOSED,NULL,NULL,0,NULL,NULL,NULL,NULL,NULL},
																  {3,WSS_STATUS_CLOSED,NULL,NULL,0,NULL,NULL,NULL,NULL,NULL},
																  {4,WSS_STATUS_CLOSED,NULL,NULL,0,NULL,NULL,NULL,NULL,NULL},
																  {5,WSS_STATUS_CLOSED,NULL,NULL,0,NULL,NULL,NULL,NULL,NULL}};
static osMessageQueueId_t open_wss_msgqueue = NULL;

#ifdef THREAD_STATIC
static StaticTask_t  gwssThreadCbMem = {0};
PLAT_FPSRAM_ZI_CUST static uint8_t gwssThreadStackMem[THREAD_STACK_SIZE_WSS_ENG_TASK] = {0};
static StaticTask_t  gRecvThreadCbMem = {0};
PLAT_FPSRAM_ZI_CUST static uint8_t gRecvThreadStackMem[THREAD_STACK_SIZE_WSS_RECV_TASK] = {0};

#endif

static const char *demoCaCrt = \
{
    \
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIDSjCCAjKgAwIBAgIQRK+wgNajJ7qJMDmGLvhAazANBgkqhkiG9w0BAQUFADA/\r\n" \
    "MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\r\n" \
    "DkRTVCBSb290IENBIFgzMB4XDTAwMDkzMDIxMTIxOVoXDTIxMDkzMDE0MDExNVow\r\n" \
    "PzEkMCIGA1UEChMbRGlnaXRhbCBTaWduYXR1cmUgVHJ1c3QgQ28uMRcwFQYDVQQD\r\n" \
    "Ew5EU1QgUm9vdCBDQSBYMzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\r\n" \
    "AN+v6ZdQCINXtMxiZfaQguzH0yxrMMpb7NnDfcdAwRgUi+DoM3ZJKuM/IUmTrE4O\r\n" \
    "rz5Iy2Xu/NMhD2XSKtkyj4zl93ewEnu1lcCJo6m67XMuegwGMoOifooUMM0RoOEq\r\n" \
    "OLl5CjH9UL2AZd+3UWODyOKIYepLYYHsUmu5ouJLGiifSKOeDNoJjj4XLh7dIN9b\r\n" \
    "xiqKqy69cK3FCxolkHRyxXtqqzTWMIn/5WgTe1QLyNau7Fqckh49ZLOMxt+/yUFw\r\n" \
    "7BZy1SbsOFU5Q9D8/RhcQPGX69Wam40dutolucbY38EVAjqr2m7xPi71XAicPNaD\r\n" \
    "aeQQmxkqtilX4+U9m5/wAl0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNV\r\n" \
    "HQ8BAf8EBAMCAQYwHQYDVR0OBBYEFMSnsaR7LHH62+FLkHX/xBVghYkQMA0GCSqG\r\n" \
    "SIb3DQEBBQUAA4IBAQCjGiybFwBcqR7uKGY3Or+Dxz9LwwmglSBd49lZRNI+DT69\r\n" \
    "ikugdB/OEIKcdBodfpga3csTS7MgROSR6cz8faXbauX+5v3gTt23ADq1cEmv8uXr\r\n" \
    "AvHRAosZy5Q6XkjEGB5YGV8eAlrwDPGxrancWYaLbumR9YbK+rlmM6pZW87ipxZz\r\n" \
    "R8srzJmwN0jP41ZL9c8PDHIyh8bwRLtTcm1D9SZImlJnt1ir/md2cXjbDaJWFBM5\r\n" \
    "JDGFoqgCWjBH4d1QB7wCCZAA62RjYJsWvIjJEubSfZGL+T0yjWW06XyxV3bqxbYo\r\n" \
    "Ob8VZRzI9neWagqNdwvYkQsEjgfbKbYK7p2CNTUQ\r\n" \
    "-----END CERTIFICATE-----"
};

/*----------------------------------------------------------------------------*
 * 					 PRIVATE FUNCTION DECLEARATION						      *
 *----------------------------------------------------------------------------*/
static int32_t get_rand_number(uint8_t *data, unsigned size) {
	if(!data || size == 0)
		return 0;
	int cnt = 0;
	uint8_t tmp = 0;
	for(;cnt < size;cnt++)
	{
		tmp = (uint8_t)(rand() & 0xff);
		if(tmp == 0)
			tmp = 128;
		data[cnt] = tmp;
	}
	ECPLAT_DUMP(UNILOG_PLAT_NETWORK, get_rand_number, P_INFO, " ", size, data);
	return 1;
}

static int base64enc(const char *input, unsigned int length, char *output, int len)
{
    static const char base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    unsigned int c, c1, c2, c3;

    if ((uint16_t)len < ((((length-1)/3)+1)<<2)) return -1;
    for(unsigned int i = 0, j = 0; i<length; i+=3,j+=4) {
        c1 = ((((unsigned char)*((unsigned char *)&input[i]))));
        c2 = (length>i+1)?((((unsigned char)*((unsigned char *)&input[i+1])))):0;
        c3 = (length>i+2)?((((unsigned char)*((unsigned char *)&input[i+2])))):0;

        c = ((c1 & 0xFC) >> 2);
        output[j+0] = base64[c];
        c = ((c1 & 0x03) << 4) | ((c2 & 0xF0) >> 4);
        output[j+1] = base64[c];
        c = ((c2 & 0x0F) << 2) | ((c3 & 0xC0) >> 6);
        output[j+2] = (length>i+1)?base64[c]:'=';
        c = (c3 & 0x3F);
        output[j+3] = (length>i+2)?base64[c]:'=';
    }
    output[(((length-1)/3)+1)<<2] = '\0';
    return 0;
}

static bool getWebSocketKey(_api_output char *key, _api_input uint8_t keyLen)
{
	if(key == NULL)
		return false;
	uint8_t randNum[16] = {0};
	memset(randNum,0x00,16*sizeof(char));
	if(!get_rand_number(randNum,16))
	{
		return false;
	}
	ECPLAT_DUMP(UNILOG_PLAT_NETWORK, getWebSocketKey, P_INFO, " ", 16, randNum);
	if(base64enc((char *)randNum,16,key,keyLen))
	{
		return false;
	}
	return true;
}

static void openWssClearContext(openWssClientCtx_t * context)
{	
	if(context == NULL)
		return;	
	context->status = WSS_STATUS_CLOSED;
	context->exDataSize = 0;
	context->actCb = NULL;
	context->onMessage = NULL;
	if(context->exData != NULL)
	{
		free(context->exData);
		context->exData = NULL;
	}
}

static bool openWssCreateContext(openWssClientCtx_t * context)
{
	if(context == NULL)
		return false;
	if(context->clientCtx == NULL)
	{
		context->clientCtx = malloc(sizeof(HttpClientContext));
		if(context->clientCtx == NULL){
			ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, openWssCreateContext, P_VALUE, "no memory");
			return false;
		}
		memset(context->clientCtx, 0, sizeof(HttpClientContext));
		context->clientCtx->timeout_s = 2;  //default send timeout 2 second,no need to wait for a long time for TCP
		context->clientCtx->timeout_r = 20;  //default recv timeout 20 second
		context->clientCtx->socket = -1;
		context->clientCtx->pdpId = 1;		//default cid
		context->clientCtx->saveMem = 1;
		context->clientCtx->ignore = 1;
		
		context->clientCtx->caCert= (char*)demoCaCrt;
		context->clientCtx->caCertLen= strlen(demoCaCrt)+1;
		context->clientCtx->seclevel = 0;
		context->clientCtx->sni = 1;
		context->clientCtx->ciphersuite[0] = 0xFFFF;
	}
	
	return true;
}

static void openWssDeleteContext(openWssClientCtx_t * context)
{
	if(context == NULL)
		return;
	HttpClientContext* clientContext = context->clientCtx;

	if(clientContext != NULL)
	{
		if(clientContext->clientPk != NULL)
		{
			free((char*)clientContext->clientPk);
		}
		free((void*)clientContext);
		context->clientCtx = NULL;
		if(context->hostUrl != NULL)
		{
			free(context->hostUrl);
			context->hostUrl = NULL;
		}
	}
}

static bool openWssCreateClientData(openWssClientCtx_t * context)
{
    if(context == NULL)
        return false;

    if(context->clientData == NULL)
    {
        context->clientData = malloc(sizeof(HttpClientData));
        if(context->clientData == NULL){
            ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, openWssCreateClientData_1, P_VALUE, "no memory");
            return false;
        }
        memset(context->clientData, 0, sizeof(HttpClientData));
        context->clientData->headerBufLen = OPEN_WSS_RSP_HEAD_BUFFER_SIZE;
        context->clientData->headerBuf = malloc(OPEN_WSS_RSP_HEAD_BUFFER_SIZE);
        if(context->clientData->headerBuf == NULL){
            ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, openWssCreateClientData_2, P_VALUE, "no memory");
            free(context->clientData);
            context->clientData = NULL;
            return false;
        }
        context->clientData->respBufLen = OPEN_WSS_RSP_CONTENT_BUFFER_SIZE;
        context->clientData->respBuf = malloc(OPEN_WSS_RSP_CONTENT_BUFFER_SIZE);
        if(context->clientData->respBuf == NULL){
            ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, openWssCreateClientData_3, P_VALUE, "no memory");
            free(context->clientData->headerBuf);
            context->clientData->headerBuf = NULL;
            free(context->clientData);
            context->clientData = NULL;
            return false;
        }
    }
    return true;
}

static void openWssDeleteClientData(openWssClientCtx_t * context)
{
    if(context == NULL)
        return;
    HttpClientData* clientData = context->clientData;
    if(clientData != NULL)
    {
        if(clientData->respBuf)
        {
            free(clientData->respBuf);
        }
        if(clientData->headerBuf)
        {
            free(clientData->headerBuf);
        }
        free(clientData);
        context->clientData = NULL;
    }
}


static int8_t openWssMutexCreate(osMutexId_t *mutex)
{
	if(*mutex != NULL)
    {
        return FALSE;
    }
     *mutex = osMutexNew(NULL);

    if(*mutex == NULL)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

static int8_t openWssMutexAcquire(osMutexId_t mutex)
{
	if(mutex == NULL)
    {
        return FALSE;
    }
    if (osMutexAcquire(mutex, osWaitForever) != osOK)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

static void openWssMutexRelease(osMutexId_t mutex)
{
	if(mutex == NULL)
    {
        return;
    }
    osMutexRelease(mutex);
}

static void openWssMutexDelete(osMutexId_t mutex)
{
	if(mutex == NULL)
    {
        return;
    }
    osMutexDelete(mutex);
    mutex = NULL;
}

static void openWssRspTimerExp(void *param)
{
	int8_t socketId = *((int8_t *)param);
    openWssQueueMsg_t queueMsg = {0};
    osStatus_t status;
    ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, openWssRspTimerExp, P_INFO, "requst timeout, close socket");
	memset(&queueMsg,0,sizeof(openWssQueueMsg_t));
    //httpRetInd(HTTPAT_REQ_TIMEOUT);
    queueMsg.action = WSS_ACTION_CLOSE;
	queueMsg.socketId = socketId;
    status = osMessageQueuePut(open_wss_msgqueue, &queueMsg, 0, 0);
    EC_ASSERT(status==osOK,status,0,0);
}
static int8_t wssSendAndEncodeFrame(openWssClientCtx_t* context,int8_t *payload,uint32_t payloadLen,bool fin,uint8_t frameType,bool mask)
{
	uint8_t finbit,maskBit = 0,opcode = 0xff;
	int i = 0,maskIndex = 0;
	uint8_t headLen = 2;
	uint32_t offset = 0;
	uint8_t lengthField = 0;
	int8_t *frame = NULL;
	uint8_t maskValue[] = {0,0,0,0};
	finbit = (fin ? 1 : 0);
	maskBit = (mask ? 1 : 0);
#ifdef OPEN_WSS_DUMP_ENABLE
	ECPLAT_PRINTF(UNILOG_PLAT_NETWORK,wssSendAndEncodeFrame,P_WARNING,"fin(%d) payloadLen(%d) frameType (%d) mask(%d)",fin,payloadLen,frameType,mask);
#endif
	if(frameType < WSS_OCT_CLS && (payloadLen == 0 || payload == NULL))
	{
		ECPLAT_PRINTF(UNILOG_PLAT_NETWORK,wssSendAndEncodeFrame_valid_0,P_WARNING,"bad payload");
	}
	switch(frameType)
	{
		case WSS_OCT_CONT:
		opcode = 0x00;
		break;
		case WSS_OCT_TXT:
			opcode = 0x01;
			break;
		case WSS_OCT_BIN:
			opcode = 0x02;
			break;
		case WSS_OCT_CLS:
			opcode = 0x08;
			break;
		case WSS_OCT_PING:
			opcode = 0x09;
			break;
		case WSS_OCT_PONG:
			opcode = 0x0A;
			break;
		default:
			break;
	}
	if(opcode == 0xff)
	{
		ECPLAT_PRINTF(UNILOG_PLAT_NETWORK,wssSendAndEncodeFrame_valid_1,P_ERROR,"rsv opcode not support");
		return E_WSS_ERR_PARAM_ERROR;
	}
	if(maskBit)
	{
		headLen = headLen + 4;
	}
	if(payloadLen >= 126 && payloadLen <= 0xffff)
	{
		lengthField = 2;
	}
	else if(payloadLen > 0xffff)
	{
		lengthField = 8;
	}
	headLen = headLen + lengthField;

	frame = (int8_t *)calloc(headLen + payloadLen,sizeof(int8_t));
	EC_ASSERT(frame,0,0,0);
	frame[offset++] = (finbit << 7) | opcode;
	switch(lengthField)
	{

		case 0:
		{
			frame[offset++] = (maskBit << 7) | (payloadLen & 0x7f);
		}
		break;
		case 2:
		{
			frame[offset++] = (maskBit << 7) | 0x7e;
			frame[offset++] = (payloadLen & 0xff00) >> 8;
			frame[offset++] = (payloadLen & 0x00ff);
		}
		break;
		case 8:
		{
			frame[offset++] = (maskBit << 7) | 0x7f;
			frame[offset++] = 0x00;
			frame[offset++] = 0x00;
			frame[offset++] = 0x00;
			frame[offset++] = 0x00;
			frame[offset++] = (payloadLen & 0xff000000) >> 24;
			frame[offset++] = (payloadLen & 0x00ff0000) >> 16;
			frame[offset++] = (payloadLen & 0x0000ff00) >> 8;
			frame[offset++] = (payloadLen & 0x000000ff);
		}
		break;
		default:
		{
			ECPLAT_PRINTF(UNILOG_PLAT_NETWORK,wssSendAndEncodeFrame_valid_3,P_ERROR,"bad Lenghtfield(%d)",lengthField);
		}
		break;
	}
	if(maskBit)
	{
		get_rand_number(maskValue, 4);
		frame[offset++] = maskValue[0];
		frame[offset++] = maskValue[1];
		frame[offset++] = maskValue[2];
		frame[offset++] = maskValue[3];
		for(i = 0,maskIndex =0; i < payloadLen; i++,maskIndex++)
		{
			frame[offset++] = payload[i] ^ maskValue[maskIndex % 4];
		}
	}
	else
		memcpy(frame + offset,payload,payloadLen);
#ifdef OPEN_WSS_DUMP_ENABLE
	ECPLAT_PRINTF(UNILOG_PLAT_NETWORK,wssSendAndEncodeFrame_1,P_DEBUG,"offset(%d)",offset);
	ECPLAT_DUMP(UNILOG_PLAT_NETWORK,wssSendAndEncodeFrame_dump_0,P_DEBUG,"",offset,frame);
#endif
	httpSend(context->clientCtx, (char *)frame, offset);
	if(frame)
		free(frame);
	frame = NULL;
	return E_WSS_OK;
}

static int8_t wssDecodeFrame(openWssClientCtx_t* context,char *frame,int32_t frameLen)
{
	uint8_t fin,maskBit,frameType,opcode,payloadLen = 0;
	int i = 0;
	int maskCnt = 0;
	uint8_t maskValue[4] = {0,0,0,0};
	uint32_t realPayLoadSize = 0;
	int32_t headSize = 0;
	uint32_t index = 0;
	
	uint32_t decodeSize = 0;

	char *readBuf = frame;
	int32_t readSize = frameLen;
	char *payLoad = NULL;


	if(readSize < 2 || readBuf == NULL)
	{
		return E_WSS_ERR_DECODE;
	}
		

	realPayLoadSize = 0;
	headSize = 0;
	decodeSize = 0;
	fin = (readBuf[index] & 0x80 ) >> 7;
	opcode = readBuf[index] & 0x0f;
	if(!(((fin == 0x00) && (opcode == 0x00)) || 
		((fin == 0x01) && ((opcode >= 0x00) && 
		(opcode <= 0x0f)))))
	{
		ECPLAT_PRINTF(UNILOG_PLAT_NETWORK,wssDecodeFrame_valid_0,P_ERROR,"decode fail:bad fin(0x%x) or opcode(0x%x)",fin,opcode);
		return E_WSS_ERR_DECODE;
	}
		
	index++;
	switch(opcode)
	{
		case 0x00:
			frameType = WSS_OCT_CONT;
			break;
		case 0x01:
			frameType = WSS_OCT_TXT;
			break;
		case 0x02:
			frameType = WSS_OCT_BIN;
			break;
		case 0x08:
			frameType = WSS_OCT_CLS;
			break;
		case 0x09:
			frameType = WSS_OCT_PING;
			break;
		case 0x0A:
			frameType = WSS_OCT_PONG;
			break;
		default:
			frameType = WSS_OCT_RSV;
			break;
			
	}
	maskBit = (readBuf[index] & 0x80 ) >> 7;
	payloadLen = readBuf[index] & 0x7f;
	if(payloadLen == 126)
	{
		headSize = (maskBit << 2) + 4;
		if(readSize < headSize)
		{
			ECPLAT_PRINTF(UNILOG_PLAT_NETWORK,wssDecodeFrame_valid_1,P_ERROR,"decode fail:readSize(%d) < headSize(%d) ",readSize,headSize);
			return E_WSS_ERR_DECODE;
		}
			
		realPayLoadSize = readBuf[index+1] << 8 | readBuf[index+2];
		index = index + 2;
		if(maskBit)
		{
			maskValue[0] = readBuf[++index];
			maskValue[1] = readBuf[++index];
			maskValue[2] = readBuf[++index];
			maskValue[3] = readBuf[++index];
		}
		else
			index++;
	}
	else if(payloadLen == 127)
	{

		headSize = (maskBit << 2) + 10;
		if(readSize < headSize)
		{
			ECPLAT_PRINTF(UNILOG_PLAT_NETWORK,wssDecodeFrame_valid_2,P_ERROR,"decode fail:readSize(%d) < headSize(%d) ",readSize,headSize);
			return E_WSS_ERR_DECODE;
		}
		if(readBuf[index+1] != 0 || readBuf[index+2] != 0 || readBuf[index+3] != 0 ||readBuf[index+4] != 0)
		{
			ECPLAT_PRINTF(UNILOG_PLAT_NETWORK,wssDecodeFrame_valid_3,P_ERROR,"bad frame length field");
			return E_WSS_ERR_DECODE;
		}		
		realPayLoadSize = (readBuf[index+5] << 24) | \
						  (readBuf[index+6] << 16) | \
						  (readBuf[index+7] << 8)  | \
						   readBuf[index+8];
		index = index + 8;	
		if(maskBit)
		{
			maskValue[0] = readBuf[++index];
			maskValue[1] = readBuf[++index];
			maskValue[2] = readBuf[++index];
			maskValue[3] = readBuf[++index];
		}
		else
			index++;
	}
	else
	{		
		headSize = (maskBit << 2) + 2;
		if(readSize < headSize)
		{
			ECPLAT_PRINTF(UNILOG_PLAT_NETWORK,wssDecodeFrame_valid_4,P_ERROR,"decode fail:readSize(%d) < headSize(%d) ",readSize,headSize);
			return E_WSS_ERR_DECODE;
		}
		realPayLoadSize = payloadLen;
		if(maskBit)
		{
			maskValue[0] = readBuf[++index];
			maskValue[1] = readBuf[++index];
			maskValue[2] = readBuf[++index];
			maskValue[3] = readBuf[++index];
		}
		else
			index++;
	}
#ifdef OPEN_WSS_DUMP_ENABLE
	ECPLAT_PRINTF(UNILOG_PLAT_NETWORK,wssDecodeFrame_1,P_DEBUG,"readSize(%d),headSize(%d),index(%d),realPayLoadSize(%d)",readSize,headSize,index,realPayLoadSize);
#endif
	payLoad = (char *)calloc(realPayLoadSize+1,sizeof(char));
	EC_ASSERT(payLoad,payLoad,0,0);

	if((readSize - index) >= realPayLoadSize)
	{
		if(maskBit)
		{			
			for(i = 0,maskCnt = 0; i < realPayLoadSize ; i++,maskCnt++)
			{
				maskCnt = maskCnt % 4;
				payLoad[i] = readBuf[index + i] ^ maskValue[maskCnt];
			}
		}
		else
		{
			memcpy(payLoad,readBuf + index,realPayLoadSize);
		}
		decodeSize = realPayLoadSize;
		index = index + decodeSize;
	}
	else
	{
		goto EXIT;
	}
	if(frameType <= WSS_OCT_PONG && context->onMessage)
	{
		context->onMessage(context->socketId,(int8_t)frameType,(int8_t *)payLoad,decodeSize);
	}

	if(frameType == WSS_OCT_CLS)
	{
		if(context->status == WSS_STATUS_ACTIVE)
		{
					
			wssSendAndEncodeFrame(context, (int8_t *)payLoad, 2, true, WSS_OCT_CLS, true);
			context->status = WSS_STATUS_CLOSING;
		}
		if(open_wss_msgqueue != NULL)
		{
			openWssQueueMsg_t queueMsg = {0};
			memset(&queueMsg,0,sizeof(openWssQueueMsg_t));
			queueMsg.socketId = context->socketId;
			queueMsg.action = WSS_ACTION_CLOSE; 		
			osMessageQueuePut(open_wss_msgqueue, &queueMsg, 0, 0);
			
		}
	}
	else if(frameType == WSS_OCT_PING)
	{
		wssSendAndEncodeFrame(context, (int8_t *)payLoad, decodeSize, true, WSS_OCT_PONG, true);
	}
EXIT:
#ifdef OPEN_WSS_DUMP_ENABLE
	ECPLAT_PRINTF(UNILOG_PLAT_NETWORK,wssDecodeFrame_2,P_DEBUG,"opcode(0x%x),mask(0x%x),fin(0x%x),decodeSize(%d)",opcode,maskBit,fin,decodeSize);
#endif
	if(payLoad)
		free(payLoad);
	payLoad = NULL;

	return E_WSS_OK;
}


static int8_t wssRecvAndDecodeFrame(openWssClientCtx_t* context)
{
	uint8_t fin,maskBit,frameType,opcode,payloadLen = 0;
	int32_t i = 0;
	int32_t maskCnt = 0;
	uint8_t maskValue[4] = {0,0,0,0};
	uint32_t realPayLoadSize = 0;
	int32_t headSize = 0;
	uint32_t remainSize = 0;
	uint32_t index = 0;
	int32_t readSize = 0;
	uint32_t decodeSize = 0;
	uint32_t maxReadSize = 0;
	int32_t tryReadSize = 0;
	char readBuf[CHUNK_SIZE] = {0};
	char *payLoad = NULL;
	memset(readBuf,0x00,CHUNK_SIZE);
	HTTPResult ret = 0;
	ret = httpRecv(context->clientCtx,readBuf,2,CHUNK_SIZE,&readSize);
	if(ret)
		return ret;
#ifdef OPEN_WSS_DUMP_ENABLE
	ECPLAT_PRINTF(UNILOG_PLAT_NETWORK,wssRecvAndDecodeFrame_0,P_DEBUG,"readSize(%d)",readSize);
	ECPLAT_DUMP(UNILOG_PLAT_NETWORK,threadWssRecv_DUMP,P_DEBUG,"",readSize,(uint8_t *)readBuf);
#endif

	if(readSize < 2)
		return E_WSS_ERR_DECODE;
	do{
		realPayLoadSize = 0;
		headSize = 0;
		remainSize = 0;
		decodeSize = 0;
		fin = (readBuf[index] & 0x80 ) >> 7;
		opcode = readBuf[index] & 0x0f;
		if(!(((fin == 0x00) && (opcode == 0x00)) || 
			((fin == 0x01) && ((opcode >= 0x00) && 
			(opcode <= 0x0f)))))
		{
			ECPLAT_PRINTF(UNILOG_PLAT_NETWORK,wssRecvAndDecodeFrame_valid_0,P_ERROR,"fin(%d) , opcode(%d) ",fin,opcode);
			return E_WSS_ERR_DECODE;
		}
			
		index++;
		switch(opcode)
		{
			case 0x00:
				frameType = WSS_OCT_CONT;
				break;
			case 0x01:
				frameType = WSS_OCT_TXT;
				break;
			case 0x02:
				frameType = WSS_OCT_BIN;
				break;
			case 0x08:
				frameType = WSS_OCT_CLS;
				break;
			case 0x09:
				frameType = WSS_OCT_PING;
				break;
			case 0x0A:
				frameType = WSS_OCT_PONG;
				break;
			default:
				frameType = WSS_OCT_RSV;
				break;
				
		}
		maskBit = (readBuf[index] & 0x80 ) >> 7;
		payloadLen = readBuf[index] & 0x7f;
		if(payloadLen == 126)
		{
			headSize = (maskBit << 2) + 4;
			if(readSize < headSize)
			{
				ECPLAT_PRINTF(UNILOG_PLAT_NETWORK,wssRecvAndDecodeFrame_valid_1,P_ERROR,"decode fail:readSize(%d) < headSize(%d) ",readSize,headSize);
				return E_WSS_ERR_DECODE;
			}
			realPayLoadSize = readBuf[index+1] << 8 | readBuf[index+2];
			index = index + 2;
			if(maskBit)
			{
				maskValue[0] = readBuf[++index];
				maskValue[1] = readBuf[++index];
				maskValue[2] = readBuf[++index];
				maskValue[3] = readBuf[++index];
			}
			else
				index++;
		}
		else if(payloadLen == 127)
		{

			headSize = (maskBit << 2) + 10;
			if(readSize < headSize)
				return E_WSS_ERR_DECODE;
			if(readBuf[index+1] != 0 || readBuf[index+2] != 0 || readBuf[index+3] != 0 ||readBuf[index+4] != 0)
			{
				ECPLAT_PRINTF(UNILOG_PLAT_NETWORK,wssRecvAndDecodeFrame_valid_2,P_ERROR,"bad frame length field ");
				return E_WSS_ERR_DECODE;
			}		
			realPayLoadSize = (readBuf[index+5] << 24) | \
							  (readBuf[index+6] << 16) | \
							  (readBuf[index+7] << 8)  | \
							   readBuf[index+8];
			index = index + 8;	
			if(maskBit)
			{
				maskValue[0] = readBuf[++index];
				maskValue[1] = readBuf[++index];
				maskValue[2] = readBuf[++index];
				maskValue[3] = readBuf[++index];
			}
			else
				index++;
		}
		else
		{
			headSize = (maskBit << 2) + 2;
			if(readSize < headSize)
			{
				ECPLAT_PRINTF(UNILOG_PLAT_NETWORK,wssRecvAndDecodeFrame_valid_3,P_ERROR,"decode fail:readSize(%d) < headSize(%d) ",readSize,headSize);
				return E_WSS_ERR_DECODE;
			}
			realPayLoadSize = payloadLen;
			if(maskBit)
			{
				maskValue[0] = readBuf[++index];
				maskValue[1] = readBuf[++index];
				maskValue[2] = readBuf[++index];
				maskValue[3] = readBuf[++index];
			}
			else
				index++;
		}
#ifdef OPEN_WSS_DUMP_ENABLE
		ECPLAT_PRINTF(UNILOG_PLAT_NETWORK,wssRecvAndDecodeFrame_1,P_DEBUG,"readSize(%d),headSize(%d),index(%d),realPayLoadSize(%d)",readSize,headSize,index,realPayLoadSize);
#endif
		payLoad = (char *)calloc(realPayLoadSize+1,sizeof(char));
		EC_ASSERT(payLoad,payLoad,0,0);

		if((readSize - index) >= realPayLoadSize)
		{
			if(maskBit)
			{			
				for(i = 0,maskCnt = 0; i < realPayLoadSize ; i++,maskCnt++)
				{
					maskCnt = maskCnt % 4;
					payLoad[i] = readBuf[index + i] ^ maskValue[maskCnt];
				}
			}
			else
			{
				memcpy(payLoad,readBuf + index,realPayLoadSize);
			}
			decodeSize = realPayLoadSize;
			index = index + decodeSize;
		}
		else
		{
			remainSize = realPayLoadSize - (readSize - index);
			//copy to playload
			decodeSize = readSize - index;
			ECPLAT_PRINTF(UNILOG_PLAT_NETWORK,wssRecvAndDecodeFrame_2,P_DEBUG,"remainSize(%d),decodeSize(%d)",remainSize,decodeSize);
			if(maskBit)
			{
				for(i = 0,maskCnt = 0; i < decodeSize ; i++,maskCnt++)
				{
					maskCnt = maskCnt % 4;
					payLoad[i] = readBuf[index + i] ^ maskValue[maskCnt];
				}
			}
			else
			{
				memcpy(payLoad,readBuf + index,decodeSize);
			}
			index = index + decodeSize;
			
			do{
				tryReadSize = 0;
				memset(readBuf,0,CHUNK_SIZE);
				maxReadSize = (remainSize > CHUNK_SIZE ? CHUNK_SIZE : remainSize);
				ret = httpRecv(context->clientCtx,readBuf,1,maxReadSize,&tryReadSize);
				if(ret != HTTP_OK)
				{
					ECPLAT_PRINTF(UNILOG_PLAT_NETWORK,wssRecvAndDecodeFrame_invalid_4,P_WARNING,"continue read fail(%d)",ret);
					break;
				}
				//copy to playload;
				ECPLAT_PRINTF(UNILOG_PLAT_NETWORK,wssRecvAndDecodeFrame_3,P_DEBUG,"tryReadSize(%d)",tryReadSize);
				if(maskBit)
				{
					for(i = 0,maskCnt = 0; i < tryReadSize ; i++,maskCnt++)
					{
						maskCnt = maskCnt % 4;
						payLoad[decodeSize + i] = readBuf[i] ^ maskValue[maskCnt];
					}
				}
				else
				{
					memcpy(payLoad + decodeSize,readBuf,tryReadSize);
				}
				decodeSize = decodeSize + tryReadSize;
				remainSize = remainSize - tryReadSize;
				EC_ASSERT(decodeSize <= realPayLoadSize,decodeSize,realPayLoadSize,remainSize);
			}while(remainSize > 0 && tryReadSize > 0);
		}
		if(frameType <= WSS_OCT_PONG && context->onMessage)
		{
			context->onMessage(context->socketId,(int8_t)frameType,(int8_t *)payLoad,decodeSize);
		}

		if(frameType == WSS_OCT_CLS)
		{
			if(context->status == WSS_STATUS_ACTIVE)
			{
						
				wssSendAndEncodeFrame(context, (int8_t *)payLoad, 2, true, WSS_OCT_CLS, true);
				context->status = WSS_STATUS_CLOSING;
			}
			if(open_wss_msgqueue != NULL)
			{
				openWssQueueMsg_t queueMsg = {0};
				memset(&queueMsg,0,sizeof(openWssQueueMsg_t));
				queueMsg.socketId = context->socketId;
				queueMsg.action = WSS_ACTION_CLOSE;			
				osMessageQueuePut(open_wss_msgqueue, &queueMsg, 0, 0);
				
			}
		}
		else if(frameType == WSS_OCT_PING)
		{
			wssSendAndEncodeFrame(context, (int8_t *)payLoad, decodeSize, true, WSS_OCT_PONG, true);
		}
#ifdef OPEN_WSS_DUMP_ENABLE
		ECPLAT_PRINTF(UNILOG_PLAT_NETWORK,wssRecvAndDecodeFrame_4,P_DEBUG,"opcode(0x%x),mask(0x%x),fin(0x%x),decodeSize(%d)",opcode,maskBit,fin,decodeSize);
#endif
		if(payLoad)
			free(payLoad);
		payLoad = NULL;
	}while(index < readSize);
	return E_WSS_OK;
}

static void threadWssRecv(void *param)
{
	int32_t result = E_WSS_OK;
	int8_t socketId = *((int8_t *)param);
	int isRecvTaskRunning = TRUE;
	uint16_t headerLen = 0;
	openWssQueueMsg_t queueMsg = {0};
	osStatus_t status;
	fd_set readFs,errorFs;
	struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    int httpSockFs = -1,ret;
	int mErr = 0;
	memset(&queueMsg,0,sizeof(openWssQueueMsg_t));
	openWssClientCtx_t *context = &gOpenWssCtx[socketId];
	ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadWssRecv, P_INFO, "socketId(%d) context->status(%d) ",socketId, context->status);
	while(1)
	{
		switch(context->status)
		{
			case WSS_STATUS_CLOSING:
			case WSS_STATUS_ACTIVE:
			{	
				httpSockFs = context->clientCtx->socket;
				if(httpSockFs >= 0)
				{
					FD_ZERO(&readFs);
                    FD_ZERO(&errorFs);
                    FD_SET(context->clientCtx->socket, &readFs);
                    FD_SET(context->clientCtx->socket, &errorFs);
                    ret = select(httpSockFs + 1, &readFs, NULL, &errorFs, &tv);
                    if(ret > 0)
                    {  
                    	if(FD_ISSET(context->clientCtx->socket, &errorFs))
                        {
                            mErr = sock_get_errno(context->clientCtx->socket);
                            if(socket_error_is_fatal(mErr))
                            {
                                //httpRetInd(HTTPAT_SOCKET_ERROR);
                                context->status = WSS_STATUS_CLOSING;
                                ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadWssRecv_01, P_VALUE, "selected fatal error socket:%d, errno:%d", context->clientCtx->socket, mErr);
                                queueMsg.action = WSS_ACTION_CLOSE;
								queueMsg.socketId = socketId;
                                status = osMessageQueuePut(open_wss_msgqueue, &queueMsg, 0, 3);
                                EC_ASSERT(status==osOK,status,0,0);
								isRecvTaskRunning = FALSE;
								break;
                            }
                            else
                            {
                                ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadWssRecv_02, P_VALUE, "selected not fatal error socket:%d, errno:%d", context->clientCtx->socket, mErr);
                            }
                        }
                    	if(FD_ISSET(context->clientCtx->socket, &readFs))
                        {
                        	if(wssRecvAndDecodeFrame(context))
                    		{
                    			ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadWssRecv_valid_01, P_ERROR, "read fail");
								context->status = WSS_STATUS_CLOSING;
	                            queueMsg.action = WSS_ACTION_CLOSE;
								queueMsg.socketId = socketId;
	                            status = osMessageQueuePut(open_wss_msgqueue, &queueMsg, 0, 3);
	                            EC_ASSERT(status==osOK,status,0,0);
								isRecvTaskRunning = FALSE;
								break;
                    		}
                        } 
                    }
                    else
                    {
                        ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadWssRecv_03, P_VALUE, "recv select return:%d, errno:%d", ret, errno);
						mErr = sock_get_errno(context->clientCtx->socket);
                        if(socket_error_is_fatal(mErr))
                        {
                            //httpRetInd(HTTPAT_SOCKET_ERROR);                                
                            ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadWssRecv_04, P_VALUE, "selected fatal error socket:%d, errno:%d", context->clientCtx->socket, mErr);
							context->status = WSS_STATUS_CLOSING;
                            queueMsg.action = WSS_ACTION_CLOSE;
							queueMsg.socketId = socketId;
                            status = osMessageQueuePut(open_wss_msgqueue, &queueMsg, 0, 3);
                            EC_ASSERT(status==osOK,status,0,0);
							isRecvTaskRunning = FALSE;
							break;
                        }
                    }
				}
				else
				{
					ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadWssRecv_05, P_VALUE, "no valid socket exit recv task, errno:%d", errno);
				}
				break;
			}
			case WSS_STATUS_HANDSHAKE:
			{	
				httpSockFs = context->clientCtx->socket;
				if(httpSockFs >= 0)
				{
					FD_ZERO(&readFs);
                    FD_ZERO(&errorFs);
                    FD_SET(context->clientCtx->socket, &readFs);
                    FD_SET(context->clientCtx->socket, &errorFs);
                    ret = select(httpSockFs + 1, &readFs, NULL, &errorFs, &tv);
                    if(ret > 0)
                    {
                        if(osTimerIsRunning(context->timer) != 0)
                        {
                            ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadWssRecv_06, P_INFO, "stop timer");
                            osTimerStop(context->timer);
                            osTimerDelete(context->timer);
                            context->timer = NULL;
                        }
                        if(FD_ISSET(context->clientCtx->socket, &readFs))
                        {
                            do
                            {
                                openWssMutexAcquire(context->mutex);

                                if(context->status != WSS_STATUS_HANDSHAKE)//user interrupted the request
                                {
                                    ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadWssRecv_07, P_VALUE, "wssMutexRelease");
                                    openWssMutexRelease(context->mutex);
                                    break;
                                }

                                memset(context->clientData->headerBuf, 0, context->clientData->headerBufLen);
                                memset(context->clientData->respBuf, 0, context->clientData->respBufLen);
								context->clientCtx->method = HTTP_HEAD;
                                result = httpRecvResponse(context->clientCtx, context->clientData);
								ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadWssRecv_08, P_VALUE, "RecvResponse(%d)",result);
                                if(result == E_WSS_OK)
                                {
                                    //output data
                                    headerLen = strlen(context->clientData->headerBuf);
									if(headerLen > 0)
									{
										//TODO :Parse HEAD
										
									}
									wssDecodeFrame(context,context->clientData->respBuf,context->clientData->respBufLen);
									context->status = WSS_STATUS_ACTIVE;
                                }
                                else if(result == E_WSS_MOREDATA)
                                {
                                	headerLen = strlen(context->clientData->headerBuf);
									if(headerLen > 0)
									{
										//TODO :Parse HEAD
									}								
                                }
                                else if(result == E_WSS_ERR_CONN)
                                {
                                    ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadWssRecv_09, P_INFO, "wssRecvResponse return E_WSS_ERR_CONN release mutex try recv data again");
                                }
                                else
                                {
                                    int mErr = sock_get_errno(context->clientCtx->socket);
                                    ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadWssRecv_10, P_VALUE, "wssRecvResponse return %d error=%d, close socket", result,mErr);
                                    queueMsg.action = WSS_ACTION_CLOSE;
									queueMsg.socketId = socketId;
                                    status = osMessageQueuePut(open_wss_msgqueue, &queueMsg, 0, 0);
                                    EC_ASSERT(status==osOK,status,0,0);
                                }
                                openWssMutexRelease(context->mutex);
                            }while(result == HTTP_MOREDATA);
							if(context->actCb)
								context->actCb(socketId,WSS_ACTION_CONNECT,result);
                        }
                        else if(FD_ISSET(context->clientCtx->socket, &errorFs))
                        {
                            int mErr = sock_get_errno(context->clientCtx->socket);
                            if(socket_error_is_fatal(mErr))
                            {
                                //httpRetInd(HTTPAT_SOCKET_ERROR);                                
                                ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadWssRecv_11, P_VALUE, "selected fatal error socket:%d, errno:%d", context->clientCtx->socket, mErr);
                                queueMsg.action = WSS_ACTION_CLOSE;
								queueMsg.socketId = socketId;
                                status = osMessageQueuePut(open_wss_msgqueue, &queueMsg, 0, 0);
                                EC_ASSERT(status==osOK,status,0,0);
                            }
                            else
                            {
                                ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadWssRecv_12, P_VALUE, "selected not fatal error socket:%d, errno:%d", context->clientCtx->socket, mErr);
                            }
                        }
                    }
                    else
                    {
                        ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadWssRecv_13, P_VALUE, "recv select return:%d, errno:%d", ret, errno);
                    }
				}
				else
				{
					ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadWssRecv_14, P_VALUE, "no valid socket exit recv task, errno:%d", errno);
				}
				osDelay(10);
				break;
			}
			case WSS_STATUS_CONNECT:
            {
                if(context->clientCtx->socket == -1){
                    ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadWssRecv_15, P_VALUE, "in HTTPSTAT_CONNECTED socket invalid exit recv task, errno:%d", errno);
                    isRecvTaskRunning = FALSE;
                }
				osDelay(10);
                break;
            }

            case WSS_STATUS_CLOSED:
            default:
            {
                isRecvTaskRunning = FALSE;
                break;
            }
		}
		if(!isRecvTaskRunning)
		{
			break;
		}
	}
    ostaskENTER_CRITICAL();
    context->isRecvRun = isRecvTaskRunning;
    ostaskEXIT_CRITICAL();
#if 0 // Service Manager
    osThreadExit();
#else
    Service_stop("service:/threadwssRecv");
#endif
}


static bool openWssRecvTaskInit(int8_t *socketId)
{
	osThreadAttr_t threadAttr = {0};
	memset(&threadAttr, 0, sizeof(threadAttr));
	threadAttr.name 	  = "threadwssRecv";
	threadAttr.stack_size = THREAD_STACK_SIZE_WSS_RECV_TASK;
	threadAttr.priority   = osPriorityBelowNormal7;
#ifdef THREAD_STATIC
	threadAttr.stack_mem  = gRecvThreadStackMem;
	threadAttr.cb_mem	  = &gRecvThreadCbMem;
	threadAttr.cb_size	  = sizeof(StaticTask_t);
#endif

#if 0
	if (osThreadNew(threadWssRecv, (void *)socketId, &threadAttr) == NULL)
#else
    char serviceName[32] = {0};
    snprintf(serviceName, sizeof(serviceName), "service:/%s", threadAttr.name);
    Service_reg(serviceName, threadWssRecv, (void *)socketId, threadAttr.cb_mem, threadAttr.cb_size, threadAttr.stack_mem, threadAttr.stack_size, threadAttr.priority);
    if ((osThreadId_t)Service_start(serviceName) == NULL)
#endif
	{
		ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, openWssRecvTaskInit, P_ERROR, "Failed to create thread for websocket receive.");
		return false;
	}
	return true;
}


static void threadOpenWsscTrack(void *param)
{
	int8_t wssResult = E_WSS_OK;
	osStatus_t ret = osOK;

 	openWssQueueMsg_t queueMsg = {0};
	NmAtiNetifInfo netInfo;
	openWssClientCtx_t *context = NULL;
	memset(&queueMsg,0,sizeof(openWssQueueMsg_t));
	while(1)
	{
		ret = osMessageQueueGet(open_wss_msgqueue,&queueMsg,0,osWaitForever);
		EC_ASSERT(ret == osOK,ret,0,0);
		if((queueMsg.action > WSS_ACTION_INIT) && (queueMsg.action <= WSS_ACTION_CLOSE))
			context = &gOpenWssCtx[queueMsg.socketId];
		switch(queueMsg.action)
		{

			case WSS_ACTION_INIT:
			{
				uint32_t flags = 0;
				for(int cnt = 0; cnt < OPEN_WSS_CLIENT_NUM_MAX;cnt++)
				{
					if(gOpenWssCtx[cnt].status < WSS_STATUS_CLOSING)
					{
						flags = (OPEN_WSS_CREATE_SUCC | (cnt << 8));
						break;
					}						
				}
				ret = osThreadFlagsSet(queueMsg.threadId,flags);
			}
			break;
			case WSS_ACTION_CONNECT:
			{
				context = &gOpenWssCtx[queueMsg.socketId];
				if(context->status >= WSS_STATUS_CONNECT)
				{
					wssResult = E_WSS_ERR_REQ_PROCESSING;
					break;
				}
				//check network status;
				appGetNetInfoSync(0, &netInfo);
				 if(!((netInfo.netStatus == NM_NETIF_ACTIVATED)&&
				 	((netInfo.ipType == NM_NET_TYPE_IPV4)||
				 	(netInfo.ipType == NM_NET_TYPE_IPV6)||
				 	(netInfo.ipType == NM_NET_TYPE_IPV4V6))))
			 	{
			 		wssResult = E_WSS_ERR_NETWORK_NOT_ACTICVE;
					break;
		 		}
				//clear or create contex
				openWssCreateContext(context);
				if(!openWssCreateClientData(context))

				{
					openWssDeleteContext(context);
					wssResult = E_WSS_ERR_INTERNAL;
					break;
				}
				openWssClearContext(context);
				if(context->hostUrl != NULL)
				{
					free(context->hostUrl);
				}
				openWssMutexCreate(&context->mutex);
				context->hostUrl = (char *)malloc(strlen(queueMsg.url) + 1);
				EC_ASSERT(context->hostUrl != NULL,0,0,0);
				memset(context->hostUrl,0x00,strlen(queueMsg.url) + 1);
				memcpy(context->hostUrl,queueMsg.url,strlen((char *)queueMsg.url));
				context->clientCtx->method = queueMsg.method;
				context->actCb = queueMsg.actCb;
				context->onMessage = queueMsg.onMessage;

				//start req
				wssResult = httpConnect(context->clientCtx, context->hostUrl);
				if(wssResult == E_WSS_OK)
				{
					//START RECV TASK
					context->status = WSS_STATUS_CONNECT;
					if(context->isRecvRun == FALSE)
                    {
                        context->isRecvRun = TRUE;
                        bool recvTask = openWssRecvTaskInit(&context->socketId);
                        EC_ASSERT(recvTask == true,0,0,0);
                    }
					//SEND REQ
					if(queueMsg.method == HTTP_GET)
                    {
						char wssHeader[OPEN_WSS_HEAD_MAX_LEN] = {0};
						char wssKey[OPEN_WSS_KEY_LEN + 1] = {0};
						uint32_t headLen = 0;
						memset(wssHeader,0x00,OPEN_WSS_HEAD_MAX_LEN*sizeof(char));
						
						memset(wssKey,0x00,OPEN_WSS_KEY_LEN + 1);
						getWebSocketKey((char *)wssKey, OPEN_WSS_KEY_LEN);
						headLen = snprintf(wssHeader,OPEN_WSS_HEAD_MAX_LEN,"Connection: Upgrade\r\nUpgrade: websocket\r\nSec-WebSocket-Version: %d\r\nSec-WebSocket-Key: %s",OPEN_WSS_VERSION,wssKey);
						if(queueMsg.extention && queueMsg.extentionLen > 0)
						{
							snprintf(wssHeader + headLen,OPEN_WSS_HEAD_MAX_LEN - headLen,"\r\n%s",queueMsg.extention);
						}
						ECPLAT_PRINTF(UNILOG_PLAT_NETWORK,threadOpenWsscTrack,P_DEBUG,"%s",wssHeader);
						context->clientCtx->custHeader = wssHeader;
						//custom can set the contextType here
                        wssResult = httpSendRequest(context->clientCtx, context->hostUrl,(HTTP_METH)queueMsg.method, context->clientData);

                    }
                    else
                    {
                    	osThreadFlagsSet(queueMsg.threadId,OPEN_WSS_CONNECT_FAIL);
						openWssMutexDelete(context->mutex);
						context->mutex = NULL;
						openWssClearContext(context);
	                    openWssDeleteClientData(context);
	                    openWssDeleteContext(context);					
	                    context->status = WSS_STATUS_CLOSED;
						continue;
                    }

					if(wssResult == E_WSS_OK)
	                {
	                    context->status = WSS_STATUS_HANDSHAKE;
	                    ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadOpenWsscTrack_1, P_INFO, "send HTTP request OK");
						osThreadFlagsSet(queueMsg.threadId,OPEN_WSS_CONNECT_SUCC);
	                    if(context->timer == NULL)
	                        context->timer = osTimerNew((osTimerFunc_t)openWssRspTimerExp, osTimerOnce, (void *)&context->socketId, NULL);
	                    osTimerStart(context->timer, 30*1000);
	                }
	                else
	                {
	                    ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadOpenWsscTrack_2, P_VALUE, "send HTTP request failed,result=%d, disconnect peer",wssResult);
						osThreadFlagsSet(queueMsg.threadId,OPEN_WSS_CONNECT_FAIL);
						if(context->actCb)
							context->actCb(queueMsg.socketId,WSS_ACTION_CONNECT,wssResult);
	                    if(context->clientCtx != NULL){
	                        httpClose(context->clientCtx);
	                    }
						openWssMutexDelete(context->mutex);
						context->mutex = NULL;
						openWssClearContext(context);
	                    openWssDeleteClientData(context);
	                    openWssDeleteContext(context);
	                    context->status = WSS_STATUS_CLOSED;
	                }							
				}
				else
					osThreadFlagsSet(queueMsg.threadId,OPEN_WSS_CONNECT_FAIL);
			}
			break;
			case WSS_ACTION_SEND:
			{
				uint32_t flags = OPEN_WSS_SEND_INPROGRESS;
				uint8_t status = WSS_STATUS_CLOSED;
				openWssMutexAcquire(context->mutex);
				status = context->status;
				openWssMutexRelease(context->mutex);
				if(status != WSS_STATUS_ACTIVE)
				{
					flags = OPEN_WSS_SEND_FAIL;
					osThreadFlagsSet(queueMsg.threadId,flags);
				}
				osThreadFlagsSet(queueMsg.threadId,flags);

				if(status == WSS_STATUS_ACTIVE)
				{

					wssResult = wssSendAndEncodeFrame(context, queueMsg.payload, queueMsg.payloadLen,queueMsg.fin, queueMsg.frameType, queueMsg.mask);
				}
				if(context->actCb)
					context->actCb(queueMsg.socketId,WSS_ACTION_SEND,wssResult);
			}
			break;
			case WSS_ACTION_CLOSE:
			{				
            	openWssMutexAcquire(context->mutex);
                if(context->status >= WSS_STATUS_CLOSING)
                {
                    //peer addr close the socket
                    ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadOpenWssTrack_3, P_VALUE, "socketclose by peer addr or network reason");
					if(context->status == WSS_STATUS_ACTIVE)
					{
						
						wssSendAndEncodeFrame(context, queueMsg.payload, queueMsg.payloadLen,true, WSS_OCT_CLS, true);
						context->status = WSS_STATUS_CLOSING;
					}
					else
					{
						httpClose(context->clientCtx);
						if(context->actCb)
							context->actCb(queueMsg.socketId,WSS_ACTION_CLOSE,E_WSS_OK);
						openWssClearContext(context);
		                openWssDeleteContext(context);
		                openWssDeleteClientData(context);
		                //httpSleepVote(HTTP_EN_SLEEP);
		                context->status = WSS_STATUS_CLOSED;
					}                   
                }
				openWssMutexRelease(context->mutex);
				openWssMutexDelete(context->mutex);
				context->mutex = NULL;
				osThreadFlagsSet(queueMsg.threadId,OPEN_WSS_CLOSE_SUCC);
                break;
        	}
			default:
			break;
		}
		if(queueMsg.url != NULL)
			free(queueMsg.url);
		queueMsg.url = NULL;
		if(queueMsg.payload != NULL)
			free(queueMsg.payload);
		queueMsg.payload = NULL;
		if(queueMsg.extention)
			free(queueMsg.extention);
		queueMsg.extention = NULL;
	}
EXIT:
#if 0 // Service Manager
	osThreadExit();
#else
    Service_stop("service:/openWssEngTask");
#endif
}

/*----------------------------------------------------------------------------*
 *						GLOBAL FUNCTIONS									  *
 *----------------------------------------------------------------------------*/
int8_t openWssCreate(void)
{
	osStatus_t status = osError;
	int8_t 	socketId = -1;
	uint32_t flag = 0;
	openWssQueueMsg_t queueMsg = {0};
	memset(&queueMsg,0,sizeof(openWssQueueMsg_t));
	if(open_wss_msgqueue != NULL)
	{
		queueMsg.action = WSS_ACTION_INIT;
		queueMsg.threadId = (void *)osThreadGetId();
		ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, openWssCreate_00, P_DEBUG, "threadId(%p)", queueMsg.threadId);
		status = osMessageQueuePut(open_wss_msgqueue, &queueMsg, 0, 0);
		EC_ASSERT(status==osOK,status,0,0);

		flag = osThreadFlagsWait(OPEN_WSS_CREATE_COMPLT,osFlagsWaitAny, osWaitForever);
		EC_ASSERT(flag & OPEN_WSS_CREATE_COMPLT,flag,0,0);
		osThreadFlagsClear(flag);
		ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, openWssCreate, P_DEBUG, "FLAG(%x)", flag);
		if(flag & OPEN_WSS_CREATE_SUCC)
		{
			socketId = ((flag & 0x0000ff00) >> 8);
		}
	}
	ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, openWssCreate_1, P_DEBUG, "socketId(%d)", socketId);
	return socketId;
}


int32_t openWssConnect(int8_t socketId, char *url,openWssActionCb actCallback,openWssOnMessage onMessage,char *extra,uint32_t extraLen)
{
	osStatus_t status = osError;
	int32_t result = E_WSS_ERR_CONN;
	uint32_t flag = 0;
	openWssQueueMsg_t queueMsg = {0};
	memset(&queueMsg,0,sizeof(openWssQueueMsg_t));
	EC_ASSERT((url != NULL) && (strlen(url) < OPEN_WSS_URL_LEN_MAX) && (socketId >= 0) ,url,strlen(url),socketId);
	if(open_wss_msgqueue != NULL)
	{
		queueMsg.socketId = socketId;
		queueMsg.action = WSS_ACTION_CONNECT;
		queueMsg.method = HTTP_GET;
		queueMsg.threadId = (void *)osThreadGetId();
		queueMsg.url = malloc(strlen(url) + 1);
		EC_ASSERT(queueMsg.url != NULL,0,0,0);
		memset(queueMsg.url,0x00,strlen(url) + 1);
		memcpy(queueMsg.url,url,strlen(url));
		if(extra && extraLen > 0)
		{
			queueMsg.extention = (int8_t *)calloc(extraLen + 1,sizeof(char));
			EC_ASSERT(queueMsg.extention,0,0,0);
			memcpy(queueMsg.extention,extra,extraLen);
			queueMsg.extentionLen = extraLen;
		}
		queueMsg.actCb = actCallback;
		queueMsg.onMessage = onMessage;
		status = osMessageQueuePut(open_wss_msgqueue, &queueMsg, 0, 0);
		EC_ASSERT(status==osOK,status,0,0);
		flag = osThreadFlagsWait(OPEN_WSS_CONNECT_COMPLT,osFlagsWaitAny, osWaitForever);
		EC_ASSERT(flag & OPEN_WSS_CONNECT_COMPLT,flag,0,0);
		ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, openWssConnect_0, P_DEBUG, "flag(%x)", flag);
		osThreadFlagsClear(flag);
		if(flag & OPEN_WSS_CONNECT_SUCC)
		{
			result = E_WSS_OK;
		}
	}
	ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, openWssConnect_1, P_DEBUG, "result(%d)", result);
	return result;
}

int32_t openWssSend(int8_t socketId,char *data,uint32_t dataLen,bool fin,openWssOpcodeT frameType,bool mask)
{
	uint32_t flag = 0;
	openWssQueueMsg_t queueMsg = {0};
	memset(&queueMsg,0,sizeof(openWssQueueMsg_t));
	int32_t result = E_WSS_ERR_SOCKET_FAIL;
	osStatus_t status = osError;
	if(frameType < WSS_OCT_CONT || frameType > WSS_OCT_RSV)
		return E_WSS_ERR_PARAM_ERROR;
	if((frameType < WSS_OCT_CLS) && (data== NULL || dataLen == 0))
		return E_WSS_ERR_PARAM_ERROR;
	if(open_wss_msgqueue != NULL)
	{
		queueMsg.socketId = socketId;
		queueMsg.action = WSS_ACTION_SEND;
		queueMsg.threadId = (void *)osThreadGetId();
		if(data && dataLen > 0)
		{
			queueMsg.payload = (int8_t *)calloc(dataLen,sizeof(int8_t));
			EC_ASSERT(queueMsg.payload,0,0,0);
			memcpy(queueMsg.payload,data,dataLen);
		}
		queueMsg.fin = fin;
		queueMsg.mask = mask;
		queueMsg.payloadLen = dataLen;
		queueMsg.frameType = frameType;
		status = osMessageQueuePut(open_wss_msgqueue, &queueMsg, 0, 0);
		EC_ASSERT(status==osOK,status,0,0);
		flag = osThreadFlagsWait(OPEN_WSS_SEND_COMPLT,osFlagsWaitAny, osWaitForever);
		EC_ASSERT(flag & OPEN_WSS_SEND_COMPLT,flag,0,0);
		ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, openWssSend, P_DEBUG, "flag(%x)", flag);
		osThreadFlagsClear(flag);
		if(flag & OPEN_WSS_SEND_INPROGRESS)
			result = E_WSS_OK;
	}
	return result;
}


int32_t openWssClose(int8_t 	socketId)
{
	osStatus_t status = osError;
 	if(socketId < 0 || socketId >= OPEN_WSS_CLIENT_NUM_MAX)
		return E_WSS_ERR_CLOSED;
	uint32_t flag = 0;
	openWssQueueMsg_t queueMsg = {0};
	memset(&queueMsg,0,sizeof(openWssQueueMsg_t));
	if(open_wss_msgqueue != NULL)
	{
		int16_t statusCode = 1000;
		char *reason = "Normal connection closure";
		
		queueMsg.payload = (int8_t *)calloc(strlen(reason) + 2 + 1,sizeof(int8_t));
		EC_ASSERT(queueMsg.payload,0,0,0);
		queueMsg.payload[0] = (statusCode & 0xff00) >> 8;
		queueMsg.payload[1] = statusCode & 0x00ff;						
		snprintf((char *)queueMsg.payload + 2,strlen(reason),"%s",reason);
		queueMsg.payloadLen = strlen(reason) + 2;
		queueMsg.socketId = socketId;
		queueMsg.action = WSS_ACTION_CLOSE;
		queueMsg.threadId = (void *)osThreadGetId();
		
		status = osMessageQueuePut(open_wss_msgqueue, &queueMsg, 0, 0);
		EC_ASSERT(status==osOK,status,0,0);
		flag = osThreadFlagsWait(OPEN_WSS_CLOSE_COMPLT,osFlagsWaitAny, osWaitForever);
		EC_ASSERT(flag & OPEN_WSS_CLOSE_COMPLT,flag,0,0);
		osThreadFlagsClear(flag);
		ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, openWssClose, P_DEBUG, "FLAG(%x)", flag);
		if(flag & OPEN_WSS_CLOSE_SUCC)
		{
			return E_WSS_OK;
		}
	}
	return E_WSS_ERR_CLOSED;
}

bool openWssEngInit(void)
{
	osThreadAttr_t threadAttr = {0};
	
	if(open_wss_msgqueue == NULL)
	{
		open_wss_msgqueue = osMessageQueueNew(16, sizeof(openWssQueueMsg_t), NULL);
	}
	else
	{
		osMessageQueueReset(open_wss_msgqueue);
	}
	if(open_wss_msgqueue == NULL)
	{
		return false;
	}
	
	memset(&threadAttr, 0, sizeof(threadAttr));
	threadAttr.name 	  = "openWssEngTask";
	threadAttr.stack_size = THREAD_STACK_SIZE_WSS_ENG_TASK;
	threadAttr.priority   = osPriorityNormal;
#ifdef THREAD_STATIC
	threadAttr.stack_mem  = gwssThreadStackMem;
	threadAttr.cb_mem     = &gwssThreadCbMem;
	threadAttr.cb_size    = sizeof(StaticTask_t);
#endif
#if 0
	if (osThreadNew(threadOpenWsscTrack, NULL, &threadAttr) == NULL)
#else
    char serviceName[32] = {0};
    snprintf(serviceName, sizeof(serviceName), "service:/%s", threadAttr.name);
    Service_reg(serviceName, threadOpenWsscTrack, NULL, threadAttr.cb_mem, threadAttr.cb_size, threadAttr.stack_mem, threadAttr.stack_size, threadAttr.priority);
    if ((osThreadId_t)Service_start(serviceName) == NULL)
#endif
	{
		ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, openWssEngInit, P_ERROR, "Failed to create thread for threadOpenWsscTrack.");
	}
	
	return true;
}


