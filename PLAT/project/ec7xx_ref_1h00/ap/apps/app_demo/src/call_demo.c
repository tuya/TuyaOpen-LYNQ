/****************************************************************************
 *
 ****************************************************************************/
#include "string.h"
#include "osasys.h"
#include "ol_call_api.h"
#include "ol_log.h"

int RingFlag = -1;
void call_urc(int event_id,void *msg)
{
	int ret = -1;
    OL_LOG_INFO("call urc len[%d] str = %s",event_id, msg);
	if(strstr(msg, "RING") != NULL)
	{
		OL_LOG_INFO("Answer incoming calls");
		RingFlag = 1;
	}
	if(strstr(msg, "CLIP") != NULL && (1 == RingFlag))
	{
		OL_LOG_INFO("Answer incoming calls");
		osDelay(10000);
		ret = ol_call_answer();
		OL_LOG_INFO("Answer incoming calls ret = %d", ret);
	}
}

void call_demo(void)
{
    int ret = -1;
    char command[128];
	ol_call_task_Init();//The task must be initialized before the phone task starts
	ol_call_event_register(call_urc);

    memset(command, 0x0, sizeof(command));
    sprintf(command, "18238036370");
	ret = ol_call_dial(command);
    OL_LOG_INFO("call dial return %d",ret);
	osDelay(58000);
	ret = ol_call_hangup();
    OL_LOG_INFO("Call hangup return %d",ret);
}
