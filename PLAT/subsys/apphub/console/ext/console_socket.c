#ifdef FEATURE_SUBSYS_CONSOLE_ENABLE
#include <stdint.h>
#include "string.h"
#include "FreeRTOS.h"

#include "cmsis_os2.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


#include "console.h"
#include "console_ex.h"
#include "console_hal.h"
#include "console_file.h"

#include "ps_lib_api.h"
#include "ps_sim_if.h"
#include "networkmgr.h"
#include "netdb.h"

#include "open_socket.h"

#ifdef FEATURE_SUBSYS_FINSH_ENABLE
#include "finsh.h"

#define GET_RESULT(result)	((result) == 0 ? "success" :"fail") 

char* actionArray[] = {"init","open","send","read","close",NULL};
void consleSocketActionCb(uint8_t socketId, uint8_t action, int8_t result, const void *extra)
{
	rt_kprintf("\r\n");
	rt_kprintf("socket %s %s \r\n",actionArray[action],GET_RESULT(result));
	switch(action)
	{
		case E_SOCKET_ACTION_INIT:
		{
			if(result == E_SOCKET_OK)
				openSocketClientTracker(socketId,E_SOCKET_ACTION_OPEN,NULL);
		}
			break;
		case E_SOCKET_ACTION_SEND:
		{
			if(result == E_SOCKET_OK)
				rt_kprintf("send length %d \r\n",*((int32_t *)extra));			
		}
		break;
		case E_SOCKET_ACTION_READ:
		{
			if(result == E_SOCKET_OK)
			{
				int32_t length;
				length = *((int32_t *)extra);
				char *ptr = ((char *)extra + 4);
				rt_kprintf("read [%d] : %s\r\n",length,ptr);		
			}	
		}
		break;
		default:
		break;
	}
	return;
}

void consleSocketStateCb(uint8_t socketId, int32_t value)
{
	rt_kprintf("\r\n");
	if(value > 0)
		rt_kprintf("URC READ: %d\r\n",socketId);
	else
		rt_kprintf("URC CLCOSE: %d\r\n",socketId);
	return;
}


void consoleNetConnect(int argc, char **argv)
{
    socketConfig_t curConfig;
	uint8_t socketId;
	memset(&curConfig,0x00,sizeof(socketConfig_t));
	socketId = atoi(argv[1]);
	if(strcasestr(argv[2],"TCPC"))
		curConfig.socketType = OPEN_TCPIP_SOCKET_TYPE_TCPC;
	else if(strcasestr(argv[2],"TCPS"))
		curConfig.socketType = OPEN_TCPIP_SOCKET_TYPE_TCPS;
	else if(strcasestr(argv[2],"UDP"))
		curConfig.socketType = OPEN_TCPIP_SOCKET_TYPE_UDPC;
	else
	{
		 
		 return;
	}	
	curConfig.hostUrl = argv[3];
	curConfig.port = atoi(argv[4]);
    curConfig.saCb = consleSocketActionCb;
	curConfig.ssCb = consleSocketStateCb;
    openSocketClientTracker(socketId,E_SOCKET_ACTION_INIT,(void *)&curConfig);
    
}

void consoleNetClose(int argc, char **argv)
{	
	uint8_t socketId;
	socketId = atoi(argv[1]);
    openSocketClientTracker(socketId,E_SOCKET_ACTION_CLOSE,NULL);
    
}

void consoleNetSend(int argc, char **argv)
{	
	uint8_t socketId = atoi(argv[1]);
	char *sendData = argv[2];
    openSocketClientTracker(socketId,E_SOCKET_ACTION_SEND,(void *)sendData);
    
}

void consoleNetRead(int argc, char **argv)
{	
	uint8_t socketId = atoi(argv[1]);
	uint32_t readLen = atoi(argv[2]);
    openSocketClientTracker(socketId,E_SOCKET_ACTION_READ,(void *)&readLen);
    
}


MSH_CMD_EXPORT_ALIAS(consoleNetClose, netClose, netClose);

MSH_CMD_EXPORT_ALIAS(consoleNetConnect, netConnect, netConnect);

MSH_CMD_EXPORT_ALIAS(consoleNetSend, netSend, netSend);

MSH_CMD_EXPORT_ALIAS(consoleNetRead, netRead, netRead);


#endif

#endif/*FEATURE_SUBSYS_CONSOLE_ENABLE*/
