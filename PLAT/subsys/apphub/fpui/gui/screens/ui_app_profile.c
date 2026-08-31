
#include "ui.h"

#define PROFILE_LIST_ITEM_LIMIT     4

lv_obj_t * ui_App_Profile = NULL;
lv_obj_t * ui_App_Profile_title = NULL;
lv_obj_t * ui_App_Profile_items = NULL;
lv_obj_t * ui_App_Profile_item[PROFILE_LIST_ITEM_LIMIT] = {NULL};

static char *_profile_title = "情景模式";
void ui_App_Profilelist_screen(fpui_profilelist_t *data)
{
    if(lv_obj_is_valid(ui_App_Profile)) lv_obj_del_async(ui_App_Profile);
    if(data == NULL) return;
    // if(ui_App_Profile==NULL)
    {
        ui_App_Profile = lv_obj_create(NULL);
        lv_obj_clear_flag(ui_App_Profile, LV_OBJ_FLAG_SCROLLABLE);  
        lv_obj_set_style_bg_color(ui_App_Profile, lv_color_make(0, 0, 0), LV_PART_MAIN|LV_STATE_DEFAULT);

        if(data->title != NULL) {
            _profile_title = data->title;
        }
        ui_App_Profile_title = lv_label_create(ui_App_Profile);
        lv_obj_set_width(ui_App_Profile_title, LV_SIZE_CONTENT);  
        lv_obj_set_height(ui_App_Profile_title, LV_SIZE_CONTENT);   
        lv_label_set_text(ui_App_Profile_title, _profile_title);
        lv_obj_set_style_text_font(ui_App_Profile_title, &free36, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_align(ui_App_Profile_title, LV_ALIGN_TOP_MID, 0, 8);
        lv_obj_set_style_bg_color(ui_App_Profile_title, lv_color_make(0, 0, 0), LV_PART_MAIN);
        lv_obj_set_style_text_color(ui_App_Profile_title, lv_color_make(0xff, 0xff, 0xff), LV_PART_MAIN);

        ui_App_Profile_items = lv_list_create(ui_App_Profile);
        lv_obj_set_size(ui_App_Profile_items, LV_PCT(100),216);
        lv_obj_set_style_bg_color(ui_App_Profile_items, lv_color_make(0, 0, 0), LV_PART_MAIN);
        lv_obj_align_to(ui_App_Profile_items, ui_App_Profile_title, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);
        // lv_obj_set_style_border_width(ui_App_Profile_items, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
        // lv_obj_set_style_border_color(ui_App_Profile_items, lv_color_white(), LV_PART_MAIN);
    }

    uint8_t total = (data->total > PROFILE_LIST_ITEM_LIMIT) ? PROFILE_LIST_ITEM_LIMIT : data->total;
    uint32_t high = data->lines * 36;
    for (uint8_t index = 0; index < total; index++) {
        ui_App_Profile_item[index] = lv_list_create(ui_App_Profile_items);
        lv_obj_set_size(ui_App_Profile_item[index], LV_PCT(100), high);
        lv_obj_align(ui_App_Profile_item[index], LV_ALIGN_TOP_MID, 0, index * high);
        if(index==data->select) lv_obj_set_style_bg_color(ui_App_Profile_item[index], lv_color_make(0xff, 0, 0), LV_PART_MAIN | LV_STATE_DEFAULT);
        else lv_obj_set_style_bg_color(ui_App_Profile_item[index], lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
        for (uint8_t i = 0; i < data->lines; i++) 
        {
            lv_obj_t *item = lv_list_add_btn(ui_App_Profile_item[index], NULL, NULL);
            lv_obj_set_style_bg_opa(item, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
            if(data->items[index] != NULL){
                lv_obj_t *label = lv_label_create(item);
                lv_label_set_text(label, data->items[index * data->lines + i]);
                lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_font(label, &free36, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_bg_color(label, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT); 
            }
        }
    }
    ui_btn_set(data->bottomBtn,lv_color_white(),lv_color_black());

    lv_obj_clear_flag(ui_App_Profile, LV_OBJ_FLAG_HIDDEN);
    // if (lv_scr_act() != ui_App_Profile) {
    lv_disp_load_scr(ui_App_Profile);
    // }
}


lv_obj_t *ui_App_Profilelist_set(fpui_profilelist_t *data)
{
    if(data == NULL) {
        if (ui_App_Profile != NULL) {
            lv_obj_add_flag(ui_App_Profile, LV_OBJ_FLAG_HIDDEN);
        }
        return NULL;
    }
    GuiMsgT msgPtr;
    msgPtr.refresh_ms = 10; 
    msgPtr.ui_set = (void* (*)(void *))ui_App_Profilelist_screen;
    msgPtr.ui_del = NULL;
    msgPtr.ui_data = data;
    guiSendMsg(&msgPtr);
    return ui_App_Profile;
}

lv_obj_t * ui_App_Profile_Show = NULL;
lv_obj_t * ui_App_Profile_Text = NULL;
void ui_app_profile_screen(fpui_profileshow_t *data)
{
    if(lv_obj_is_valid(ui_App_Profile_Show)) lv_obj_del_async(ui_App_Profile_Show);
    if(data == NULL) return;
    ui_App_Profile_Show = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_App_Profile_Show, LV_OBJ_FLAG_SCROLLABLE);  
    lv_obj_set_style_bg_color(ui_App_Profile_Show, lv_color_black(), LV_PART_MAIN|LV_STATE_DEFAULT);

    lv_obj_t * ui_Title = lv_label_create(ui_App_Profile_Show);
    ui_App_Profile_Text = lv_label_create(ui_App_Profile_Show);

    if(data->title != NULL && strlen(data->title) > 1 && strlen(data->title) < 16) {
        lv_label_set_text(ui_Title, data->title);
        lv_obj_set_width(ui_Title, LV_SIZE_CONTENT);  
        lv_obj_set_height(ui_Title, LV_SIZE_CONTENT); 
        lv_obj_set_style_text_font(ui_Title, &free36, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_align(ui_Title, LV_ALIGN_TOP_MID, 0, 8);
        lv_obj_set_style_bg_color(ui_Title, lv_color_black(), LV_PART_MAIN);
        lv_obj_set_style_text_color(ui_Title, lv_color_white(), LV_PART_MAIN);
        lv_obj_align(ui_App_Profile_Text, LV_ALIGN_TOP_LEFT, 0, 48);
        lv_obj_set_size(ui_App_Profile_Text, LV_PCT(100), 216);
    }  
    else{
        // lv_obj_add_flag(ui_Title, LV_OBJ_FLAG_HIDDEN);
        lv_obj_align(ui_App_Profile_Text, LV_ALIGN_TOP_LEFT, 0, 8);
        lv_obj_set_size(ui_App_Profile_Text, LV_PCT(100), 252);
    } 

    if(data->message != NULL && strlen(data->message) > 1) {
        lv_label_set_long_mode(ui_App_Profile_Text, LV_LABEL_LONG_WRAP); 
        lv_label_set_text(ui_App_Profile_Text, data->message);
        lv_obj_set_style_text_font(ui_App_Profile_Text, &free36, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(ui_App_Profile_Text, lv_color_white(), LV_PART_MAIN);
        lv_obj_set_scrollbar_mode(ui_App_Profile_Text, LV_SCROLLBAR_MODE_AUTO);
        lv_obj_set_style_bg_color(ui_App_Profile_Text, lv_color_black(), LV_PART_MAIN);
        lv_obj_set_style_border_width(ui_App_Profile_Text, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(ui_App_Profile_Text, lv_color_make(0, 0xff, 0), LV_PART_MAIN);
    }
    ui_btn_set(data->bottomBtn,lv_color_white(),lv_color_black());
    lv_obj_clear_flag(ui_App_Profile_Show, LV_OBJ_FLAG_HIDDEN);
    // if (lv_scr_act() != ui_App_Profile_Show) 
    {
        lv_disp_load_scr(ui_App_Profile_Show);
    }
}


lv_obj_t *ui_app_profile_show(fpui_profileshow_t *data)
{
    GuiMsgT msgPtr;
    msgPtr.refresh_ms = 10; 
    msgPtr.ui_set = (void* (*)(void *))ui_app_profile_screen;
    msgPtr.ui_del = NULL;
    msgPtr.ui_data = data;
    guiSendMsg(&msgPtr);
    return ui_App_Profile;
}