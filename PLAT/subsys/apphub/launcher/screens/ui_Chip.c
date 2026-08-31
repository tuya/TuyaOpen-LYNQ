
#include "../ui.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "ostask.h"
#include "osasys.h"
#include "bsp.h"
#include "bsp_custom.h"
#include "mode_config.h"
#include "sctdef.h"

lv_obj_t *ui_Chip;
lv_obj_t *ui_Chip_container;
lv_obj_t *ui_Chip_name;
lv_obj_t *ui_info_main;
lv_obj_t *ui_info_main_name;
lv_obj_t *ui_info_panel;
lv_obj_t *ui_info_main_Text;


static uint16_t info_panel_height = 80;

void ui_event_Chip(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    if ( event_code == LV_EVENT_SCREEN_LOADED) 
    {
        lv_label_set_text(ui_info_main_Text,"测试中文字库加载速度，每次打开页面重新加载"); 
    }
    if ( event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT) 
    {
        lv_indev_wait_release(lv_indev_get_act());
        _ui_screen_change(&ui_Menu1, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Menu1_screen_init); 
    }
    if ( event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_LEFT) 
    {
        lv_indev_wait_release(lv_indev_get_act());
        _ui_screen_change(&ui_Menu1, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Menu1_screen_init); 
    }
}



void ui_chip_debug_create(lv_obj_t * container)
{
    ui_info_main = lv_obj_create(container);
    lv_obj_set_width( ui_info_main, lv_pct(100));
    lv_obj_set_height( ui_info_main, lv_pct(90));
    lv_obj_align_to(ui_info_main, ui_Chip_name, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);
    lv_obj_clear_flag( ui_info_main, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_obj_set_style_bg_color(ui_info_main, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(ui_info_main, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_info_main, 12, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_info_main, 12, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_info_main, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_info_main, 0, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_info_main_name = ui_Small_Label_create(ui_info_main);
    lv_obj_align(ui_info_main_name, LV_ALIGN_TOP_LEFT, 0, 0);
    char RspBuf[30];
    sprintf(RspBuf,"%s",SOFTVERSION);
    lv_label_set_text(ui_info_main_name,RspBuf);
    lv_obj_set_style_text_color(ui_info_main_name, lv_color_hex(0x293062), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(ui_info_main_name, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_info_main_name, &lv_font_montserrat_12, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_info_panel = lv_obj_create(ui_info_main);
    lv_obj_set_height(ui_info_panel,lv_pct(85));  
    lv_obj_set_width( ui_info_panel, lv_pct(100));
    lv_obj_align(ui_info_panel, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_clear_flag( ui_info_panel, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_obj_set_style_radius(ui_info_panel, 12, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_info_panel, lv_color_hex(0x293062), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(ui_info_panel, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_info_panel, 8, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_info_panel, 8, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_info_panel, 8, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_info_panel, 8, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_info_main_Text = ui_Small_Label_create(ui_info_panel);
    lv_obj_set_width( ui_info_main_Text, lv_pct(100));
    lv_obj_set_height( ui_info_main_Text, LV_SIZE_CONTENT); 
    lv_obj_align(ui_info_main_Text, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_label_set_text(ui_info_main_Text,"测试"); 
    lv_obj_set_style_text_color(ui_info_main_Text, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(ui_info_main_Text, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_info_main_Text, &ui_font_song16, LV_PART_MAIN| LV_STATE_DEFAULT);

    lv_obj_t *ui_info_gcc_time = ui_Small_Label_create(ui_info_main);
    lv_obj_align_to(ui_info_gcc_time, ui_info_panel, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);
    memset(RspBuf,0,sizeof(RspBuf));
    sprintf(RspBuf, "%s %s", __DATE__, __TIME__);
    lv_label_set_text(ui_info_gcc_time,RspBuf);
    lv_obj_set_style_text_color(ui_info_gcc_time, lv_color_hex(0x293062), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(ui_info_gcc_time, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_info_gcc_time, &lv_font_montserrat_12, LV_PART_MAIN| LV_STATE_DEFAULT);
}
// FlashXIPSize = (uint32_t)&flashXIPLimit;

void ui_Chip_screen_init(void)
{
    ui_Chip = lv_obj_create(NULL);
    lv_obj_clear_flag( ui_Chip, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_obj_set_style_bg_img_src( ui_Chip, &ui_img_pattern_png, LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_img_tiled(ui_Chip, true, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_Chip_container = lv_obj_create(ui_Chip);
    lv_obj_set_width( ui_Chip_container, lv_pct(100));
    lv_obj_set_height( ui_Chip_container, lv_pct(80));
    lv_obj_set_align( ui_Chip_container, LV_ALIGN_CENTER );
    lv_obj_set_style_bg_color(ui_Chip_container, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(ui_Chip_container, 0, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_Chip_name = ui_Small_Label_create(ui_Chip_container);
    lv_obj_align(ui_Chip_name, LV_ALIGN_TOP_MID, 0, 0);
    lv_label_set_text(ui_Chip_name,BOARD_NAME);
    lv_obj_set_style_text_color(ui_Chip_name, lv_color_hex(0x000746), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(ui_Chip_name, 255, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_chip_debug_create(ui_Chip_container);

    lv_obj_add_event_cb(ui_Chip, ui_event_Chip, LV_EVENT_ALL, NULL);

}

int ui_printf(const char *format, ...)
{
    static char buff[128];
    va_list args;
    va_start(args, format);
    vsnprintf(buff, sizeof(buff),format, args);
    va_end(args);
    lv_label_set_text(ui_info_main_Text,buff); 
}