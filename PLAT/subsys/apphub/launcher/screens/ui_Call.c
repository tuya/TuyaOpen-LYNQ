
#include "../ui.h"
#include <stdio.h>
#include <stdlib.h>
#include "ostask.h"
#include "osasys.h"
#include "bsp.h"
#include "bsp_custom.h"
#include DEBUG_LOG_HEADER_FILE

#define DEFAULT_EC_LOGO     "D:/ui_img_eigencomm_png_216.bin"

#define APP_TRACE(subId, argLen, format,  ...)  \
    ECOMM_TRACE(UNILOG_REF_APP, subId, P_VALUE, argLen, format,  ##__VA_ARGS__) 



lv_obj_t *ui_Call = NULL;
lv_obj_t *ui_phone_number;
lv_obj_t *ui_Incoming;
lv_obj_t *ui_Dialout;
lv_obj_t *ui_Call_Incon1;
lv_obj_t *ui_Phone1;
lv_obj_t *ui_Call_Incon2;
lv_obj_t *ui_Phone2;
lv_obj_t *ui_Call_Incon3;
lv_obj_t *ui_Phone3;
lv_obj_t *ui_Avatar;


static void call_btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    lv_obj_t * kb = lv_event_get_user_data(e);
    // APP_TRACE(call_btn_event_cb, 2, "%d,0x%X",code,(ta->user_data));
    if (code == LV_EVENT_CLICKED)
    {
        if(ta->user_data==2)
        {
            #ifdef FEATURE_IMS_ENABLE
            phoneCall(NULL);
            #endif
        } 
        else 
        {
            #ifdef FEATURE_IMS_ENABLE
            phoneHangUp();
            #endif
        }
    }
}

extern char phone_text[20];
void ui_Call_screen_init(void)
{
    ui_Call = lv_obj_create(NULL);
    lv_obj_clear_flag( ui_Call, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_obj_set_style_bg_color(ui_Call, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(ui_Call, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src( ui_Call, &ui_img_pattern_png, LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_img_tiled(ui_Call, true, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_phone_number = ui_Small_Label_create(ui_Call);
    lv_obj_set_x( ui_phone_number, 0 );
    lv_obj_set_y( ui_phone_number, 0 );
    lv_obj_set_align( ui_phone_number, LV_ALIGN_CENTER );
    lv_label_set_text(ui_phone_number,phone_text);  
    lv_obj_set_style_text_color(ui_phone_number, lv_color_hex(0x000746), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(ui_phone_number, 255, LV_PART_MAIN| LV_STATE_DEFAULT);


    ui_Dialout = ui_Small_Label_create(ui_Call);
    lv_obj_set_x( ui_Dialout, 0 );
    lv_obj_set_y( ui_Dialout, 25 );
    lv_obj_set_align( ui_Dialout, LV_ALIGN_CENTER );
    lv_label_set_text(ui_Dialout,"Dialing Out");
    lv_obj_set_style_text_color(ui_Dialout, lv_color_hex(0x9C9CD9), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(ui_Dialout, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_add_flag(ui_Dialout, LV_OBJ_FLAG_HIDDEN);

    ui_Incoming = ui_Small_Label_create(ui_Call);
    lv_obj_set_x( ui_Incoming, 0 );
    lv_obj_set_y( ui_Incoming, 25 );
    lv_obj_set_align( ui_Incoming, LV_ALIGN_CENTER );
    lv_label_set_text(ui_Incoming,"Incoming Call");
    lv_obj_set_style_text_color(ui_Incoming, lv_color_hex(0x9C9CD9), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(ui_Incoming, 255, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_Call_Incon1 = lv_obj_create(ui_Call);
    lv_obj_set_width( ui_Call_Incon1, 60);
    lv_obj_set_height( ui_Call_Incon1, 60);
    lv_obj_set_x( ui_Call_Incon1, -50 );
    lv_obj_set_y( ui_Call_Incon1, -42 );
    lv_obj_set_align( ui_Call_Incon1, LV_ALIGN_BOTTOM_MID );
    lv_obj_clear_flag( ui_Call_Incon1, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_obj_set_style_radius(ui_Call_Incon1, 60, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Call_Incon1, lv_color_hex(0xE63431), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(ui_Call_Incon1, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_Call_Incon1, lv_color_hex(0xE63431), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_shadow_opa(ui_Call_Incon1, 150, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui_Call_Incon1, 30, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui_Call_Incon1, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_x(ui_Call_Incon1, 3, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(ui_Call_Incon1, 6, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Call_Incon1, lv_color_hex(0xFF5855), LV_PART_MAIN | LV_STATE_PRESSED );
    lv_obj_set_style_bg_opa(ui_Call_Incon1, 255, LV_PART_MAIN| LV_STATE_PRESSED);

    lv_obj_set_user_data(ui_Call_Incon1, (uint16_t)1);
    lv_obj_add_event_cb(ui_Call_Incon1, call_btn_event_cb,LV_EVENT_CLICKED,NULL);

    ui_Phone1 = lv_img_create(ui_Call_Incon1);
    lv_img_set_src(ui_Phone1, &ui_img_phone_png);
    lv_obj_set_width( ui_Phone1, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height( ui_Phone1, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_align( ui_Phone1, LV_ALIGN_CENTER );
    lv_obj_add_flag( ui_Phone1, LV_OBJ_FLAG_ADV_HITTEST );   /// Flags
    lv_obj_clear_flag( ui_Phone1, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_img_set_angle(ui_Phone1,1360);

    ui_Call_Incon2 = lv_obj_create(ui_Call);
    lv_obj_set_width( ui_Call_Incon2, 60);
    lv_obj_set_height( ui_Call_Incon2, 60);
    lv_obj_set_x( ui_Call_Incon2, 50 );
    lv_obj_set_y( ui_Call_Incon2, -42 );
    lv_obj_set_align( ui_Call_Incon2, LV_ALIGN_BOTTOM_MID );
    lv_obj_clear_flag( ui_Call_Incon2, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_obj_set_style_radius(ui_Call_Incon2, 60, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Call_Incon2, lv_color_hex(0x10D262), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(ui_Call_Incon2, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_Call_Incon2, lv_color_hex(0x10D262), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_shadow_opa(ui_Call_Incon2, 150, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui_Call_Incon2, 30, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui_Call_Incon2, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_x(ui_Call_Incon2, 3, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(ui_Call_Incon2, 6, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Call_Incon2, lv_color_hex(0x31EF81), LV_PART_MAIN | LV_STATE_PRESSED );
    lv_obj_set_style_bg_opa(ui_Call_Incon2, 255, LV_PART_MAIN| LV_STATE_PRESSED);

    lv_obj_set_user_data(ui_Call_Incon2, (uint16_t)2);
    lv_obj_add_event_cb(ui_Call_Incon2, call_btn_event_cb,LV_EVENT_CLICKED,NULL);

    ui_Phone2 = lv_img_create(ui_Call_Incon2);
    lv_img_set_src(ui_Phone2, &ui_img_phone_png);
    lv_obj_set_width( ui_Phone2, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height( ui_Phone2, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_align( ui_Phone2, LV_ALIGN_CENTER );
    lv_obj_add_flag( ui_Phone2, LV_OBJ_FLAG_ADV_HITTEST );   /// Flags
    lv_obj_clear_flag( ui_Phone2, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    

    ui_Call_Incon3 = lv_obj_create(ui_Call);
    lv_obj_set_width( ui_Call_Incon3, 60);
    lv_obj_set_height( ui_Call_Incon3, 60);
    lv_obj_set_x( ui_Call_Incon3, 0 );
    lv_obj_set_y( ui_Call_Incon3, -42 );
    lv_obj_set_align( ui_Call_Incon3, LV_ALIGN_BOTTOM_MID );
    lv_obj_clear_flag( ui_Call_Incon3, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_obj_set_style_radius(ui_Call_Incon3, 60, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Call_Incon3, lv_color_hex(0xE63431), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(ui_Call_Incon3, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_Call_Incon3, lv_color_hex(0xE63431), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_shadow_opa(ui_Call_Incon3, 150, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui_Call_Incon3, 30, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui_Call_Incon3, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_x(ui_Call_Incon3, 3, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(ui_Call_Incon3, 6, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Call_Incon3, lv_color_hex(0xFF5855), LV_PART_MAIN | LV_STATE_PRESSED );
    lv_obj_set_style_bg_opa(ui_Call_Incon3, 255, LV_PART_MAIN| LV_STATE_PRESSED);
    
    lv_obj_set_user_data(ui_Call_Incon3, (uint16_t)3);
    lv_obj_add_event_cb(ui_Call_Incon3, call_btn_event_cb,LV_EVENT_CLICKED,NULL);
    lv_obj_add_flag(ui_Call_Incon3, LV_OBJ_FLAG_HIDDEN); 

    ui_Phone3 = lv_img_create(ui_Call_Incon3);
    lv_img_set_src(ui_Phone3, &ui_img_phone_png);
    lv_obj_set_width( ui_Phone3, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height( ui_Phone3, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_align( ui_Phone3, LV_ALIGN_CENTER );
    lv_obj_add_flag( ui_Phone3, LV_OBJ_FLAG_ADV_HITTEST );   /// Flags
    lv_obj_clear_flag( ui_Phone3, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_img_set_angle(ui_Phone3,1360);


    ui_Avatar = lv_img_create(ui_Call);
    // lv_img_set_src(ui_Avatar,DEFAULT_EC_LOGO);
    // lv_img_set_src(ui_Avatar, &ui_img_eigencomm_png_216);
    lv_obj_set_width( ui_Avatar, LV_SIZE_CONTENT); 
    lv_obj_set_height( ui_Avatar, LV_SIZE_CONTENT); 
    lv_obj_set_x( ui_Avatar, 0 );
    lv_obj_set_y( ui_Avatar, 40 );
    lv_obj_set_align( ui_Avatar, LV_ALIGN_TOP_MID );
    lv_obj_add_flag( ui_Avatar, LV_OBJ_FLAG_ADV_HITTEST );   /// Flags
    lv_obj_clear_flag( ui_Avatar, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_obj_set_style_radius(ui_Avatar, 300, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_Avatar, lv_color_hex(0xD5D2D5), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_shadow_opa(ui_Avatar, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui_Avatar, 30, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui_Avatar, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_x(ui_Avatar, 3, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(ui_Avatar, 6, LV_PART_MAIN| LV_STATE_DEFAULT);

    lv_obj_add_event_cb(ui_Call, ui_event_Call, LV_EVENT_ALL, NULL);

}



void ui_event_Call( lv_event_t * e) 
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    // if(event_code<LV_EVENT_COVER_CHECK || event_code>LV_EVENT_DRAW_PART_END) APP_TRACE(ui_event_Call, 1,"%d",event_code);
    if (event_code == LV_EVENT_SCREEN_LOADED) 
    {
        // Up_Animation(ui_Avatar, 100);
        // Up_Animation(ui_phone_number, 200);
        // Up_Animation(ui_Incoming, 300);
        // Up_Animation(ui_Call_Incon1, 200);
        // Up_Animation(ui_Call_Incon2, 300);
        // Up_Animation(ui_Call_Incon3, 400);
        // lv_obj_clear_flag(ui_Incoming, LV_OBJ_FLAG_HIDDEN);
        // lv_obj_clear_flag(ui_Call_Incon1, LV_OBJ_FLAG_HIDDEN); 
        // lv_obj_clear_flag(ui_Call_Incon2, LV_OBJ_FLAG_HIDDEN);
        // lv_obj_add_flag(ui_Dialout, LV_OBJ_FLAG_HIDDEN);
    }
    if ( event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_LEFT ) 
    {
        lv_indev_wait_release(lv_indev_get_act());
        _ui_screen_change(&ui_Menu1, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Menu1_screen_init); 
    }
    else if ( event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT ) 
    {
        lv_indev_wait_release(lv_indev_get_act());
        _ui_screen_change(&ui_Menu1, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Menu1_screen_init); 
    }
}

int ui_incomming_show(char *phone)
{
    lv_label_set_text(ui_phone_number,phone);
    extern osEventFlagsId_t guiEvtHandle; 
    osEventFlagsSet(guiEvtHandle,UI_CALL_RINING);
}
