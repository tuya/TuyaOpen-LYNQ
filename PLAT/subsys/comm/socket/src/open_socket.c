/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    main_tcpip.c
 * Description:  EC718 tcpip open socket entry source file
 * History:      Rev1.0   2024-04-08
 *
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
#include "open_socket.h"
#include "servicemanager.h"
/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*
 *						GLOBAL VARIABLES									  *
 *----------------------------------------------------------------------------*/
static osMessageQueueId_t gSocketQueue = NULL;
static socketHandler_t gaSocketHandler[OPEN_TCPIP_SOCKLET_ID_MAX] = {0};
openSocketNode_t *head = NULL;

/*----------------------------------------------------------------------------*
 *                      PRIVATE FUNCTION DECLEARATION                         *
 *----------------------------------------------------------------------------*/

int32_t socketStateListPrepend(openSocketNode_t * newNode)
{
	int32_t Ret = 0;

    if (newNode != NULL)
	{
		newNode->next = head;
		head = newNode;
	}

	Ret = 1;
	return Ret;
}

int32_t socketStateListRemove(openSocketNode_t * dstNode)
{
	int32_t Ret = 0;
	openSocketNode_t *tempNode = NULL;
	openSocketNode_t *tempLastnode = NULL;

	tempNode = head;
	while(tempNode != NULL)
	{
		if(tempNode != dstNode)
		{
			tempLastnode = tempNode;
			tempNode = tempNode->next;
		}
		else
		{
			if(tempNode == head)
			{
				head = head->next;
			}
			else
			{
				tempLastnode->next = tempNode->next;
			}
			tempNode->Release(&(tempNode->param));			
            free(tempNode);
			Ret = 1;
			break;
		}
	}	
	return Ret;
}

int32_t socketStateListFree(void)
{
    int32_t Ret = 1;

    openSocketNode_t * curNode = NULL;
    openSocketNode_t * nextNode = NULL;

    curNode = head;
    while(curNode != NULL)
    {
        nextNode = curNode->next;
        socketStateListRemove(curNode);
        curNode = nextNode;
    }

	return Ret;
}

static int32_t socketStateListOperation(openSocketNode_t *node)
{
	int32_t i_op_status = 0;

    if (   (node != NULL)
        && (node->Function != NULL))
	{
            i_op_status = node->Function(&(node->param));
    }
    return i_op_status;
}

uint8_t socketStateListDoCycle(void)
{
	uint8_t ret = 0;
	int32_t i_op_status = 0;
	openSocketNode_t *nextNode = NULL;
	openSocketNode_t *curNode = NULL;
	curNode = head;
	while(curNode != NULL)
	{
		ret = 1;
		nextNode = curNode->next;

        i_op_status = socketStateListOperation(curNode);
        if(i_op_status == 0)
        {
            socketStateListRemove(curNode);
        }
        curNode = nextNode;
		osDelay(500);
	}
	return ret;
}


int32_t openSocketStateUrcCallbackFunc(void * param)
{
	int32_t ret = 1;
	int32_t recvData = -1;
	uint8_t socketId = *((uint8_t *)param);
	socketHandler_t * SocketPtr = &gaSocketHandler[socketId];
	if((socketId >= OPEN_TCPIP_SOCKLET_ID_MAX) || 
		(SocketPtr->sockfd < 0) || (SocketPtr->socketState < E_SOCKET_STATE_CONNECT))
		return ret;
	switch(SocketPtr->appConfigInfo.socketType)
	{
		case OPEN_TCPIP_SOCKET_TYPE_TCPC:
		case OPEN_TCPIP_SOCKET_TYPE_UDPC:
		{ 
			struct timeval tv={0,5000};
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(SocketPtr->sockfd, &readfds);
            recvData = select(FD_SETSIZE, &readfds, NULL, NULL, &tv);			

			if( recvData != 0)
			{
				if(SocketPtr->urcFlag == 0)
				{
					SocketPtr->urcFlag = 1;
					if(SocketPtr->appConfigInfo.ssCb)
					{
						SocketPtr->appConfigInfo.ssCb(socketId,recvData);
					}
				}
			}
			if(recvData < 0)
			{
				openSocketClientTracker(socketId,E_SOCKET_ACTION_CLOSE,NULL);
				ret = 0;
			}
		}
		break;
		case OPEN_TCPIP_SOCKET_TYPE_TCPS:
		break;
		default:
		break;
	}
	
	return ret;
}

int32_t openSocketStateUrcCallbackRelease(void * param)
{
    return 1;
}

static void SocketConfigDeinit(uint8_t socketId)
{
	socketHandler_t * SocketPtr = &gaSocketHandler[socketId];
	if(SocketPtr->rxBuff != NULL)
	{
		free(SocketPtr->rxBuff);
		SocketPtr->rxBuff = NULL;
	}
	if(SocketPtr->localIp != NULL)
	{
		free(SocketPtr->localIp);
		SocketPtr->localIp = NULL;
	}
	if(SocketPtr->appConfigInfo.hostUrl != NULL)
	{
		free(SocketPtr->appConfigInfo.hostUrl);
		SocketPtr->appConfigInfo.hostUrl = NULL;
	}
	socketStateListRemove(SocketPtr->stateNode);
	memset(SocketPtr,0x00,sizeof(socketHandler_t));
	memset(&SocketPtr->appConfigInfo,0x00,sizeof(socketConfig_t));
	SocketPtr->sockfd = -1;		
}

static void threadTcpipStateTrigger(void *param)
{
	while(1)
	{
		if(socketStateListDoCycle() == 0)
		{
			osDelay(500);
		}		
	}
}

static void threadOpenSocket(void *param)
{
	uint8_t       	pinState    = CMI_SIM_PIN_STATE_UNKNOWN;	
	int8_t			Result = E_SOCKET_OK;
	uint8_t socketId;
	uint8_t action; 
	void * extra = NULL;
	int32_t 		ret = 0;
	NmAtiNetifInfo 	pNetInfo;
	queueTcpipSocket_t queue = {0};
	
	//step 1:check SIM card
	do
	{
		osDelay(1000);
		ret = simGetPinStateSync(&pinState);
		 if(ret != CME_SUCC || pinState != CMI_SIM_PIN_STATE_READY)
	 	{
			ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadOpenSocket_0, P_INFO, "sim fail,ret [%d] pinState [%d]", ret, pinState);			
			continue;
		}

		//step 2:check network
		ret = appGetNetInfoSync(1, &pNetInfo);
		if((ret != CMS_RET_SUCC) || (pNetInfo.netStatus != NM_NETIF_ACTIVATED))
		{
			ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadOpenSocket_1, P_INFO, "network fail ,ret [%d] netStatus [%d]",pNetInfo.netStatus);
			continue;
		}
		break;
	}
	while(1);
	gSocketQueue = osMessageQueueNew(QUEUE_SIZE_TCPIP, sizeof(queue), NULL);
	if(gSocketQueue == NULL)
	{
		return;
	}
	memset(gaSocketHandler,0x00,sizeof(socketHandler_t) * OPEN_TCPIP_SOCKLET_ID_MAX);
	while(1)
	{
		memset(&queue, 0, sizeof(queue));
		Result = E_SOCKET_OK;
        if (osMessageQueueGet(gSocketQueue, &queue, 0, osWaitForever) == osOK)
    	{
    		socketId = queue.socketId;
			action = queue.action;
    		switch(action)
			{
				case E_SOCKET_ACTION_INIT:
				{
					if(gaSocketHandler[socketId].socketState > E_SOCKET_STATE_SET)
					{
						Result = E_SOCKET_ERR_CONNET;
						break;
					}
					if((queue.value.config.socketType >= OPEN_TCPIP_SOCKET_TYPE_MAX) || 
							((queue.value.config.socketType != OPEN_TCPIP_SOCKET_TYPE_TCPS) && (queue.value.config.hostUrl == NULL)) || 
							(queue.value.config.port > OPEN_TCPIP_SOCKET_PORT_INDEX_MAX) || 
							(queue.value.config.rxBufSize >= OPEN_TCPIP_SOCKET_RXBUF_SIZE_MAX))
					{
						Result = E_SOCKET_ERR_RARAM;
						break;
					}
					size_t xWantedSize = ( queue.value.config.rxBufSize == 0 ? 1500: queue.value.config.rxBufSize);
					
					if(gaSocketHandler[socketId].rxBuff != NULL)
					{							
						gaSocketHandler[socketId].rxBuff = (int8_t *)realloc(gaSocketHandler[socketId].rxBuff, xWantedSize);
						EC_ASSERT(gaSocketHandler[socketId].rxBuff != NULL,0,0,0);
					}
					else
					{
						gaSocketHandler[socketId].rxBuff = (int8_t *)malloc(xWantedSize * sizeof(int8_t));
						if(gaSocketHandler[socketId].rxBuff == NULL)
						{
							EC_ASSERT(gaSocketHandler[socketId].rxBuff != NULL,0,0,0);
						}
						memset(gaSocketHandler[socketId].rxBuff,0x00,xWantedSize *sizeof(uint8_t));
						gaSocketHandler[socketId].appConfigInfo.rxBufSize = queue.value.config.rxBufSize;							
					}
					if(queue.value.config.socketType != OPEN_TCPIP_SOCKET_TYPE_TCPS)
					{
						if(gaSocketHandler[socketId].appConfigInfo.hostUrl != NULL)
						{							
							gaSocketHandler[socketId].appConfigInfo.hostUrl = \
								(int8_t *)realloc(gaSocketHandler[socketId].appConfigInfo.hostUrl, \
								strlen((char *)queue.value.config.hostUrl) + 1);
							EC_ASSERT(gaSocketHandler[socketId].appConfigInfo.hostUrl != NULL,0,0,0);
						}
						else
						{
							gaSocketHandler[socketId].appConfigInfo.hostUrl = (int8_t *)malloc(strlen((char *)queue.value.config.hostUrl) + 1);
							EC_ASSERT(gaSocketHandler[socketId].appConfigInfo.hostUrl != NULL,0,0,0);
							memset(gaSocketHandler[socketId].appConfigInfo.hostUrl,0x00,strlen((char *)queue.value.config.hostUrl) + 1);
							memcpy(gaSocketHandler[socketId].appConfigInfo.hostUrl,queue.value.config.hostUrl,strlen((char *)queue.value.config.hostUrl));										
						}
					}
					gaSocketHandler[socketId].appConfigInfo.port = queue.value.config.port;
					gaSocketHandler[socketId].appConfigInfo.saCb = queue.value.config.saCb;
					gaSocketHandler[socketId].appConfigInfo.ssCb = queue.value.config.ssCb;
					gaSocketHandler[socketId].appConfigInfo.socketType = queue.value.config.socketType;

					gaSocketHandler[socketId].socketState = E_SOCKET_STATE_SET;
					break;
				}
				break;
				case E_SOCKET_ACTION_OPEN:
				{
					if(gaSocketHandler[socketId].socketState != E_SOCKET_STATE_SET)
					{
						Result = E_SOCKET_ERR_DEINIT;
						break;
					}
					Result = openSocketConnect(socketId);
					if(Result == E_SOCKET_OK)
					{
						openSocketNode_t *newNode = (openSocketNode_t *)malloc(sizeof(openSocketNode_t));
						EC_ASSERT(newNode != NULL, 0, 0, 0);
						newNode->Function = openSocketStateUrcCallbackFunc;
						newNode->Release = openSocketStateUrcCallbackRelease;
						newNode->param = socketId;
						socketStateListPrepend(newNode);
						gaSocketHandler[socketId].stateNode = newNode;
			
					}
				}
					break;
				case E_SOCKET_ACTION_SEND:
				{
					char *sendData = queue.value.txData;
					int32_t sendCnt = 0;
					int32_t trySendLen = strlen(sendData);
					sendCnt = openSocketSend(socketId,sendData,trySendLen);
					extra = (void *)&sendCnt;
					if(sendCnt < 0)					
						Result = E_SOCKET_ERR_SOCKET_SND_FAIL;
					if(gaSocketHandler[socketId].appConfigInfo.saCb)
					{
						ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadOpenSocket_2, P_INFO, "socketId[%d]_action[%d]_result[%d]", socketId, action, Result);
						gaSocketHandler[socketId].appConfigInfo.saCb(socketId, action, Result,extra);
					}
					goto RELEASE_EXIT;
				}
					break;
				case E_SOCKET_ACTION_READ:
				{
					int32_t readCnt = 0;
					uint32_t tryReadLen = queue.value.readLen;
					int8_t *readBuf = malloc(tryReadLen + 4/*save the read len*/ + 1);
					EC_ASSERT(readBuf != NULL, readBuf, 0, 0);
					memset(readBuf,0x00,tryReadLen + 4 + 1);
					readCnt = openSocketRead(socketId, readBuf + 4, tryReadLen);
					
					if(readCnt > 0)
					{	
						extra = readBuf;
						memcpy(readBuf,(char *)&readCnt,4);
					}
					else
					{
						ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadOpenSocket_5, P_INFO, "read ret [%d]", readCnt);
						Result = E_SOCKET_ERR_RECV_FAIL;
					}
					if(gaSocketHandler[socketId].appConfigInfo.saCb)
					{
						ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadOpenSocket_3, P_INFO, "socketId[%d]_action[%d]_result[%d]", socketId, action, Result);
						gaSocketHandler[socketId].appConfigInfo.saCb(socketId, action, Result,extra);
					}
					if(readBuf != NULL)
					{
						free(readBuf);
						readBuf = NULL;
					}
					extra = NULL;
					goto RELEASE_EXIT;
				}
					break;
				case E_SOCKET_ACTION_CLOSE:
					{
						Result = openSocketClose(socketId);
						if(Result == E_SOCKET_OK)
						{
							if(gaSocketHandler[socketId].appConfigInfo.saCb)
								gaSocketHandler[socketId].appConfigInfo.saCb(socketId,action,Result,NULL);
							SocketConfigDeinit(socketId);
						}
					}
					break;
				default:
					break;					
			}
			
			if(gaSocketHandler[socketId].appConfigInfo.saCb)
			{
				ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, threadOpenSocket_4, P_INFO, "socketId[%d]_action[%d]_result[%d]", socketId, action, Result);
				gaSocketHandler[socketId].appConfigInfo.saCb(socketId, action, Result,NULL);
			}
RELEASE_EXIT:			
			if((queue.action == E_SOCKET_ACTION_INIT) && (queue.value.config.hostUrl != NULL))
			{
				free(queue.value.config.hostUrl);
				queue.value.config.hostUrl = NULL;
			}
			if((queue.action == E_SOCKET_ACTION_SEND) && (queue.value.txData != NULL))
			{
				free(queue.value.txData);
				queue.value.txData = NULL;
			}
    	}
	}
}

/*----------------------------------------------------------------------------*
 *                      GLOBAL FUNCTIONS                                      *
 *----------------------------------------------------------------------------*/

int openSocketClientTracker(uint8_t socketId, uint8_t action, void * value)
{
	queueTcpipSocket_t queue = {0};
	osStatus_t status = osOK;

	if((socketId >= OPEN_TCPIP_SOCKLET_ID_MAX) || (action >= E_SOCKET_ACTION_MAX ))
		return osError;
	if(gSocketQueue != NULL)
	{
		queue.action = action;
		switch(queue.action)
		{
			case E_SOCKET_ACTION_INIT:
			{

				socketConfig_t * config = (socketConfig_t *)value;
				switch(config->socketType)
				{
					case OPEN_TCPIP_SOCKET_TYPE_TCPC:
					case OPEN_TCPIP_SOCKET_TYPE_UDPC:
					{
						EC_ASSERT(config->hostUrl != NULL,0,0,0);
						queue.value.config.hostUrl = (int8_t *)malloc(strlen((char *)config->hostUrl) + 1);
						if(queue.value.config.hostUrl == NULL)
							return osError;
						memset(queue.value.config.hostUrl,0x00,strlen((char *)config->hostUrl) + 1);
						memcpy(queue.value.config.hostUrl,config->hostUrl,strlen((char *)config->hostUrl));
						break;				
					}
					case OPEN_TCPIP_SOCKET_TYPE_TCPS:
					//TODO
					default:
						break;
						
				}
				queue.socketId = socketId;
				queue.value.config.rxBufSize = config->rxBufSize == 0 ? OPEN_TCPIP_RX_BUF_SIZE : config->rxBufSize;
				queue.value.config.port = config->port;
				queue.value.config.saCb = config->saCb;
				queue.value.config.ssCb = config->ssCb;
				queue.value.config.socketType = config->socketType;
				ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, openSocketClientTracker_1, P_INFO, "url[%s] port[%d] socketId[%d] socketType[%d]",
					queue.value.config.hostUrl,queue.value.config.port,queue.socketId,queue.value.config.socketType);
				status = osMessageQueuePut(gSocketQueue, &queue, 0, 0);
				EC_ASSERT(status == osOK, 0, 0, 0);
			}
			break;
			case E_SOCKET_ACTION_OPEN:
			case E_SOCKET_ACTION_CLOSE:
				queue.socketId = socketId;
				status = osMessageQueuePut(gSocketQueue, &queue, 0, 0);
				EC_ASSERT(status == osOK, 0, 0, 0);
				break;
			case E_SOCKET_ACTION_SEND:
				{
					char *txData = (char *)value;			
					queue.socketId = socketId;
					queue.value.txData =  (int8_t *)malloc(strlen(txData) + 1);
					EC_ASSERT(txData && queue.value.txData,txData,queue.value.txData,0);
					memset(queue.value.txData,0x00,strlen(txData) + 1);
					memcpy(queue.value.txData,txData,strlen(txData));
					status = osMessageQueuePut(gSocketQueue, &queue, 0, 0);
					EC_ASSERT(status == osOK, 0, 0, 0);
				}
				break;
			case E_SOCKET_ACTION_READ:
				{
					uint32_t readLen = *((uint32_t *)value);
					ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, openSocketClientTracker_2, P_INFO, "readLen[%d]",readLen);
					queue.socketId = socketId;
					queue.value.readLen = readLen;
					status = osMessageQueuePut(gSocketQueue, &queue, 0, 0);
					EC_ASSERT(status == osOK, 0, 0, 0);
				}
				break;
			default:
				break;			
		
}
		return status;
	}
	return osError;
}

int8_t openSocketConnect(uint8_t socketId)
{
	uint8_t socketType = 0;
	int32_t protocol  = 0;
	int32_t ret = 0;
	int8_t	Result = E_SOCKET_OK;
	
	socketHandler_t * SocketPtr = &gaSocketHandler[socketId];
	socketType = SocketPtr->appConfigInfo.socketType;
	
	switch(socketType)
	{
		case OPEN_TCPIP_SOCKET_TYPE_TCPC:
		case OPEN_TCPIP_SOCKET_TYPE_UDPC:
		{
			//step 1: get addrinfo
			struct addrinfo hints, *target_address = PNULL;
			struct addrinfo *cur;
			memset(&hints,0,sizeof(struct addrinfo));
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = (socketType == OPEN_TCPIP_SOCKET_TYPE_UDPC ? SOCK_DGRAM : SOCK_STREAM);
   			hints.ai_protocol = (protocol == OPEN_TCPIP_SOCKET_TYPE_UDPC ? IPPROTO_UDP : IPPROTO_TCP);
			char tmpPort[6] = {0};
			lwip_itoa(tmpPort, 6, SocketPtr->appConfigInfo.port);
			ret = getaddrinfo((char *)SocketPtr->appConfigInfo.hostUrl, tmpPort, &hints, &target_address);
			if (ret != 0)
			{
				ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, openSocketConnect_1, P_ERROR, "getaddrinfo fail [%d]",ret);
				Result = E_SOCKET_ERR_GET_ADDR_INFO_FAIL;
				break;
			}
			if (target_address->ai_family == AF_INET)
		    {
		        ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, openSocketConnect_2, P_INFO, "resolve url success, it is ipv4 type");
		        struct sockaddr_in *to = (struct sockaddr_in *)target_address->ai_addr;
		        IP_SET_TYPE(&(SocketPtr->remoteIp), IPADDR_TYPE_V4);
		        inet_addr_to_ip4addr(ip_2_ip4(&SocketPtr->remoteIp), &to->sin_addr);
		    }
		    else if (target_address->ai_family == AF_INET6)
		    {
		        ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, openSocketConnect_3, P_INFO, "resolve url success, it is ipv6 type");
		        struct sockaddr_in6 *to = (struct sockaddr_in6 *)target_address->ai_addr;
		        IP_SET_TYPE(&(SocketPtr->remoteIp), IPADDR_TYPE_V6);
		        inet6_addr_to_ip6addr(ip_2_ip6(&(SocketPtr->remoteIp)),&to->sin6_addr);
		    }
		    else
		    {
				ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, openSocketConnect_4, P_ERROR, "resolve url fail");
				Result = E_SOCKET_ERR_GET_ADDR_INFO_FAIL;
				freeaddrinfo(target_address);
				break;
		    }
			SocketPtr->ai_family = target_address->ai_family;
			//step 2: connect socket		
			for( cur = target_address; cur != NULL; cur = cur->ai_next )
		    {
		        SocketPtr->sockfd = (int) socket( cur->ai_family, cur->ai_socktype,
		                            cur->ai_protocol );
		        if( SocketPtr->sockfd < 0 )
		        {
		            continue;
		        }
				
		        if( connect(SocketPtr->sockfd, cur->ai_addr, (int32_t)cur->ai_addrlen) == 0)
		        {
		            Result = E_SOCKET_OK;					
		            break;
		        }
		        close(SocketPtr->sockfd);
		        Result = E_SOCKET_ERR_CONNECT_REJECT;
		    }
			if((SocketPtr->sockfd >= 0) && (Result == E_SOCKET_OK))
			{
				struct timeval tv;
				SocketPtr->socketState = E_SOCKET_STATE_ACTIVE;
				tv.tv_sec = (SocketPtr->appConfigInfo.revTimeOut > OPEN_TCPIP_SOCKET_RXTIMEOUT_MAX ? OPEN_TCPIP_SOCKET_RXTIMEOUT_MAX : SocketPtr->appConfigInfo.revTimeOut);
	    		tv.tv_usec = 0;
			    if(setsockopt(SocketPtr->sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
				{
					ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, openSocketConnect_5, P_ERROR, "setsockopt SO_RCVTIMEO error");
					close(SocketPtr->sockfd);
					Result = E_SOCKET_ERR_CONNECT_REJECT;
			    }
			}
			if(target_address != NULL)
			{
				freeaddrinfo(target_address);
				target_address = NULL;
			}
		}
		
		break;
		case OPEN_TCPIP_SOCKET_TYPE_TCPS:
		//TODO
		break;
		default:
		break;
	}
	
	return Result;
}
int32_t openSocketSend(uint8_t socketId, int8_t * sndbuffer, uint32_t sndLength)
{
	int ret;
	uint8_t socketType = 0;
	int32_t sendCnt = 0;
	int32_t realSendLen = 0;
	int32_t waitToSend = 0;
	fd_set writeFs;
	uint32_t preSelTime = 0,passedTime=0;
	struct timeval tv;
	socketHandler_t * SocketPtr = &gaSocketHandler[socketId];
	if((SocketPtr->sockfd < 0) || 
		((SocketPtr->socketState < E_SOCKET_STATE_CONNECT) || 
		(SocketPtr->socketState >= E_SOCKET_STATE_SENDING)))
		return E_SOCKET_ERR_RARAM;
	
	socketType = SocketPtr->appConfigInfo.socketType;	
	switch(socketType)
	{
		case OPEN_TCPIP_SOCKET_TYPE_TCPC:
		{
			FD_ZERO(&writeFs);
    		FD_SET(SocketPtr->sockfd, &writeFs);
			waitToSend = sndLength;
			tv.tv_sec = 0;
			tv.tv_usec = 2 *1000000;
			SocketPtr->socketState = E_SOCKET_STATE_SENDING;
			do
		    {
		        tv.tv_usec -= (passedTime*1000);
		        preSelTime = osKernelGetTickCount();
		        ret = select(SocketPtr->sockfd + 1, NULL, &writeFs, NULL, &tv);
		        if(ret > 0)
		        {
		            realSendLen = send(SocketPtr->sockfd, (sndbuffer + sndLength - waitToSend), waitToSend, MSG_DONTWAIT);
		            ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, openSocketSend_1, P_INFO, "%d bytes data has sent to server", realSendLen);
		            if(realSendLen > 0)
		            {
		                waitToSend -= realSendLen;
						sendCnt += realSendLen;
		            }
		            else if(realSendLen == 0)
		            {		               
						break;
		            }
		            else
		            {
		                ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, openSocketSend_2, P_VALUE, "send failed (errno:%d)", errno);
		                break;
		            }
		            
		            ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, openSocketSend_3, P_DEBUG, "preSelTime: %d,current tick %d", preSelTime,osKernelGetTickCount());
		            passedTime=(osKernelGetTickCount()-preSelTime) > 0 ? (osKernelGetTickCount() - preSelTime):(0xFFFFFFFF - preSelTime + osKernelGetTickCount());
		            
		        }		        
		    }while(waitToSend > 0);
			SocketPtr->socketState = E_SOCKET_STATE_ACTIVE;
		}
			break;
		case OPEN_TCPIP_SOCKET_TYPE_UDPC:
			{
				struct sockaddr_storage to;
				if (SocketPtr->ai_family == AF_INET)
			    {
			        struct sockaddr_in *to4 = (struct sockaddr_in*)&to;
			        to4->sin_len    = sizeof(struct sockaddr_in);
			        to4->sin_family = AF_INET;
			        to4->sin_port = htons(SocketPtr->appConfigInfo.port);
			        inet_addr_from_ip4addr(&to4->sin_addr, ip_2_ip4(&SocketPtr->remoteIp));
					
			    }
			    else if(SocketPtr->ai_family == AF_INET6)
			    {
			        struct sockaddr_in6 *to6 = (struct sockaddr_in6*)&to;
			        to6->sin6_len    = sizeof(struct sockaddr_in6);
			        to6->sin6_family = AF_INET6;
			        to6->sin6_port = htons(SocketPtr->appConfigInfo.port);
			        inet6_addr_from_ip6addr(&to6->sin6_addr, ip_2_ip6(&SocketPtr->remoteIp));
			    }
			    else
			    {
			       return E_SOCKET_ERR_RARAM;
			    }
				sendCnt = sendto(SocketPtr->sockfd,sndbuffer,sndLength,0,(struct sockaddr*)&to, sizeof(to));
			}
			break;
		case OPEN_TCPIP_SOCKET_TYPE_TCPS:
			break;
		default:
			break;
	}
	return sendCnt;
}
int32_t openSocketRead(uint8_t socketId, int8_t *buffer,uint32_t rdLength)
{
	uint8_t socketType = 0;
	int32_t realReadSize = 0;
	int32_t readCnt = 0;
	int32_t tryReadSize =  rdLength;
	socketHandler_t * SocketPtr = &gaSocketHandler[socketId];
	if((SocketPtr->sockfd < 0) || 
		(SocketPtr->socketState < E_SOCKET_STATE_ACTIVE))
		return E_SOCKET_ERR_RARAM;
	socketType = SocketPtr->appConfigInfo.socketType;
	switch(socketType)
	{
		case OPEN_TCPIP_SOCKET_TYPE_TCPC:
		{
			do{
			    realReadSize = (int)recv(SocketPtr->sockfd, buffer + readCnt, tryReadSize, MSG_DONTWAIT);
				ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, openSocketRead_1, P_DEBUG, "realReadSize %d", realReadSize);
			    if(realReadSize < 0){
			        if( errno == EPIPE || errno == ECONNRESET) {
			            readCnt = E_SOCKET_ERR_NET_CONN_RESET;
						break;
			        }
			        if( errno == EINTR ) {
			            readCnt  = E_SOCKET_ERR_RECV_FAIL;
						break;
			        }
			        if(realReadSize == -1 && errno == EWOULDBLOCK) {
						break;
			        }
			    }
				else if(realReadSize == 0)
				{
					break;
				}
				else
				{
					readCnt += realReadSize;
					tryReadSize -= realReadSize;
				}
			}while(tryReadSize > 0);
			SocketPtr->urcFlag = 0;
		}
			break;
		case OPEN_TCPIP_SOCKET_TYPE_UDPC:
		{
			struct sockaddr_storage addr;
			do{
			    realReadSize = recvfrom(SocketPtr->sockfd, buffer + readCnt, tryReadSize, MSG_DONTWAIT, (struct sockaddr *)&addr, sizeof(addr));
				ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, openSocketRead_2, P_DEBUG, "realReadSize %d", realReadSize);
			    if(realReadSize < 0){
			        if( errno == EPIPE || errno == ECONNRESET) {
			            readCnt = E_SOCKET_ERR_NET_CONN_RESET;
						break;
			        }
			        if( errno == EINTR ) {
			            readCnt  = E_SOCKET_ERR_RECV_FAIL;
						break;
			        }
			        if(realReadSize == -1 && errno == EWOULDBLOCK) {
						break;
			        }
			    }
				else if(realReadSize == 0)
				{
					break;
				}
				else
				{
					readCnt += realReadSize;
					tryReadSize -= realReadSize;
				}
			}while(tryReadSize > 0);
		}
			break;
		case OPEN_TCPIP_SOCKET_TYPE_TCPS:
			break;
		default:
			break;
	
}
	
	return readCnt;
}

int8_t openSocketClose(uint8_t socketId)
{
	int8_t result = E_SOCKET_OK;
	socketHandler_t * SocketPtr = &gaSocketHandler[socketId];
	if((SocketPtr->sockfd >= 0) && (SocketPtr->socketState >= E_SOCKET_STATE_ACTIVE))
	{
		SocketPtr->socketState = E_SOCKET_STATE_CLOSING;
		close(SocketPtr->sockfd);
		result = E_SOCKET_OK;
	}
	else
	{
		result = E_SOCKET_ERR_NO_CONNET;
	}
	return result;
}

int openSocketTaskinit(void)
{
    osThreadAttr_t threadAttr = {0};

    memset(&threadAttr, 0, sizeof(threadAttr));
    threadAttr.name       = "openSocketTask";
    threadAttr.stack_size = THREAD_STACK_SIZE_TCPIP;
    threadAttr.priority   = osPriorityBelowNormal7;
#if 0
    if (osThreadNew(threadOpenSocket, NULL, &threadAttr) == NULL)
#else
    char serviceName[32] = {0};
    snprintf(serviceName, sizeof(serviceName), "service:/%s", threadAttr.name);
    Service_reg(serviceName, threadOpenSocket, NULL, threadAttr.cb_mem, threadAttr.cb_size, threadAttr.stack_mem, threadAttr.stack_size, threadAttr.priority);
    if ((osThreadId_t)Service_start(serviceName) == NULL)
#endif
    {
		ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, openSocketTaskinit, P_ERROR, "Failed to create thread for open socket.");
    }
	
	memset(&threadAttr, 0, sizeof(threadAttr));
	threadAttr.name       = "tcpipStateTrigger";
    threadAttr.stack_size = THREAD_STACK_SIZE_TCPIP_STA;
    threadAttr.priority   = osPriorityBelowNormal6;
#if 0
    if (osThreadNew(threadTcpipStateTrigger, NULL, &threadAttr) == NULL)
#else
    char serviceName2[32] = {0};
	snprintf(serviceName2, sizeof(serviceName2), "service:/%s", threadAttr.name);
    Service_reg(serviceName2, threadTcpipStateTrigger, NULL, threadAttr.cb_mem, threadAttr.cb_size, threadAttr.stack_mem, threadAttr.stack_size, threadAttr.priority);
    if ((osThreadId_t)Service_start(serviceName2) == NULL)
#endif
    {
		ECPLAT_PRINTF(UNILOG_PLAT_NETWORK, tcpip_thread_init_1, P_ERROR, "Failed to create thread for threadTcpipStateTrigger.");
    }
	
    return 0;
};
