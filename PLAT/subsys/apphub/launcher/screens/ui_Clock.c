
#include "../ui.h"

lv_obj_t *ui_Clock;
lv_obj_t *ui_Clock_Panel;
lv_obj_t *ui_Dot1;
lv_obj_t *ui_Dot2;
lv_obj_t *ui_Dot3;
lv_obj_t *ui_Dot4;
lv_obj_t *ui_Dot5;
lv_obj_t *ui_Dot6;
lv_obj_t *ui_Dot7;
lv_obj_t *ui_Dot8;
lv_obj_t *ui_Clock_Number1;
lv_obj_t *ui_Clock_Number2;
lv_obj_t *ui_Clock_Number3;
lv_obj_t *ui_Clock_Number4;
lv_obj_t *ui_Min;
lv_obj_t *ui_Hour;
lv_obj_t *ui_Sec;
lv_obj_t *ui_Clock_Center;
lv_obj_t *ui_Clock_Number;
// lv_obj_t *ui_Date;


void hour_Animation( lv_obj_t *TargetObject, int delay,int32_t value);
void min_Animation( lv_obj_t *TargetObject, int delay, int32_t value);
void sec_Animation( lv_obj_t *TargetObject, int delay, int32_t value);
extern uint8_t hour;
extern uint8_t mins;
void ui_event_Clock( lv_event_t * e) 
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    if ( event_code == LV_EVENT_SCREEN_LOAD_START) 
    {      
        hour_Animation(ui_Hour, 200, (hour%12)*300+mins*5);
        min_Animation(ui_Min, 400,mins*60);
        // Up_Animation(ui_Clock_Panel, 100);
        // Up_Animation(ui_Clock_Number, 300);
        // Up_Animation(ui_Date, 200);
    }
    if ( event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_LEFT) 
    {
        lv_indev_wait_release(lv_indev_get_act());
        _ui_screen_change( &ui_Home, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Home_screen_init);
    }
    else if ( event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT) 
    {
        lv_indev_wait_release(lv_indev_get_act());
        _ui_screen_change( &ui_Menu1, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Menu1_screen_init);
    }
    else if( event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) ==LV_DIR_BOTTOM) 
    {
        lv_indev_wait_release(lv_indev_get_act());
        _ui_screen_change(&ui_Setting, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Setting_screen_init);
    }
    else if( event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_TOP) 
    {
        lv_indev_wait_release(lv_indev_get_act());
        _ui_screen_change(&ui_Dial, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Dial_screen_init);
    }
}

void ui_Clock_screen_init(void)
{
    ui_Clock = lv_obj_create(NULL);
    lv_obj_clear_flag( ui_Clock, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_obj_set_style_bg_img_src( ui_Clock, &ui_img_pattern_png, LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_img_tiled(ui_Clock, true, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_Clock_Panel = lv_obj_create(ui_Clock);
    lv_obj_set_width( ui_Clock_Panel, 180);
    lv_obj_set_height( ui_Clock_Panel, 180);
    lv_obj_set_x( ui_Clock_Panel, 0 );
    lv_obj_set_y( ui_Clock_Panel, 40 );
    lv_obj_set_align( ui_Clock_Panel, LV_ALIGN_CENTER );
    lv_obj_clear_flag( ui_Clock_Panel, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_obj_set_style_radius(ui_Clock_Panel, 500, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Clock_Panel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(ui_Clock_Panel, 0, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_Dot1 = ui_Clock_Dot_create(ui_Clock_Panel);
    lv_obj_set_x( ui_Dot1, -40 );
    lv_obj_set_y( ui_Dot1, 15 );
    lv_obj_set_align( ui_Dot1, LV_ALIGN_TOP_RIGHT );

    ui_Dot2 = ui_Clock_Dot_create(ui_Clock_Panel);
    lv_obj_set_x( ui_Dot2, -10 );
    lv_obj_set_y( ui_Dot2, 50 );

    ui_Dot3 = ui_Clock_Dot_create(ui_Clock_Panel);
    lv_obj_set_x( ui_Dot3, 40 );
    lv_obj_set_y( ui_Dot3, 15 );
    lv_obj_set_align( ui_Dot3, LV_ALIGN_TOP_LEFT );

    ui_Dot4 = ui_Clock_Dot_create(ui_Clock_Panel);
    lv_obj_set_x( ui_Dot4, 10 );
    lv_obj_set_y( ui_Dot4, 50 );
    lv_obj_set_align( ui_Dot4, LV_ALIGN_TOP_LEFT );

    ui_Dot5 = ui_Clock_Dot_create(ui_Clock_Panel);
    lv_obj_set_x( ui_Dot5, 10 );
    lv_obj_set_y( ui_Dot5, -50 );
    lv_obj_set_align( ui_Dot5, LV_ALIGN_BOTTOM_LEFT );

    ui_Dot6 = ui_Clock_Dot_create(ui_Clock_Panel);
    lv_obj_set_x( ui_Dot6, 40 );
    lv_obj_set_y( ui_Dot6, -15 );
    lv_obj_set_align( ui_Dot6, LV_ALIGN_BOTTOM_LEFT );

    ui_Dot7 = ui_Clock_Dot_create(ui_Clock_Panel);
    lv_obj_set_x( ui_Dot7, -10 );
    lv_obj_set_y( ui_Dot7, -50 );
    lv_obj_set_align( ui_Dot7, LV_ALIGN_BOTTOM_RIGHT );

    ui_Dot8 = ui_Clock_Dot_create(ui_Clock_Panel);
    lv_obj_set_x( ui_Dot8, -40 );
    lv_obj_set_y( ui_Dot8, -15 );
    lv_obj_set_align( ui_Dot8, LV_ALIGN_BOTTOM_RIGHT );

    ui_Clock_Number1 = ui_Small_Label_create(ui_Clock_Panel);
    lv_obj_set_x( ui_Clock_Number1, 0 );
    lv_obj_set_y( ui_Clock_Number1, 0 );

    ui_Clock_Number2 = ui_Small_Label_create(ui_Clock_Panel);
    lv_obj_set_x( ui_Clock_Number2, 0 );
    lv_obj_set_y( ui_Clock_Number2, 0 );
    lv_obj_set_align( ui_Clock_Number2, LV_ALIGN_BOTTOM_MID );
    lv_label_set_text(ui_Clock_Number2,"6");

    ui_Clock_Number3 = ui_Small_Label_create(ui_Clock_Panel);
    lv_obj_set_x( ui_Clock_Number3, 0 );
    lv_obj_set_y( ui_Clock_Number3, 0 );
    lv_obj_set_align( ui_Clock_Number3, LV_ALIGN_LEFT_MID );
    lv_label_set_text(ui_Clock_Number3,"9");

    ui_Clock_Number4 = ui_Small_Label_create(ui_Clock_Panel);
    lv_obj_set_x( ui_Clock_Number4, 0 );
    lv_obj_set_y( ui_Clock_Number4, 0 );
    lv_obj_set_align( ui_Clock_Number4, LV_ALIGN_RIGHT_MID );
    lv_label_set_text(ui_Clock_Number4,"3");

    ui_Min = lv_img_create(ui_Clock_Panel);
    lv_img_set_src(ui_Min, &ui_img_clock_min_png);
    lv_obj_set_width( ui_Min, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height( ui_Min, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_align( ui_Min, LV_ALIGN_CENTER );
    lv_obj_add_flag( ui_Min, LV_OBJ_FLAG_ADV_HITTEST );   /// Flags
    lv_obj_clear_flag( ui_Min, LV_OBJ_FLAG_SCROLLABLE );    /// Flags

    ui_Hour = lv_img_create(ui_Clock_Panel);
    lv_img_set_src(ui_Hour, &ui_img_clock_hour_png);
    lv_obj_set_width( ui_Hour, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height( ui_Hour, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_align( ui_Hour, LV_ALIGN_CENTER );
    lv_obj_add_flag( ui_Hour, LV_OBJ_FLAG_ADV_HITTEST );   /// Flags
    lv_obj_clear_flag( ui_Hour, LV_OBJ_FLAG_SCROLLABLE );    /// Flags

    ui_Sec = lv_img_create(ui_Clock_Panel);
    lv_img_set_src(ui_Sec, &ui_img_clock_sec_png);
    lv_obj_set_width( ui_Sec, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height( ui_Sec, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_align( ui_Sec, LV_ALIGN_CENTER );
    lv_obj_add_flag( ui_Sec, LV_OBJ_FLAG_ADV_HITTEST );   /// Flags
    lv_obj_clear_flag( ui_Sec, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_img_set_angle(ui_Sec,150);

    ui_Clock_Center = lv_obj_create(ui_Clock_Panel);
    lv_obj_set_width( ui_Clock_Center, 8);
    lv_obj_set_height( ui_Clock_Center, 8);
    lv_obj_set_align( ui_Clock_Center, LV_ALIGN_CENTER );
    lv_obj_clear_flag( ui_Clock_Center, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_obj_set_style_radius(ui_Clock_Center, 10, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Clock_Center, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(ui_Clock_Center, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_Clock_Center, lv_color_hex(0x1937D2), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_border_opa(ui_Clock_Center, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Clock_Center, 2, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_Clock_Number = lv_label_create(ui_Clock);
    lv_obj_set_width( ui_Clock_Number, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height( ui_Clock_Number, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_x( ui_Clock_Number, 0 );
    lv_obj_set_y( ui_Clock_Number, 24 );
    lv_obj_set_align( ui_Clock_Number, LV_ALIGN_TOP_MID );
    lv_label_set_text(ui_Clock_Number,"20:00");
    lv_obj_set_style_text_color(ui_Clock_Number, lv_color_hex(0x293062), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(ui_Clock_Number, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Clock_Number, &ui_font_Number, LV_PART_MAIN| LV_STATE_DEFAULT);

    // ui_Date = ui_Small_Label_create(ui_Clock);
    // lv_obj_set_x( ui_Date, 0 );
    // lv_obj_set_y( ui_Date, 80 );
    // lv_label_set_text(ui_Date,"Mon 28 Oct");
    // lv_obj_set_style_text_color(ui_Date, lv_color_hex(0x9C9CD9), LV_PART_MAIN | LV_STATE_DEFAULT );
    // lv_obj_set_style_text_opa(ui_Date, 255, LV_PART_MAIN| LV_STATE_DEFAULT);

    lv_obj_add_event_cb(ui_Clock, ui_event_Clock, LV_EVENT_ALL, NULL);
}




void hour_Animation( lv_obj_t *TargetObject, int delay,int32_t value)
{
    ui_anim_user_data_t *PropertyAnimation_0_user_data = lv_mem_alloc(sizeof(ui_anim_user_data_t));
    PropertyAnimation_0_user_data->target = TargetObject;
    PropertyAnimation_0_user_data->val = -1;
    lv_anim_t PropertyAnimation_0;
    lv_anim_init(&PropertyAnimation_0);
    lv_anim_set_time(&PropertyAnimation_0, 1000);
    lv_anim_set_user_data(&PropertyAnimation_0, PropertyAnimation_0_user_data);
    lv_anim_set_custom_exec_cb(&PropertyAnimation_0, _ui_anim_callback_set_image_angle );
    lv_anim_set_values(&PropertyAnimation_0, 0, value );
    lv_anim_set_path_cb( &PropertyAnimation_0, lv_anim_path_ease_out);
    lv_anim_set_delay( &PropertyAnimation_0, delay + 0 );
    lv_anim_set_deleted_cb( &PropertyAnimation_0, _ui_anim_callback_free_user_data );
    lv_anim_set_playback_time(&PropertyAnimation_0, 0);
    lv_anim_set_playback_delay(&PropertyAnimation_0, 0);
    lv_anim_set_repeat_count(&PropertyAnimation_0, 0);
    lv_anim_set_repeat_delay(&PropertyAnimation_0, 0);
    lv_anim_set_early_apply( &PropertyAnimation_0, false );
    lv_anim_start(&PropertyAnimation_0);
    ui_anim_user_data_t *PropertyAnimation_1_user_data = lv_mem_alloc(sizeof(ui_anim_user_data_t));
    PropertyAnimation_1_user_data->target = TargetObject;
    PropertyAnimation_1_user_data->val = -1;
    lv_anim_t PropertyAnimation_1;
    lv_anim_init(&PropertyAnimation_1);
    lv_anim_set_time(&PropertyAnimation_1, 300);
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
void min_Animation( lv_obj_t *TargetObject, int delay,int32_t value)
{
    ui_anim_user_data_t *PropertyAnimation_0_user_data = lv_mem_alloc(sizeof(ui_anim_user_data_t));
    PropertyAnimation_0_user_data->target = TargetObject;
    PropertyAnimation_0_user_data->val = -1;
    lv_anim_t PropertyAnimation_0;
    lv_anim_init(&PropertyAnimation_0);
    lv_anim_set_time(&PropertyAnimation_0, 1000);
    lv_anim_set_user_data(&PropertyAnimation_0, PropertyAnimation_0_user_data);
    lv_anim_set_custom_exec_cb(&PropertyAnimation_0, _ui_anim_callback_set_image_angle );
    lv_anim_set_values(&PropertyAnimation_0, 0, value );
    lv_anim_set_path_cb( &PropertyAnimation_0, lv_anim_path_ease_out);
    lv_anim_set_delay( &PropertyAnimation_0, delay + 0 );
    lv_anim_set_deleted_cb( &PropertyAnimation_0, _ui_anim_callback_free_user_data );
    lv_anim_set_playback_time(&PropertyAnimation_0, 0);
    lv_anim_set_playback_delay(&PropertyAnimation_0, 0);
    lv_anim_set_repeat_count(&PropertyAnimation_0, 0);
    lv_anim_set_repeat_delay(&PropertyAnimation_0, 0);
    lv_anim_set_early_apply( &PropertyAnimation_0, false );
    lv_anim_start(&PropertyAnimation_0);
    ui_anim_user_data_t *PropertyAnimation_1_user_data = lv_mem_alloc(sizeof(ui_anim_user_data_t));
    PropertyAnimation_1_user_data->target = TargetObject;
    PropertyAnimation_1_user_data->val = -1;
    lv_anim_t PropertyAnimation_1;
    lv_anim_init(&PropertyAnimation_1);
    lv_anim_set_time(&PropertyAnimation_1, 200);
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
void sec_Animation( lv_obj_t *TargetObject, int delay, int32_t value)
{
    ui_anim_user_data_t *PropertyAnimation_0_user_data = lv_mem_alloc(sizeof(ui_anim_user_data_t));
    PropertyAnimation_0_user_data->target = TargetObject;
    PropertyAnimation_0_user_data->val = -1;
    lv_anim_t PropertyAnimation_0;
    lv_anim_init(&PropertyAnimation_0);
    lv_anim_set_time(&PropertyAnimation_0, 60000);
    lv_anim_set_user_data(&PropertyAnimation_0, PropertyAnimation_0_user_data);
    lv_anim_set_custom_exec_cb(&PropertyAnimation_0, _ui_anim_callback_set_image_angle );
    lv_anim_set_values(&PropertyAnimation_0, 0, value );
    lv_anim_set_path_cb( &PropertyAnimation_0, lv_anim_path_linear);
    lv_anim_set_delay( &PropertyAnimation_0, delay + 0 );
    lv_anim_set_deleted_cb( &PropertyAnimation_0, _ui_anim_callback_free_user_data );
    lv_anim_set_playback_time(&PropertyAnimation_0, 0);
    lv_anim_set_playback_delay(&PropertyAnimation_0, 0);
    lv_anim_set_repeat_count(&PropertyAnimation_0, LV_ANIM_REPEAT_INFINITE );
    lv_anim_set_repeat_delay(&PropertyAnimation_0, 0);
    lv_anim_set_early_apply( &PropertyAnimation_0, false );
    lv_anim_start(&PropertyAnimation_0);
    ui_anim_user_data_t *PropertyAnimation_1_user_data = lv_mem_alloc(sizeof(ui_anim_user_data_t));
    PropertyAnimation_1_user_data->target = TargetObject;
    PropertyAnimation_1_user_data->val = -1;
    lv_anim_t PropertyAnimation_1;
    lv_anim_init(&PropertyAnimation_1);
    lv_anim_set_time(&PropertyAnimation_1, 1000);
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