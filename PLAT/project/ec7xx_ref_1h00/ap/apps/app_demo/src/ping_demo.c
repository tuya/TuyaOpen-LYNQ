/****************************************************************************
 *
 ****************************************************************************/
#include "string.h"
#include "osasys.h"
#include "cmsis_os2.h"
#include "ping.h"
#include "ol_nw_api.h"
#include "ol_log.h"


void ping_callback(uint8_t ping_result, uint32_t recv_byte,uint32_t time_ms,uint32_t ttl)
{
    OL_LOG_PRINTF("test ping result= %d, recv_byte= %d, time= %d ms, ttl= %d",ping_result,recv_byte,time_ms,ttl);
}

void ping_demo(void)
{
    int count = 5;

    ol_ping_register_callback(ping_callback);

    ping_terminat(); //stop last ping

    ping_url_init((const CHAR*)"www.baidu.com", count, 0, 0, FALSE, 0x0001, 1);

    osDelay(1000);
    OL_LOG_PRINTF("check ping status = %d", PingChkStatus());
    osDelay(1000);
    OL_LOG_PRINTF("check ping status = %d", PingChkStatus());
}
