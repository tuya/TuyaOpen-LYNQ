
#include "../ui.h"
#include <stdio.h>
#include <stdlib.h>
#include "ostask.h"
#include "osasys.h"
#include "bsp.h"
#include "bsp_custom.h"

#define APP_TRACE(subId, argLen, format,  ...)  \
    ECOMM_TRACE(UNILOG_REF_APP, subId, P_VALUE, argLen, format,  ##__VA_ARGS__) 

lv_obj_t *ui_Menu2;
lv_obj_t *desktop_menu2;
lv_obj_t *ui_Menu_Incon2;
lv_obj_t *ui_Scrolldots2;

// lv_obj_t *ui_Scrolldots3;
// lv_obj_t *ui_Scrolldots4;
// lv_obj_t *ui_Scrolldots5;

#define M2_APP_ICON_SIZE    40
#define M2_APP_FONT_SIZE    12

static lv_style_t menu_style;

static lv_obj_t *alarm_imgbtn;
static lv_obj_t *books_imgbtn;
static lv_obj_t *calendar_imgbtn;
static lv_obj_t *call_imgbtn;
static lv_obj_t *camera_imgbtn;
static lv_obj_t *clocks_imgbtn;
static lv_obj_t *contacts_imgbtn;
static lv_obj_t *files_imgbtn;
static lv_obj_t *message_imgbtn;
static lv_obj_t *music_imgbtn;
static lv_obj_t *notes_imgbtn;
static lv_obj_t *photos_imgbtn;
static lv_obj_t *recorder_imgbtn;
static lv_obj_t *settings_imgbtn;
static lv_obj_t *weather_imgbtn;

// LV_IMG_DECLARE(appstore);
// LV_IMG_DECLARE(books);
// LV_IMG_DECLARE(calendar);
// LV_IMG_DECLARE(call);
// LV_IMG_DECLARE(camera);
// LV_IMG_DECLARE(clocks);
// LV_IMG_DECLARE(contacts);
// LV_IMG_DECLARE(files);
// LV_IMG_DECLARE(message);
// LV_IMG_DECLARE(music);
// LV_IMG_DECLARE(notes);
// LV_IMG_DECLARE(photos);
// LV_IMG_DECLARE(recorder);
// LV_IMG_DECLARE(settings);
// LV_IMG_DECLARE(weather);

static lv_obj_t *app_wigets_list[] = {
    &alarm_imgbtn, &books_imgbtn, &calendar_imgbtn, &call_imgbtn,
    &camera_imgbtn, &clocks_imgbtn, &contacts_imgbtn, &files_imgbtn,
    &message_imgbtn, &music_imgbtn, &notes_imgbtn, &photos_imgbtn,
    &recorder_imgbtn, &settings_imgbtn, &weather_imgbtn
};


// static const lv_img_dsc_t *app_icon_list[] = {
//     &appstore, &books, &calendar, &call,
//     &camera, &clocks, &contacts, &files,
//     &message, &music, &notes,&photos, 
//     &recorder, &settings, &weather
// };

static lv_obj_t *app_name_list[15];
static char *app_name[] = {
    "Alarm", "Books", "Calendar", "Phone",
    "Camera", "Clock", "Contact", "Folder",
    "Message", "Music", "Notes", "Photos", 
    "Recorder", "Setting", "Weather"
};


static void menu2_app_click_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    lv_obj_t * kb = lv_event_get_user_data(e);
    // APP_TRACE(menu2_app_click_cb, 2, "%d,0x%X",code,(ta->user_data));
    if (code == LV_EVENT_CLICKED)
    {
        if(ta->user_data==0)
        {
            _ui_screen_change(&ui_Alarm, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Alarm_screen_init);
        } 
        else if(ta->user_data==3)
        {
            _ui_screen_change(&ui_Dial, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Dial_screen_init);
        } 
        else if(ta->user_data==5)
        {
            _ui_screen_change(&ui_Clock, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Clock_screen_init);
        } 
        else if(ta->user_data==8)
        {
            _ui_screen_change(&ui_Chat, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Chat_screen_init);
        } 
        else if(ta->user_data==9)
        {
            _ui_screen_change(&ui_Music_Player, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Music_Player_screen_init);
        } 
        else if(ta->user_data==13)
        {
            _ui_screen_change(&ui_Setting, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Setting_screen_init);
        } 
        else if(ta->user_data==14)
        {
            _ui_screen_change(&ui_Weather, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Weather_screen_init);
        } 
    }
}


void ui_Menu2_create(lv_obj_t * desktop)
{
    ui_Menu_Incon2 = lv_tabview_add_tab(desktop, "Menu_2");
    lv_obj_set_style_bg_img_src(ui_Menu_Incon2, &ui_img_pattern_png, LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_img_tiled(ui_Menu_Incon2, true, LV_PART_MAIN | LV_STATE_DEFAULT);
    // int16_t app_x_offset = 0, app_y_offset = 22, count = 0;
    // for(size_t y = 0; y < 4; y++) 
    // {
    //     app_x_offset = 22;
    //     if(count >= sizeof(app_wigets_list)/sizeof(lv_obj_t *)) break;
    //     for (size_t x = 0; x < 4; x ++) 
    //     {
    //         if(count >= sizeof(app_wigets_list)/sizeof(lv_obj_t *)) break;
    //         app_wigets_list[count] = lv_imgbtn_create(ui_Menu_Incon2);
    //         lv_obj_set_width(app_wigets_list[count], M2_APP_ICON_SIZE);
    //         lv_obj_set_height(app_wigets_list[count], M2_APP_ICON_SIZE+M2_APP_FONT_SIZE);
    //         lv_imgbtn_set_src(app_wigets_list[count], LV_IMGBTN_STATE_PRESSED, NULL, app_icon_list[count], NULL);
    //         lv_imgbtn_set_src(app_wigets_list[count], LV_IMGBTN_STATE_RELEASED, NULL, app_icon_list[count], NULL);
    //         lv_obj_align(app_wigets_list[count], LV_ALIGN_TOP_LEFT, app_x_offset, app_y_offset);
    //         lv_obj_set_user_data(app_wigets_list[count], (uint16_t)count);
    //         lv_obj_add_event_cb(app_wigets_list[count], menu2_app_click_cb, LV_EVENT_CLICKED, NULL);
    //         app_x_offset += M2_APP_FONT_SIZE; 
    //         app_x_offset += M2_APP_ICON_SIZE;  
    //         app_name_list[count] = lv_label_create(ui_Menu_Incon2);
    //         lv_label_set_text(app_name_list[count],app_name[count]);
    //         lv_obj_set_style_text_font(app_name_list[count], &lv_font_montserrat_12, 0);
    //         lv_obj_align_to(app_name_list[count], app_wigets_list[count], LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    //         count += 1;
    //         // APP_TRACE(icon_list,P_VALUE,"%d %d %d %d",count,pages,app_x_offset,app_y_offset);
    //     }
    //     app_y_offset += M2_APP_ICON_SIZE; 
    //     app_y_offset += M2_APP_FONT_SIZE;  
    //     app_y_offset += M2_APP_FONT_SIZE/2;  // 40 + 12 + 6
    // }
    ui_Scrolldots2 = ui_Scrolldots_create(ui_Menu_Incon2);
    lv_obj_set_x( ui_Scrolldots2, 0 );
    lv_obj_set_y( ui_Scrolldots2, -8 );
    lv_obj_set_width( ui_comp_get_child(ui_Scrolldots2, UI_COMP_SCROLLDOTS_D1), 4);
    lv_obj_set_height( ui_comp_get_child(ui_Scrolldots2, UI_COMP_SCROLLDOTS_D1), 4);
    lv_obj_set_x( ui_comp_get_child(ui_Scrolldots2, UI_COMP_SCROLLDOTS_D2), 10 );
    lv_obj_set_y( ui_comp_get_child(ui_Scrolldots2, UI_COMP_SCROLLDOTS_D2), 0 );

    lv_obj_set_width( ui_comp_get_child(ui_Scrolldots2, UI_COMP_SCROLLDOTS_D3), 8);
    lv_obj_set_height( ui_comp_get_child(ui_Scrolldots2, UI_COMP_SCROLLDOTS_D3), 8);
    lv_obj_set_x( ui_comp_get_child(ui_Scrolldots2, UI_COMP_SCROLLDOTS_D3), 21 );
    lv_obj_set_y( ui_comp_get_child(ui_Scrolldots2, UI_COMP_SCROLLDOTS_D3), 0 );
    lv_obj_set_style_bg_color(ui_comp_get_child(ui_Scrolldots2, UI_COMP_SCROLLDOTS_D3), lv_color_hex(0x101C52), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(ui_comp_get_child(ui_Scrolldots2, UI_COMP_SCROLLDOTS_D3), 255, LV_PART_MAIN| LV_STATE_DEFAULT);
}

void ui_event_Menu2(lv_event_t * e) 
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    if (event_code == LV_EVENT_SCREEN_LOADED) 
    {
        // scrolldot_Animation(ui_Scrolldots2, 0);
    }
    if ( event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_LEFT ) 
    {
        lv_indev_wait_release(lv_indev_get_act());
        _ui_screen_change( &ui_Home, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Home_screen_init);
    }
    if ( event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT ) 
    {
        lv_indev_wait_release(lv_indev_get_act());
        _ui_screen_change( &ui_Menu1, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Menu1_screen_init);
    }
}


void ui_Menu2_screen_init(void)
{
    ui_Menu2 = lv_obj_create(NULL);
    lv_obj_clear_flag( ui_Menu2, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    desktop_menu2 = lv_tabview_create(ui_Menu2, LV_DIR_TOP, 0);
    ui_Menu2_create(desktop_menu2);
    lv_obj_add_event_cb(ui_Menu2, ui_event_Menu2, LV_EVENT_ALL, NULL);
}


    // ui_Scrolldots3 = ui_Scrolldots_create(ui_Music_Player);
    // lv_obj_set_x( ui_Scrolldots3, 0 );
    // lv_obj_set_y( ui_Scrolldots3, -8 );

    // lv_obj_set_width( ui_comp_get_child(ui_Scrolldots3, UI_COMP_SCROLLDOTS_D1), 4);
    // lv_obj_set_height( ui_comp_get_child(ui_Scrolldots3, UI_COMP_SCROLLDOTS_D1), 4);

    // lv_obj_set_x( ui_comp_get_child(ui_Scrolldots3, UI_COMP_SCROLLDOTS_D2), 10 );
    // lv_obj_set_y( ui_comp_get_child(ui_Scrolldots3, UI_COMP_SCROLLDOTS_D2), 0 );

    // lv_obj_set_x( ui_comp_get_child(ui_Scrolldots3, UI_COMP_SCROLLDOTS_D3), 20 );
    // lv_obj_set_y( ui_comp_get_child(ui_Scrolldots3, UI_COMP_SCROLLDOTS_D3), 0 );

    // lv_obj_set_width( ui_comp_get_child(ui_Scrolldots3, UI_COMP_SCROLLDOTS_D4), 8);
    // lv_obj_set_height( ui_comp_get_child(ui_Scrolldots3, UI_COMP_SCROLLDOTS_D4), 8);
    // lv_obj_set_x( ui_comp_get_child(ui_Scrolldots3, UI_COMP_SCROLLDOTS_D4), 31 );
    // lv_obj_set_y( ui_comp_get_child(ui_Scrolldots3, UI_COMP_SCROLLDOTS_D4), 0 );
    // lv_obj_set_style_bg_color(ui_comp_get_child(ui_Scrolldots3, UI_COMP_SCROLLDOTS_D4), lv_color_hex(0x101C52), LV_PART_MAIN | LV_STATE_DEFAULT );
    // lv_obj_set_style_bg_opa(ui_comp_get_child(ui_Scrolldots3, UI_COMP_SCROLLDOTS_D4), 255, LV_PART_MAIN| LV_STATE_DEFAULT);


    // ui_Scrolldots4 = ui_Scrolldots_create(ui_Weather);
    // lv_obj_set_x( ui_Scrolldots4, 0 );
    // lv_obj_set_y( ui_Scrolldots4, -8 );

    // lv_obj_set_width( ui_comp_get_child(ui_Scrolldots4, UI_COMP_SCROLLDOTS_D1), 4);
    // lv_obj_set_height( ui_comp_get_child(ui_Scrolldots4, UI_COMP_SCROLLDOTS_D1), 4);

    // lv_obj_set_x( ui_comp_get_child(ui_Scrolldots4, UI_COMP_SCROLLDOTS_D2), 10 );
    // lv_obj_set_y( ui_comp_get_child(ui_Scrolldots4, UI_COMP_SCROLLDOTS_D2), 0 );

    // lv_obj_set_x( ui_comp_get_child(ui_Scrolldots4, UI_COMP_SCROLLDOTS_D3), 20 );
    // lv_obj_set_y( ui_comp_get_child(ui_Scrolldots4, UI_COMP_SCROLLDOTS_D3), 0 );

    // lv_obj_set_x( ui_comp_get_child(ui_Scrolldots4, UI_COMP_SCROLLDOTS_D4), 30 );
    // lv_obj_set_y( ui_comp_get_child(ui_Scrolldots4, UI_COMP_SCROLLDOTS_D4), 0 );

    // lv_obj_set_width( ui_comp_get_child(ui_Scrolldots4, UI_COMP_SCROLLDOTS_D5), 8);
    // lv_obj_set_height( ui_comp_get_child(ui_Scrolldots4, UI_COMP_SCROLLDOTS_D5), 8);
    // lv_obj_set_x( ui_comp_get_child(ui_Scrolldots4, UI_COMP_SCROLLDOTS_D5), 41 );
    // lv_obj_set_y( ui_comp_get_child(ui_Scrolldots4, UI_COMP_SCROLLDOTS_D5), 0 );
    // lv_obj_set_style_bg_color(ui_comp_get_child(ui_Scrolldots4, UI_COMP_SCROLLDOTS_D5), lv_color_hex(0x101C52), LV_PART_MAIN | LV_STATE_DEFAULT );
    // lv_obj_set_style_bg_opa(ui_comp_get_child(ui_Scrolldots4, UI_COMP_SCROLLDOTS_D5), 255, LV_PART_MAIN| LV_STATE_DEFAULT);


    // ui_Scrolldots5 = ui_Scrolldots_create(ui_Alarm);
    // lv_obj_set_x( ui_Scrolldots5, 0 );
    // lv_obj_set_y( ui_Scrolldots5, -8 );

    // lv_obj_set_width( ui_comp_get_child(ui_Scrolldots5, UI_COMP_SCROLLDOTS_D1), 4);
    // lv_obj_set_height( ui_comp_get_child(ui_Scrolldots5, UI_COMP_SCROLLDOTS_D1), 4);

    // lv_obj_set_x( ui_comp_get_child(ui_Scrolldots5, UI_COMP_SCROLLDOTS_D2), 10 );
    // lv_obj_set_y( ui_comp_get_child(ui_Scrolldots5, UI_COMP_SCROLLDOTS_D2), 0 );

    // lv_obj_set_x( ui_comp_get_child(ui_Scrolldots5, UI_COMP_SCROLLDOTS_D3), 20 );
    // lv_obj_set_y( ui_comp_get_child(ui_Scrolldots5, UI_COMP_SCROLLDOTS_D3), 0 );

    // lv_obj_set_x( ui_comp_get_child(ui_Scrolldots5, UI_COMP_SCROLLDOTS_D4), 30 );
    // lv_obj_set_y( ui_comp_get_child(ui_Scrolldots5, UI_COMP_SCROLLDOTS_D4), 0 );

    // lv_obj_set_x( ui_comp_get_child(ui_Scrolldots5, UI_COMP_SCROLLDOTS_D5), 40 );
    // lv_obj_set_y( ui_comp_get_child(ui_Scrolldots5, UI_COMP_SCROLLDOTS_D5), 0 );

    // lv_obj_set_width( ui_comp_get_child(ui_Scrolldots5, UI_COMP_SCROLLDOTS_D6), 8);
    // lv_obj_set_height( ui_comp_get_child(ui_Scrolldots5, UI_COMP_SCROLLDOTS_D6), 8);
    // lv_obj_set_x( ui_comp_get_child(ui_Scrolldots5, UI_COMP_SCROLLDOTS_D6), 50 );
    // lv_obj_set_y( ui_comp_get_child(ui_Scrolldots5, UI_COMP_SCROLLDOTS_D6), 0 );
    // lv_obj_set_style_bg_color(ui_comp_get_child(ui_Scrolldots5, UI_COMP_SCROLLDOTS_D6), lv_color_hex(0x101C52), LV_PART_MAIN | LV_STATE_DEFAULT );
    // lv_obj_set_style_bg_opa(ui_comp_get_child(ui_Scrolldots5, UI_COMP_SCROLLDOTS_D6), 255, LV_PART_MAIN| LV_STATE_DEFAULT);


