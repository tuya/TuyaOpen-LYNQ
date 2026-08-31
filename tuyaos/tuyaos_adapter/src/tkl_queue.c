#include <stdlib.h>
#include "tkl_queue.h"
#include "cmsis_os2.h"
#include "vlog.h"

/**
 * @brief Create message queue
 *
 * @param[in] msgsize message size
 * @param[in] msgcount message number
 * @param[out] queue the queue handle created
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_queue_create_init(TKL_QUEUE_HANDLE *queue, INT_T msgsize, INT_T msgcount)
{	
	if (!queue) {
		LOGE("create queue failed, queue is null");
		return OPRT_OS_ADAPTER_INVALID_PARM;
	}
	
	if (0 >= msgsize || 0 >= msgcount) {
		LOGE("invalid msgsize or msgcount, msgsize: %d, msgcount: %d", msgsize, msgcount);
		return OPRT_OS_ADAPTER_INVALID_PARM;
	}
	
	osMessageQueueId_t id = osMessageQueueNew((uint32_t)msgcount, (uint32_t)msgsize, NULL);
	if (NULL == id) {
		LOGE("create queue failed");
		return OPRT_OS_ADAPTER_QUEUE_CREAT_FAILED;
	}
	
	*queue = id;
	return OPRT_OK;
}

/**
 * @brief post a message to the message queue
 *
 * @param[in] queue the handle of the queue
 * @param[in] data the data of the message
 * @param[in] timeout timeout time
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
int tkl_queue_post(CONST TKL_QUEUE_HANDLE queue, VOID_T *msg, UINT_T timeout)
{	
	if (!queue || !msg) {
		LOGE("queue post failed, invalid param, queue: %p, msg: %p", queue, msg);
		return OPRT_OS_ADAPTER_INVALID_PARM;
	}
	
	osStatus_t status = osMessageQueuePut(queue, msg, 0, timeout);
	return (osOK == status) ? OPRT_OK : OPRT_OS_ADAPTER_QUEUE_SEND_FAIL;
}

/**
 * @brief fetch message from the message queue
 *
 * @param[in] queue the message queue handle
 * @param[inout] msg the message fetch form the message queue
 * @param[in] timeout timeout time
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_queue_fetch(CONST TKL_QUEUE_HANDLE queue, VOID_T *msg, UINT_T timeout)
{	
	if (!queue || !msg) {
		LOGE("queue fetch failed, invalid param, queue: %p, msg: %p", queue, msg);
		return OPRT_OS_ADAPTER_INVALID_PARM;
	}

	osStatus_t status = osMessageQueueGet(queue, msg, NULL, timeout);
	return (osOK == status) ? OPRT_OK : OPRT_OS_ADAPTER_QUEUE_RECV_FAIL;
}

/**
 * @brief free the message queue
 *
 * @param[in] queue the message queue handle
 *
 * @return VOID_T
 */
VOID_T tkl_queue_free(CONST TKL_QUEUE_HANDLE queue)
{
	if (!queue) {
		LOGE("queue free failed, invalid param");
		return;
	}
	
	osStatus_t status = osMessageQueueDelete(queue);
	if (osOK != status) {
		LOGE("queue free failed, status: %d", status);
	}
}


#if 0
#include "tkl_thread.h"
#include "tkl_system.h"

static TKL_THREAD_HANDLE test_thread1, test_thread2;
static TKL_QUEUE_HANDLE test_queue;

static void test_thread1_task(void *arg)
{
	int ret = (int)arg;
	LOGI("thread1 running, arg: %d", ret);
	char c = 0;
	while(1)
	{
		ret = tkl_queue_post(test_queue, &c, 0);
		c++;
		LOGI("thread1 put queue %d", ret);
		tkl_system_sleep(100);
	}
}

static void test_thread2_task(void *arg)
{
	int ret = (int)arg;
	LOGI("thread2 running, arg: %d", ret);
	char c;
	while(1)
	{
		ret = tkl_queue_fetch(test_queue, &c, 200);
		if(ret) {
			LOGI("thread2 wait queue timeout");
		} else {
			LOGI("thread2 wait queue succ %d", c);
		}

	}
}

void tkl_queue_test(void)
{
	OPERATE_RET ret = 0;
	ret = tkl_queue_create_init(&test_queue, sizeof(char), 10);
	ret = tkl_thread_create(&test_thread1, "test_thread1", 4096,  2, test_thread1_task, (void *)1);
	ret = tkl_thread_create(&test_thread2, "test_thread1", 4096,  2, test_thread2_task, (void *)2);
}

#endif