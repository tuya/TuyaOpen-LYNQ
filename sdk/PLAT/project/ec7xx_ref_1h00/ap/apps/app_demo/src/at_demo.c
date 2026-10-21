/****************************************************************************
 *
 ****************************************************************************/
#include "string.h"
#include "osasys.h"
#include "cmsis_os2.h"
#include "ps_lib_api.h"
#include "ol_at_api.h"
#include "ol_log.h"

void at_urc(const CHAR *urc_str, UINT32 strlen)
{
    OL_LOG_INFO("at urc len[%d] str = %s",strlen, urc_str);
}

void at_demo(void)
{
    int ret = 0;
    char command[128];
    char respbuff[1024];

    ol_at_urc_register(at_urc);

    memset(command, 0x0, sizeof(command));
    memset(respbuff, 0x0, sizeof(respbuff));
    sprintf(command, "ATI\r\n");
    ret = ol_at_send_wait_resp(command, 60, respbuff, 1024);
    OL_LOG_INFO("send ATI return %d, resp=%s",ret, respbuff);

    memset(command, 0x0, sizeof(command));
    memset(respbuff, 0x0, sizeof(respbuff));
    sprintf(command, "AT+CFUN?\r\n");
    ret = ol_at_send_wait_resp(command, 60, respbuff, 1024);
    OL_LOG_INFO("send AT+CFUN? return %d, resp=%s",ret, respbuff);
}


