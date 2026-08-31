/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of EigenComm Ltd.
 * File name:    console.c
 * Description:  EC718 
 * History:      Rev1.0   2023-03-03
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_CONSOLE_ENABLE
#include <stdint.h>
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include "cmsis_os2.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "bsp_custom.h"
#include "osasys.h"
#include "ostask.h"
#include "ps_lib_api.h"
#include "cmisim.h"
#include "cmips.h"
#include "networkmgr.h"
#include "slpman.h"
#include "time.h"
#include "storage.h"
#ifdef FEATURE_SUBSYS_STATUS_ENABLE
#include "status.h"
#endif
#include "console_hal.h"
#include "bsp.h"
#include "console_ex.h"
#include "console.h"
#include "console_file.h"

#ifdef FEATURE_SUBSYS_FINSH_ENABLE
#include "rtthread.h"
#include "shell.h"
#endif
#ifdef FEATURE_SUBSYS_PIKAPYTHON_ENABLE
#include "pikaScript.h"
#endif
#ifdef FEATURE_SUBSYS_UARTSERVICE_ENABLE
#include "uartservice.h"
#endif
#ifdef USB_CONSOLE
#include <stdarg.h>
#include "ccio_opaq.h"
extern int gConsoleShadowMemReadLen;
static char gUsbBuffer[USB_BUFFER_SIZE] = {0};
#endif
#include DEBUG_LOG_HEADER_FILE

static ARM_DRIVER_USART *gConsoleUart = NULL;
EventGroupHandle_t gConsoleHalEvents = NULL;

static TaskHandle_t gConsoleHalHdl = NULL;
 
static char gCharRecv[MSH_CMD_LENMAX] = {0};
static bool gSwitched = false;


#ifdef USB_CONSOLE
void usbHalCallback(uint32_t event)
{
    if ((event & USB_RX_DONE) != 0)
    {
        xEventGroupSetBits(gConsoleHalEvents, USB_RX_DONE);
    }
}

int32_t usbSend(uint8_t *data, uint32_t length)
{
    int32_t       retVal = -1;
    DlPduBlock_t *dlpdu  = NULL;

    if ((data != NULL) && (length > 0))
    {
#if (RTE_OPAQ_EN == 1)
        dlpdu  = (DlPduBlock_t *)OsaDlfcAllocDlPduNonBlocking(length);
        if(dlpdu != NULL)
        {
            memcpy(dlpdu->pPdu, data, length);
            dlpdu->length = length;
            retVal = opaqDataOutputEx(OPAQ_CHAN_CUST1, dlpdu, NULL);
        }
#endif
    }

    return retVal;
}
#endif

void eConsoleSendData(uint8_t *data,int len)
{
    ECPLAT_PRINTF(UNILOG_PLA_APP, eConsoleSendData, P_DEBUG, "len=%d, data=%s", len, data);
    xEventGroupClearBits(gConsoleHalEvents, UART_TX_DONE); 
    gConsoleUart->Send(data, len);
#ifdef USB_CONSOLE
    usbSend(data, len);
#endif
    xEventGroupWaitBits(
        gConsoleHalEvents,
        UART_TX_DONE,
        pdFALSE,
        pdFALSE,
        1000/portTICK_PERIOD_MS
    );
}

char eConsoleGetChar(void) 
{
    xEventGroupWaitBits(
        gConsoleHalEvents,
        UART_RX_READY | UART_RX_TIMEOUT,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY
    );
    return gCharRecv[0];
}
char pikaPlatformGetchar(void) {
    xEventGroupWaitBits(
        gConsoleHalEvents,
        UART_RX_READY | UART_RX_TIMEOUT,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY
    );
    return gCharRecv[0];
}

void ecConsoleGetStr(uint8_t* str,int len)
{
    xEventGroupClearBits(gConsoleHalEvents, UART_RX_READY | UART_RX_TIMEOUT);
    gConsoleUart->Receive(str, len);
    EventBits_t uxBits = xEventGroupWaitBits(
        gConsoleHalEvents,
        UART_RX_READY | UART_RX_TIMEOUT , 
        pdFALSE,
        pdFALSE,
        portMAX_DELAY
    );
    uxBits = uxBits;
}

void ecConsolePutStr(const char* str)
{
    static osMutexId_t  mutex  = NULL;
    uint32_t            length = 0;
    char               *buffer = NULL;

    if (mutex == NULL)
    {
        mutex = osMutexNew(NULL);
        if(mutex == NULL)
        {
            ECPLAT_PRINTF(UNILOG_PLA_APP, ecConsolePutStr5, P_DEBUG, "Failed to create mutex.");
            return;
        }
    }

    if ((str == NULL) || (strlen(str) == 0))
    {
        ECPLAT_PRINTF(UNILOG_PLA_APP, ecConsolePutStr, P_DEBUG, "str error");
        return;
    }
    ECPLAT_PRINTF(UNILOG_PLA_APP, ecConsolePutStr2, P_DEBUG, "str=%s", str);

    length = strlen(str);
    buffer = malloc(length + 1);
    if (buffer == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLA_APP, ecConsolePutStr3, P_DEBUG, "Failed to malloc %d bytes.", length + 1);
        return;
    }
    memset(buffer, 0,   length + 1);
    memcpy(buffer, str, length);

    osMutexAcquire(mutex, osWaitForever);

    xEventGroupClearBits(gConsoleHalEvents, UART_TX_DONE); 
    gConsoleUart->Send(buffer, length);
#ifdef USB_CONSOLE
    usbSend((uint8_t *)buffer, length);
#endif
    xEventGroupWaitBits(
        gConsoleHalEvents,
        UART_TX_DONE,
        pdFALSE,
        pdFALSE,
        1000/portTICK_PERIOD_MS
    );

    if (buffer != NULL)
    {
        free(buffer);
        buffer = NULL;
    }

    osMutexRelease(mutex);
}
int pikaPlatformPutChar(char ch)
{
    xEventGroupClearBits(gConsoleHalEvents, UART_TX_DONE); 
    gConsoleUart->Send(&ch, 1);
    xEventGroupWaitBits(
        gConsoleHalEvents,
        UART_TX_DONE,
        pdFALSE,
        pdFALSE,
        100/portTICK_PERIOD_MS
    );
    return 0;
}

void uartHalCallback(uint32_t event)
{
    BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
    if(event & ARM_USART_EVENT_RX_TIMEOUT){
        xEventGroupSetBitsFromISR(gConsoleHalEvents, UART_RX_TIMEOUT, &pxHigherPriorityTaskWoken);
    }
    if(event & ARM_USART_EVENT_RECEIVE_COMPLETE){
        xEventGroupSetBitsFromISR(gConsoleHalEvents, UART_RX_READY, &pxHigherPriorityTaskWoken);
    }
    if(event & ARM_USART_EVENT_SEND_COMPLETE){
        xEventGroupSetBitsFromISR(gConsoleHalEvents, UART_TX_DONE, &pxHigherPriorityTaskWoken);
    }
    if(event & (ARM_USART_EVENT_RX_BREAK | ARM_USART_EVENT_RX_FRAMING_ERROR | ARM_USART_EVENT_RX_PARITY_ERROR)){
        gConsoleUart->Control(ARM_USART_CONTROL_PURGE_COMM, 0);
        xEventGroupSetBitsFromISR(gConsoleHalEvents, UART_RX_ERROR, &pxHigherPriorityTaskWoken);
    }
}

extern void writeShadowMem(uint8_t *buf);
static void consoleHalTask(void *arg)
{
    char cmdBuffer[CMD_RX_SIZE_MAX];
    char cmd[CMD_RX_SIZE_MAX];
    char *buffer = NULL;
    int length = 0;
    uint32_t bitsToWaitFor = 0;

    consoleExInit(); 
    while(1)
    {
        memset(cmdBuffer, 0, sizeof(cmdBuffer));
        memset(cmd, 0, sizeof(cmd));
        // ecConsoleGetStr(gCharRecv, MSH_CMD_LENMAX);
        bitsToWaitFor = UART_RX_READY | UART_RX_TIMEOUT | UART_RX_ERROR;
#ifdef USB_CONSOLE
        bitsToWaitFor |= USB_RX_DONE;
        memset(gUsbBuffer, 0, sizeof(gUsbBuffer));
        usbReceive((uint8_t *)gUsbBuffer, USB_BUFFER_SIZE);
#endif
        xEventGroupClearBits(gConsoleHalEvents, bitsToWaitFor);
        gConsoleUart->Receive(cmdBuffer, CMD_RX_SIZE_MAX);
        EventBits_t uxBits = xEventGroupWaitBits(
            gConsoleHalEvents,
            bitsToWaitFor,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY
        );
        if (gSwitched == true)
        {
            gSwitched = false;
            continue;
        }

#ifdef USB_CONSOLE
        if (uxBits & USB_RX_DONE)
        {
            length = strlen(gUsbBuffer);
            ECPLAT_PRINTF(UNILOG_PLA_APP, consoleHalTask, P_DEBUG, "gConsoleShadowMemReadLen=%d, length=%d, gUsbBuffer=%s", gConsoleShadowMemReadLen, length, gUsbBuffer);
            ECPLAT_PRINTF(UNILOG_PLA_APP, consoleHalTask5, P_DEBUG, "%x, %x, %x, %x, %x, %x, %x, %x", gUsbBuffer[0], gUsbBuffer[1], gUsbBuffer[2], gUsbBuffer[3], gUsbBuffer[4], gUsbBuffer[5], gUsbBuffer[6], gUsbBuffer[7]);
            if (gConsoleShadowMemReadLen > 0)
            {
                writeShadowMem((uint8_t *)gUsbBuffer);
            }
            else if (length <= CMD_RX_SIZE_MAX)
            {
                for (uint32_t i=0; i<length; i++)
                {
                    if(consoleChar2Cmd(gUsbBuffer[i], cmd))
                    {
                        sendConsoleCmd(cmd, 0);
                    }
                }
            }
        }
#endif

        if (uxBits & UART_RX_READY) {
            buffer = malloc(FILE_RX_SIZE_MAX);
            if (buffer == NULL)
            {
                rt_kprintf("Failed to malloc %d for file recv.\r\n",FILE_RX_SIZE_MAX);
            }
            memset(buffer, 0, FILE_RX_SIZE_MAX);
            memmove(buffer, cmdBuffer, CMD_RX_SIZE_MAX);
            memset(cmdBuffer, 0, CMD_RX_SIZE_MAX);
            xEventGroupClearBits(gConsoleHalEvents, UART_RX_READY | UART_RX_TIMEOUT);
            gConsoleUart->Receive(buffer+CMD_RX_SIZE_MAX, FILE_RX_SIZE_MAX-CMD_RX_SIZE_MAX);
            uxBits = xEventGroupWaitBits(
                gConsoleHalEvents,
                UART_RX_READY | UART_RX_TIMEOUT , 
                pdFALSE,
                pdFALSE,
                10000/portTICK_PERIOD_MS
            );

            extern int gConsoleShadowMemReadLen;
            if(gConsoleShadowMemReadLen > 0)
                writeShadowMem((uint8_t *)buffer);

            // sendConsoleFile(buffer,0);  //for org file transfer
            free(buffer);
        }
        #ifdef FEATURE_SUBSYS_FINSH_ENABLE
		else if(uxBits & UART_RX_TIMEOUT) {
            extern int gConsoleShadowMemReadLen;
            if(gConsoleShadowMemReadLen > 0)
                writeShadowMem((uint8_t *)cmdBuffer);

            for (int i = 0; i < strlen(cmdBuffer);i++){
                // finsh_cmd_char(cmdBuffer[i]);
                if(consoleChar2Cmd(cmdBuffer[i],cmd)){    
                    sendConsoleCmd(cmd,0);     
                }
            }
        }
        #endif
    }
    vTaskDelete(NULL);
}

 
int consoleHalInit(void)
{
#ifdef FEATURE_SUBSYS_UARTSERVICE_ENABLE
    gConsoleUart = (ARM_DRIVER_USART *)uartserviceHandleGet();
#endif
    if(gConsoleUart != NULL){
        if (gConsoleHalEvents == NULL)
        {
            gConsoleHalEvents = xEventGroupCreate();
            if (gConsoleHalEvents == NULL)
            {
                printf("Failed to create event for gConsoleHalEvents.\r\n");
                return 0;
            }
        }
        else
        {
            gSwitched = true;
        }
        if(gConsoleHalHdl == NULL){
            xTaskCreate(consoleHalTask, "consoleHalTask", TASK_SIZE_CONSOLE_HAL, NULL, TASK_PRIO_CONSOLE_HAL, &gConsoleHalHdl);
            if (gConsoleHalHdl == NULL)
            {
                printf("Failed to create thread for gConsoleHalHdl.\r\n");
                return 0;
            }
        }
    }
    return 1;
}

int consoleHalDeinit(void)
{
    if(gConsoleHalHdl != NULL){
        consoleExDeinit();
        vTaskDelete(gConsoleHalHdl);
    }
    return 0;
}

#endif