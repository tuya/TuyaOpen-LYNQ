
#include "ui.h"
#include "app_hal.h"
#include "ui_helpers.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_SUBSYS_STATUS_ENABLE
#include "status.h"
#endif
#include DEBUG_LOG_HEADER_FILE
#define EPAT_LOG(subId, debugLevel, format, ...)  \
    ECPLAT_PRINTF(UNILOG_LVGL, subId, debugLevel, format, ##__VA_ARGS__)

void ui_event_brightness_slider_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_REFR_EXT_DRAW_SIZE) {
        lv_coord_t * s = lv_event_get_param(e);
        *s = LV_MAX(*s, 60);
    }
    else if(code == LV_EVENT_DRAW_PART_END) {
        lv_obj_draw_part_dsc_t * dsc = lv_event_get_param(e);
        if(dsc->part == LV_PART_KNOB && lv_obj_has_state(obj, LV_STATE_PRESSED)) {
            onBrightnessChange(e);
        }
    }
}
/**
  \fn
  \brief
  \return
*/
void ui_event_button_volumeUp(lv_event_t *e)
{
    lv_disp_t *display = lv_disp_get_default();
    lv_obj_t *actScr = lv_disp_get_scr_act(display);
    if (actScr != ui_controlScreen)
    {
        return;
    }
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        onVolumeUp();
    }
}
/**
  \fn
  \brief
  \return
*/
void ui_event_button_volumeDown(lv_event_t *e)
{
    lv_disp_t *display = lv_disp_get_default();
    lv_obj_t *actScr = lv_disp_get_scr_act(display);
    if (actScr != ui_controlScreen)
    {
        return;
    }
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        onVolumeDown(e);
        // watch_lable.camera[0] = "点击开始预览";//"Click capture to close to close"
        // lv_label_set_text(ui_cameraLabel, watch_lable.camera[0]);
        // lv_scr_load_anim(ui_cameraScreen, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
    }
}
/**
  \fn
  \brief
  \return
*/
void ui_event_messageClick(lv_event_t *e)
{
    lv_disp_t *display = lv_disp_get_default();
    lv_obj_t *actScr = lv_disp_get_scr_act(display);
    if (actScr != ui_notificationScreen)
    {
        return;
    }
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        onMessageClick();
        lv_obj_scroll_to_y(ui_messagePanel, 0, LV_ANIM_ON);
        lv_obj_add_flag(ui_messageList, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_messagePanel, LV_OBJ_FLAG_HIDDEN);
    }
}
/**
  \fn
  \brief
  \return
*/
void ui_event_button_reserve(lv_event_t *e)
{
    lv_disp_t *display = lv_disp_get_default();
    lv_obj_t *actScr = lv_disp_get_scr_act(display);
    if (actScr != ui_controlScreen)
    {
        return;
    }
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_VALUE_CHANGED && !lv_obj_has_state(target, LV_STATE_CHECKED))
    {
        _ui_flag_modify(ui_searchPanel, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_ADD);
    }
    if (event_code == LV_EVENT_VALUE_CHANGED && lv_obj_has_state(target, LV_STATE_CHECKED))
    {
        _ui_flag_modify(ui_searchPanel, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_REMOVE);
    }
}
/**
  \fn
  \brief
  \return
*/
uint8_t phone_text_index=0;
extern ui_label_t call_label;
const char *btn_dial_list[20] = {
    "1", "2", "3", "\n",
    "4", "5", "6", "\n",
    "7", "8", "9", "\n",
    "*", "0", "#", "\n",
    LV_SYMBOL_CALL, LV_SYMBOL_STOP, LV_SYMBOL_NEXT
};

static uint8_t dialout_number_add(uint8_t code)
{
    if((code>='0' && code<='9') || code =='*' || code=='#')
    {
        call_label.numstr[phone_text_index] = code;
        call_label.numstr[phone_text_index+1] ='\0';
        phone_text_index++;
        if(phone_text_index>=sizeof(call_label.numstr)) phone_text_index = 0;
        if(phone_text_index<15){
            lv_label_set_text(ui_dialpadLabel, call_label.numstr);
        }
        else{
            lv_label_set_text(ui_dialpadLabel, call_label.numstr+(phone_text_index-14));
        }
    }
    return phone_text_index;
}
/**
  \fn
  \brief
  \return
*/
static uint8_t dialout_number_del(uint8_t code)
{
    if(code < phone_text_index){
        phone_text_index-=code;
        call_label.numstr[phone_text_index] ='\0';
    }
    else {
        memset(call_label.numstr,0,sizeof(call_label.numstr));
        phone_text_index=0;
    }
    lv_label_set_text(ui_dialpadLabel, call_label.numstr);
    return phone_text_index;
}
/**
  \fn
  \brief
  \return
*/
lv_obj_t * ui_Label_Sim = NULL;
void ui_event_btnmatrix_dialpad(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    uint16_t id = lv_btnmatrix_get_selected_btn(obj);
    char* txt = lv_btnmatrix_get_btn_text(obj,id);
    if(code == LV_EVENT_CLICKED)
    {
        if(id >=0 && id<12)
        {
            dialout_number_add(txt[0]);
        }
        else if(id == 12) //dialout
        {
            if (simGetStatus() == SIM_REMOVED){
                if(ui_Label_Sim == NULL){
                    ui_Label_Sim = lv_label_create(lv_scr_act());
                    lv_obj_set_width(ui_Label_Sim, LV_SIZE_CONTENT);   
                    lv_obj_set_height(ui_Label_Sim, LV_SIZE_CONTENT);
                    lv_label_set_text(ui_Label_Sim, "请插入SIM卡");
                    lv_obj_set_style_text_color(ui_Label_Sim, lv_color_make(0xff, 0, 0), LV_PART_MAIN | LV_STATE_DEFAULT); 
                    lv_obj_align(ui_Label_Sim, LV_ALIGN_TOP_MID, 0, 8);
                }
                lv_obj_clear_flag(ui_Label_Sim, LV_OBJ_FLAG_HIDDEN);
            }
            else if(phone_text_index>2 && phone_text_index<19)
            {
                SYSLOG_INFO("%s",call_label.numstr);
                EPAT_LOG(matrix_dialpad, P_INFO, "%s", call_label.numstr);
                _ui_screen_change(ui_callScreen, LV_SCR_LOAD_ANIM_MOVE_BOTTOM, 500, 0);
                _ui_label_set_property(ui_callNumber, _UI_LABEL_PROPERTY_TEXT, call_label.numstr);
                _ui_state_modify(ui_callButton, LV_STATE_CHECKED, _UI_MODIFY_STATE_REMOVE);
                _ui_flag_modify(ui_callPanel, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_REMOVE);
                onDialout(call_label.numstr);
            }
        }
        else if(id == 13)
        {
            dialout_number_del(phone_text_index);
            if(ui_Label_Sim != NULL){
                lv_obj_add_flag(ui_Label_Sim, LV_OBJ_FLAG_HIDDEN);
            }
        }
        else if(id == 14)
        {
            dialout_number_del(1);
        }
    }
}
/**
  \fn
  \brief
  \return
*/
void ui_event_button_dial(lv_event_t *e)
{
    lv_disp_t *display = lv_disp_get_default();
    lv_obj_t *actScr = lv_disp_get_scr_act(display);
    if (actScr != ui_controlScreen)
    {
        return;
    }
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        toAppList = false;  // flag was open from app list
        phone_text_index=0;
        memset(call_label.numstr,0,sizeof(call_label.numstr));
        _ui_screen_change(ui_dialpadScreen, LV_SCR_LOAD_ANIM_MOVE_BOTTOM, 500, 0);
        // lv_scr_load_anim(ui_dialpadScreen, LV_SCR_LOAD_ANIM_MOVE_BOTTOM, 500, 0, false);
    }
    if(event_code < LV_EVENT_COVER_CHECK && event_code > LV_EVENT_DRAW_PART_END){
        LV_LOG_USER("%d",event_code);
    }
}
/**
  \fn
  \brief
  \return
*/
void ui_event_button_call(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    // SYSLOG_INFO("ui_event_button_call %d",event_code);
    if (event_code == LV_EVENT_VALUE_CHANGED && !lv_obj_has_state(target, LV_STATE_CHECKED))
    {
        // LV_LOG_USER("call -> hangup");
        SYSLOG_INFO("call -> hangup");
        onHangUp();
        _ui_screen_change(ui_home, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0);
    }
    if (event_code == LV_EVENT_VALUE_CHANGED && lv_obj_has_state(target, LV_STATE_CHECKED))
    {
        SYSLOG_INFO("talk -> hangup");
        onHangUp();
        _ui_screen_change(ui_home, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0);
    }
}
/**
  \fn
  \brief    来电->响铃->通话中or挂断
  \return
*/
void ui_event_button_ringing(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_VALUE_CHANGED && lv_obj_has_state(target, LV_STATE_CHECKED))
    {
        LV_LOG_USER("ring -> talk");
        // lv_obj_clear_flag(ui_ringIcon, LV_OBJ_FLAG_HIDDEN);
        _ui_label_set_property(ui_ringName, _UI_LABEL_PROPERTY_TEXT, "通话中");
        _ui_label_set_property(ui_ringButtonText, _UI_LABEL_PROPERTY_TEXT, "挂断");
        onTalking();
    }
    if (event_code == LV_EVENT_VALUE_CHANGED && !lv_obj_has_state(target, LV_STATE_CHECKED))
    {
        LV_LOG_USER("talk -> hangup");
        onHangUp();
        _ui_screen_change(ui_home, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0);
    }
    if(event_code < LV_EVENT_COVER_CHECK && event_code > LV_EVENT_DRAW_PART_END){
        LV_LOG_USER("%d",event_code);
    }
}
/**
  \fn
  \brief
  \return
*/
void ui_event_resp_ringing(ui_label_t *label)
{
    LV_LOG_USER("%s,%s,%s,%s",label->numstr,label->name,label->icon,label->button);
    _ui_screen_change(ui_ringScreen, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0);
    _ui_label_set_property(ui_ringNumber, _UI_LABEL_PROPERTY_TEXT,label->numstr);
    // _ui_label_set_property(ui_ringName, _UI_LABEL_PROPERTY_TEXT,label->name);
    _ui_label_set_property(ui_ringIcon, _UI_LABEL_PROPERTY_TEXT,label->icon);
    // _ui_label_set_property(ui_ringButtonText, _UI_LABEL_PROPERTY_TEXT,label->button);
    lv_obj_clear_flag(ui_ringIcon, LV_OBJ_FLAG_HIDDEN);
    _ui_label_set_property(ui_ringName, _UI_LABEL_PROPERTY_TEXT, "响铃中");
    _ui_label_set_property(ui_ringButtonText, _UI_LABEL_PROPERTY_TEXT, "接听");
    EPAT_LOG(ui_event_resp_ringing, P_INFO, "%s", label->numstr);
}
/**
  \fn
  \brief
  \return
*/
// extern ui_label_t ring_label;
// void ui_event_ringing(lv_event_t *e)
// {
//     lv_disp_t *display = lv_disp_get_default();
//     lv_obj_t *actScr = lv_disp_get_scr_act(display);
//     if (actScr != ui_controlScreen)
//     {
//         return;
//     }
//     lv_event_code_t event_code = lv_event_get_code(e);
//     lv_obj_t *target = lv_event_get_target(e);
//     if (event_code == LV_EVENT_CLICKED)
//     {
//         // memset(&ring_label,0,sizeof(ring_label));
//         memset(ring_label.numstr,0,sizeof(call_label.numstr));
//         strcpy(ring_label.numstr,"199199166");
//         memset(ring_label.name,0,sizeof(call_label.name));
//         strcpy(ring_label.name,"芯大爷");
//         memset(ring_label.icon,0,sizeof(call_label.icon));
//         strcpy(ring_label.icon,"D:/ui_img_ring.png");
//         // strcpy(ring_label.button,"接听");
//         ui_event_resp_ringing(&ring_label);
//         lv_obj_clear_flag(ui_ringIcon, LV_OBJ_FLAG_HIDDEN);
//     }
//     if(event_code < LV_EVENT_COVER_CHECK && event_code > LV_EVENT_DRAW_PART_END)
//     {
//         LV_LOG_USER("%d",event_code);
//     }
// }
