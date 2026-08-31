#include "lvgl.h"
#include <stdio.h>
#include <stdlib.h>
#include "ostask.h"
#include "osasys.h"
#include "bsp.h"
#include "bsp_custom.h"
#include DEBUG_LOG_HEADER_FILE

#ifdef FEATURE_SUBSYS_STATUS_ENABLE
#include "status.h"
extern StatusT gStatus;
extern utc_timer_value_t gUtcTime;
#endif

#include "ui.h"
#include "ui_helpers.h"

#if LV_MEM_CUSTOM == 0 && LV_MEM_SIZE < (60ul * 1024ul)
#error Insufficient memory. Please set LV_MEM_SIZE to at least 60KB.
#endif

#define APP_TRACE(subId, argLen, format,  ...)  \
    ECOMM_TRACE(UNILOG_REF_APP, subId, P_VALUE, argLen, format,  ##__VA_ARGS__) 

lv_disp_t *dispp = NULL;
lv_theme_t *theme = NULL;
lv_timer_t *lvgl_time = NULL;

lv_obj_t *ui_status;

lv_obj_t *bat_label;
lv_obj_t *net_label;
lv_obj_t *usb_label;
lv_obj_t *gnss_label;
lv_obj_t *bell_label;
lv_obj_t *call_label;
lv_obj_t *time_label;

lv_obj_t **bar_label;

char show_time_str[] = "2024-03-06 10:00";

uint8_t hour =  10;
uint8_t mins =  0;
uint8_t secs =  0;
uint8_t day =  6;
uint8_t mon =  3;
uint16_t year = 2024;
#include "pwrkey.h"
#ifdef FEATURE_SUBSYS_OPENHAL_ENABLE
#include "api_tp.h"
#include "api_scr.h"
#include "api_comm.h"
extern uint32_t scr_dev_UsrId;
extern uint8_t backLightVal;
#endif
void ui_Pwroff_cb(lv_timer_t * tmr)
{
    #ifdef FEATURE_SUBSYS_OPENHAL_ENABLE
    backLightVal = 0;
    api_scr_ioctl(scr_dev_UsrId,SCREEN_BACKLIGHT_SET, &backLightVal);
    #endif
    pwrKeyStartPowerOff();
}

void ui_bar_update(uiShowBits flag)
{
    // APP_TRACE(ui_bar_update,1,"0x%X",flag);
    if (flag & UI_BAR_LTE)
    {
        lv_obj_clear_flag(net_label, LV_OBJ_FLAG_HIDDEN);
    }
    else 
    {
        lv_obj_add_flag(net_label, LV_OBJ_FLAG_HIDDEN);
    }
    if(flag & UI_BAR_SIM)
    {
        lv_label_set_text(ui_Home_SIM,"已插SIM卡");
        lv_obj_set_style_text_font(ui_Home_SIM, &ui_font_song16, LV_PART_MAIN|LV_STATE_DEFAULT);
        // lv_obj_clear_flag(ui_Home_SIM, LV_OBJ_FLAG_HIDDEN);
    }
    else 
    {
        lv_label_set_text(ui_Home_SIM,"未插SIM卡");
        lv_obj_set_style_text_font(ui_Home_SIM, &ui_font_song16, LV_PART_MAIN|LV_STATE_DEFAULT);
        // lv_obj_add_flag(ui_Home_SIM, LV_OBJ_FLAG_HIDDEN);
    }
    if(flag & UI_BAR_USB)
    {
        lv_obj_clear_flag(usb_label, LV_OBJ_FLAG_HIDDEN);
    }
    else 
    {
        lv_obj_add_flag(usb_label, LV_OBJ_FLAG_HIDDEN);  
    }
    if (flag & UI_BAR_CALL)
    {
        lv_obj_clear_flag(call_label, LV_OBJ_FLAG_HIDDEN);
    }
    else 
    {
        lv_obj_add_flag(call_label, LV_OBJ_FLAG_HIDDEN);
    }
    if (flag & UI_BAR_BELL)
    {
        lv_obj_clear_flag(bell_label, LV_OBJ_FLAG_HIDDEN);
    }
    else 
    {
        lv_obj_add_flag(bell_label, LV_OBJ_FLAG_HIDDEN);
    }
    if(flag & UI_BAR_CHARGING)
    {
        lv_label_set_text(bat_label, LV_SYMBOL_CHARGE);
    }
    else if(flag & UI_BAR_BAT_LACK)
    {
        lv_label_set_text(bat_label, LV_SYMBOL_BATTERY_EMPTY);
    }
    else if(flag & UI_BAR_BAT_HALF)
    {
        lv_label_set_text(bat_label, LV_SYMBOL_BATTERY_2);
    }
    else if(flag & UI_BAR_BAT_FULL)
    {
        lv_label_set_text(bat_label, LV_SYMBOL_BATTERY_FULL);
    }
} 

void ui_evt_update(uiShowBits flag)
{
    // APP_TRACE(ui_evt_update,1,"0x%X",flag);
    #ifdef FEATURE_IMS_ENABLE
    if (flag & UI_CALL_ACCEPT)
    {
        _ui_screen_change(&ui_Call, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Call_screen_init); 
        lv_obj_clear_flag(ui_Incoming, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_Dialout, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_Call_Incon1, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_Call_Incon2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_Call_Incon3, LV_OBJ_FLAG_HIDDEN);
    }
    if (flag & UI_CALL_CANCEL)
    {
        _ui_screen_change(&ui_Menu1, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Menu1_screen_init);
    }
    if (flag & UI_CALL_RINING)
    {
        _ui_screen_change(&ui_Call, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Call_screen_init); 
        lv_obj_clear_flag(ui_Incoming, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_Call_Incon1, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_clear_flag(ui_Call_Incon2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_Call_Incon3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_Dialout, LV_OBJ_FLAG_HIDDEN);
    }
    if (flag & UI_DIALING_OUT && ui_Call)
    {
        _ui_screen_change(&ui_Call, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Call_screen_init); 
        lv_obj_add_flag(ui_Incoming, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_Call_Incon1, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_Call_Incon2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_Dialout, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_Call_Incon3, LV_OBJ_FLAG_HIDDEN);     
    }
    if (flag & UI_SMS_RECEIVE)  //接收到新短信
    {
        extern int sms_load(int rank);
        sms_load(0);
        _ui_screen_change(&ui_Chat, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Chat_screen_init);
    }
    if (flag & UI_SMS_DELIVER)  //短信发送成功
    {
        LV_LOG_USER("SMS TX OK"); 
        if(ui_sms_tx_flag) lv_obj_clear_flag(ui_sms_tx_flag, LV_OBJ_FLAG_HIDDEN);
        // _ui_screen_change(&ui_Chat, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Chat_screen_init);
    }
    #endif
    if (flag & UI_EVT_POWROFF)  //关机动画
    {
        lv_timer_t *pwroff_timer = lv_timer_create(ui_Pwroff_cb, 5000, 0);
        lv_disp_load_scr(ui_Charging);
        // _ui_screen_change(&ui_Splash, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Splash_screen_init);
    }
}
/**
  \fn          
  \brief       
  \return
*/
static lv_style_t style_bar;
void ui_bar_init(uiShowBits flag)
{
    ui_status = lv_obj_create(lv_layer_top());
    lv_obj_set_width( ui_status, lv_pct(100));
    lv_obj_set_height( ui_status, 20); 
    lv_obj_set_style_bg_opa( ui_status, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    net_label = lv_label_create(ui_status);
    lv_label_set_text(net_label, LV_SYMBOL_HOME);
    // lv_label_set_text(net_label,"CONNECT"); //后期可改为图标
    lv_obj_align(net_label, LV_ALIGN_TOP_LEFT, 2, 2);

    time_label = lv_label_create(ui_status);
    lv_label_set_text(time_label, show_time_str);
    lv_obj_align_to(time_label, net_label, LV_ALIGN_OUT_RIGHT_TOP, 8, 0);
    lv_obj_add_flag(time_label, LV_OBJ_FLAG_HIDDEN);

    bat_label = lv_label_create(ui_status); 
    lv_label_set_text(bat_label, LV_SYMBOL_BATTERY_3);
    lv_obj_align(bat_label, LV_ALIGN_TOP_RIGHT, -4, 2);

    usb_label = lv_label_create(ui_status);
    lv_label_set_text(usb_label, LV_SYMBOL_USB);
    lv_obj_align_to(usb_label, bat_label, LV_ALIGN_OUT_LEFT_TOP, -4, 0);

    gnss_label = lv_label_create(ui_status);
    lv_label_set_text(gnss_label, LV_SYMBOL_GPS);
    lv_obj_align_to(gnss_label, usb_label, LV_ALIGN_OUT_LEFT_TOP, -4, 0);
    lv_obj_add_flag(gnss_label, LV_OBJ_FLAG_HIDDEN);

    call_label = lv_label_create(ui_status);
    lv_label_set_text(call_label, LV_SYMBOL_CALL);
    lv_obj_align_to(call_label, gnss_label, LV_ALIGN_OUT_LEFT_TOP, -4, 0);

    bell_label = lv_label_create(ui_status);
    lv_label_set_text(bell_label, LV_SYMBOL_BELL);
    lv_obj_align_to(bell_label, call_label, LV_ALIGN_OUT_LEFT_TOP, -4, 0);
    // lv_obj_add_flag(bell_label, LV_OBJ_FLAG_HIDDEN);
    ui_bar_update(flag);
    // obj = lv_bar_create(main_page);
    // lv_bar_set_range(obj, -1000, 2000);
    // lv_obj_set_size(obj1, 20, 200);
}
/**
  \fn          
  \brief       
  \return
*/
void ui_back()
{
    _ui_screen_change(&ui_Menu1, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Menu1_screen_init);
}
///////////////////// ANIMATIONS ////////////////////
void Up_Animation( lv_obj_t *TargetObject, int delay)
{
    ui_anim_user_data_t *PropertyAnimation_0_user_data = lv_mem_alloc(sizeof(ui_anim_user_data_t));
    PropertyAnimation_0_user_data->target = TargetObject;
    PropertyAnimation_0_user_data->val = -1;
    lv_anim_t PropertyAnimation_0;
    lv_anim_init(&PropertyAnimation_0);
    lv_anim_set_time(&PropertyAnimation_0, 200);
    lv_anim_set_user_data(&PropertyAnimation_0, PropertyAnimation_0_user_data);
    lv_anim_set_custom_exec_cb(&PropertyAnimation_0, _ui_anim_callback_set_y );
    lv_anim_set_values(&PropertyAnimation_0, -30, 0 );
    lv_anim_set_path_cb( &PropertyAnimation_0, lv_anim_path_ease_out);
    lv_anim_set_delay( &PropertyAnimation_0, delay + 0 );
    lv_anim_set_deleted_cb( &PropertyAnimation_0, _ui_anim_callback_free_user_data );
    lv_anim_set_playback_time(&PropertyAnimation_0, 0);
    lv_anim_set_playback_delay(&PropertyAnimation_0, 0);
    lv_anim_set_repeat_count(&PropertyAnimation_0, 0);
    lv_anim_set_repeat_delay(&PropertyAnimation_0, 0);
    lv_anim_set_early_apply( &PropertyAnimation_0, false );
    lv_anim_set_get_value_cb(&PropertyAnimation_0, &_ui_anim_callback_get_y );
    lv_anim_start(&PropertyAnimation_0);
    ui_anim_user_data_t *PropertyAnimation_1_user_data = lv_mem_alloc(sizeof(ui_anim_user_data_t));
    PropertyAnimation_1_user_data->target = TargetObject;
    PropertyAnimation_1_user_data->val = -1;
    lv_anim_t PropertyAnimation_1;
    lv_anim_init(&PropertyAnimation_1);
    lv_anim_set_time(&PropertyAnimation_1, 100);
    lv_anim_set_user_data(&PropertyAnimation_1, PropertyAnimation_1_user_data);
    lv_anim_set_custom_exec_cb(&PropertyAnimation_1, _ui_anim_callback_set_opacity );
    lv_anim_set_values(&PropertyAnimation_1, 0, 255 );
    lv_anim_set_path_cb( &PropertyAnimation_1, lv_anim_path_linear);
    lv_anim_set_delay( &PropertyAnimation_1, delay + 0 );
    lv_anim_set_deleted_cb( &PropertyAnimation_1, _ui_anim_callback_free_user_data );
    lv_anim_set_playback_time(&PropertyAnimation_1, 0);
    lv_anim_set_playback_delay(&PropertyAnimation_1, 0);
    lv_anim_set_repeat_count(&PropertyAnimation_1, 0);
    lv_anim_set_repeat_delay(&PropertyAnimation_1, 0);
    lv_anim_set_early_apply( &PropertyAnimation_1, true );
    lv_anim_start(&PropertyAnimation_1);
}

void scrolldot_Animation(lv_obj_t *TargetObject, int delay)
{
    ui_anim_user_data_t *PropertyAnimation_0_user_data = lv_mem_alloc(sizeof(ui_anim_user_data_t));
    PropertyAnimation_0_user_data->target = TargetObject;
    PropertyAnimation_0_user_data->val = -1;
    lv_anim_t PropertyAnimation_0;
    lv_anim_init(&PropertyAnimation_0);
    lv_anim_set_time(&PropertyAnimation_0, 300);
    lv_anim_set_user_data(&PropertyAnimation_0, PropertyAnimation_0_user_data);
    lv_anim_set_custom_exec_cb(&PropertyAnimation_0, _ui_anim_callback_set_y );
    lv_anim_set_values(&PropertyAnimation_0, 30, -8 );
    lv_anim_set_path_cb( &PropertyAnimation_0, lv_anim_path_ease_out);
    lv_anim_set_delay( &PropertyAnimation_0, delay + 0 );
    lv_anim_set_deleted_cb( &PropertyAnimation_0, _ui_anim_callback_free_user_data );
    lv_anim_set_playback_time(&PropertyAnimation_0, 0);
    lv_anim_set_playback_delay(&PropertyAnimation_0, 0);
    lv_anim_set_repeat_count(&PropertyAnimation_0, 0);
    lv_anim_set_repeat_delay(&PropertyAnimation_0, 0);
    lv_anim_set_early_apply( &PropertyAnimation_0, true );
    lv_anim_start(&PropertyAnimation_0);

}

lv_obj_t *ui_initial_actions0 = NULL;
void ui_event_initial_actions( lv_event_t * e) 
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    if ( event_code == LV_EVENT_SCREEN_LOAD_START) 
    {
        // uint8_t hour =  (((gUtcTime.UTCtimer2 >> 24) & 0xFF) + 8) % 24;
        // uint8_t mins =  ((gUtcTime.UTCtimer2 >> 16) & 0xFF);
        // uint8_t secs =  ((gUtcTime.UTCtimer2 >> 8) & 0xFF);
        sec_Animation(ui_Sec, 0, 3600);
        // hour_Animation(ui_Min, mins);
        // hour_Animation(ui_Hour, hour);
    }
}

void ui_timer_cb(lv_timer_t * tmr)
{
    if(gUtcTime.UTCsecs > 1702546376)
    {
        lv_obj_clear_flag(time_label, LV_OBJ_FLAG_HIDDEN);
    }
    year =  (gUtcTime.UTCtimer1 >> 16);
    mon =  ((gUtcTime.UTCtimer1 >> 8) & 0xFF);
    day =  ((gUtcTime.UTCtimer1 ) & 0xFF);
    uint8_t  buff =  (((gUtcTime.UTCtimer2 >> 24) & 0xFF) + 8) % 24;
    if(hour != buff){
        hour = buff;
        hour_Animation(ui_Hour, 0, (hour % 12 +1) * 300);
    } 
    buff =  ((gUtcTime.UTCtimer2 >> 16) & 0xFF);
    if(mins != buff){
        mins = buff;
        min_Animation(ui_Min, 0, mins*60);
    } 
    secs =  ((gUtcTime.UTCtimer2 >> 8) & 0xFF);
    memset(show_time_str,0,sizeof(show_time_str));
    sprintf(show_time_str, "%02u:%02u", hour, mins);
    lv_label_set_text(ui_Clock_Number,show_time_str);
    memset(show_time_str,0,sizeof(show_time_str));
    sprintf(show_time_str,"%04u-%02u-%02u %02u:%02u", year, mon, day, hour, mins);
    lv_label_set_text(time_label, show_time_str);
}

void ui_init(uiShowBits flag)
{
    // APP_TRACE(ui_init,1,"0x%X",flag);
    if (flag & UI_READY)
    {
        LV_EVENT_GET_COMP_CHILD = lv_event_register_id();
        dispp = lv_disp_get_default();
        theme = lv_theme_basic_init(dispp);
        lv_disp_set_theme(dispp, theme);
        // ui_Splash_screen_init();
        ui_Clock_screen_init();
        ui_Menu1_screen_init();
        ui_Menu2_screen_init();
        ui_Home_screen_init();
        ui_Call_screen_init();
        ui_Dial_screen_init();
        ui_Chat_screen_init();
        ui_Chip_screen_init();
        ui_Alarm_screen_init();
        ui_Music_Player_screen_init();
        ui_Recorder_screen_init();
        ui_Setting_screen_init();
        ui_Weather_screen_init();
        ui_Charging_screen_init(); 
        
        ui_initial_actions0 = lv_obj_create(NULL);
        lv_obj_add_event_cb(ui_initial_actions0, ui_event_initial_actions, LV_EVENT_ALL, NULL);
        lv_disp_load_scr(ui_initial_actions0);
        ui_bar_init(flag);
        lv_disp_load_scr(ui_Home);
        lvgl_time = lv_timer_create(ui_timer_cb, 5000, 0);
    }
    else     
    {
        ui_Charging_screen_init();
        lv_disp_load_scr(ui_Charging);
    }
}

