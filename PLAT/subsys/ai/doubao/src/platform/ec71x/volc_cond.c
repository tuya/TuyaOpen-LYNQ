#include "volc_cond.h"

#include "cmsis_os2.h"

#include "volc_errno.h"
#include "volc_memory.h"
#include "volc_time.h"

volc_cond_t volc_cond_create(void) {
    return NULL;
}

uint32_t volc_cond_signal(volc_cond_t cond) {
   	(void)cond;
    return VOLC_SUCCESS;

}

uint32_t volc_cond_broadcast(volc_cond_t cond) {
    (void)cond;

    return VOLC_SUCCESS;
}

uint32_t volc_cond_wait(volc_cond_t cond, volc_mutex_t mutex, uint64_t timeout) {
    (void)cond;
	(void)mutex;
	(void)timeout;
    return VOLC_SUCCESS;
}

void volc_cond_destroy(volc_cond_t cond) {
    (void)cond;
	return;
}

