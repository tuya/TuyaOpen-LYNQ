/**
 * @file page.h
 *
 */

#ifndef __PAGE_H__
#define __PAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include "lvgl.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    lv_obj_t obj;
    lv_obj_t* main_content;
    lv_obj_t* title_bar;
} ui_page_t;

extern const lv_obj_class_t ui_page_class;

typedef enum {
    UI_PAGE_PART_PLACEHOLD,     // 与status bar重合的部分, 作为status bar的背景色
    UI_PAGE_PART_CONTENT,          // 主要内容部分, 用作其内容obj的parent
    UI_PAGE_PART_TITLE_BAR,     // 标题栏
    UI_PAGE_PART_RETURN_BTN,    // 返回按钮
    UI_PAGE_PART_TITLE,         // 标题栏上的标题label    
    UI_PAGE_PART_CLOSE_BTN,     // 关闭按钮
} ui_page_part_t;

typedef enum {
    UI_PAGE_RETURN_DISABLE,
    UI_PAGE_RETURN_ENABLE,
} ui_page_return_enable_t;

typedef enum {
    UI_PAGE_CLOSE_DISABLE,
    UI_PAGE_CLOSE_ENABLE,
} ui_page_close_enable_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a page object
 * @param title             标题,位于页面上方; 为""时, 不显示标题, 但是显示标题栏; 为NULL时不显示标题栏(位于标题栏中的返回和关闭按钮都不会显示)
 * @param enable_return_btn 是否显示返回按钮
 * @param enable_close_btn  是否显示关闭按钮
 * @return           pointer to the created page
 */
lv_obj_t* ui_page_create(const char* title, ui_page_return_enable_t enable_return_btn, ui_page_close_enable_t enable_close_btn);

/**
 * 获取page的各部件obj
 * @param obj       obj pointer to the page
 * @return          obj pointer to the part of the page
 */
lv_obj_t* ui_page_get_part(lv_obj_t* obj, ui_page_part_t part);

/**
 * 通过page中的子部件获取page
 * @param obj       obj pointer to the child of page
 * @return          obj pointer to the page
 */
lv_obj_t* ui_page_root(lv_obj_t* obj);

//void ui_page_set_style_title_bg_color(lv_color_t color, bool with_status_bar);

/**********************
 *      MACROS
 **********************/
#define ui_page_get_content(page)  ui_page_get_part(page, UI_PAGE_PART_CONTENT)


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*__PAGE_H__*/
