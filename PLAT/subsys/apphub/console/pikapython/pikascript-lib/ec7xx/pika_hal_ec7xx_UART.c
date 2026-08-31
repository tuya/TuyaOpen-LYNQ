#include <stdint.h>
#include "BaseObj.h"
#include "ec7xx_UART.h"
#include "ec7xx_common.h"
#include "dataStrs.h"

#include "bsp.h"
#include "bsp_custom.h"
#include "cmsis_os2.h"
#include "osasys.h"
#include "ostask.h"
#include "uart_device.h"
#include "servicemanager.h"
// PikaStdDevice_UART.c

#define UART_APP_STATK_SIZE         (1024*4)
#define UART_RX_SIZE_MAX             (256)
#define UART_EVENTS_WAIT            ARM_USART_EVENT_RX_TIMEOUT|ARM_USART_EVENT_RECEIVE_COMPLETE

typedef struct
{
    uint8_t port;
    uint32_t settings;
    uint32_t baudrate;
    ARM_POWER_STATE powerMode;
    ARM_DRIVER_USART * uart; 
    void * uartMsgQueue; 
    PIKA_BOOL event_started;
    osEventFlagsId_t evtHandle;
} ec7xx_uart_t;



// extern ARM_DRIVER_USART *BSP_UartDriverGet(uint8_t symbol,
//                                 ARM_POWER_STATE powerMode,
//                                 uint32_t settings,
//                                 uint32_t baudrate,
//                                 ARM_USART_SignalEvent_t cb_event);

#if 1
uint32_t s_uart_event = 0;
static void s_uartCallback(uint32_t event)
{
    s_uart_event = event;
    // osEventFlagsSet(uartEvtHandle,event);
    #ifdef FEATURE_SUBSYS_INPUT_ENABLE
    inputNotify();
    #endif
}

void pika_uart_poll(void)
{
    if(s_uart_event)
    {
        printf("uart input %d\r\n",s_uart_event);
        s_uart_event = 0;
    }
}

static void uartTask(void *arg)
{
    ec7xx_uart_t* ec718 = (ec7xx_uart_t*)arg;
    ARM_DRIVER_USART * sUart = ec718->uart;
    osEventFlagsId_t uartEvtHandle = ec718->evtHandle;
    if(sUart==NULL) vTaskDelete(NULL);
    char *rxBuffer = malloc(UART_RX_SIZE_MAX);
    if(rxBuffer==NULL) vTaskDelete(NULL);
    while (1)
    {
        memset(rxBuffer, 0, sizeof(rxBuffer));
        sUart->Receive(rxBuffer, UART_RX_SIZE_MAX);
        uint32_t flag = osEventFlagsWait(uartEvtHandle,UART_EVENTS_WAIT,osFlagsWaitAny,osWaitForever);
        uint32_t rx_cnt = sUart->GetRxCount();
        pika_platform_printf("\r\nRX[%d]:",rx_cnt);
        for (int i = 0; i < rx_cnt;i++) pika_platform_printf("%02X ",rxBuffer[i]);

    }
    free(rxBuffer);
    vTaskDelete(NULL);
}

static void uartTaskInit(void)
{
    osThreadAttr_t uart_attr;
    memset(&uart_attr,0,sizeof(uart_attr));
    uart_attr.name = "uartTask";
    uart_attr.stack_size = UART_APP_STATK_SIZE;
    uart_attr.priority = osPriorityAboveNormal;
#if 0
    osThreadNew(uartTask, NULL, &uart_attr);
#else
    char serviceName[32] = {0};
    snprintf(serviceName, sizeof(serviceName), "service:/%s", uart_attr.name);
    Service_reg(serviceName, uartTask, NULL, uart_attr.cb_mem, uart_attr.cb_size, uart_attr.stack_mem, uart_attr.stack_size, uart_attr.priority);
    Service_start(serviceName);
#endif
}

#endif

int pika_hal_platform_UART_open(pika_dev* dev, char* name) 
{
    uint8_t port = fast_atoi(name + 4);
    if (name[0] != 'U' || name[1] != 'A' || name[2] != 'R' || name[3] != 'T' || port>3) {
        pika_debug("%s Name error",name);
        return 1;
    }
    if(dev->platform_data) pikaFree(dev->platform_data, sizeof(ec7xx_uart_t));
    dev->platform_data = pikaMalloc(sizeof(ec7xx_uart_t));
    if (dev->platform_data == NULL) {
        pika_debug("UART pikaMalloc error");
        return 2;
    }
    ec7xx_uart_t* ec718 = dev->platform_data;
    ec718->port = port;
    ec718->evtHandle = osEventFlagsNew(NULL);
    // uartEvtHandle = osEventFlagsNew(NULL);
    pika_debug("open:UART%d,0x%X,0x%X",ec718->port,ec718->evtHandle,ec718);
    return 0;
}


int pika_hal_platform_UART_close(pika_dev* dev) 
{
    if (NULL != dev->platform_data) {
        pikaFree(dev->platform_data, sizeof(ec7xx_uart_t));
        dev->platform_data = NULL;
    }
    pika_debug("UART_close");
    return 0;
}

int pika_hal_platform_UART_ioctl_config(pika_dev* dev,pika_hal_UART_config* cfg) 
{
    ec7xx_uart_t* ec718 = dev->platform_data;
    if (NULL == ec718) {
        pika_debug("UART invalid");
        return 1;
    }
    if(cfg == NULL) {
        pika_debug("UART config invalid 0x%X",cfg);
        return 2;
    }
    if(dev->ioctl_config  != NULL) {
        dev->ioctl_config = cfg;
    }
    ec718->baudrate = cfg->baudrate;
    ec718->settings = ARM_USART_MODE_ASYNCHRONOUS;
    if(ec718->baudrate>9600) ec718->powerMode = ARM_POWER_FULL;
    else ec718->powerMode = ARM_POWER_LOW;
    switch (cfg->data_bits) {
        case PIKA_HAL_UART_DATA_BITS_5:
            ec718->settings |= ARM_USART_DATA_BITS_5;
            break;
        case PIKA_HAL_UART_DATA_BITS_6:
            ec718->settings |= ARM_USART_DATA_BITS_6;
            break;
        case PIKA_HAL_UART_DATA_BITS_7:
            ec718->settings |= ARM_USART_DATA_BITS_7;
            break;
        case PIKA_HAL_UART_DATA_BITS_8:
            ec718->settings |= ARM_USART_DATA_BITS_8;
            break;
        default:
            ec718->settings |= ARM_USART_DATA_BITS_8;
            break;
    }
    switch (cfg->parity) {
        case PIKA_HAL_UART_PARITY_NONE:
            ec718->settings |= ARM_USART_PARITY_NONE;
            break;
        case PIKA_HAL_UART_PARITY_ODD:
            ec718->settings |= ARM_USART_PARITY_ODD;
            break;
        case PIKA_HAL_UART_PARITY_EVEN:
            ec718->settings |= ARM_USART_PARITY_EVEN;
            break;
        default:
            ec718->settings |= ARM_USART_PARITY_NONE;
            break;
    }
    switch (cfg->stop_bits) {
        case PIKA_HAL_UART_STOP_BITS_1:
            ec718->settings |= ARM_USART_STOP_BITS_1;
            break;
        case PIKA_HAL_UART_STOP_BITS_1_5:
            ec718->settings |= ARM_USART_STOP_BITS_1_5;
            break;
        case PIKA_HAL_UART_STOP_BITS_2:
            ec718->settings |= ARM_USART_STOP_BITS_2;
            break;
        default:
            ec718->settings |= ARM_USART_STOP_BITS_1;
            break;
    }
    switch (cfg->flow_control) {
        case PIKA_HAL_UART_FLOW_CONTROL_NONE:
            ec718->settings |= ARM_USART_FLOW_CONTROL_NONE;
            break;
        case PIKA_HAL_UART_FLOW_CONTROL_RTS:
            ec718->settings |= ARM_USART_FLOW_CONTROL_RTS;
            break;
        case PIKA_HAL_UART_FLOW_CONTROL_CTS:
            ec718->settings |= ARM_USART_FLOW_CONTROL_CTS;
            break;
        case PIKA_HAL_UART_FLOW_CONTROL_RTS_CTS:
            ec718->settings |= ARM_USART_FLOW_CONTROL_RTS_CTS;
            break;
        default:
            ec718->settings |= ARM_USART_FLOW_CONTROL_NONE;
            break;
    }
    /* support event callback */
    // if (dev->is_enabled == PIKA_TRUE && NULL != cfg->event_callback &&
    //     PIKA_HAL_EVENT_CALLBACK_ENA_ENABLE == cfg->event_callback_ena) 
    // {
    //     switch (cfg->event_callback_filter) {
    //         /* Configure UART to interrupt mode */
    //         case PIKA_HAL_UART_EVENT_SIGNAL_RX:
    //             pika_debug("Setting UART_RX callback");
    //             // uart_enable_rx_intr(uart->uartPort);
    //             break;
    //         case PIKA_HAL_UART_EVENT_SIGNAL_TX:
    //             pika_debug("Setting UART_TX callback");
    //             // uart_enable_tx_intr(uart->uartPort, 1, 0);
    //             break;
    //         default:
    //             __platform_printf(
    //                 "Error: not supported event callback filter %d\r\n",
    //                 cfg->event_callback_filter);
    //             return -1;
    //     }
    //     if (ec718->event_started == PIKA_FALSE){
    //         pika_debug("Starting uart event task:%p", dev);
    //         ec718->event_started = PIKA_TRUE;
    //     }
    // }
    // ec718->uart = BSP_UartDriverGet(ec718->port,ec718->powerMode,ec718->settings,ec718->baudrate,s_uartCallback);
    // ARM_DRIVER_USART * sUart = ec718->uart;
    // sUart->SendPolling("test", 4);
    pika_debug("config:UART%d,0x%X,0x%X,b%d", ec718->port,ec718->uart,ec718->settings,ec718->baudrate);
    return 0;
}

int pika_hal_platform_UART_ioctl_enable(pika_dev* dev) 
{
    ec7xx_uart_t* ec718 = dev->platform_data;
    ARM_DRIVER_USART * sUart = ec718->uart;
    if (NULL == sUart) {
        pika_debug("UART invalid 0x%X",sUart);
        return 1;
    }
    // uartTaskInit();  //加task AP hardFault
    pika_debug("enable:UART%d,0x%X,0x%X,b%d",ec718->port,sUart,ec718->settings,ec718->baudrate);
    return 0;
}

int pika_hal_platform_UART_ioctl_disable(pika_dev* dev) 
{
    ec7xx_uart_t* ec718 = dev->platform_data;
    ARM_DRIVER_USART * sUart = ec718->uart;
    if (NULL == sUart) {
        pika_debug("UART disable NULL");
        return 1;
    }
    pika_debug("disable:UART%d,0x%X,0x%X,b%d",ec718->port,sUart,ec718->settings,ec718->baudrate);
    return 0;
}

int pika_hal_platform_UART_read(pika_dev* dev, void* buf, size_t count) 
{
    ec7xx_uart_t* ec718 = dev->platform_data;
    ARM_DRIVER_USART * sUart = ec718->uart;
    if (NULL == sUart) {
        pika_debug("UART read NULL");
        return 1;
    }
    pika_debug("read:UART%d,0x%X %d",ec718->port,sUart,count);
    sUart->Receive(buf, count);
    // sUart->Receive(buf, 100);
    // uart_read_bytes(ec718->uartPort, buf, count, 100);
    // return sUart->Receive(buf, count);
}

int pika_hal_platform_UART_write(pika_dev* dev, void* buf, size_t count) 
{
    ec7xx_uart_t* ec718 = dev->platform_data;
    ARM_DRIVER_USART * sUart = ec718->uart;
    if (NULL == sUart) {
        pika_debug("UART write NULL");
        return 1;
    }
    sUart->SendPolling((uint8_t *)buf, count);
    // sUart->Send(buf, count);
    // pika_platform_printf("write:UART 0x%X,%d:",sUart,count);
    // for (int i = 0; i < count;i++)
    //     pika_platform_printf("0x%x ",*((uint8_t *)buf+i));
    // pika_platform_printf("\r\n");
    return 0;
}


