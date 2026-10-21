/****************************************************************************
 *
 ****************************************************************************/
#include "string.h"
#include "bsp.h"
#include "osasys.h"
#include "cmsis_os2.h"
#include "sockets.h"
#include "netdb.h"
#include "ec_tcpip_api.h"
#include "ol_log.h"
#include "ol_tcp_api.h"


#define TEST_IP_ADDR "118.114.239.159"
#define TEST_PORT 30165

void tcp_api_cb(UINT8 connectionEventType, void* bodyEvent)
{
    OL_LOG_INFO("----------- tcp_api_cb event : %d -------------", connectionEventType);

    if(TCPIP_CONNECTION_STATUS_EVENT == connectionEventType)//connection event
    {
        TcpipConnectionStatusInd* status = (TcpipConnectionStatusInd*)bodyEvent;
        OL_LOG_INFO("connection event id = %d", status->connectionId);
        OL_LOG_INFO("connection event status = %d", status->status); //enum TcpipConnectionStatus
        OL_LOG_INFO("connection event cause = %d", status->cause); //enum MtErrorResultCode
    }
    else if(TCPIP_CONNECTION_RECEIVE_EVENT == connectionEventType)//recieve data event
    {
        TcpipConnectionRecvDataInd* recv = (TcpipConnectionRecvDataInd*)bodyEvent;
        OL_LOG_INFO("recieve event id = %d, recv length = %d", recv->connectionId, recv->length);

        char *ptr = malloc(recv->length + 1);
        memset(ptr, 0x0, recv->length + 1);
        memcpy(ptr, recv->data, recv->length);
        OL_LOG_INFO("recieve event data = %s", ptr);
        free(ptr);
    }
    else if(TCPIP_CONNECTION_UL_STATUS_EVENT == connectionEventType)//send data event,sequence:1~255
    {
        TcpipConnectionUlDataStatusInd* ul = (TcpipConnectionUlDataStatusInd*)bodyEvent;
        OL_LOG_INFO("send event id = %d", ul->connectionId);
        OL_LOG_INFO("send event status = %d", ul->status); //enum TcpipConnectionUlStatus
        OL_LOG_INFO("send event sequence = %d", ul->sequence);
    }
    else if(TCPIP_CONNECTION_UL_TOTAL_LEN_EVENT == connectionEventType)//send data total length
    {
        TcpipConnectionUlTotalLenInd* ul = (TcpipConnectionUlTotalLenInd*)bodyEvent;
        OL_LOG_INFO("send total length id = %d", ul->connectionId);
        OL_LOG_INFO("send total length len = %d", ul->totalLen);
    }
}

void tcp_demo(void)
{
    int index[5] = {0};
    int ret = 0;
    int i = 0;

    memset(index, 0x0, sizeof(index));
    for(i = 0; i < 5; i++)
    {
        index[i] = ol_tcp_init(OL_PROTOCOL_TCP, NULL, 0, TEST_IP_ADDR, TEST_PORT, tcp_api_cb);
        OL_LOG_INFO("ol_tcp_init index[i] = %d", index[i]);
    }

    osDelay(2000);

    for(i = 0; i < 5; i++)
    {
        char buf[10] = {0};
        memset(buf, 0x0, sizeof(buf));
        snprintf(buf, 10, "index[%d]", i);
        ret = ol_tcp_send(index[i], (UINT8*)buf, strlen(buf), 0, 0, 0);
        OL_LOG_INFO("ol_tcp_send ret = %d", ret);
    }

    osDelay(20000);//close after 20s

    for(i = 0; i < 5; i++)
    {
        ret = ol_tcp_uninit(index[i], 5);
        OL_LOG_INFO("ol_tcp_uninit ret = %d", ret);
    }
}

