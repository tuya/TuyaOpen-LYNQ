#ifndef __HTTP_DOWNLOADER_H__
#define __HTTP_DOWNLOADER_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <stdbool.h>
#include "HTTPClient.h"
#include "rbuffer.h"

#define THREAD_STACK_SIZE_DOWNLOADER (16 * 1024)
#define DOWNLOADER_CMD_QUEUE_LENGTH (4)
#define DOWNLOADER_BUFFER_COUNT (4)

typedef enum HttpDownloaderState_
{
    HTTP_DOWNLOADER_STATE_IDLE = 0,
    HTTP_DOWNLOADER_STATE_RUNNING,
} HttpDownloaderState_t;

typedef enum HttpDownloaderStatus_
{
    HTTP_DOWNLOADER_STATUS_IDLE = 0,
    HTTP_DOWNLOADER_STATUS_DOWNLOADING,
    HTTP_DOWNLOADER_STATUS_PAUSED,
    HTTP_DOWNLOADER_STATUS_STOPPED,
    HTTP_DOWNLOADER_STATUS_COMPLETED,
    HTTP_DOWNLOADER_STATUS_ERROR,
} HttpDownloaderStatus_t;

typedef struct DownloadInfo_
{
    HttpDownloaderStatus_t status;
    uint32_t cur_pos;
    uint32_t total_pos;
} DownloadInfo_t;

typedef int (*HttpDownloaderCb)(DownloadInfo_t* info, uint8_t* data,
                                 uint32_t len, void* userData);

typedef struct HttpDownloaderCfg_
{
    char* url;
    uint32_t start_pos;
    HttpDownloaderCb cb;
    void* user_data;
    uint32_t section_cnt;
} HttpDownloaderCfg_t;

typedef struct HttpDownloader_
{
    osThreadId_t thread_id;
    osTimerId_t timer_data;
    HttpDownloaderState_t state;
    HttpClientContext client;
    RBuffer_t* rbuff;
    char* url;
    uint32_t start_pos;
    uint32_t section_size;
    HttpDownloaderCb cb;
    void* user_data;
    bool stop_flag;
    uint32_t section_pos;
    uint32_t total_size;
    uint32_t write_size;
    bool error_flag;
} HttpDownloader_t;

HttpDownloader_t* http_downloader_create(HttpDownloaderCfg_t* cfg);
int http_downloader_start(HttpDownloader_t* downloader);
int http_downloader_stop(HttpDownloader_t* downloader);
int http_downloader_destroy(HttpDownloader_t* downloader);

#ifdef __cplusplus
}
#endif

#endif
