#ifndef _OL_TCP_API_H_
#define _OL_TCP_API_H_

typedef enum
{
    OL_PROTOCOL_UDP = 0,
    OL_PROTOCOL_TCP = 1,
}ol_tcp_type;


typedef void (*ol_tcp_event_cb)(UINT8 event,void *body);

/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to initialize the tcp connections
 * PARAMETERS
 *      protocol    [IN]: (mandatory)type of connection(TCP or UDP), enum ol_tcp_type
 *      localip     [IN]: (option)local ip address
 *      localport   [IN]: (option)local port number
 *      destip      [IN]: (mandatory)destination address(server ip address)
 *      destport    [IN]: (mandatory)destination port number(server port number)
 *      callback    [IN]: event callback function for TCP links
 * RETURN
 *      >=0  : Link Number
 *      <0   : error
 *
 *****************************************************************************/
extern INT32 ol_tcp_init(ol_tcp_type protocol, char *localip, UINT16 localport, char *destip, UINT16 destport, ol_tcp_event_cb callback);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to send data
 * PARAMETERS
 *      index         [IN]: Link Number(The return value of ol_tcp_init)
 *      data          [IN]: send data
 *      datalen       [IN]: send data length
 *      raiinfo       [IN]: reserve
 *      expectflag    [IN]: reserve
 *      sequence      [IN]: (option)the sequence number of the UL data. 1~255: a UL status event will be indicate with the socket event callback(TCPIP_CONNECTION_UL_STATUS_EVENT); 0:no UL status event(only TCPIP_CONNECTION_UL_TOTAL_LEN_EVENT)
 * RETURN
 *      >=0  : successfully sent length
 *      <0   : error
 *
 *****************************************************************************/
extern INT32 ol_tcp_send(INT32 index, UINT8 *data, UINT16 datalen, UINT8 raiinfo, UINT8 expectflag, UINT8 sequence);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to close the tcp connections
 * PARAMETERS
 *      index         [IN]: Link Number(The return value of ol_tcp_init)
 *      timeout       [IN]: timeout(seconds, suggest > 3 seconds)(if the timeout is 0 ==>disable close timeout feature)
 * RETURN
 *      =0     : successfully
 *      other  : error
 *
 *****************************************************************************/
extern INT32 ol_tcp_uninit(INT32 index, UINT16 timeout);

#endif
