#ifndef __RBUFFER_H__
#define __RBUFFER_H__

#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os2.h"
#include "rqueue.h"


typedef struct FrameItem_
{
    int32_t offset;
    uint32_t size;
} FrameItem_t;

typedef struct RBuffer_
{
    uint8_t *addr;
    bool is_allocated;
    uint32_t size;
    uint32_t front_offset;
    uint32_t rear_offset;
    FrameItem_t frame_free;
    FrameItem_t frame_data;
    RQueue_t *rqueue_item;
    osSemaphoreId_t lock_data;
    bool lock;
} RBuffer_t;

RBuffer_t *rbuffer_create(uint8_t *addr, uint32_t size, uint32_t count,
                          RBufferLockType_e lock_type);
int rbuffer_close(RBuffer_t *rb);
uint32_t rbuffer_get_data_frames_num(RBuffer_t *rb);
int rbuffer_reset(RBuffer_t *rb);

int rbuffer_push_back(RBuffer_t *rb, uint8_t *frame, uint32_t frame_size);
int rbuffer_pop_front(RBuffer_t *rb, uint8_t *frame, uint32_t *frame_size,
                      uint32_t Timeout);
int rbuffer_lock_back_free_frame(RBuffer_t *rb, uint8_t **frame_addr,
                                 uint32_t frame_size);
int rbuffer_unlock_and_push_back(RBuffer_t *rb, uint32_t frame_size);
int rbuffer_unlock_back_free_frame(RBuffer_t *rbuffer);

int rbuffer_lock_front_data_frame(RBuffer_t *rb, uint8_t **frame_addr,
                                  uint32_t *frame_size, uint32_t timeout_ms);
int rbuffer_unlock_and_pop_front(RBuffer_t *rb);
int rbuffer_unlock_front_data_frame(RBuffer_t *rbuffer);

#endif /* __RBUFFER_H__ */
