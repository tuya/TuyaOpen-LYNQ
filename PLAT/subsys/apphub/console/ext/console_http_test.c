/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of EigenComm Ltd.
 * File name:    console_http_test.c
 * Description:  EC718
 * History:      Rev1.0   2024-09-23
 *
 ****************************************************************************/

#ifdef FEATURE_SUBSYS_CONSOLE_ENABLE

#include "rtthread.h"
#include "open_httpc.h"
#include "http_downloader.h"
#include "storage.h"
#include "rbuffer.h"
extern int skip_atoi(const char **s);

void httpRteCb(int8_t result, int8_t dataType, const void *extra)
{
    // rt_kprintf("\r\n");
    switch(dataType)
    {
        case HTTPC_RTEDATA_IDLE:
            rt_kprintlnf("HTTP CONNECT result : %d", result);
            break;
        case HTTPC_RTEDATA_HEADER:
            rt_kprintlnf("HTTP RSP HEAD : %s", (int8_t *)extra);
            break;
        case HTTPC_RTEDATA_CONTEXT:
            rt_kprintlnf("HTTP RSP DATA : %s", (int8_t *)extra);
            break;
        default:
            break;
    }
}

typedef enum DlState_
{
    DLSTATE_NOT_STARTED = -1,
    DLSTATE_DOWNLOADING = 0,
    DLSTATE_COMPLETED = 1,
    DLSTATE_ERROR = 2,
} DlState_e;
HttpDownloader_t *gFileDownloader = NULL;
char gHttpDownloaderSavePath[64] = {0};
uint32_t gHttpDownloadPos = 0;
osThreadId_t threadSave = NULL;
bool gHttpDownloadPause = false;
RBuffer_t *gFileWriteRBuffer = NULL;
DlState_e gFileDownloadOver = DLSTATE_NOT_STARTED;
HttpDownloaderCfg_t gDownloadCfg = {0};
char *gHttpDownloaderUrl = NULL;
bool gSaveExit = false;
int http_downloader_cb(DownloadInfo_t *info, uint8_t *data, uint32_t len,
                       void *userData)
{
    int ret = 0;
    uint8_t *frame_addr = NULL;
    if(info->status == HTTP_DOWNLOADER_STATUS_DOWNLOADING)
    {
        printf("downloading(%d%%) %d|%d\r\n",
               info->cur_pos * 100 / info->total_pos, info->cur_pos,
               info->total_pos);
        if(gFileWriteRBuffer)
        {
            ret = rbuffer_lock_back_free_frame(gFileWriteRBuffer, &frame_addr,
                                               len);
            if(ret != 0)
            {
                printf("no frame buffer\r\n");
                return -1;
            }
            memcpy(frame_addr, data, len);
            rbuffer_unlock_and_push_back(gFileWriteRBuffer, len);
        }
    }
    else if(info->status == HTTP_DOWNLOADER_STATUS_ERROR)
    {
        gFileDownloadOver = DLSTATE_ERROR;
        gSaveExit = true;
        printf("download error\r\n");
    }
    else if(info->status == HTTP_DOWNLOADER_STATUS_STOPPED)
    {
        gFileDownloadOver = DLSTATE_COMPLETED;
        if(!gHttpDownloadPause)
        {
            gHttpDownloadPos = 0;
        }
        gSaveExit = true;
        printf("download stop\r\n");
    }
    else if(info->status == HTTP_DOWNLOADER_STATUS_COMPLETED)
    {
        gHttpDownloadPos = 0;
        gSaveExit = true;
        gFileDownloadOver = DLSTATE_COMPLETED;
        printf("download completed\r\n");
    }
    return 0;
}

static void thread_http_download_save(void *argument)
{
    HttpDownloader_t *downloader = (HttpDownloader_t *)argument;
    FILE *file_dl = NULL;
    int ret = 0;
    gFileWriteRBuffer = rbuffer_create(NULL, 64 * 1024, 10, RQ_LOCK_TYPE_MUTEX);
    file_dl = file_fopen(gHttpDownloaderSavePath, "wb");
    if(!file_dl)
    {
        printf("file_fopen %s fail\r\n", gHttpDownloaderSavePath);
        return;
    }
    while(!gSaveExit)
    {
        uint8_t *frame_addr = NULL;
        uint32_t frame_size = 0;
        ret = rbuffer_lock_front_data_frame(gFileWriteRBuffer, &frame_addr,
                                            &frame_size, 1000);
        if(ret != 0)
        {
            continue;
        }
        file_fwrite(frame_addr, frame_size, 1, file_dl);
        rbuffer_unlock_and_pop_front(gFileWriteRBuffer);
    }
    file_fclose(file_dl);
    while((osThreadGetState(downloader->thread_id) != osThreadTerminated))
    {
        osDelay(40);
    }
    rbuffer_close(gFileWriteRBuffer);
    http_downloader_destroy(downloader);
    printf("thread_http_download_save exit\r\n");
    osThreadExit();
}

int cmd_http(int argc, char **argv)
{
    char *sub_cmd = argv[1];
    // http get http://xxx.xxx.xxx.xxx/get
    if(strcmp(sub_cmd, "get") == 0)
    {
        int8_t *url = argv[2];
        openHttpcGet(url, NULL, 0, httpRteCb);
    }
    // http post http://xxx.xxx.xxx.xxx/get xxxxx
    else if(strcmp(sub_cmd, "post") == 0)
    {
        int8_t *url = argv[1];
        int8_t *postData = argv[2];
        openHttpcPost(url, NULL, 0, postData, strlen(postData), httpRteCb);
    }
    else if(strcmp(sub_cmd, "wget") == 0)
    {
        if(argc != 4)
        {
            rt_kprintlnf("Usage: http dl [url] [savePath]");
            return -1;
        }
        char *url = argv[2];
        char *savePath = argv[3];
        memset(gHttpDownloaderSavePath, 0, 64);
        strcpy(gHttpDownloaderSavePath, savePath);
        if(!gHttpDownloaderUrl)
        {
            gHttpDownloaderUrl = malloc(strlen(url) + 1);
            memset(gHttpDownloaderUrl, 0, strlen(url) + 1);
            strcpy(gHttpDownloaderUrl, url);
        }
        else {
            if(strcmp(gHttpDownloaderUrl, url) != 0)
            {
                free(gHttpDownloaderUrl);
                gHttpDownloaderUrl = malloc(strlen(url) + 1);
                memset(gHttpDownloaderUrl, 0, strlen(url) + 1);
                strcpy(gHttpDownloaderUrl, url);
            }
        }
        gDownloadCfg.cb = http_downloader_cb;
        gDownloadCfg.section_cnt = 3;
        gDownloadCfg.url = url;
        gFileDownloader = http_downloader_create(&gDownloadCfg);
        http_downloader_start(gFileDownloader);
        gFileDownloadOver = DLSTATE_DOWNLOADING;
        gSaveExit = false;
        osThreadAttr_t threadAttr = {0};
        memset(&threadAttr, 0, sizeof(threadAttr));
        threadAttr.name = "dl_fwrite";
        threadAttr.stack_size = 16 * 1024;
        threadAttr.priority = osPriorityBelowNormal6;
        threadSave = osThreadNew(thread_http_download_save, gFileDownloader,
                                 &threadAttr);
    }
    else
    {
        rt_kprintlnf("Usage: http [options]");
        rt_kprintlnf("[options]:");
        rt_kprintlnf("    %-10s - http get request", "get");
        rt_kprintlnf("    %-10s - http post request", "post");
    }
    return 0;
}
MSH_CMD_EXPORT_ALIAS(cmd_http, http, http test);

#endif
