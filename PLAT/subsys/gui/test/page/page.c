/**
 * @file page.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "../ui_conf.h"
#include "page.h"


/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &ui_page_class

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void ui_page_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t ui_page_class  = {
    .constructor_cb = ui_page_constructor,
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(ui_page_t),
    .base_class = &lv_obj_class,
};

typedef struct {
    const char* title;
    ui_page_return_enable_t enable_return_btn;
    ui_page_close_enable_t enable_close_btn;
} ui_page_create_info_t;

// only used in lv_obj_class_create_obj, no affect multiple instances
static ui_page_create_info_t _create_info;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
// TODO: enable_return_btn/enable_close_btn
lv_obj_t * ui_page_create(const char* title, ui_page_return_enable_t enable_return_btn, ui_page_close_enable_t enable_close_btn)
{
    LV_LOG_INFO("begin");
    _create_info.title = title;
    _create_info.enable_return_btn = enable_return_btn;
    _create_info.enable_close_btn = enable_close_btn;

    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, NULL);
    lv_obj_class_init_obj(obj);
    return obj;
}

lv_obj_t* ui_page_get_part(lv_obj_t* obj, ui_page_part_t part)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    ui_page_t* page = (ui_page_t*)obj;

    switch (part) {
    case UI_PAGE_PART_PLACEHOLD:
        return lv_obj_get_child(obj, 0);

    case UI_PAGE_PART_CONTENT:
        return page->main_content;

    case UI_PAGE_PART_TITLE_BAR:
        return page->title_bar;

    case UI_PAGE_PART_RETURN_BTN:
        if (page->title_bar != NULL) {
            return lv_obj_get_child(page->title_bar, 0);
        }
        break;

    case UI_PAGE_PART_TITLE:
        if (page->title_bar != NULL) {
            return lv_obj_get_child(page->title_bar, 1);
        }
        break;

    case UI_PAGE_PART_CLOSE_BTN:
        if (page->title_bar != NULL) {
            return lv_obj_get_child(page->title_bar, 2);
        }
        break;
    }

    return NULL;
}

lv_obj_t* ui_page_root(lv_obj_t* obj)
{
    LV_ASSERT_NULL(obj);

    return lv_obj_get_screen(obj);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
void my_event_cb(lv_event_t* e)
{
    lv_obj_t* target = lv_event_get_target(e);

    LV_LOG("%s\n", lv_obj_get_class(ui_page_root(target))->user_data);
}

static void ui_page_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    ui_page_t* page = (ui_page_t*)obj;
    lv_obj_t* placehold;
    lv_obj_t* container;
    lv_obj_t* title_bar;
    lv_obj_t* title_label;
    lv_obj_t* return_btn;
    lv_obj_t* close_btn;
    lv_obj_t* main_content;

    lv_obj_set_style_bg_color(obj, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    placehold = lv_obj_create(obj); 
    container = lv_obj_create(obj);
    page->main_content = container;

    // placehold
    lv_obj_clear_flag(placehold, LV_OBJ_FLAG_SCROLLABLE);    
    lv_obj_set_style_radius(placehold, 0, 0);
    lv_obj_set_style_border_width(placehold, 0, 0);
    lv_obj_set_size(placehold, LV_PCT(100), UI_STATUS_BAR_HEIGHT);
    
    // container
    lv_obj_align_to(container, placehold, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);    
    lv_obj_set_style_radius(container, 0, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, 0, 0);
    lv_obj_set_size(container, LV_PCT(100), lv_obj_get_height(obj) - UI_STATUS_BAR_HEIGHT);

    if (_create_info.title == NULL) {        
        return;
    }
    

    title_bar = lv_obj_create(container);
    main_content = lv_obj_create(container);

    // 以下三个顺序不能乱
    return_btn = lv_btn_create(title_bar);
    title_label = lv_label_create(title_bar);
    close_btn = lv_btn_create(title_bar);
    
    page->title_bar = title_bar;
    page->main_content = main_content;

    // title_bar
    lv_obj_clear_flag(title_bar, LV_OBJ_FLAG_SCROLLABLE);    
    lv_obj_set_style_radius(title_bar, 0, 0);
    lv_obj_set_style_border_width(title_bar, 0, 0);
    lv_obj_set_style_pad_hor(title_bar, 10, 0);
    lv_obj_set_style_bg_opa(title_bar, LV_OPA_100, 0); // 0 全透, 100 不透.不设置透明度, lv_obj_set_style_bg_color设置label的背景色似乎无效
    lv_obj_set_size(title_bar, LV_PCT(100), UI_PAGE_TITLE_HEIGHT);

    // return btn
    lv_obj_t* return_text = lv_label_create(return_btn);
    lv_label_set_text(return_text, "<-");
    lv_obj_align(return_text, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_text_color(return_text, lv_color_white(), 0);

    lv_obj_align(return_btn, LV_ALIGN_LEFT_MID, 0, 0);    
    lv_obj_set_style_pad_hor(return_btn, 0, 0);
    lv_obj_set_style_radius(return_btn, 0, 0);
    lv_obj_set_style_border_width(return_btn, 0, 0);
    lv_obj_set_style_border_opa(return_btn, LV_OPA_0, 0);
    lv_obj_set_style_bg_opa(return_btn, LV_OPA_0, 0);
    lv_obj_set_size(return_btn, 32, lv_obj_get_height(title_bar));

    lv_obj_add_event_cb(return_btn, my_event_cb, LV_EVENT_CLICKED, NULL);

    // title label
    lv_obj_center(title_label);    
    lv_label_set_text(title_label, _create_info.title);    
    lv_obj_set_style_radius(title_label, 0, 0);
    lv_obj_set_style_border_width(title_label, 0, 0);
    lv_obj_set_style_text_color(title_label, lv_color_white(), 0);
    lv_obj_set_style_text_align(title_label, LV_TEXT_ALIGN_CENTER, 0);
    //lv_obj_set_style_bg_opa(title_label, LV_OPA_0, 0);
    //lv_obj_set_size(title_label, LV_PCT(100), UI_PAGE_TITLE_HEIGHT);


    // close btn
    lv_obj_t* close_text = lv_label_create(close_btn);
    lv_label_set_text(close_text, "X");
    lv_obj_align(close_text, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_text_color(close_text, lv_color_white(), 0);

    lv_obj_align(close_btn, LV_ALIGN_RIGHT_MID, 0, 0);    
    lv_obj_set_style_pad_hor(close_btn, 0, 0);
    lv_obj_set_style_radius(close_btn, 0, 0);    
    lv_obj_set_style_border_width(close_btn, 0, 0);
    lv_obj_set_style_border_opa(close_btn, LV_OPA_0, 0);
    lv_obj_set_style_bg_opa(close_btn, LV_OPA_0, 0);
    lv_obj_set_size(close_btn, 32, lv_obj_get_height(title_bar));

    lv_obj_add_event_cb(close_btn, my_event_cb, LV_EVENT_CLICKED, NULL);
    
    // main_content
    lv_obj_align_to(main_content, title_bar, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_obj_clear_flag(main_content, LV_OBJ_FLAG_SCROLLABLE);    
    lv_obj_set_style_radius(main_content, 0, 0);
    lv_obj_set_style_border_width(main_content, 0, 0);
    lv_obj_set_style_pad_all(main_content, 0, 0);
    lv_obj_set_size(main_content, LV_PCT(100), lv_obj_get_height(container) - lv_obj_get_height(title_bar));

    LV_TRACE_OBJ_CREATE("finished");
}


