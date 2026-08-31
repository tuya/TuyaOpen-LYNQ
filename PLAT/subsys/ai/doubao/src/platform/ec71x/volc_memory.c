#include DEBUG_LOG_HEADER_FILE
#include "volc_memory.h"

#include <stdlib.h>
#include "cmsis_os2.h"
#include "exception_process.h"
void* volc_malloc(size_t size) {
	void *ptr = NULL;
    ptr =  malloc(size);
	EC_ASSERT(ptr,ptr,size,0);
	//ECPLAT_PRINTF(UNILOG_PLAT_VOLC,volc_malloc,P_DEBUG,"ptr(%p)_size(%d)",ptr,size);
	return ptr;
}

void* volc_align_alloc(size_t size, size_t alignment) {
	void *ptr = NULL;
    ptr =  malloc(size);
	EC_ASSERT(ptr,ptr,size,alignment);
	//ECPLAT_PRINTF(UNILOG_PLAT_VOLC,volc_align_alloc,P_DEBUG,"ptr(%p)_size(%d)",ptr,size);
	return ptr;
}

void* volc_calloc(size_t num, size_t size) {
	void *ptr = NULL;
 	ptr = calloc(num,size);
	EC_ASSERT(ptr,ptr,size,0);
	//ECPLAT_PRINTF(UNILOG_PLAT_VOLC,volc_calloc,P_DEBUG,"ptr(%p)_size(%d)",ptr,size*num);
	return ptr;
}


void* volc_realloc(void* ptr, size_t new_size) {
    ptr = realloc(ptr,new_size);
	EC_ASSERT(ptr,ptr,new_size,0);
	//ECPLAT_PRINTF(UNILOG_PLAT_VOLC,volc_realloc,P_DEBUG,"ptr(%p)_size(%d)",ptr,new_size);
	return ptr;
}

void volc_free(void* ptr) {
    free(ptr);
}


bool volc_memory_check(void* ptr, uint8_t val, size_t size) {
    uint8_t* p_buf = (uint8_t *)ptr;
    
    if (NULL == p_buf) {
        return false;
    }

    for (int i = 0; i < size; p_buf++, i++) {
        if (*p_buf != val) {
            return false;
        }
    }

    return true;
}