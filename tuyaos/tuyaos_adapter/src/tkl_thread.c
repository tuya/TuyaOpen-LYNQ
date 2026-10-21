#include <string.h>
#include "cmsis_os2.h"
#include "tkl_thread.h"
#include "vlog.h"

#define THREAD_MAX_STACK_SIZE (64*1024 - 8)

/**
* @brief Create thread
*
* @param[out] thread: thread handle
* @param[in] name: thread name
* @param[in] stack_size: stack size of thread
* @param[in] priority: priority of thread
* @param[in] func: the main thread process function
* @param[in] arg: the args of the func, can be null
*
* @note This API is used for creating thread.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_thread_create(TKL_THREAD_HANDLE* thread,
                              CONST CHAR_T* name,
                              UINT_T stack_size,
                              UINT_T priority,
                              CONST THREAD_FUNC_T func,
                              VOID_T* CONST arg)
{	
	osThreadAttr_t attr;
	memset(&attr, 0, sizeof(attr));
	attr.name = name;
	stack_size = stack_size > THREAD_MAX_STACK_SIZE ? THREAD_MAX_STACK_SIZE : stack_size;
    attr.stack_size = stack_size;
	switch (priority) {
		// 优先级超过24，osPriorityNormal的优先级为20,超过24时，在下发dp时，测试会出现cp dump，按原厂建议，优先级不超过osPriorityNormal
		case 5: attr.priority = osPriorityNormal; break;
		case 4: attr.priority = osPriorityNormal; break;
		case 3: attr.priority = osPriorityLow; break;
		case 2: attr.priority = osPriorityLow; break;
		case 1: attr.priority = osPriorityIdle; break;
		case 0: attr.priority = osPriorityIdle; break;
		default: attr.priority = osPriorityIdle; break;
	}

	*thread = osThreadNew(func, arg, &attr);
	if (NULL == *thread) {
		LOGE("create thread %s failed", name);
		return OPRT_OS_ADAPTER_THRD_CREAT_FAILED;
	}
	
	return OPRT_OK;
}

OPERATE_RET tkl_thread_create_in_psram(TKL_THREAD_HANDLE* thread,
                              CONST CHAR_T* name,
                              UINT_T stack_size,
                              UINT_T priority,
                              CONST THREAD_FUNC_T func,
                              VOID_T* CONST arg)
{
    return tkl_thread_create(thread, name, stack_size, priority, func, arg);
}

/**
* @brief Terminal thread and release thread resources
*
* @param[in] thread: thread handle
*
* @note This API is used to terminal thread and release thread resources.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_thread_release(CONST TKL_THREAD_HANDLE thread)
{
    osStatus_t status = osThreadTerminate(thread);
	if (osOK != status) {
		LOGE("release thread %p failed, status: %d", thread, status);
		return OPRT_COM_ERROR;
	}

    return OPRT_OK;
}

/**
* @brief Get the thread stack's watermark
*
* @param[in] thread: thread handle
* @param[out] watermark: watermark in Bytes
*
* @note This API is used to get the thread stack's watermark.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_thread_get_watermark(CONST TKL_THREAD_HANDLE thread, UINT_T* watermark)
{
    *watermark = osThreadGetStackSpace(thread);
    return OPRT_OK;
}

/**
* @brief Get current thread thread handle
*
* @param[out] thread: thread handle
*
* @note This API is used to get the thread handle.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_thread_get_id(TKL_THREAD_HANDLE *thread)
{
    if (NULL == thread) {
		LOGE("get id failed, thread is null");
        return OPRT_OS_ADAPTER_INVALID_PARM;
    }
    *thread = osThreadGetId();
    return OPRT_OK;
}

/**
* @brief Set name of self thread
*
* @param[in] name: thread name
*
* @note This API is used to set name of self thread.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_thread_set_self_name(CONST CHAR_T* name)
{
	LOGE("thread set self name not support, name: %s", name);
    return OPRT_NOT_SUPPORTED;
}

/**
* @brief Check thread is self thread
*
* @param[in] thread: thread handle
* @param[out] is_self: is self thread or not
*
* @note This API is used to check thread is self thread.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_thread_is_self(TKL_THREAD_HANDLE thread, BOOL_T* is_self)
{
    TKL_THREAD_HANDLE self_handle;

    if (NULL == thread || NULL == is_self) {
		LOGE("thread is self, param null");
        return OPRT_OS_ADAPTER_INVALID_PARM;
    }

    tkl_thread_get_id(&self_handle);
    if (NULL == self_handle) {
		LOGE("get current thread id failed");
        return OPRT_OS_ADAPTER_THRD_JUDGE_SELF_FAILED;
    }

    *is_self = (thread == self_handle);

    return OPRT_OK;
}

/**
* @brief Get thread priority
*
* @param[in] thread: thread handle, If NULL indicates the current thread
* @param[in] priority: thread priority
*
* @note This API is used to get thread priority.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_thread_get_priority(TKL_THREAD_HANDLE thread, INT_T *priority)
{
    LOGE("thread get priority not support!");
    return OPRT_NOT_SUPPORTED;
}

/**
* @brief Set thread priority
*
* @param[in] thread: thread handle, If NULL indicates the current thread
* @param[in] priority: thread priority
*
* @note This API is used to Set thread priority.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_thread_set_priority(TKL_THREAD_HANDLE thread, INT_T priority)
{
	osPriority_t osprio;
	osStatus_t status;
	switch (priority) {
		case 5: osprio = osPriorityNormal; break;
		case 4: osprio = osPriorityNormal; break;
		case 3: osprio = osPriorityLow; break;
		case 2: osprio = osPriorityLow; break;
		case 1: osprio = osPriorityIdle; break;
		case 0: osprio = osPriorityIdle; break;
		default: osprio = osPriorityIdle; break;

	}
	
	status = osThreadSetPriority(thread, osprio);
	if (osOK != status) {
		LOGE("thread set priority failed, thread: %p, prio: %d", thread, priority);
		return OPRT_COM_ERROR;
	}
    return OPRT_OK;
}

/**
* @brief Diagnose the thread(dump task stack, etc.)
*
* @param[in] thread: thread handle
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_thread_diagnose(TKL_THREAD_HANDLE thread)
{
	LOGE("thread diagnose not supported, thead: %p", thread);
    return OPRT_NOT_SUPPORTED;
}
