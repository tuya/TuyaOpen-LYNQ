#include <string.h>
#include "rbuffer.h"
#include "cmsis_os2.h"

RBuffer_t *rbuffer_create(uint8_t *addr, uint32_t size, uint32_t count,
                          RBufferLockType_e lock_type)
{
    int ret = -1;
    RBuffer_t *pCT = NULL;
    do
    {
        // allocate buffer pool memory
        pCT = malloc(sizeof(RBuffer_t));
        if(pCT == NULL)
        {
            break;
        }

        memset(pCT, 0, sizeof(RBuffer_t));
        if(addr)
        {
            // if input pointer is not null, do not allocate memory
            pCT->is_allocated = false;
            pCT->addr = addr;
        }
        else
        {
            // if input pointer is null, allocate memory
            pCT->is_allocated = true;
            pCT->addr = malloc(size);
            if(pCT->addr == NULL)
            {
                break;
            }
        }

        // initialize frame state
        pCT->size = size;
        pCT->frame_free.offset = -1;
        pCT->frame_data.offset = -1;

        // if frame maxim count is 0, create a ring queue with maxim count value
        // of 30
        if(0 == count)
        {
            // create ring queue
            pCT->rqueue_item =
                rqueue_create(sizeof(FrameItem_t), 30, 0, lock_type);
        }
        else
        {
            // create ring queue
            pCT->rqueue_item =
                rqueue_create(sizeof(FrameItem_t), count, 0, lock_type);
        }

        // failed to create ring queue, break
        if(pCT->rqueue_item == NULL)
        {
            break;
        }

        // create semaphore
        pCT->lock_data = osSemaphoreNew(count, 0, NULL);
        if(pCT->lock_data == NULL)
        {
            break;
        }

        ret = 0;
    } while(0);

    if(ret != 0 && pCT != NULL)
    {
        rbuffer_close(pCT);
        pCT = NULL;
    }

    return pCT;
}

int rbuffer_close(RBuffer_t *rb)
{
    if(rb != NULL)
    {
        if(rb->is_allocated && rb->addr != NULL)
        {
            free(rb->addr);
            rb->addr = NULL;
        }

        if(rb->rqueue_item != NULL)
        {
            rqueue_destroy(rb->rqueue_item);
            rb->rqueue_item = NULL;
        }

        if(rb->lock_data != NULL)
        {
            osSemaphoreDelete(rb->lock_data);
            rb->lock_data = NULL;
        }

        free(rb);
    }

    return 0;
}

uint32_t rbuffer_get_data_frames_num(RBuffer_t *rb)
{
    uint32_t ret = 0;
    if(rb == NULL || rb->rqueue_item == NULL || rb->lock)
    {
        return 0;
    }

    ret = rqueue_get_count(rb->rqueue_item);
    return ret;
}

int rbuffer_reset(RBuffer_t *rb)
{
    int ret = 0;
    if(rb == NULL || rb->rqueue_item == NULL)
    {
        return -1;
    }

    rb->lock = true;

    rb->front_offset = 0;
    rb->rear_offset = 0;
    ret = rqueue_clear(rb->rqueue_item);

    while(osSemaphoreAcquire(rb->lock_data, 0) == osOK);

    rb->lock = false;
    return ret;
}

int rbuffer_push_back(RBuffer_t *rb, uint8_t *frame, uint32_t frame_size)
{
    int ret = 0;
    uint8_t *tmp_addr = NULL;

    if(rb == NULL || frame == NULL || frame_size <= 0)
    {
        return -1;
    }

    ret = rbuffer_lock_back_free_frame(rb, &tmp_addr, frame_size);
    if(ret == 0)
    {
        memcpy(tmp_addr, frame, frame_size);
        ret = rbuffer_unlock_and_push_back(rb, frame_size);
    }

    return ret;
}

int rbuffer_pop_front(RBuffer_t *rb, uint8_t *frame, uint32_t *frame_size,
                      uint32_t timeout_ms)
{
    int ret = 0;
    void *tmp_addr;
    uint32_t tmp_size;

    if(rb == NULL || frame == NULL || frame_size == NULL)
    {
        return -1;
    }

    ret = rbuffer_lock_front_data_frame(rb, (uint8_t**)&tmp_addr, &tmp_size, timeout_ms);
    if(ret == 0)
    {
        if(*frame_size >= tmp_size)
        {
            memcpy(frame, tmp_addr, tmp_size);
            *frame_size = tmp_size;
        }
        else
        {
            ret = -2;
            *frame_size = 0;
        }
        rbuffer_unlock_and_pop_front(rb);
    }

    return ret;
}

int rbuffer_lock_back_free_frame(RBuffer_t *rb, uint8_t **frame_addr,
                                 uint32_t frame_size)
{
    if(rb == NULL || frame_addr == NULL || rb->rqueue_item == NULL || rb->lock)
    {
        return -1;
    }

    rb->frame_free.offset = -1;

    if((rb->rear_offset > rb->front_offset) ||
       (rb->rear_offset == rb->front_offset &&
        rqueue_is_empty(rb->rqueue_item)))
    {
        if(rb->size - rb->rear_offset >= frame_size)
        {
            rb->frame_free.offset = rb->rear_offset;
        }
        else if(rb->front_offset >= frame_size)
        {
            rb->frame_free.offset = 0;
        }
    }
    else if(rb->rear_offset < rb->front_offset)
    {
        if(rb->front_offset - rb->rear_offset >= frame_size)
        {
            rb->frame_free.offset = rb->rear_offset;
        }
    }

    // no free back space valid
    if(rb->frame_free.offset < 0)
    {
        return -2;
    }

    rb->frame_free.size = frame_size;
    *frame_addr = rb->addr + rb->frame_free.offset;

    return 0;
}

int rbuffer_unlock_and_push_back(RBuffer_t *rb, uint32_t frame_size)
{
    int ret = 0;

    if(rb == NULL || rb->rqueue_item == NULL || rb->lock)
    {
        return -1;
    }

    // no free space valid
    if(rb->frame_free.offset < 0 || rb->frame_free.size < frame_size)
    {
        return -2;
    }
    rb->frame_free.size = frame_size;
    ret = rqueue_push_back(rb->rqueue_item, &rb->frame_free);
    if(ret != 0)
    {
        return -3;
    }

    rb->rear_offset = rb->frame_free.offset + frame_size;

    osSemaphoreRelease(rb->lock_data);

    rb->frame_free.offset = -1;
    rb->frame_free.size = 0;

    return 0;
}

int rbuffer_unlock_back_free_frame(RBuffer_t *rb)
{
    int ret = 0;

    if(rb == NULL || rb->lock)
    {
        return -1;
    }

    if(rb->frame_free.offset < 0)
    {
        return -2;
    }

    rb->frame_free.offset = -1;
    rb->frame_free.size = 0;

    return ret;
}

int rbuffer_lock_front_data_frame(RBuffer_t *rb, uint8_t **frame_addr,
                                  uint32_t *frame_size, uint32_t timeout_ms)
{
    int ret = 0;

    if(rb == NULL || frame_addr == NULL || frame_size == NULL ||
       rb->rqueue_item == NULL || rb->lock)
    {
        return -1;
    }

    rb->frame_data.offset = -1;

    if(osSemaphoreAcquire(rb->lock_data, timeout_ms) != osOK)
    {
        return -2;
    }
    osSemaphoreRelease(rb->lock_data);

    if(rqueue_is_empty(rb->rqueue_item))
    {
        return -3;
    }

    ret = rqueue_get_front(rb->rqueue_item, &rb->frame_data);
    if(ret != 0)
    {
        rb->frame_data.offset = -1;
        return -4;
    }

    *frame_addr = rb->addr + rb->frame_data.offset;
    *frame_size = rb->frame_data.size;

    return 0;
}

int rbuffer_unlock_and_pop_front(RBuffer_t *rb)
{
    int ret = 0;

    if(rb == NULL || rb->rqueue_item == NULL || rb->lock)
    {
        return -1;
    }

    if(rb->frame_data.offset < 0)
    {
        return -2;
    }

    if(rqueue_is_empty(rb->rqueue_item))
    {
        return -3;
    }

    ret = rqueue_pop_front(rb->rqueue_item, &rb->frame_data);
    if(ret != 0)
    {
        return -4;
    }

    rb->front_offset = rb->frame_data.offset + rb->frame_data.size;

    osSemaphoreRelease(rb->lock_data);

    rb->frame_data.offset = -1;
    rb->frame_data.size = 0;

    return 0;
}

int rbuffer_unlock_front_data_frame(RBuffer_t *rb)
{
    int ret = 0;
    if(rb == NULL || rb->lock)
    {
        return -1;
    }

    if(rb->frame_data.offset < 0)
    {
        return -2;
    }

    rb->frame_data.offset = -1;
    rb->frame_data.size = 0;

    return ret;
}