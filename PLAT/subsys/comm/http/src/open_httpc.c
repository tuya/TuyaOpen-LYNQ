/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    open_httpc.c
 * Description:  EC718 tcpip demo entry source file
 * History:      Rev1.0   2024-05-08
 *
 *http client send task
 *http client recv task
 *request :GET/POST/HEAD/PUT/DELETE
 *cache?
 ****************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include DEBUG_LOG_HEADER_FILE
#include <string.h>

#include "cmsis_os2.h"
#include "slpman.h"
#include "charge.h"

#include "ps_lib_api.h"
#include "ps_sim_if.h"
#include "networkmgr.h"
#include "netdb.h"

#include "open_httpc.h"
#include "servicemanager.h"

/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/
static openHttpcClientCtx_t gOpenHttpcCtx = {0};
static osMessageQueueId_t open_httpc_msgqueue = NULL;
bool gOpenHttpcRecvIsRunning = false;
static osTimerId_t openHttpsRspTimer = NULL;
static osMutexId_t    openHttpcMutex = NULL;

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

static bool openHttpRecvTaskInit(void);

/*----------------------------------------------------------------------------*
 *						GLOBAL VARIABLES									  *
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*
 * 					 PRIVATE FUNCTION DECLEARATION						      *
 *----------------------------------------------------------------------------*/
 static int8_t openHttpcMutexCreate(void)
{
    if(openHttpcMutex == NULL)
    {
        openHttpcMutex = osMutexNew(NULL);
    }
    if(openHttpcMutex == NULL)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

static int8_t openHttpcMutexAcquire(void)
{
    if (osMutexAcquire(openHttpcMutex, osWaitForever) != osOK)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

static void openHttpcMutexRelease(void)
{
    osMutexRelease(openHttpcMutex);
}

static void openHttpcMutexDelete(void)
{
    if(openHttpcMutex != NULL)
    {
        osMutexDelete(openHttpcMutex);
        openHttpcMutex = NULL;
    }
}

static void openHttpcClearContext(openHttpcClientCtx_t * context)
{	
	if(context == NULL)
		return;	
	context->urcFlag = false; 
	context->status = HTTPC_STATUS_CLOSED;
	context->exDataSize = 0;
	context->cb = NULL;
	if(context->exData != NULL)
	{
		free(context->exData);
		context->exData = NULL;
	}
}
static bool openHttpcCreateContext(openHttpcClientCtx_t * context)
{
	if(context == NULL)
		return false;
	if(context->clientCtx == NULL)
	{
		context->clientCtx = malloc(sizeof(HttpClientContext));
		if(context->clientCtx == NULL){
			ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, httpcCreateContext, P_VALUE, "no memory");
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
	context->urcFlag = false;
	
	return true;
}

static void openHttpcDeleteContext(openHttpcClientCtx_t * context)
{
	if(context == NULL)
		return;
	HttpClientContext* clientContext = context->clientCtx;

	if(clientContext != NULL)
	{
#if 0
		if(clientContext->caCert != NULL)
		{
			free((char*)clientContext->caCert);
		}
		if(clientContext->clientCert != NULL)
		{
			free((char*)clientContext->clientCert);
		}
#endif
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

static bool openHttpcCreateClientData(openHttpcClientCtx_t * context)
{
    if(context == NULL)
        return false;

    if(context->clientData == NULL)
    {
        context->clientData = malloc(sizeof(HttpClientData));
        if(context->clientData == NULL){
            ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, openHttpcCreateClientData_1, P_VALUE, "no memory");
            return false;
        }
        memset(context->clientData, 0, sizeof(HttpClientData));
        context->clientData->headerBufLen = OPEN_HTTPC_RSP_HEAD_BUFFER_SIZE;
        context->clientData->headerBuf = malloc(OPEN_HTTPC_RSP_HEAD_BUFFER_SIZE);
        if(context->clientData->headerBuf == NULL){
            ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, openHttpcCreateClientData_2, P_VALUE, "no memory");
            free(context->clientData);
            context->clientData = NULL;
            return false;
        }
        context->clientData->respBufLen = OPEN_HTTPC_RSP_CONTENT_BUFFER_SIZE;
        context->clientData->respBuf = malloc(OPEN_HTTPC_RSP_CONTENT_BUFFER_SIZE);
        if(context->clientData->respBuf == NULL){
            ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, openHttpcCreateClientData_3, P_VALUE, "no memory");
            free(context->clientData->headerBuf);
            context->clientData->headerBuf = NULL;
            free(context->clientData);
            context->clientData = NULL;
            return false;
        }
    }
    return true;
}

static void openHttpcDeleteClientData(openHttpcClientCtx_t * context)
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

static void openHttpsRspTimerExp(void)
{
    openHttpcQueueMsg_t queueMsg = {0};
    osStatus_t status;
	memset(&queueMsg,0,sizeof(openHttpcQueueMsg_t));
    //httpRetInd(HTTPAT_REQ_TIMEOUT);
    queueMsg.action = HTTPC_ACTION_CLOSE;
    status = osMessageQueuePut(open_httpc_msgqueue, &queueMsg, 0, 0);
    EC_ASSERT(status==osOK,status,0,0);
}


static void threadOpenHttpcRecv(void *param)
{
	int32_t result = E_HTTPC_OK;
	int isRecvTaskRunning = TRUE;
	uint16_t headerLen = 0;
	openHttpcQueueMsg_t queueMsg = {0};
	osStatus_t status;
	fd_set readFs,errorFs;
	struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    int httpSockFs = -1,ret;
	openHttpcClientCtx_t *context = &gOpenHttpcCtx;
	while(1)
	{
		switch(context->status)
		{
			case HTTPC_STATUS_ACTIVE:
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
                        if(osTimerIsRunning(openHttpsRspTimer) != 0)
                        {
                            ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadOpenHttpcRecv_0, P_INFO, "stop timer");
                            osTimerStop(openHttpsRspTimer);
                            osTimerDelete(openHttpsRspTimer);
                            openHttpsRspTimer = NULL;
                        }
                        if(FD_ISSET(context->clientCtx->socket, &readFs))
                        {
                            do
                            {
                                openHttpcMutexAcquire();

                                if(context->status != HTTPC_STATUS_ACTIVE)//user interrupted the request
                                {
                                    ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadOpenHttpcRecv_1, P_VALUE, "httpMutexRelease");
                                    openHttpcMutexRelease();
                                    break;
                                }

                                memset(context->clientData->headerBuf, 0, context->clientData->headerBufLen);
                                memset(context->clientData->respBuf, 0, context->clientData->respBufLen);
                                result = httpRecvResponse(context->clientCtx, context->clientData);

                                if(result == E_HTTPC_OK)
                                {
                                    //output data
                                    headerLen = strlen(context->clientData->headerBuf);
									if(headerLen > 0)
									{
										if(context->cb)
											context->cb(result,HTTPC_RTEDATA_HEADER,context->clientData->headerBuf,headerLen);
									}
						            if(context->clientData->blockContentLen > 0)
						            {
						            	if(context->cb)
											context->cb(result,HTTPC_RTEDATA_CONTEXT,context->clientData->respBuf,context->clientData->blockContentLen);
						            }		
                                    ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadOpenHttpcRecv_2, P_INFO, "all data recevied, to close socket");
                                    queueMsg.action = HTTPC_ACTION_CLOSE;//request has recevied all response data, close socket
                                    status = osMessageQueuePut(open_httpc_msgqueue, &queueMsg, 0, 0);
                                    EC_ASSERT(status==osOK,status,0,0);

                                }
                                else if(result == E_HTTPC_MOREDATA)
                                {
                                	headerLen = strlen(context->clientData->headerBuf);
									if(headerLen > 0)
									{
										if(context->cb)
											context->cb(result,HTTPC_RTEDATA_HEADER,context->clientData->headerBuf,headerLen);
										//ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadOpenHttpcRecv_4, P_INFO, "httpc rsp header: %s", context->clientData->headerBuf);
									}
						            if(context->clientData->blockContentLen > 0)
						            {
						            	if(context->cb)
											context->cb(result,HTTPC_RTEDATA_CONTEXT,context->clientData->respBuf,context->clientData->blockContentLen);
						            	//ECPLAT_DUMP(UNILOG_PLAT_NETWORK, threadOpenHttpcRecv_5, P_INFO, "http recv: ", context->clientData->blockContentLen, context->clientData->respBuf);
						            }									
                                }
                                else if(result == E_HTTPC_ERR_CONN)
                                {
                                    ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadOpenHttpcRecv_6, P_INFO, "httpRecvResponse return HTTP_CONN release mutex try recv data again");
                                }
                                else
                                {
                                    if(result == E_HTTPC_ERR_NO_MEMORY){
                                        //httpRetInd(HTTPAT_MEMORY_NOT_ENOUGH);
                                    }else{
                                        //httpRetInd(HTTPAT_SOCKET_ERROR);
                                    }

                                    int mErr = sock_get_errno(context->clientCtx->socket);
                                    ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadOpenHttpcRecv_7, P_VALUE, "httpRecvResponse return %d error=%d, close socket", result,mErr);
                                    queueMsg.action = HTTPC_ACTION_CLOSE;
                                    status = osMessageQueuePut(open_httpc_msgqueue, &queueMsg, 0, 0);
                                    EC_ASSERT(status==osOK,status,0,0);
                                }
                                openHttpcMutexRelease();
                            }while(result == HTTP_MOREDATA);
                        }
                        else if(FD_ISSET(context->clientCtx->socket, &errorFs))
                        {
                            int mErr = sock_get_errno(context->clientCtx->socket);
                            if(socket_error_is_fatal(mErr))
                            {
                                //httpRetInd(HTTPAT_SOCKET_ERROR);                                
                                ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadOpenHttpcRecv_8, P_VALUE, "selected fatal error socket:%d, errno:%d", context->clientCtx->socket, mErr);
                                queueMsg.action = HTTPC_ACTION_CLOSE;

                                status = osMessageQueuePut(open_httpc_msgqueue, &queueMsg, 0, 0);
                                EC_ASSERT(status==osOK,status,0,0);
                            }
                            else
                            {
                                ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadOpenHttpcRecv_9, P_VALUE, "selected not fatal error socket:%d, errno:%d", context->clientCtx->socket, mErr);
                            }
                        }
                    }
                    else
                    {
                        ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadOpenHttpcRecv_10, P_VALUE, "recv select return:%d, errno:%d", ret, errno);
                    }
				}
				else
				{
					ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadOpenHttpcRecv_11, P_VALUE, "no valid socket exit recv task, errno:%d", errno);
				}
				break;
			}
			case HTTPC_STATUS_CONNECT:
            {
                if(context->clientCtx->socket == -1){
                    ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadOpenHttpcRecv_12, P_VALUE, "in HTTPSTAT_CONNECTED socket invalid exit recv task, errno:%d", errno);
                    isRecvTaskRunning = FALSE;
                }
                break;
            }

            case HTTPC_STATUS_CLOSED:
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
		osDelay(10);
	}
    ostaskENTER_CRITICAL();
    gOpenHttpcRecvIsRunning = FALSE;
    ostaskEXIT_CRITICAL();
#if 0 // Service Manager
    osThreadExit();
#else
    Service_stop("service:/openHttpcRecvTask");
#endif
}

 static void threadOpenHttpcTrack(void *param)
 {
 	int32_t result = E_HTTPC_OK;
	uint8_t waitloop = 10;
 	openHttpcQueueMsg_t queueMsg = {0};
	memset(&queueMsg,0,sizeof(openHttpcQueueMsg_t));
	NmAtiNetifInfo netInfo;
	openHttpcClientCtx_t *context = &gOpenHttpcCtx;
	openHttpcMutexCreate();
	 while(1)
	 {
	 	if(osMessageQueueGet(open_httpc_msgqueue,&queueMsg,0,osWaitForever) == osOK)
 		{
 			switch(queueMsg.action)
			{
				case HTTPC_ACTION_SEND:
				{
					//check http status
					if((gOpenHttpcCtx.status == HTTPC_STATUS_CONNECT) || 
						(gOpenHttpcCtx.status == HTTPC_STATUS_ACTIVE))
					{
						result = E_HTTPC_ERR_REQ_PROCESSING;
						break;
					}
					//check network status;
					appGetNetInfoSync(0, &netInfo);
					 if(!((netInfo.netStatus == NM_NETIF_ACTIVATED)&&
					 	((netInfo.ipType == NM_NET_TYPE_IPV4)||
					 	(netInfo.ipType == NM_NET_TYPE_IPV6)||
					 	(netInfo.ipType == NM_NET_TYPE_IPV4V6))))
				 	{
				 		result = E_HTTPC_ERR_NETWORK_NOT_ACTICVE;
						break;
			 		}
					//clear or create contex
					openHttpcClearContext(context);
					openHttpcCreateContext(context);
					if(!openHttpcCreateClientData(context))
					{
						openHttpcDeleteContext(context);
						result = E_HTTPC_ERR_INTERNAL;
						break;
					}
					openHttpcClearContext(context);
					if(context->hostUrl != NULL)
					{
						free(context->hostUrl);
					}
					context->hostUrl = (char *)malloc(strlen(queueMsg.url) + 1);
					EC_ASSERT(context->hostUrl != NULL,0,0,0);
					memset(context->hostUrl,0x00,strlen(queueMsg.url) + 1);
					memcpy(context->hostUrl,queueMsg.url,strlen((char *)queueMsg.url));
					context->clientCtx->method = queueMsg.method;
					context->clientCtx->custHeader = queueMsg.extHeader;
					context->cb = (queueMsg.cb != NULL ? queueMsg.cb : NULL);
	
					//start req
					result = httpConnect(context->clientCtx, context->hostUrl);
					if(context->cb)
						context->cb(result,HTTPC_RTEDATA_IDLE,NULL,0);
					if(result == E_HTTPC_OK)
					{
						//START RECV TASK
						context->status = HTTPC_STATUS_CONNECT;
						if(gOpenHttpcRecvIsRunning == FALSE)
	                    {
	                        gOpenHttpcRecvIsRunning = TRUE;
	                        bool recvTask = openHttpRecvTaskInit();
	                        EC_ASSERT(recvTask == true,0,0,0);
	                    }
						//SEND REQ
						if(queueMsg.method == HTTP_POST)
                        {
                        	EC_ASSERT(queueMsg.extra != NULL,queueMsg.extra,0,0);
							if(context->exData != NULL)
							{
								free(context->exData);
								context->exData = NULL;
							}
							context->exData = (char *)malloc(queueMsg.exDataLen);
							EC_ASSERT(context->exData != NULL,0,0,0);
							memset(context->exData,0x00,queueMsg.exDataLen);
							memcpy(context->exData,queueMsg.extra,queueMsg.exDataLen);
							context->exDataSize = queueMsg.exDataLen;
						
                            context->clientData->postBuf = context->exData;
                            context->clientData->postBufLen = context->exDataSize;
#ifdef FEATURE_SUBSYS_AI_DOUBAO_ENABLE
                            extern char rtcPostHeader[1024];
                            context->clientCtx->custHeader = (char *)rtcPostHeader;
#endif
							//custom can set the contextType here
                            result = httpSendRequest(context->clientCtx, context->hostUrl,(HTTP_METH)queueMsg.method, context->clientData);

                        }
                        else
                        {
                         	result = httpSendRequest(context->clientCtx, context->hostUrl,(HTTP_METH)queueMsg.method, context->clientData);
                        }

						if(result == E_HTTPC_OK)
		                {
		                    context->status = HTTPC_STATUS_ACTIVE;
		                    ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadOpenHttpcTrack_1, P_INFO, "send HTTP request OK");
		                    if(openHttpsRspTimer == NULL)
		                        openHttpsRspTimer = osTimerNew((osTimerFunc_t)openHttpsRspTimerExp, osTimerOnce, NULL, NULL);
		                    osTimerStart(openHttpsRspTimer, 30*1000);
		                }
		                else
		                {
		                    ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadOpenHttpcTrack_2, P_VALUE, "send HTTP request failed,result=%d, disconnect peer",result);	                    
							if(context->cb)
								context->cb(result,HTTPC_RTEDATA_IDLE,NULL,0);
		                    if(context->clientCtx != NULL){
		                        httpClose(context->clientCtx);
		                    }
							openHttpcClearContext(context);
		                    openHttpcDeleteClientData(context);
		                    openHttpcDeleteContext(context);
		                    context->status = HTTPC_STATUS_CLOSED;
		                }							
					}
				}
				break;
				case HTTPC_ACTION_STOP:
				{
	                ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadOpenHttpcTrack_3, P_INFO, "recv stop command httpMutexAcquire");
	                openHttpcMutexAcquire();
	                if(osTimerIsRunning(openHttpsRspTimer) != 0)
	                {
	                    ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadOpenHttpcTrack_4, P_INFO, "stop response timer");
	                    osTimerStop(openHttpsRspTimer);
	                    osTimerDelete(openHttpsRspTimer);
	                    openHttpsRspTimer = NULL;
	                }
	                result = httpClose(context->clientCtx);
	                ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadOpenHttpcTrack_5, P_INFO, "http close 0x%x",result);
	                openHttpcMutexRelease();

	                //wait for recv task really deleted
	                if(gOpenHttpcRecvIsRunning == TRUE)//no conenct recv task should delete itself
	                {
	                    while(waitloop)//max 10s
	                    {
	                        if(gOpenHttpcRecvIsRunning == FALSE)//deteled,break
	                        {
	                            break;
	                        }
	                        osDelay(1000);
	                        waitloop--;
	                    }
	                    if(waitloop==0)
	                    {
	                        ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadOpenHttpcTrack_6, P_INFO, " wait recv task deleted tiemout");
	                    }
	                }
	                ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadOpenHttpcTrack_7, P_INFO, " wait recv task deleted %d and send task exit",gOpenHttpcRecvIsRunning);

	                gOpenHttpcRecvIsRunning = FALSE;
	                //applSendCmsCnf(httpAtCxt->reqhandle, APPL_RET_SUCC, APPL_HTTP, APPL_HTTP_STOP_CNF, primSize, &cnfMsg);
	                openHttpcClearContext(context);
	                openHttpcDeleteContext(context);
	                openHttpcDeleteClientData(context);
	                //httpSleepVote(HTTP_EN_SLEEP);
	                context->status = HTTPC_STATUS_CLOSED;
	                break;
	            }
				case HTTPC_ACTION_CLOSE:
				{
                	openHttpcMutexAcquire();
	                if(context->status != HTTPC_STATUS_CLOSED)
	                {
	                    //peer addr close the socket
	                    ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadOpenHttpcTrack_9, P_VALUE, "socketclose by peer addr or network reason");
	                    httpClose(context->clientCtx);
	                }
					openHttpcClearContext(context);
	                openHttpcDeleteContext(context);
	                openHttpcDeleteClientData(context);
	                //httpSleepVote(HTTP_EN_SLEEP);
	                context->status = HTTPC_STATUS_CLOSED;
	                openHttpcMutexRelease();
	                break;
            	}
				default:
				break;
			}
 		}

		if(queueMsg.action == HTTPC_ACTION_SEND)
		{
			if(queueMsg.url != NULL)
			{
				free(queueMsg.url);
				queueMsg.url = NULL;
			}
			if(queueMsg.extra != NULL)
			{
				free(queueMsg.extra);
				queueMsg.extra = NULL;
			}
			if(queueMsg.extHeader != NULL)
			{
				free(queueMsg.extHeader);
				queueMsg.extHeader = NULL;
			}
		}
	 }
	 if(open_httpc_msgqueue != NULL)
    {
        osMessageQueueDelete(open_httpc_msgqueue);
        open_httpc_msgqueue = NULL;
    }
    openHttpcMutexDelete();
#if 0 // Service Manager
    osThreadExit();
#else
    Service_stop("service:/openHttpcSendTask");
#endif
 }

static bool openHttpRecvTaskInit(void)
{
	osThreadAttr_t threadAttr = {0};
	memset(&threadAttr, 0, sizeof(threadAttr));
    threadAttr.name       = "openHttpcRecvTask";
    threadAttr.stack_size = THREAD_STACK_SIZE_HTTPC_RECV_TASK;
    threadAttr.priority   = osPriorityNormal;
#if 0
    if (osThreadNew(threadOpenHttpcRecv, NULL, &threadAttr) == NULL)
#else
    char serviceName[32] = {0};
    snprintf(serviceName, sizeof(serviceName), "service:/%s", threadAttr.name);
    Service_reg(serviceName, threadOpenHttpcRecv, NULL, threadAttr.cb_mem, threadAttr.cb_size, threadAttr.stack_mem, threadAttr.stack_size, threadAttr.priority);
    if ((osThreadId_t)Service_start(serviceName) == NULL)
#endif
    {
		ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, openHttpRecvTaskInit, P_ERROR, "Failed to create thread for threadOpenHttpcReq.");
		return false;
    }
	return true;
}

 /*----------------------------------------------------------------------------*
  * 					 GLOBAL FUNCTIONS									   *
  *----------------------------------------------------------------------------*/
int32_t openHttpcGet(char *url,const char *extHeader,uint32_t extHdrLen,openHttpcActionCb cb)
{
	osStatus_t status = osError;
	openHttpcQueueMsg_t queueMsg = {0};
	memset(&queueMsg,0,sizeof(openHttpcQueueMsg_t));
	EC_ASSERT(((url != NULL) || (strlen(url) >= OPEN_HTTPC_URL_LEN_MAX)),url,strlen(url),0);
	if(open_httpc_msgqueue != NULL)
	{
		queueMsg.action = HTTPC_ACTION_SEND;
		queueMsg.method = HTTP_GET;
		queueMsg.url = malloc(strlen(url) + 1);
		EC_ASSERT(queueMsg.url != NULL,0,0,0);
		memset(queueMsg.url,0x00,strlen(url) + 1);
		memcpy(queueMsg.url,url,strlen(url));
		if(extHeader && extHdrLen > 0)
		{
			queueMsg.extHeader = calloc(extHdrLen+1,sizeof(char));
			EC_ASSERT(queueMsg.extHeader != NULL,0,0,0);
			memcpy(queueMsg.extHeader,extHeader,extHdrLen);
		}
		queueMsg.cb = cb;
		status = osMessageQueuePut(open_httpc_msgqueue, &queueMsg, 0, 0);
        EC_ASSERT(status==osOK,status,0,0);
		status = osOK;
	}
     return status;
}
int32_t openHttpcPost(char *url,const char *extHeader,uint32_t extHdrLen,char *data,uint32_t dataLen,openHttpcActionCb cb)
{
	osStatus_t status = osError;
	uint32_t exDataLen = 0;
	openHttpcQueueMsg_t queueMsg = {0};
	memset(&queueMsg,0,sizeof(openHttpcQueueMsg_t));
	EC_ASSERT(((url != NULL) || (strlen(url) >= OPEN_HTTPC_URL_LEN_MAX) || (data != NULL)),url,strlen(url),data);
		
	if(open_httpc_msgqueue != NULL)
	{
		queueMsg.action = HTTPC_ACTION_SEND;
		queueMsg.method = HTTP_POST;
		queueMsg.url = (char *)malloc(strlen(url) + 1);
		EC_ASSERT(queueMsg.url != NULL,0,0,0);
		memset(queueMsg.url,0x00,strlen(url) + 1);
		memcpy(queueMsg.url,url,strlen(url));
		if(extHeader && extHdrLen > 0)
		{
			queueMsg.extHeader = calloc(extHdrLen,sizeof(char));
			EC_ASSERT(queueMsg.extHeader != NULL,0,0,0);
			memcpy(queueMsg.extHeader,extHeader,extHdrLen);
		}
		exDataLen =  (dataLen > 0 ? dataLen : strlen(data));
		queueMsg.extra = (int8_t *)malloc(exDataLen);
		EC_ASSERT(queueMsg.extra != NULL,0,0,0);
		memset(queueMsg.extra,0x00,exDataLen);
		memcpy(queueMsg.extra,data,exDataLen);
		queueMsg.exDataLen = exDataLen;
		queueMsg.cb = cb;
		status = osMessageQueuePut(open_httpc_msgqueue, &queueMsg, 0, 0);
		EC_ASSERT(status==osOK,status,0,0);
		status = osOK;
	}
    return status;
}



bool openHttpcEngInit(void)
{
	osThreadAttr_t threadAttr = {0};
	
	if(open_httpc_msgqueue == NULL)
    {
        open_httpc_msgqueue = osMessageQueueNew(16, sizeof(openHttpcQueueMsg_t), NULL);
    }
    else
    {
        osMessageQueueReset(open_httpc_msgqueue);
    }
    if(open_httpc_msgqueue == NULL)
    {
        return false;
    }
		
	memset(&threadAttr, 0, sizeof(threadAttr));
	threadAttr.name       = "openHttpcSendTask";
    threadAttr.stack_size = THREAD_STACK_SIZE_HTTPC_SEND_TASK;
    threadAttr.priority   = osPriorityBelowNormal7;
#if 0
    if (osThreadNew(threadOpenHttpcTrack, NULL, &threadAttr) == NULL)
#else
    char serviceName[32] = {0};
    snprintf(serviceName, sizeof(serviceName), "service:/%s", threadAttr.name);
    Service_reg(serviceName, threadOpenHttpcTrack, NULL, threadAttr.cb_mem, threadAttr.cb_size, threadAttr.stack_mem, threadAttr.stack_size, threadAttr.priority);
    if ((osThreadId_t)Service_start(serviceName) == NULL)
#endif
    {
		ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, openHttpcEngInit, P_ERROR, "Failed to create thread for threadOpenHttpcTrack.");
    }
	
    return true;
}

