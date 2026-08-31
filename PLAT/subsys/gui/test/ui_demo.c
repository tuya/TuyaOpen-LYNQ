/**
 * @file ui_demo.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "ui_demo.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    DISP_SMALL,
    DISP_LARGE,
} disp_size_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void ui_demo(void)
{    
    lv_obj_t* desktop = ui_desktop_create(3, 1);
    //lv_obj_t* desktop = ui_page_create(NULL, false, false);
    // measure_time(NULL);
    // uint32_t cost_tick = 0;
    lv_scr_load(desktop);   //194us,5047
    // uint32_t cost_us = measure_time(&cost_tick);
    // SYSLOG_INFO("lv_scr_load %d,%d\r\n",cost_us,cost_tick);
    lv_obj_set_style_bg_color(ui_page_get_part(desktop, UI_PAGE_PART_PLACEHOLD), lv_color_black(), 0);
    lv_obj_set_style_bg_color(ui_page_get_content(desktop), lv_palette_main(LV_PALETTE_BLUE), 0);
    //lv_obj_set_style_bg_color(ui_page_get_part(desktop, UI_PAGE_PART_TITLE_BAR), lv_palette_main(LV_PALETTE_RED), 0);

    static const lv_palette_t color_list[] = { LV_PALETTE_GREEN, LV_PALETTE_PINK ,LV_PALETTE_ORANGE, LV_PALETTE_LIGHT_BLUE, LV_PALETTE_LIME };
    for (uint32_t i = 0; i < ui_desktop_get_frame_count(desktop); i++) {
        lv_obj_t* t = ui_desktop_get_frame(desktop, i);
        int index = i % (sizeof(color_list) / sizeof(color_list[0]));
        lv_obj_set_style_bg_color(t, lv_palette_main(color_list[index]), 0);
    }
    // SYSLOG_INFO("lv_obj_set_style_bg_color %d-%d=%d\r\n",endTime,startTime,(endTime-startTime)&0x1FFFFFFF);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
