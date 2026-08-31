
#include "../ui.h"
#include <stdio.h>
#include <stdlib.h>
#include "ostask.h"
#include "osasys.h"
#include "bsp.h"
#include "bsp_custom.h"
#include "storage.h"
#ifdef FEATURE_SUBSYS_MEDIA_ENABLE
#include "media.h"
extern audioPlayStatusInfo audioStatusInfo;
#endif

lv_obj_t *ui_Music_Player;
lv_obj_t *ui_Music_Title;
lv_obj_t *ui_Author;
lv_obj_t *ui_Play_btn;
lv_obj_t *ui_Play;
lv_obj_t *ui_Album;
lv_obj_t *ui_Backward;
lv_obj_t *ui_Forward;
static lv_obj_t *ui_Slider;
static lv_obj_t * ui_Slider_total;
static lv_obj_t *ui_Slider_timer;   //show time
static lv_timer_t  *sec_counter_timer;

static uint16_t slider_totals = 100;
static uint16_t slider_timeval = 0;
static uint32_t slider_rate = 16*1000;
LV_IMG_DECLARE(ui_img_music_btn_play);
LV_IMG_DECLARE(ui_img_music_btn_pause);
LV_IMG_DECLARE(ui_img_music_slider_knob);

// static char mp3_list[3][12] = {"dukou.mp3","xhls.mp3","in.mp3"};
#define MP3_LIST_NUM_MAX        3
#define MP3_NAME_LEN_MAX        15

typedef struct
{
    uint16_t index;     //编号索引
    uint16_t totals;    //播放时长
    uint32_t nbytes;    //文件大小
    char name[MP3_NAME_LEN_MAX];
    char disk[3];       //盘符 "D:/"
} music_data_t;

static char mp3_list[MP3_LIST_NUM_MAX][MP3_NAME_LEN_MAX] = {0};
static uint8_t play_ptr = 0;
static bool mp3playing = false;

/**
  \fn          
  \brief        
  \return
*/
void audioFileListGet(void)
{
    uint8_t found_cnt = 0;
    struct lfs_info *info = NULL;
    INT32 errCode = -1;
    DIR *dir = opendir("D:/");
    if(dir == NULL)
    {
        LV_LOG_USER("disk D open error");
    }
    while(true)
    {
        info = (struct lfs_info *)readdir(dir);
        if(info == NULL) break;
        if(strstr(info->name,".mp3") && info->size>10*1024)
        {
            if(found_cnt < MP3_LIST_NUM_MAX && strlen(info->name)<MP3_NAME_LEN_MAX)
            {
                memset(mp3_list[found_cnt],0,MP3_NAME_LEN_MAX);
                memcpy(mp3_list[found_cnt],info->name,strlen(info->name));
                found_cnt++;
            }
            // LV_LOG_USER("%d,%s\t%u", found_cnt,info->name, info->size);
        }
    }
    errCode = closedir(dir);
    if(errCode){
    }
}     

/**
  \fn          
  \brief        
  \return
*/    
void _ui_music_timer_clear(void)
{
    lv_slider_set_value(ui_Slider, 0, LV_ANIM_OFF);
    lv_label_set_text(ui_Slider_timer, "0:00");   
    lv_obj_add_flag(ui_Slider_total, LV_OBJ_FLAG_HIDDEN);
    slider_timeval = 0;
    slider_totals = 0;
}

void _ui_music_end_cb(void)
{
    if(getMp3PlaySchedule() == 0)
    {
        slider_timeval = 0;
        slider_totals = 0;
        // LV_LOG_USER("%d,%d",slider_timeval,slider_totals);
    }
    mp3playing = false;
    lv_timer_pause(sec_counter_timer);
    lv_imgbtn_set_state(ui_Play_btn, LV_IMGBTN_STATE_RELEASED);
    // LV_LOG_USER("%d/%d,%d/%d",slider_timeval,getMp3PlaySchedule(),audioStatusInfo.decodeSize,audioStatusInfo.fileSize);
}

void _ui_music_resume(void)
{
    // lv_slider_set_range(slider_obj, 0, _lv_demo_music_get_track_length(track_id));
    lv_timer_resume(sec_counter_timer);
    lv_imgbtn_set_state(ui_Play_btn, LV_IMGBTN_STATE_CHECKED_RELEASED);
    #ifdef FEATURE_SUBSYS_MP3_ENABLE
    if(!mp3playing){
        mp3playing = true;
        char folder[15] = "D:/";
        strcat(folder, mp3_list[play_ptr]);
        audioPlayMp3(folder,_ui_music_end_cb, true, true);
    }
    #endif
}

void _ui_music_pause(void)
{
    lv_timer_pause(sec_counter_timer);
    lv_imgbtn_set_state(ui_Play_btn, LV_IMGBTN_STATE_RELEASED);
    // LV_LOG_USER("%d,%d,%d,%d/%d",mp3playing,slider_timeval,getMp3PlaySchedule(),audioStatusInfo.decodeSize,audioStatusInfo.fileSize);
    #ifdef FEATURE_SUBSYS_MP3_ENABLE
    if(mp3playing){
        audioStopPlay();
        mp3playing = false;
    }
    #endif  
}

static void play_event_click_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    if(lv_obj_has_state(obj, LV_STATE_CHECKED)) {
        _ui_music_resume();
    }
    else {
        _ui_music_pause();
    }
}

static void timer_cb(lv_timer_t * t)
{
    LV_UNUSED(t);
    // uint16_t music_long = mp3PlayTimeGet()/1000;
    // if (music_long && slider_totals != music_long)
    // {
    //     slider_totals = music_long;
    //     lv_obj_clear_flag(ui_Slider_total, LV_OBJ_FLAG_HIDDEN);
    //     lv_label_set_text_fmt(ui_Slider_total, "%"LV_PRIu32":%02"LV_PRIu32, slider_totals / 60, slider_totals % 60);
    //     // LV_LOG_USER("%d/%d,%d/%d",slider_timeval,slider_totals,audioStatusInfo.decodeSize,audioStatusInfo.fileSize);
    // }
    slider_timeval++;
    lv_label_set_text_fmt(ui_Slider_timer, "%"LV_PRIu32":%02"LV_PRIu32, slider_timeval / 60, slider_timeval % 60);
    lv_slider_set_value(ui_Slider, getMp3PlaySchedule(), LV_ANIM_ON);
}

static void music_btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    lv_obj_t * kb = lv_event_get_user_data(e);
    if (code == LV_EVENT_CLICKED)
    {
        if(ta->user_data==2) 
        {
            if(play_ptr)
            {
                _ui_music_pause();
                _ui_music_timer_clear();
                play_ptr--;
                // LV_LOG_USER("pre: %d",  play_ptr);
            } 
            lv_label_set_text(ui_Music_Title,mp3_list[play_ptr]);
        } 
        if(ta->user_data==3) 
        {
            if(play_ptr<2){
                _ui_music_pause();
                _ui_music_timer_clear();
                play_ptr++;
                // LV_LOG_USER("next: %d",  play_ptr);
            } 
            lv_label_set_text(ui_Music_Title,mp3_list[play_ptr]);
        } 
    }
}

void ui_Music_Player_screen_init(void)
{
    audioFileListGet();
    ui_Music_Player = lv_obj_create(NULL);
    lv_obj_clear_flag( ui_Music_Player, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_obj_set_style_bg_color(ui_Music_Player, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(ui_Music_Player, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src( ui_Music_Player, &ui_img_pattern_png, LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_img_tiled(ui_Music_Player, true, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_Album = lv_img_create(ui_Music_Player);
    lv_img_set_src(ui_Album, &ui_img_album_png);
    lv_obj_set_width( ui_Album, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height( ui_Album, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_x( ui_Album, 0 );
    lv_obj_set_y( ui_Album, 24 );
    lv_obj_set_align( ui_Album, LV_ALIGN_TOP_MID );
    lv_obj_add_flag( ui_Album, LV_OBJ_FLAG_ADV_HITTEST );   /// Flags
    lv_obj_clear_flag( ui_Album, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_obj_set_style_radius(ui_Album, 300, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_Album, lv_color_hex(0xD5D2D5), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_shadow_opa(ui_Album, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui_Album, 30, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui_Album, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_x(ui_Album, 3, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(ui_Album, 6, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_Music_Title = ui_Small_Label_create(ui_Music_Player);
    lv_obj_set_x( ui_Music_Title, 0 );
    lv_obj_set_y( ui_Music_Title, 0 );
    lv_obj_set_align( ui_Music_Title, LV_ALIGN_CENTER );
    if(play_ptr>1) lv_label_set_text(ui_Music_Title,mp3_list[1]);
    else lv_label_set_text(ui_Music_Title,mp3_list[0]);
    lv_obj_set_style_text_color(ui_Music_Title, lv_color_hex(0x000746), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(ui_Music_Title, 255, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_Slider = lv_slider_create(ui_Music_Player);
    lv_obj_add_flag(ui_Slider, LV_OBJ_FLAG_CLICKABLE); 
    lv_obj_set_height(ui_Slider, 3);
    lv_slider_set_range(ui_Slider, 0, 100);
    // lv_obj_set_style_anim_time(ui_Slider, slider_totals, 0);
    lv_obj_set_width(ui_Slider, 160);
    lv_obj_align_to(ui_Slider, ui_Music_Title, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

    lv_obj_set_style_bg_img_src(ui_Slider, &ui_img_music_slider_knob, LV_PART_KNOB);
    lv_obj_set_style_bg_opa(ui_Slider, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_set_style_pad_all(ui_Slider, 20, LV_PART_KNOB);

    ui_Slider_total = lv_label_create(ui_Music_Player);
    lv_obj_set_style_text_font(ui_Slider_total, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(ui_Slider_total, lv_color_hex(0x293062), 0);
    lv_label_set_text_fmt(ui_Slider_total, "%"LV_PRIu32":%02"LV_PRIu32, slider_totals / 60, slider_totals % 60);
    lv_obj_align_to(ui_Slider_total, ui_Slider, LV_ALIGN_OUT_TOP_RIGHT, 0, 0);
    lv_obj_add_flag(ui_Slider_total, LV_OBJ_FLAG_HIDDEN);

    ui_Slider_timer = lv_label_create(ui_Music_Player);
    lv_obj_set_style_text_font(ui_Slider_timer, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(ui_Slider_timer, lv_color_hex(0x293062), 0);
    lv_label_set_text(ui_Slider_timer, "0:00");
    lv_obj_align_to(ui_Slider_timer, ui_Slider, LV_ALIGN_OUT_TOP_MID, 0, 0);

    sec_counter_timer = lv_timer_create(timer_cb, 1000, NULL);
    lv_timer_pause(sec_counter_timer);

    ui_Play_btn = lv_imgbtn_create(ui_Music_Player);
    lv_imgbtn_set_src(ui_Play_btn, LV_IMGBTN_STATE_RELEASED, NULL, &ui_img_music_btn_play, NULL);
    lv_imgbtn_set_src(ui_Play_btn, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &ui_img_music_btn_pause, NULL);
    lv_obj_add_flag(ui_Play_btn, LV_OBJ_FLAG_CHECKABLE);

    lv_obj_add_event_cb(ui_Play_btn, play_event_click_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(ui_Play_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_width(ui_Play_btn, ui_img_music_btn_play.header.w);
    // lv_obj_set_x( ui_Play_btn, 0 );
    // lv_obj_set_y( ui_Play_btn, -36 );
    lv_obj_set_pos( ui_Play_btn, 0 , -36);
    lv_obj_set_align( ui_Play_btn, LV_ALIGN_BOTTOM_MID );
    lv_obj_clear_flag( ui_Play_btn, LV_OBJ_FLAG_SCROLLABLE );    /// Flags

    ui_Backward = lv_img_create(ui_Music_Player);
    lv_img_set_src(ui_Backward, &ui_img_backward_png);
    lv_obj_set_width( ui_Backward, LV_SIZE_CONTENT);    
    lv_obj_set_height( ui_Backward, LV_SIZE_CONTENT);   
    lv_obj_set_x( ui_Backward, 30 );
    lv_obj_set_y( ui_Backward, -60 );
    lv_obj_set_align( ui_Backward, LV_ALIGN_BOTTOM_LEFT );
    lv_obj_add_flag( ui_Backward, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_ADV_HITTEST );   /// Flags
    lv_obj_clear_flag( ui_Backward, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_obj_set_style_img_recolor(ui_Backward, lv_color_hex(0x293062), LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor_opa(ui_Backward, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor(ui_Backward, lv_color_hex(0x515EB5), LV_PART_MAIN| LV_STATE_PRESSED);
    lv_obj_set_style_img_recolor_opa(ui_Backward, 255, LV_PART_MAIN| LV_STATE_PRESSED);

    lv_obj_set_user_data(ui_Backward, (uint16_t)2);
    lv_obj_add_event_cb(ui_Backward, music_btn_event_cb,LV_EVENT_CLICKED,NULL);

    ui_Forward = lv_img_create(ui_Music_Player);
    lv_img_set_src(ui_Forward, &ui_img_forward_png);
    lv_obj_set_width( ui_Forward, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height( ui_Forward, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_x( ui_Forward, -30 );
    lv_obj_set_y( ui_Forward, -60 );
    lv_obj_set_align( ui_Forward, LV_ALIGN_BOTTOM_RIGHT );
    lv_obj_add_flag( ui_Forward, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_ADV_HITTEST );   /// Flags
    lv_obj_clear_flag( ui_Forward, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_obj_set_style_img_recolor(ui_Forward, lv_color_hex(0x293062), LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor_opa(ui_Forward, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor(ui_Forward, lv_color_hex(0x515EB5), LV_PART_MAIN| LV_STATE_PRESSED);
    lv_obj_set_style_img_recolor_opa(ui_Forward, 255, LV_PART_MAIN| LV_STATE_PRESSED);
    lv_obj_set_user_data(ui_Forward, (uint16_t)3);
    lv_obj_add_event_cb(ui_Forward, music_btn_event_cb,LV_EVENT_CLICKED,NULL);

    lv_obj_add_event_cb(ui_Music_Player, ui_event_Music_Player, LV_EVENT_ALL, NULL);
}

void ui_event_Music_Player( lv_event_t * e) 
{
    lv_event_code_t event_code = lv_event_get_code(e);lv_obj_t * target = lv_event_get_target(e);
    if ( event_code == LV_EVENT_SCREEN_LOADED) 
    {
        // Up_Animation(ui_Album, 100);
        // Up_Animation(ui_Music_Title, 200);
        // Up_Animation(ui_Author, 300);
        // Up_Animation(ui_Play_btn, 100);
        // Up_Animation(ui_Forward, 1500);
        // Up_Animation(ui_Backward, 1800);
    }
    if ( event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT) 
    {
        lv_indev_wait_release(lv_indev_get_act());
        _ui_screen_change(&ui_Menu1, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Menu1_screen_init); 
    }
    else if( event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_LEFT) 
    {
        lv_indev_wait_release(lv_indev_get_act());
        _ui_screen_change(&ui_Menu1, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Menu1_screen_init); 
    }
}