/****************************************************************************
 *
 ****************************************************************************/
#include "string.h"
#include "osasys.h"
#include "cmsis_os2.h"
#include "ol_log.h"
#include "ol_lwm2m_api.h"


extern void ctiotEngineInit(void);

void lwm2m_urc(const char *urc_str, unsigned long strlen)
{
    unsigned char* read_msg = NULL;
    int read_len = 0;

    OL_LOG_INFO("urc strlen = %d",strlen);
    OL_LOG_INFO("urc str = %s",urc_str);
    if (strcmp(urc_str, "+CTM2MRECV") == 0)
    {
        ol_lwm2m_read(&read_len,&read_msg);
        OL_LOG_INFO("read len %d,read msg %s",read_len,read_msg);
    }
}

void lwm2m_demo(void)
{
    ol_lwm2m_config_type config = {0};
    int ret = 0;

    ctiotEngineInit();

    memset(&config,0,sizeof(ol_lwm2m_config_type));
    memcpy(config.serverIP,"2000386509.nb.ctwing.cn",sizeof("2000386509.nb.ctwing.cn"));
    config.port = 5683;
    config.lifeTime = 3000;

    ol_lwm2m_set_recv_mode(OL_RECVMODE__WITH_INDICATE);
    ol_lwm2m_urc_register(lwm2m_urc);

    ret = ol_lwm2m_config(config);
    OL_LOG_INFO("set config = %d",ret);

    ret = ol_lwm2m_reg();
    OL_LOG_INFO("register ret = %d",ret);

    osDelay(1000);
    ret = ol_lwm2m_send("6c696572646131323334353637383930", OL_SENDMODE_CON);
    OL_LOG_INFO("ol_lwm2m_send = %d",ret);

    osDelay(20000);
    ol_lwm2m_dereg();
}
