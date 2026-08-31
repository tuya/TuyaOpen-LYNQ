
#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "version.h"
#ifdef FEATURE_SUBSYS_STATUS_ENABLE
#include "status.h"
#endif
#ifdef FEATURE_SUBSYS_SYSTIME_ENABLE
#include "systime.h"
#endif


extern uint32_t millis(void);

static bool _simCard = true;
static bool _isInited = false;
static bool autoUpdateEnable = true;   // 自动获取并更新时间
lv_obj_t * ui_Home = NULL;
static lv_obj_t * ui_HomeTime = NULL;
static lv_obj_t * ui_HomeDate = NULL;
lv_obj_t * ui_ImageSig = NULL;
lv_obj_t * ui_ImageBat = NULL;
static lv_obj_t * ui_Label_Sim = NULL;
static lv_obj_t * ui_Label_Version = NULL;
PLAT_FPSRAM_ZI_CUST char fpui_debug_label[LABEL_LENGTH_MAX+1];
PLAT_FPSRAM_ZI_CUST char fpui_home_paddr[ADDR_LENGTH_MAX+1];
PLAT_FPSRAM_ZI_CUST char fpui_bat_paddr[ADDR_LENGTH_MAX+1];
PLAT_FPSRAM_ZI_CUST char fpui_sig_paddr[ADDR_LENGTH_MAX+1];
PLAT_FPSRAM_ZI_CUST char time_str[12]; 
PLAT_FPSRAM_ZI_CUST char date_str[20];
/**
  \fn
  \brief
  \return
*/
void ui_home_screen(PhoneUI_home_t *data)
{
    if(lv_obj_is_valid(ui_Home) && ui_Home != NULL){
        lv_obj_clean(ui_Home);
        lv_obj_del_async(ui_Home);
        ui_Home = NULL;
    }
    if(data == NULL){
        osEventFlagsSet(uiEvtHandle, (1U << PAGE_HOME));
        return;  
    } 
    ui_Home = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_Home, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_bg_color(ui_Home, lv_color_make(0, 0, 0), LV_PART_MAIN|LV_STATE_DEFAULT);
    if(strlen(fpui_home_paddr) && strlen(fpui_home_paddr)<sizeof(fpui_home_paddr)){
        lv_obj_set_style_bg_img_src(ui_Home, fpui_home_paddr, LV_PART_MAIN | LV_STATE_DEFAULT);
    } 

    if(strlen(fpui_sig_paddr) && strlen(fpui_sig_paddr)<sizeof(fpui_sig_paddr)){
        ui_ImageSig = lv_img_create(ui_Home);
        lv_img_set_src(ui_ImageSig, fpui_sig_paddr);
        lv_obj_set_width(ui_ImageSig, LV_SIZE_CONTENT);  
        lv_obj_set_height(ui_ImageSig, LV_SIZE_CONTENT);  
        lv_obj_align(ui_ImageSig, LV_ALIGN_TOP_LEFT, 0, 0); 
        lv_obj_add_flag(ui_ImageSig, LV_OBJ_FLAG_ADV_HITTEST);   
        lv_obj_clear_flag(ui_ImageSig, LV_OBJ_FLAG_SCROLLABLE);  
    } 

    if(strlen(fpui_bat_paddr) && strlen(fpui_bat_paddr)<sizeof(fpui_bat_paddr)){
        ui_ImageBat = lv_img_create(ui_Home);
        lv_img_set_src(ui_ImageBat, fpui_bat_paddr);
        lv_obj_set_width(ui_ImageBat, LV_SIZE_CONTENT);  
        lv_obj_set_height(ui_ImageBat, LV_SIZE_CONTENT);  
        lv_obj_align(ui_ImageBat, LV_ALIGN_TOP_RIGHT, -2, 0); 
        lv_obj_add_flag(ui_ImageBat, LV_OBJ_FLAG_ADV_HITTEST);     
        lv_obj_clear_flag(ui_ImageBat, LV_OBJ_FLAG_SCROLLABLE);   
    }

    ui_HomeTime = lv_label_create(ui_Home);
    lv_obj_set_width(ui_HomeTime, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_HomeTime, LV_SIZE_CONTENT);
    lv_obj_set_pos(ui_HomeTime, 4, 40);
    lv_obj_set_align(ui_HomeTime, LV_ALIGN_TOP_LEFT);

    lv_obj_set_style_text_color(ui_HomeTime, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_HomeTime, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_HomeTime, pSystem66, LV_PART_MAIN | LV_STATE_DEFAULT);
    if(strlen(time_str)>1 && strlen(time_str)<sizeof(time_str)){
        lv_label_set_text(ui_HomeTime, time_str);
    }

    ui_HomeDate = lv_label_create(ui_Home);
    lv_obj_set_width(ui_HomeDate, LV_SIZE_CONTENT); 
    lv_obj_set_height(ui_HomeDate, LV_SIZE_CONTENT);    
    lv_obj_set_pos(ui_HomeDate, 12, 108);
    lv_obj_set_align(ui_HomeDate, LV_ALIGN_TOP_LEFT);
    
    lv_obj_set_style_text_color(ui_HomeDate, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_HomeDate, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_text_font(ui_HomeDate, pLight36, LV_PART_MAIN | LV_STATE_DEFAULT);
    if(strlen(date_str)>1 && strlen(date_str)<sizeof(date_str)){
        lv_label_set_text(ui_HomeDate, date_str);
    }

    ui_Label_Sim = lv_label_create(ui_Home);
    lv_obj_set_width(ui_Label_Sim, LV_SIZE_CONTENT);   
    lv_obj_set_height(ui_Label_Sim, LV_SIZE_CONTENT);    
    lv_obj_align(ui_Label_Sim, LV_ALIGN_CENTER, 0, 64); 
    if (simGetStatus(0) != SIM_READY){
        lv_label_set_text(ui_Label_Sim, "请插入SIM卡");
        _simCard = false;
    }
    else lv_label_set_text(ui_Label_Sim, " ");
    lv_obj_set_style_text_color(ui_Label_Sim, lv_color_make(0, 0, 0xff), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_text_font(ui_Label_Sim, pLight36, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_Label_Version = lv_label_create(ui_Home);
    lv_obj_set_width(ui_Label_Version, LV_SIZE_CONTENT);   
    lv_obj_set_height(ui_Label_Version, LV_SIZE_CONTENT);
    lv_obj_set_style_text_font(ui_Label_Version, pSystem16, LV_PART_MAIN | LV_STATE_DEFAULT); 
    lv_obj_align(ui_Label_Version, LV_ALIGN_BOTTOM_MID, 0, -48);
    lv_obj_set_style_text_color(ui_Label_Version, lv_color_make(0, 0xff, 0), LV_PART_MAIN | LV_STATE_DEFAULT);
    if(strlen(fpui_debug_label) && strlen(fpui_debug_label)<sizeof(fpui_debug_label)){
        lv_label_set_text(ui_Label_Version, fpui_debug_label);
    }
    else lv_label_set_text(ui_Label_Version, " ");
    // UI_LOG("%s,%s",data->bottomBtn[0],data->bottomBtn[2]);
    ui_btn_set(data->bottomBtn,lv_color_white(),lv_color_black());
    lv_disp_load_scr(ui_Home);
    osEventFlagsSet(uiEvtHandle, (1U << PAGE_HOME));
}
/**
  \fn
  \brief
  \return
*/
static uiUpdateIndex_e _dVersion=NEXT_HOLDON;
void ui_version_set(char *str)
{
    if(strcmp(fpui_debug_label, str) == 0 || strlen(str)>=sizeof(fpui_debug_label)){
        return;
    }
    memset(fpui_debug_label,0,sizeof(fpui_debug_label));
    if(str != NULL) strcpy(fpui_debug_label,str);
    _dVersion=NEXT_UPDATE;
}

/**
  \fn
  \brief
  \return
*/
static uiUpdateIndex_e _dateStr=NEXT_HOLDON;
static uiUpdateIndex_e _timeStr=NEXT_HOLDON;
void ui_time_set(char *date,char *time)
{
    if(date !=NULL && strlen(date)<sizeof(date_str)){
        memset(date_str,0,sizeof(date_str));
        strcpy(date_str,date);
        _dateStr=NEXT_UPDATE;
    }
    if(time !=NULL && strlen(time)<sizeof(time_str)){
        memset(time_str,0,sizeof(time_str));
        strcpy(time_str,time);
        _timeStr=NEXT_UPDATE;
    }
}
/**
  \fn          
  \brief       
  \return
*/
static uiUpdateIndex_e _backround=NEXT_HOLDON;
static uiUpdateIndex_e _sigIcon=NEXT_HOLDON;
static uiUpdateIndex_e _batIcon=NEXT_HOLDON;
void ui_addr_set(lv_obj_t * img,char *addr)
{
    if(img == ui_Home && addr !=NULL && ui_Home != NULL){
        if(strcmp(fpui_home_paddr, addr) != 0 && strlen(addr)<ADDR_LENGTH_MAX){
            memset(fpui_home_paddr,0,sizeof(fpui_home_paddr));
            strcpy(fpui_home_paddr,addr);
            _backround=NEXT_UPDATE;
        }
    }
    if(img == ui_ImageSig && addr !=NULL && ui_ImageSig != NULL){
        if(strcmp(fpui_sig_paddr, addr) != 0 && strlen(addr)<ADDR_LENGTH_MAX){
            memset(fpui_sig_paddr,0,sizeof(fpui_sig_paddr));
            strcpy(fpui_sig_paddr,addr);
            _sigIcon=NEXT_UPDATE;
        }
    }
    if(img == ui_ImageBat && addr !=NULL && ui_ImageBat != NULL){
        if(strcmp(fpui_bat_paddr, addr) != 0 && strlen(addr)<ADDR_LENGTH_MAX){
            memset(fpui_bat_paddr,0,sizeof(fpui_bat_paddr));
            strcpy(fpui_bat_paddr,addr);
            _batIcon=NEXT_UPDATE;
        }
    }
}
/**
  \fn          
  \brief       
  \return
*/
static uint8_t _runmin=0;
static uint8_t _runday=0;
void ui_auto_update_time(bool en)
{
    autoUpdateEnable = en;
}
/**
  \fn
  \brief
  \return
*/
uint8_t ui_home_auto_update(void)
{
    uint8_t ret = 0;
    if(_isInited)
    {
        time_t now = time_time(NULL);
        struct tm *timeinfo = time_localtime(&now);
        if(timeinfo->tm_min != _runmin && autoUpdateEnable){
            _runmin=timeinfo->tm_min;
            memset(time_str,0,sizeof(time_str));
            strftime(time_str, sizeof(time_str), "%H:%M", timeinfo);
            if(lv_obj_is_valid(ui_HomeTime)) {
                lv_label_set_text(ui_HomeTime,time_str);
                ret |= 0x1;
            }
        }
        else if(_timeStr==NEXT_UPDATE && lv_obj_is_valid(ui_HomeTime)){
            lv_label_set_text(ui_HomeTime,time_str);
            _timeStr=NEXT_HOLDON;
            ret |= 0x1;
        }

        if(timeinfo->tm_mday != _runday && autoUpdateEnable){
            _runday=timeinfo->tm_mday;
            memset(date_str,0,sizeof(date_str));
            strftime(date_str, sizeof(date_str), "%Y-%m-%d", timeinfo);
            if(lv_obj_is_valid(ui_HomeDate)) {
                lv_label_set_text(ui_HomeDate,date_str);
                ret |= 0x2;
            }
        }
        else if(_dateStr==NEXT_UPDATE && lv_obj_is_valid(ui_HomeDate)){
            lv_label_set_text(ui_HomeDate,date_str);
            _dateStr=NEXT_HOLDON;
            ret |= 0x2;
        }

        if(_sigIcon==NEXT_UPDATE && lv_obj_is_valid(ui_ImageSig)){
            lv_img_set_src(ui_ImageSig, fpui_sig_paddr);
            _sigIcon=NEXT_HOLDON;
            ret |= 0x4;
        }
        if(_batIcon==NEXT_UPDATE && lv_obj_is_valid(ui_ImageBat)){
            lv_img_set_src(ui_ImageBat, fpui_bat_paddr);
            _batIcon=NEXT_HOLDON;
            ret |= 0x8;
        }
        if(_backround==NEXT_UPDATE && lv_obj_is_valid(ui_Home)){
            lv_obj_set_style_bg_img_src(ui_Home, fpui_home_paddr, LV_PART_MAIN | LV_STATE_DEFAULT);
            _backround=NEXT_HOLDON;
            ret |= 0x10;
        }
        if (_simCard && simGetStatus(0) != SIM_READY && lv_obj_is_valid(ui_Label_Sim)){
            lv_label_set_text(ui_Label_Sim, "请插入SIM卡");
            _simCard = false;
            ret |= 0x20;
        }
        else if (!_simCard && simGetStatus(0) == SIM_READY && lv_obj_is_valid(ui_Label_Sim)){
            lv_label_set_text(ui_Label_Sim, " ");
            _simCard = true;
            ret |= 0x20;
        }
        if (_dVersion==NEXT_UPDATE && lv_obj_is_valid(ui_Label_Version)){
            if(strlen(fpui_debug_label))lv_label_set_text(ui_Label_Version,fpui_debug_label);
            else lv_label_set_text(ui_Label_Version, " ");
            _dVersion=NEXT_HOLDON;
            ret |= 0x40;
        }
        if(ret) lv_obj_update_layout(ui_Home);
    }
    return ret;
}
/**
  \fn
  \brief
  \return
*/
lv_obj_t *ui_home_set(PhoneUI_home_t *data)
{
    if(!_isInited){
        _isInited = true;
        memset(fpui_debug_label,0,sizeof(fpui_debug_label));
        sprintf(fpui_debug_label,"%s v%s.%s",__DATE__,SDK_MINOR_VERSION,SDK_PATCH_VERSION);
        time_t now = time_time(NULL);
        struct tm *timeinfo = time_localtime(&now);
        // uint8_t _runmin=timeinfo->tm_min;
        // uint8_t _runday=timeinfo->tm_mday;
        memset(time_str,0,sizeof(time_str));
        memset(date_str,0,sizeof(date_str));
        strftime(date_str, sizeof(date_str), "%Y-%m-%d", timeinfo);
        strftime(time_str, sizeof(time_str), "%H:%M", timeinfo);
        memset(fpui_sig_paddr,0,sizeof(fpui_sig_paddr));
        memset(fpui_bat_paddr,0,sizeof(fpui_bat_paddr));
        memset(fpui_home_paddr,0,sizeof(fpui_home_paddr));
        strcpy(fpui_home_paddr,LV_IMAGE_09);
    }
    if(data == NULL && ui_Home != NULL){
        if(lv_obj_is_valid(ui_Home)) lv_obj_add_flag(ui_Home, LV_OBJ_FLAG_HIDDEN);
    }
    else {
        if(data->date !=NULL && strlen(data->date)<sizeof(date_str)){
            memset(date_str,0,sizeof(date_str));
            strcpy(date_str,data->date);
        }
        
        if(data->time !=NULL && strlen(data->time)<sizeof(time_str)){
            memset(time_str,0,sizeof(time_str));
            strcpy(time_str,data->time);
        }

        if(data->image !=NULL && strlen(data->image)<ADDR_LENGTH_MAX){
            memset(fpui_home_paddr,0,sizeof(fpui_home_paddr));
            strcpy(fpui_home_paddr,data->image);
        }

        if(data->net_sig !=NULL && strlen(data->net_sig)<ADDR_LENGTH_MAX){
            memset(fpui_sig_paddr,0,sizeof(fpui_sig_paddr));
            strcpy(fpui_sig_paddr,data->net_sig);
            // SYSLOG_WARNING("update %s",fpui_sig_paddr);
        }

        if(data->bat_vol !=NULL && strlen(data->bat_vol)<ADDR_LENGTH_MAX){
            memset(fpui_bat_paddr,0,sizeof(fpui_bat_paddr));
            strcpy(fpui_bat_paddr,data->bat_vol);
            // SYSLOG_WARNING("update %s",fpui_bat_paddr);
        }
    } 
    GuiMsgT msgPtr;
    msgPtr.refresh_ms = 10; 
    msgPtr.ui_set = (void* (*)(void *))ui_home_screen;
    msgPtr.ui_del = NULL;
    msgPtr.ui_data = data;
    guiSendMsg(&msgPtr); 
    osEventFlagsClear(uiEvtHandle, (1U << PAGE_HOME));
    osEventFlagsWait(uiEvtHandle, (1U << PAGE_HOME), osFlagsWaitAll, 200);
    // SYSLOG_INFO("%d",(millis()-_mes_time));
    return ui_Home;
}

/**
  \fn          
  \brief       
  \return
*/
// static lv_obj_t *_screen_slider = NULL;
static lv_obj_t *_bar_slider = NULL;
PLAT_FPSRAM_ZI_CUST fpui_bar_t _ui_bar_data;

static uint32_t mes_bar_time_ms = 0;
lv_obj_t *ui_bar_create(fpui_bar_t *data) 
{
    uint32_t mark = millis();
    if(_bar_slider != NULL){
        if(lv_obj_is_valid(_bar_slider)){
            lv_obj_clean(_bar_slider);
        }
        _bar_slider = NULL;
    }
    if(data == NULL){
        // goto labelEnd;
        return NULL;
    }
    if(data->screen == NULL || lv_scr_act() != data->screen){
        UI_ERR("invalid screen --- %ds,0x%X != 0x%X(new)",millis()/1000,data->screen,lv_scr_act());
        return NULL;
    }
    if(data->range < data->value || data->width > LCD_WIDTH || data->height > LCD_HEIGHT ){
        UI_ERR("invalid data --- %d,%d,%d,%d",data->value,data->range,data->width,data->height);
        return NULL;
    }
    _bar_slider = lv_obj_create(data->screen);
    lv_obj_set_width(_bar_slider, LV_SIZE_CONTENT);
    lv_obj_set_height(_bar_slider, LV_SIZE_CONTENT);
    lv_obj_align(_bar_slider, LV_ALIGN_CENTER, data->offsetX, data->offsetY);
    lv_obj_set_style_bg_opa(_bar_slider, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT); 

    lv_obj_t * _slider = lv_slider_create(_bar_slider);
    lv_obj_set_width(_slider, data->width);
    lv_obj_set_height(_slider, data->height);
    if(data->offsetX > 0){
        lv_obj_align(_slider, LV_ALIGN_TOP_RIGHT, 0, 0);  
    }else{
        lv_obj_align(_slider, LV_ALIGN_TOP_LEFT, 0, 0);
    }
    lv_slider_set_range(_slider, 0, data->range);
    lv_slider_set_value(_slider, data->value, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(_slider, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(_slider, LV_OPA_40, LV_PART_MAIN); 

    lv_obj_set_style_bg_color(_slider, (lv_color_t)(data->color), LV_PART_INDICATOR); 
    lv_obj_set_style_bg_opa(_slider, LV_OPA_TRANSP, LV_PART_KNOB);

    lv_obj_t * _label = lv_label_create(_bar_slider);
    lv_label_set_text_fmt(_label, "%d", data->value);
    lv_obj_set_style_bg_opa(_label, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_text_color(_label, (lv_color_t)(data->color), LV_PART_MAIN);
    // lv_obj_set_style_text_color(_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(_label, pSystem16, LV_PART_MAIN | LV_STATE_DEFAULT);
    if(data->height > data->width){
        if(data->offsetX > 0){
            lv_obj_align_to(_label, _slider, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 0);
        }else{
            lv_obj_align_to(_label, _slider, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
        }
    }
    else if(data->height < data->width){
        lv_obj_align_to(_label, _slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    }
// labelEnd:
    mark = millis() - mark;
    mes_bar_time_ms += mark;
    return _bar_slider;
}
/**
  \fn
  \brief
  \return
*/
void *ui_bar_set(fpui_bar_t *data)   
{
    GuiMsgT msgPtr;
    uint32_t mark = millis();
    if(lv_scr_act() == NULL){
        UI_LOG("invalid screen --- %ds",millis()/1000);
        return NULL;
    }
    if(data == NULL){
        memset(&_ui_bar_data,0,sizeof(fpui_bar_t));
        msgPtr.ui_data = NULL;
    }
    else if(data->range < data->value || data->offsetX>LCD_WIDTH || data->offsetY>LCD_HEIGHT){
        UI_LOG("invalid data %d.%03ds --- %d/%d,X%d,Y%d",mark/1000,mark%1000, data->value,data->range,data->offsetX,data->offsetY);
        return NULL;
    }
    else if(data->width<1 || data->height<1 || data->width >= LCD_WIDTH || data->height >= LCD_HEIGHT){
        UI_LOG("invalid data %d.%03ds --- W%d,H%d",mark/1000,mark%1000,data->width,data->height);
        return NULL;
    }
    else {
        memset(&_ui_bar_data,0,sizeof(fpui_bar_t));
        memcpy(&_ui_bar_data,data,sizeof(fpui_bar_t));
        _ui_bar_data.screen = lv_scr_act();
        msgPtr.ui_data = &_ui_bar_data;
    }
    msgPtr.refresh_ms = 10;
    msgPtr.ui_set = (void* (*)(void *))ui_bar_create;
    msgPtr.ui_del = NULL;
    guiSendMsg(&msgPtr);
    osDelay(9);    //绘图时间9(≤23)ms
    return &_ui_bar_data;
}
/**
  \fn
  \brief
  \return
*/
lv_obj_t *ui_home_test(void * bat,void * net,void * image)
{
   static char home_btn[3][8] = {"查看","发送","捷径"};
    PhoneUI_home_t home = {
        .date = NULL,
        .time = NULL,
        .bat_vol = bat,
        .net_sig = net,
        .image = image,
        .simStat = simGetStatus(0),
        .bottomBtn = home_btn,
    };
    return ui_home_set(&home);
}
/**
  \fn
  \brief
  \return
*/
lv_obj_t *ui_bar_test(uint8_t value,uint8_t range,int16_t offsetX,int16_t offsetY,int16_t width,int16_t height,uint16_t color)
{
    fpui_bar_t bar_test = {
        .object = NULL,
        .screen = NULL,
        .value = value,
        .range = range,
        .offsetX = offsetX,
        .offsetY = offsetY,
        .width = width,
        .height = height,
        .color = color,
    };
    if(range<value || width>=LCD_WIDTH || height>=LCD_HEIGHT || width < 4 || height < 4 ){
        return NULL;
    }
    #ifdef UI_TEST_ITEM
    UI_LOG("%d,%d/%d,%d,%d,%d,%d",mes_bar_time_ms,value,range,offsetX,offsetY,width,height,color);
    #endif
    return ui_bar_set(&bar_test);
}
