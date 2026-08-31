#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include DEBUG_LOG_HEADER_FILE
#include <string.h>
#if USED_WATCH_ALBUM
#include "watch_album.h"
#endif
#include "watch_camera.h"
#include "camera.h"
#include "open_image.h"
#include "syslog.h"
#include "systime.h"
#include "ui.h"
#include "ui_helpers.h"

lv_obj_t *ui_cameraScreen;
lv_obj_t *ui_cameraPreview;
lv_obj_t *ui_cameraPanel;
lv_obj_t *ui_cameraCapture;
lv_obj_t *ui_cameraTitle;
lv_obj_t *ui_cameraIcon;
lv_obj_t *ui_cameraButton;
lv_obj_t *ui_cameraButtonLabel;

static osThreadId_t s_tsk_camera = NULL;
static lv_timer_t *s_timer_camera_show = NULL;
static bool s_cam_running = false;
static bool s_cam_capture = false;
static lv_img_dsc_t lv_preview_img = {0};
static lv_img_dsc_t lv_cap_img = {0};
static uint8_t preview_buf[CAMERA_IMG_WIDTH * CAMERA_IMG_HEIGHT * 2] = {0};
static uint8_t cap_prev_buf[CAPTURE_PREVIEW_IMG_WIDTH *
                            CAPTURE_PREVIEW_IMG_HEIGHT * 2] = {0};

static int get_snapshot_file_name(char *file_name, uint32_t size)
{
    time_t curtime;
    int ret = 0;
    struct tm *tmTime;
    struct timeval tv;
    struct timezone tz;
    curtime = time_time(NULL);
    tmTime = time_localtime(&curtime);
    time_gettimeofday(&tv, &tz);
    ret = snprintf(file_name, size, "D:/DCIM_%d%02d%02d_%02d%02d%02d_%d.jpg",
                   1900 + tmTime->tm_year, tmTime->tm_mon + 1, tmTime->tm_mday,
                   tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec,
                   tv.tv_usec / 10000);
    return (ret != size - 1) ? 0 : -1;
}

void ui_event_camera_capture(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED)
    {
        SYSLOG_INFO("ui_event_camera_capture clicked\r\n");
#ifdef USED_WATCH_ALBUM
        _ui_screen_change(ui_photoAlbumScreen, LV_SCR_LOAD_ANIM_FADE_ON, 500,
                          0);
#endif
    }
}

static void show_camera_capture(CamImg_t *img, uint8_t *cap_prev_buf)
{
    OpImg_t *cam_img = open_image_create(img->width, img->height,
                                         IMG_FMT_YUV422S, img->addr, img->size);
    OpImg_t *cam_img_rgb =
        open_image_create(img->width, img->height, IMG_FMT_RGB565, NULL, 0);
    OpImg_t *cam_img_save = open_image_create(
        CAMERA_IMG_WIDTH, CAMERA_IMG_HEIGHT, IMG_FMT_RGB565, NULL, 0);
    OpImg_t *cam_img_preview = open_image_create(
        CAPTURE_PREVIEW_IMG_WIDTH, CAPTURE_PREVIEW_IMG_HEIGHT, IMG_FMT_RGB565,
        cap_prev_buf,
        CAPTURE_PREVIEW_IMG_WIDTH * CAPTURE_PREVIEW_IMG_HEIGHT * 2);
    open_image_conv_fmt(cam_img, cam_img_rgb);
    CropInfo_t info = {0};
    info.x = (img->width - CAMERA_IMG_WIDTH) / 2;
    info.y = (img->height - CAMERA_IMG_HEIGHT) / 2;
    info.width = CAMERA_IMG_WIDTH;
    info.height = CAMERA_IMG_HEIGHT;
    open_image_scale(cam_img_rgb, cam_img_save, &info);

    char file_name[64] = {0};
    int ret = 0;
    ret = get_snapshot_file_name(file_name, 64);
    ret = open_image_save(file_name, cam_img_save, NULL);
    SYSLOG_ERR("save capture file %s %s\r\n", file_name,
               ret == 0 ? "success" : "failed");
    info.x = 0;
    info.y = 0;
    info.width = CAMERA_IMG_WIDTH;
    info.height = CAMERA_IMG_HEIGHT;
    open_image_scale(cam_img_save, cam_img_preview, &info);
    lv_cap_img.data = open_image_get_data(cam_img_preview);
    lv_img_set_src(ui_cameraCapture, &lv_cap_img);
    open_image_destroy(cam_img);
    open_image_destroy(cam_img_preview);
    open_image_destroy(cam_img_rgb);
}

static void show_camera_preview(CamImg_t *img, uint8_t *preview_buf)
{
    OpImg_t *cam_img = open_image_create(img->width, img->height,
                                         IMG_FMT_YUV422S, img->addr, img->size);
    OpImg_t *cam_img_rgb =
        open_image_create(img->width, img->height, IMG_FMT_RGB565, NULL, 0);
    OpImg_t *cam_img_show = open_image_create(
        CAMERA_IMG_WIDTH, CAMERA_IMG_HEIGHT, IMG_FMT_RGB565, preview_buf,
        CAMERA_IMG_WIDTH * CAMERA_IMG_HEIGHT * 2);
    open_image_conv_fmt(cam_img, cam_img_rgb);
    CropInfo_t info = {0};
    info.x = (img->width - CAMERA_IMG_WIDTH) / 2;
    info.y = (img->height - CAMERA_IMG_HEIGHT) / 2;
    info.width = CAMERA_IMG_WIDTH;
    info.height = CAMERA_IMG_HEIGHT;
    open_image_scale(cam_img_rgb, cam_img_show, &info);
    lv_preview_img.data = (void *)open_image_get_data(cam_img_show);
    lv_img_set_src(ui_cameraPreview, &lv_preview_img);
    open_image_destroy(cam_img);
    open_image_destroy(cam_img_rgb);
    open_image_destroy(cam_img_show);
}

static void tsk_camera_func(void *arg)
{
    int ret = 0;
    cameraStart();
    lv_preview_img.header.cf = LV_IMG_CF_TRUE_COLOR;
    lv_preview_img.header.always_zero = 0;
    lv_preview_img.header.reserved = 0;
    lv_preview_img.header.w = CAMERA_IMG_WIDTH;
    lv_preview_img.header.h = CAMERA_IMG_HEIGHT;
    lv_cap_img.header.cf = LV_IMG_CF_TRUE_COLOR;
    lv_cap_img.header.always_zero = 0;
    lv_cap_img.header.reserved = 0;
    lv_cap_img.header.w = CAPTURE_PREVIEW_IMG_WIDTH;
    lv_cap_img.header.h = CAPTURE_PREVIEW_IMG_HEIGHT;
    CamType_e type = hal_cam_get_type();
    SYSLOG_INFO("camera type: %d\r\n", type);
    while(s_cam_running)
    {
        CamImg_t img = {0};
        ret = cameraGetBuf(&img, 100);
        if(ret != 0)
        {
            osDelay(80);
            continue;
        }
        if(s_cam_capture)
        {
            show_camera_capture(&img, cap_prev_buf);
            s_cam_capture = false;
        }
        else
        {
            show_camera_preview(&img, preview_buf);
        }
        cameraReleaseBuf(&img);
    }
    cameraStop();
    SYSLOG_INFO("watch camera view thread exit!\r\n");
    osThreadExit();
}

void on_camera_start()
{
    osThreadAttr_t task_attr;
    if(s_tsk_camera)
    {
        SYSLOG_ERR("task camera not stop\r\n");
        return;
    }
    memset(&task_attr, 0, sizeof(osThreadAttr_t));
    task_attr.name = TSK_NAME_CAMERA;
    task_attr.stack_size = 4 * 1024;
    task_attr.priority = osPriorityBelowNormal6;
    s_cam_running = true;
    s_tsk_camera = osThreadNew(tsk_camera_func, NULL, &task_attr);
}

static void on_hide_capture_preview(void)
{
    bool is_hidden = lv_obj_has_flag(ui_cameraCapture, LV_OBJ_FLAG_HIDDEN);
    if(!is_hidden)
    {
        lv_obj_add_flag(ui_cameraCapture, LV_OBJ_FLAG_HIDDEN);
        lv_timer_del(s_timer_camera_show);
        s_timer_camera_show = NULL;
    }
}

static void capture_show_timer_callback(lv_timer_t *timer)
{
    on_hide_capture_preview();
}

void on_camera_stop()
{
    if(!s_cam_running)
    {
        SYSLOG_ERR("task camera not need stop\r\n");
        return;
    }
    s_cam_running = false;
    s_cam_capture = false;
    while((osThreadGetState(s_tsk_camera) != osThreadTerminated))
    {
        osDelay(100);
    }
    on_hide_capture_preview();
    s_tsk_camera = NULL;
    SYSLOG_INFO("camera capture view stop!\r\n");
}

void on_camera_capture()
{
    bool is_hidden = false;
    if(!lv_obj_is_valid(ui_cameraCapture))
    {
        return;
    }
    s_cam_capture = true;
    is_hidden = lv_obj_has_flag(ui_cameraCapture, LV_OBJ_FLAG_HIDDEN);
    if(is_hidden)
    {
        lv_obj_clear_flag(ui_cameraCapture, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_cameraCapture, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(ui_cameraCapture, ui_event_camera_capture,
                            LV_EVENT_CLICKED, NULL);
    }
    SYSLOG_INFO("update capture!\r\n");
    if(!s_timer_camera_show)
    {
        s_timer_camera_show =
            lv_timer_create(capture_show_timer_callback, 5000, NULL);
    }
    else
    {
        lv_timer_reset(s_timer_camera_show);
    }
}

void ui_event_screen_camera(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if(event_code == LV_EVENT_GESTURE &&
       lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT)
    {
        _ui_screen_change(ui_appListScreen, LV_SCR_LOAD_ANIM_OVER_RIGHT, 500, 0);
    }
    else if(event_code == LV_EVENT_SCREEN_LOAD_START)
    {
        on_camera_start();
    }
    else if(event_code == LV_EVENT_SCREEN_UNLOAD_START)
    {
        on_camera_stop();
    }
}

void ui_event_button_capture(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if(event_code == LV_EVENT_CLICKED)
    {
        on_camera_capture();
    }
}

void ui_watch_camera_init(void)
{
    ui_cameraScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_cameraScreen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_cameraScreen, lv_color_hex(0x000000),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_cameraScreen, 255,
                            LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(ui_cameraScreen, ui_event_screen_camera, LV_EVENT_ALL,
                        NULL);

    ui_cameraPreview = lv_img_create(ui_cameraScreen);
    lv_obj_set_width(ui_cameraPreview, 240);
    lv_obj_set_height(ui_cameraPreview, 240);

    ui_cameraCapture = lv_img_create(ui_cameraPreview);
    lv_obj_set_width(ui_cameraCapture, CAPTURE_PREVIEW_IMG_WIDTH);
    lv_obj_set_height(ui_cameraCapture, CAPTURE_PREVIEW_IMG_HEIGHT);
    lv_obj_add_flag(ui_cameraCapture, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_cameraCapture, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_border_color(ui_cameraCapture, lv_color_hex(0xFFFFFF),
                                  LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_cameraCapture, 2,
                                  LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui_cameraCapture, 8,
                            LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(ui_cameraCapture, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_add_event_cb(ui_cameraCapture, ui_event_camera_capture,
                        LV_EVENT_CLICKED, NULL);

    ui_cameraButton = lv_btn_create(ui_cameraPreview);
    lv_obj_set_size(ui_cameraButton, 80, 40);
    lv_obj_align(ui_cameraButton, LV_ALIGN_BOTTOM_MID, 0, -8);
    lv_obj_set_style_bg_color(ui_cameraButton, lv_color_hex(0xFF3B30), 0);
    lv_obj_set_style_border_color(ui_cameraButton, lv_color_white(), 0);
    lv_obj_set_style_border_width(ui_cameraButton, 2, 0);
    lv_obj_set_style_radius(ui_cameraButton, 12, 0);

    ui_cameraButtonLabel = lv_label_create(ui_cameraButton);
    lv_label_set_text(ui_cameraButtonLabel, "拍照");
    lv_obj_center(ui_cameraButtonLabel);
    lv_obj_add_event_cb(ui_cameraButton, ui_event_button_capture,
                        LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_color(ui_cameraButton,
                              lv_color_darken(lv_color_hex(0xFF3B30), 30),
                              LV_STATE_PRESSED);
}

void ui_watch_camera_destroy(void)
{
    if(ui_cameraScreen)
    {
        lv_obj_del(ui_cameraScreen);
    }
}