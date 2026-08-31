#include <string.h>
#include "rqueue.h"
#include "cmsis_os2.h"
#include "system_ec7xx.h"

RQueue_t* rqueue_create(int item_size, int item_count, bool auto_increase,
                        RBufferLockType_e lock_type)
{
    int ret = -1;
    RQueue_t* rq = NULL;

    do
    {
        rq = (RQueue_t*)malloc(sizeof(RQueue_t));
        if(rq == NULL)
        {
            break;
        }

        // set ring queue value
        memset(rq, 0, sizeof(RQueue_t));
        rq->item_size = item_size;
        rq->auto_increase = auto_increase;
        rq->item_array_size = item_count;
        rq->item_array = malloc(rq->item_size * rq->item_array_size);
        rq->lock_type = lock_type;
        if(rq->item_array == NULL)
        {
            break;
        }

        // create mutex lock by type
        if(RQ_LOCK_TYPE_MUTEX == rq->lock_type)
        {
            rq->lock = osMutexNew(NULL);
            if(rq->lock == NULL)
            {
                break;
            }
        }

        ret = 0;
    } while(0);

    if(ret != 0)
    {
        // failed to create and close pointer
        rqueue_destroy(rq);
        rq = NULL;
    }

    return rq;
}

int rqueue_destroy(RQueue_t* rq)
{
    if(rq != NULL)
    {
        if(rq->item_array != NULL)
        {
            free(rq->item_array);
            rq->item_array = NULL;
        }

        if(RQ_LOCK_TYPE_MUTEX == rq->lock_type)
        {
            if(rq->lock != NULL)
            {
                osMutexDelete(rq->lock);
                rq->lock = NULL;
            }
        }
    }

    return 0;
}

int rqueue_get_capacity(RQueue_t* rq)
{
    int ret;
    uint32_t isrm = 0;
    if(rq == NULL)
    {
        return 0;
    }

    if(RQ_LOCK_TYPE_MUTEX == rq->lock_type)
    {
        osMutexAcquire(rq->lock, osWaitForever);
    }
    else if(RQ_LOCK_TYPE_CRITICAL == rq->lock_type)
    {
        isrm = SaveAndSetIRQMask();
    }

    ret = rq->item_array_size;

    if(RQ_LOCK_TYPE_MUTEX == rq->lock_type)
    {
        osMutexRelease(rq->lock);
    }
    else if(RQ_LOCK_TYPE_CRITICAL == rq->lock_type)
    {
        RestoreIRQMask(isrm);
    }
    return ret;
}

int rqueue_increase_capacity(RQueue_t* rq)
{
    int ret;
    uint32_t new_size;
    uint8_t* new_array;
    uint32_t isrm = 0;
    if(rq == NULL)
    {
        return RQ_ERR_INVALIDPARAM;
    }

    // if mutex lock is used,lock
    if(RQ_LOCK_TYPE_MUTEX == rq->lock_type)
    {
        osMutexAcquire(rq->lock, osWaitForever);
    }
    else if(RQ_LOCK_TYPE_CRITICAL == rq->lock_type)
    {
        isrm = SaveAndSetIRQMask();
    }

    // calculate new queue size
    new_size = rq->item_array_size * 2;
    // allocate memory
    new_array = malloc(rq->item_size * new_size);
    if(new_array != NULL)
    {
        ret = RQ_SUCCESS;

        if(rq->count > 0)
        {
            if(rq->rear > rq->front)
            {
                memcpy(new_array, rq->item_array + rq->item_size * rq->front,
                       (rq->rear - rq->front) * rq->item_size);
            }
            else
            {
                memcpy(new_array, rq->item_array + rq->item_size * rq->front,
                       (rq->item_array_size - rq->front) * rq->item_size);
                if(rq->rear > 0)
                {
                    memcpy(new_array + rq->item_size *
                                           (rq->item_array_size - rq->front),
                           rq->item_array, rq->rear * rq->item_size);
                }
            }
        }
        // freee old memory
        free(rq->item_array);
        rq->item_array = new_array;
        rq->item_array_size = new_size;
        rq->front = 0;
        rq->rear = rq->count % rq->item_array_size;
    }
    else
    {
        ret = RQ_ERR_OUTOFMEM;
    }

    // unlock
    if(RQ_LOCK_TYPE_MUTEX == rq->lock_type)
    {
        osMutexRelease(rq->lock);
    }
    else if(RQ_LOCK_TYPE_CRITICAL == rq->lock_type)
    {
        RestoreIRQMask(isrm);
    }

    return ret;
}

uint32_t rqueue_get_count(RQueue_t* rq)
{
    uint32_t ret;
    uint32_t isrm = 0;
    if(rq == NULL)
    {
        return 0;
    }

    if(RQ_LOCK_TYPE_MUTEX == rq->lock_type)
    {
        osMutexAcquire(rq->lock, osWaitForever);
    }
    else if(RQ_LOCK_TYPE_CRITICAL == rq->lock_type)
    {
        isrm = SaveAndSetIRQMask();
    }

    ret = rq->count;

    if(RQ_LOCK_TYPE_MUTEX == rq->lock_type)
    {
        osMutexRelease(rq->lock);
    }
    else if(RQ_LOCK_TYPE_CRITICAL == rq->lock_type)
    {
        RestoreIRQMask(isrm);
    }

    return ret;
}

bool rqueue_is_empty(RQueue_t* rq)
{
    bool ret = false;
    uint32_t isrm = 0;
    if(rq == NULL)
    {
        return true;
    }

    if(RQ_LOCK_TYPE_MUTEX == rq->lock_type)
    {
        osMutexAcquire(rq->lock, osWaitForever);
    }
    else if(RQ_LOCK_TYPE_CRITICAL == rq->lock_type)
    {
        isrm = SaveAndSetIRQMask();
    }

    ret = (rq->count == 0);

    if(RQ_LOCK_TYPE_MUTEX == rq->lock_type)
    {
        osMutexRelease(rq->lock);
    }
    else if(RQ_LOCK_TYPE_CRITICAL == rq->lock_type)
    {
        RestoreIRQMask(isrm);
    }

    return ret;
}

bool rqueue_is_full(RQueue_t* rq)
{
    bool ret = false;
    uint32_t isrm = 0;
    if(rq == NULL)
    {
        return false;
    }

    if(RQ_LOCK_TYPE_MUTEX == rq->lock_type)
    {
        osMutexAcquire(rq->lock, osWaitForever);
    }
    else if(RQ_LOCK_TYPE_CRITICAL == rq->lock_type)
    {
        isrm = SaveAndSetIRQMask();
    }
    ret = (rq->count == rq->item_array_size);
    if(RQ_LOCK_TYPE_MUTEX == rq->lock_type)
    {
        osMutexRelease(rq->lock);
    }
    else if(RQ_LOCK_TYPE_CRITICAL == rq->lock_type)
    {
        RestoreIRQMask(isrm);
    }

    return ret;
}

int rqueue_clear(RQueue_t* rq)
{
    uint32_t isrm = 0;
    if(rq == NULL)
    {
        return RQ_ERR_INVALIDPARAM;
    }

    if(RQ_LOCK_TYPE_MUTEX == rq->lock_type)
    {
        osMutexAcquire(rq->lock, osWaitForever);
    }
    else if(RQ_LOCK_TYPE_CRITICAL == rq->lock_type)
    {
        isrm = SaveAndSetIRQMask();
    }
    rq->count = 0;
    rq->front = 0;
    rq->rear = 0;
    if(RQ_LOCK_TYPE_MUTEX == rq->lock_type)
    {
        osMutexRelease(rq->lock);
    }
    else if(RQ_LOCK_TYPE_CRITICAL == rq->lock_type)
    {
        RestoreIRQMask(isrm);
    }

    return RQ_SUCCESS;
}

int rqueue_push_back(RQueue_t* rq, void* item)
{
    int ret = 0;
    uint32_t isrm = 0;
    if(rq == NULL || item == NULL)
    {
        return RQ_ERR_INVALIDPARAM;
    }

    if(rqueue_is_full(rq))
    {
        if(rq->auto_increase)
        {
            ret = rqueue_increase_capacity(rq);
            if(ret != 0)
            {
                return ret;
            }
        }
        else
        {
            return RQ_ERR_OVERFLOW;
        }
    }

    if(RQ_LOCK_TYPE_MUTEX == rq->lock_type)
    {
        osMutexAcquire(rq->lock, osWaitForever);
    }
    else if(RQ_LOCK_TYPE_CRITICAL == rq->lock_type)
    {
        isrm =SaveAndSetIRQMask();
    }
    memcpy(rq->item_array + rq->item_size * rq->rear, item, rq->item_size);
    rq->rear = (rq->rear + 1) % rq->item_array_size;
    rq->count++;
    if(RQ_LOCK_TYPE_MUTEX == rq->lock_type)
    {
        osMutexRelease(rq->lock);
    }
    else if(RQ_LOCK_TYPE_CRITICAL == rq->lock_type)
    {
        RestoreIRQMask(isrm);
    }

    return RQ_SUCCESS;
}

int rqueue_pop_front(RQueue_t* rq, void* item)
{
    uint32_t isrm = 0;
    if((rq == NULL || item == NULL))
    {
        return RQ_ERR_INVALIDPARAM;
    }

    if(rqueue_is_empty(rq))
    {
        return RQ_ERR_NOITEM;
    }

    if(RQ_LOCK_TYPE_MUTEX == rq->lock_type)
    {
        osMutexAcquire(rq->lock, osWaitForever);
    }
    else if(RQ_LOCK_TYPE_CRITICAL == rq->lock_type)
    {
        isrm = SaveAndSetIRQMask();
    }
    if(item)
    {
        memcpy(item, rq->item_array + rq->item_size * rq->front, rq->item_size);
    }
    rq->front = (rq->front + 1) % rq->item_array_size;
    rq->count--;
    if(RQ_LOCK_TYPE_MUTEX == rq->lock_type)
    {
        osMutexRelease(rq->lock);
    }
    else if(RQ_LOCK_TYPE_CRITICAL == rq->lock_type)
    {
        RestoreIRQMask(isrm);
    }
    return RQ_SUCCESS;
}

int rqueue_get_front(RQueue_t* rq, void* item)
{
    uint32_t isrm = 0;
    if(rq == NULL || item == NULL)
    {
        return RQ_ERR_INVALIDPARAM;
    }

    if(rqueue_is_empty(rq))
    {
        return RQ_ERR_NOITEM;
    }

    if(RQ_LOCK_TYPE_MUTEX == rq->lock_type)
    {
        osMutexAcquire(rq->lock, osWaitForever);
    }
    else if(RQ_LOCK_TYPE_CRITICAL == rq->lock_type)
    {
        isrm = SaveAndSetIRQMask();
    }
    memcpy(item, rq->item_array + rq->item_size * rq->front, rq->item_size);
    if(RQ_LOCK_TYPE_MUTEX == rq->lock_type)
    {
        osMutexRelease(rq->lock);
    }
    else if(RQ_LOCK_TYPE_CRITICAL == rq->lock_type)
    {
        RestoreIRQMask(isrm);
    }

    return RQ_SUCCESS;
}