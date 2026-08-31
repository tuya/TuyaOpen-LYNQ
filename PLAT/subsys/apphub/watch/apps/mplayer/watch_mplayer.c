#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include DEBUG_LOG_HEADER_FILE
#include <string.h>
#include "watch_mplayer.h"
#include "syslog.h"
#include "storage.h"
#include "ui.h"
#include "mp3.h"
#include "openplayer.h"

lv_obj_t* ui_scrFileList = NULL;
lv_obj_t* ui_scrMusicPlayer = NULL;
lv_obj_t* ui_ibExit = NULL;
lv_obj_t* ui_ibPlay = NULL;
lv_obj_t* ui_ibLast = NULL;
lv_obj_t* ui_ibNext = NULL;
lv_obj_t* ui_labCurTime = NULL;
lv_obj_t* ui_labTotalTime = NULL;
lv_obj_t* ui_sliTime = NULL;
lv_obj_t* ui_imgMusicPlay = NULL;
lv_obj_t* ui_lbMusicTitle = NULL;
lv_obj_t* ui_mp3FileList = NULL;  // MP3文件列表控件
static lv_timer_t* s_playback_timer = NULL;
#define MAX_MP3_FILES 64

typedef enum UiPlayState_
{
    UI_PLAY_STATE_PLAY = 0,
    UI_PLAY_STATE_PAUSE = 1,
    UI_PLAY_STATE_STOP = 2,
} UiPlayState_e;

static char s_mp3_files[MAX_MP3_FILES][255];
static int s_mp3_file_count = 0;
static uint32_t s_cur_mps_index = 0;
static openPlayer s_open_player = NULL;
static osEventFlagsId_t s_open_player_event = NULL;
static int s_current_play_time = 0;
static int s_total_duration = 0;
static bool s_is_pause = false;
static UiPlayState_e s_play_state = UI_PLAY_STATE_STOP;
static void search_mp3_files(const char* dir_path)
{
    DIR* dir;
    struct lfs_info* info = NULL;
    // 打开目录
    dir = opendir(dir_path);
    if(dir == NULL)
    {
        SYSLOG_ERR("open DIR:[%s] failed!\r\n", dir_path);
        return;
    }
    while(true)
    {
        info = (struct lfs_info*)readdir(dir);
        if(info == NULL)
        {
            break;
        }
        if(strcmp(info->name, ".") == 0 || strcmp(info->name, "..") == 0)
        {
            continue;
        }

        if(info->type == LFS_TYPE_DIR)
        {
            char sub_dir_path[512];
            snprintf(sub_dir_path, sizeof(sub_dir_path), "%s%c%s", dir_path,
                     '/', info->name);
            search_mp3_files(sub_dir_path);
            continue;
        }
        else if(info->type == LFS_TYPE_REG)
        {
            size_t len = strlen(info->name);
            if(len >= 8 && (strcasecmp(info->name + len - 4, ".mp3") == 0))
            {
                snprintf(s_mp3_files[s_mp3_file_count], 255, "%s%s", dir_path,
                         info->name);
                s_mp3_file_count++;
                // 检查是否达到最大文件数量
                if(s_mp3_file_count >= MAX_MP3_FILES)
                {
                    break;
                }
            }
        }
    }

    // 关闭目录
    closedir(dir);
}

static void ui_mp3_file_click_cb(lv_event_t* e)
{
    int index = (int)(intptr_t)lv_event_get_user_data(e);
    SYSLOG_INFO("Selected MP3: %s\r\n", s_mp3_files[index]);
    s_cur_mps_index = index;
    lv_scr_load_anim(ui_scrMusicPlayer, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0,
                     false);
}

static void ui_scrFileList_load_event_cb(lv_event_t* e)
{
    SYSLOG_INFO(
        "ui_scrFileList screen loaded, start scanning MP3 files...\r\n");
    lv_obj_clean(ui_mp3FileList);
    s_mp3_file_count = 0;
    memset(s_mp3_files, 0, MAX_MP3_FILES * sizeof(char) * 255);
    search_mp3_files("D:/");
    for(int i = 0; i < s_mp3_file_count; i++)
    {
        if(strlen(s_mp3_files[i]) > 0)
        {
            char* filename = strrchr(s_mp3_files[i], '/');
            if(filename)
                filename++;
            else
                filename = s_mp3_files[i];
            lv_obj_t* btn = lv_list_add_btn(
                ui_mp3FileList, UI_IMG_MUSIC_FILE_PNG, s_mp3_files[i]);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x2C3E50), 0);
            lv_obj_set_style_bg_opa(btn, LV_OPA_50, 0);
            lv_obj_set_style_border_width(btn, 1, 0);
            lv_obj_set_style_border_color(btn, lv_color_hex(0x34495E), 0);
            lv_obj_set_style_radius(btn, 4, 0);
            lv_obj_set_style_pad_all(btn, 8, 0);

            // 添加悬停效果
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x3498DB),
                                      LV_STATE_PRESSED);
            // 为每个按钮添加点击事件
            lv_obj_add_event_cb(btn, ui_mp3_file_click_cb, LV_EVENT_CLICKED,
                                (void*)(intptr_t)i);
        }
    }

    SYSLOG_INFO("Found %d MP3 files\r\n", s_mp3_file_count);
}

static void ui_back_btn_click_cb(lv_event_t* e)
{
    SYSLOG_INFO("Back button clicked, returning to home screen\r\n");
    if(ui_home)
    {
        lv_scr_load_anim(ui_appListScreen, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 300, 0,
                         false);
    }
}

void ui_scrFileList_screen_init(void)
{
    ui_scrFileList = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_scrFileList, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
    lv_obj_set_style_bg_color(ui_scrFileList, lv_color_hex(0x1A2A3A), 0);
    lv_obj_set_style_bg_grad_color(ui_scrFileList, lv_color_hex(0x0F1A24), 0);
    lv_obj_set_style_bg_grad_dir(ui_scrFileList, LV_GRAD_DIR_VER, 0);

    lv_obj_t* title_bar = lv_obj_create(ui_scrFileList);
    lv_obj_set_size(title_bar, 240, 40);
    lv_obj_align(title_bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(title_bar, lv_color_hex(0x2C3E50), 0);
    lv_obj_set_style_border_width(title_bar, 0, 0);
    lv_obj_set_style_radius(title_bar, 0, 0);
    // 标题文字
    lv_obj_t* title_label = lv_label_create(title_bar);
    lv_label_set_text(title_label, "音乐文件");
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xECF0F1), 0);
    lv_obj_center(title_label);

    // 返回按钮（左上角）
    lv_obj_t* back_btn = lv_btn_create(title_bar);
    lv_obj_set_size(back_btn, 30, 30);
    lv_obj_align(back_btn, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_bg_color(back_btn, lv_color_hex(0xE74C3C), 0);
    lv_obj_set_style_border_width(back_btn, 0, 0);
    lv_obj_set_style_radius(back_btn, 20, 0);

    lv_obj_t* back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, "<");
    lv_obj_set_style_text_color(back_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(back_label, &lv_font_montserrat_20, 0);
    lv_obj_center(back_label);

    lv_obj_add_event_cb(back_btn, ui_back_btn_click_cb, LV_EVENT_CLICKED, NULL);
    ui_mp3FileList = lv_list_create(ui_scrFileList);
    lv_obj_set_size(ui_mp3FileList, 240, 190);
    lv_obj_align(ui_mp3FileList, LV_ALIGN_TOP_MID, 0,
                 45);  // 从y=45开始（标题栏下方5px）
    lv_obj_add_event_cb(ui_scrFileList, ui_scrFileList_load_event_cb,
                        LV_EVENT_SCREEN_LOADED, NULL);
}

void ui_scrFileList_screen_destroy(void)
{
    if(ui_scrFileList) lv_obj_del(ui_scrFileList);

    // NULL screen variables
    ui_scrFileList = NULL;
}

static void playback_timer_cb(lv_timer_t* timer)
{
    if(!s_open_player) return;

    s_current_play_time++;

    // 更新当前时间标签
    char cur_time[10] = {0};
    snprintf(cur_time, 10, "%02d:%02d", s_current_play_time / 60,
             s_current_play_time % 60);
    lv_label_set_text(ui_labCurTime, cur_time);

    // 更新进度条
    if(s_total_duration > 0)
    {
        int progress = (s_current_play_time * 100) / s_total_duration;
        lv_slider_set_value(ui_sliTime, progress, LV_ANIM_ON);
    }

    // 检查是否播放完成
    if(s_current_play_time >= s_total_duration)
    {
        if(s_playback_timer)
        {
            lv_timer_del(s_playback_timer);
            s_playback_timer = NULL;
        }
    }
}

const char* get_filename_simple(const char* path)
{
    if(path == NULL)
    {
        return NULL;
    }

    const char* last_sep = strrchr(path, '/');
    const char* last_sep_win = strrchr(path, '\\');

    // 取最后一个分隔符
    if(last_sep_win > last_sep)
    {
        last_sep = last_sep_win;
    }

    return (last_sep != NULL) ? last_sep + 1 : path;
}

static void ui_set_play_state(UiPlayState_e state, bool resume)
{
    s_play_state = state;
    if(state == UI_PLAY_STATE_PLAY)
    {
        lv_img_set_src(ui_ibPlay, UI_IMG_PAUSE_PNG);
        if((resume) && (s_playback_timer))
        {
            lv_timer_resume(s_playback_timer);
        }
        else
        {
            s_current_play_time = 0;
            s_playback_timer = lv_timer_create(playback_timer_cb, 1000, NULL);
        }
    }
    else if(state == UI_PLAY_STATE_PAUSE)
    {
        lv_img_set_src(ui_ibPlay, UI_IMG_PLAY_PNG);
        if(s_playback_timer)
        {
            lv_timer_pause(s_playback_timer);
        }
    }
    else if(state == UI_PLAY_STATE_STOP)
    {
        lv_img_set_src(ui_ibPlay, UI_IMG_PLAY_PNG);
        if(s_playback_timer)
        {
            lv_timer_del(s_playback_timer);
            s_playback_timer = NULL;
        }
        s_playback_timer = NULL;
        s_current_play_time = 0;
        lv_slider_set_value(ui_sliTime, 0, LV_ANIM_ON);
        lv_label_set_text(ui_labCurTime, "00:00");
    }
}

static void open_player_callback(void* userdata, int32_t result)
{
    SYSLOG_INFO("open_player_callback result = %d\r\n", result);
    if(result == AV_RET_PLAY_STOP)
    {
        if(s_is_pause)
        {
            ui_set_play_state(UI_PLAY_STATE_PAUSE, false);
        }
        else
        {
            ui_set_play_state(UI_PLAY_STATE_STOP, false);
        }
    }
    else if(result == AV_RET_PLAY_EOF)
    {
        ui_set_play_state(UI_PLAY_STATE_STOP, false);
    }
    osEventFlagsSet(s_open_player_event, 0x01);
}

static void mp3_info_refresh(void)
{
    AudioInfo_t info = {0};
    lv_label_set_text(ui_lbMusicTitle,
                      get_filename_simple(s_mp3_files[s_cur_mps_index]));
    int ret = mp3GetInfo(s_mp3_files[s_cur_mps_index], &info);
    if(ret != 0)
    {
        SYSLOG_ERR("mp3GetInfo failed!\r\n");
        return;
    }
    else
    {
        char time[16] = {0};
        sprintf(time, "%02d:%02d", info.duration / 60, info.duration % 60);
        lv_label_set_text(ui_labTotalTime, time);
        s_total_duration = info.duration;
    }
}

static void open_player_set_param(void)
{
    if(!s_open_player)
    {
        return;
    }
    openPlayerConfigT param = {0};
    param.playParam.codec = AUDIO_PLAY_CODEC_TYPE_MP3;
    param.playParam.store = AUDIO_PLAY_FILE;
    param.playParam.sampleRate = SAMPLERATE_16K;
    s_open_player = openPlayCreate(&param);
}
static void ui_scrMusicPlayer_load_event_cb(lv_event_t* e)
{
    s_open_player_event = osEventFlagsNew(NULL);
    if(s_open_player_event == NULL)
    {
        SYSLOG_ERR("Failed to create event flags\r\n");
        return;
    }
    openPlayerConfigT param = {0};
    param.playParam.codec = AUDIO_PLAY_CODEC_TYPE_MP3;
    param.playParam.store = AUDIO_PLAY_FILE;
    param.playParam.sampleRate = SAMPLERATE_16K;
    s_open_player = openPlayCreate(&param);
    openPlaySetCallback(s_open_player, open_player_callback, NULL);
    if(!s_open_player)
    {
        SYSLOG_ERR("openPlayCreate failed\r\n");
        return;
    }
    if(openPlayerGetStatus(s_open_player) != OPEN_PLAY_STA_IDLE)
    {
        SYSLOG_ERR("player is not idle\r\n");
        openPlayStop(s_open_player);
        osEventFlagsWait(s_open_player_event, 0x01, osFlagsWaitAny, 2000);
    }
    mp3_info_refresh();
    openPlay(s_open_player, s_mp3_files[s_cur_mps_index]);
    ui_set_play_state(UI_PLAY_STATE_PLAY, false);
}

static void ui_scrMusicPlayer_unloaded_event_cb(lv_event_t* e)
{
    if(s_open_player)
    {
        if(openPlayerGetStatus(s_open_player) != OPEN_PLAY_STA_IDLE)
        {
            SYSLOG_ERR("player is not idle\r\n");
            openPlayStop(s_open_player);
            osEventFlagsWait(s_open_player_event, 0x01, osFlagsWaitAny, 2000);
        }
        s_open_player = NULL;
    }

    if(s_open_player_event)
    {
        osEventFlagsDelete(s_open_player_event);
        s_open_player_event = NULL;
    }
    if(s_playback_timer)
    {
        lv_timer_del(s_playback_timer);
        s_playback_timer = NULL;
    }
}

static void ui_exit_btn_click_cb(lv_event_t* e)
{
    if(ui_scrFileList)
    {
        lv_scr_load_anim(ui_scrFileList, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 300, 0,
                         false);
    }
    else
    {
        // 如果文件列表屏幕已被销毁，重新初始化
        ui_scrFileList_screen_init();
        lv_scr_load_anim(ui_scrFileList, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 300, 0,
                         false);
    }
}

static void ui_btn_scale_effect_cb(lv_event_t* e)
{
    lv_obj_t* btn = lv_event_get_target(e);
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, btn);
    lv_anim_set_time(&a, 100);

    if(e->code == LV_EVENT_PRESSED)
    {
        // 按下时缩小
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_img_set_zoom);
        lv_anim_set_values(&a, 256, 220);
        lv_anim_start(&a);

        // 添加半透明效果
        lv_obj_set_style_img_opa(btn, LV_OPA_70, 0);
    }
    else if(e->code == LV_EVENT_RELEASED || e->code == LV_EVENT_CLICKED)
    {
        // 释放时恢复
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_img_set_zoom);
        lv_anim_set_values(&a, 220, 256);
        lv_anim_start(&a);

        // 恢复透明度
        lv_obj_set_style_img_opa(btn, LV_OPA_100, 0);
    }
}
static void ui_btn_glow_effect_cb(lv_event_t* e)
{
    lv_obj_t* btn = lv_event_get_target(e);

    if(e->code == LV_EVENT_PRESSED)
    {
        // 添加发光效果（通过阴影和边框）
        lv_obj_set_style_shadow_width(btn, 10, 0);
        lv_obj_set_style_shadow_color(btn, lv_color_hex(0x3498DB), 0);
        lv_obj_set_style_shadow_opa(btn, LV_OPA_80, 0);
        lv_obj_set_style_border_width(btn, 2, 0);
        lv_obj_set_style_border_color(btn, lv_color_hex(0xECF0F1), 0);

        // 稍微增加亮度
        lv_obj_set_style_img_recolor(btn, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_img_recolor_opa(btn, LV_OPA_30, 0);
    }
    else if(e->code == LV_EVENT_RELEASED || e->code == LV_EVENT_CLICKED)
    {
        // 移除发光效果
        lv_obj_set_style_shadow_width(btn, 0, 0);
        lv_obj_set_style_border_width(btn, 0, 0);
        lv_obj_set_style_img_recolor_opa(btn, LV_OPA_0, 0);
    }
}

// 播放按钮特殊效果（带旋转）
static void ui_play_btn_effect_cb(lv_event_t* e)
{
    lv_obj_t* btn = lv_event_get_target(e);

    if(e->code == LV_EVENT_PRESSED)
    {
        // 按下时缩小并改变透明度
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, btn);
        lv_anim_set_time(&a, 100);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_img_set_zoom);
        lv_anim_set_values(&a, 256, 230);
        lv_anim_start(&a);

        lv_obj_set_style_img_opa(btn, LV_OPA_80, 0);
    }
    else if(e->code == LV_EVENT_RELEASED)
    {
        // 释放时恢复
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, btn);
        lv_anim_set_time(&a, 100);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_img_set_zoom);
        lv_anim_set_values(&a, 230, 256);
        lv_anim_start(&a);

        lv_obj_set_style_img_opa(btn, LV_OPA_100, 0);
    }
    else if(e->code == LV_EVENT_CLICKED)
    {
        // 点击后额外效果
        SYSLOG_INFO("Play button clicked\n");

// 创建涟漪效果（如果支持）
#if LV_USE_LVGL_VERSION >= 80000
        lv_obj_add_state(btn, LV_STATE_FOCUSED);
#endif
    }
}

// 上一首按钮点击回调
static void ui_last_btn_click_cb(lv_event_t* e)
{
    SYSLOG_INFO("Next track\r\n");
    if(s_open_player)
    {
        if(openPlayerGetStatus(s_open_player) != OPEN_PLAY_STA_IDLE)
        {
            SYSLOG_ERR("player is not idle\r\n");
            openPlayStop(s_open_player);
            osEventFlagsWait(s_open_player_event, 0x01, osFlagsWaitAny, 2000);
        }
        if(s_playback_timer)
        {
            lv_timer_del(s_playback_timer);
            s_playback_timer = NULL;
        }

        if(s_cur_mps_index == 0)
        {
            s_cur_mps_index = s_mp3_file_count - 1;
        }
        else
        {
            s_cur_mps_index--;
        }
        mp3_info_refresh();
        open_player_set_param();
        openPlay(s_open_player, s_mp3_files[s_cur_mps_index]);
        ui_set_play_state(UI_PLAY_STATE_PLAY, false);
    }
}

// 播放/暂停按钮点击回调
static void ui_play_btn_click_cb(lv_event_t* e)
{
    if(s_play_state == UI_PLAY_STATE_PLAY)
    {
        s_is_pause = true;
        openPlayPause(s_open_player, true);
    }
    else if(s_play_state == UI_PLAY_STATE_PAUSE)
    {
        openPlayPause(s_open_player, false);
        ui_set_play_state(UI_PLAY_STATE_PLAY, true);
    }
    else
    {
        open_player_set_param();
        openPlay(s_open_player, s_mp3_files[s_cur_mps_index]);
        ui_set_play_state(UI_PLAY_STATE_PLAY, false);
    }
}

// 下一首按钮点击回调
static void ui_next_btn_click_cb(lv_event_t* e)
{
    SYSLOG_INFO("Next track\n");
    if(s_open_player)
    {
        if(openPlayerGetStatus(s_open_player) != OPEN_PLAY_STA_IDLE)
        {
            SYSLOG_ERR("player is not idle\r\n");
            openPlayStop(s_open_player);
            osEventFlagsWait(s_open_player_event, 0x01, osFlagsWaitAny, 2000);
        }
        if(s_playback_timer)
        {
            lv_timer_del(s_playback_timer);
            s_playback_timer = NULL;
        }
        s_cur_mps_index++;
        if(s_cur_mps_index >= s_mp3_file_count)
        {
            s_cur_mps_index = 0;
        }
        mp3_info_refresh();
        open_player_set_param();
        openPlay(s_open_player, s_mp3_files[s_cur_mps_index]);
        ui_set_play_state(UI_PLAY_STATE_PLAY, false);
    }
}

void ui_scrMusicPlayer_screen_init(void)
{
    ui_scrMusicPlayer = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_scrMusicPlayer, LV_OBJ_FLAG_SCROLLABLE);  /// Flags

    lv_obj_set_style_bg_color(ui_scrMusicPlayer, lv_color_hex(0x1A2A3A), 0);
    lv_obj_set_style_bg_grad_color(ui_scrMusicPlayer, lv_color_hex(0x0F1A24),
                                   0);
    lv_obj_set_style_bg_grad_dir(ui_scrMusicPlayer, LV_GRAD_DIR_VER, 0);

    ui_ibExit = lv_img_create(ui_scrMusicPlayer);
    lv_img_set_src(ui_ibExit, UI_IMG_EXIT_PNG);
    lv_obj_set_width(ui_ibExit, LV_SIZE_CONTENT);   /// 32
    lv_obj_set_height(ui_ibExit, LV_SIZE_CONTENT);  /// 32
    lv_obj_set_x(ui_ibExit, -100);
    lv_obj_set_y(ui_ibExit, 0);
    lv_obj_set_align(ui_ibExit, LV_ALIGN_TOP_MID);
    lv_obj_add_flag(ui_ibExit, LV_OBJ_FLAG_ADV_HITTEST);   /// Flags
    lv_obj_clear_flag(ui_ibExit, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
    lv_obj_add_flag(ui_ibExit, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(ui_ibExit, ui_exit_btn_click_cb, LV_EVENT_CLICKED,
                        NULL);
    // 添加退出按钮效果
    lv_obj_add_event_cb(ui_ibExit, ui_btn_scale_effect_cb, LV_EVENT_PRESSED,
                        NULL);
    lv_obj_add_event_cb(ui_ibExit, ui_btn_scale_effect_cb, LV_EVENT_RELEASED,
                        NULL);
    lv_obj_add_event_cb(ui_ibExit, ui_exit_btn_click_cb, LV_EVENT_CLICKED,
                        NULL);

    ui_ibPlay = lv_img_create(ui_scrMusicPlayer);
    lv_img_set_src(ui_ibPlay, UI_IMG_PLAY_PNG);
    lv_obj_set_width(ui_ibPlay, LV_SIZE_CONTENT);   /// 64
    lv_obj_set_height(ui_ibPlay, LV_SIZE_CONTENT);  /// 64
    lv_obj_set_x(ui_ibPlay, 0);
    lv_obj_set_y(ui_ibPlay, 50);
    lv_obj_set_align(ui_ibPlay, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_ibPlay, LV_OBJ_FLAG_ADV_HITTEST);   /// Flags
    lv_obj_clear_flag(ui_ibPlay, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
    lv_obj_add_flag(ui_ibPlay, LV_OBJ_FLAG_CLICKABLE);
    // 添加播放按钮效果
    lv_obj_add_event_cb(ui_ibPlay, ui_play_btn_effect_cb, LV_EVENT_PRESSED,
                        NULL);
    lv_obj_add_event_cb(ui_ibPlay, ui_play_btn_effect_cb, LV_EVENT_RELEASED,
                        NULL);
    lv_obj_add_event_cb(ui_ibPlay, ui_play_btn_click_cb, LV_EVENT_CLICKED,
                        NULL);

    ui_ibLast = lv_img_create(ui_scrMusicPlayer);
    lv_img_set_src(ui_ibLast, UI_IMG_LAST_PNG);
    lv_obj_set_width(ui_ibLast, LV_SIZE_CONTENT);   /// 32
    lv_obj_set_height(ui_ibLast, LV_SIZE_CONTENT);  /// 32
    lv_obj_set_x(ui_ibLast, -80);
    lv_obj_set_y(ui_ibLast, 50);
    lv_obj_set_align(ui_ibLast, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_ibLast, LV_OBJ_FLAG_ADV_HITTEST);   /// Flags
    lv_obj_clear_flag(ui_ibLast, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
    lv_obj_add_flag(ui_ibLast, LV_OBJ_FLAG_CLICKABLE);
    // 添加上一首按钮效果
    lv_obj_add_event_cb(ui_ibLast, ui_btn_glow_effect_cb, LV_EVENT_PRESSED,
                        NULL);
    lv_obj_add_event_cb(ui_ibLast, ui_btn_glow_effect_cb, LV_EVENT_RELEASED,
                        NULL);
    lv_obj_add_event_cb(ui_ibLast, ui_last_btn_click_cb, LV_EVENT_CLICKED,
                        NULL);

    ui_ibNext = lv_img_create(ui_scrMusicPlayer);
    lv_img_set_src(ui_ibNext, UI_IMG_NEXT_PNG);
    lv_obj_set_width(ui_ibNext, LV_SIZE_CONTENT);   /// 32
    lv_obj_set_height(ui_ibNext, LV_SIZE_CONTENT);  /// 32
    lv_obj_set_x(ui_ibNext, 80);
    lv_obj_set_y(ui_ibNext, 50);
    lv_obj_set_align(ui_ibNext, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_ibNext, LV_OBJ_FLAG_ADV_HITTEST);   /// Flags
    lv_obj_clear_flag(ui_ibNext, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
    lv_obj_add_flag(ui_ibNext, LV_OBJ_FLAG_CLICKABLE);
    // 添加下一首按钮效果
    lv_obj_add_event_cb(ui_ibNext, ui_btn_glow_effect_cb, LV_EVENT_PRESSED,
                        NULL);
    lv_obj_add_event_cb(ui_ibNext, ui_btn_glow_effect_cb, LV_EVENT_RELEASED,
                        NULL);
    lv_obj_add_event_cb(ui_ibNext, ui_next_btn_click_cb, LV_EVENT_CLICKED,
                        NULL);

    ui_labCurTime = lv_label_create(ui_scrMusicPlayer);
    lv_obj_set_width(ui_labCurTime, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_labCurTime, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_x(ui_labCurTime, 15);
    lv_obj_set_y(ui_labCurTime, 80);
    lv_obj_set_align(ui_labCurTime, LV_ALIGN_LEFT_MID);
    lv_label_set_text(ui_labCurTime, "00:00");

    ui_labTotalTime = lv_label_create(ui_scrMusicPlayer);
    lv_obj_set_width(ui_labTotalTime, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_labTotalTime, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_x(ui_labTotalTime, -25);
    lv_obj_set_y(ui_labTotalTime, 80);
    lv_obj_set_align(ui_labTotalTime, LV_ALIGN_RIGHT_MID);
    lv_label_set_text(ui_labTotalTime, "03:00");

    ui_sliTime = lv_slider_create(ui_scrMusicPlayer);
    lv_obj_clear_flag(ui_sliTime, LV_OBJ_FLAG_CLICKABLE);
    lv_slider_set_value(ui_sliTime, 0, LV_ANIM_OFF);
    if(lv_slider_get_mode(ui_sliTime) == LV_SLIDER_MODE_RANGE)
        lv_slider_set_left_value(ui_sliTime, 0, LV_ANIM_OFF);
    lv_obj_set_width(ui_sliTime, 200);
    lv_obj_set_height(ui_sliTime, 10);
    lv_obj_set_x(ui_sliTime, 0);
    lv_obj_set_y(ui_sliTime, 100);
    lv_obj_set_align(ui_sliTime, LV_ALIGN_CENTER);

    ui_imgMusicPlay = lv_img_create(ui_scrMusicPlayer);
    lv_img_set_src(ui_imgMusicPlay, UI_IMG_MUSIC_PLAYER_PNG);
    lv_obj_set_width(ui_imgMusicPlay, LV_SIZE_CONTENT);   /// 96
    lv_obj_set_height(ui_imgMusicPlay, LV_SIZE_CONTENT);  /// 96
    lv_obj_set_x(ui_imgMusicPlay, 0);
    lv_obj_set_y(ui_imgMusicPlay, 10);
    lv_obj_set_align(ui_imgMusicPlay, LV_ALIGN_TOP_MID);
    lv_obj_add_flag(ui_imgMusicPlay, LV_OBJ_FLAG_ADV_HITTEST);   /// Flags
    lv_obj_clear_flag(ui_imgMusicPlay, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
    // 美化专辑图片
    lv_obj_set_style_border_width(ui_imgMusicPlay, 2, 0);
    lv_obj_set_style_border_color(ui_imgMusicPlay, lv_color_hex(0x3498DB), 0);
    lv_obj_set_style_radius(ui_imgMusicPlay, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_shadow_width(ui_imgMusicPlay, 5, 0);
    lv_obj_set_style_shadow_color(ui_imgMusicPlay, lv_color_hex(0x000000), 0);

    ui_lbMusicTitle = lv_label_create(ui_scrMusicPlayer);
    lv_obj_set_width(ui_lbMusicTitle, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_lbMusicTitle, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_align(ui_lbMusicTitle, LV_ALIGN_CENTER);
    lv_label_set_text(ui_lbMusicTitle, "music.mp3");
    lv_obj_set_style_text_color(ui_lbMusicTitle, lv_color_hex(0xECF0F1), 0);
    lv_label_set_long_mode(ui_lbMusicTitle, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_add_event_cb(ui_scrMusicPlayer, ui_scrMusicPlayer_load_event_cb,
                        LV_EVENT_SCREEN_LOADED, NULL);
    lv_obj_add_event_cb(ui_scrMusicPlayer, ui_scrMusicPlayer_unloaded_event_cb,
                        LV_EVENT_SCREEN_UNLOADED, NULL);
}

void ui_scrMusicPlayer_screen_destroy(void)
{
    if(ui_scrMusicPlayer)
    {
        lv_obj_del(ui_scrMusicPlayer);
    }
    // NULL screen variables
    ui_scrMusicPlayer = NULL;
    ui_ibExit = NULL;
    ui_ibPlay = NULL;
    ui_ibLast = NULL;
    ui_ibNext = NULL;
    ui_labCurTime = NULL;
    ui_labTotalTime = NULL;
    ui_sliTime = NULL;
    ui_imgMusicPlay = NULL;
    ui_lbMusicTitle = NULL;
}

void ui_mplayer_init(void)
{
    lv_disp_t* dispp = lv_disp_get_default();
    lv_theme_t* theme = lv_theme_default_init(
        dispp, lv_palette_main(LV_PALETTE_BLUE),
        lv_palette_main(LV_PALETTE_RED), true, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    ui_scrFileList_screen_init();
    ui_scrMusicPlayer_screen_init();
}

void ui_mplayer_destroy(void)
{
    ui_scrFileList_screen_destroy();
    ui_scrMusicPlayer_screen_destroy();
}
