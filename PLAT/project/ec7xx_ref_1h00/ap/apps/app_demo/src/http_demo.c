/****************************************************************************
 *
 ****************************************************************************/
#include "osasys.h"
#include "ostask.h"
#include "cmsis_os2.h"
#include "HTTPClient.h"
#include "ol_log.h"
#include "ol_http_api.h"

/* set 1 to test https */
#define TEST_HTTPS              (0)

#define HTTP_RECV_BUF_SIZE      (1500)
#define HTTP_HEAD_BUF_SIZE      (800)

#if TEST_HTTPS
char testCaCrt[] =
{
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIDKjCCAhICCQCOewfZiRCiNjANBgkqhkiG9w0BAQUFADBXMQswCQYDVQQGEwJD\r\n"
    "TjEQMA4GA1UECBMHU2lDaHVhbjEVMBMGA1UEChMMTU9CSUxFVEVLLkNBMQswCQYD\r\n"
    "VQQLEwJJVDESMBAGA1UEAxMJTU9CSUxFVEVLMB4XDTE4MDkxODA4MDUzMloXDTMz\r\n"
    "MDkxOTA4MDUzMlowVzELMAkGA1UEBhMCQ04xEDAOBgNVBAgTB1NpQ2h1YW4xFTAT\r\n"
    "BgNVBAoTDE1PQklMRVRFSy5DQTELMAkGA1UECxMCSVQxEjAQBgNVBAMTCU1PQklM\r\n"
    "RVRFSzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAOkdYJF1h1xjKbY0\r\n"
    "ipbl88G653PiCh8ZMjmIUYeiDKC8+0wtXZtHvQIl6AncOzBy9XHVOctbKn34exC8\r\n"
    "SEotMuo2T49vs9VtE8GYu2pOrf3m42NpLRnYAxfm9qw53CMHx+Jn7Oa9fnxa8haA\r\n"
    "pRc2BTVadWGoS8EEwoZFk0eNb7Z2Gc7U0c+GhISI4oVTTocGvGgMzkvduu5JJbbc\r\n"
    "BOcNFrii9sRO9vtOYQtqOEg01Uum2Dwp/o2bDLXNJEqAIh4WACiM4iPmmlRHWT2y\r\n"
    "NjQ3vcbEdrFwbHRtO46+Vw54HnSyCoFb3uCHMNMvXObZ/8AU9E3Cgat4j0sgEeB0\r\n"
    "hqA4MiMCAwEAATANBgkqhkiG9w0BAQUFAAOCAQEAtEAjf0CjsLgG9ROdmp1qXYft\r\n"
    "+ndIT5l82KRK57ZQsfdFbnJOvALeF/ICKU0M2TXgJNiGOA5RxDi00YYdMbOIPwVZ\r\n"
    "JH4b87J/LYdLAGf+Q+kVI6gWH3hPm4Jzfzq/40KVrf3mpa54yWz6ZYtwfxBjrMgr\r\n"
    "IVe0O5SIJ99lsddgzgUkqYN2vWJW2zZ50xuXOAyo+pOnjzX0wuOcaBT3JCHWJRAb\r\n"
    "VhJCf9JbswDgnddJerqFtB8pnpAOdGokLCOoM06q3s3P9mhGX+72HXdX7G8CSAuG\r\n"
    "PVCGf6RaF0/G4B9R1c3du3lZRlQWfx2pxyU0LS86iFQFWqzqcWEXIcULVdcErQ==\r\n"
    "-----END CERTIFICATE-----\r\n"
};
#endif

void http_response_cb(char *headbuf, char *buffer, int size)
{
    OL_LOG_PRINTF("recv head buffer = %s",headbuf);
    if(size > 0)
    {
        OL_LOG_PRINTF("recv buffer = %s",buffer);
        OL_LOG_PRINTF("recv size = %d",size);
    }
}

void http_demo(void)
{
    OL_LOG_INFO("enter http_demo");
    ol_http_cfg config;
    http_client httpClient = NULL;
    OL_HTTP_Result result = 0;
    int test_meth = 0;

#if TEST_HTTPS
    config.url = "https://220.167.54.26:4430/ceshi.txt";
    config.caCert = (char *)testCaCrt;
    config.seclevel = 1;
#else
    config.url = "http://leshan.eigencomm.com:8888/test";
#endif

    config.timeout_s = 2;
    config.timeout_r = 20;
    config.callback = http_response_cb;

    httpClient = ol_http_client_init();
    if(httpClient == NULL)
    {
        OL_LOG_PRINTF("Httpclient Init Error");
        return;
    }
    ol_http_set_cfg(httpClient, config);
    result = ol_http_connect(httpClient);
    if(result == OL_HTTP_OK)
    {
        if(test_meth == 0) //test http GET
        {
            result = ol_http_get(httpClient);
            OL_LOG_INFO("test http get result %d",result);
        }
        else if(test_meth == 1) //test http POST
        {
            result = ol_http_post(httpClient,"test post data");
            OL_LOG_INFO("test http post result %d",result);
        }
        else if(test_meth == 2) //test http HEAD
        {
            ol_http_send_request(httpClient, OL_HTTP_HEAD,NULL);
        }
    }
    else
    {
        OL_LOG_INFO("connect error %d",result);
    }
    ol_http_close(httpClient);
}
