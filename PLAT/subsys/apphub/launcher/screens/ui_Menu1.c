
#include "../ui.h"
#include <stdio.h>
#include <stdlib.h>
#include "ostask.h"
#include "osasys.h"
#include "bsp.h"
#include "bsp_custom.h"
#include "mode_config.h"

#define APP_TRACE(subId, argLen, format,  ...)  \
    ECOMM_TRACE(UNILOG_REF_APP, subId, P_VALUE, argLen, format,  ##__VA_ARGS__) 

lv_obj_t *ui_Menu1 = NULL;
lv_obj_t *desktop_menu1;
lv_obj_t *ui_Menu_Incon1;
lv_obj_t *ui_Scrolldots1;

#define APP_ICON_SIZE       58
#define APP_FONT_SIZE       16

static lv_obj_t *app_name_list[9];
typedef enum
{
    APP_TEST,
    APP_SET,
    APP_CHIP,
    APP_MUSIC,
    APP_AMR,
    APP_FILE,
    APP_IMS,
    APP_SMS,
    // APP_PIC,
}menu1_app_t;


static lv_style_t menu_style;

static lv_obj_t *flash_imgbtn;
static lv_obj_t *settings_imgbtn;
static lv_obj_t *call_imgbtn;
static lv_obj_t *music_imgbtn;
static lv_obj_t *chip_imgbtn;
static lv_obj_t *sms_imgbtn;
static lv_obj_t *record_imgbtn;
static lv_obj_t *file_imgbtn;
// static lv_obj_t *photo_imgbtn;

LV_IMG_DECLARE(ui_icon_flash);
LV_IMG_DECLARE(ui_icon_settings);
LV_IMG_DECLARE(ui_icon_chip);

LV_IMG_DECLARE(ui_icon_music);
LV_IMG_DECLARE(ui_icon_record);
LV_IMG_DECLARE(ui_icon_file);

LV_IMG_DECLARE(ui_icon_call);
LV_IMG_DECLARE(ui_icon_sms);
// LV_IMG_DECLARE(ui_icon_photo);

static lv_obj_t *app_obj_list[] = {
    &flash_imgbtn,&settings_imgbtn, &chip_imgbtn,
    #ifdef FEATURE_SUBSYS_MEDIA_ENABLE 
    &music_imgbtn, &record_imgbtn,&file_imgbtn,
    #endif
    #ifdef FEATURE_IMS_ENABLE
    &call_imgbtn, &sms_imgbtn
    #endif
};
static const lv_img_dsc_t *app_icon_list[] = {
    &ui_icon_flash, &ui_icon_settings, &ui_icon_chip,  
    #ifdef FEATURE_SUBSYS_MEDIA_ENABLE
    &ui_icon_music, &ui_icon_record, &ui_icon_file,
    #endif
    #ifdef FEATURE_IMS_ENABLE
    &ui_icon_call, &ui_icon_sms
    #endif
};

static char *app_name[] = {
    "Test","Setting","EC718",
    #ifdef FEATURE_SUBSYS_MEDIA_ENABLE
    "Music","Record","File",
    #endif
    #ifdef FEATURE_IMS_ENABLE
    "VoLTE","SMS"
    #endif
};

static uint16_t app_user_data[] = {
    APP_TEST,APP_SET,APP_CHIP,
    #ifdef FEATURE_SUBSYS_MEDIA_ENABLE
    APP_MUSIC,APP_AMR,APP_FILE,
    #endif
    #ifdef FEATURE_IMS_ENABLE
    APP_IMS,APP_SMS
    #endif
};

static void menu1_app_click_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    lv_obj_t * kb = lv_event_get_user_data(e);
    if (code == LV_EVENT_CLICKED)
    {
        // APP_TRACE(menu1_app_click_cb, 2, "%d,0x%X",code,(ta->user_data));
        if(ta->user_data==APP_TEST)
        {
            guiModeSet(THREAD_FLAG_TEST);
            // _ui_screen_change(&ui_Alarm, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Alarm_screen_init);
        } 
        else if(ta->user_data==APP_SET)
        {
            _ui_screen_change(&ui_Setting, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Setting_screen_init);
        } 
        else if(ta->user_data==APP_CHIP)   //EC718 INFO
        {
            _ui_screen_change(&ui_Chip, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Chip_screen_init);
        } 
        #ifdef FEATURE_SUBSYS_MEDIA_ENABLE
        else if(ta->user_data==APP_MUSIC)
        { 
            _ui_screen_change(&ui_Music_Player, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Music_Player_screen_init);  
        } 
        else if(ta->user_data==APP_AMR)
        {
            _ui_screen_change(&ui_Recorder, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Recorder_screen_init);
        } 
        #endif
        #ifdef FEATURE_IMS_ENABLE
        else if(ta->user_data==APP_IMS)
        {
            _ui_screen_change(&ui_Dial, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Dial_screen_init);
        } 
        else if(ta->user_data==APP_SMS)
        {
            _ui_screen_change(&ui_Chat, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Chat_screen_init);
        } 
        #endif
    }
}
void ui_Menu1_create(lv_obj_t * desktop)
{
    ui_Menu_Incon1 = lv_tabview_add_tab(desktop, "Menu_1");
    lv_obj_set_style_bg_img_src(ui_Menu_Incon1, &ui_img_pattern_png, LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_img_tiled(ui_Menu_Incon1, true, LV_PART_MAIN | LV_STATE_DEFAULT);
    int16_t app_x_offset = 0, app_y_offset = 22, count = 0;
    for(size_t y = 0; y < 3; y++) 
    {
        app_x_offset = APP_FONT_SIZE;
        if(count >= sizeof(app_obj_list)/sizeof(lv_obj_t *)) break;
        for (size_t x = 0; x < 3; x ++) 
        {
            if(count >= sizeof(app_obj_list)/sizeof(lv_obj_t *)) break;
            app_obj_list[count] = lv_imgbtn_create(ui_Menu_Incon1);
            lv_obj_set_width(app_obj_list[count], APP_ICON_SIZE);
            lv_obj_set_height(app_obj_list[count], APP_ICON_SIZE+APP_FONT_SIZE);
            lv_imgbtn_set_src(app_obj_list[count], LV_IMGBTN_STATE_PRESSED, NULL, app_icon_list[count], NULL);
            lv_imgbtn_set_src(app_obj_list[count], LV_IMGBTN_STATE_RELEASED, NULL, app_icon_list[count], NULL);
            lv_obj_align(app_obj_list[count], LV_ALIGN_TOP_LEFT, app_x_offset, app_y_offset);
            lv_obj_set_user_data(app_obj_list[count], app_user_data[count]);
            lv_obj_add_event_cb(app_obj_list[count], menu1_app_click_cb, LV_EVENT_CLICKED, NULL);
            app_x_offset += APP_FONT_SIZE; 
            app_x_offset += APP_ICON_SIZE;  
            app_name_list[count] = lv_label_create(ui_Menu_Incon1);
            lv_label_set_text(app_name_list[count],app_name[count]);
            lv_obj_set_style_text_font(app_name_list[count], &lv_font_montserrat_12, 0);
            lv_obj_align_to(app_name_list[count], app_obj_list[count], LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
            count += 1;
        }
        app_y_offset += APP_ICON_SIZE; 
        app_y_offset += APP_FONT_SIZE;  
        // app_y_offset += APP_FONT_SIZE/2;  
    }
    ui_Scrolldots1 = ui_Scrolldots_create(ui_Menu_Incon1);
    lv_obj_set_x( ui_Scrolldots1, 0 );
    lv_obj_set_y( ui_Scrolldots1, -8 );
    lv_obj_set_width( ui_comp_get_child(ui_Scrolldots1, UI_COMP_SCROLLDOTS_D1), 4);
    lv_obj_set_height( ui_comp_get_child(ui_Scrolldots1, UI_COMP_SCROLLDOTS_D1), 4);

    lv_obj_set_width( ui_comp_get_child(ui_Scrolldots1, UI_COMP_SCROLLDOTS_D2), 8);
    lv_obj_set_height( ui_comp_get_child(ui_Scrolldots1, UI_COMP_SCROLLDOTS_D2), 8);
    lv_obj_set_x( ui_comp_get_child(ui_Scrolldots1, UI_COMP_SCROLLDOTS_D2), 11 );
    lv_obj_set_y( ui_comp_get_child(ui_Scrolldots1, UI_COMP_SCROLLDOTS_D2), 0 );
    lv_obj_set_style_bg_color(ui_comp_get_child(ui_Scrolldots1, UI_COMP_SCROLLDOTS_D2), lv_color_hex(0x101C52), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(ui_comp_get_child(ui_Scrolldots1, UI_COMP_SCROLLDOTS_D2), 255, LV_PART_MAIN| LV_STATE_DEFAULT);
}

void ui_event_Menu1(lv_event_t * e) 
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    // if(event_code<LV_EVENT_COVER_CHECK || event_code>LV_EVENT_DRAW_PART_END) APP_TRACE(ui_event_Menu, 1,"%d",event_code);
    if (event_code == LV_EVENT_SCREEN_LOADED) 
    {
        // scrolldot_Animation(ui_Scrolldots1, 0);
    }
    if ( event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_LEFT ) 
    {
        lv_indev_wait_release(lv_indev_get_act());
        // _ui_screen_change( &ui_Home, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Home_screen_init);
        _ui_screen_change( &ui_Menu2, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Menu2_screen_init);
    }
    else if ( event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT ) 
    {
        lv_indev_wait_release(lv_indev_get_act());
        _ui_screen_change( &ui_Home, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Home_screen_init);
    }
    else if( event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) ==LV_DIR_BOTTOM) 
    {
        lv_indev_wait_release(lv_indev_get_act());
        _ui_screen_change(&ui_Setting, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Setting_screen_init);
    }
}


void ui_Menu1_screen_init(void)
{
    ui_Menu1 = lv_obj_create(NULL);
    lv_obj_clear_flag( ui_Menu1, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    desktop_menu1 = lv_tabview_create(ui_Menu1, LV_DIR_TOP, 0);
    ui_Menu1_create(desktop_menu1);
    lv_obj_add_event_cb(ui_Menu1, ui_event_Menu1, LV_EVENT_ALL, NULL);
}

