
#include "ui.h"
#include "storage.h"

extern uint32_t millis(void);

static uint8_t file_index = 0;
static uint8_t file_select = 0;
static bool _need_update = true;
static lv_obj_t *_ui_file = NULL;
static lv_obj_t *_ui_file_list = NULL;
PLAT_FPSRAM_ZI_CUST fpui_file_t _ui_file_data;
PLAT_FPSRAM_ZI_CUST char _fpui_file_title[MAX_TITLE_LENGTH+1];
PLAT_FPSRAM_ZI_CUST char _fpui_file_folder[MAX_TITLE_LENGTH];
// PLAT_FPSRAM_ZI_CUST char _fpui_file_image[ADDR_LENGTH_MAX+1];
PLAT_FPSRAM_ZI_CUST char _fpui_file_items[MAX_LIST_ITEMS][LABEL_LENGTH_MAX+1];
static uint32_t _mes_file_ms = 0;
static uint32_t _mes_file_cnt = 0;
/**
  \fn
  \brief
  \return
*/
lv_obj_t *ui_file_screen(fpui_file_t *data)
{
    uint32_t mark = millis();
    if(data == NULL) {
        if(_ui_file != NULL){
            if(lv_obj_is_valid(_ui_file)){
                // lv_obj_clean(_ui_file);
                lv_obj_del_async(_ui_file);
            }
            _need_update = false;
            _ui_file = NULL;
        }
        _mes_file_ms = 0;
        _mes_file_cnt = 0;
        // goto labelEnd;
        return NULL;
    }
    uint8_t total = (data->total > MAX_LIST_ITEMS) ? MAX_LIST_ITEMS : data->total;
    if(data->select>=total || data->select>=MAX_LIST_ITEMS){
        UI_ERR("invalid select %d,%d",data->select,total);
        // osDelay(1000);
        return NULL;
    }
    if((_ui_file != NULL && data->object != _ui_file) || _need_update){
        if(lv_obj_is_valid(_ui_file)){
            lv_obj_del_async(_ui_file);
        }
        _need_update = false;
        _ui_file = NULL;
    }
    if(_ui_file == NULL){
        // UI_LOG("index %d.%d.%d",data->index,data->select,data->total);
        _ui_file = lv_obj_create(NULL);
        data->object = _ui_file;
        lv_obj_clear_flag(_ui_file, LV_OBJ_FLAG_SCROLLABLE); 
        lv_obj_set_style_bg_color(_ui_file, (lv_color_t)data->color, LV_PART_MAIN | LV_STATE_DEFAULT);
        if(data->title != NULL) {
            lv_obj_t *_ui_file_title = lv_label_create(_ui_file);
            lv_obj_set_width(_ui_file_title, LV_SIZE_CONTENT);
            lv_obj_set_height(_ui_file_title, LV_SIZE_CONTENT);
            lv_label_set_text(_ui_file_title, data->title);
            lv_obj_align(_ui_file_title, LV_ALIGN_TOP_MID, 0, 4);
            lv_obj_set_style_bg_color(_ui_file_title, (lv_color_t)data->color, LV_PART_MAIN);
            // lv_obj_set_style_text_font(_ui_file_title, pLight36, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(_ui_file_title, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        if(data->title != NULL) {
            lv_obj_t * list_folder = lv_label_create(_ui_file);
            lv_obj_set_width(list_folder, LV_SIZE_CONTENT);
            lv_obj_set_height(list_folder, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(list_folder, pSystem16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(list_folder, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(list_folder, data->folder);
            lv_obj_align(list_folder, LV_ALIGN_TOP_LEFT, 0, 36);
        }
        if(data->total>0) {
            if(_ui_file_list != NULL && lv_obj_is_valid(_ui_file_list)){
                lv_obj_del(_ui_file_list);
                _ui_file_list = NULL;
            }
            _ui_file_list = lv_list_create(_ui_file);
            lv_obj_set_size(_ui_file_list, LV_PCT(100), 216);
            lv_obj_align(_ui_file_list, LV_ALIGN_TOP_MID, 0, 54);
            lv_obj_remove_style(_ui_file_list, NULL, LV_PART_SCROLLBAR);
            lv_obj_set_style_bg_color(_ui_file_list, (lv_color_t)data->color, LV_PART_MAIN | LV_STATE_DEFAULT);
            for (uint8_t index = 0; index < total; index++) {
                if (data->context[index] != NULL && strlen(data->context[index]) < LABEL_LENGTH_MAX) {
                    lv_obj_t *_ui_file_item = lv_list_add_btn(_ui_file_list, NULL, NULL);
                    if(index == (data->select)){
                        file_select = index;
                        lv_obj_set_style_bg_color(_ui_file_item, lv_color_make(0xff, 0, 0), LV_PART_MAIN | LV_STATE_DEFAULT);
                    }
                    else lv_obj_set_style_bg_color(_ui_file_item, (lv_color_t)data->color, LV_PART_MAIN | LV_STATE_DEFAULT); 
                    lv_obj_t *_ui_file_text = lv_label_create(_ui_file_item);
                    lv_label_set_text(_ui_file_text, data->context[index]);
                    lv_obj_set_style_text_align(_ui_file_text, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
                    // lv_obj_set_style_text_font(_ui_file_text, pSystem16, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(_ui_file_text, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
            if (data->total > MAX_LIST_ITEMS) {
                lv_obj_t * list_number = lv_label_create(_ui_file);
                lv_obj_set_width(list_number, LV_SIZE_CONTENT);
                lv_obj_set_height(list_number, LV_SIZE_CONTENT);
                lv_obj_set_style_text_font(list_number, pSystem16, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_color(list_number, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_label_set_text_fmt(list_number, "%"LV_PRIu32"/%"LV_PRIu32, ((data->index)+(data->select)+1), data->total);
                lv_obj_align(list_number, LV_ALIGN_TOP_RIGHT, 0, 36); 
                lv_obj_t * scrollbar = ui_scrollbar(_ui_file,data->index,data->total,200);
                lv_obj_align(scrollbar, LV_ALIGN_TOP_RIGHT, 0, 60);
            }
        }
        if(data->bottomBtn){
            ui_btn_set(data->bottomBtn,lv_color_white(),lv_color_black());
        }
    }
    else if(lv_obj_is_valid(data->object)){
        if(_ui_file_list != NULL && lv_obj_is_valid(_ui_file_list)){
            lv_obj_t *_ui_file_item = lv_obj_get_child(_ui_file_list,(data->select));
            lv_obj_set_style_bg_color(_ui_file_item, lv_color_make(0xff, 0, 0), LV_PART_MAIN | LV_STATE_DEFAULT);
            if(file_select != (data->select)){
                _ui_file_item = lv_obj_get_child(_ui_file_list, file_select);
                lv_obj_set_style_bg_color(_ui_file_item, (lv_color_t)data->color, LV_PART_MAIN | LV_STATE_DEFAULT); 
            }
            file_select = (data->select);
        }
    }
    lv_disp_load_scr(_ui_file);
// labelEnd:
    mark = millis() - mark;
    _mes_file_ms += mark;
    _mes_file_cnt += 1;
    return _ui_file;
}
/**
  \fn
  \brief
  \return
*/
void *ui_file_set(fpui_file_t *data)
{
    GuiMsgT msgPtr;
    if(data == NULL){
        memset(&_ui_file_data,0,sizeof(fpui_file_t));
        msgPtr.ui_data = NULL;
    }
    else if(strlen(data->folder)>MAX_TITLE_LENGTH || strlen(data->title)>MAX_TITLE_LENGTH){
        UI_LOG("invalid data --- %d",millis());
        return NULL;
    }
    else if(data->select>=MAX_LIST_ITEMS){
        UI_LOG("invalid select %d",data->select);
        return NULL;
    }
    else {
        memset(&_ui_file_data,0,sizeof(fpui_file_t));
        memcpy(&_ui_file_data,data,sizeof(fpui_file_t));

        memset(_fpui_file_title,0,sizeof(_fpui_file_title));
        if(data->title != NULL){
            strcpy(_fpui_file_title,data->title);
        }
        _ui_file_data.title = _fpui_file_title;
        if(strlen(data->folder)>1){ //指定目录
            INT32 errCode;
            struct lfs_info *info = NULL;
            uint8_t total = 0;
            uint8_t index = 0;
            DIR *dir = opendir(data->folder);
            while(true){
                info = (struct lfs_info *)readdir(dir);
                if (info == NULL)
                {
                    break;
                }
                if (strlen(info->name) > 1 && info->size > 0) {
                    total += 1;
                    if (data->index < total && index < MAX_LIST_ITEMS) {
                        memset(_fpui_file_items[index],0,sizeof(_fpui_file_items[index]));
                        strcpy(_fpui_file_items[index],info->name);
                        _ui_file_data.context[index] = _fpui_file_items[index];
                        index += 1;
                    }
                }
                // UI_LOG("%d/%d,%d,%s",index,total,info->size,info->name);
            }
            errCode = closedir(dir);
            errCode = errCode;
            if(total < data->index || index < data->select){
                UI_ERR("invalid %d,%d=%d+%d",index,total,data->index,data->select);
                return NULL;
            }
            _ui_file_data.total = total;
        }
        else {  //根目录
            memset(_fpui_file_items[0],0,sizeof(_fpui_file_items[0]));
            strcpy(_fpui_file_items[0],"C:/");
            _ui_file_data.context[0] = _fpui_file_items[0];
            memset(_fpui_file_items[1],0,sizeof(_fpui_file_items[1]));
            strcpy(_fpui_file_items[1],"D:/");
            _ui_file_data.context[1] = _fpui_file_items[1];
            uint8_t select = (data->select > 2) ? 2 : data->select;
            _ui_file_data.select = select;
            _ui_file_data.total = 2;
            _ui_file_data.index = 0;
        }
        if (file_index != data->index){
            // UI_WRN("%d!=%d",file_index,data->index);
            file_index = data->index;
            _need_update = true;
        }
        if (strcasecmp(_fpui_file_folder, data->folder) != 0){
            // UI_WRN("%s!=%s",_fpui_file_folder,data->folder);
            _need_update = true;
        }
        memset(_fpui_file_folder,0,sizeof(_fpui_file_folder));
        if(data->folder != NULL){
            snprintf(_fpui_file_folder, MAX_TITLE_LENGTH-1, "%s",data->folder);
        }
        _ui_file_data.folder = _fpui_file_folder;
        msgPtr.ui_data = &_ui_file_data;
    }
    msgPtr.refresh_ms = 10;
    msgPtr.ui_set = (void* (*)(void *))ui_file_screen;
    msgPtr.ui_del = NULL;
    guiSendMsg(&msgPtr);
    osDelay(5);    //绘图时间 44(≤45)ms
    return msgPtr.ui_data;
}
/**
  \fn
  \brief
  \return
*/
void *ui_file_test(char *title,char *folder,uint8_t start,uint8_t select,uint16_t color)
{
    static char file_btn[3][8] = {"保存","测试","删除"};
    fpui_file_t file_data = {
        .object = NULL,
        .folder = folder,
        .title = title,
        .index = start,
        .select = select,
        .color = color,
        .bottomBtn = file_btn,
    };
    #ifdef UI_TEST_ITEM
    UI_LOG("%d/%d,%d-%d,0x%04X,%s,%s",_mes_file_ms,_mes_file_cnt,\
        file_data.index,file_data.select,file_data.color,file_data.title,file_data.folder);
    #endif
    return ui_file_set(&file_data);
}