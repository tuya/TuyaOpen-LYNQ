
#include "ui.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif

lv_obj_t * _ui_list_sms = NULL;
lv_obj_t * _ui_list_sms_item[2];
PLAT_FPSRAM_ZI_CUST char fpui_sms_title[MAX_TITLE_LENGTH+1];

void ui_list_sms_screen(fpui_sms_t *data)
{
    if(lv_obj_is_valid(_ui_list_sms)) lv_obj_del_async(_ui_list_sms);
    if(data == NULL) return NULL;
    _ui_list_sms = lv_obj_create(NULL);
    lv_obj_clear_flag(_ui_list_sms, LV_OBJ_FLAG_SCROLLABLE);  
    lv_obj_set_style_bg_color(_ui_list_sms, lv_color_make(0, 0, 0), LV_PART_MAIN|LV_STATE_DEFAULT);

    lv_obj_t * _ui_list_sms_title = lv_label_create(_ui_list_sms);
    lv_obj_set_width(_ui_list_sms_title, LV_SIZE_CONTENT);  
    lv_obj_set_height(_ui_list_sms_title, LV_SIZE_CONTENT);   
    lv_label_set_text(_ui_list_sms_title, fpui_sms_title);
    lv_obj_set_style_text_font(_ui_list_sms_title, &free36, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(_ui_list_sms_title, LV_ALIGN_TOP_MID, 0, 8);
    lv_obj_set_style_bg_color(_ui_list_sms_title, lv_color_make(0, 0, 0), LV_PART_MAIN);
    lv_obj_set_style_text_color(_ui_list_sms_title, lv_color_make(0xff, 0xff, 0xff), LV_PART_MAIN);

    lv_obj_t * _ui_list_sms_items = lv_list_create(_ui_list_sms);
    lv_obj_set_size(_ui_list_sms_items, LV_PCT(100),220);
    lv_obj_set_style_bg_color(_ui_list_sms_items, lv_color_make(0, 0, 0), LV_PART_MAIN);
    lv_obj_align_to(_ui_list_sms_items, _ui_list_sms_title, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);

    uint32_t high = data->lines * 37;
    for (uint8_t index = 0; index < data->total; index++) {
        _ui_list_sms_item[index] = lv_list_create(_ui_list_sms_items);
        lv_obj_set_size(_ui_list_sms_item[index], LV_PCT(100), high);
        lv_obj_align(_ui_list_sms_item[index], LV_ALIGN_TOP_MID, 0, index * high);
        if(index==data->select) lv_obj_set_style_bg_color(_ui_list_sms_item[index], lv_color_make(0xff, 0, 0), LV_PART_MAIN | LV_STATE_DEFAULT);
        else lv_obj_set_style_bg_color(_ui_list_sms_item[index], lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
        for (uint8_t i = 0; i < data->lines; i++) 
        {
            lv_obj_t *item = lv_list_add_btn(_ui_list_sms_item[index], NULL, NULL);
            lv_obj_set_style_bg_opa(item, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
            if(data->context != NULL){
                lv_obj_t *label = lv_label_create(item);
                lv_label_set_text(label, &data->context[index * (data->lines * 40) + i * 40]);
                lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
                if(i == 1) lv_obj_set_style_text_font(label, &free36, LV_PART_MAIN | LV_STATE_DEFAULT);
                else lv_obj_set_style_text_font(label, &free36, LV_PART_MAIN | LV_STATE_DEFAULT);
                // lv_obj_set_style_bg_opa(label, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_bg_color(label, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT); 
            }
        }
    }
    ui_btn_set(data->bottomBtn,lv_color_white(),lv_color_black());
    lv_disp_load_scr(_ui_list_sms);
}

lv_obj_t *ui_sms_set(fpui_sms_t *data)
{
    if(data != NULL) {
        if(data->title !=NULL && strlen(data->title)<MAX_TITLE_LENGTH){
            memset(fpui_sms_title,0,sizeof(fpui_sms_title));
            strcpy(fpui_sms_title,data->title);
        }
        else if(_ui_list_sms == NULL){
            memset(fpui_sms_title,0,sizeof(fpui_sms_title));
        }  
    }
    GuiMsgT msgPtr;
    msgPtr.refresh_ms = 10; 
    msgPtr.ui_set = ui_list_sms_screen;
    msgPtr.ui_del = NULL;
    msgPtr.ui_data = data;
    guiSendMsg(&msgPtr);
    return _ui_list_sms;
}
