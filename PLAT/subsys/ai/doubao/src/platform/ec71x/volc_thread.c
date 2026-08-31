#include "volc_thread.h"

#include <unistd.h>
#include "ps_lib_api.h"
//#include <freertos/FreeRTOS.h>
#include "osasys.h"
#include "volc_errno.h"
#include "volc_memory.h"
#include "volc_time.h"
#include "volc_type.h"

volc_thread_local_t g_thread_local = NULL;
#define THREAD_STACK_SIZE_VOLC (8*1024)
static void __attribute__((used)) _volc_thread_local_delete_key(volc_thread_local_t local) {
    (void)local;
} 

volc_tid_t volc_thread_get_id(void) {
    return (volc_tid_t)osThreadGetId();
}

uint32_t volc_thread_get_name(volc_tid_t thread, char* name, uint32_t len) {
	uint32_t nameLen = (len > VOLC_THREAD_NAME_MAX_LENGTH ? VOLC_THREAD_NAME_MAX_LENGTH : len);
	const char *threadName = NULL;
	threadName = osThreadGetName((osThreadId_t)thread);
	if(name)
		memcpy(name,threadName,nameLen);
	ECPLAT_PRINTF(UNILOG_PLAT_VOLC,volc_thread_get_name,P_DEBUG,"name(%s)_len(%d)",name,len);
    return VOLC_SUCCESS;
}

uint32_t volc_thread_set_name(const char* name) {
    (void)name;
	ECPLAT_PRINTF(UNILOG_PLAT_VOLC,volc_thread_set_name,P_DEBUG,"name_%s",name);
    return VOLC_SUCCESS;
}

uint32_t volc_thread_create(volc_tid_t* thread, const volc_thread_param_t* param, void* (*start_routine)(void *), void* args) {
    osThreadId_t threadId;

	if(param)
		ECPLAT_PRINTF(UNILOG_PLAT_VOLC,volc_thread_create_1,P_DEBUG,"name(%s),stack_size(%d),priority(%d)",param->name,param->stack_size,param->priority);
    if (NULL == thread || NULL == start_routine) {
        return VOLC_FAILED;
    }
	osThreadAttr_t threadAttr = {0};
    memset(&threadAttr, 0, sizeof(threadAttr));
    
    if (NULL != param) {
        threadAttr.stack_size = (threadAttr.stack_size > 0 ? threadAttr.stack_size : THREAD_STACK_SIZE_VOLC);
        threadAttr.name = param->name;
    } 
	else
	{
       threadAttr.stack_size = THREAD_STACK_SIZE_VOLC;
    }    
    threadAttr.priority = osPriorityBelowNormal7;
    threadId = osThreadNew((osThreadFunc_t)start_routine,args,&threadAttr);
    if (NULL == threadId) {
        return VOLC_FAILED;
    }
	*thread = threadId;
    return VOLC_SUCCESS;
}

void volc_thread_destroy(volc_tid_t thread) {
	ECPLAT_PRINTF(UNILOG_PLAT_VOLC,volc_thread_destroy,P_DEBUG,"thread_%p",thread);
  	if (NULL == thread) {
        return;
    }
	osThreadTerminate((osThreadId_t)thread);
}

void volc_thread_exit(volc_tid_t thread) {
	ECPLAT_PRINTF(UNILOG_PLAT_VOLC,volc_thread_exit,P_DEBUG,"thread_%p",thread);
    //if (osThreadGetId() != thread) {
    //    return;
    //}
	osThreadExit();
}

void volc_thread_sleep(uint64_t time) {
	ECPLAT_PRINTF(UNILOG_PLAT_VOLC,volc_thread_sleep,P_DEBUG,"sleep_%d",time);
	osDelay((uint32_t)time);
}

void volc_thread_sleep_until(uint64_t time) {
	ECPLAT_PRINTF(UNILOG_PLAT_VOLC,volc_thread_sleep_until,P_DEBUG,"sleep_until_%d",time);
	utc_timer_value_t curTime   = {0};
	appGetSystemTimeUtcSync(&curTime);

	uint64_t cur_time = (uint64_t)(curTime.UTCsecs * 1000 + curTime.UTCms);
    if (time > cur_time) {
        volc_thread_sleep(time - cur_time);
    }
}

uint32_t volc_thread_join(volc_tid_t thread, void* ret) {
    (void)thread;
    (void)ret;
	ECPLAT_PRINTF(UNILOG_PLAT_VOLC,volc_thread_join,P_DEBUG,"thread_%p",thread);
    return VOLC_SUCCESS;
}

uint32_t volc_thread_cancel(volc_tid_t thread) {
    (void)thread;
	ECPLAT_PRINTF(UNILOG_PLAT_VOLC,volc_thread_cancel,P_DEBUG,"thread_%p",thread);
    return VOLC_SUCCESS;
}

uint32_t volc_thread_detach(volc_tid_t thread) {
    (void)thread;
	ECPLAT_PRINTF(UNILOG_PLAT_VOLC,volc_thread_detach,P_DEBUG,"thread_%p",thread);
    return VOLC_SUCCESS;
}

volc_thread_local_t volc_thread_local_create(void) {
    return NULL;
}

uint32_t volc_thread_local_destroy(volc_thread_local_t local) {
	(void)local;
    return VOLC_FAILED;
}

uint32_t volc_thread_local_set(volc_thread_local_t local, void* data) {
    (void)local;
	(void)data;
    return VOLC_FAILED;
}

uint32_t volc_thread_local_clean(volc_thread_local_t local) {
    (void)local;
    return VOLC_FAILED;
}

void* volc_thread_local_get(volc_thread_local_t local) {
    (void)local;
    return NULL;
}