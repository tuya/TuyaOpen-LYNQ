#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include DEBUG_LOG_HEADER_FILE
#include <string.h>
#include "watch_album.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#if USED_WATCH_CAMERA
#include "watch_camera.h"
#endif
#include "open_image.h"
#include "syslog.h"
#include "storage.h"
#include "ui.h"
#include "ui_helpers.h"

typedef enum AlbumMsgCmd_
{
    ALBUM_MSG_CMD_REFRESH_PICTURE = 1,
    ALBUM_MSG_CMD_REFRESH_EXIT = 2,
    ALBUM_MSG_CMD_REFRESH_FULL_SCREEN = 3,
} AlbumMsgCmd_e;

typedef struct
{
    uint32_t cmd;
    uint8_t *buffer;
    uint32_t size;
} MsgCameraData_t;

lv_obj_t *ui_photoAlbumScreen = NULL;
lv_obj_t *ui_photoListView = NULL;
lv_obj_t *ui_photoThumbnail[MAX_ALBUM_SHOW_COUNT] = {NULL};
lv_obj_t *ui_photoAlbumPreview = NULL;
lv_obj_t *ui_albumPreviewButtonDel = NULL;
lv_obj_t *ui_albumPreviewDelLabel = NULL;
static lv_coord_t sAlbumColDsc[3] = {0};
static lv_coord_t sAlbumRowDsc[MAX_FILE_COUNT / 2 + 1] = {0};

static osMessageQueueId_t s_msg_album = NULL;
static AlbumPage_t s_album_page = {0};
static FileList_t s_list_img_file = {0};
static osThreadId_t s_tsk_album = NULL;
static uint8_t
    s_imgbuf_album_list[MAX_ALBUM_SHOW_COUNT]
                       [ALBUM_PREVIEW_WIDTH * ALBUM_PREVIEW_HEIGHT * 2] = {0};
static lv_img_dsc_t preview_img_dsc = {0};
static uint8_t *s_preview_buff = NULL;
static void delete_message_timer_cb(lv_timer_t *timer)
{
    lv_obj_t *label = (lv_obj_t *)timer->user_data;
    lv_obj_del(label);
    lv_timer_del(timer);
}

static void refresh_album_picture(uint8_t cur_page)
{
    MsgCameraData_t msg = {0};
    msg.cmd = ALBUM_MSG_CMD_REFRESH_PICTURE;
    msg.size = cur_page;
    osMessageQueuePut(s_msg_album, &msg, 0, 0);
}

void show_message_with_timeout(const char *message)
{
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, message);
    lv_obj_center(label);
    lv_timer_create(delete_message_timer_cb, 1000, label);
}

static void tsk_load_album_func(void *arg)
{
    FileList_t *list = (FileList_t *)arg;
    int file_index = 0;
    int file_count = 0;
    char *file_name = NULL;
    int ret = 0;
    int cur_page = 0;
    MsgCameraData_t msg = {0};
    lv_img_dsc_t img_dsc[MAX_ALBUM_SHOW_COUNT] = {0};
    uint8_t *album_img_buf[MAX_ALBUM_SHOW_COUNT] = {0};
    for(int i = 0; i < MAX_ALBUM_SHOW_COUNT; i++)
    {
        album_img_buf[i] = s_imgbuf_album_list[i];
    }
    if(!s_preview_buff)
    {
        s_preview_buff = malloc(CAMERA_IMG_WIDTH * CAMERA_IMG_HEIGHT * 2);
        if(!s_preview_buff)
        {
            SYSLOG_ERR("malloc preview buff failed\r\n");
            osThreadExit();
        }
    }
    while(1)
    {
        memset(&msg, 0, sizeof(MsgCameraData_t));
        if(osMessageQueueGet(s_msg_album, &msg, 0, osWaitForever) != osOK)
        {
            continue;
        }
        SYSLOG_INFO("recv message: cmd: %d, page: %d\r\n", msg.cmd, msg.size);
        if(msg.cmd == ALBUM_MSG_CMD_REFRESH_EXIT)
        {
            break;
        }
        if(msg.cmd == ALBUM_MSG_CMD_REFRESH_PICTURE)
        {
            cur_page = msg.size;
            file_index = cur_page * MAX_ALBUM_SHOW_COUNT;
            file_count = list->file_cnt >= (file_index + MAX_ALBUM_SHOW_COUNT)
                             ? MAX_ALBUM_SHOW_COUNT
                             : (list->file_cnt - file_index);
            SYSLOG_INFO("page: %d, file_index: %d, file_count: %d\r\n",
                        cur_page, file_index, file_count);
            for(int i = 0; i < MAX_ALBUM_SHOW_COUNT; i++)
            {
                if(i >= file_count)
                {
                    lv_obj_add_flag(ui_photoThumbnail[i], LV_OBJ_FLAG_HIDDEN);
                    continue;
                }
                file_name = list->file_path[file_index];
                SYSLOG_INFO(
                    "total: %d, page: %d, current file[%d]:%s, show index: "
                    "%d\r\n",
                    list->file_cnt, cur_page, file_index, file_name, i);
                OpImg_t *img =
                    open_image_create(CAMERA_IMG_WIDTH, CAMERA_IMG_HEIGHT,
                                      IMG_FMT_RGB565, NULL, 0);
                ret = open_image_read(file_name, img, NULL);
                if(ret != 0)
                {
                    open_image_destroy(img);
                    SYSLOG_ERR("open %s failed, ret: %d, %p\r\n", file_name,
                               ret, img);
                    continue;
                }
                OpImg_t *img_album = open_image_create(
                    ALBUM_PREVIEW_WIDTH, ALBUM_PREVIEW_HEIGHT, IMG_FMT_RGB565,
                    album_img_buf[i],
                    ALBUM_PREVIEW_WIDTH * ALBUM_PREVIEW_HEIGHT * 2);
                CropInfo_t info = {0};
                info.x = 0;
                info.y = 0;
                info.width = CAMERA_IMG_WIDTH;
                info.height = CAMERA_IMG_HEIGHT;
                open_image_scale(img, img_album, &info);
                img_dsc[i].header.cf = LV_IMG_CF_TRUE_COLOR;
                img_dsc[i].header.always_zero = 0;
                img_dsc[i].header.reserved = 0;
                img_dsc[i].header.w = ALBUM_PREVIEW_WIDTH;
                img_dsc[i].header.h = ALBUM_PREVIEW_HEIGHT;
                img_dsc[i].data_size = open_image_get_size(img_album);
                img_dsc[i].data = open_image_get_data(img_album);
                lv_img_set_src(ui_photoThumbnail[i], &img_dsc[i]);
                lv_obj_clear_flag(ui_photoThumbnail[i], LV_OBJ_FLAG_HIDDEN);
                file_index++;
                open_image_destroy(img);
                open_image_destroy(img_album);
            }
            lv_obj_invalidate(NULL);
            if(file_count == 0)
            {
                show_message_with_timeout("没有任何图片");
            }
        }
        else if(msg.cmd == ALBUM_MSG_CMD_REFRESH_FULL_SCREEN)
        {
            file_index =
                s_album_page.cur_page * MAX_ALBUM_SHOW_COUNT + msg.size;
            if(file_index > (s_list_img_file.file_cnt - 1))
            {
                continue;
            }
            s_album_page.cur_file = file_index;
            file_name = s_list_img_file.file_path[file_index];
            OpImg_t *img_prev = open_image_create(
                CAMERA_IMG_WIDTH, CAMERA_IMG_HEIGHT, IMG_FMT_RGB565,
                s_preview_buff, CAMERA_IMG_WIDTH * CAMERA_IMG_HEIGHT * 2);
            ret = open_image_read(file_name, img_prev, NULL);
            SYSLOG_INFO("preview file %s, index: %d\r\n", file_name,
                        file_index);
            if(ret != 0)
            {
                SYSLOG_ERR("open %s failed\r\n", file_name);
                continue;
            }
            preview_img_dsc.header.cf = LV_IMG_CF_TRUE_COLOR;
            preview_img_dsc.header.always_zero = 0;  // 必须设置为0
            preview_img_dsc.header.reserved = 0;     // 必须设置为0
            preview_img_dsc.header.w = CAMERA_IMG_WIDTH;
            preview_img_dsc.header.h = CAMERA_IMG_HEIGHT;
            preview_img_dsc.data_size =
                CAMERA_IMG_WIDTH * CAMERA_IMG_HEIGHT * 2;
            preview_img_dsc.data = s_preview_buff;
            SYSLOG_INFO("show preview %p, size: %d\r\n", preview_img_dsc.data,
                        preview_img_dsc.data_size);
            lv_img_set_src(ui_photoAlbumPreview, &preview_img_dsc);
            lv_obj_clear_flag(ui_photoAlbumPreview, LV_OBJ_FLAG_HIDDEN);
            open_image_destroy(img_prev);
        }
    }

    osThreadExit();
}

int tsk_loadalbum_create(void)
{
    osThreadAttr_t task_attr;
    if(s_tsk_album)
    {
        return -1;
    }

    s_msg_album = osMessageQueueNew(2, sizeof(MsgCameraData_t), NULL);
    if(s_msg_album == NULL)
    {
        SYSLOG_ERR("Failed to create queue for camera data process.\r\n");
        return -1;
    }

    memset(&task_attr, 0, sizeof(osThreadAttr_t));
    task_attr.name = TSK_NAME_ALBUM;
    task_attr.stack_size = 4 * 1024;
    task_attr.priority = osPriorityBelowNormal6;
    s_tsk_album =
        osThreadNew(tsk_load_album_func, &s_list_img_file, &task_attr);
    return s_tsk_album ? 0 : -1;
}

int tsk_loadalbum_destroy(void)
{
    MsgCameraData_t msg = {0};
    if(!s_tsk_album)
    {
        return 0;
    }
    msg.cmd = ALBUM_MSG_CMD_REFRESH_EXIT;
    osMessageQueuePut(s_msg_album, &msg, 0, 0);
    while((osThreadGetState(s_tsk_album) != osThreadTerminated))
    {
        osDelay(100);
    }
    if(s_tsk_album)
    {
        s_tsk_album = NULL;
    }
    s_tsk_album = NULL;
    if(s_msg_album)
    {
        osMessageQueueDelete(s_msg_album);
        s_msg_album = NULL;
    }
    SYSLOG_INFO("tsk_loadalbum_destroy exit\r\n");
    return 0;
}

uint32_t search_all_captured_images(FileList_t *list)
{
    struct lfs_info *info = NULL;
    DIR *dir = NULL;
    dir = opendir("D:/");
    list->file_cnt = 0;
    if(dir == NULL)
    {
        SYSLOG_ERR("open DIR:[D:/] failed!\r\n");
        return list->file_cnt;
    }

    while(true)
    {
        info = (struct lfs_info *)readdir(dir);
        if(info == NULL)
        {
            break;
        }
        if(info->type == LFS_TYPE_DIR)
        {
            continue;
        }
        else if(info->type == LFS_TYPE_REG)
        {
            size_t len = strlen(info->name);
            if(len >= 8 && strncmp(info->name, "DCIM", 4) == 0 &&
               (strcasecmp(info->name + len - 4, ".jpg") == 0 ||
                strcasecmp(info->name + len - 5, ".jpeg") == 0))
            {
                memset(list->file_path[list->file_cnt], 0, 32);
                snprintf(list->file_path[list->file_cnt], 32, "D:/%s",
                         info->name);
                list->file_cnt++;
            }
        }
    }
    SYSLOG_INFO("total PIC: %d!\r\n", list->file_cnt);
    return list->file_cnt;
}

static void refresh_file_list(void)
{

    uint32_t file_count = 0;
    file_count = search_all_captured_images(&s_list_img_file);
    s_album_page.cur_page = 0;
    s_album_page.max_page = (file_count % MAX_ALBUM_SHOW_COUNT == 0)
                                ? file_count / MAX_ALBUM_SHOW_COUNT
                                : file_count / MAX_ALBUM_SHOW_COUNT + 1;
}

void on_album_open()
{
    lv_obj_add_flag(ui_photoAlbumPreview, LV_OBJ_FLAG_HIDDEN);
    tsk_loadalbum_create();
    refresh_file_list();
    refresh_album_picture(0);
}

void on_album_close() { tsk_loadalbum_destroy(); }

void on_album_page_updown(bool up)
{
    char chmsg[64] = {0};
    if(up)
    {
        if(s_album_page.cur_page == s_album_page.max_page - 1)
        {
            snprintf(chmsg, 64, "已经到最后一页");
            show_message_with_timeout(chmsg);
        }
        else
        {
            s_album_page.cur_page++;
            sprintf(chmsg, "相册%d/%d", s_album_page.cur_page + 1,
                    s_album_page.max_page);
            show_message_with_timeout(chmsg);
            refresh_album_picture(s_album_page.cur_page);
        }
    }
    else
    {
        if(s_album_page.cur_page == 0)
        {
            snprintf(chmsg, 64, "已经到第一页");
            show_message_with_timeout(chmsg);
        }
        else
        {
            s_album_page.cur_page--;
            sprintf(chmsg, "相册%d/%d", s_album_page.cur_page + 1,
                    s_album_page.max_page);
            show_message_with_timeout(chmsg);
            refresh_album_picture(s_album_page.cur_page);
        }
    }
}

void on_thumbnail_preview(int index)
{
    MsgCameraData_t msg = {0};
    msg.cmd = ALBUM_MSG_CMD_REFRESH_FULL_SCREEN;
    msg.size = index;
    osMessageQueuePut(s_msg_album, &msg, 0, 0);
}

void on_picture_del()
{
    char *file_name = s_list_img_file.file_path[s_album_page.cur_file];
    remove(file_name);
    lv_obj_add_flag(ui_photoAlbumPreview, LV_OBJ_FLAG_HIDDEN);
    refresh_file_list();
    refresh_album_picture(0);
}

void ui_event_button_photo_del(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if(event_code == LV_EVENT_CLICKED)
    {
        on_picture_del();
    }
}

void ui_event_thumbnail_photo_album(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if(code == LV_EVENT_LONG_PRESSED)
    {
        int index = -1;
        for(int i = 0; i < MAX_ALBUM_SHOW_COUNT; i++)
        {
            if(target == ui_photoThumbnail[i])
            {
                index = i;
            }
        }
        if(index != -1)
        {
            SYSLOG_INFO("click %d\r\n", index);
            on_thumbnail_preview(index);
        }
    }
}

void ui_event_screen_photo_album(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if(event_code == LV_EVENT_GESTURE)
    {
        if(lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT)
        {
            if(lv_obj_has_flag(ui_photoAlbumPreview, LV_OBJ_FLAG_HIDDEN))
            {
                _ui_screen_change(ui_appListScreen, LV_SCR_LOAD_ANIM_OVER_RIGHT,
                                  500, 0);
            }
            else
            {
                lv_obj_add_flag(ui_photoAlbumPreview, LV_OBJ_FLAG_HIDDEN);
            }
        }
        else if(lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_TOP)
        {
            if(lv_obj_has_flag(ui_photoAlbumPreview, LV_OBJ_FLAG_HIDDEN))
            {
                on_album_page_updown(false);
            }
        }
        else if(lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_BOTTOM)
        {
            if(lv_obj_has_flag(ui_photoAlbumPreview, LV_OBJ_FLAG_HIDDEN))
            {
                on_album_page_updown(true);
            }
        }
    }
    else if(event_code == LV_EVENT_SCREEN_LOAD_START)
    {
        on_album_open();
    }
    else if(event_code == LV_EVENT_SCREEN_UNLOAD_START)
    {
        on_album_close();
    }
}

void ui_watch_album_init(void)
{
    ui_photoAlbumScreen = lv_obj_create(NULL);
    ui_photoListView = lv_obj_create(ui_photoAlbumScreen);
    lv_obj_set_size(ui_photoListView, lv_pct(100), lv_pct(100));
    lv_obj_set_layout(ui_photoListView, LV_LAYOUT_GRID);
    for(int i = 0; i < 2; i++)
    {
        sAlbumColDsc[i] = 100;
    }
    sAlbumColDsc[2] = LV_GRID_TEMPLATE_LAST;
    for(int i = 0; (i < MAX_FILE_COUNT / 2 - 1); i++)
    {
        sAlbumRowDsc[i] = 100;
    }
    sAlbumRowDsc[MAX_FILE_COUNT / 2 - 1] = LV_GRID_TEMPLATE_LAST;
    lv_obj_set_grid_dsc_array(ui_photoListView, sAlbumColDsc, sAlbumRowDsc);
    for(int i = 0; i < 4; i++)
    {
        ui_photoThumbnail[i] = lv_img_create(ui_photoListView);
        lv_obj_add_flag(ui_photoThumbnail[i], LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_grid_cell(ui_photoThumbnail[i], LV_GRID_ALIGN_CENTER, i % 2,
                             1, LV_GRID_ALIGN_CENTER, i / 2, 1);
        lv_obj_add_event_cb(ui_photoThumbnail[i],
                            ui_event_thumbnail_photo_album, LV_EVENT_ALL, NULL);
    }
    lv_obj_add_event_cb(ui_photoAlbumScreen, ui_event_screen_photo_album,
                        LV_EVENT_ALL, NULL);

    ui_photoAlbumPreview = lv_img_create(ui_photoAlbumScreen);
    lv_obj_set_width(ui_photoAlbumPreview, 240);
    lv_obj_set_height(ui_photoAlbumPreview, 240);
    lv_obj_add_flag(ui_photoAlbumPreview, LV_OBJ_FLAG_HIDDEN);

    ui_albumPreviewButtonDel = lv_btn_create(ui_photoAlbumPreview);
    lv_obj_set_size(ui_albumPreviewButtonDel, 60, 30);
    lv_obj_align(ui_albumPreviewButtonDel, LV_ALIGN_BOTTOM_MID, 0, -8);
    lv_obj_set_style_bg_color(ui_albumPreviewButtonDel, lv_color_hex(0xFF3B30),
                              0);
    lv_obj_set_style_border_color(ui_albumPreviewButtonDel, lv_color_white(),
                                  0);
    lv_obj_set_style_border_width(ui_albumPreviewButtonDel, 2, 0);
    lv_obj_set_style_radius(ui_albumPreviewButtonDel, 12, 0);

    ui_albumPreviewDelLabel = lv_label_create(ui_albumPreviewButtonDel);
    lv_label_set_text(ui_albumPreviewDelLabel, "删除");
    lv_obj_center(ui_albumPreviewDelLabel);
    lv_obj_add_event_cb(ui_albumPreviewButtonDel, ui_event_button_photo_del,
                        LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_color(ui_albumPreviewDelLabel,
                              lv_color_darken(lv_color_hex(0xFF3B30), 30),
                              LV_STATE_PRESSED);
}

void ui_watch_album_destroy(void)
{
    if(ui_photoAlbumScreen)
    {
        lv_obj_del(ui_photoAlbumScreen);
    }
}