
#include "ui.h"

extern uint32_t millis(void);

static lv_obj_t *_ui_recorder = NULL;
static lv_obj_t *_ui_recorder_time = NULL;
static lv_obj_t *_ui_recorder_label = NULL;
// static lv_obj_t *_ui_bk_image = NULL;
static lv_timer_t *_recorder_timer = NULL;
static bool _isInited = false;
static bool _need_update = true;
PLAT_FPSRAM_ZI_CUST fpui_recorder_t _ui_recorder_data;
PLAT_FPSRAM_ZI_CUST char _ui_recorder_title[MAX_TITLE_LENGTH+1];
PLAT_FPSRAM_ZI_CUST char _fpui_recorder_btn[3][8];
static uint32_t _recorder_time_ms = 0;
#ifdef UI_TEST_ITEM
static uint32_t ui_free_size = 0;
static uint32_t _mes_recorder_ms = 0;
static uint32_t _mes_recorder_cnt = 0;
 #endif
/**
  \fn
  \brief
  \return
*/
static void recorder_timer_cb(lv_timer_t *tmr)
{
    if(_recorder_time_ms){
        uint16_t _time_ms = (millis() - _recorder_time_ms)/1000;
        if(_ui_recorder_time != NULL && lv_obj_is_valid(_ui_recorder_time)) {
            lv_label_set_text_fmt(_ui_recorder_time, "%02d:%02d:%02d",_time_ms/3600,(_time_ms%3600)/60,(_time_ms%60));
        }
    }
}
/**
  \fn
  \brief +1032 B
  \return
*/
lv_obj_t *ui_recorder_screen(fpui_recorder_t *data)
{
#ifdef UI_TEST_ITEM
    uint32_t mark = millis();
#endif
    if(data == NULL) {
        if(lv_obj_is_valid(_ui_recorder)){
            lv_obj_clean(_ui_recorder);
            lv_disp_t *disp = lv_obj_get_disp(_ui_recorder);
            if(disp->act_scr != _ui_recorder){
                // UI_WRN("delete 0x%X",_ui_recorder);
                lv_obj_del_async(_ui_recorder);
            }
        }
        _ui_recorder_label = NULL;
        _ui_recorder_time = NULL;
        _ui_recorder = NULL;
        #ifdef UI_TEST_ITEM
        _mes_recorder_ms = 0;
        _mes_recorder_cnt = 0;
         #endif
        return NULL;
    }
    if(lv_obj_is_valid(data->object) && data->object == _ui_recorder && !_need_update){
        // UI_LOG("no need create 0x%X,%d,%s",data->object,_need_update,data->title);
        _need_update = false;
    }
    else if(_ui_recorder != NULL){
        // if(data->title != NULL)  UI_LOG("0x%X,%d,%s",_ui_recorder,_need_update,data->title);
        // else UI_LOG("0x%X\t%d",_ui_recorder,_need_update);
        if(lv_obj_is_valid(_ui_recorder)){
            lv_obj_del_async(_ui_recorder);
        }
        _ui_recorder_label = NULL;
        _ui_recorder_time = NULL;
        _ui_recorder = NULL;
    }
    if(_recorder_timer == NULL){
        _recorder_timer = lv_timer_create(recorder_timer_cb, 1000, 0);
        lv_timer_pause(_recorder_timer);
    }
    if(!_isInited){
        _isInited = true;
    }
    if(_ui_recorder == NULL){
        _ui_recorder = lv_obj_create(NULL);
        data->object = _ui_recorder;
        lv_obj_clear_flag(_ui_recorder, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_opa(_ui_recorder, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(_ui_recorder, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN | LV_STATE_DEFAULT );

        lv_obj_t *_ui_head = lv_obj_create(_ui_recorder);   //400KB
        lv_obj_set_width(_ui_head, LV_PCT(100));
        lv_obj_set_height(_ui_head, 40);
        lv_obj_align(_ui_head, LV_ALIGN_TOP_LEFT, 0, 0);
        lv_obj_set_style_bg_opa(_ui_head, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
         lv_obj_set_style_bg_color(_ui_head, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_t *_ui_title = lv_label_create(_ui_head);
        lv_obj_set_width(_ui_title, LV_SIZE_CONTENT);
        lv_obj_set_height(_ui_title, LV_SIZE_CONTENT);
        lv_label_set_text(_ui_title, "录音机");
        lv_obj_align(_ui_title, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_set_style_bg_opa(_ui_title, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(_ui_title, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(_ui_title, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);

        _ui_recorder_label = lv_label_create(_ui_recorder);
        lv_obj_set_size(_ui_recorder_label, LV_PCT(100), 74);
        lv_obj_set_pos(_ui_recorder_label, 0, 48);
        lv_label_set_text(_ui_recorder_label," ");
        lv_obj_set_align(_ui_recorder_label, LV_ALIGN_TOP_MID);
        lv_obj_set_style_text_color(_ui_recorder_label, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT );
        lv_obj_set_style_text_opa(_ui_recorder_label, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);

        _ui_recorder_time = lv_label_create(_ui_recorder);
        lv_obj_set_width(_ui_recorder_time, LV_SIZE_CONTENT);
        lv_obj_set_height(_ui_recorder_time, LV_SIZE_CONTENT);
        lv_obj_align(_ui_recorder_time, LV_ALIGN_TOP_MID, 0, 112);
        lv_label_set_text(_ui_recorder_time,"00:00:00");
        lv_obj_set_style_text_font(_ui_recorder_time, pSystem16, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(_ui_recorder_time, lv_color_make(0xff, 0, 0), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_opa(_ui_recorder_time, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
        if(data->bottomBtn){
            ui_btn_set(data->bottomBtn,lv_color_white(),lv_color_black());
        }
    }
    if(data->title != NULL && strlen(data->title) > 1){
        if(_ui_recorder_label != NULL && lv_obj_is_valid(_ui_recorder_label)) {
            lv_label_set_text(_ui_recorder_label, data->title);
            lv_obj_clear_flag(_ui_recorder_label, LV_OBJ_FLAG_HIDDEN);
        }
        if(_recorder_timer) lv_timer_resume(_recorder_timer);
        _recorder_time_ms = millis();
    }
    else{
        // lv_obj_add_flag(_ui_recorder_label, LV_OBJ_FLAG_HIDDEN);
        if(_recorder_timer) lv_timer_pause(_recorder_timer);
        _recorder_time_ms = 0;
    }
    lv_disp_load_scr(_ui_recorder);
// labelEnd:
#ifdef UI_TEST_ITEM
    mark = millis() - mark;
    _mes_recorder_ms += mark;
    _mes_recorder_cnt += 1;
    lv_mem_monitor_t mem_mon;
    lv_mem_monitor(&mem_mon);
    ui_free_size = ui_free_size - mem_mon.free_size;
    // UI_LOG("%d-%d = %d",ui_free_size,mem_mon.free_size,(ui_free_size-mem_mon.free_size));
 #endif
    return _ui_recorder;
}
/**
  \fn
  \brief
  \return
*/
fpui_recorder_t *ui_recorder_get(void)
{
    if(!_isInited){
        memset(_ui_recorder_title,0,sizeof(_ui_recorder_title));
        memset(_fpui_recorder_btn,0,sizeof(_fpui_recorder_btn));
        memset(&_ui_recorder_data,0,sizeof(fpui_recorder_t));
        _ui_recorder_data.bottomBtn = _fpui_recorder_btn;
        _ui_recorder_data.title = _ui_recorder_title;
        _isInited = true;
    }
    return &_ui_recorder_data;
}
/**
  \fn
  \brief
  \return
*/
void *ui_recorder_set(fpui_recorder_t *data)
{
    GuiMsgT msgPtr;
    if(data == NULL){
        // memset(_ui_recorder_title,0,sizeof(_ui_recorder_title));
        // memset(&_ui_recorder_data,0,sizeof(fpui_recorder_t));
        msgPtr.ui_data = NULL;
    }
    else if(strlen(data->title) > MAX_TITLE_LENGTH){
        UI_LOG("invalid data --- %d s",millis()/1000);
        return NULL;
    }
    else if(data == &_ui_recorder_data){
        msgPtr.ui_data = &_ui_recorder_data;
    }
    else {
        // UI_LOG("%d,%s",strlen(data->title),data->title);
        if (data->title != NULL && strcasecmp(_ui_recorder_title, data->title)){
            memset(_ui_recorder_title,0,sizeof(_ui_recorder_title));
            strcpy(_ui_recorder_title,data->title);
            _need_update = true;
        }
        else if(_ui_recorder == data->object && _ui_recorder != NULL){
            _need_update = false;
        }
        if (data->title == NULL){
            memset(_ui_recorder_title,0,sizeof(_ui_recorder_title));
        }
        memset(&_ui_recorder_data,0,sizeof(fpui_recorder_t));
        memcpy(&_ui_recorder_data,data,sizeof(fpui_recorder_t));
        memset(_fpui_recorder_btn,0,sizeof(_fpui_recorder_btn));
        if(data->bottomBtn != NULL && strlen(data->bottomBtn[0])<=8){
            memcpy(_fpui_recorder_btn,data->bottomBtn,sizeof(_fpui_recorder_btn));
        }
        _ui_recorder_data.bottomBtn = _fpui_recorder_btn;
        _ui_recorder_data.title = _ui_recorder_title;
        msgPtr.ui_data = &_ui_recorder_data;
    }
    msgPtr.refresh_ms = 10;
    msgPtr.ui_set = (void* (*)(void *))ui_recorder_screen;
    msgPtr.ui_del = NULL;
    guiSendMsg(&msgPtr);
    osDelay(5); //绘图时间 13(≤23)ms
    return msgPtr.ui_data;
}
/**
  \fn
  \brief
  \return
*/
// void *ui_recorder_test(char *title, uint16_t color)
// {
//     static char app_btn[3][8] = {"文件","录音","返回"};
//     fpui_recorder_t recorder_data = {
//         .object = NULL,
//         .title = NULL,
//         .color = color,
//         .bottomBtn = app_btn,
//     };
//     if(title != NULL && strlen(title) < MAX_TITLE_LENGTH){
//         recorder_data.title = title;
//     }
//     else recorder_data.title = "";
//     #ifdef UI_TEST_ITEM
//     lv_mem_monitor_t mem_monitor;
//     lv_mem_monitor(&mem_monitor);
//     UI_LOG("%d/%d,%d,0x%X\t%d,%s",_mes_recorder_ms,_mes_recorder_cnt,ui_free_size,color,strlen(recorder_data.title),recorder_data.title);
//     ui_free_size = mem_monitor.free_size;   //1020B
//     #endif
//     return ui_recorder_set(&recorder_data);
// }
void *ui_recorder_test(char *title, uint16_t color)
{
    bool inited = _isInited;
    fpui_recorder_t *recorder_ptr = ui_recorder_get();
    if(title != NULL && strlen(title) < MAX_TITLE_LENGTH){
        memset(recorder_ptr->title,0,strlen(recorder_ptr->title));
        strcpy(recorder_ptr->title,title);
    }
    else{
        memset(recorder_ptr->title,0,strlen(recorder_ptr->title));
        if(inited) recorder_ptr->object = _ui_recorder;
    }
    if(!inited){
        static char app_btn[3][8] = {"文件","录音","返回"};
        memcpy(recorder_ptr->bottomBtn,app_btn,sizeof(app_btn));
    }
    recorder_ptr->color = color;
    #ifdef UI_TEST_ITEM
    lv_mem_monitor_t mem_monitor;
    lv_mem_monitor(&mem_monitor);
    UI_LOG("%d/%d,%d,0x%04X\t%d,%s",_mes_recorder_ms,_mes_recorder_cnt,ui_free_size,\
        recorder_ptr->color,strlen(recorder_ptr->title),recorder_ptr->title);
    ui_free_size = mem_monitor.free_size;   //1020B
    #endif
    return ui_recorder_set(recorder_ptr);
}