
#include "../ui.h"
#include <stdio.h>
#include <stdlib.h>
#include "ostask.h"
#include "osasys.h"
#include "bsp.h"
#include "bsp_custom.h"

#ifdef FEATURE_SUBSYS_OPENPLAYER_API_ENABLE
#include "openplayer.h"
#endif
#ifdef FEATURE_SUBSYS_OPENRECORDER_API_ENABLE
#include "openrecorder.h"
#endif
#include "audio.h"

lv_obj_t *ui_Recorder;
lv_obj_t *Recorder_Title;
lv_obj_t *ui_Record_btn;
lv_obj_t *ui_Record_btn_name;
lv_obj_t *ui_Player_btn;
lv_obj_t *ui_Player_btn_name;

// static lv_obj_t *ui_Slider;
static lv_obj_t *ui_Record_timer;
static lv_timer_t  * sec_counter_timer;

static bool isrecording = false;
static bool recordplaying = false;

static uint16_t slider_totals = 100;
static uint16_t slider_timeval;
static bool start_flag = false;

LV_IMG_DECLARE(ui_img_music_btn_play);
LV_IMG_DECLARE(ui_img_music_btn_pause);
LV_IMG_DECLARE(ui_img_music_slider_knob);

#ifdef FEATURE_SUBSYS_OPENPLAYER_API_ENABLE
openPlayer gUiPlayHandler = NULL;
#endif

#ifdef FEATURE_SUBSYS_OPENRECORDER_API_ENABLE
openRecorder gUiRecordHandler = NULL;
static void _ui_record_end_cb(void)
{
    isrecording = false;
    lv_timer_pause(sec_counter_timer);
    lv_imgbtn_set_state(ui_Record_btn, LV_IMGBTN_STATE_RELEASED);
    lv_imgbtn_set_state(ui_Player_btn, LV_IMGBTN_STATE_RELEASED);
    // LV_LOG_USER("%d/%d,%d/%d",slider_timeval,getMp3PlaySchedule(),audioStatusInfo.decodeSize,audioStatusInfo.fileSize);
}

static void _ui_record_play_end_cb(void)
{
    recordplaying = false;
    lv_imgbtn_set_state(ui_Player_btn, LV_IMGBTN_STATE_RELEASED);
    lv_imgbtn_set_state(ui_Record_btn, LV_IMGBTN_STATE_RELEASED);
}
#endif


void _ui_record_pause(void)
{
    isrecording = false;
    lv_imgbtn_set_state(ui_Record_btn, LV_IMGBTN_STATE_RELEASED);
    lv_imgbtn_set_state(ui_Player_btn, LV_IMGBTN_STATE_RELEASED);
    lv_timer_pause(sec_counter_timer);
#ifdef FEATURE_SUBSYS_OPENRECORDER_API_ENABLE
	openRecorderStop(gUiRecordHandler);
#endif  
}
void _ui_record_play_pause(void)
{
    recordplaying = false;
    lv_imgbtn_set_state(ui_Player_btn, LV_IMGBTN_STATE_RELEASED);
    lv_imgbtn_set_state(ui_Record_btn, LV_IMGBTN_STATE_RELEASED);
#ifdef FEATURE_SUBSYS_OPENPLAYER_API_ENABLE
	openPlayStop(gUiPlayHandler);
#endif
}


void _ui_record_play_resume(void)
{
    if(isrecording){
        _ui_record_pause();
    }
    recordplaying = true;
    lv_imgbtn_set_state(ui_Player_btn, LV_IMGBTN_STATE_CHECKED_RELEASED);
    lv_imgbtn_set_state(ui_Record_btn, LV_IMGBTN_STATE_DISABLED);

#ifdef FEATURE_SUBSYS_OPENPLAYER_API_ENABLE
	openPlayerConfigT opParam = {0};
	opParam.playParam.sampleRate = SAMPLERATE_16K;//default,it will change when decode if not pcm
	opParam.playParam.store = AUDIO_PLAY_FILE;

	gUiPlayHandler = openPlayCreate(&opParam);
	openPlaySetCallback(gUiPlayHandler,_ui_record_play_end_cb,NULL);
	openPlay(gUiPlayHandler,"D:/record.amr");
#endif
}
/**
  \fn          
  \brief        
  \return
*/
void _ui_record_resume(void)
{
    isrecording = true;
    lv_imgbtn_set_state(ui_Record_btn, LV_IMGBTN_STATE_CHECKED_RELEASED);
    lv_imgbtn_set_state(ui_Player_btn, LV_IMGBTN_STATE_DISABLED);
    start_flag = false;
    lv_timer_resume(sec_counter_timer);
#ifdef FEATURE_SUBSYS_OPENRECORDER_API_ENABLE
	openRecorderConfigT param = {0};
	memset(&param,0x00,sizeof(openRecorderConfigT));
	param.recordParam.recordTime = 60;
	gUiRecordHandler = openRecorderCreate(&param);
	openRecorderSetCallback(gUiRecordHandler,_ui_record_end_cb,NULL);
	openRecorderStart(gUiRecordHandler,"D:/record.amr");
#endif
}

static void play_event_click_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    if(lv_obj_has_state(obj, LV_STATE_CHECKED)) {
        _ui_record_play_resume();
    }
    else {
        _ui_record_play_pause();
    }
}

static void record_event_click_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    if(lv_obj_has_state(obj, LV_STATE_CHECKED)) 
    {
        _ui_record_resume();
        // LV_LOG_USER("_ui_record_resume");
    }
    else {
        _ui_record_pause();
        // LV_LOG_USER("_ui_record_pause");
    }
}  
static void timer_cb(lv_timer_t * t)
{
    LV_UNUSED(t);
    if(start_flag)
    {
        slider_timeval++;
        lv_label_set_text_fmt(ui_Record_timer, "%"LV_PRIu32":%02"LV_PRIu32, slider_timeval / 60, slider_timeval % 60);
        // lv_slider_set_value(ui_Slider, slider_timeval, LV_ANIM_ON);
    } 
    else{
        start_flag = true;
        slider_timeval = 0;
    } 
}

void ui_event_Recorder(lv_event_t * e) 
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    if ( event_code == LV_EVENT_SCREEN_LOADED) 
    {
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

void ui_Recorder_screen_init(void)
{
    ui_Recorder = lv_obj_create(NULL);
    lv_obj_clear_flag( ui_Recorder, LV_OBJ_FLAG_SCROLLABLE );   
    lv_obj_set_style_bg_color(ui_Recorder, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(ui_Recorder, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src( ui_Recorder, &ui_img_pattern_png, LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_img_tiled(ui_Recorder, true, LV_PART_MAIN| LV_STATE_DEFAULT);

    Recorder_Title = ui_Small_Label_create(ui_Recorder);
    lv_obj_set_x( Recorder_Title, 0 );
    lv_obj_set_y( Recorder_Title, 20 );
    lv_obj_set_align( Recorder_Title, LV_ALIGN_TOP_MID );
    lv_label_set_text(Recorder_Title,"Recorder");
    lv_obj_set_style_text_color(Recorder_Title, lv_color_hex(0x000746), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(Recorder_Title, 255, LV_PART_MAIN| LV_STATE_DEFAULT);

    // ui_Slider = lv_slider_create(ui_Recorder);
    // lv_obj_set_style_anim_time(ui_Slider, slider_totals, 0);
    // lv_obj_add_flag(ui_Slider, LV_OBJ_FLAG_CLICKABLE); /*No input from the slider*/
    // lv_obj_set_height(ui_Slider, 3);
    // lv_obj_set_width(ui_Slider, LV_PCT(80));
    // lv_obj_set_align( ui_Slider, LV_ALIGN_CENTER );

    // lv_obj_set_style_bg_img_src(ui_Slider, &ui_img_music_slider_knob, LV_PART_KNOB);
    // lv_obj_set_style_bg_opa(ui_Slider, LV_OPA_TRANSP, LV_PART_KNOB);
    // lv_obj_set_style_pad_all(ui_Slider, 20, LV_PART_KNOB);

    // lv_obj_t * ui_Record_total = lv_label_create(ui_Recorder);
    // lv_obj_set_style_text_font(ui_Record_total, &lv_font_montserrat_12, 0);
    // lv_obj_set_style_text_color(ui_Record_total, lv_color_hex(0x293062), 0);
    // lv_obj_set_align( ui_Record_total, LV_ALIGN_RIGHT_MID );
    // lv_label_set_text_fmt(ui_Record_total, "%"LV_PRIu32":%02"LV_PRIu32, slider_totals / 60, slider_totals % 60);
    // lv_obj_align_to(ui_Record_total, ui_Slider, LV_ALIGN_OUT_TOP_RIGHT, 0, 0);

    ui_Record_timer = lv_label_create(ui_Recorder);
    lv_obj_set_style_text_font(ui_Record_timer, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(ui_Record_timer, lv_color_hex(0x293062), 0);
    lv_label_set_text(ui_Record_timer, "0:00");
    lv_obj_set_pos( ui_Record_timer, 0 , -10);
    lv_obj_set_align( ui_Record_timer, LV_ALIGN_CENTER );
    // lv_obj_align_to(ui_Record_timer, ui_Slider, LV_ALIGN_OUT_TOP_MID, 0, 0);
    // lv_obj_set_grid_cell(ui_Record_timer, LV_GRID_ALIGN_END, 5, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    sec_counter_timer = lv_timer_create(timer_cb, 1000, NULL);
    lv_timer_pause(sec_counter_timer);

    ui_Record_btn = lv_imgbtn_create(ui_Recorder);
    lv_imgbtn_set_src(ui_Record_btn, LV_IMGBTN_STATE_RELEASED, NULL, &ui_img_music_btn_play, NULL);
    lv_imgbtn_set_src(ui_Record_btn, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &ui_img_music_btn_pause, NULL);
    lv_obj_add_flag(ui_Record_btn, LV_OBJ_FLAG_CHECKABLE);

    lv_obj_add_event_cb(ui_Record_btn, record_event_click_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(ui_Record_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_width(ui_Record_btn, ui_img_music_btn_play.header.w);

    lv_obj_set_pos( ui_Record_btn, 30 , -60);
    lv_obj_set_align( ui_Record_btn, LV_ALIGN_BOTTOM_LEFT );
    lv_obj_clear_flag( ui_Record_btn, LV_OBJ_FLAG_SCROLLABLE );

    ui_Record_btn_name = lv_label_create(ui_Recorder);
    lv_label_set_text(ui_Record_btn_name,"Record");
    lv_obj_set_style_text_font(ui_Record_btn_name, &lv_font_montserrat_12, 0);
    lv_obj_align_to(ui_Record_btn_name, ui_Record_btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    ui_Player_btn = lv_imgbtn_create(ui_Recorder);
    lv_imgbtn_set_src(ui_Player_btn, LV_IMGBTN_STATE_RELEASED, NULL, &ui_img_music_btn_play, NULL);
    lv_imgbtn_set_src(ui_Player_btn, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &ui_img_music_btn_pause, NULL);
    lv_obj_add_flag(ui_Player_btn, LV_OBJ_FLAG_CHECKABLE);

    lv_obj_add_event_cb(ui_Player_btn, play_event_click_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(ui_Player_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_width(ui_Player_btn, ui_img_music_btn_play.header.w);

    lv_obj_set_pos( ui_Player_btn, -30 , -60);
    lv_obj_set_align( ui_Player_btn, LV_ALIGN_BOTTOM_RIGHT );
    lv_obj_clear_flag( ui_Player_btn, LV_OBJ_FLAG_SCROLLABLE );

    ui_Player_btn_name = lv_label_create(ui_Recorder);
    lv_label_set_text(ui_Player_btn_name,"Play");
    lv_obj_set_style_text_font(ui_Player_btn_name, &lv_font_montserrat_12, 0);
    lv_obj_align_to(ui_Player_btn_name, ui_Player_btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    lv_obj_add_event_cb(ui_Recorder, ui_event_Recorder, LV_EVENT_ALL, NULL);
}