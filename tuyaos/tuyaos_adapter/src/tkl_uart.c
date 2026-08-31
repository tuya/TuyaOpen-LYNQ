#include <stdint.h>
#include "tkl_uart.h"
#include "ol_uart_api.h"
#include "vlog.h"
#include "bsp.h"
#include "cmsis_os2.h"
#include "tuya_ringbuf.h"
#include "tkl_thread.h"
#include "tkl_queue.h"

#undef LOGD
#define LOGD(fmt, ...)

#define UART_DEV_NUM    4
#define USB_DEV_INDEX  3
#define UART_RECV_BUFFER_LEN    (3 * 1024)
#define OL_UART_RECV_BUFF_LEN   (3 * 1024)

static TKL_THREAD_HANDLE uart_recv_handle = NULL;
static TKL_QUEUE_HANDLE uart_queue_handle = NULL;

typedef struct {
	bool 	init_flag;
	OL_UART_PORT_ENUM port;
	TUYA_UART_IRQ_CB ty_cb;
	TUYA_RINGBUFF_T ringbuffer;
	osMutexId_t rb_lock;
	uint8_t *buffer;
} uart_dev_t;

static uart_dev_t uart_dev[UART_DEV_NUM] = {
	{
		.init_flag = false,
		.port = OL_UART_1,
	},
	{
		.init_flag = false,
		.port = OL_UART_2,
	},
	{
		.init_flag = false,
		.port = OL_UART_3,
	},
	{
		.init_flag = false,
        .port = OL_USB_SERL,
	}
};

static void uart_callback(uint8_t port, uint32_t event)
{
    LOGD("test uart cb event  port:%d %d", port, event);
    if(event & ARM_USART_EVENT_RX_TIMEOUT || event & ARM_USART_EVENT_RECEIVE_COMPLETE)
    {
		if(uart_queue_handle)
			tkl_queue_post(uart_queue_handle, &port, 0);
    }
}

static void uart0_callback(uint32_t event)
{
	uart_callback(0, event);
}
static void uart1_callback(uint32_t event)
{
	uart_callback(1, event);
}
static void uart2_callback(uint32_t event)
{
	uart_callback(2, event);
}

static void usb_recv_callback(uint8_t *data, uint32_t len)
{
	uint8_t port = USB_DEV_INDEX;
	osMutexAcquire(uart_dev[USB_DEV_INDEX].rb_lock, osWaitForever);
	tuya_ring_buff_write(uart_dev[USB_DEV_INDEX].ringbuffer, data, len);
	osMutexRelease (uart_dev[USB_DEV_INDEX].rb_lock);
	if(uart_queue_handle)
		tkl_queue_post(uart_queue_handle, &port, 0);
}

static void uart_recv_thread(void *arg)
{
	int ret, recv_len = 0;
	uint8_t port;
	while(1)
	{	
		ret = tkl_queue_fetch(uart_queue_handle, &port, -1);
		if(ret == 0) {
			if(port == USB_DEV_INDEX) {
				if(uart_dev[port].ty_cb) {
					uart_dev[port].ty_cb(port);
				}
			} else {
				if(uart_dev[port].init_flag) {
					osMutexAcquire(uart_dev[port].rb_lock, osWaitForever);
					recv_len = ol_uart_rx_count(uart_dev[port].port);
					int len = tuya_ring_buff_write(uart_dev[port].ringbuffer, uart_dev[port].buffer, recv_len);
					osMutexRelease (uart_dev[port].rb_lock);
					if(len != recv_len) {
						LOGE("uart%d rb full %d %d", port, len, recv_len);
					}
					ret = ol_uart_recv_async(uart_dev[port].port, uart_dev[port].buffer , OL_UART_RECV_BUFF_LEN);
					if(uart_dev[port].ty_cb) {
						uart_dev[port].ty_cb(port);
					}
				}
			}
		}
	}
}

static OPERATE_RET tkl_uart_init_pre(void)
{
	OPERATE_RET ret = OPRT_OK;
	if(uart_queue_handle == NULL)
		ret = tkl_queue_create_init(&uart_queue_handle, sizeof(uint8_t), 10);
	if(uart_recv_handle == NULL)
		ret |= tkl_thread_create(&uart_recv_handle, "uart", 4096, 4, uart_recv_thread, NULL);
	return ret;
}

static OPERATE_RET __uart_init(TUYA_UART_NUM_E port_id, TUYA_UART_BASE_CFG_T* cfg)
{
	ol_uart_config_t config = {0};

	config.baudRate = cfg->baudrate;
	config.control = ARM_USART_MODE_ASYNCHRONOUS;
	switch (cfg->databits) {
		case TUYA_UART_DATA_LEN_5BIT: 	config.control |= ARM_USART_DATA_BITS_5; 	break;
		case TUYA_UART_DATA_LEN_6BIT: 	config.control |= ARM_USART_DATA_BITS_6; 	break;
		case TUYA_UART_DATA_LEN_7BIT: 	config.control |= ARM_USART_DATA_BITS_7; 	break;
		case TUYA_UART_DATA_LEN_8BIT:
		default: config.control |= ARM_USART_DATA_BITS_8; 	break;
	}

	switch (cfg->parity) {
		case TUYA_UART_PARITY_TYPE_NONE:
		default: 							config.control |= ARM_USART_PARITY_NONE; 	break;
		case TUYA_UART_PARITY_TYPE_ODD: 	config.control |= ARM_USART_PARITY_ODD; 	break;
		case TUYA_UART_PARITY_TYPE_EVEN: 	config.control |= ARM_USART_PARITY_EVEN; 	break;
	}

	switch (cfg->stopbits) {
		case TUYA_UART_STOP_LEN_1BIT:
		default: 						config.control |= ARM_USART_STOP_BITS_1; 	break;
		case TUYA_UART_STOP_LEN_1_5BIT1:config.control |= ARM_USART_STOP_BITS_1_5; 	break;
		case TUYA_UART_STOP_LEN_2BIT: 	config.control |= ARM_USART_STOP_BITS_2; 	break;
	}

	switch (cfg->flowctrl) {
		case TUYA_UART_FLOWCTRL_NONE:
		default: 							config.control |= ARM_USART_FLOW_CONTROL_NONE; 	break;
		case TUYA_UART_FLOWCTRL_RTSCTS: 	config.control |= ARM_USART_FLOW_CONTROL_RTS_CTS; 	break;
		case TUYA_UART_FLOWCTRL_XONXOFF: 	config.control |= ARM_USART_FLOW_CONTROL_RTS; 	break;
		case TUYA_UART_FLOWCTRL_DTRDSR: 	config.control |= ARM_USART_FLOW_CONTROL_CTS; 	break;
	}
	if(port_id == 0)
		config.cb_event = uart0_callback;
	else if(port_id == 1) 
		config.cb_event = uart1_callback;
	else {
		config.cb_event = uart2_callback;
	}

	int ret;
	if (OL_USB_SERL == uart_dev[port_id].port) {
		ret = ol_uart_init(uart_dev[port_id].port, NULL);
	} else {
		ret = ol_uart_init(uart_dev[port_id].port, &config);
	}
	if (OL_UART_OK != ret) {
		LOGE("uart %d init failed, ret: %d", port_id, ret);
		return OPRT_COM_ERROR;
	}

	if (OL_USB_SERL != uart_dev[port_id].port) {
		uart_dev[port_id].buffer = (uint8_t*)malloc(OL_UART_RECV_BUFF_LEN + 1);
		if (NULL == uart_dev[port_id].buffer) {
			LOGE("malloc ol uart buffer failed");
			return OPRT_COM_ERROR;
		}
		memset(uart_dev[port_id].buffer , 0, OL_UART_RECV_BUFF_LEN + 1);
		ol_uart_recv_async(uart_dev[port_id].port, uart_dev[port_id].buffer , OL_UART_RECV_BUFF_LEN);
	} else {
		ol_uart_register_input_cb(uart_dev[port_id].port, usb_recv_callback);
	}

	return OPRT_OK;
}

OPERATE_RET tkl_uart_init(TUYA_UART_NUM_E port_id, TUYA_UART_BASE_CFG_T *cfg)
{
	if(tkl_uart_init_pre()) {
		return OPRT_COM_ERROR;
	}

	if (port_id >= UART_DEV_NUM) {
		LOGE("invalid port %d", port_id);
		return OPRT_INVALID_PARM;
	}

	if (NULL == cfg) {
		LOGE("cfg is null, uart %d init failed", port_id);
		return OPRT_COM_ERROR;
	}

	if(uart_dev[port_id].init_flag)
		return OPRT_OK;

	OL_UART_PORT_ENUM ori_port = uart_dev[port_id].port;
	memset(&uart_dev[port_id], 0, sizeof(uart_dev_t));
	uart_dev[port_id].port = ori_port;

	tuya_ring_buff_create(UART_RECV_BUFFER_LEN, OVERFLOW_STOP_TYPE, &uart_dev[port_id].ringbuffer);
	if (NULL == uart_dev[port_id].ringbuffer) {
		LOGE("create ringbuffer failed");
		tkl_uart_deinit(port_id);
		return OPRT_COM_ERROR;
	}

	uart_dev[port_id].rb_lock = osMutexNew(NULL);
	if (NULL == uart_dev[port_id].rb_lock) {
		LOGE("create rb lock failed");
		tkl_uart_deinit(port_id);
		return OPRT_COM_ERROR;
	}

	if(__uart_init(port_id, cfg))
		return OPRT_COM_ERROR;

	LOGI("uart %d init success, baud: %d, databit: %d, stopbit: %d, parity: %d, flowctrl: %d", port_id, cfg->baudrate, cfg->databits, cfg->stopbits, cfg->parity, cfg->flowctrl);
	uart_dev[port_id].init_flag = true;
	return OPRT_OK;
}

OPERATE_RET tkl_uart_deinit(TUYA_UART_NUM_E port_id)
{
	LOGD("tkl_uart_deinit:%d", port_id);

    if (port_id >= UART_DEV_NUM || uart_dev[port_id].init_flag == false) {
		LOGE("invalid port %d", port_id);
		return OPRT_INVALID_PARM;
	}

	osMutexAcquire(uart_dev[port_id].rb_lock, osWaitForever);
	uart_dev[port_id].init_flag = false;
	ol_uart_uninit(uart_dev[port_id].port);

	if(uart_dev[port_id].port != OL_USB_SERL)
		if(uart_dev[port_id].buffer) {
			free(uart_dev[port_id].buffer);
			uart_dev[port_id].buffer = NULL;
		}

	if (uart_dev[port_id].ringbuffer) {
		tuya_ring_buff_free(uart_dev[port_id].ringbuffer);
		uart_dev[port_id].ringbuffer = NULL;
	}
	osMutexRelease(uart_dev[port_id].rb_lock);

	if(uart_dev[port_id].rb_lock)
		osMutexDelete(uart_dev[port_id].rb_lock);

	LOGI("uart %d deinit success", port_id);
	return OPRT_OK;
}

INT_T tkl_uart_write(TUYA_UART_NUM_E port_id, VOID_T* buff, UINT16_T len)
{
	LOGD("uart send port %d %d len:%d", port_id, uart_dev[port_id].port, len);
	int ret = ol_uart_send(uart_dev[port_id].port, (uint8_t*)buff, len);
	return (0 == ret) ? len : 0;
}

VOID_T tkl_uart_rx_irq_cb_reg(TUYA_UART_NUM_E port_id, TUYA_UART_IRQ_CB rx_cb)
{	
	if (port_id < UART_DEV_NUM) {
		uart_dev[port_id].ty_cb = rx_cb;
	} else {
		LOGE("uart rx irq cb register failed, illegal port id %d", port_id);
	}
}

VOID_T tkl_uart_tx_irq_cb_reg(TUYA_UART_NUM_E port_id, TUYA_UART_IRQ_CB tx_cb)
{
	LOGE("uart %d tx irq cb register not support", port_id);
    return ;
}

INT_T tkl_uart_read(TUYA_UART_NUM_E port_id, VOID_T* buff, UINT16_T len)
{
	UINT32_T read_len = 0;

	
	if (port_id >= UART_DEV_NUM || uart_dev[port_id].init_flag == false) {
		LOGE("uart read failed, illegal port id: %d", port_id);
		return 0;
	}

	osMutexAcquire(uart_dev[port_id].rb_lock, osWaitForever);
	read_len = tuya_ring_buff_read(uart_dev[port_id].ringbuffer, buff, (uint32_t)len);
	osMutexRelease(uart_dev[port_id].rb_lock);

	LOGD("tkl_uart_read %d len:%d %d", port_id, len, tuya_ring_buff_used_size_get(uart_dev[port_id].ringbuffer));
	return (INT_T)read_len;
}

OPERATE_RET tkl_uart_set_tx_int(TUYA_UART_NUM_E port_id, BOOL_T enable)
{
	LOGE("uart set tx int not supported");
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_uart_set_rx_flowctrl(TUYA_UART_NUM_E port_id, BOOL_T enable)
{
	LOGE("uart set rx flowctrl not supported");
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_uart_wait_for_data(TUYA_UART_NUM_E port_id, INT_T timeout_ms)
{
	return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_uart_ioctl(TUYA_UART_NUM_E port_id, UINT32_T cmd, VOID_T* arg)
{
	if (port_id >= UART_DEV_NUM) {
		LOGE("uart ioctl failed, illegal port: %u", port_id);
		return OPRT_COM_ERROR;
	}

	LOGI("tkl_uart_ioctl cmd:%d",cmd);
	TUYA_UART_BASE_CFG_T* cfg = (TUYA_UART_BASE_CFG_T*) arg;
	TUYA_UART_IRQ_CB cb;
	OPERATE_RET ret = OPRT_OK;
	switch (cmd) {
		case TUYA_UART_RECONFIG_CMD:
			if (!cfg) {
				LOGE("uart ioctl failed, reconfig arg null, port: %d", port_id);
				return OPRT_COM_ERROR;
			}
			cb = uart_dev[port_id].ty_cb;
			tkl_uart_deinit(port_id);
			ret = tkl_uart_init(port_id, cfg);
			uart_dev[port_id].ty_cb = cb;
			break;
		default:
			LOGE("uart ioctl failed, port: %d, cmd: %d", port_id, cmd);
			break;
	}
	return ret;
}

#if 0
#include "tkl_thread.h"
#include "tkl_semaphore.h"
#include "tkl_system.h"

static TKL_THREAD_HANDLE uart_thread[UART_DEV_NUM];
static TKL_SEM_HANDLE uart_sem[UART_DEV_NUM];

void uart_irq_cb(TUYA_UART_NUM_E port_id)
{
	tkl_semaphore_post(uart_sem[port_id]);
}

void uart_task(void *args)
{
	int len, ret;
	int port = (int)args;
	char *buffer = malloc(2048);
	TUYA_UART_BASE_CFG_T cfg = {
		.baudrate = 115200,
		.databits = 8,
		.flowctrl = 0,
		.parity = 0,
		.stopbits = 1,
	};
	tkl_uart_init(port, &cfg);
	tkl_uart_rx_irq_cb_reg(port, uart_irq_cb);
	tkl_uart_write(port, "Uart test\r\n", strlen("Uart test\r\n"));
	sprintf(buffer, "%s %s\r\n", __DATE__, __TIME__);
	tkl_uart_write(port, buffer, strlen(buffer));

	int cnt = 0;
	while(1) {
		ret = tkl_semaphore_wait(uart_sem[port], -1);
		memset(buffer, 0, 2048);
		len = tkl_uart_read(port, buffer, 2048);
		if(len) {
			// tkl_uart_write(port, "uart echo>>\r\n", strlen("uart echo>>\r\n"));
			tkl_uart_write(port, buffer, len);
		}
	}
}

void tkl_uart_test(void)
{
	OPERATE_RET ret = 0;
	ret = tkl_semaphore_create_init(&uart_sem[0], 0, 1);
	ret = tkl_thread_create(&uart_thread[0], "uart0_thread", 4096,  2, uart_task, (void *)0);
	ret = tkl_semaphore_create_init(&uart_sem[1], 0, 1);
	ret = tkl_thread_create(&uart_thread[1], "uart1_thread", 4096,  2, uart_task, (void *)1);
	ret = tkl_semaphore_create_init(&uart_sem[2], 0, 1);
	ret = tkl_thread_create(&uart_thread[2], "uart2_thread", 4096,  2, uart_task, (void *)2);
	ret = tkl_semaphore_create_init(&uart_sem[3], 0, 1);
	ret = tkl_thread_create(&uart_thread[3], "uart3_thread", 4096,  2, uart_task, (void *)3);
	while(1)
	{
		tkl_system_sleep(100);
	}
}
#endif
