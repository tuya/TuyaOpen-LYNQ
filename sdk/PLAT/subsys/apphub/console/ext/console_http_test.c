/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of EigenComm Ltd.
 * File name:    console_http_test.c
 * Description:  EC718
 * History:      Rev1.0   2024-09-23
 *
 ****************************************************************************/

#ifdef FEATURE_SUBSYS_CONSOLE_ENABLE

#include "rtthread.h"
#include "open_httpc.h"

extern int skip_atoi(const char **s);

void httpRteCb(int8_t result, int8_t dataType, const void *extra)
{
	// rt_kprintf("\r\n");
	switch (dataType)
	{
	case HTTPC_RTEDATA_IDLE:
		rt_kprintlnf("HTTP CONNECT result : %d", result);
		break;
	case HTTPC_RTEDATA_HEADER:
		rt_kprintlnf("HTTP RSP HEAD : %s", (int8_t *)extra);
		break;
	case HTTPC_RTEDATA_CONTEXT:
		rt_kprintlnf("HTTP RSP DATA : %s", (int8_t *)extra);
		break;
	default:
		break;
	}
}

int cmd_http(int argc, char **argv)
{

	char *sub_cmd = argv[1];
	// http get http://xxx.xxx.xxx.xxx/get
	if (strcmp(sub_cmd, "get") == 0)
	{
		int8_t *url = argv[2];
		openHttpcGet(url, httpRteCb);
	}
	// http post http://xxx.xxx.xxx.xxx/get xxxxx
	else if (strcmp(sub_cmd, "post") == 0)
	{
		int8_t *url = argv[1];
		int8_t *postData = argv[2];
		openHttpcPost(url, postData, strlen(postData), httpRteCb);
	}
	else
	{
		rt_kprintlnf("Usage: http [options]");
		rt_kprintlnf("[options]:");
		rt_kprintlnf("    %-10s - http get request", "get");
		rt_kprintlnf("    %-10s - http post request", "post");
	}
	rt_kprintf("\r\n%s", getConsolePrompt());
	return 0;
}
MSH_CMD_EXPORT_ALIAS(cmd_http, http, http test);

#endif
