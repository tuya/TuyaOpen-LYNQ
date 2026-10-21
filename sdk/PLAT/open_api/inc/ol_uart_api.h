/****************************************************************************
 *
 ****************************************************************************/
#ifndef __OL_UART_API__
#define __OL_UART_API__


#define OL_UART_OK                         0 ///< Operation succeeded
#define OL_UART_ERROR                     -1 ///< Unspecified error
#define OL_UART_ERROR_BUSY                -2 ///< Driver is busy
#define OL_UART_ERROR_TIMEOUT             -3 ///< Timeout occurred


typedef enum   /* The number of UART ports*/
{
    OL_UART_0,         //debug port
    OL_UART_1,
    OL_UART_2,
    OL_UART_3,
    OL_UART_4,         //reserve
    OL_USB_SERL,
}OL_UART_PORT_ENUM;

typedef void (*ol_uart_recv_cb_f)(uint8_t *data, uint32_t len);

//UART driver event define in Driver_USART.h
typedef void (*ol_uart_event_cb_f)(uint32_t event);

typedef struct
{
    uint32_t baudRate;
    uint32_t control;
    ol_uart_event_cb_f cb_event;
}ol_uart_config_t;


/*****************************************************************************
 *
 * DESCRIPTION
 *    This API is to open init uart port
 * PARAMETERS
 *    portNumber      : [IN]  uart id define in OL_UART_PORT_ENUM
 *    info            : [IN]  uart config params defined in ol_uart_config_t
 * RETURN
 *    =0 : success
 *    <0 : fail
 *
 *****************************************************************************/
extern int ol_uart_init(uint32_t portNumber, ol_uart_config_t* info);


/*****************************************************************************
 *
 * DESCRIPTION
 *    This API is to uninit uart port
 * PARAMETERS
 *    portNumber      : [IN]  uart id define in OL_UART_PORT_ENUM
 * RETURN
 *    =0 : success
 *    <0 : fail
 *
 *****************************************************************************/
extern int ol_uart_uninit(uint32_t portNumber);


/*****************************************************************************
 *
 * DESCRIPTION
 *    This API is to get uart rx data counts with DMA_MODE or IRQ_MODE
 * PARAMETERS
 *    portNumber      : [IN]  uart id
 * RETURN
 *    rx data counts
 *
 *****************************************************************************/
extern uint32_t ol_uart_rx_count(uint32_t portNumber);


/*****************************************************************************
 *
 * DESCRIPTION
 *    This API is to send data to uart port, wait until tx is empty
 * PARAMETERS
 *    portNumber      : [IN]  uart id define in OL_UART_PORT_ENUM
 *    data            : [IN]  point to data want to be send
 *    length          : [IN]  send data length
 * RETURN
 *    =0 : success
 *    <0 : fail
 *
 *****************************************************************************/
extern int ol_uart_send(uint32_t portNumber, uint8_t *data, uint32_t length);


/*****************************************************************************
 *
 * DESCRIPTION
 *    This API is to send data to uart port, complte event from init cb_event
 * PARAMETERS
 *    portNumber      : [IN]  uart id define in OL_UART_PORT_ENUM
 *    data            : [IN]  point to data want to be send
 *    length          : [IN]  send data length
 * RETURN
 *    =0 : success
 *    <0 : fail
 *
 *****************************************************************************/
extern int ol_uart_send_async(uint32_t portNumber, uint8_t *data, uint32_t length);


/*****************************************************************************
 *
 * DESCRIPTION
 *    This API is used to synchronously receive data from the UARTx port
 * PARAMETERS
 *    portNumber      : [IN]  uart id define in OL_UART_PORT_ENUM
 *    data            : [IN]  point to receive data buffer
 *    length          : [IN]  receive data buffer length
 *    timeout         : [IN]  timeout time for blocking and waiting, unit(ms)
 * RETURN
 *    >=0 : receive data count
 *    <0 : fail
 *
 *NOTE
 *    timeout = 0 : API will continue to block until data is received
 *    timeout > 0 : API blocking waiting until data is received or timeout is reached
 *    OL_USB_SERL port do nothing and returns 0 directly
 *
 *****************************************************************************/
extern int ol_uart_recv(uint32_t portNumber, uint8_t *data, uint32_t length, uint32_t timeout_ms);


/*****************************************************************************
 *
 * DESCRIPTION
 *    This API is used to asynchronously receive data from the UARTx port
 * PARAMETERS
 *    portNumber      : [IN]  uart id define in OL_UART_PORT_ENUM
 *    data            : [IN]  point to receive data buffer
 *    length          : [IN]  receive data buffer length
 * RETURN
 *    >=0 : receive data count
 *    <0 : fail
 *
 *NOTE
 *    If no data is read, the API will directly return
 *      when asyn message is received (cb_event: define in init config info),
 *      use ol_uart_rx_count check size of the data received in the buffer, 
 *      there is no need to call this API again to read the data
 *
 *    OL_USB_SERL port do nothing and returns 0 directly
 *
 *****************************************************************************/
extern int ol_uart_recv_async(uint32_t portNumber, uint8_t* data, uint32_t length);


/*****************************************************************************
 *
 * DESCRIPTION
 *    This API is to register receive data callback
 * PARAMETERS
 *    portNumber      : [IN]  uart id  //current only OL_USB_SERL support
 *    recv_cb         : [IN]  receive data callback function
 * RETURN
 *    =0 : success
 *    <0 : fail
 *
 *****************************************************************************/
extern int ol_uart_register_input_cb(uint32_t portNumber, ol_uart_recv_cb_f recv_cb);


#endif/*__OL_UART_API__*/
