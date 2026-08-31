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

#include "open_websocket.h"

#ifdef FEATURE_SUBSYS_FINSH_ENABLE
#include "finsh.h"

#define GET_RESULT(result)	((result) == E_WSS_OK ? "success" :"fail") 

void consleWssAction(int8_t socketId, int8_t actionType,int8_t result)
{
	rt_kprintf("\r\n");
	switch(actionType)
	{
		case WSS_ACTION_CONNECT:
			rt_kprintf("\r\nWebsocket(%d) CONNECT result : %s",socketId,GET_RESULT(result));
		break;
		case WSS_ACTION_SEND:
			rt_kprintf("\r\nWebsocket(%d) send result : %s",socketId,GET_RESULT(result));
		break;
		case WSS_ACTION_CLOSE:
			rt_kprintf("\r\nWebsocket(%d) CONNECT result : %s",socketId,GET_RESULT(result));
		break;
		default:
		break;
	}
	return;
}

void consleWssOnMessage(int8_t socketId, int8_t frameType,int8_t *data,uint32_t length)
{
	rt_kprintf("\r\n");
	rt_kprintf("\r\nWebsocket(%d) recv frameType(%d)\r\n",socketId,frameType);
	rt_kprintf("data >>\r\n%s",data);
	return;
}

void consoleWssConnect(int argc, char **argv)
{	
	int8_t *url = argv[1];
	int8_t socketId = 0;
	socketId = openWssCreate();
	if(socketId >= 0)
    	openWssConnect(socketId,url,consleWssAction,consleWssOnMessage,NULL,0);
}

void consoleWssSend(int argc, char **argv)
{	
#if 1
	if(argc < 4)
	{
		rt_kprintf("error param\r\n");
	}
	else
	{
		int8_t *payload = NULL;
		uint32_t payloadLen = 0;
		int8_t socketId = (int8_t)atoi(argv[1]);	
		int mask = atoi(argv[2]);
		int frameType = atoi(argv[3]);
		if(argc >= 4)
			payloadLen = atoi(argv[4]);
		if(argc >= 5)
		 	payload = argv[5];
		bool fin = true;
	    openWssSend(socketId,payload,payloadLen,fin,frameType,mask);
	}
#else
		int8_t *payload = "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
		uint32_t payloadLen = 320;
		int8_t socketId = (int8_t)atoi(argv[1]);
		bool mask = 1;
		int frameType = 1;
		bool fin = true;
		openWssSend(socketId,payload,payloadLen,fin,frameType,mask);
#endif
}


void consoleWssClose(int argc, char **argv)
{	
	int8_t socketId = atoi(argv[1]);
    openWssClose(socketId);
}


MSH_CMD_EXPORT_ALIAS(consoleWssConnect, wssConnect, connect websocket);
MSH_CMD_EXPORT_ALIAS(consoleWssClose, wssClose, close websocket);
MSH_CMD_EXPORT_ALIAS(consoleWssSend, wssSend, send websocket data);


#endif

#endif/*FEATURE_SUBSYS_CONSOLE_ENABLE*/

