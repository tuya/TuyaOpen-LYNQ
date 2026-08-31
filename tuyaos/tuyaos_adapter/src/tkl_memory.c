#include <stdlib.h>
#include "tkl_memory.h"
#include "cmsis_os2.h"
#include "vlog.h"
#include "os_common.h"
#include "tkl_mutex.h"
#include "tuya_mem_heap.h"
#include "tkl_output.h"

#define MAX_HEAP_SIZE (1024 * 1024 * 2)
static char s_mem_buf[MAX_HEAP_SIZE] __attribute__((aligned(8))) = {0};

static HEAP_HANDLE s_heap_handle    = NULL;
static TKL_MUTEX_HANDLE mem_mutex = NULL;

static void __heap_lock(void)
{
    tkl_mutex_lock(mem_mutex);
}

static void __heap_unlock(void)
{
    tkl_mutex_unlock(mem_mutex);
}

static void __heap_init(void)
{
    heap_context_t ctx = { 0 };
    ctx.dbg_output     = tkl_log_output;
    ctx.enter_critical = __heap_lock;
    ctx.exit_critical  = __heap_unlock;

    tkl_mutex_create_init(&mem_mutex);
    tuya_mem_heap_init(&ctx);
    tuya_mem_heap_create(s_mem_buf, MAX_HEAP_SIZE, &s_heap_handle);
} 

/**
* @brief Alloc memory of system
*
* @param[in] size: memory size
*
* @note This API is used to alloc memory of system.
*
* @return the memory address malloced
*/
VOID_T *tkl_system_malloc(SIZE_T size)
{
    if (!s_heap_handle) {
        __heap_init();
    }
    void * ptr = tuya_mem_heap_malloc(s_heap_handle, size);
    if(NULL == ptr)
        LOGE("malloc %d failed, freeheap_size:%d", size, tkl_system_get_free_heap_size());
    return ptr;

//     void * ptr = malloc(size);
//     if(NULL == ptr)
//         LOGE("malloc %d failed, freeheap_size:%d", size, tkl_system_get_free_heap_size());
//     return ptr;
}

VOID_T *tkl_system_psram_malloc(SIZE_T size)
{
    return tkl_system_malloc(size);
}

VOID_T tkl_system_psram_free(VOID_T* ptr)
{
    tkl_system_free(ptr);
}

/**
* @brief Free memory of system
*
* @param[in] ptr: memory point
*
* @note This API is used to free memory of system.
*
* @return VOID_T
*/
VOID_T tkl_system_free(VOID_T* ptr)
{
    if (ptr) {
        // free(ptr);
        tuya_mem_heap_free(s_heap_handle, ptr);
    }
}

/**
 * @brief Allocate and clear the memory
 *
 * @param[in]       nitems      the numbers of memory block
 * @param[in]       size        the size of the memory block
 *
 * @return the memory address calloced
 */
VOID_T *tkl_system_calloc(size_t nitems, size_t size)
{
    // void * ptr = calloc(nitems, size);
    void *ptr = tuya_mem_heap_calloc(s_heap_handle, nitems * size);
    if(NULL == ptr)
        LOGE("calloc %d failed, freeheap_size:%d", size, tkl_system_get_free_heap_size());
    return ptr;
}

/**
 * @brief Re-allocate the memory
 *
 * @param[in]       nitems      source memory address
 * @param[in]       size        the size after re-allocate
 *
 * @return VOID_T
 */
VOID_T *tkl_system_realloc(VOID_T* ptr, size_t size)
{
    // return realloc(ptr, size);
    return tuya_mem_heap_realloc(s_heap_handle, ptr, size);
}

/**
* @brief Get system free heap size
*
* @param none
*
* @return heap size
*/
INT_T tkl_system_get_free_heap_size(VOID_T)
{
    // return (INT_T)xPortGetFreeHeapSize();
    return tuya_mem_heap_available(s_heap_handle);
}

#if 0
void tkl_memory_test(void)
{
    int free_heap_size = xPortGetFreeHeapSize();
    LOGI("free_heap_size:%d", free_heap_size);

    char * ptr;
    for(int i = 1;i< 1024;i++) {
        ptr = malloc( 1024 * 10);
        LOGI("free_heap_size:%d", xPortGetFreeHeapSize());
        if(ptr == NULL) {
            LOGE("malloc %dk failed", 10);
            break;
        }
        LOGI("malloc %dk succ", i * 10);
    }
}
#endif