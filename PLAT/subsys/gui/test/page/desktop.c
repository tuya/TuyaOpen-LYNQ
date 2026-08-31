/**
 * @file desktop.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
 #include "lvgl.h"
#include "../ui_conf.h"
#include "page.h"
#include "desktop.h"


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
static lv_obj_t* indicator_create(lv_obj_t* parent, uint32_t count, uint32_t home_index);
static void tab_changed_cb(lv_event_t* e);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
lv_obj_t* ui_desktop_create(uint32_t frame_count, uint32_t home_index)
{
    lv_obj_t* desktop;
    lv_obj_t* tv;
    lv_obj_t* indicator;

    if (frame_count == 0) {
        frame_count = 1;
    }
    if (frame_count > UI_DESKTOP_FRAME_COUNT_MAX) {
        frame_count = UI_DESKTOP_FRAME_COUNT_MAX;
    }
    if (home_index >= frame_count) {
        home_index = frame_count;
    }

    // desktop是一个page, 以page的content为parent创建一个tabview(tab按钮不显示), 以便使用tabview的左右滑动效果
    // 再以tabview为parent创建一个indicator(页面指示器, 底部的几个小圆点)

    desktop = ui_page_create(NULL, UI_PAGE_RETURN_DISABLE, UI_PAGE_CLOSE_DISABLE);
    uint8_t i;

    // tv必须为ui_page_content的第一个child
    tv = lv_tabview_create(ui_page_get_content(desktop), LV_DIR_TOP, 0);
    
    for (i = 0; i < frame_count; i++) {
        char buf[10];        
        lv_snprintf(buf, 10, "tab%d", i);
        lv_obj_t* t = lv_tabview_add_tab(tv, buf);
        lv_obj_set_style_bg_opa(t, LV_OPA_100, 0);
    }
    lv_obj_set_user_data(desktop, (void*)home_index);
    lv_tabview_set_act(tv, home_index, LV_ANIM_OFF);    

    // indicator必须为tv的最后一个child
    indicator = indicator_create(tv, frame_count, home_index);

    lv_obj_add_event_cb(tv, tab_changed_cb, LV_EVENT_VALUE_CHANGED, indicator);

    return desktop;
}

// frame_custom_height: frame的用户可用高度. 由于frame底部有页面指示器, 因此可用高度需要减掉指示器高度
uint32_t ui_desktop_get_frame_custom_height(lv_obj_t* obj)
{
    lv_obj_t* page_content = ui_page_get_content(obj);
    lv_tabview_t* tv = (lv_tabview_t*)lv_obj_get_child(page_content, 0);
    lv_obj_t* indicator = lv_obj_get_child(tv, -1);

    return lv_obj_get_y(indicator);
}

lv_obj_t* ui_desktop_get_frame(lv_obj_t* obj, uint32_t frame_index)
{
    lv_obj_t* page_content = ui_page_get_content(obj);
    lv_tabview_t* tv = (lv_tabview_t*)lv_obj_get_child(page_content, 0);
    lv_obj_t* content = lv_tabview_get_content(tv);
    uint32_t count = lv_obj_get_child_cnt(content);

    if (frame_index >= count) {
        frame_index = count - 1;
    }

    return lv_obj_get_child(content, frame_index);
}


uint32_t ui_desktop_get_frame_count(lv_obj_t* obj)
{
    lv_obj_t* page_content = ui_page_get_content(obj);
    lv_tabview_t* tv = (lv_tabview_t*)lv_obj_get_child(page_content, 0);
    lv_obj_t* content = lv_tabview_get_content(tv);
    uint32_t count = lv_obj_get_child_cnt(content);

    return count;
}

uint32_t ui_desktop_get_home_index(lv_obj_t* obj)
{
    return lv_obj_get_user_data(obj);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static lv_obj_t* indicator_create(lv_obj_t* parent, uint32_t count, uint32_t home_index)
{
    lv_obj_t* indicator = lv_obj_create(parent);
    const lv_coord_t circle_d = 8;      // 小圆点直径
    const lv_coord_t circle_pad = 15;   // 小圆点之间的间隔
    const lv_coord_t h_pad = 4;         // 上下间隔
    lv_coord_t indicator_w;
    lv_coord_t indicator_h;
    lv_coord_t max_w = lv_obj_get_width(lv_obj_get_parent(indicator));

    indicator_h = circle_d + h_pad * 2;
    indicator_w = (count + 1) * circle_pad + count * circle_d; // count + 1: 圆点之间间隔+左右间隔
    indicator_w = LV_MIN(indicator_w, max_w);


    lv_obj_remove_style_all(indicator);
    lv_obj_set_flex_flow(indicator, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(indicator, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(indicator, LV_OBJ_FLAG_FLOATING);

    lv_obj_set_style_bg_color(indicator, lv_color_white(), 0);
    lv_obj_set_style_pad_right(indicator, 0, 0);
    //lv_obj_set_style_bg_opa(indicator, LV_OPA_40, 0);
    lv_obj_set_style_bg_opa(indicator, LV_OPA_0, 0);
    lv_obj_set_style_radius(indicator, LV_RADIUS_CIRCLE, 0);

    lv_obj_set_size(indicator, LV_DPX(indicator_w), LV_DPX(indicator_h));

    lv_obj_align(indicator, LV_ALIGN_BOTTOM_MID, -LV_DPX(0), -LV_DPX(10));


    uint8_t i;
    for (i = 0; i < count; i++) {
        lv_obj_t* c = lv_btn_create(indicator);
        lv_obj_set_style_bg_color(c, lv_color_white(), 0);
        lv_obj_set_style_radius(c, LV_RADIUS_CIRCLE, 0);
        if (i == home_index) {
            lv_obj_set_style_opa(c, LV_OPA_COVER, 0);
        }
        else {
            lv_obj_set_style_opa(c, LV_OPA_60, 0);
        }
        
        lv_obj_set_size(c, LV_DPX(circle_d), LV_DPX(circle_d));
        lv_obj_clear_flag(c, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    }

    return indicator;
}

static void tab_changed_cb(lv_event_t* e)
{
    lv_obj_t* indicator = lv_event_get_user_data(e);
    lv_obj_t* tv = lv_event_get_target(e);    
    uint32_t count = lv_obj_get_child_cnt(indicator);    
    uint32_t index = lv_tabview_get_tab_act(tv);
    

    uint8_t i;
    for (i = 0; i < count; i++) {
        lv_obj_t* c = lv_obj_get_child(indicator, i);
        if (i == index) {
            lv_obj_set_style_opa(c, LV_OPA_COVER, 0);
        }
        else {
            lv_obj_set_style_opa(c, LV_OPA_50, 0);
        }
    }
}
