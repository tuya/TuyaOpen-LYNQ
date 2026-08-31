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

#include "open_httpc.h"

#ifdef FEATURE_SUBSYS_FINSH_ENABLE
#include "finsh.h"

#define GET_RESULT(result)	((result) == E_HTTPC_OK ? "success" :"fail") 

void consleHttpRteCb(int8_t result, int8_t dataType,const void *extra)
{
	rt_kprintf("\r\n");
	switch(dataType)
	{
		case HTTPC_RTEDATA_IDLE:
			rt_kprintf("\r\nHTTP CONNECT result : %s",GET_RESULT(result));
		break;
		case HTTPC_RTEDATA_HEADER:
			rt_kprintf("\r\nHTTP RSP HEAD : %s",(int8_t *)extra);
		break;
		case HTTPC_RTEDATA_CONTEXT:
			rt_kprintf("\r\nHTTP RSP DATA : %s",(int8_t *)extra);
		break;
		default:
		break;
	}
	return;
}


void consoleOpenHttpcPost(int argc, char **argv)
{	
	int8_t *url = argv[1];
	int8_t *postData = argv[2];
    openHttpcPost(url,NULL,0,postData,strlen(postData),consleHttpRteCb);
}


void consoleOpenHttpcGet(int argc, char **argv)
{	
	int8_t *url = argv[1];
    openHttpcGet(url, NULL,0,consleHttpRteCb);
}


MSH_CMD_EXPORT_ALIAS(consoleOpenHttpcPost, ohcp, open httpc post);

MSH_CMD_EXPORT_ALIAS(consoleOpenHttpcGet, ohcg, open httpc get);


#endif

#endif/*FEATURE_SUBSYS_CONSOLE_ENABLE*/

