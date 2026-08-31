
#include "../ui.h"
#include <stdio.h>
#include <stdlib.h>
#include "ostask.h"
#include "osasys.h"
#include "bsp.h"
#include "bsp_custom.h"

#define APP_TRACE(subId, argLen, format,  ...)  \
    ECOMM_TRACE(UNILOG_REF_APP, subId, P_VALUE, argLen, format,  ##__VA_ARGS__) 

lv_obj_t *ui_Setting;
lv_obj_t *voice_panel;
lv_obj_t *light_panel;
lv_obj_t *ui_setting_group;
lv_obj_t *lcdbk_percent_label;
lv_obj_t *lcdbk_slider;

lv_obj_t *volume_percent_label;
lv_obj_t *volume_slider;

lv_obj_t *ui_Background;
lv_obj_t * ui_Display;
lv_obj_t * ui_Label_artist;
lv_obj_t * ui_Label_song;
lv_obj_t * ui_Audio_Wave;
lv_obj_t * ui_Label_Time;
lv_obj_t * ui_Indicator_group;
lv_obj_t * ui_Label_Levels;
lv_obj_t * ui_Min1;

static lv_style_t style_text_muted;
static lv_style_t style_title;
static lv_style_t style_outline;

static lv_obj_t * meter1;
static lv_obj_t * meter2;
static lv_obj_t * meter3;

static lv_obj_t * chart1;
static lv_obj_t * chart2;
static lv_obj_t * chart3;

static lv_chart_series_t * ser1;
static lv_chart_series_t * ser2;
static lv_chart_series_t * ser3;
static lv_chart_series_t * ser4;
#ifdef FEATURE_SUBSYS_OPENHAL_ENABLE
#include "api_tp.h"
#include "api_scr.h"
#include "api_comm.h"
extern uint32_t scr_dev_UsrId;
extern uint8_t backLightVal;
#endif

uint8_t lcdBackLightVal()
{
    #ifdef FEATURE_SUBSYS_OPENHAL_ENABLE
    return backLightVal;
    #endif
}
static void slider_light_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    if(code == LV_EVENT_REFR_EXT_DRAW_SIZE) {
        lv_coord_t * s = lv_event_get_param(e);
        *s = LV_MAX(*s, 60);
    }
    else if(code == LV_EVENT_DRAW_PART_END) {
        lv_obj_draw_part_dsc_t * dsc = lv_event_get_param(e);
        if(dsc->part == LV_PART_KNOB && lv_obj_has_state(obj, LV_STATE_PRESSED)) {
            char buf[8];
            lv_snprintf(buf, sizeof(buf), "%"LV_PRId32, lv_slider_get_value(obj));
            lv_label_set_text_fmt(lcdbk_percent_label, "BackLight: %"LV_PRIu32,lv_slider_get_value(obj));
            lv_point_t text_size;
            lv_txt_get_size(&text_size, buf, &lv_font_montserrat_12, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);

            lv_area_t txt_area;
            txt_area.x1 = dsc->draw_area->x1 + lv_area_get_width(dsc->draw_area) / 2 - text_size.x / 2;
            txt_area.x2 = txt_area.x1 + text_size.x;
            txt_area.y2 = dsc->draw_area->y1 - 10;
            txt_area.y1 = txt_area.y2 - text_size.y;

            lv_area_t bg_area;
            bg_area.x1 = txt_area.x1 - LV_DPX(8);
            bg_area.x2 = txt_area.x2 + LV_DPX(8);
            bg_area.y1 = txt_area.y1 - LV_DPX(8);
            bg_area.y2 = txt_area.y2 + LV_DPX(8);

            lv_draw_rect_dsc_t rect_dsc;
            lv_draw_rect_dsc_init(&rect_dsc);
            rect_dsc.bg_color = lv_palette_darken(LV_PALETTE_GREY, 3);
            rect_dsc.radius = LV_DPX(5);
            lv_draw_rect(dsc->draw_ctx, &rect_dsc, &bg_area);

            lv_draw_label_dsc_t label_dsc;
            lv_draw_label_dsc_init(&label_dsc);
            label_dsc.color = lv_color_white();
            label_dsc.font = &lv_font_montserrat_12;
            lv_draw_label(dsc->draw_ctx, &label_dsc, &txt_area, buf, NULL);
            // extern void lcdBackLightLevel(int16_t level);
            // lcdBackLightLevel(lv_slider_get_value(obj)+4);
            #ifdef FEATURE_SUBSYS_OPENHAL_ENABLE
            backLightVal = lv_slider_get_value(obj)%100;
            api_scr_ioctl(scr_dev_UsrId,SCREEN_BACKLIGHT_SET, &backLightVal);
            #endif
        }
    }
}

void ui_Setting_Light(lv_obj_t * panel)
{
    static lv_coord_t grid_2_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t grid_2_row_dsc[] = {
        LV_GRID_CONTENT,  /*Title*/
        5,                /*Separator*/
        LV_GRID_CONTENT,  /*Box title*/
        40,               /*Box*/
        LV_GRID_CONTENT,  /*Box title*/
        40,               /*Box*/
        LV_GRID_CONTENT,  /*Box title*/
        40,               /*Box*/
        LV_GRID_CONTENT,  /*Box title*/
        40, LV_GRID_TEMPLATE_LAST               /*Box*/
    };
    lv_obj_set_grid_dsc_array(panel, grid_2_col_dsc, grid_2_row_dsc);

    lv_obj_t * panel_title = lv_label_create(panel);
    lv_label_set_text(panel_title, "Display");
    lv_obj_add_style(panel_title, &style_title, 0);

    lcdbk_percent_label = lv_label_create(panel);
    lv_label_set_text_fmt(lcdbk_percent_label, "BackLight: %"LV_PRIu32,lcdBackLightVal());
    // lv_label_set_text(lcdbk_percent_label, "BackLight: 50");
    lv_obj_add_style(lcdbk_percent_label, &style_text_muted, 0);

    lcdbk_slider = lv_slider_create(panel);
    lv_obj_set_width(lcdbk_slider, LV_PCT(80));
    lv_slider_set_range(lcdbk_slider, 0, 100);
    lv_slider_set_value(lcdbk_slider, lcdBackLightVal(), LV_ANIM_OFF);
    lv_obj_add_event_cb(lcdbk_slider, slider_light_cb, LV_EVENT_ALL, NULL);
    lv_obj_refresh_ext_draw_size(lcdbk_slider);

    lv_obj_t * team_player_label = lv_label_create(panel);
    lv_label_set_text(team_player_label, "Converter");
    lv_obj_add_style(team_player_label, &style_text_muted, 0);

    lv_obj_t * sw1 = lv_switch_create(panel);

    lv_obj_t * hard_working_label = lv_label_create(panel);
    lv_label_set_text(hard_working_label, "LE Mode");
    lv_obj_add_style(hard_working_label, &style_text_muted, 0);

    lv_obj_t * sw2 = lv_switch_create(panel);

    lv_obj_set_grid_cell(panel_title, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_set_grid_cell(lcdbk_percent_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    lv_obj_set_grid_cell(lcdbk_slider, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 3, 1);
    lv_obj_set_grid_cell(hard_working_label, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 4, 1);
    lv_obj_set_grid_cell(sw1, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 5, 1);
    lv_obj_set_grid_cell(team_player_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_START, 4, 1);
    lv_obj_set_grid_cell(sw2, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_START, 5, 1);
}

static void slider_voice_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_REFR_EXT_DRAW_SIZE) {
        lv_coord_t * s = lv_event_get_param(e);
        *s = LV_MAX(*s, 60);
    }
    else if(code == LV_EVENT_DRAW_PART_END) {
        lv_obj_draw_part_dsc_t * dsc = lv_event_get_param(e);
        if(dsc->part == LV_PART_KNOB && lv_obj_has_state(obj, LV_STATE_PRESSED)) {
            char buf[8];
            lv_snprintf(buf, sizeof(buf), "%"LV_PRId32, lv_slider_get_value(obj));
            lv_label_set_text_fmt(volume_percent_label, "Volume: %"LV_PRIu32,lv_slider_get_value(obj));
            lv_point_t text_size;
            lv_txt_get_size(&text_size, buf, &lv_font_montserrat_12, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);

            lv_area_t txt_area;
            txt_area.x1 = dsc->draw_area->x1 + lv_area_get_width(dsc->draw_area) / 2 - text_size.x / 2;
            txt_area.x2 = txt_area.x1 + text_size.x;
            txt_area.y2 = dsc->draw_area->y1 - 10;
            txt_area.y1 = txt_area.y2 - text_size.y;

            lv_area_t bg_area;
            bg_area.x1 = txt_area.x1 - LV_DPX(8);
            bg_area.x2 = txt_area.x2 + LV_DPX(8);
            bg_area.y1 = txt_area.y1 - LV_DPX(8);
            bg_area.y2 = txt_area.y2 + LV_DPX(8);

            lv_draw_rect_dsc_t rect_dsc;
            lv_draw_rect_dsc_init(&rect_dsc);
            rect_dsc.bg_color = lv_palette_darken(LV_PALETTE_GREY, 3);
            rect_dsc.radius = LV_DPX(5);
            lv_draw_rect(dsc->draw_ctx, &rect_dsc, &bg_area);

            lv_draw_label_dsc_t label_dsc;
            lv_draw_label_dsc_init(&label_dsc);
            label_dsc.color = lv_color_white();
            label_dsc.font = &lv_font_montserrat_12;
            lv_draw_label(dsc->draw_ctx, &label_dsc, &txt_area, buf, NULL);
            #ifdef FEATURE_IMS_ENABLE
            extern void PlayVolumeSet(int8_t val);
            PlayVolumeSet(lv_slider_get_value(obj));
            #endif
        }
    }
}

void ui_Setting_Voice(lv_obj_t * panel)
{
    static lv_coord_t grid_2_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t grid_2_row_dsc[] = {
        LV_GRID_CONTENT,  /*Title*/
        5,                /*Separator*/
        LV_GRID_CONTENT,  /*Box title*/
        40,               /*Box*/
        LV_GRID_CONTENT,  /*Box title*/
        40,               /*Box*/
        LV_GRID_CONTENT,  /*Box title*/
        40,               /*Box*/
        LV_GRID_CONTENT,  /*Box title*/
        40, LV_GRID_TEMPLATE_LAST               /*Box*/
    };
    lv_obj_set_grid_dsc_array(panel, grid_2_col_dsc, grid_2_row_dsc);

    lv_obj_t * voice_title = lv_label_create(panel);
    lv_label_set_text(voice_title, "Voice");
    lv_obj_add_style(voice_title, &style_title, 0);
    volume_percent_label = lv_label_create(panel);
    #ifdef FEATURE_IMS_ENABLE
    extern int8_t PlayVolumeGet(void);
    lv_label_set_text_fmt(volume_percent_label, "Volume: %"LV_PRIu32,PlayVolumeGet());
    #else 
    lv_label_set_text(volume_percent_label, "Volume: 65");
    #endif
    lv_obj_add_style(volume_percent_label, &style_text_muted, 0);

    volume_slider = lv_slider_create(panel);
    lv_obj_set_width(volume_slider, LV_PCT(80));
    lv_slider_set_range(volume_slider, 50, 75);
    #ifdef FEATURE_IMS_ENABLE
    lv_slider_set_value(volume_slider, PlayVolumeGet(), LV_ANIM_OFF);
    #else 
    lv_slider_set_value(volume_slider, 70, LV_ANIM_OFF);
    #endif
    lv_obj_add_event_cb(volume_slider, slider_voice_cb, LV_EVENT_ALL, NULL);
    lv_obj_refresh_ext_draw_size(volume_slider);

    lv_obj_t * SW_LowPower_label = lv_label_create(panel);
    lv_label_set_text(SW_LowPower_label, "DENOISE");
    lv_obj_add_style(SW_LowPower_label, &style_text_muted, 0);
    lv_obj_t * sw1 = lv_switch_create(panel);

    lv_obj_t * SW_Network_label = lv_label_create(panel);
    lv_label_set_text(SW_Network_label, "DE-ECHO");
    lv_obj_add_style(SW_Network_label, &style_text_muted, 0);

    lv_obj_t * sw2 = lv_switch_create(panel);

    lv_obj_set_grid_cell(voice_title, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_set_grid_cell(volume_percent_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_END, 2, 1);
    lv_obj_set_grid_cell(volume_slider, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 3, 1);

    lv_obj_set_grid_cell(SW_Network_label, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 4, 1);
    lv_obj_set_grid_cell(sw1, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 5, 1);
    lv_obj_set_grid_cell(SW_LowPower_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_START, 4, 1);
    lv_obj_set_grid_cell(sw2, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_START, 5, 1);
}

void ui_event_Setting(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    if ( event_code == LV_EVENT_SCREEN_LOADED) 
    {
        // scrolldot_Animation(ui_Scrolldots2, 0);
    }
    if ( event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT) 
    {
        lv_indev_wait_release(lv_indev_get_act());
        _ui_screen_change(&ui_Menu1, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Menu1_screen_init); 
    }
    else if( event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_LEFT) 
    {
        lv_indev_wait_release(lv_indev_get_act());
        _ui_screen_change(&ui_Menu1, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Menu1_screen_init); 
    }
}

void ui_Setting_screen_init(void)
{
    ui_Setting = lv_obj_create(NULL);
    lv_obj_clear_flag( ui_Setting, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_obj_set_style_bg_img_src( ui_Setting, &ui_img_pattern_png, LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_img_tiled(ui_Setting, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_style_init(&style_outline);
    lv_style_set_radius(&style_outline, 5);
    lv_style_set_bg_opa(&style_outline, LV_OPA_10);

    lv_style_init(&style_title);
    lv_style_set_text_font(&style_title, &lv_font_montserrat_16);
    lv_style_set_text_color(&style_title, lv_color_hex(0x000746));

    lv_style_init(&style_text_muted);
    lv_style_set_text_opa(&style_text_muted, LV_OPA_80);

    ui_setting_group = lv_obj_create(ui_Setting);
    lv_obj_set_style_bg_opa(ui_setting_group, LV_OPA_10, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_outline_color(ui_setting_group, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_height(ui_setting_group,lv_pct(90));
    lv_obj_set_width(ui_setting_group, lv_pct(90));
    lv_obj_set_pos(ui_setting_group, 0, 32);
    lv_obj_set_align(ui_setting_group, LV_ALIGN_TOP_MID);
    // lv_obj_center(ui_setting_group);

    static lv_coord_t grid_main_col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t grid_main_row_dsc[] = {LV_GRID_CONTENT, 20, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(ui_setting_group, grid_main_col_dsc, grid_main_row_dsc);

    voice_panel = lv_obj_create(ui_setting_group);
    lv_obj_set_height(voice_panel, LV_SIZE_CONTENT);
    lv_obj_set_width(voice_panel, lv_pct(80));
    // lv_obj_set_align(voice_panel, LV_ALIGN_TOP_MID);
    // lv_obj_set_style_radius(voice_panel, LV_RADIUS_CIRCLE, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(voice_panel, LV_OPA_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_outline_color(voice_panel, lv_color_white(), LV_PART_MAIN);
    // lv_obj_set_style_outline_width(voice_panel, 3, LV_PART_MAIN);
    lv_obj_add_style(voice_panel, &style_outline, 0);
    lv_obj_set_grid_cell(voice_panel, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    ui_Setting_Voice(voice_panel);

    light_panel = lv_obj_create(ui_setting_group);
    lv_obj_set_width(light_panel, lv_pct(80));
     lv_obj_set_height(light_panel, LV_SIZE_CONTENT);
    lv_obj_set_align(light_panel, LV_ALIGN_TOP_MID);
    lv_obj_set_style_bg_opa(light_panel, LV_OPA_20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(light_panel, &style_outline, 0);
    lv_obj_set_grid_cell(light_panel, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 2, 1);
    ui_Setting_Light(light_panel);

    lv_obj_add_event_cb(ui_Setting, ui_event_Setting, LV_EVENT_ALL, NULL);
}



