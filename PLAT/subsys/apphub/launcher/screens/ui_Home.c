
#include "../ui.h"
#include <stdio.h>
#include <stdlib.h>
#include "ostask.h"
#include "osasys.h"
#include "bsp.h"
#include "bsp_custom.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#define APP_TRACE(subId, argLen, format,  ...)  \
    ECOMM_TRACE(UNILOG_REF_APP, subId, P_VALUE, argLen, format,  ##__VA_ARGS__) 

lv_obj_t *ui_Home = NULL;
lv_obj_t *desktop_home;
lv_obj_t *ui_Scrolldots;
lv_obj_t *ui_Home_SIM;
lv_obj_t *ui_Home_Cloud;
lv_obj_t *ui_Home_Temp;
lv_obj_t *ui_Home_Weather_Cloud;
lv_obj_t *ui_Home_Weather_Icons;


void ui_event_Home(lv_event_t * e) 
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    // if(event_code<LV_EVENT_COVER_CHECK || event_code>LV_EVENT_DRAW_PART_END) {
    //     SYSLOG_INFO("%d,%d\r\n",event_code,lv_indev_get_gesture_dir(lv_indev_get_act())); 
    // }
    if (event_code == LV_EVENT_SCREEN_LOADED) 
    {
        // scrolldot_Animation(ui_Scrolldots, 0);
        lv_obj_clear_flag(ui_status, LV_OBJ_FLAG_HIDDEN);
    }
    if ( event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_LEFT ) 
    {
        lv_indev_wait_release(lv_indev_get_act());
        _ui_screen_change(&ui_Menu1, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Menu1_screen_init);
    }
    else if ( event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT ) 
    {
        lv_indev_wait_release(lv_indev_get_act());
        _ui_screen_change(&ui_Clock, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Clock_screen_init);
    }
    else if( event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) ==LV_DIR_BOTTOM) 
    {
        lv_indev_wait_release(lv_indev_get_act());
        _ui_screen_change(&ui_Setting, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Setting_screen_init);
    }
}


void ui_Home_screen_init(void)
{
    ui_Home = lv_obj_create(NULL);
    lv_obj_clear_flag( ui_Home, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_obj_set_style_bg_color(ui_Home, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(ui_Home, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src( ui_Home, &ui_img_pattern_png, LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_img_tiled(ui_Home, true, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_Home_Weather_Cloud = ui_Small_Label_create(ui_Home);
    lv_obj_set_x( ui_Home_Weather_Cloud, 0 );
    lv_obj_set_y( ui_Home_Weather_Cloud, -47 );
    lv_obj_set_align( ui_Home_Weather_Cloud, LV_ALIGN_CENTER );
    lv_label_set_text(ui_Home_Weather_Cloud,"Party cloud");
    lv_obj_set_style_text_color(ui_Home_Weather_Cloud, lv_color_hex(0x000746), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(ui_Home_Weather_Cloud, 255, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_Home_SIM = ui_Small_Label_create(ui_Home);
    lv_obj_set_x( ui_Home_SIM, 0 );
    lv_obj_set_y( ui_Home_SIM, 57 );
    lv_obj_set_align( ui_Home_SIM, LV_ALIGN_CENTER );
    lv_label_set_text(ui_Home_SIM,"未插SIM卡");
    lv_obj_set_style_text_font(ui_Home_SIM, &ui_font_song16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_Home_SIM, lv_color_hex(0x000746), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(ui_Home_SIM, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    // lv_obj_add_flag(ui_Home_SIM, LV_OBJ_FLAG_HIDDEN);

    ui_Home_Cloud = lv_img_create(ui_Home);
    lv_img_set_src(ui_Home_Cloud, &ui_img_cloud_png);
    lv_obj_set_width( ui_Home_Cloud, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height( ui_Home_Cloud, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_x( ui_Home_Cloud, -2 );
    lv_obj_set_y( ui_Home_Cloud, 39 );
    lv_obj_set_align( ui_Home_Cloud, LV_ALIGN_TOP_MID );
    lv_obj_add_flag( ui_Home_Cloud, LV_OBJ_FLAG_ADV_HITTEST );   /// Flags
    lv_obj_clear_flag( ui_Home_Cloud, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_obj_set_style_img_recolor(ui_Home_Cloud, lv_color_hex(0x293062), LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor_opa(ui_Home_Cloud, 255, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_Home_Temp = lv_label_create(ui_Home);
    lv_obj_set_width( ui_Home_Temp, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height( ui_Home_Temp, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_x( ui_Home_Temp, 0 );
    lv_obj_set_y( ui_Home_Temp, 135 );
    lv_obj_set_align( ui_Home_Temp, LV_ALIGN_TOP_MID );
    lv_label_set_text(ui_Home_Temp,"18°");
    lv_obj_set_style_text_color(ui_Home_Temp, lv_color_hex(0x293062), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(ui_Home_Temp, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Home_Temp, &ui_font_Number, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_Scrolldots = ui_Scrolldots_create(ui_Home);
    lv_obj_set_x( ui_Scrolldots, 0 );
    lv_obj_set_y( ui_Scrolldots, -8 );
    lv_obj_set_style_bg_color(ui_comp_get_child(ui_Scrolldots, UI_COMP_SCROLLDOTS_D1), lv_color_hex(0x101C52), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(ui_comp_get_child(ui_Scrolldots, UI_COMP_SCROLLDOTS_D1), 255, LV_PART_MAIN| LV_STATE_DEFAULT);

    lv_obj_add_event_cb(ui_Home, ui_event_Home, LV_EVENT_ALL, NULL);
}