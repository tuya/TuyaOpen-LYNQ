#include "volc_mutex.h"

//#include <pthread.h>
#include "cmsis_os2.h"
#include "commontypedef.h"
#include "volc_errno.h"
#include "volc_memory.h"

/* 最好不要依赖可重入锁 */
volc_mutex_t volc_mutex_create(bool reentrant) {
	osMutexAttr_t attr = {.attr_bits = 1};
     osMutexId_t mutex = osMutexNew(&attr);
    if (!mutex) {
        return NULL;
    }
  
    return (volc_mutex_t)mutex;
}

void volc_mutex_lock(volc_mutex_t mutex) {
    if (!mutex) {
        return;
    }
	osMutexAcquire((osMutexId_t)mutex, osWaitForever);
	return;
}

bool volc_mutex_trylock(volc_mutex_t mutex) {
	if(!mutex)
		return false;
    return (osMutexAcquire((osMutexId_t)mutex, 0) == osOK ? TRUE : FALSE);
    return TRUE;
}

void volc_mutex_unlock(volc_mutex_t mutex) {
     if (!mutex) {
        return;
    }
	osMutexRelease((osMutexId_t)mutex);
	return;
}

void volc_mutex_destroy(volc_mutex_t mutex) {
     if (!mutex) {
        return;
    }

    osMutexDelete((osMutexId_t)mutex);
	 return;
}
