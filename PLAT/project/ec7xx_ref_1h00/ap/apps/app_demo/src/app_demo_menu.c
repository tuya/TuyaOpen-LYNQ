/****************************************************************************
 *
 ****************************************************************************/
#include "string.h"
#include "bsp.h"
#include "bsp_custom.h"
#include "os_common.h"
#include "osasys.h"
#include "ostask.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "plat_config.h"
#include "uart.h"
#include "slpman.h"
#include "app.h"
#include "ol_log.h"
#include "ol_uart_api.h"


typedef struct
{
    char demo_str[32];
    void (*func)(void);
} demo_menu_t;

typedef struct
{
    uint32_t lenght;
    uint8_t *data;
}test_msg_t;

static QueueHandle_t test_msg_q = NULL;

//---------------------------------------------------------
//                   Test Demo Menu
//---------------------------------------------------------
static demo_menu_t test_demo_menu[] =
{
    {"--- Demo Menu ---",      NULL},
    {"test OS demo",           os_demo},
    {"test FS demo",           fs_demo},
    {"test Time demo",         time_demo},
    {"test Sleep demo",        sleep_demo},
    {"test UART demo",         uart_demo},
    {"test GPIO demo",         gpio_demo},
    {"test ADC demo",          adc_demo},
    {"test I2C demo",          i2c_demo},
    {"test SPI demo",          spi_demo},
    {"test PWM demo",          pwm_demo},
    {"test Net Info demo",     network_demo},
    {"test SOCKET demo",       socket_demo},
    {"test Ping demo",         ping_demo},
    {"test LBS demo",          lbs_demo},
    {"test FTP demo",          ftp_demo},
    {"test Http demo",         http_demo},
    {"test FOTA demo",         fota_demo},

#ifdef MBTK_OPENCPU_SUPPORT_AT
    {"test AT demo",           at_demo},
#endif
#ifdef MBTK_OPENCPU_SUPPORT_TCP
    {"test TCP_API demo",      tcp_demo},
#endif
#ifdef MBTK_OPENCPU_SUPPORT_SSL
    {"test SSL_SOCKET demo",   ssl_demo},
#endif
#ifdef MBTK_OPENCPU_SUPPORT_SMS
    {"test SMS demo",          sms_demo},
#endif
#ifdef MBTK_OPENCPU_SUPPORT_MQTT
    {"test MQTT demo",         mqtt_demo},
#endif
#ifdef MBTK_OPENCPU_SUPPORT_LWM2M
    {"test lwm2m demo",        lwm2m_demo},
#endif
#ifdef MBTK_OPENCPU_SUPPORT_ZBAR
    {"test zbar demo",         zbar_demo},
#endif
    {"test ex flash demo",     ex_flash_demo},
    {"test WIFISCAN demo",     wifiscan_demo},
#ifdef FEATURE_SUBSYS_AUDIO_ENABLE
    {"test AUDIO demo",        audio_demo},
#ifdef MBTK_OPENCPU_SUPPORT_CALL
	{"test call demo",	       call_demo},
#endif
#ifdef MBTK_OPENCPU_GPS_SUPPORT
	{"gps demo",		   gps_demo},
#endif
#endif
};


void display_demo_menu(int port)
{
    int index = 0;
    char temp_buffer[50] = {0};

    for(index = 0; index < sizeof(test_demo_menu) / sizeof(test_demo_menu[0]); index++)
    {
        memset(temp_buffer, 0, sizeof(temp_buffer));
        if(strlen(test_demo_menu[index].demo_str) != 0)
        {
            sprintf(temp_buffer, "%2d - %s\r\n", index, test_demo_menu[index].demo_str);
            ol_uart_send(port, (uint8_t*)temp_buffer, strlen(temp_buffer));
        }
    }
    ol_uart_send(port, (uint8_t*)"Input Choice:\r\n", strlen("Input Choice:\r\n"));
}

void exec_demo_menu(int index, int port)
{
    char temp_buffer[50] = {0};

    if(index >= 0 && index < sizeof(test_demo_menu)/sizeof(test_demo_menu[0]))
    {
        if(NULL != test_demo_menu[index].func)
        {
            memset(temp_buffer, 0, sizeof(temp_buffer));
            sprintf(temp_buffer, "Begin>>>%s", test_demo_menu[index].demo_str);
            ol_uart_send(port, (uint8_t*)temp_buffer, strlen(temp_buffer));

            test_demo_menu[index].func();

            memset(temp_buffer, 0, sizeof(temp_buffer));
            sprintf(temp_buffer, "<<<End\r\n", test_demo_menu[index].demo_str);
            ol_uart_send(port, (uint8_t*)temp_buffer, strlen(temp_buffer));
        }
    }
    else
    {
        ol_uart_send(port, (uint8_t*)"Invalid input !!!\r\n", strlen("Invalid input !!!\r\n"));
    }
}


//---------------------------------------------------------
//                  UART Input Test Cmd
//---------------------------------------------------------
static osEventFlagsId_t uart_cmd_evt_handle = NULL;

void uart_cmd_test_cb(uint32_t event)
{
    OL_LOG_PRINTF("uart cmd test cb event %d", event);

    if(event & ARM_USART_EVENT_RX_TIMEOUT)
    {
        osEventFlagsSet(uart_cmd_evt_handle, 0x00000001);
    }
    if(event & ARM_USART_EVENT_RECEIVE_COMPLETE)
    {
        osEventFlagsSet(uart_cmd_evt_handle, 0x00000001);
    }
}

void uart_test_task(void* arg)
{
    int ret = -1;
    int index = 0;
    char cmd[12];
    ol_uart_config_t config = {0};
    uint8_t uart_cmd_slp_handle = 0xFF;

    config.baudRate = 115200;
    config.cb_event = uart_cmd_test_cb;
    config.control = (ARM_USART_MODE_ASYNCHRONOUS |
                      ARM_USART_DATA_BITS_8 |
                      ARM_USART_PARITY_NONE |
                      ARM_USART_STOP_BITS_1 |
                      ARM_USART_FLOW_CONTROL_NONE);

    ret = ol_uart_init(OL_UART_1, &config);
    if(OL_UART_OK != ret)
    {
        OL_LOG_INFO("init open  new msg queue fail NULL");
        return;
    }

    slpManApplyPlatVoteHandle("uart_cmd_slp_handle", &uart_cmd_slp_handle);

    uart_cmd_evt_handle = osEventFlagsNew(NULL);
    if(NULL == uart_cmd_evt_handle)
    {
        OL_LOG_PRINTF("create uart cmd test event flag handle fail, return");
        return;
    }

    while(1)
    {
        memset(cmd, 0, sizeof(cmd));
        display_demo_menu(OL_UART_1);
        slpManPlatVoteEnableSleep(uart_cmd_slp_handle, SLP_SLP1_STATE);

        ret = ol_uart_recv_async(OL_UART_1, (uint8_t*)cmd, sizeof(cmd));
        OL_LOG_PRINTF("uart async receive ret = %d", ret);

        osEventFlagsWait(uart_cmd_evt_handle, 0x00000001, osFlagsWaitAny, osWaitForever);

        slpManPlatVoteDisableSleep(uart_cmd_slp_handle, SLP_SLP1_STATE);
        OL_LOG_INFO("UART input - %s", cmd);

        index = atoi(cmd);
        exec_demo_menu(index, OL_UART_1);
    }
}

void uart_test_demo_menu(void)
{
    osThreadAttr_t task_attr;

    memset(&task_attr, 0, sizeof(task_attr));
    task_attr.name = "app test task";
    task_attr.stack_size = 16*1048;
    task_attr.priority = osPriorityNormal;

    osThreadNew(uart_test_task, NULL, &task_attr);
}


//---------------------------------------------------------
//                  USB Input Test Cmd
//---------------------------------------------------------
void usb_serl_input_cb(uint8_t *data, uint32_t len)
{
    test_msg_t msg = {0};

    if(NULL != data && len > 0)
    {
        msg.data = (uint8_t *)malloc(len);
        memset(msg.data, 0x0, len);
        msg.lenght = len;
        memcpy(msg.data, data, len);
        osMessageQueuePut(test_msg_q, &msg, 0, 1000);
    }
}

void usb_test_task(void *arg)
{
    int ret = -1;
    int index = 0;
    osStatus_t status = osError;
    test_msg_t msg = {0};

    test_msg_q = osMessageQueueNew(10, sizeof(test_msg_t), NULL);
    if(NULL == test_msg_q)
    {
        OL_LOG_INFO("create new msg queue fail NULL");
        return;
    }

    ret = ol_uart_init(OL_USB_SERL, NULL);
    if(OL_UART_OK != ret)
    {
        OL_LOG_INFO("init open  new msg queue fail NULL");
        return;
    }

    ol_uart_register_input_cb(OL_USB_SERL, usb_serl_input_cb);
    while(1)
    {
        display_demo_menu(OL_USB_SERL);
        status = osMessageQueueGet(test_msg_q, &msg, 0, osWaitForever);
        if(osOK == status)
        {
            index = atoi((char *)msg.data);
            exec_demo_menu(index, OL_USB_SERL);
        }
        free(msg.data);
    }

    ol_uart_uninit(OL_USB_SERL);
    osMessageQueueDelete(test_msg_q);
}

void usb_test_demo_menu(void)
{
    osThreadAttr_t task_attr;

    memset(&task_attr, 0, sizeof(task_attr));
    task_attr.name = "app test task";
    task_attr.stack_size = 16*1024;
    task_attr.priority = osPriorityNormal;

    osThreadNew(usb_test_task, NULL, &task_attr);
}

