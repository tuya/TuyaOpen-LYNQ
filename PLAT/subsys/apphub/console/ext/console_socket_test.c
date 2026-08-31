/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of EigenComm Ltd.
 * File name:    console_socket_test.c
 * Description:  EC718
 * History:      Rev1.0   2024-09-05
 *
 ****************************************************************************/

#ifdef FEATURE_SUBSYS_CONSOLE_ENABLE

#include "open_socket.h"
#include "rtthread.h"

extern int skip_atoi(const char **s);

char *actions[] = {"INIT", "OPEN", "SEND", "READ", "CLOSE"};
int consleSocketActionCb(uint8_t socketId, uint8_t action, int8_t result, const void *extra)
{
	rt_kprintlnf("");
	rt_kprintlnf("%d %d %d", socketId, action, result);
	if (result != 0)
	{
		rt_kprintlnf("%s FAIL\r\n", actions[action]);
		return 1;
	}

	switch (action)
	{
	case E_SOCKET_ACTION_INIT:
	{
		if (result == E_SOCKET_OK)
			openSocketClientTracker(socketId, E_SOCKET_ACTION_OPEN, NULL);
	}
	break;
	case E_SOCKET_ACTION_SEND:
	{
	}
	break;
	case E_SOCKET_ACTION_READ:
	{
		if (result == E_SOCKET_OK)
		{
			int32_t length;
			length = *((int32_t *)extra);
			char *ptr = ((char *)extra + 4);
			rt_kprintf("read [%d] : %s\r\n", length, ptr);
		}
	}
	break;
	default:
		break;
	}
	return 0;
}

void consleSocketStateCb(uint8_t socketId, int32_t value)
{
	rt_kprintf("\r\n");
	if (value > 0)
		rt_kprintf("URC READ: %d\r\n", socketId);
	else
		rt_kprintf("URC CLOSE: %d\r\n", socketId);
	return;
}

int cmd_socket(int argc, char **argv)
{

	char *sub_cmd = argv[1];
	// socket connect 0 tcpc host port
	if (strcmp(sub_cmd, "connect") == 0)
	{
		uint8_t socketId = skip_atoi(&argv[2]);
		char *socketType = argv[3];

		socketConfig_t curConfig;
		memset(&curConfig, 0x00, sizeof(socketConfig_t));

		if (strcmp(socketType, "TCPC") == 0)
			curConfig.socketType = OPEN_TCPIP_SOCKET_TYPE_TCPC;
		else if (strcmp(socketType, "TCPS") == 0)
			curConfig.socketType = OPEN_TCPIP_SOCKET_TYPE_TCPS;
		else if (strcmp(socketType, "UDP") == 0)
			curConfig.socketType = OPEN_TCPIP_SOCKET_TYPE_UDPC;
		else
		{
			rt_kprintlnf("Error: Bad connect type \"%s\"", socketType);
			
			return 1;
		}
		curConfig.hostUrl = argv[4];
		curConfig.port = skip_atoi(&argv[5]);
		curConfig.saCb = consleSocketActionCb;
		curConfig.ssCb = consleSocketStateCb;
		openSocketClientTracker(socketId, E_SOCKET_ACTION_INIT, (void *)&curConfig);
	}
	// socket close 0
	else if (strcmp(sub_cmd, "close") == 0)
	{
		uint8_t socketId = skip_atoi(&argv[2]);
		openSocketClientTracker(socketId, E_SOCKET_ACTION_CLOSE, NULL);
	}
	// socket send 0 12345
	else if (strcmp(sub_cmd, "send") == 0)
	{
		uint8_t socketId = skip_atoi(&argv[2]);
		char *sendData = argv[3];
		openSocketClientTracker(socketId, E_SOCKET_ACTION_SEND, (void *)sendData);
	}
	// socket read 0 100
	else if (strcmp(sub_cmd, "read") == 0)
	{
		uint8_t socketId = skip_atoi(&argv[2]);
		uint32_t readLen = skip_atoi(&argv[3]);
		openSocketClientTracker(socketId, E_SOCKET_ACTION_READ, (void *)&readLen);
	}

	else
	{
		goto _usage;
	}
	
	return 0;

_usage:
	rt_kprintlnf("Usage: socket [options]");
	rt_kprintlnf("[options]:");
	rt_kprintlnf("    %-10s - connect socket", "connect");
	rt_kprintlnf("    %-10s - close socket", "close");
	rt_kprintlnf("    %-10s - send data from socket", "send");
	rt_kprintlnf("    %-10s - receive data from socket", "read");

	
	return 0;
}
MSH_CMD_EXPORT_ALIAS(cmd_socket, socket, socket test);

#endif
