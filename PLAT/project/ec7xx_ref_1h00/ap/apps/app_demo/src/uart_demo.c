/****************************************************************************
 *
 ****************************************************************************/
#include <stdio.h>
#include "string.h"
#include "osasys.h"
#include "cmsis_os2.h"
#include "uart.h"
#include "queue.h"
#include "bsp_custom.h"
#include "ol_uart_api.h"
#include "ol_log.h"


//---------------------------------------------------------------
// test usb serial port
//---------------------------------------------------------------
typedef struct
{
    uint32_t lenght;
    uint8_t *data;
}test_msg_t;

static QueueHandle_t test_msgq_handle = NULL;

void usb_serial_callback(uint8_t *data, uint32_t len)
{
    test_msg_t msg = {0};

    if(NULL != data && len > 0)
    {
        msg.data = (uint8_t *)malloc(len);
        memset(msg.data, 0x0, len);
        msg.lenght = len;
        memcpy(msg.data, data, len);
        osMessageQueuePut(test_msgq_handle, &msg, 0, 1000);
    }
}

void test_usb_serial(void)
{
    int ret = -1;
    osStatus_t status = osError;
    test_msg_t msg = {0};

    test_msgq_handle = osMessageQueueNew(10, sizeof(test_msg_t), NULL);
    if(NULL == test_msgq_handle)
    {
        OL_LOG_PRINTF("new msg queue fail");
        return;
    }

    ret = ol_uart_init(OL_USB_SERL, NULL);
    if(OL_UART_OK == ret)
    {
        ol_uart_register_input_cb(OL_USB_SERL, usb_serial_callback);
        ol_uart_send(OL_USB_SERL, (uint8_t *)"test_usb_serial\r\n", strlen("test_usb_serial\r\n"));

        while(1)
        {
            status = osMessageQueueGet(test_msgq_handle, &msg, 0, osWaitForever);
            if(osOK == status)
            {
                if(0 == strncmp("quit", (const char *)msg.data, 4))
                {
                    break;
                }
                else
                {
                    ol_uart_send(OL_USB_SERL, (uint8_t *)"\r\necho>>", strlen("\r\necho>>"));
                    ol_uart_send(OL_USB_SERL, msg.data, msg.lenght);
                }
            }
            free(msg.data);
        }
    }

    ol_uart_uninit(OL_USB_SERL);
    osMessageQueueDelete(test_msgq_handle);
}


//---------------------------------------------------------------
// test UART async receive
//---------------------------------------------------------------
volatile bool test_event_timeout = FALSE;
volatile bool test_event_complete = FALSE;
volatile bool test_send_complete = FALSE;
static osEventFlagsId_t test_evtflag_handle = NULL;

void test_uart_callback(uint32_t event)
{
    OL_LOG_PRINTF("test uart cb event %d", event);

    if(event & ARM_USART_EVENT_RX_TIMEOUT)
    {
        test_event_timeout = TRUE;
        osEventFlagsSet(test_evtflag_handle, 0x00000001);
    }
    if(event & ARM_USART_EVENT_RECEIVE_COMPLETE)
    {
        test_event_complete = TRUE;
        osEventFlagsSet(test_evtflag_handle, 0x00000001);
    }
    if(event & ARM_USART_EVENT_SEND_COMPLETE)
    {
        test_send_complete = TRUE;
    }
}

int test_uart_check_mode(int port)
{
    if(OL_UART_1 == port)
    {
        return RTE_UART1_TX_IO_MODE;
    }
    else if(OL_UART_2 == port)
    {
        return RTE_UART2_TX_IO_MODE;
    }

    return RTE_UART0_TX_IO_MODE;
}

void test_uart_async(int port)
{
    int ret = -1;
    ol_uart_config_t config = {0x0};
    uint8_t test_buffer[1024] = {"uart async echo test\r\n"};

    config.baudRate = 115200;
    config.cb_event = test_uart_callback;
    config.control = (ARM_USART_MODE_ASYNCHRONOUS |
                      ARM_USART_DATA_BITS_8 |
                      ARM_USART_PARITY_NONE |
                      ARM_USART_STOP_BITS_1 |
                      ARM_USART_FLOW_CONTROL_NONE);

    test_evtflag_handle = osEventFlagsNew(NULL);
    if(NULL == test_evtflag_handle)
    {
        OL_LOG_PRINTF("create event flag handle fail, return");
        return;
    }

    ret = ol_uart_init(port, &config);
    if(OL_UART_OK != ret)
    {
        OL_LOG_PRINTF("init open UART%d fail = %d, return", port, ret);
        return;
    }

    OL_LOG_PRINTF("init open UART%d OK", port);
    ol_uart_send(port, test_buffer, strlen((char *)test_buffer));

    while(1)
    {
        int lenght = 0;
        memset(test_buffer, 0, sizeof(test_buffer));
        ret = ol_uart_recv_async(port, test_buffer, sizeof(test_buffer));
        OL_LOG_PRINTF("uart async receive ret = %d", ret);

        osEventFlagsWait(test_evtflag_handle, 0x00000001, osFlagsWaitAny, osWaitForever);

        lenght = ol_uart_rx_count(port);
        OL_LOG_PRINTF("uart async receive data lenght = %d", lenght);

        ol_uart_send(port, (uint8_t *)"\r\necho>>", strlen("\r\necho>>"));

        if(DMA_MODE == test_uart_check_mode(port))
        {
            ol_uart_send_async(port, test_buffer, lenght);
            while(FALSE == test_send_complete)
            {
                osDelay(5);
            }
        }
        else
        {
            ol_uart_send(port, test_buffer, lenght);
        }

        test_event_timeout = FALSE;
        test_event_complete = FALSE;
        test_send_complete = FALSE;

        if(0 == strncmp("quit", (const char *)test_buffer, 4))
        {
            break;
        }
    }

    ol_uart_uninit(port);
    osEventFlagsDelete(test_evtflag_handle);
}


//---------------------------------------------------------------
// test UART async receive
//---------------------------------------------------------------
void test_uart_sync(int port)
{
    int ret = -1;
    ol_uart_config_t config = {0x0};
    uint8_t test_buffer[1024] = {"uart sync echo test\r\n"};

    config.baudRate = 115200;
    config.control = (ARM_USART_MODE_ASYNCHRONOUS |
                      ARM_USART_DATA_BITS_8 |
                      ARM_USART_PARITY_NONE |
                      ARM_USART_STOP_BITS_1 |
                      ARM_USART_FLOW_CONTROL_NONE);

    ret = ol_uart_init(port, &config);
    if(OL_UART_OK != ret)
    {
        OL_LOG_PRINTF("init open UART%d fail = %d, return", port, ret);
        return;
    }

    OL_LOG_PRINTF("init open UART%d OK", port);
    ol_uart_send(port, test_buffer, strlen((char *)test_buffer));

    while (1)
    {
        int lenght = 0;
        memset(test_buffer, 0, sizeof(test_buffer));
        lenght = ol_uart_recv(port, test_buffer, sizeof(test_buffer), 0);
        OL_LOG_PRINTF("uart sync receive data lenght = %d", lenght);

        ol_uart_send(port, (uint8_t *)"\r\necho>>", strlen("\r\necho>>"));
        ol_uart_send(port, test_buffer, lenght);

        if(0 == strncmp("quit", (const char *)test_buffer, 4))
        {
            break;
        }
    }

    ol_uart_uninit(port);
}

void uart_demo(void)
{
    test_uart_sync(OL_UART_1);
    test_uart_async(OL_UART_1);
    //test_usb_serial();
}

