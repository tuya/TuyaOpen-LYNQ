/*******************************************************************************
 * Copyright (c) 2014, 2015 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *******************************************************************************/

#if !defined(MQTTLiteOS_H)
#define MQTTLiteOS_H

#include "cmsis_os2.h"


#ifdef FEATURE_MQTT_TLS_ENABLE
#include "MQTTTls.h"
#endif

#include "sockets.h"
#include "api.h"
#include "netdb.h"
#define LiteOS_setsockopt  			    setsockopt
#define LiteOS_getsockopt				getsockopt
#define LiteOS_recv 		 			recv
#define LiteOS_closesocket 			    close
#define LiteOS_shutdownsocket   		shutdown
#define LiteOS_socket					socket
#define LiteOS_connect				    connect
#define	LiteOS_gethostbyname 			getaddrinfo
#define LiteOS_htons					htons
#define LiteOS_send					    send

#define LITEOS_SO_RCVTIMEO 			    SO_RCVTIMEO
#define LITEOS_SO_SNDTIMEO			    SO_SNDTIMEO
#define LITEOS_SO_ERROR				    SO_ERROR
#define LITEOS_AF_INET			    	AF_INET
#define LITEOS_SOCK_STREAM			    SOCK_STREAM
#define LITEOS_IPPROTO_TCP		    	IPPROTO_TCP
#define LITEOS_SOL_SOCKET				SOL_SOCKET

#define liteos_sockaddr 				sockaddr_in



typedef int xSocket_t;

///MQTT client results
typedef enum {
    MQTT_CONN_OK = 0,        ///<Success
    MQTT_PROCESSING,    ///<Processing
    MQTT_PARSE,         ///<url Parse error
    MQTT_DNS,           ///<Could not resolve name
    MQTT_PRTCL,         ///<Protocol error
    MQTT_NOTFOUND,      ///<HTTP 404 Error
    MQTT_REFUSED,       ///<HTTP 403 Error
    MQTT_ERROR,         ///<HTTP xxx error
    MQTT_TIMEOUT,       ///<Connection timeout
    MQTT_CONN,          ///<Connection error
    MQTT_FATAL_ERROR, //fatal error when conenct
    MQTT_CLOSED,        ///<Connection was closed by remote host
    MQTT_MOREDATA,      ///<Need get more data
    MQTT_OVERFLOW,      ///<Buffer overflow
    MQTT_MBEDTLS_ERR,  
    MQTT_SOCKET_FAIL,   ///4<create socket fail
    MQTT_BIND_FAIL,     ///5<bind fail  

}MQTTResult;
typedef struct Timer 
{
    TickType_t xTicksToWait;
    unsigned int xTimeOut;
} Timer;

typedef struct Network Network;

struct Network
{
    xSocket_t my_socket;
    #ifdef	MQTT_RAI_OPTIMIZE
    int (*mqttread) (Network*, unsigned char*, int, int);
    int (*mqttwrite) (Network*, unsigned char*, int, int, int, bool);
    int (*disconnect) (Network*);
    #else
    int (*mqttread) (Network*, unsigned char*, int, int);
    int (*mqttwrite) (Network*, unsigned char*, int, int);
    int (*disconnect) (Network*);
    #endif
};

void TimerInit(Timer*);
char TimerIsExpired(Timer*);
void TimerCountdownMS(Timer*, unsigned int);
void TimerCountdown(Timer*, unsigned int);
int TimerLeftMS(Timer*);

typedef struct Mutex
{
    osMutexId_t sem;
} Mutex;

void MutexInit(Mutex*);
int MutexLock(Mutex*);
int MutexUnlock(Mutex*);

typedef struct Thread
{
    osThreadId_t task;
} Thread;

int ThreadStart(Thread*, void (*fn)(void*), void* arg);

int LiteOS_read(Network*, unsigned char*, int, int);
int LiteOS_write(Network*, unsigned char*, int, int);
int LiteOS_disconnect(Network*);

void NetworkInit(Network*);
int NetworkConnect(Network*, char*, int);
int NetworkSetConnTimeout(Network* n, int send_timeout, int recv_timeout);
/*int NetworkConnectTLS(Network*, char*, int, SlSockSecureFiles_t*, unsigned char, unsigned int, char);*/
int NetworkConnectTimeout(Network* n, char* addr, int port, int send_timeout, int recv_timeout);

#endif
