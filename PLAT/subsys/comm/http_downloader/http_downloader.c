#include "cmsis_os2.h"
#include "http_downloader.h"
#include "ps_lib_api.h"
#include "networkmgr.h"
#include DEBUG_LOG_HEADER_FILE
#include <string.h>

#define HTTP_RECV_BUF_SIZE (1501)
#define HTTP_HEAD_BUF_SIZE (800)

static const char* http_ca_crt = {

    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIDSjCCAjKgAwIBAgIQRK+wgNajJ7qJMDmGLvhAazANBgkqhkiG9w0BAQUFADA/\r\n"
    "MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\r\n"
    "DkRTVCBSb290IENBIFgzMB4XDTAwMDkzMDIxMTIxOVoXDTIxMDkzMDE0MDExNVow\r\n"
    "PzEkMCIGA1UEChMbRGlnaXRhbCBTaWduYXR1cmUgVHJ1c3QgQ28uMRcwFQYDVQQD\r\n"
    "Ew5EU1QgUm9vdCBDQSBYMzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\r\n"
    "AN+v6ZdQCINXtMxiZfaQguzH0yxrMMpb7NnDfcdAwRgUi+DoM3ZJKuM/IUmTrE4O\r\n"
    "rz5Iy2Xu/NMhD2XSKtkyj4zl93ewEnu1lcCJo6m67XMuegwGMoOifooUMM0RoOEq\r\n"
    "OLl5CjH9UL2AZd+3UWODyOKIYepLYYHsUmu5ouJLGiifSKOeDNoJjj4XLh7dIN9b\r\n"
    "xiqKqy69cK3FCxolkHRyxXtqqzTWMIn/5WgTe1QLyNau7Fqckh49ZLOMxt+/yUFw\r\n"
    "7BZy1SbsOFU5Q9D8/RhcQPGX69Wam40dutolucbY38EVAjqr2m7xPi71XAicPNaD\r\n"
    "aeQQmxkqtilX4+U9m5/wAl0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNV\r\n"
    "HQ8BAf8EBAMCAQYwHQYDVR0OBBYEFMSnsaR7LHH62+FLkHX/xBVghYkQMA0GCSqG\r\n"
    "SIb3DQEBBQUAA4IBAQCjGiybFwBcqR7uKGY3Or+Dxz9LwwmglSBd49lZRNI+DT69\r\n"
    "ikugdB/OEIKcdBodfpga3csTS7MgROSR6cz8faXbauX+5v3gTt23ADq1cEmv8uXr\r\n"
    "AvHRAosZy5Q6XkjEGB5YGV8eAlrwDPGxrancWYaLbumR9YbK+rlmM6pZW87ipxZz\r\n"
    "R8srzJmwN0jP41ZL9c8PDHIyh8bwRLtTcm1D9SZImlJnt1ir/md2cXjbDaJWFBM5\r\n"
    "JDGFoqgCWjBH4d1QB7wCCZAA62RjYJsWvIjJEubSfZGL+T0yjWW06XyxV3bqxbYo\r\n"
    "Ob8VZRzI9neWagqNdwvYkQsEjgfbKbYK7p2CNTUQ\r\n"
    "-----END CERTIFICATE-----"};

static bool wait_for_net_ready(uint32_t timeout)
{
    NmAtiNetifInfo netInfo;
    int32_t timeout_ms = timeout;
    do
    {
        appGetNetInfoSync(0, &netInfo);
        if(((netInfo.netStatus == NM_NETIF_ACTIVATED) &&
            ((netInfo.ipType == NM_NET_TYPE_IPV4) ||
             (netInfo.ipType == NM_NET_TYPE_IPV6) ||
             (netInfo.ipType == NM_NET_TYPE_IPV4V6))))
        {
            return true;
        }
        osDelay(20);
        timeout_ms -= 20;
    } while(timeout_ms > 0);
    return false;
}

static void http_downloader_http_init(HttpClientContext* client)
{
    memset(client, 0, sizeof(HttpClientContext));
    client->caCert = (char*)http_ca_crt;
    client->caCertLen = strlen(http_ca_crt) + 1;
    client->timeout_s = 2;
    client->timeout_r = 10;
    client->ignore = 1;
    client->seclevel = 0;
    client->sni = 1;
    client->ciphersuite[0] = 0xFFFF;
}

void timer_data_call_back(void* argument)
{
    HttpDownloader_t* downloader = (HttpDownloader_t*)argument;
    int ret = 0;
    uint8_t* data_buff = NULL;
    uint32_t data_size = 0;
    while(!downloader->stop_flag)
    {
        ret = rbuffer_lock_front_data_frame(downloader->rbuff, &data_buff,
                                            &data_size, 10);
        if(ret != 0)
        {
            break;
        }
        DownloadInfo_t info = {0};
        info.cur_pos = downloader->section_pos;
        info.total_pos = downloader->total_size;
        info.status = HTTP_DOWNLOADER_STATUS_DOWNLOADING;
        ret =
            downloader->cb(&info, data_buff, data_size, downloader->user_data);
        if(ret == 0)
        {
            downloader->write_size += data_size;
            rbuffer_unlock_and_pop_front(downloader->rbuff);
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, timer_data_call_back_free_push,
                          P_INFO, "push buffer size: %d, write|total: %d|%d",
                          data_size, downloader->write_size,
                          downloader->total_size);
        }
        else
        {
            // ECPLAT_PRINTF(UNILOG_PLAT_MEDIA,
            //               timer_data_call_back_free_buffer_not_enough,
            //               P_INFO, "free buffer is no enough");
            rbuffer_unlock_front_data_frame(downloader->rbuff);
            break;
        }
    }
}

static void http_downloader_thread(void* argument)
{
    HttpDownloader_t* downloader = (HttpDownloader_t*)argument;
    DownloadInfo_t info = {0};
    HTTPResult http_result = HTTP_INTERNAL;
    HttpClientData http_data = {0};
    downloader->state = HTTP_DOWNLOADER_STATE_RUNNING;
    char* head_buffer = NULL;
    char* data_buffer = NULL;
    uint32_t download_pos = downloader->start_pos;
    uint32_t download_size = 0;
    int ret = 0;
    // 2s内没有成功驻网直接返回错误退出
    bool net_ready = wait_for_net_ready(2000);
    if(!net_ready)
    {
        if(downloader->cb)
        {
            info.cur_pos = 0;
            info.total_pos = -1;
            info.status = HTTP_DOWNLOADER_STATUS_ERROR;
            downloader->cb(&info, NULL, 0, downloader->user_data);
        }
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, http_downloader_net_not_ready, P_INFO,
                      "network not ready");
        downloader->state = HTTP_DOWNLOADER_STATE_IDLE;
        osThreadExit();
    }
    http_downloader_http_init(&downloader->client);
    head_buffer = malloc(HTTP_HEAD_BUF_SIZE);
    data_buffer = malloc(HTTP_RECV_BUF_SIZE);
    http_result = httpConnect(&downloader->client, downloader->url);
    // 无法连接服务器则退出下载
    if(http_result != HTTP_OK)
    {
        if(downloader->cb)
        {
            info.cur_pos = 0;
            info.total_pos = -1;
            info.status = HTTP_DOWNLOADER_STATUS_ERROR;
            downloader->cb(&info, NULL, 0, downloader->user_data);
        }
        free(head_buffer);
        free(data_buffer);
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, http_downloader_net_conn_failed,
                      P_INFO, "connect HTTP server failed");
        downloader->state = HTTP_DOWNLOADER_STATE_IDLE;
        osThreadExit();
    }
    ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, http_downloader_net_connected, P_INFO,
                  "connect HTTP server successfully");
    downloader->write_size = downloader->start_pos;
    downloader->section_pos = downloader->start_pos;
    downloader->timer_data =
        osTimerNew(timer_data_call_back, osTimerPeriodic, downloader, NULL);
    osTimerStart(downloader->timer_data, 20);

    while(1)
    {
        uint8_t* frame_addr = NULL;
        ret = rbuffer_lock_back_free_frame(downloader->rbuff, &frame_addr,
                                           downloader->section_size);
        if(ret != 0)
        {
            if(!downloader->stop_flag)
            {
                // 没有空闲的数据缓冲区，暂时等待
                // ECPLAT_PRINTF(UNILOG_PLAT_MEDIA,
                //              http_downloader_net_no_free_frame, P_INFO,
                //              "no valid data frame");
                osDelay(20);
                continue;
            }
            else
            {
                // 收到停止信号
                ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, http_downloader_net_stop_flag,
                              P_INFO, "stop exit");
                break;
            }
        }

        // 发送本次分段请求
        uint32_t this_section = 0;
        http_data.headerBuf = head_buffer;
        http_data.headerBufLen = HTTP_HEAD_BUF_SIZE;
        http_data.respBuf = data_buffer;
        http_data.respBufLen = HTTP_RECV_BUF_SIZE;
        http_data.isRange = true;
        http_data.rangeHead = download_pos;
        if(downloader->total_size == 0)
        {
            http_data.rangeTail = download_pos + downloader->section_size - 1;
        }
        else
        {
            uint32_t request_size =
                ((download_pos + downloader->section_size) >
                 downloader->total_size)
                    ? (downloader->total_size - download_pos)
                    : downloader->section_size;
            http_data.rangeTail = download_pos + request_size - 1;
        }
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, http_downloader_request_range, P_INFO,
                      "HTTP request range: %d~%d", http_data.rangeHead,
                      http_data.rangeTail);
        http_result =
            httpSendRequest(&downloader->client, (const char*)downloader->url,
                            HTTP_GET, &http_data);
        if(http_result != HTTP_OK)
        {
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA,
                          http_downloader_request_data_failed, P_INFO,
                          "HTTP request data failed");
            break;
        }
        do
        {
            http_result = httpRecvResponse(&downloader->client, &http_data);
            if((http_result == HTTP_OK) || (http_result == HTTP_MOREDATA))
            {
                if(http_data.blockContentLen > 0)
                {
                    memcpy(frame_addr + this_section, http_data.respBuf,
                           http_data.blockContentLen);
                    this_section += http_data.blockContentLen;
                    if(this_section >= downloader->section_size)
                    {
                        break;
                    }
                }
                if(downloader->total_size == 0)
                {
                    if(downloader->client.httpResponseCode == 206)
                    {
                        // 支持分段请求从content range字段获取总长度
                        downloader->total_size = http_data.contentRange;
                    }
                    else if(downloader->client.httpResponseCode == 200)
                    {
                        // 不支持分段请求从获取内容的长度获取总长度
                        downloader->total_size = http_data.recvContentLength;
                    }
                    else
                    {
                        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA,
                                      http_downloader_request_get_size_failed,
                                      P_INFO, "HTTP request data failed");
                        break;
                    }
                    ECPLAT_PRINTF(UNILOG_PLAT_MEDIA,
                                  http_downloader_request_get_size, P_INFO,
                                  "file total size is %d",
                                  downloader->total_size);
                }
            }
        } while((http_result == HTTP_MOREDATA || http_result == HTTP_CONN) &&
                (!downloader->error_flag));

        if(((http_result != HTTP_MOREDATA) && (http_result != HTTP_CONN) &&
            (http_result != HTTP_OK)))
        {
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, http_downloader_http_error, P_INFO,
                          "HTTP download error, code: %d, http_result: %d",
                          downloader->client.httpResponseCode, http_result);
            rbuffer_unlock_back_free_frame(downloader->rbuff);
            downloader->error_flag = true;
            break;
        }
        // 206代表服务器支持分段请求，200和其他代表不支持分段请求
        if(downloader->client.httpResponseCode == 206)
        {
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, http_downloader_range_support,
                          P_INFO, "HTTP range download is supported, code: %d",
                          downloader->client.httpResponseCode);
        }
        else if(downloader->client.httpResponseCode == 200)
        {
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, http_downloader_range_unsupport,
                          P_INFO,
                          "HTTP range download is unsupported, code: %d",
                          downloader->client.httpResponseCode);
        }
        else if((downloader->client.httpResponseCode < 200) ||
                (downloader->client.httpResponseCode >= 400))
        {
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, http_downloader_http_error, P_INFO,
                          "HTTP download error, code: %d, http_result: %d",
                          downloader->client.httpResponseCode, http_result);
            rbuffer_unlock_back_free_frame(downloader->rbuff);
            downloader->error_flag = true;
            break;
        }
        downloader->section_pos += this_section;
        rbuffer_unlock_and_push_back(downloader->rbuff, this_section);
        download_pos += this_section;
        download_size += this_section;
        if(downloader->start_pos + download_size >= downloader->total_size)
        {
            break;
        }
        if(downloader->stop_flag)
        {
            break;
        }
    }

    int timeout_cnt = 500;
    if(!downloader->error_flag)
    {
        // 此处等待定时器内数据全部被写完
        while((!downloader->stop_flag) && (timeout_cnt != 0))
        {
            if(downloader->write_size >= downloader->total_size)
            {
                ECPLAT_PRINTF(UNILOG_PLAT_MEDIA,
                              http_downloader_request_play_over, P_INFO,
                              "data write over!");
                break;
            }
            else
            {
                ECPLAT_PRINTF(UNILOG_PLAT_MEDIA,
                              http_downloader_request_wait_playing, P_INFO,
                              "wait for play all data!");
                osDelay(10);
                timeout_cnt--;
            }
        }
    }

    httpClose(&downloader->client);
    info.cur_pos = downloader->section_pos;
    info.total_pos = downloader->total_size;
    if(downloader->stop_flag)
    {
        info.status = HTTP_DOWNLOADER_STATUS_STOPPED;
        ret = downloader->cb(&info, NULL, 0, downloader->user_data);
    }
    else if(downloader->error_flag)
    {
        info.status = HTTP_DOWNLOADER_STATUS_ERROR;
        ret = downloader->cb(&info, NULL, 0, downloader->user_data);
    }
    else
    {
        info.status = HTTP_DOWNLOADER_STATUS_COMPLETED;
        ret = downloader->cb(&info, NULL, 0, downloader->user_data);
    }

    if(data_buffer)
    {
        free(data_buffer);
    }
    if(head_buffer)
    {
        free(head_buffer);
    }
    if(osTimerIsRunning(downloader->timer_data))
    {
        osTimerStop(downloader->timer_data);
    }
    osTimerDelete(downloader->timer_data);
    downloader->timer_data = NULL;
    downloader->state = HTTP_DOWNLOADER_STATE_IDLE;
    ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, http_downloader_request_exit, P_INFO,
                  "download thread exit!");
    osThreadExit();
}

HttpDownloader_t* http_downloader_create(HttpDownloaderCfg_t* cfg)
{
    if(!cfg)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, http_downloader_create_cfg_failed,
                      P_INFO, "downloader cfg is NULL");
        return NULL;
    }

    if(!cfg->url)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, http_downloader_create_url_failed,
                      P_INFO, "downloader url is NULL");
        return NULL;
    }

    HttpDownloader_t* downloader = malloc(sizeof(HttpDownloader_t));
    if(downloader == NULL)
    {
        return NULL;
    }
    memset(downloader, 0, sizeof(HttpDownloader_t));
    uint32_t url_len = strlen(cfg->url) + 1;
    downloader->url = malloc(url_len);
    memset(downloader->url, 0, url_len);
    strcpy(downloader->url, cfg->url);
    downloader->start_pos = cfg->start_pos;
    downloader->section_size = cfg->section_cnt * (HTTP_RECV_BUF_SIZE - 1);
    downloader->cb = cfg->cb;
    downloader->user_data = cfg->user_data;

    downloader->rbuff =
        rbuffer_create(NULL, downloader->section_size * DOWNLOADER_BUFFER_COUNT,
                       DOWNLOADER_BUFFER_COUNT, RQ_LOCK_TYPE_MUTEX);
    if(!downloader->rbuff)
    {
        free(downloader->url);
        free(downloader);
        return NULL;
    }
    return downloader;
}

int http_downloader_start(HttpDownloader_t* downloader)
{
    if(!downloader)
    {
        return -1;
    }
    if(downloader->state != HTTP_DOWNLOADER_STATE_IDLE)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, http_downloader_start, P_INFO,
                      "downloader is running\n");
        return -1;
    }
    osThreadAttr_t threadAttr = {0};
    memset(&threadAttr, 0, sizeof(threadAttr));
    threadAttr.name = "au_dl";
    threadAttr.stack_size = THREAD_STACK_SIZE_DOWNLOADER;
    threadAttr.priority = osPriorityBelowNormal6;
    downloader->thread_id =
        osThreadNew(http_downloader_thread, downloader, &threadAttr);
    if(downloader->thread_id == NULL)
    {
        free(downloader);
        return -1;
    }
    return 0;
}

int http_downloader_stop(HttpDownloader_t* downloader)
{
    if(!downloader)
    {
        return -1;
    }
    if(downloader->state != HTTP_DOWNLOADER_STATE_RUNNING)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, http_downloader_stop, P_INFO,
                      "downloader is not running\n");
        return -1;
    }
    downloader->stop_flag = true;
    return 0;
}

int http_downloader_destroy(HttpDownloader_t* downloader)
{
    if(!downloader)
    {
        return -1;
    }
    if((downloader->state != HTTP_DOWNLOADER_STATE_RUNNING) &&
       (downloader->state != HTTP_DOWNLOADER_STATE_IDLE))
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, http_downloader_destroy, P_INFO,
                      "downloader is not running or idle\n");
        return -1;
    }
    int32_t time_count = 375;
    if(downloader->state == HTTP_DOWNLOADER_STATE_RUNNING)
    {
        while((osThreadGetState(downloader->thread_id) != osThreadTerminated) &&
              (time_count > 0))
        {
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, http_downloader_destroy_wait,
                          P_INFO, "wait for downloader terminated");
            osDelay(40);
            time_count--;
        }
    }

    if(downloader->timer_data)
    {
        if(osTimerIsRunning(downloader->timer_data))
        {
            osTimerStop(downloader->timer_data);
        }
        osTimerDelete(downloader->timer_data);
        downloader->timer_data = NULL;
    }

    if(downloader->rbuff)
    {
        rbuffer_close(downloader->rbuff);
    }
    if(downloader->url)
    {
        free(downloader->url);
    }
    free(downloader);
    return 0;
}
