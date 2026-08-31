/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    test.c
 * Description:  EC618 lwm2m demo entry source file
 * History:      Rev1.0   2018-10-12
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "cmsis_os2.h"
#include "bsp.h"
#ifdef FEATURE_SUBSYS_CMDPARSE_ENABLE
#include "cmdparse.h"
#endif
#ifdef FEATURE_SUBSYS_MODE_ENABLE
#include "mode.h"
#endif
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#include "subsys.h"
#include "servicemanager.h"


#define THREAD_STACK_SIZE_RECV          (10 * 1024)
#define THREAD_STACK_SIZE_SEND          (10 * 1024)
#define QUEUE_SIZE_RECV                 100
#define QUEUE_SIZE_SEND                 100
#ifdef FEATURE_SUBSYS_SYSTEST_ENABLE
#define UART_BAUD_RATE                  115200
#else
#define UART_BAUD_RATE                  115200
#endif


typedef struct
{
    uint32_t event;
} UartRecvT;

typedef struct
{
    char *buffer;
} UartSendT;


#ifdef FEATURE_SUBSYS_CONSOLE_ENABLE
extern void uartHalCallback(uint32_t event);
#endif
extern ARM_DRIVER_USART   Driver_LPUSART1;
static ARM_DRIVER_USART   *gUart          = &Driver_LPUSART1;
static osThreadId_t       gUartRecvThread = NULL;
static osThreadId_t       gUartSendThread = NULL;
static osMessageQueueId_t gUartRecvQueue  = NULL;
static osMessageQueueId_t gUartSendQueue  = NULL;
#ifdef THREAD_STATIC
static StaticTask_t       gUartRecvThreadCbMem                            = {0};
static uint8_t            gUartRecvThreadStackMem[THREAD_STACK_SIZE_RECV] = {0};
static StaticTask_t       gUartSendThreadCbMem                            = {0};
static uint8_t            gUartSendThreadStackMem[THREAD_STACK_SIZE_SEND] = {0};
#endif

#ifdef FEATURE_SUBSYS_MICROPYTHON_ENABLE
osMessageQueueId_t gMpyCharQueue  = NULL;
uint8_t mpyCharQueueBuff[100] = {0};
int mpyCharQueueBuffindex = 0;
#endif

static void uartServiceDelete(osMessageQueueId_t *queue, osThreadId_t *thread)
{
    if ((queue != NULL) && (*queue != NULL))
    {
        osMessageQueueDelete(*queue);
        *queue = NULL;
    }

    if ((thread != NULL) && (*thread != NULL))
    {
        if (osThreadGetId() != *thread)
        {
            osThreadTerminate(*thread);
            *thread = NULL;
        }
        else
        {
            *thread = NULL;
            osThreadExit();
        }
    }
}

static void threadUartSend(void *argument)
{
    UartSendT uartSend = {0};

    gUartSendQueue = osMessageQueueNew(QUEUE_SIZE_SEND, sizeof(uartSend), NULL);
    if (gUartSendQueue == NULL)
    {
        SYSLOG_EMERG("Failed to create queue for gUartSendQueue.\r\n");
        goto labelEnd;
    }

    while (1)
    {
        memset(&uartSend, 0, sizeof(uartSend));
        if ((osMessageQueueGet(gUartSendQueue, &uartSend, 0, osWaitForever) == osOK) && (uartSend.buffer != NULL))
        {
            gUart->Send(uartSend.buffer, strlen(uartSend.buffer));
            free(uartSend.buffer);
            uartSend.buffer = NULL;
        }
    }

labelEnd:
    uartServiceDelete(&gUartSendQueue, &gUartSendThread);
}

static void threadUartRecv(void *argument)
{
    UartRecvT uartRecv     = {0};
    uint8_t   buffer[1024] = {0};
#ifdef FEATURE_SUBSYS_MICROPYTHON_ENABLE
    mpyCharQueueBuffindex = 0;
    gMpyCharQueue = osMessageQueueNew(100, 1, NULL);
#endif
    gUartRecvQueue = osMessageQueueNew(QUEUE_SIZE_RECV, sizeof(uartRecv), NULL);
    if (gUartRecvQueue == NULL)
    {
        SYSLOG_EMERG("Failed to create queue for gUartRecvQueue.\r\n");
        goto labelEnd;
    }
    osMessageQueuePut(gUartRecvQueue, (void *)&uartRecv, 0, osWaitForever);

    while (1)
    {
        memset(&uartRecv, 0, sizeof(uartRecv));
        if (osMessageQueueGet(gUartRecvQueue, &uartRecv, 0, osWaitForever) == osOK)
        {
            if (uartRecv.event & (ARM_USART_EVENT_RX_TIMEOUT | ARM_USART_EVENT_RECEIVE_COMPLETE))
            {
#ifdef FEATURE_SUBSYS_CMDPARSE_ENABLE
                cmdparseQueuePut(buffer, gUart->GetRxCount());
#endif
#ifdef FEATURE_SUBSYS_MICROPYTHON_ENABLE
                // if (gUart->GetRxCount() > 0)    
                // {
                //     for(int i = 0; i < gUart->GetRxCount(); i++)
                //     {
                //         mpyCharQueueBuff[mpyCharQueueBuffindex] = buffer[i];
                //         osMessageQueuePut(gMpyCharQueue, (void *)mpyCharQueueBuff, 0, 0);
                //         mpyCharQueueBuffindex++;
                //         if (mpyCharQueueBuffindex >= 100)
                //         {
                //             /* code */
                //             mpyCharQueueBuffindex = 0;
                //         }
                        
                //     }
                // }
#endif
            }

            memset(buffer, 0, sizeof(buffer));
            gUart->Receive(buffer, sizeof(buffer));
        }
    }

labelEnd:
    uartServiceDelete(&gUartRecvQueue, &gUartRecvThread);
}

static void uartEventCallback(uint32_t event)
{
    uint8_t   debug    = DEBUG_CMD;
    UartRecvT uartRecv = {.event = event};

#ifdef FEATURE_SUBSYS_MODE_ENABLE
    debug = debugGet();
#endif

    switch (debug)
    {
        case DEBUG_JSON:
            if (event & (ARM_USART_EVENT_RX_TIMEOUT | ARM_USART_EVENT_RECEIVE_COMPLETE))
            {
                osMessageQueuePut(gUartRecvQueue, (void *)&uartRecv, 0, 0);
            }
            break;

#ifdef FEATURE_SUBSYS_CONSOLE_ENABLE
        case DEBUG_CMD:
            uartHalCallback(event);
            break;
#endif

        default:
            break;
    }
}

void uartServiceInit(void)
{
    osThreadAttr_t threadAttr = {0};

    GPR_clockDisable(FCLK_UART1);
    GPR_setClockSrc(FCLK_UART1, FCLK_UART1_SEL_26M);
    GPR_clockEnable(FCLK_UART1);
    GPR_swReset(RST_FCLK_UART1);
    gUart->Initialize(uartEventCallback);
    gUart->PowerControl((UART_BAUD_RATE > 9600) ? ARM_POWER_FULL : ARM_POWER_LOW);
    gUart->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_DATA_BITS_8 | ARM_USART_PARITY_NONE
                 | ARM_USART_FLOW_CONTROL_NONE | ARM_USART_STOP_BITS_1,  UART_BAUD_RATE);
    UsartPrintHandle = gUart;
    memset(&threadAttr, 0, sizeof(threadAttr));
    threadAttr.name       = "thread_uart_recv";
    threadAttr.stack_size = THREAD_STACK_SIZE_RECV;
    threadAttr.priority   = osPriorityNormal;
#ifdef THREAD_STATIC
    threadAttr.stack_mem  = gUartRecvThreadStackMem;
    threadAttr.cb_mem     = &gUartRecvThreadCbMem;
    threadAttr.cb_size    = sizeof(StaticTask_t);
#endif
    if (gUartRecvThread == NULL)
    {
#if 1 // Service Manager
        gUartRecvThread = osThreadNew(threadUartRecv, NULL, &threadAttr);
#else
        char serviceName[32] = {0};
        snprintf(serviceName, sizeof(serviceName), "service:/%s", threadAttr.name);
        Service_reg(serviceName, threadUartRecv, NULL, threadAttr.cb_mem, threadAttr.cb_size, threadAttr.stack_mem, threadAttr.stack_size, threadAttr.priority);
        gUartRecvThread = (osThreadId_t)Service_start(serviceName);
#endif
    }

    memset(&threadAttr, 0, sizeof(threadAttr));
    threadAttr.name       = "thread_uart_send";
    threadAttr.stack_size = THREAD_STACK_SIZE_SEND;
    threadAttr.priority   = osPriorityNormal;
#ifdef THREAD_STATIC
    threadAttr.stack_mem  = gUartSendThreadStackMem;
    threadAttr.cb_mem     = &gUartSendThreadCbMem;
    threadAttr.cb_size    = sizeof(StaticTask_t);
#endif
    if (gUartSendThread == NULL)
    {
#if 1 // Service Manager
        gUartSendThread = osThreadNew(threadUartSend, NULL, &threadAttr);
#else
        char serviceName2[32] = {0};
        snprintf(serviceName2, sizeof(serviceName2), "service:/%s", threadAttr.name);
        Service_reg(serviceName2, threadUartSend, NULL, threadAttr.cb_mem, threadAttr.cb_size, threadAttr.stack_mem, threadAttr.stack_size, threadAttr.priority);
        gUartSendThread = (osThreadId_t)Service_start(serviceName2);
#endif
    }
}

void uartServiceSend(char *response, uint32_t timeout)
{
    UartSendT uartSend = {0};
    
    if (response != NULL)
    {
        uartSend.buffer = malloc(strlen(response) + 1);
        if (uartSend.buffer != NULL)
        {
            memset(uartSend.buffer, 0,        strlen(response) + 1);
            memcpy(uartSend.buffer, response, strlen(response));
            osMessageQueuePut(gUartSendQueue, &uartSend, 0, timeout);
        }
    }
}

void *uartserviceHandleGet(void)
{
    return (void *)gUart;
}

