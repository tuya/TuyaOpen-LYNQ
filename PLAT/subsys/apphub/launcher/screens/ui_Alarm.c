
#include "../ui.h"
#include <stdio.h>
#include <stdlib.h>
#include "ostask.h"
#include "osasys.h"
#include "bsp.h"
#include "bsp_custom.h"

lv_obj_t *ui_Alarm;
lv_obj_t *ui_Alarm_container;
lv_obj_t *ui_Set_alarm;
lv_obj_t *ui_Alarm_Comp;
lv_obj_t *ui_Alarm_Comp1;
lv_obj_t *ui_Alarm_Comp2;
lv_obj_t *ui_Alarm_Comp3;

void ui_event_Alarm( lv_event_t * e) 
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    if ( event_code == LV_EVENT_SCREEN_LOADED) 
    {
        // Up_Animation(ui_Set_alarm, 100);
        // Up_Animation(ui_Alarm_Comp, 200);
        // Up_Animation(ui_Alarm_Comp1, 300);
        // Up_Animation(ui_Alarm_Comp2, 400);
        // Up_Animation(ui_Alarm_Comp3, 500);
    }
}

void ui_Alarm_screen_init(void)
{
    ui_Alarm = lv_obj_create(NULL);
    lv_obj_clear_flag( ui_Alarm, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_obj_set_style_bg_color(ui_Alarm, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(ui_Alarm, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src( ui_Alarm, &ui_img_pattern_png, LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_img_tiled(ui_Alarm, true, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_Alarm_container = lv_obj_create(ui_Alarm);
    lv_obj_set_width( ui_Alarm_container, lv_pct(100));
    lv_obj_set_height( ui_Alarm_container, lv_pct(100));
    lv_obj_set_align( ui_Alarm_container, LV_ALIGN_CENTER );
    lv_obj_set_style_bg_color(ui_Alarm_container, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(ui_Alarm_container, 0, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_Set_alarm = ui_Small_Label_create(ui_Alarm_container);
    lv_obj_set_x( ui_Set_alarm, 0 );
    lv_obj_set_y( ui_Set_alarm, 17 );
    lv_obj_set_align( ui_Set_alarm, LV_ALIGN_TOP_MID );
    lv_label_set_text(ui_Set_alarm,"Set alarm");
    lv_obj_set_style_text_color(ui_Set_alarm, lv_color_hex(0x000746), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(ui_Set_alarm, 255, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_Alarm_Comp = ui_Alarm_Comp_create(ui_Alarm_container);
    lv_obj_set_x( ui_Alarm_Comp, 0 );
    lv_obj_set_y( ui_Alarm_Comp, 43 );

    ui_Alarm_Comp1 = ui_Alarm_Comp_create(ui_Alarm_container);
    lv_obj_set_x( ui_Alarm_Comp1, 0 );
    lv_obj_set_y( ui_Alarm_Comp1, 128 );

    lv_label_set_text(ui_comp_get_child(ui_Alarm_Comp1, UI_COMP_ALARM_COMP_ALARM_NUM2),"8:00");

    lv_label_set_text(ui_comp_get_child(ui_Alarm_Comp1, UI_COMP_ALARM_COMP_PERIOD),"Breakfast");

    lv_obj_add_state( ui_comp_get_child(ui_Alarm_Comp1, UI_COMP_ALARM_COMP_SWITCH1), LV_STATE_CHECKED );     /// States


    ui_Alarm_Comp2 = ui_Alarm_Comp_create(ui_Alarm_container);
    lv_obj_set_x( ui_Alarm_Comp2, 0 );
    lv_obj_set_y( ui_Alarm_Comp2, 213 );

    lv_label_set_text(ui_comp_get_child(ui_Alarm_Comp2, UI_COMP_ALARM_COMP_ALARM_NUM2),"9:30");

    lv_label_set_text(ui_comp_get_child(ui_Alarm_Comp2, UI_COMP_ALARM_COMP_PERIOD),"Yoga");

    ui_Alarm_Comp3 = ui_Alarm_Comp_create(ui_Alarm_container);
    lv_obj_set_x( ui_Alarm_Comp3, 0 );
    lv_obj_set_y( ui_Alarm_Comp3, 298 );
    lv_obj_set_style_border_color(ui_Alarm_Comp3, lv_color_hex(0x293062), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_border_opa(ui_Alarm_Comp3, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Alarm_Comp3, 1, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_Alarm_Comp3, LV_BORDER_SIDE_NONE, LV_PART_MAIN| LV_STATE_DEFAULT);

    lv_label_set_text(ui_comp_get_child(ui_Alarm_Comp3, UI_COMP_ALARM_COMP_ALARM_NUM2),"11:00");

    lv_label_set_text(ui_comp_get_child(ui_Alarm_Comp3, UI_COMP_ALARM_COMP_PERIOD),"Sleep");

    lv_obj_add_event_cb(ui_Alarm, ui_event_Alarm, LV_EVENT_ALL, NULL);

}

