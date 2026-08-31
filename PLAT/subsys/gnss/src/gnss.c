/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    gnss.c
 * Description:  GNSS module
 * History:      Rev1.0
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_GNSS_ENABLE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "cmsis_os2.h"
#include "bsp.h"
#include "gnss.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#include "api_uart.h"
#include "servicemanager.h"

#define QUEUE_SIZE_RECV 100

uint8_t subsys_gnss_task_stack[4096];

uint8_t subsys_gnss_queue_task_stack[4096];

osThreadId_t gnssTaskId;
static osMessageQueueId_t gGnssQueue = NULL;
static osMessageQueueId_t gUartRecvQueue = NULL;
static osMutexId_t gGnssMutex = NULL;
static uint8_t uartId = 2;
static uint32_t usrId = 0;
static bool gOpenStatus = false;
char gLatitude[15];
char gLongitude[15];
char gAltitude[15];
gnss_data_t gnssData;

typedef struct
{
    uint32_t event;
} GnssUartRecvT;

static void sGnssInit(void)
{
    gGnssQueue = osMessageQueueNew(QUEUE_SIZE_GNSS, sizeof(QueueGnssT), NULL);
    if (gGnssQueue == NULL)
    {
        SYSLOG_ERR("Failed to create queue for gGnssQueue.\r\n");
        return;
    }

    gGnssMutex = osMutexNew(NULL);
    if (gGnssMutex == NULL)
    {
        SYSLOG_ERR("Failed to create mutex for gGnssMutex.\r\n");
        return;
    }

    gUartRecvQueue = osMessageQueueNew(QUEUE_SIZE_RECV, sizeof(GnssUartRecvT), NULL);
    if (gUartRecvQueue == NULL)
    {
        SYSLOG_EMERG("Failed to create queue for gUartRecvQueue.\r\n");
        return;
    }
}

static void test_uart_cb(uint32_t event)
{
    GnssUartRecvT uartRecv = {.event = event};

    if (event & (ARM_USART_EVENT_RX_TIMEOUT | ARM_USART_EVENT_RECEIVE_COMPLETE))
    {
        osMessageQueuePut(gUartRecvQueue, (void *)&uartRecv, 0, 0);
    }
}

void sGnssStart(void)
{
    uartId = 2;
    uart_config_t Config = {0};
    Config.rxd = 27;
    Config.rxd = 28;
    Config.baud = 12;
    Config.mode = 1U;
    Config.bits = 8U;
    Config.parity = 0;
    Config.stop = 1;
    Config.flow = 0;
    api_uart_create(uartId, &Config, &usrId);
    api_uart_open(usrId, &Config, 100);
    api_uart_ioctl(usrId, OPEN_UART_IOCTL_ISR_CB, test_uart_cb);

    unsigned int baud = 115200;
    api_uart_ioctl(usrId, OPEN_UART_IOCTL_BAUDRATE, &baud);

    char *str1 = "$PDTINFO\r\n";
    api_uart_write(usrId, str1, strlen(str1));
    char *strBuffer = malloc(GNSS_RX_SIZE_MAX + 1);
    if (strBuffer == NULL)
    {
        printf("Failed to malloc strBuffer.\r\n");
    }

    GnssUartRecvT uartRecv = {0};

    osMessageQueuePut(gUartRecvQueue, (void *)&uartRecv, 0, osWaitForever);
    while (1)
    {
        memset(&uartRecv, 0, sizeof(uartRecv));
        if (osMessageQueueGet(gUartRecvQueue, &uartRecv, 0, osWaitForever) == osOK)
        {
            if (uartRecv.event & (ARM_USART_EVENT_RX_TIMEOUT | ARM_USART_EVENT_RECEIVE_COMPLETE))
            {

                int count = 0;
                api_uart_ioctl(usrId, GET_RX_COUNT, &count);
                strBuffer[count] = '\0';
#if GNSS_DEBUG_PRINTF
                printf("NMEA = \r\n%s\r\n", strBuffer);
#endif
                gnss_parse(strBuffer, &gnssData);
            }
            memset(strBuffer, 0, GNSS_RX_SIZE_MAX + 1);
            api_uart_read(usrId, strBuffer, GNSS_RX_SIZE_MAX + 1);
        }
    }
}

void subGnssTask(void)
{
    QueueGnssT msg = {0};

    sGnssInit();

    while (1)
    {
        memset(&msg, 0, sizeof(msg));
        if (osMessageQueueGet(gGnssQueue, &msg, 0, osWaitForever) == osOK)
        {
            switch (msg.action)
            {
            case POWERCONTROL:
                if (msg.open == 1 && gOpenStatus == false)
                {
                    gOpenStatus = true;

                    osThreadAttr_t taskAttr;

                    memset(&taskAttr, 0, sizeof(taskAttr));
                    memset(subsys_gnss_task_stack, 0xA5, 4096);
                    taskAttr.name = "gnss_start";
                    taskAttr.stack_mem = subsys_gnss_task_stack;
                    taskAttr.stack_size = 4096;
                    taskAttr.priority = osPriorityAboveNormal;
#if 0
                    gnssTaskId = osThreadNew(sGnssStart, NULL, &taskAttr);
#else
                    char serviceName[32] = {0};
                    snprintf(serviceName, sizeof(serviceName), "service:/%s", taskAttr.name);
                    Service_reg(serviceName, sGnssStart, NULL, taskAttr.cb_mem, taskAttr.cb_size, taskAttr.stack_mem, taskAttr.stack_size, taskAttr.priority);
                    gnssTaskId = (osThreadId_t)Service_start(serviceName);
#endif
                }
                break;
            default:
                break;
            }
        }
    }
}

void subGnssInit()
{
    osThreadAttr_t taskAttr;

    memset(&taskAttr, 0, sizeof(taskAttr));
    memset(subsys_gnss_queue_task_stack, 0xA5, 1024);
    taskAttr.name = "gnss_queue";
    taskAttr.stack_mem = subsys_gnss_queue_task_stack;
    taskAttr.stack_size = 1024;
    taskAttr.priority = osPriorityAboveNormal;

#if 0
    osThreadNew(subGnssTask, NULL, &taskAttr);
#else
    char serviceName[32] = {0};
    snprintf(serviceName, sizeof(serviceName), "service:/%s", taskAttr.name);
    Service_reg(serviceName, subGnssTask, NULL, taskAttr.cb_mem, taskAttr.cb_size, taskAttr.stack_mem, taskAttr.stack_size, taskAttr.priority);
    Service_start(serviceName);
#endif
}

void gnssStart(void)
{
    if (gOpenStatus == false)
    {
        QueueGnssT msg = {0};
        msg.open = 1;
        msg.action = POWERCONTROL;
        osMessageQueuePut(gGnssQueue, &msg, 0, osWaitForever);
    }
}

void gnssStop(void)
{
    if (gOpenStatus == true)
    {
        gOpenStatus = false;
        osThreadTerminate(gnssTaskId);
        api_uart_close(usrId);
        api_uart_delete(usrId);
    }
}

bool gnssIsFix(void)
{
    if (gOpenStatus == false)
    {
        return false;
    }
    return gnssData.valid;
}

char *gnssGetLatitude(void)
{
    if (gnssData.valid == false || gOpenStatus == false)
    {
        return "0";
    }
    if (gnssData.latitude_scale != 0)
    {
        sprintf(gLatitude, "%d.%d", (gnssData.latitude / gnssData.latitude_scale), (gnssData.latitude % gnssData.latitude_scale));
        return gLatitude;
    }
    return "0";
}

char *gnssGetLongitude(void)
{
    if (gnssData.valid == false || gOpenStatus == false)
    {
        return "0";
    }
    if (gnssData.longitude_scale != 0)
    {
        sprintf(gLongitude, "%d.%d", (gnssData.longitude / gnssData.longitude_scale), (gnssData.longitude % gnssData.longitude_scale));
        return gLongitude;
    }
    return "0";
}

char *gnssGetAltitude(void)
{
    if (gnssData.valid == false || gOpenStatus == false)
    {
        return "0";
    }

    if (gnssData.altitude_scale != 0)
    {
        sprintf(gAltitude, "%d.%d", (gnssData.altitude / gnssData.altitude_scale), (gnssData.altitude % gnssData.altitude_scale));
        return gAltitude;
    }
    return "0";
}

#endif
