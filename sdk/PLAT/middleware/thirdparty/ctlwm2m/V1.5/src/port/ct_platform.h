/*******************************************************************************
 *
 * Copyright (c) 2015 Intel Corporation and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * The Eclipse Distribution License is available at
 *    http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Simon Bernard - initial API and implementation
 *    Christian Renz - Please refer to git log
 *
 *******************************************************************************/

#ifndef CT_PLATFORM_H_
#define CT_PLATFORM_H_
#include DEBUG_LOG_HEADER_FILE
#include "osasys.h"
#include "liblwm2m.h"

#define DATA_REPORT_OBJECT                  19

typedef enum
{
    LWM2M_SEND_TASK,
    LWM2M_MAIN_TASK,
    LWM2M_FOTA_DOWNLOAD_TASK,
    LWM2M_INIT_TASK
} lwm2m_task_type;

void * ct_lwm2m_malloc(size_t s);
void ct_lwm2m_free(void * p);
char * ct_lwm2m_strdup(const char * str);

void * ct_lwm2m_connect_server(uint16_t secObjInstID, void * userData);
void ct_lwm2m_close_connection(void * sessionH, void * userData);
//uint8_t ct_lwm2m_buffer_send(void * sessionH, uint8_t * buffer, size_t length, void * userData,uint8_t sendOption);

//wakaama not declair these function
void prv_deleteServerList(lwm2m_context_t * context);
void prv_deleteServerList(lwm2m_context_t * context);
void prv_deleteBootstrapServerList(lwm2m_context_t * context);

bool ct_reboot_timer(void);

typedef osThreadId_t thread_handle_t;
typedef osThreadAttr_t thread_attr_t;
typedef osMutexId_t  thread_mutex_t;
void usleep(uint32_t usec);
int thread_create(void(*start_routine)(void *), lwm2m_task_type type);
int thread_exit(void);

int thread_mutex_init(thread_mutex_t *mutex, const char *name);
int thread_mutex_lock(thread_mutex_t *mutex);
int thread_mutex_unlock(thread_mutex_t *mutex);
int thread_mutex_destory(thread_mutex_t *mutex);
uint32_t ct_conn_radiosignalstrength(void);
uint32_t ct_conn_cellid(void);
void ct_conn_localIP(char* localIP);
void ct_conn_aep(int8_t* txPower, INT8* snr, INT8* rsrp);

#endif //PLATFORM_H_
