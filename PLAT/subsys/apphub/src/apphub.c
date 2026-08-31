#include "string.h"
#include "FreeRTOS.h"
#include "ostask.h"
#include "cmsis_os2.h"
#include "apphub.h"
#include "app.h"

#include DEBUG_LOG_HEADER_FILE
#include "plat_config.h"

#ifdef FEATURE_SUBSYS_GUI_LVGL_ENABLE
#include "lvgl.h"
#endif
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#include "sysservice.h"
#include "servicemanager.h"


extern const uint32_t psyscall_index;

AppT* apphub[APPHUB_MAX_SLOT]={0};

//AppMsgQueueT gMsgQ={0};
osMessageQueueId_t ApphubQ = NULL;

AppHubInfoT gAppHubInfo={0};



void appMsgQInit(void)
{
    // memset(&gMsgQ, 0, sizeof(gMsgQ));
	ApphubQ = osMessageQueueNew(APP_MSG_QUEUE_SIZE, sizeof(AppMsgT), NULL);
    if (ApphubQ == NULL)
    {
        SYSLOG_EMERG("Failed to create queue for ApphubQ.\r\n");
    }
}

int32_t appSendMsg(AppMsgT* msgPtr)
{
	osMessageQueuePut(ApphubQ, msgPtr, 0, osWaitForever);
    return 0;
}

int32_t appGetMsg(AppMsgT* msgPtr)
{
    return osMessageQueueGet(ApphubQ, msgPtr, 0, osWaitForever);
}

int32_t getActiveAppId()
{
	return gAppHubInfo.curActiveApp;
}

int32_t getApphubSlotStat()
{
	return gAppHubInfo.slotStat;
}

int32_t mountApp(AppT* appPtr,int32_t slot)
{
	appPtr->info->initStatus = 0;
	apphub[slot]=appPtr;
	gAppHubInfo.slotStat |= 1<<(2*slot);
	return 1;
}

int32_t setAppStat(int32_t slot,SlotStatusTypeT stat)
{
	uint32_t stat_mask=0;
	stat_mask = 0x3<<(2*slot);
	gAppHubInfo.slotStat &=stat_mask;
	gAppHubInfo.slotStat |= stat<<(2*slot);
	return gAppHubInfo.slotStat;
}
int32_t setActiveApp(int32_t slot)
{
    AppMsgT msgPtr = {0};
	gAppHubInfo.curActiveApp = slot;
    appSendMsg(&msgPtr);
	return gAppHubInfo.curActiveApp;
}

void subApphubTask()
{
	int32_t curAppId=0;
	AppMsgT msg;
	while(1)
    {
        memset(&msg, 0, sizeof(msg));
        if (appGetMsg(&msg) == osOK)
        {
            curAppId = getActiveAppId();
            if (curAppId < 0)
            {
                continue;
            }

            if(apphub[curAppId]->info->initStatus == 0)
            {
                apphub[curAppId]->init(apphub[curAppId]->info, 0, 0, (uint32_t)&psyscall_index);
            }
            apphub[curAppId]->msgProc(apphub[curAppId]->info,&msg, 0, (uint32_t)&psyscall_index);
#if 0
            //backgound
            for(int i=0;i<curAppId;i++)
            {
                if(apphub[i] != NULL)
                    apphub[i]->msgProc(apphub[curAppId]->info,&msg, 0, &psyscall_index);
            }
            for(int i=curAppId+1;i<APPHUB_MAX_SLOT;i++)
            {
                if(apphub[i] != NULL)
                    apphub[i]->msgProc(apphub[curAppId]->info,&msg, 0, &psyscall_index);
            }
#endif
            //sys service
            sysservice_proc(apphub[curAppId]->info,&msg, 0, (uint32_t)&psyscall_index);
        }
	}
}

void subApphubInit()
{
	appMsgQInit();
    gAppHubInfo.curActiveApp   = -1;
	gAppHubInfo.freeSlotNumber = APPHUB_MAX_SLOT;
	
    osThreadAttr_t task_attr;
    memset(&task_attr,0,sizeof(task_attr));
#if 0 //mbtk change
    memset(subsys_apphub_task_stack, 0xA5,SUBSYS_APPHUB_TASK_STACK_SIZE);
    task_attr.name = "apphub";
    task_attr.stack_mem = subsys_apphub_task_stack;
    task_attr.stack_size = SUBSYS_APPHUB_TASK_STACK_SIZE;
    task_attr.priority = osPriorityNormal;
    task_attr.cb_mem = &subsys_apphub_task;     //task control block
    task_attr.cb_size = sizeof(StaticTask_t);   //size of task control block


    osThreadNew(subApphubTask, NULL, &task_attr);
#else
	task_attr.name = "apphub";
	task_attr.stack_size = 4*1024;
	task_attr.priority = osPriorityNormal;
    char serviceName[32] = {0};
    snprintf(serviceName, sizeof(serviceName), "service:/%s", task_attr.name);
    Service_reg(serviceName, subApphubTask, NULL, task_attr.cb_mem, task_attr.cb_size, task_attr.stack_mem, task_attr.stack_size, task_attr.priority);
    Service_start(serviceName);
#endif
}

