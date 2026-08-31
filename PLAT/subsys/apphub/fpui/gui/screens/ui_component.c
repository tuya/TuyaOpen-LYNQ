
#include "ui.h"

static bool _style_inited = false;
static lv_style_t style_line1 = {0};
static lv_style_t style_line2 = {0};
static lv_point_t scroll_line1[2] = {{0, 0},{0, 0}};
static lv_point_t scroll_line2[2] = {{0, 0},{0, 0}};
/**
  \fn
  \brief
  \return
*/
lv_obj_t *ui_scrollbar(lv_obj_t *base,uint8_t start,uint8_t total,uint8_t length)
{
    if(!_style_inited){
        lv_style_init(&style_line1);
        lv_style_set_line_width(&style_line1, 8);
        lv_style_set_line_color(&style_line1, lv_color_white());
        lv_style_set_line_rounded(&style_line1, true);
        lv_style_init(&style_line2);
        lv_style_set_line_width(&style_line2, 8);
        lv_style_set_line_color(&style_line2, lv_palette_main(LV_PALETTE_GREY));
        lv_style_set_line_rounded(&style_line2, true);
        _style_inited = true;
    }
    lv_obj_t *scroll = lv_line_create(base);
    lv_obj_set_width(scroll, 8);
    lv_obj_set_height(scroll, length+8);
    lv_obj_set_style_bg_opa(scroll, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT); 
    scroll_line2[1].y = length;
    lv_obj_t *scrollbar = lv_line_create(scroll);
    lv_line_set_points(scrollbar, scroll_line2, 2);
    lv_obj_add_style(scrollbar, &style_line2, 0);
    lv_obj_align(scrollbar, LV_ALIGN_RIGHT_MID, 4, 4);
    uint8_t _line_per = length/total;
    uint8_t _line_end = (start+MAX_LIST_ITEMS)*_line_per;
    if(_line_end > length || start+MAX_LIST_ITEMS>=total){
        _line_end = length;
    }
    scroll_line1[0].y = (start*_line_per);
    scroll_line1[1].y = _line_end;
    lv_obj_t *scrolldot = lv_line_create(scroll);
    lv_line_set_points(scrolldot, scroll_line1, 2);
    lv_obj_add_style(scrolldot, &style_line1, 0);
    lv_obj_align(scrolldot, LV_ALIGN_TOP_RIGHT, 4, 4);
    return scroll;
}
/**
  \fn
  \brief
  \return
*/
uint8_t ui_obj_clean(lv_obj_t *object)
{
    uint8_t num = 0;
    if(object != NULL && lv_obj_is_valid(object)){
        lv_obj_clean(object);
        lv_disp_t *disp = lv_obj_get_disp(object);
        if(disp->act_scr != object){
            #ifdef UI_TEST_ITEM
            // UI_LOG("clean 0x%X --- now 0x%X",object,lv_scr_act());
            #endif
            lv_obj_del(object);
            num += 1;
        }
    }
    return num;
}
/**
  \fn
  \brief
  \return
*/
static lv_obj_t * ui_sys_btn[3] = {NULL, NULL, NULL};
lv_obj_t **ui_btn_set(char (*bottomBtn)[8],lv_color_t text,lv_color_t bg)
{
    for (uint8_t i = 0; i < 3; i++) {
        if(ui_sys_btn[i] == NULL){
            ui_sys_btn[i] = lv_label_create(lv_layer_sys());
        }
        if(bottomBtn[i] != NULL && strlen(bottomBtn[i])>1 && strlen(bottomBtn[i])<8) {
            lv_obj_set_width(ui_sys_btn[i], LV_SIZE_CONTENT);
            lv_obj_set_height(ui_sys_btn[i], LV_SIZE_CONTENT);
            if(i==BOTTOM_BTN_LEFT) lv_obj_align(ui_sys_btn[i], LV_ALIGN_BOTTOM_LEFT, 0, 0); 
            else if(i==BOTTOM_BTN_CENTER) lv_obj_align(ui_sys_btn[i], LV_ALIGN_BOTTOM_MID, 0, 0); 
            else if(i==BOTTOM_BTN_RIGHT) lv_obj_align(ui_sys_btn[i], LV_ALIGN_BOTTOM_RIGHT, 0, 0); 
            lv_label_set_text(ui_sys_btn[i], bottomBtn[i]);
            // lv_obj_set_style_text_font(ui_sys_btn[i], pLight36, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(ui_sys_btn[i], bg, LV_PART_MAIN);
            lv_obj_set_style_text_color(ui_sys_btn[i], text, LV_PART_MAIN);
            lv_obj_clear_flag(ui_sys_btn[i], LV_OBJ_FLAG_HIDDEN);
        }
        else {
            lv_obj_add_flag(ui_sys_btn[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
    return ui_sys_btn;
}