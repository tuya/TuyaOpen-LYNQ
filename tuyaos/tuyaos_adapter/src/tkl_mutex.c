#include <stdlib.h>
#include "tkl_mutex.h"
#include "cmsis_os2.h"
#include "vlog.h"

#undef LOGD
#define LOGD(fmt, ...)

/**
* @brief Create mutex
*
* @param[out] pMutexHandle: mutex handle
*
* @note This API is used to create and init mutex.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_mutex_create_init(TKL_MUTEX_HANDLE* pMutexHandle)
{	
	if (!pMutexHandle) {
		LOGE("create mutex failed, para is null");
		return OPRT_OS_ADAPTER_INVALID_PARM;
	}

	osMutexAttr_t attr = {
		.name = NULL,
		.attr_bits = osMutexRecursive,
		.cb_mem = NULL,
		.cb_size = 0,
	};
	
	osMutexId_t id = osMutexNew(&attr);
	if (NULL == id) {
		LOGE("create mutex failed");
		return OPRT_COM_ERROR;
	}

	*pMutexHandle = id;
	LOGD("create mutex success");
	return OPRT_OK;
}

/**
* @brief Lock mutex
*
* @param[in] mutexHandle: mutex handle
*
* @note This API is used to lock mutex.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_mutex_lock(CONST TKL_MUTEX_HANDLE mutexHandle)
{	
	if (!mutexHandle) {
		LOGE("mutex lock failed, invalid param");
		return OPRT_OS_ADAPTER_INVALID_PARM;
	}

	osStatus_t status = osMutexAcquire(mutexHandle, osWaitForever);
	if (osOK != status) {
		LOGE("mutex lock failed, status: %d", status);
		return OPRT_COM_ERROR;
	}

	LOGD("mutex lock success");
	return OPRT_OK;
}

/**
* @brief Unlock mutex
*
* @param[in] mutexHandle: mutex handle
*
* @note This API is used to unlock mutex.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_mutex_unlock(CONST TKL_MUTEX_HANDLE mutexHandle)
{	
	if (!mutexHandle) {
		LOGE("mutex unlock failed, invalid param");
		return OPRT_OS_ADAPTER_INVALID_PARM;
	}
	
	osStatus_t status = osMutexRelease(mutexHandle);
	if (osOK != status) {
		LOGE("mutex unlock failed, status: %d", status);
		return OPRT_COM_ERROR;
	}
	
	LOGD("mutex unlock success");
	return OPRT_OK;
}

/**
* @brief Try Lock mutex
*
* @param[in] mutexHandle: mutex handle
*
* @note This API is used to try lock mutex.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_mutex_trylock(CONST TKL_MUTEX_HANDLE mutexHandle)
{	
	if (!mutexHandle) {
		LOGE("mutex try lock failed, invalid param");
		return OPRT_OS_ADAPTER_INVALID_PARM;
	}

	osStatus_t status = osMutexAcquire(mutexHandle, 0);
	if (osOK != status) {
		LOGE("mutex try lock failed");
		return OPRT_OS_ADAPTER_MUTEX_LOCK_FAILED;
	}

	LOGD("mutex try lock success");
	return OPRT_OK;
}

/**
* @brief Release mutex
*
* @param[in] mutexHandle: mutex handle
*
* @note This API is used to release mutex.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_mutex_release(CONST TKL_MUTEX_HANDLE mutexHandle)
{	
	if (!mutexHandle) {
		LOGE("mutex release failed, invalid param");
		return OPRT_OS_ADAPTER_INVALID_PARM;
	}
	
	osStatus_t status = osMutexDelete(mutexHandle);
	if (osOK != status) {
		LOGE("mutex release failed, status: %d", status);
		return OPRT_COM_ERROR;
	}
	
	LOGD("mutex release success");
	return OPRT_OK;
}

#if 0
#include "tkl_thread.h"
#include "tkl_system.h"

static TKL_THREAD_HANDLE test_thread1, test_thread2;
static TKL_MUTEX_HANDLE test_mutex;
static int test_value = 0;

static void test_thread1_task(void *arg)
{
	int ret = (int)arg;
	LOGI("thread1 running, arg: %d", ret);
	while(1)
	{
		ret = tkl_mutex_lock(test_mutex);
		LOGI("thread1 lock %d, value: %d", ret, test_value);
		test_value = 1;
		tkl_mutex_unlock(test_mutex);
		tkl_system_sleep(100);
	}
}

static void test_thread2_task(void *arg)
{
	int ret = (int)arg;
	LOGI("thread2 running, arg: %d", ret);
	while(1)
	{
		ret = tkl_mutex_lock(test_mutex);
		LOGI("thread2 lock %d, value: %d", ret, test_value);
		test_value = 2;
		tkl_mutex_unlock(test_mutex);
		tkl_system_sleep(100);
	}
}

void tkl_mutex_test(void)
{
	OPERATE_RET ret = 0;
	ret = tkl_mutex_create_init(&test_mutex);
	ret = tkl_thread_create(&test_thread1, "test_thread1", 4096,  2, test_thread1_task, (void *)1);
	ret = tkl_thread_create(&test_thread2, "test_thread1", 4096,  2, test_thread2_task, (void *)2);
}

#endif