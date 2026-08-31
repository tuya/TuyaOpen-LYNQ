
#include "../ui.h"


extern uint32_t millis(void);

static lv_obj_t *_ui_Call = NULL;
static lv_obj_t *ui_Call_Time = NULL;
static lv_timer_t *call_timer = NULL;

PLAT_FPSRAM_ZI_CUST char _ui_call_title[MAX_TITLE_LENGTH+1];
// PLAT_FPSRAM_ZI_CUST char _fpui_call_background[ADDR_LENGTH_MAX+1];

PLAT_FPSRAM_ZI_CUST PhoneUI_call_t _ui_call_data;

static uint8_t _call_labelIdx = 10; 
static uint32_t s_Call_Hangup_ms = 0;
static uint32_t s_Call_Time_ms = 0;
static ui_func_t ui_hangup_cb = NULL;
/**
  \fn
  \brief
  \return
*/
uint32_t ui_hangup_cb_register(ui_func_t cb)
{
    ui_hangup_cb = cb;

    return 0;
}
/**
  \fn
  \brief
  \return
*/
void call_timer_cb(lv_timer_t * tmr)
{
    if(s_Call_Time_ms == 0) {
        return;
    }
    if (_ui_Call == lv_scr_act() && _ui_Call != NULL) {
        uint32_t s_Call_Time = (millis() - s_Call_Time_ms)/1000;
        char time_str[9] = {0}; 
        sprintf(time_str,"%02d:%02d:%02d", s_Call_Time/3600,(s_Call_Time%3600)/60,(s_Call_Time%60));
        if(ui_Call_Time != NULL && lv_obj_is_valid(ui_Call_Time) ) {
            lv_label_set_text(ui_Call_Time, time_str);
        }
    }
    if(_call_labelIdx == UI_CALL_HANGUP){
        if(ui_hangup_cb != NULL){
            if(s_Call_Hangup_ms==0) {
                s_Call_Hangup_ms = millis();
                s_Call_Time_ms = 0;
            }
            else if(millis()-s_Call_Hangup_ms>1000)
            {
                ui_hangup_cb((void *)(millis()-s_Call_Hangup_ms));
                s_Call_Hangup_ms = 0;
            }
        }else if(call_timer  != NULL){
            lv_timer_pause(call_timer);
        } 
    }
}
/**
  \fn
  \brief
  \return
*/
void ui_call_screen(PhoneUI_call_t *data)
{
    if(lv_obj_is_valid(_ui_Call) && _ui_Call != NULL){
        lv_obj_clean(_ui_Call);
        lv_obj_del_async(_ui_Call);
        _ui_Call = NULL;
    }
    if(data == NULL) {
        goto labelEnd;
    }
    if(data->type > UI_CALL_HANGUP){
        UI_ERR("invalid type --- %d,%d",millis()/1000,data->type);
        goto labelEnd;
    }
    if(_call_labelIdx != data->type){
        _call_labelIdx = data->type;
    }
    // if(call_timer == NULL){
    //     call_timer = lv_timer_create(call_timer_cb, 1000, 0);
    //     lv_timer_pause(call_timer);
    // }
    _ui_Call = lv_obj_create(NULL);
    lv_obj_clear_flag(_ui_Call, LV_OBJ_FLAG_SCROLLABLE);
    // if(strlen(_fpui_call_background) && strlen(_fpui_call_background)< ADDR_LENGTH_MAX){
    //     lv_obj_set_style_bg_img_src(_ui_Call, _fpui_call_background, LV_PART_MAIN | LV_STATE_DEFAULT);
    // } 
    lv_obj_set_style_bg_color(_ui_Call, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(_ui_Call, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // UI_LOG("---%d,%d,%s,%d",_call_labelIdx,data->type,data->numstr,strlen(data->numstr));
    if(data->numstr != NULL && strlen(data->numstr)< MAX_PHONE_NUMBER){
        lv_obj_t * ui_Call_number = lv_label_create(_ui_Call);
        lv_obj_set_width(ui_Call_number, LV_PCT(100)); 
        lv_obj_set_height(ui_Call_number, 138);
        lv_obj_align(ui_Call_number, LV_ALIGN_TOP_LEFT, 4, 20); 
        lv_label_set_text(ui_Call_number, data->numstr);
        if(data->type == UI_CALL_INPUT){
            lv_obj_set_style_text_font(ui_Call_number, pSystem66, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        lv_obj_set_style_text_color(ui_Call_number, lv_color_make(0xff, 0xff, 0xff), LV_PART_MAIN | LV_STATE_DEFAULT );
    }

    switch (data->type) {
        case UI_CALL_INPUT:
            // if(strlen(_ui_call_title)<1){
            //     memset(_ui_call_title,0,sizeof(_ui_call_title));
            // }
            // if (call_timer) lv_timer_pause(call_timer);
            // s_Call_Time_ms = 0;
            break;
        case UI_CALL_DIAL:
            // if(strlen(_ui_call_title)<1){
            //     memset(_ui_call_title,0,sizeof(_ui_call_title));
            //     strcpy(_ui_call_title,"呼叫");
            // }
            // if (call_timer) lv_timer_resume(call_timer);
            // s_Call_Time_ms = millis();
            break;
        case UI_CALL_RING:
            // if(strlen(_ui_call_title)<1){
            //     memset(_ui_call_title,0,sizeof(_ui_call_title));
            //     strcpy(_ui_call_title,"来电");
            // }
            // if (call_timer) lv_timer_pause(call_timer);
            // s_Call_Time_ms = 0;
            break;
        case UI_CALL_CONNECT:
            // if(strlen(_ui_call_title)<1){
            //     memset(_ui_call_title,0,sizeof(_ui_call_title));
            //     strcpy(_ui_call_title,"通话中");
            // }
            // if (call_timer) lv_timer_resume(call_timer);
            // s_Call_Time_ms = millis();
            break;
        case UI_CALL_HANGUP:
            // if(strlen(_ui_call_title)<1){
            //     memset(_ui_call_title,0,sizeof(_ui_call_title));
            //     strcpy(_ui_call_title,"通话结束");
            // }
            break;
        default:
            break;
    }
    // if(s_Call_Time_ms){
    //     ui_Call_Time = lv_label_create(_ui_Call);
    //     lv_obj_set_width(ui_Call_Time, LV_SIZE_CONTENT); 
    //     lv_obj_set_height(ui_Call_Time, LV_SIZE_CONTENT);
    //     lv_obj_align(ui_Call_Time, LV_ALIGN_CENTER, 0, 100);     
    //     lv_label_set_text(ui_Call_Time,"00:00:00");
    //     lv_obj_set_style_text_color(ui_Call_Time, lv_color_make(0xff, 0xff, 0), LV_PART_MAIN | LV_STATE_DEFAULT );
    //     lv_obj_set_style_text_opa(ui_Call_Time, LV_OPA_COVER, LV_PART_MAIN| LV_STATE_DEFAULT);
    // }
    if(data->title != NULL && strlen(data->title)<MAX_TITLE_LENGTH){
        lv_obj_t *ui_Call_Label = lv_label_create(_ui_Call);
        lv_obj_set_width(ui_Call_Label, LV_SIZE_CONTENT);
        lv_obj_set_height(ui_Call_Label, LV_SIZE_CONTENT);
        lv_obj_align(ui_Call_Label, LV_ALIGN_CENTER, 0,56);
        lv_label_set_text(ui_Call_Label, data->title);
        lv_obj_set_style_text_color(ui_Call_Label, lv_color_make(0xff, 0xff, 0xff), LV_PART_MAIN | LV_STATE_DEFAULT );
        lv_obj_set_style_text_opa(ui_Call_Label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        // lv_obj_set_style_text_font(ui_Call_Label, pLight36, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    ui_btn_set(data->bottomBtn,lv_color_white(),lv_color_black());
    lv_disp_load_scr(_ui_Call);
labelEnd:
    osEventFlagsSet(uiEvtHandle, (1U << PAGE_CALL));
}
/**
  \fn
  \brief
  \return
*/
lv_obj_t *ui_call_set(PhoneUI_call_t *data)
{
    uint32_t mark = millis();
    if(data == NULL) {
        memset(&_ui_call_data,0,sizeof(PhoneUI_call_t));
        memset(_ui_call_title,0,sizeof(_ui_call_title));
        if(lv_obj_is_valid(_ui_Call)) lv_obj_add_flag(_ui_Call, LV_OBJ_FLAG_HIDDEN);
    }
    else if(strlen(data->title)>=MAX_TITLE_LENGTH || strlen(data->image)>=ADDR_LENGTH_MAX){
        UI_LOG("invalid data --- %d.%03d s",mark/1000,mark%1000);
        return NULL;
    }
    else {
        memset(_ui_call_title,0,sizeof(_ui_call_title));
        memset(&_ui_call_data,0,sizeof(PhoneUI_call_t));
        memcpy(&_ui_call_data,data,sizeof(PhoneUI_call_t));
        if(data->title != NULL){
            strcpy(_ui_call_title,data->title);
        }
        else if(data->type == UI_CALL_DIAL){
            strcpy(_ui_call_title,"呼叫");
        }
        else if(data->type == UI_CALL_RING){
            strcpy(_ui_call_title,"来电");
        }
        else if(data->type == UI_CALL_CONNECT){
            strcpy(_ui_call_title,"通话中");
        }
        else if(data->type == UI_CALL_HANGUP){
            strcpy(_ui_call_title,"通话结束");
        }
        _ui_call_data.title = _ui_call_title;
    }
    GuiMsgT msgPtr;
    msgPtr.refresh_ms = 10; 
    msgPtr.ui_set = (void* (*)(void *))ui_call_screen;
    msgPtr.ui_del = NULL;
    msgPtr.ui_data = &_ui_call_data;
    guiSendMsg(&msgPtr);
    osEventFlagsClear(uiEvtHandle, (1U << PAGE_CALL));
    osEventFlagsWait(uiEvtHandle, (1U << PAGE_CALL), osFlagsWaitAll, 300); 
    return _ui_Call;
}
/**
  \fn
  \brief
  \return
*/
lv_obj_t *ui_call_test(char *title,calltype_e type,char *numstr)
{
    static char call_btn[3][8] = {"选项","挂断","删除"};
    PhoneUI_call_t call_test = {
        .title = title,
        .type = type,
        .numstr = "",
        .bottomBtn = call_btn,
    };
    if(type>UI_CALL_HANGUP){
        return NULL;
    }
    if(numstr != NULL && strlen(numstr) >= MAX_PHONE_NUMBER){
        return NULL;
    }
    memset(call_test.numstr,0,sizeof(call_test.numstr));
    strcpy(call_test.numstr,numstr);
    #ifdef UI_TEST_ITEM
    UI_LOG("%ds,\ttype %d,%s",millis()/1000,type,numstr);
    #endif
    return ui_call_set(&call_test);
}