#ifndef _CTIOT_AEP_MSG_QUEUE_H
#define _CTIOT_AEP_MSG_QUEUE_H

#include "../port/ct_platform.h"


typedef struct _ctiot_list_t
{
    struct _ctiot_list_t * next;
    uint16_t   msgid;
} ctiot_list_t;

typedef struct _ctiot_msg_list_head{
    ctiot_list_t* head;
    ctiot_list_t* tail;
    uint16_t msg_count;
    uint16_t max_msg_num;
    thread_mutex_t mut;
    uint8_t init;
    uint16_t recv_msg_num;
    uint16_t del_msg_count;
}ctiot_msg_list_head;

ctiot_msg_list_head* ctiot_coap_queue_init(uint32_t maxMsgCount);
uint16_t ctiot_coap_queue_add(ctiot_msg_list_head* list, void* ptr, void* delptr);
ctiot_list_t* ctiot_coap_queue_find(ctiot_msg_list_head* list, uint16_t msgid);
ctiot_list_t* ctiot_coap_queue_get(ctiot_msg_list_head* list);
uint16_t ctiot_coap_queue_remove(ctiot_msg_list_head* list, uint16_t msgid, void *ptr);
uint16_t ctiot_coap_queue_free(ctiot_msg_list_head** list);

#endif
