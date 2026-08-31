#include "camera.h"
#include "hal_screen.h"
#include "api_scr.h"
#include "mode_config.h"
#include "devicemanager.h"
#include DEBUG_LOG_HEADER_FILE
#include "servicemanager.h"

#define CAMERA_MSG_QUEUE_SIZE (10)

typedef enum SubCamMsgId_
{
    CAMERA_MSG_ID_START,
    CAMERA_MSG_ID_STOP,
    CAMERA_MSG_ID_START_PERVIEW,
    CAMERA_MSG_ID_STOP_PERVIEW,
    CAMERA_MSG_ID_GET_FRAME,
    CAMERA_MSG_ID_SET_PARAMS,
    CAMERA_MSG_ID_GET_PARAMS,
    CAMERA_MSG_ID_WR_REG,
    CAMERA_MSG_ID_RD_REG,
    CAMERA_MSG_ID_NUM
} SubCamMsgId_e;

typedef struct SubCamMsg_
{
    SubCamMsgId_e id;
    uint32_t len;
    uint8_t* data;
} SubCamMsg_t;

typedef struct CamCfgItem_
{
    CamType_e type;
    CamCfg_t cfg;
} CamCfgItem_t;

typedef struct SubCamCtx_
{
    CamStatus_e status;
    uint32_t usr_id;
    osMessageQueueId_t msg_queue;
    osMessageQueueId_t msg_resp_queue;
    CamType_e cam_type;
    CamImgInfo_t img_info;
    uint32_t cfg_list_cnt;
    CamCfgItem_t cfg_list[CAM_TYPE_NUM];
    CamCfg_t cur_cfg;
    uint8_t* img_data;
} SubCamCtx_t;

#if 1
int nLen = 0;
int nType;
char acBarcode[2048];
unsigned int Initial_Decoder()
{
    return (unsigned int)CDWiInit("input crt 512B");
}

int Decoding_Image(unsigned char *img_buffer, int width, int height)
{
	nLen = CDWiDecodeBar(width, height, (char *)img_buffer, &nType, acBarcode, sizeof(acBarcode));
    return nLen;
}

unsigned int GetResultLength()
{
	return (unsigned int)nLen;
}

int GetDecoderResult(unsigned char *result)
{
	if (nLen > 0)
    {
        memcpy(result, acBarcode, nLen);
    }
    return nLen;
}

void Get_Decoder_Version(unsigned char *version)
{
	CDWiGetVer((char *)version, 32);
    return;
}
int g98301_is_timeout1(void)
{
    return 0;
}

void g98301_reset_timeout1(void)
{

}

int g98301_is_timeout2(void)
{
    return  0;
}

void g98301_reset_timeout2(void)
{

}

int nvmReadImei(int opt,char *imei)
{
    char    buffer[16] = {"122834584466709"};
    memcpy(buffer,imei,15);
    return 15;
}
#endif

SubCamCtx_t* get_subcam_ctx(void)
{
    static SubCamCtx_t* local_ctx = NULL;
    if(local_ctx == NULL)
    {
        local_ctx = malloc(sizeof(SubCamCtx_t));
        memset(local_ctx, 0, sizeof(SubCamCtx_t));
        local_ctx->cam_type = CAM_TYPE_UNKNWON;
    }
    return local_ctx;
}

int camAddCfg(CamType_e type, CamCfg_t* cfg)
{
    SubCamCtx_t* ctx = get_subcam_ctx();
    if(ctx->cfg_list_cnt >= CAM_TYPE_NUM)
    {
        return -1;
    }
    for(uint32_t i = 0; i < ctx->cfg_list_cnt; i++)
    {
        if(ctx->cfg_list[i].type == type)
        {
            memcpy(&ctx->cfg_list[i].cfg, cfg, sizeof(CamCfg_t));
            return 0;
        }
    }
    ctx->cfg_list[ctx->cfg_list_cnt].type = type;
    memcpy(&ctx->cfg_list[ctx->cfg_list_cnt].cfg, cfg, sizeof(CamCfg_t));
    ctx->cfg_list_cnt++;
    return 0;
}

static CamCfg_t* sub_cam_get_cfg(SubCamCtx_t* ctx, CamType_e type)
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

static OpenCamParamId_e cam_settings_to_open_cam_id(CamSetting_e settings)
{
    OpenCamParamId_e cam_id = 0;
    switch(settings)
    {
        case CAMERA_FPS:
            cam_id = OPEN_CAM_PARAM_ID_FPS;
            break;
        case CAMERA_CMOS_MODEL:
            cam_id = OPEN_CAM_PARAM_ID_CMOS_MODEL;
            break;
        case CAMERA_CMOS_MAX_FPS:
            cam_id = OPEN_CAM_PARAM_ID_CMOS_MAX_FPS;
            break;
        case CAMERA_RESOLUTION:
            cam_id = OPEN_CAM_PARAM_ID_RESOLUTION;
            break;
        case CAMERA_MIRROR_FLIP:
            cam_id = OPEN_CAM_PARAM_ID_MIRROR_FLIP;
            break;
        case CAMERA_SCENE:
            cam_id = OPEN_CAM_PARAM_ID_SCENE;
            break;
        case CAMERA_EV:
            cam_id = OPEN_CAM_PARAM_ID_EV;
            break;
        case CAMERA_CONTRAST:
            cam_id = OPEN_CAM_PARAM_ID_CONTRAST;
            break;
        case CAMERA_SATURATION:
            cam_id = OPEN_CAM_PARAM_ID_SATURATION;
            break;
        case CAMERA_SHARP:
            cam_id = OPEN_CAM_PARAM_ID_SHARP;
            break;
        case CAMERA_AWB:
            cam_id = OPEN_CAM_PARAM_ID_AWB;
            break;
        case CAMERA_AE:
            cam_id = OPEN_CAM_PARAM_ID_AE;
            break;
        default:
            break;
    }
    return cam_id;
}

static int sub_cam_set_param(CamSetting_e settings, int value)
{
    OpenCamParam_t param;
    param.id = cam_settings_to_open_cam_id(settings);
    param.value = value;
     Device_ioctl("dev:/camera",OPEN_CAM_IOCTL_SET_PARAM, &param);
    return 0;
}
static int sub_cam_get_param(CamSetting_e settings, int* value)
{
    if(value == NULL)
    {
        return -1;
    }
    OpenCamParam_t param = {0};
    param.id = cam_settings_to_open_cam_id(settings);
     Device_ioctl("dev:/camera",OPEN_CAM_IOCTL_GET_PARAM, &param);
    *value = param.value;
    return 0;
}

static int sub_cam_write_reg(uint8_t addr, uint8_t value)
{
    CamReg_t reg = {0};
    reg.addr = addr;
    reg.value = value;
     Device_ioctl("dev:/camera",OPEN_CAM_IOCTL_WR_CMOS_REG, &reg);
    return 0;
}

static int sub_cam_read_reg(uint8_t addr, uint8_t* value)
{
    uint8_t data = addr;
    Device_ioctl("dev:/camera", OPEN_CAM_IOCTL_RD_CMOS_REG, &data);
    *value = data;
    return 0;
}

int32_t sub_cam_register_dma_callback(CamCbCfg_t* camCbCfg)
{
    Device_ioctl("dev:/camera",OPEN_CAM_IOCTL_REGISTER_CALLBACK, camCbCfg);
    return 0;
}

void cameraInit()
{
    SubCamCtx_t* ctx = get_subcam_ctx();
    if(ctx->msg_queue == NULL)
    {
        ctx->msg_queue =
            osMessageQueueNew(CAMERA_MSG_QUEUE_SIZE, sizeof(SubCamMsg_t), NULL);
    }
    if(ctx->msg_resp_queue == NULL)
    {
        ctx->msg_resp_queue =
            osMessageQueueNew(CAMERA_MSG_QUEUE_SIZE, sizeof(SubCamMsg_t), NULL);
    }
}

static uint32_t camera_hal_start()
{
    uint32_t usr_id = 0;
    SubCamCtx_t* ctx = get_subcam_ctx();

    GPR_swReset(RST_FCLK_USP1);
    CamCfg_t* usr_cfg = sub_cam_get_cfg(ctx, CAM_TYPE_UNKNWON);
    if(usr_cfg == NULL)
    {
        CamExtPwrCfg_t ext_pwr_cfg = {
            .enable = true, .pad_num = 11, .mux = PAD_MUX_ALT4, .io_num = 16};
        CamRstPinCfg_t rst_pin_cfg = {
            .pad_num = 20, .mux = PAD_MUX_ALT0, .io_num = 5, .reset_level = 1};
        CamI2cCfg_t i2c_cfg = {.i2c_port = 0, .speed = OPEN_I2C_SPEED_100KHZ};
        ctx->cam_type =
            api_cam_probe_type(1, &ext_pwr_cfg, &rst_pin_cfg, &i2c_cfg);
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, cameraInit_get_type_default, P_INFO,
                      "no user cfg, use default cfg");
    }
    else
    {
        ctx->cam_type = api_cam_probe_type(
            1, &usr_cfg->ext_pwr_cfg, &usr_cfg->rst_pin_cfg, &usr_cfg->i2c_cfg);
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, cameraInit_get_type_user, P_INFO,
                      "use user cfg");
    }
    printf("\r\ncamera type is %d\r\n", ctx->cam_type);
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, cameraInit_get_type, P_INFO,
                  "camera type is %d", ctx->cam_type);
    usr_cfg = sub_cam_get_cfg(ctx, ctx->cam_type);
    if(usr_cfg == NULL)
    {
        // 没有用户配置则使用默认配置
        api_cam_default(ctx->cam_type, &ctx->cur_cfg);
        printf("use default camera configuration\r\n");
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, camera_hal_start_default, P_INFO,
                      "use default camera configuration");
    }
    else
    {
        memcpy(&ctx->cur_cfg, usr_cfg, sizeof(CamCfg_t));
        printf("use custom camera configuration\r\n");
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, camera_hal_start_custom, P_INFO,
                      "use custom camera configuration");
    }

    ctx->cam_type = ctx->cam_type;
    int ret = Device_open("dev:/camera", &ctx->cur_cfg, 0, &usr_id);
    if(ret != 0)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, camera_hal_start_failed, P_INFO,
                      "Device_open dev:/camera failed, ret = %d", ret);
        return 0;
    }
    return usr_id;
}

static void camera_hal_stop(uint32_t usr_id) { Device_close("dev:/camera"); }

static void lcd_hal_clear_fifo(uint32_t scr_id)
{
    Device_ioctl_by_index(scr_id, OPEN_SCREEN_RST_CLEAR_FIFO, NULL);
}

static void lcd_hal_set_pixfmt(uint32_t scr_id, uint8_t pixfmt)
{
    uint8_t value = pixfmt;
    Device_ioctl_by_index(scr_id, OPEN_SCREEN_SET_DISPLAY_PIX_MODE, &value);
}

static void lcd_hal_auto_preview(uint32_t scr_id)
{
    lcdIoCtrl_t lcdIo;
    lcdIo.previewModeSel = CAM_PREVIEW_SET_AUTO;
    Device_ioctl_by_index(scr_id, OPEN_SCREEN_SET_LCDIO, &lcdIo);
}

static void lcd_hal_preview(uint32_t scr_id, bool enable, uint32_t width,
                            uint32_t height)
{
    ScrPrevInfo_t prev_info = {0};
    prev_info.enable = enable;
    prev_info.img_width = width;
    prev_info.img_height = height;
    Device_ioctl_by_index(scr_id, OPEN_SCREEN_CAM_PREVIEW, &prev_info);
}

static void cam_hal_cspi2lspi(uint32_t usr_id, bool enable)
{
    uint8_t value = enable ? 1 : 0;
    Device_ioctl("dev:/camera", OPEN_CAM_IOCTL_CSPI2LSPI, &value);
}

static void cam_hal_get_img_info(uint32_t usr_id, CamImgInfo_t* img_info)
{
    Device_ioctl("dev:/camera", OPEN_CAM_IOCTL_ENUM_IMAGE_INFO, img_info);
}

void sub_cam_start(void)
{
    SubCamCtx_t* ctx = get_subcam_ctx();
    ctx->usr_id = camera_hal_start();
    cam_hal_get_img_info(ctx->usr_id, &ctx->img_info);
    ctx->status = CAMERA_STATUS_STARTED;
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, sub_cam_start, P_INFO, "camera started");
}

void sub_cam_stop(void)
{
    SubCamCtx_t* ctx = get_subcam_ctx();
    camera_hal_stop(ctx->usr_id);
    ctx->status = CAMERA_STATUS_STOPPED;
    if(ctx->img_data)
    {
        free(ctx->img_data);
        ctx->img_data = NULL;
    }
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, sub_cam_stop, P_INFO, "camera stoped");
}

static bool is_screen_rotate(uint32_t img_width, uint32_t img_height)
{
    if(((img_width > img_height) && (LCD_WIDTH < LCD_HEIGHT)) ||
       ((img_width < img_height) && (LCD_WIDTH > LCD_HEIGHT)))
    {
        return true;
    }
    return false;
}

static void sub_scr_set_rotate(uint32_t scr_id, bool rotate)
{
    uint8_t value = 0;
    if(rotate)
    {
        LcdDispWin_t disp_win;
        disp_win.start_x = 0;
        disp_win.start_y = 0;
        disp_win.width = LCD_HEIGHT;
        disp_win.height = LCD_WIDTH;
        value = DISPLAY_MODE_SWAP_XY | DISPLAY_MODE_MIRROR_Y;
        Device_ioctl_by_index(scr_id, OPEN_SCREEN_SET_DISPLAY_MODE, &value);
        Device_ioctl_by_index(scr_id, OPEN_SCREEN_SET_LCD_WINDOW, &disp_win);
    }
    else
    {
        Device_ioctl_by_index(scr_id, OPEN_SCREEN_SET_DISPLAY_MODE, &value);
        LcdDispWin_t disp_win;
        disp_win.start_x = 0;
        disp_win.start_y = 0;
        disp_win.width = LCD_WIDTH;
        disp_win.height = LCD_HEIGHT;
        Device_ioctl_by_index(scr_id, OPEN_SCREEN_SET_LCD_WINDOW, &disp_win);
    }
}

void sub_cam_start_preview(uint32_t scr_id)
{
    SubCamCtx_t* ctx = get_subcam_ctx();
    uint32_t img_width = 0;
    uint32_t img_height = 0;
    bool rotate = false;
    lcd_hal_clear_fifo(scr_id);
    lcd_hal_set_pixfmt(scr_id, DISPLAY_PIXMODE_RGB);
    lcd_hal_auto_preview(scr_id);
    ctx->usr_id = camera_hal_start(ctx->cam_type);
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, sub_cam_start_preview_usr_id, P_INFO,
                  "camera user id is %d", ctx->usr_id);
    // 检查分辨率判断是否需要旋转屏幕显示相机图像
    cam_hal_get_img_info(ctx->usr_id, &ctx->img_info);
    img_width = ctx->img_info.sns_width;
    img_height = ctx->img_info.sns_height;
    rotate = is_screen_rotate(img_width, img_height);
    sub_scr_set_rotate(scr_id, rotate);
    lcd_hal_preview(scr_id, true, img_width, img_height);
    cam_hal_cspi2lspi(ctx->usr_id, true);
    ctx->status = CAMERA_STATUS_STARTED;
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, sub_cam_start_preview, P_INFO,
                  "camera preview started");
}

void sub_cam_stop_preview(uint32_t scr_id)
{
    SubCamCtx_t* ctx = get_subcam_ctx();
    cam_hal_cspi2lspi(ctx->usr_id, false);
    camera_hal_stop(ctx->usr_id);
    lcd_hal_clear_fifo(scr_id);
    lcd_hal_preview(scr_id, false, 0, 0);
    ctx->status = CAMERA_STATUS_STOPPED;
#ifdef FEATURE_LCD_RGB_SWAP
    lcd_hal_set_pixfmt(scr_id, DISPLAY_PIXMODE_BGR);
#endif
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, sub_cam_stop_preview, P_INFO,
                  "camera preview stopped");
}

uint8_t decode_data[256] = {0};

void tsk_subsys_camera(void* arg)
{
    int ret = 0;
    SubCamMsg_t recv_msg = {0};
    SubCamCtx_t* ctx = get_subcam_ctx();

#ifdef FEATURE_SUBSYS_POWER_MANAGER_ENABLE
    uint8_t prohibitionHandle = 0xFF;
#endif

#ifdef USE_DECODE_LIB
    Initial_Decoder();
#endif
#ifdef FEATURE_SUBSYS_POWER_MANAGER_ENABLE
    powerManagerProhibitionCreate(&prohibitionHandle);
    printf("prohibitionHandle=%d\r\n", prohibitionHandle);
#endif
    cameraInit();
    printf("camera task start\r\n");
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
            case CAMERA_MSG_ID_START: {
                SubCamMsg_t resp_msg = {0};
                sub_cam_start();
                resp_msg.id = CAMERA_MSG_ID_START;
                osMessageQueuePut(ctx->msg_resp_queue, &resp_msg, 0, 100);
                break;
            }
            case CAMERA_MSG_ID_STOP: {
                SubCamMsg_t resp_msg = {0};
                sub_cam_stop();
                resp_msg.id = CAMERA_MSG_ID_STOP;
                osMessageQueuePut(ctx->msg_resp_queue, &resp_msg, 0, 100);
                break;
            }
            case CAMERA_MSG_ID_START_PERVIEW: {
                uint32_t screenId = *((uint32_t*)recv_msg.data);
                sub_cam_start_preview(screenId);
                break;
            }

            case CAMERA_MSG_ID_STOP_PERVIEW: {
                uint32_t screenId = *((uint32_t*)recv_msg.data);
                sub_cam_stop_preview(screenId);
                break;
            }
            case CAMERA_MSG_ID_GET_FRAME: {
                data_frame_cb callback_func =
                    (data_frame_cb)((void**)recv_msg.data)[0];
                void* param = (void*)((void**)recv_msg.data)[1];
                if(ctx->img_data == NULL)
                {
                    ctx->img_data = malloc(ctx->cur_cfg.qbuf_cfg.item_size);
                    if(!ctx->img_data)
                    {
                        printf("malloc img_data failed\r\n");
                    }
                }
                CamImg_t img_buf = {0};
                int ret = cameraGetBuf(&img_buf, 100);
                if(ret != 0)
                {
                    printf("cameraGetBuf failed, ret=%d\r\n", ret);
                    break;
                }
                ImageData_t image = {0};
                memcpy(ctx->img_data, img_buf.addr, img_buf.size);
                image.data = ctx->img_data;
                image.width = img_buf.width;
                image.height = img_buf.height;
                image.fmt = img_buf.fmt;
                image.size = img_buf.size;
                image.timestamp = img_buf.timestamp;
                callback_func(&image, param);
                cameraReleaseBuf(&img_buf);
                break;
            }
            case CAMERA_MSG_ID_SET_PARAMS: {
                CamSetting_e settings = (CamSetting_e)(*(int*)recv_msg.data);
                int value = *((int*)recv_msg.data + 1);
                sub_cam_set_param(settings, value);
                break;
            }
            case CAMERA_MSG_ID_GET_PARAMS: {
                SubCamMsg_t resp_msg = {0};
                resp_msg.id = CAMERA_MSG_ID_GET_PARAMS;
                resp_msg.data = malloc(2 * sizeof(int));
                resp_msg.len = 2 * sizeof(int);
                CamSetting_e settings = (CamSetting_e)(*(int*)recv_msg.data);
                sub_cam_get_param(settings, (int*)resp_msg.data + 1);
                *((int*)resp_msg.data) = (int)settings;
                osMessageQueuePut(ctx->msg_resp_queue, &resp_msg, 0, 100);
                break;
            }
            case CAMERA_MSG_ID_WR_REG: {
                uint8_t addr = (*(uint8_t*)recv_msg.data);
                uint8_t value = *((uint8_t*)recv_msg.data + 1);
                sub_cam_write_reg(addr, value);
                break;
            }
            case CAMERA_MSG_ID_RD_REG: {
                SubCamMsg_t resp_msg = {0};
                resp_msg.id = CAMERA_MSG_ID_RD_REG;
                resp_msg.data = malloc(sizeof(uint8_t));
                resp_msg.len = sizeof(uint8_t);
                uint8_t addr = (*(uint8_t*)recv_msg.data);
                sub_cam_read_reg(addr, (uint8_t*)resp_msg.data);
                osMessageQueuePut(ctx->msg_resp_queue, &resp_msg, 0, 100);
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

void subCameraInit()
{
#if 0 //mbtk change
    memset(&taskAttr,0,sizeof(taskAttr));
    memset(subsys_camera_task_stack, 0xA5,SUBSYS_CAMERA_TASK_STACK_SIZE);
    taskAttr.name = "camera";
    taskAttr.stack_mem = subsys_camera_task_stack;
    taskAttr.stack_size = SUBSYS_CAMERA_TASK_STACK_SIZE;
    taskAttr.priority = osPriorityNormal;
    taskAttr.cb_mem = &subsys_camera_task;//task control block
    taskAttr.cb_size = sizeof(StaticTask_t);//size of task control block
#else
    memset(&taskAttr, 0, sizeof(taskAttr));
    taskAttr.name = "camera";
    taskAttr.stack_size = 2024;
    taskAttr.priority = osPriorityNormal;
#endif
#if 0
    osThreadNew(tsk_subsys_camera, NULL, &taskAttr);
#else
    char serviceName[32] = {0};
    snprintf(serviceName, sizeof(serviceName), "service:/%s", taskAttr.name);
    Service_reg(serviceName, tsk_subsys_camera, NULL, taskAttr.cb_mem,
                taskAttr.cb_size, taskAttr.stack_mem, taskAttr.stack_size,
                taskAttr.priority);
    Service_start(serviceName);
#endif
}

void cameraStart(void)
{
    SubCamMsg_t msg = {0};
    SubCamMsg_t recv_msg = {0};
    SubCamCtx_t* ctx = get_subcam_ctx();
    if((ctx->status != CAMERA_STATUS_IDLE) &&
       (ctx->status != CAMERA_STATUS_STOPPED))
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, cameraStart_failed_0, P_INFO,
                      "camera has been started");
        return;
    }
    msg.id = CAMERA_MSG_ID_START;
    ctx->status = CAMERA_STATUS_STARTING;
    osMessageQueuePut(ctx->msg_queue, &msg, 0, osWaitForever);
    osMessageQueueGet(ctx->msg_resp_queue, &recv_msg, NULL, 1000);
}

void cameraStop(void)
{
    SubCamMsg_t msg = {0};
    SubCamMsg_t recv_msg = {0};
    SubCamCtx_t* ctx = get_subcam_ctx();
    if(ctx->status != CAMERA_STATUS_STARTED)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, cameraStop_failed_0, P_INFO,
                      "camera has not been started");
        return;
    }
    msg.id = CAMERA_MSG_ID_STOP;
    ctx->status = CAMERA_STATUS_STOPPING;
    osMessageQueuePut(ctx->msg_queue, &msg, 0, osWaitForever);
    osMessageQueueGet(ctx->msg_resp_queue, &recv_msg, NULL, 1000);
}

int cameraSetSettings(CamSetting_e settings, uint32_t value)
{
    SubCamMsg_t msg = {0};
    SubCamCtx_t* ctx = get_subcam_ctx();
    if(ctx->status != CAMERA_STATUS_STARTED)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, cameraSetSettings_failed_0, P_INFO,
                      "camera has not been started");
        return -1;
    }
    msg.id = CAMERA_MSG_ID_SET_PARAMS;
    msg.data = malloc(2 * sizeof(int));
    *((int*)msg.data) = settings;
    *((int*)msg.data + 1) = value;
    osMessageQueuePut(ctx->msg_queue, &msg, 0, osWaitForever);
    return 0;
}

int cameraGetSettings(CamSetting_e settings, uint32_t* value)
{
    SubCamMsg_t msg = {0};
    SubCamMsg_t recv_msg = {0};
    SubCamCtx_t* ctx = get_subcam_ctx();
    if(ctx->status != CAMERA_STATUS_STARTED)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, cameraGetSettings_failed_0, P_INFO,
                      "camera has not been started");
        return -1;
    }
    msg.id = CAMERA_MSG_ID_GET_PARAMS;
    msg.data = malloc(2 * sizeof(int));
    *((int*)msg.data) = settings;
    *((int*)msg.data + 1) = (int)value;
    osMessageQueuePut(ctx->msg_queue, &msg, 0, osWaitForever);
    memset(&recv_msg, 0, sizeof(SubCamMsg_t));
    int ret = osMessageQueueGet(ctx->msg_resp_queue, &recv_msg, NULL, 100);
    if(ret != osOK)
    {
        return -1;
    }
    if((recv_msg.data == NULL) || (recv_msg.len != 2 * sizeof(int)) ||
       (recv_msg.id != CAMERA_MSG_ID_GET_PARAMS))
    {
        return -1;
    }
    *value = *((int*)recv_msg.data + 1);
    free(recv_msg.data);
    return 0;
}

void cameraStartPreview(uint32_t screenId)
{
    SubCamMsg_t msg = {0};
    SubCamCtx_t* ctx = get_subcam_ctx();
    if((ctx->status != CAMERA_STATUS_IDLE) &&
       (ctx->status != CAMERA_STATUS_STOPPED))
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, cameraStartPreview_failed_0, P_INFO,
                      "camera has been started");
        return;
    }
    msg.id = CAMERA_MSG_ID_START_PERVIEW;
    ctx->status = CAMERA_STATUS_STARTING;
    msg.data = malloc(sizeof(uint32_t));
    *((uint32_t*)msg.data) = screenId;
    osMessageQueuePut(ctx->msg_queue, &msg, 0, osWaitForever);
}

void cameraStopPreview(uint32_t screenId)
{
    SubCamMsg_t msg = {0};
    SubCamCtx_t* ctx = get_subcam_ctx();
    if(ctx->status != CAMERA_STATUS_STARTED)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, cameraStopPreview_failed_0, P_INFO,
                      "camera has not been started");
        return;
    }
    msg.id = CAMERA_MSG_ID_STOP_PERVIEW;
    ctx->status = CAMERA_STATUS_STOPPING;
    msg.data = malloc(sizeof(uint32_t));
    *((uint32_t*)msg.data) = screenId;
    osMessageQueuePut(ctx->msg_queue, &msg, 0, osWaitForever);
}

void cameraGetFrame(data_frame_cb callback_func, void* param)
{
    SubCamCtx_t* ctx = get_subcam_ctx();
    if(callback_func == NULL)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, cameraGetFrame_failed_0, P_INFO,
                      "callback_func is null");
        return;
    }
    if(CAMERA_STATUS_STARTED != ctx->status)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, cameraGetFrame_failed_1, P_INFO,
                      "camera has not been started");
        return;
    }
    SubCamMsg_t msg = {0};
    msg.id = CAMERA_MSG_ID_GET_FRAME;
    msg.data = malloc(2 * sizeof(void*));
    ((void**)msg.data)[0] = callback_func;
    ((void**)msg.data)[1] = param;
    osMessageQueuePut(ctx->msg_queue, &msg, 0, osWaitForever);
}

int cameraWrReg(uint32_t addr, uint32_t value)
{
    SubCamMsg_t msg = {0};
    SubCamCtx_t* ctx = get_subcam_ctx();
    if(ctx->status != CAMERA_STATUS_STARTED)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, cameraWrReg_failed_0, P_INFO,
                      "camera has not been started");
        return -1;
    }
    msg.id = CAMERA_MSG_ID_WR_REG;
    msg.data = malloc(2 * sizeof(uint8_t));
    *((uint8_t*)msg.data) = addr;
    *((uint8_t*)msg.data + 1) = value;
    osMessageQueuePut(ctx->msg_queue, &msg, 0, osWaitForever);
    return 0;
}

uint32_t cameraRdReg(uint32_t addr)
{
    SubCamMsg_t msg = {0};
    SubCamMsg_t recv_msg = {0};
    uint32_t reg_value = 0;
    SubCamCtx_t* ctx = get_subcam_ctx();
    if(ctx->status != CAMERA_STATUS_STARTED)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, cameraRdReg_failed_0, P_INFO,
                      "camera has not been started");
        return -1;
    }
    msg.id = CAMERA_MSG_ID_RD_REG;
    msg.data = malloc(sizeof(uint8_t));
    msg.len = sizeof(uint8_t);
    *((uint8_t*)msg.data) = addr;
    osMessageQueuePut(ctx->msg_queue, &msg, 0, osWaitForever);
    memset(&recv_msg, 0, sizeof(SubCamMsg_t));
    int ret = osMessageQueueGet(ctx->msg_resp_queue, &recv_msg, NULL, 100);
    if(ret != osOK)
    {
        return -1;
    }
    if((recv_msg.data == NULL) || (recv_msg.len != sizeof(uint8_t)) ||
       (recv_msg.id != CAMERA_MSG_ID_RD_REG))
    {
        return -1;
    }
    reg_value = *(uint8_t*)recv_msg.data;
    free(recv_msg.data);
    return reg_value;
}

int cameraGetBuf(CamImg_t* img, int timeout)
{
    if(img == NULL)
    {
        return -1;
    }
    OpenCamGetBuf_t param = {0};
    param.img = img;
    param.timeout_ms = timeout;
    return Device_ioctl("dev:/camera", OPEN_CAM_IOCTL_GET_BUF, &param);
}

int cameraReleaseBuf(CamImg_t* img)
{
    return Device_ioctl("dev:/camera", OPEN_CAM_IOCTL_RELEASE_BUF, img);
}

int cameraClearBuf(void)
{
    return Device_ioctl("dev:/camera", OPEN_CAM_IOCTL_CLEAR_BUF, NULL);
}
