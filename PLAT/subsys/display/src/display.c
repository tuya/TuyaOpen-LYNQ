#include "display.h"
#include "api_scr.h"
#include "rbuffer.h"
#include "rqueue.h"
#include DEBUG_LOG_HEADER_FILE

#define SUBSYS_DISPLAY_TASK_STACK_SIZE (1024)
#define DISPLAY_MSG_QUEUE_SIZE (10)
StaticTask_t subsys_display_task;
__ALIGNED(8) uint8_t subsys_display_task_stack[SUBSYS_DISPLAY_TASK_STACK_SIZE];
#define DISPLAY_FRAME_BUFFER_SIZE (LCD_WIDTH * LCD_HEIGHT * 5)
#ifdef FEATURE_SUBSYS_POWER_MANAGER_ENABLE
#include "powermanager.h"
uint8_t prohibitionHandle = 0xFF;
#endif
typedef enum SubDisPlayMsgId_
{
    DISPLAY_MSG_ID_START,
    DISPLAY_MSG_ID_STOP,
    DISPLAY_SET_BACKLIGHT,
    DISPLAY_MSG_ID_ADD_FRAME,
    DISPLAY_MSG_ID_REFRESH_FRAME,
    DISPLAY_MSG_ID_PIXMODE,
    DISPLAY_MSG_ID_NUM
} SubDisplayMsgId_e;

typedef struct DisplayCfgItem_
{
    ScreenType_e type;
    ScrConfig_t cfg;
} DisplayCfgItem_t;

typedef struct SubDisplayCtx_
{
    DisplayStatus_e status;
    uint32_t usr_id;
    osMessageQueueId_t msg_queue;
    osMessageQueueId_t msg_resp_queue;
    RBuffer_t* frame_buffer;
    uint32_t cfg_list_cnt;
    DisplayCfgItem_t cfg_list[SCREEN_TYPE_TOTAL];
    display_transfer_done_cb transfer_done_cb;
    ScrConfig_t cur_cfg;
} SubDisplayCtx_t;

typedef struct SubDisplayMsg_
{
    SubDisplayMsgId_e id;
    uint32_t len;
    uint8_t* data;
} SubDisplayMsg_t;

SubDisplayCtx_t* get_subdisplay_ctx(void)
{
    static SubDisplayCtx_t* local_ctx = NULL;
    if(local_ctx == NULL)
    {
        local_ctx = malloc(sizeof(SubDisplayCtx_t));
        memset(local_ctx, 0, sizeof(SubDisplayCtx_t));
    }
    return local_ctx;
}

void displayInit()
{
    SubDisplayCtx_t* ctx = get_subdisplay_ctx();
    if(ctx->msg_queue == NULL)
    {
        ctx->msg_queue = osMessageQueueNew(DISPLAY_MSG_QUEUE_SIZE,
                                           sizeof(SubDisplayMsg_t), NULL);
    }
    if(ctx->msg_resp_queue == NULL)
    {
        ctx->msg_resp_queue = osMessageQueueNew(DISPLAY_MSG_QUEUE_SIZE,
                                                sizeof(SubDisplayMsg_t), NULL);
    }
    ctx->frame_buffer =
        rbuffer_create(NULL, DISPLAY_FRAME_BUFFER_SIZE, 2, RQ_LOCK_TYPE_MUTEX);
    if(ctx->frame_buffer == NULL)
    {
        printf("create frame buffer failed\r\n");
    }
}

static void usrLspiHandler(uint32_t state)
{
    SubDisplayCtx_t* ctx = get_subdisplay_ctx();
    SubDisplayMsg_t msg = {0};
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, usrLspiHandler_enter, P_INFO,
                  "usrLspiHandler enter");
    if(ctx->status != DISPLAY_STATUS_STARTED)
    {
        return;
    }
    rbuffer_unlock_and_pop_front(ctx->frame_buffer);
    msg.id = DISPLAY_MSG_ID_REFRESH_FRAME;
    osMessageQueuePut(ctx->msg_queue, &msg, 0, 0);
}

static ScrConfig_t* sub_display_get_cfg(SubDisplayCtx_t* ctx, ScreenType_e type)
{
    if(ctx->cfg_list_cnt == 0)
    {
        return NULL;
    }
    if(type == -1)
    {
        return &ctx->cfg_list[0].cfg;
    }
    for(uint32_t i = 0; i < ctx->cfg_list_cnt; i++)
    {
        if(ctx->cfg_list[i].type == type)
        {
            return &ctx->cfg_list[i].cfg;
        }
    }
    return NULL;
}

void sub_display_start(ScreenType_e type, display_transfer_done_cb cb)
{
    SubDisplayCtx_t* ctx = get_subdisplay_ctx();
    ScrConfig_t* usr_cfg = sub_display_get_cfg(ctx, type);
    if(usr_cfg == NULL)
    {
        // 没有用户配置则使用默认配置
        api_scr_default(type, &ctx->cur_cfg);
        printf("use default screen configuration\r\n");
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, sub_display_start_default, P_INFO,
                      "use default screen configuration");
    }
    else
    {
        memcpy(&ctx->cur_cfg, usr_cfg, sizeof(ScrConfig_t));
        printf("use custom screen configuration\r\n");
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, sub_display_start_custom, P_INFO,
                      "use custom screen configuration");
    }
    api_scr_default(type, &ctx->cur_cfg);
    ctx->transfer_done_cb = cb;
    api_scr_create(0, NULL, &ctx->usr_id);
    api_scr_open(ctx->usr_id, &ctx->cur_cfg, 0);
    api_scr_ioctl(ctx->usr_id, OPEN_SCREEN_SET_USP_CALLBACK, usrLspiHandler);
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, sub_display_start, P_INFO,
                  "screen init type: %d, cb: %p", type, ctx->transfer_done_cb);
    uint8_t level = 100;
    api_scr_ioctl(ctx->usr_id, OPEN_SCREEN_BACKLIGHT_SET, &level);
    ctx->status = DISPLAY_STATUS_STARTED;
}

void sub_display_stop(void)
{
    SubDisplayCtx_t* ctx = get_subdisplay_ctx();
    api_scr_close(ctx->usr_id);
    api_scr_delete(ctx->usr_id);
    ctx->status = DISPLAY_STATUS_STOPPED;
#ifdef FEATURE_SUBSYS_POWER_MANAGER_ENABLE
    powerManagerProhibitionSet(prohibitionHandle, PROHIBIT_NOTHING);
#endif
}

void sub_display_set_backlight(uint8_t backlight)
{
    SubDisplayCtx_t* ctx = get_subdisplay_ctx();
    uint8_t level = backlight;
    api_scr_ioctl(ctx->usr_id, OPEN_SCREEN_BACKLIGHT_SET, &level);
}

void sub_display_set_pixmode(uint8_t mode)
{
    SubDisplayCtx_t* ctx = get_subdisplay_ctx();
    uint8_t pixmode = mode;
    api_scr_ioctl(ctx->usr_id, OPEN_SCREEN_SET_DISPLAY_PIX_MODE, &pixmode);
}

void tsk_subsys_display(void* arg)
{
    int ret = 0;
    SubDisplayMsg_t recv_msg = {0};
    ScrWriteParam_t fill_info = {0};
    bool frame_pending = false;
    bool stop_pending = false;
    uint8_t* frame_addr = NULL;
    uint32_t frame_size = 0;

    SubDisplayCtx_t* ctx = get_subdisplay_ctx();
#ifdef FEATURE_SUBSYS_POWER_MANAGER_ENABLE
    uint8_t prohibitionHandle = 0xFF;
#endif
#ifdef FEATURE_SUBSYS_POWER_MANAGER_ENABLE
    powerManagerProhibitionCreate(&prohibitionHandle);
    printf("prohibitionHandle=%d\r\n", prohibitionHandle);
#endif
    displayInit();
    printf("\r\ndisplay task start\r\n");
    while(1)
    {
        memset(&recv_msg, 0, sizeof(recv_msg));
        ret = osMessageQueueGet(ctx->msg_queue, &recv_msg, NULL, osWaitForever);
        if(ret != osOK)
        {
            printf("osMessageQueueGet failed, ret=%d\r\n", ret);
            continue;
        }
        switch(recv_msg.id)
        {
            case DISPLAY_MSG_ID_START: {
                ScreenType_e type = *(ScreenType_e*)recv_msg.data;
                display_transfer_done_cb cb =
                    *(display_transfer_done_cb*)(recv_msg.data +
                                                 sizeof(ScreenType_e));
                sub_display_start(type, cb);
                break;
            }
            case DISPLAY_MSG_ID_STOP: {
                if(!frame_pending)
                {
                    sub_display_stop();
                    SubDisplayMsg_t resp_msg = {0};
                    resp_msg.id = DISPLAY_MSG_ID_STOP;
                    osMessageQueuePut(ctx->msg_resp_queue, &resp_msg, 0, 100);
                }
                else
                {
                    stop_pending = true;
                }
                break;
            }
            case DISPLAY_SET_BACKLIGHT: {
                uint8_t backlight = *(uint8_t*)recv_msg.data;
                sub_display_set_backlight(backlight);
                break;
            }
            case DISPLAY_MSG_ID_ADD_FRAME: {
                if(!frame_pending)
                {
                    int ret = rbuffer_lock_front_data_frame(
                        ctx->frame_buffer, &frame_addr, &frame_size, 10);
                    if(ret == 0)
                    {
                        DisplayRegion_t* disp_region =
                            (DisplayRegion_t*)frame_addr;
                        fill_info.start_x = disp_region->x;
                        fill_info.start_y = disp_region->y;
                        fill_info.width = disp_region->width;
                        fill_info.height = disp_region->height;
                        fill_info.data = frame_addr + sizeof(DisplayRegion_t);
                        fill_info.size =
                            disp_region->width * disp_region->height * 2;
                        fill_info.fmt = OPEN_SCREEN_DATA_FMT_RGB565;
                        api_scr_write(ctx->usr_id, &fill_info,
                                      sizeof(ScrWriteParam_t));
                        frame_pending = true;
                    }
                }
                if(ctx->transfer_done_cb != NULL)
                {
                    ctx->transfer_done_cb(0);
                }
                break;
            }
            case DISPLAY_MSG_ID_REFRESH_FRAME: {
                if(stop_pending)
                {
                    sub_display_stop();
                    stop_pending = false;
                    rbuffer_reset(ctx->frame_buffer);
                    SubDisplayMsg_t resp_msg = {0};
                    resp_msg.id = DISPLAY_MSG_ID_STOP;
                    osMessageQueuePut(ctx->msg_resp_queue, &resp_msg, 0, 100);
                    break;
                }
                int ret = rbuffer_lock_front_data_frame(
                    ctx->frame_buffer, &frame_addr, &frame_size, 10);
                if(ret == 0)
                {
                    DisplayRegion_t* disp_region = (DisplayRegion_t*)frame_addr;
                    fill_info.start_x = disp_region->x;
                    fill_info.start_y = disp_region->y;
                    fill_info.width = disp_region->width;
                    fill_info.height = disp_region->height;
                    fill_info.data = frame_addr + sizeof(DisplayRegion_t);
                    fill_info.size =
                        disp_region->width * disp_region->height * 2;
                    fill_info.fmt = OPEN_SCREEN_DATA_FMT_RGB565;
                    api_scr_write(ctx->usr_id, &fill_info,
                                  sizeof(ScrWriteParam_t));
                }
                else
                {
                    frame_pending = false;
                }
                break;
            }
            case DISPLAY_MSG_ID_PIXMODE: {
                uint8_t pixmode = *(uint8_t*)recv_msg.data;
                sub_display_set_pixmode(pixmode);
                break;
            }
            default:
                break;
        }
        if(recv_msg.data != NULL)
        {
            free(recv_msg.data);
        }
    }
}

void subDisplayInit(void)
{
    osThreadAttr_t taskAttr;
    memset(&taskAttr, 0, sizeof(taskAttr));
    memset(subsys_display_task_stack, 0xA5, SUBSYS_DISPLAY_TASK_STACK_SIZE);
    taskAttr.name = "subsys_display";
    taskAttr.stack_mem = subsys_display_task_stack;
    taskAttr.stack_size = SUBSYS_DISPLAY_TASK_STACK_SIZE;
    taskAttr.priority = osPriorityNormal;
    taskAttr.cb_mem = &subsys_display_task;   // task control block
    taskAttr.cb_size = sizeof(StaticTask_t);  // size of task control block
    osThreadNew(tsk_subsys_display, NULL, &taskAttr);
    printf("subDisplayInit finish!\r\n");
}

void displayStart(ScreenType_e type, display_transfer_done_cb cb)
{
    SubDisplayMsg_t msg = {0};
    SubDisplayCtx_t* ctx = get_subdisplay_ctx();
    if((ctx->status != DISPLAY_STATUS_IDLE) &&
       (ctx->status != DISPLAY_STATUS_STOPPED))
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, displayStart_failed_0, P_INFO,
                      "display has been started");
        return;
    }
    printf("displayStart \r\n", ctx->status);
    msg.id = DISPLAY_MSG_ID_START;
    msg.data = malloc(sizeof(display_transfer_done_cb) + sizeof(ScreenType_e));
    *(ScreenType_e*)msg.data = type;
    *(display_transfer_done_cb*)(msg.data + sizeof(ScreenType_e)) = cb;
    ctx->status = DISPLAY_STATUS_STARTING;
    osMessageQueuePut(ctx->msg_queue, &msg, 0, osWaitForever);
}

int displayStop(void)
{
    SubDisplayMsg_t msg = {0};
    SubDisplayMsg_t recv_msg = {0};
    SubDisplayCtx_t* ctx = get_subdisplay_ctx();
    if(ctx->status != DISPLAY_STATUS_STARTED)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, displayStop_failed_0, P_INFO,
                      "display has not been started");
        printf("displayStop failed, no started, status: %d\r\n", ctx->status);
        return -1;
    }
    msg.id = DISPLAY_MSG_ID_STOP;
    ctx->status = DISPLAY_STATUS_STOPPING;
    int ret = osMessageQueuePut(ctx->msg_queue, &msg, 0, osWaitForever);
    ret = osMessageQueueGet(ctx->msg_resp_queue, &recv_msg, NULL, 1000);
    if(ret != osOK)
    {
        return -1;
    }
    if(recv_msg.id != DISPLAY_MSG_ID_STOP)
    {
        return -1;
    }
    return 0;
}

void displaySetBacklight(uint8_t backlight)
{
    SubDisplayMsg_t msg = {0};
    SubDisplayCtx_t* ctx = get_subdisplay_ctx();
    if(ctx->status != DISPLAY_STATUS_STARTED)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, displaySetBacklight_failed_0, P_INFO,
                      "display has not been started");
        return;
    }
    msg.id = DISPLAY_SET_BACKLIGHT;
    msg.data = malloc(sizeof(uint8_t));
    *(uint8_t*)msg.data = backlight;
    osMessageQueuePut(ctx->msg_queue, &msg, 0, osWaitForever);
}

int displayAddCfg(ScreenType_e type, ScrConfig_t* cfg)
{
    SubDisplayCtx_t* ctx = get_subdisplay_ctx();
    if((type >= SCREEN_TYPE_TOTAL) || (cfg == NULL))
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, displayAddCfg_failed_0, P_INFO,
                      "pamameter is error, type: %d, cfg: %p", type, cfg);
        return -1;
    }
    for(uint32_t i = 0; i < ctx->cfg_list_cnt; i++)
    {
        if(ctx->cfg_list[i].type == type)
        {
            memcpy(&ctx->cfg_list[i].cfg, cfg, sizeof(ScrConfig_t));
            return 0;
        }
    }
    ctx->cfg_list[ctx->cfg_list_cnt].type = type;
    memcpy(&ctx->cfg_list[ctx->cfg_list_cnt].cfg, cfg, sizeof(ScrConfig_t));
    ctx->cfg_list_cnt++;
    return 0;
}

int displayWriteData(uint8_t* data, uint32_t size, DisplayRegion_t* region)
{
    SubDisplayMsg_t msg = {0};
    SubDisplayCtx_t* ctx = get_subdisplay_ctx();
    int ret = 0;
    uint8_t* frame_addr = NULL;
    uint32_t frame_size = size + sizeof(DisplayRegion_t);
    if(region == NULL)
    {
        return -1;
    }
    if(ctx->status != DISPLAY_STATUS_STARTED)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, displaySetBacklight_failed_0, P_INFO,
                      "display has not been started");
        return -1;
    }
    ret = rbuffer_lock_back_free_frame(ctx->frame_buffer, &frame_addr,
                                       frame_size);
    if(ret != 0)
    {
        return -2;
    }

    memcpy(frame_addr, region, sizeof(DisplayRegion_t));
    memcpy(frame_addr + sizeof(DisplayRegion_t), data, size);
    rbuffer_unlock_and_push_back(ctx->frame_buffer, frame_size);

    msg.id = DISPLAY_MSG_ID_ADD_FRAME;
    osMessageQueuePut(ctx->msg_queue, &msg, 0, osWaitForever);
    return 0;
}

int displaySetPixMode(DisplayPixMode_e mode)
{
    SubDisplayMsg_t msg = {0};
    SubDisplayCtx_t* ctx = get_subdisplay_ctx();
    if(ctx->status != DISPLAY_STATUS_STARTED)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, displaySetPixMode_failed_0, P_INFO,
                      "display has not been started");
        return -1;
    }
    msg.id = DISPLAY_MSG_ID_PIXMODE;
    msg.data = malloc(sizeof(uint8_t));
    *(uint8_t*)msg.data = mode;
    osMessageQueuePut(ctx->msg_queue, &msg, 0, osWaitForever);
    return 0;
}