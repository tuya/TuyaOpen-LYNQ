#include <stdlib.h>
#include "tkl_semaphore.h"
#include "cmsis_os2.h"
#include "vlog.h"

/**
* @brief Create semaphore
*
* @param[out] handle: semaphore handle
* @param[in] sem_cnt: semaphore init count
* @param[in] sem_max: semaphore max count
*
* @note This API is used for creating semaphore.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_semaphore_create_init(TKL_SEM_HANDLE *pHandle,
                                          CONST UINT_T semCnt,
                                          CONST UINT_T sem_max)
{	
	if (!pHandle) {
		LOGE("create semaphore failed, pHandle null");
		return OPRT_OS_ADAPTER_INVALID_PARM;
	}
	
	if (semCnt > sem_max) {
		LOGE("create semaphore failed, invalid semCnt(%u:%u)", semCnt, sem_max);
		return OPRT_OS_ADAPTER_INVALID_PARM;
	}
	
	osSemaphoreId_t id = osSemaphoreNew(sem_max, semCnt, NULL);
	if (NULL == id) {
		LOGE("create semaphore failed");
		return OPRT_COM_ERROR;
	}

	*pHandle = id;
	return OPRT_OK;
}

/**
* @brief Wait semaphore
*
* @param[in] handle: semaphore handle
* @param[in] timeout: wait timeout, SEM_WAIT_FOREVER means wait until get semaphore
*
* @note This API is used for waiting semaphore.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_semaphore_wait(CONST TKL_SEM_HANDLE handle, UINT_T timeout)
{	
	if (!handle) {
		LOGE("semaphore wait failed, invalid handle");
		return OPRT_OS_ADAPTER_INVALID_PARM;
	}
	
	osStatus_t status = osSemaphoreAcquire(handle, timeout);
	if (osErrorResource == status || osErrorParameter == status) {
		// LOGE("semaphore wait failed, status: %d", status);
		return OPRT_OS_ADAPTER_INVALID_PARM;
	} else if (osErrorTimeout == status) {
		return OPRT_OS_ADAPTER_SEM_WAIT_FAILED;
	}

	return OPRT_OK;
}

/**
* @brief Post semaphore
*
* @param[in] handle: semaphore handle
*
* @note This API is used for posting semaphore.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_semaphore_post(CONST TKL_SEM_HANDLE handle)
{
	if (!handle) {
		LOGE("semaphore post failed, invalid handle");
		return OPRT_OS_ADAPTER_INVALID_PARM;
	}

	osStatus_t status = osSemaphoreRelease(handle);
	return (osOK == status) ? OPRT_OK : OPRT_COM_ERROR;
}

/**
* @brief Release semaphore
*
* @param[in] handle: semaphore handle
*
* @note This API is used for releasing semaphore.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_semaphore_release(CONST TKL_SEM_HANDLE handle)
{	
	if (!handle) {
		LOGE("semaphore release failed, invalid handle");
		return OPRT_OS_ADAPTER_INVALID_PARM;
	}

	osStatus_t status = osSemaphoreDelete(handle);
	return (osOK == status) ? OPRT_OK : OPRT_COM_ERROR;
}

#if 0
#include "tkl_thread.h"
#include "tkl_system.h"

static TKL_THREAD_HANDLE test_thread1, test_thread2;
static TKL_SEM_HANDLE test_sem;

static void test_thread1_task(void *arg)
{
	int ret = (int)arg;
	LOGI("thread1 running, arg: %d", ret);
	while(1)
	{
		ret = tkl_semaphore_post(test_sem);
		LOGI("thread1 put semaphore %d", ret);
		tkl_system_sleep(1000);
	}
}

static void test_thread2_task(void *arg)
{
	int ret = (int)arg;
	LOGI("thread2 running, arg: %d", ret);
	while(1)
	{
		LOGI("thread2 wait semaphore");
		ret = tkl_semaphore_wait(test_sem, 800);
		if(ret) {
			LOGI("thread2 wait semaphore timeout");
		} else {
			LOGI("thread2 wait semaphore succ");
		}
	}
}

void tkl_semaphore_test(void)
{
	OPERATE_RET ret = 0;
	ret = tkl_semaphore_create_init(&test_sem, 0, 1);
	ret = tkl_thread_create(&test_thread1, "test_thread1", 4096,  2, test_thread1_task, (void *)1);
	ret = tkl_thread_create(&test_thread2, "test_thread1", 4096,  2, test_thread2_task, (void *)2);
}

#endif
