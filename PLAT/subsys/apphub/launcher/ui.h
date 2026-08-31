#ifndef _EC_GUI_H
#define _EC_GUI_H
#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "cmsis_os2.h"
#include "ui_helpers.h"
#include "components/ui_comp.h"
#include "components/ui_comp_hook.h"
#include "ui_events.h"

typedef enum ui_show_bits {
    UI_READY =      (1UL << 0), //设备状态就绪
    UI_BAR_LTE =    (1UL << 1),      
    UI_BAR_USB =    (1UL << 2),      
    UI_BAR_SIM =    (1UL << 3),       
    UI_BAR_CALL =   (1UL << 4), //未接来电  
    UI_BAR_BELL =   (1UL << 5),
    UI_BAR_BAT_FULL  =   (1UL << 6),   
    UI_BAR_BAT_HALF  =   (1UL << 7),   
    UI_BAR_BAT_LACK  =   (1UL << 8),   
    UI_BAR_CHARGING  =   (1UL << 9), 
    UI_DIALING_OUT  =   (1UL << 10),    //拨号
    UI_CALL_CANCEL  =   (1UL << 11), 
    UI_CALL_RINING  =   (1UL << 12),
    UI_CALL_ACCEPT  =   (1UL << 13),
    UI_SMS_RECEIVE  =   (1UL << 14),
    UI_SMS_DELIVER  =   (1UL << 15),
    UI_EVT_POWROFF  =   (1UL << 20),    //关机
    UI_INVALID = (1UL << 30)
} uiShowBits;  

#define UI_EVT_WAIT (UI_DIALING_OUT|UI_CALL_RINING|UI_CALL_CANCEL|UI_CALL_ACCEPT|UI_SMS_RECEIVE|UI_SMS_DELIVER|UI_EVT_POWROFF)

extern osEventFlagsId_t guiEvtHandle; 

extern lv_obj_t *bat_label;
extern lv_obj_t *net_label;
extern lv_obj_t *gnss_label;
extern lv_obj_t *usb_label;
extern lv_obj_t *bell_label;
extern lv_obj_t *call_label;
extern lv_obj_t *time_label;

void Up_Animation( lv_obj_t *TargetObject, int delay);
void scrolldot_Animation( lv_obj_t *TargetObject, int delay);
void ui_event_initial_actions( lv_event_t * e);
extern lv_obj_t *ui_initial_actions0;

void ui_Charging_screen_init(void);
extern lv_obj_t *ui_Charging;

// SCREEN: ui_Splash
void ui_Splash_screen_init(void);
void ui_event_Splash( lv_event_t * e);
extern lv_obj_t *ui_Splash;
extern lv_obj_t *ui_Demo;
extern lv_obj_t *ui_Smart_Gadget;
extern lv_obj_t *ui_SLS_Logo;


// SCREEN: ui_Clock
void ui_Clock_screen_init(void);
void ui_event_Clock( lv_event_t * e);
extern lv_obj_t *ui_Clock;
extern lv_obj_t *ui_Clock_Panel;
extern lv_obj_t *ui_Dot1;
extern lv_obj_t *ui_Dot2;
extern lv_obj_t *ui_Dot3;
extern lv_obj_t *ui_Dot4;
extern lv_obj_t *ui_Dot5;
extern lv_obj_t *ui_Dot6;
extern lv_obj_t *ui_Dot7;
extern lv_obj_t *ui_Dot8;
extern lv_obj_t *ui_Clock_Number1;
extern lv_obj_t *ui_Clock_Number2;
extern lv_obj_t *ui_Clock_Number3;
extern lv_obj_t *ui_Clock_Number4;
extern lv_obj_t *ui_Min;
extern lv_obj_t *ui_Hour;
extern lv_obj_t *ui_Sec;
extern lv_obj_t *ui_Clock_Center;
extern lv_obj_t *ui_Clock_Number;
// extern lv_obj_t *ui_Date;


// SCREEN: ui_Call
void ui_Call_screen_init(void);
void ui_event_Call( lv_event_t * e);
extern lv_obj_t *ui_Call;
extern lv_obj_t *ui_phone_number;
extern lv_obj_t *ui_Dialout;
extern lv_obj_t *ui_Incoming;
extern lv_obj_t *ui_Call_Incon1;
extern lv_obj_t *ui_Phone1;
extern lv_obj_t *ui_Call_Incon2;
extern lv_obj_t *ui_Phone2;
extern lv_obj_t *ui_Call_Incon3;
extern lv_obj_t *ui_Phone3;
extern lv_obj_t *ui_Avatar;


// SCREEN: ui_Dial
void ui_Dial_screen_init(void);
void ui_event_Dial( lv_event_t * e);
extern lv_obj_t *ui_Dial;
extern lv_obj_t *ui_Dial_container;
extern lv_obj_t *dial_text;
extern lv_obj_t *dial_btnm;

void ui_Chip_screen_init(void);
extern lv_obj_t *ui_Chip;
// SCREEN: ui_Chat
void ui_Chat_screen_init(void);
void ui_event_Chat( lv_event_t * e);
extern lv_obj_t *ui_Chat;
extern lv_obj_t *ui_Chat_container;
extern lv_obj_t *ui_sms_tx_flag;


void ui_Recorder_screen_init(void);
extern lv_obj_t *ui_Recorder;
// SCREEN: ui_Music_Player
void ui_Music_Player_screen_init(void);
void ui_event_Music_Player( lv_event_t * e);
extern lv_obj_t *ui_Music_Player;
extern lv_obj_t *ui_Music_Title;
// extern lv_obj_t *ui_Author;
extern lv_obj_t *ui_Play_btn;
extern lv_obj_t *ui_Play;
extern lv_obj_t *ui_Album;
extern lv_obj_t *ui_Backward;
extern lv_obj_t *ui_Forward;


// SCREEN: ui_Weather
void ui_Weather_screen_init(void);
void ui_event_Weather( lv_event_t * e);
extern lv_obj_t *ui_Weather;
extern lv_obj_t *ui_Pary_Cloud;
extern lv_obj_t *ui_New_York;
extern lv_obj_t *ui_Cloud;
extern lv_obj_t *ui_Celsius;
extern lv_obj_t *ui_Weather_Icons;
extern lv_obj_t *ui_w1;
extern lv_obj_t *ui_w2;
extern lv_obj_t *ui_w3;
extern lv_obj_t *ui_W1_Num;
extern lv_obj_t *ui_W2_Num;
extern lv_obj_t *ui_W3_Num;


// SCREEN: ui_Alarm
void ui_Alarm_screen_init(void);
void ui_event_Alarm( lv_event_t * e);
extern lv_obj_t *ui_Alarm;
extern lv_obj_t *ui_Alarm_container;
extern lv_obj_t *ui_Set_alarm;
extern lv_obj_t *ui_Alarm_Comp;
extern lv_obj_t *ui_Alarm_Comp1;
extern lv_obj_t *ui_Alarm_Comp2;
extern lv_obj_t *ui_Alarm_Comp3;

void ui_Setting_screen_init(void);
void ui_event_Setting(lv_event_t *e);
extern lv_obj_t *ui_Setting;

void ui_Menu1_screen_init(void);
extern lv_obj_t *ui_Menu1;
extern lv_obj_t *desktop_menu1;
extern lv_obj_t *ui_Scrolldots1;

void ui_Menu2_screen_init(void);
extern lv_obj_t *ui_Menu2;
extern lv_obj_t *desktop_menu2;
extern lv_obj_t *ui_Scrolldots2;

void ui_Home_screen_init(void);
extern lv_obj_t *ui_Home;
extern lv_obj_t *ui_Home_SIM;
extern lv_obj_t *ui_Scrolldots;
extern lv_obj_t *ui_status;
// extern lv_obj_t *ui_Scrolldots3;
// extern lv_obj_t *ui_Scrolldots4;
// extern lv_obj_t *ui_Scrolldots5;

LV_IMG_DECLARE( ui_img_sls_logo_png);   // assets\sls_logo.png
LV_IMG_DECLARE( ui_img_pattern_png);   // assets\pattern.png
LV_IMG_DECLARE( ui_img_clock_min_png);   // assets\clock_min.png
LV_IMG_DECLARE( ui_img_clock_hour_png);   // assets\clock_hour.png
LV_IMG_DECLARE( ui_img_clock_sec_png);   // assets\clock_sec.png
LV_IMG_DECLARE( ui_img_phone_png);   // assets\phone.png
LV_IMG_DECLARE( ui_img_avatar_png);   // assets\avatar.png
// LV_IMG_DECLARE( ui_img_eigencomm_png_216);
LV_IMG_DECLARE( ui_img_chatbox_png);   // assets\chatbox.png
LV_IMG_DECLARE( ui_img_chatbox2_png);   // assets\chatbox2.png
LV_IMG_DECLARE( ui_img_play_png);   // assets\play.png
LV_IMG_DECLARE( ui_img_album_png);   // assets\album.png
LV_IMG_DECLARE( ui_img_backward_png);   // assets\backward.png
LV_IMG_DECLARE( ui_img_forward_png);   // assets\forward.png
LV_IMG_DECLARE( ui_img_cloud_png);   // assets\cloud.png
LV_IMG_DECLARE( ui_img_weather_1_png);   // assets\weather_1.png
LV_IMG_DECLARE( ui_img_weather_2_png);   // assets\weather_2.png
LV_IMG_DECLARE( ui_img_weather_3_png);   // assets\weather_3.png
LV_IMG_DECLARE( ui_img_audio_wave_1_png);

LV_FONT_DECLARE( ui_font_Number);
LV_FONT_DECLARE( ui_font_song16);

void ui_init(uiShowBits flag);
void ui_back();
// void ui_screen_init(uiShowBits flag);

void ui_bar_update(uiShowBits flag);
void ui_evt_update(uiShowBits flag);
int ui_printf(const char *format, ...);
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
