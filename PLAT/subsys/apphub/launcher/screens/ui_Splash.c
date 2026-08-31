
#include "../ui.h"
#include <stdio.h>
#include <stdlib.h>
#include "ostask.h"
#include "osasys.h"
#include "bsp.h"
#include "bsp_custom.h"
#include "mode_config.h"
#include "sctdef.h"

lv_obj_t *ui_Splash;
lv_obj_t *ui_Demo;
lv_obj_t *ui_Smart_Gadget;
lv_obj_t *ui_SLS_Logo;

#define DEFAULT_EC_LOGO     "D:/ui_img_eigencomm_png_216.bin"


void ui_event_Splash( lv_event_t * e) 
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    if ( event_code == LV_EVENT_SCREEN_LOADED) 
    {
        // Up_Animation(ui_SLS_Logo, 100);
        // Up_Animation(ui_Smart_Gadget, 50);
        // Up_Animation(ui_Demo, 300);
        _ui_screen_change(&ui_Clock, LV_SCR_LOAD_ANIM_FADE_ON, 0, 500, &ui_Clock_screen_init);
    }
}

void ui_Splash_screen_init(void)
{
    ui_Splash = lv_obj_create(NULL);
    lv_obj_clear_flag( ui_Splash, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_obj_set_style_bg_color(ui_Splash, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(ui_Splash, 255, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_Demo = ui_Small_Label_create(ui_Splash);
    lv_obj_set_x( ui_Demo, 0 );
    lv_obj_set_y( ui_Demo, 75 );
    lv_obj_set_align( ui_Demo, LV_ALIGN_CENTER );
    lv_label_set_text(ui_Demo, BOARD_NAME);
    lv_obj_set_style_text_color(ui_Demo, lv_color_hex(0x9C9CD9), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(ui_Demo, 255, LV_PART_MAIN| LV_STATE_DEFAULT);

    // ui_Smart_Gadget = ui_Small_Label_create(ui_Splash);
    // lv_obj_set_x( ui_Smart_Gadget, 0 );
    // lv_obj_set_y( ui_Smart_Gadget, 40 );
    // lv_obj_set_align( ui_Smart_Gadget, LV_ALIGN_CENTER );
    // lv_label_set_text(ui_Smart_Gadget, "上海移芯通信");
    // lv_obj_set_style_text_font(ui_Smart_Gadget, &ui_font_song16, 0);
    // lv_obj_set_style_text_color(ui_Smart_Gadget, lv_color_hex(0x000746), LV_PART_MAIN | LV_STATE_DEFAULT );
    // lv_obj_set_style_text_opa(ui_Smart_Gadget, 255, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_SLS_Logo = lv_img_create(ui_Splash);
    // lv_img_set_src(ui_SLS_Logo, DEFAULT_EC_LOGO);
    // lv_img_set_src(ui_SLS_Logo, &ui_img_eigencomm_png_216);
    lv_obj_set_width( ui_SLS_Logo, LV_SIZE_CONTENT); 
    lv_obj_set_height( ui_SLS_Logo, LV_SIZE_CONTENT);
    lv_obj_set_x( ui_SLS_Logo, 0 );
    lv_obj_set_y( ui_SLS_Logo, -40);
    lv_obj_set_align( ui_SLS_Logo, LV_ALIGN_CENTER );
    lv_obj_add_flag( ui_SLS_Logo, LV_OBJ_FLAG_ADV_HITTEST );   /// Flags
    lv_obj_clear_flag( ui_SLS_Logo, LV_OBJ_FLAG_SCROLLABLE );    /// Flags

    lv_obj_add_event_cb(ui_Splash, ui_event_Splash, LV_EVENT_ALL, NULL);
}

