#include "string.h"
#include "FreeRTOS.h"
#include "ostask.h"
#include "cmsis_os2.h"
#include "input.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#include "servicemanager.h"


void inputProcMount();


static osSemaphoreId_t    gInputSemaphore                  = NULL;
static InputProcCallbackT gInputProcHub[INPUTPROC_MAX_NUM] = {0};


void mountInputProc(InputProcCallbackT pFun,int index)
{
    gInputProcHub[index]=pFun;
}

int32_t inputNotify(void)
{
    int32_t retVal = -1;

    if (gInputSemaphore != NULL)
    {
        osSemaphoreRelease(gInputSemaphore);
        retVal = 0;
    }

    return retVal;
}

void subInputTask(void *argument)
{
    gInputSemaphore = osSemaphoreNew(100, 1, NULL);
    if (gInputSemaphore == NULL)
    {
        SYSLOG_EMERG("Failed to create semaphore for gInputSemaphore.\r\n");
        goto labelEnd;
    }

	while(1)
    {
        osSemaphoreAcquire(gInputSemaphore, osWaitForever);
        for(int i=0; i<INPUTPROC_MAX_NUM; i++)
        {
            if(gInputProcHub[i] != NULL)
            {
                (gInputProcHub[i])();
            }
        }
    }

labelEnd:
#if 0 // Service Manager
    osThreadExit();
#else
    Service_stop("service:/input");
#endif
}

void subInputInit(void)
{
    osThreadAttr_t taskAttr;
	
	inputProcMount();

    memset(&taskAttr,0,sizeof(taskAttr));
    memset(subsysInputTaskStack, 0xA5,SUBSYS_INPUT_TASK_STACK_SIZE);
    taskAttr.name = "input";
    taskAttr.stack_mem = subsysInputTaskStack;
    taskAttr.stack_size = SUBSYS_INPUT_TASK_STACK_SIZE;
    taskAttr.priority = osPriorityNormal;
    taskAttr.cb_mem = &subsys_input_task;//task control block
    taskAttr.cb_size = sizeof(StaticTask_t);//size of task control block

#if 0
    osThreadNew(subInputTask, NULL, &taskAttr);
#else
    char serviceName[32] = {0};
    snprintf(serviceName, sizeof(serviceName), "service:/%s", taskAttr.name);
    Service_reg(serviceName, subInputTask, NULL, taskAttr.cb_mem, taskAttr.cb_size, taskAttr.stack_mem, taskAttr.stack_size, taskAttr.priority);
    Service_start(serviceName);
#endif
}

