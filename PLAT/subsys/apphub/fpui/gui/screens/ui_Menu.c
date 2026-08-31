
#include "ui.h"
#include "cmsis_os2.h"
#include "osasys.h"

extern uint32_t millis(void);

static lv_obj_t * _ui_menu = NULL;
PLAT_FPSRAM_ZI_CUST char fpui_menu_title[MAX_TITLE_LENGTH+1];
PLAT_FPSRAM_ZI_CUST char fpui_menu_image[ADDR_LENGTH_MAX+1];
PLAT_FPSRAM_ZI_CUST PhoneUI_menu_t _ui_menu_data;
static uint32_t mes_menu_ms = 0;
static uint32_t mes_menu_cnt = 0;
PLAT_FPSRAM_ZI_CUST uint16_t __ALIGNED(4) dispJpegBuffer[240*240*2];
static lv_img_dsc_t ui_img_dsc_menu;
/**
  \fn
  \brief
  \return
*/
lv_obj_t *ui_menu_screen(PhoneUI_menu_t *data)
{
    uint32_t mark = millis();
    if(_ui_menu != NULL){
        ui_obj_clean(_ui_menu);
        _ui_menu = NULL;
    }
    if(data == NULL){
        return NULL;
    }
    _ui_menu = lv_obj_create(NULL);
    lv_obj_clear_flag(_ui_menu, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_align(_ui_menu, LV_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_color(_ui_menu, lv_color_white(), LV_PART_MAIN|LV_STATE_DEFAULT);
    if(data->image != NULL && strlen(data->image)<ADDR_LENGTH_MAX){
        ui_img_dsc_menu.data = (const uint8_t *)dispJpegBuffer;
        // memset(dispJpegBuffer, 0, sizeof(dispJpegBuffer));
        uint32_t imgsize = conv_jpeg_rgb565(data->image,&ui_img_dsc_menu);
        // UI_LOG("%s[%d,%d,%d]\r\n",data->image,imgsize,ui_img_dsc_menu.header.w,ui_img_dsc_menu.header.h); 
        if(imgsize) lv_obj_set_style_bg_img_src(_ui_menu, &ui_img_dsc_menu, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    if(data->title != NULL && strlen(data->title)<MAX_TITLE_LENGTH){
        lv_obj_t * ui_menu_title = lv_label_create(_ui_menu);
        lv_obj_set_width(ui_menu_title, LV_SIZE_CONTENT);
        lv_obj_set_height(ui_menu_title, LV_SIZE_CONTENT);
        lv_label_set_text(ui_menu_title, data->title);
        lv_obj_align(ui_menu_title, LV_ALIGN_TOP_MID, 0, 16); 
        lv_obj_set_style_bg_color(ui_menu_title, lv_color_make(0, 0, 0), LV_PART_MAIN);
        lv_obj_set_style_text_color(ui_menu_title, lv_color_black(), LV_PART_MAIN);
    }
    if(data->bottomBtn){
        ui_btn_set(data->bottomBtn,lv_color_black(),lv_color_white());
    }
    lv_disp_load_scr(_ui_menu);
// labelEnd:
    mark = millis() - mark;
    mes_menu_ms += mark;
    mes_menu_cnt += 1;
    return _ui_menu;
}
/**
  \fn
  \brief
  \return
*/
void *ui_menu_set(PhoneUI_menu_t *data)
{
    GuiMsgT msgPtr;
    if(data == NULL){
        memset(&_ui_menu_data,0,sizeof(PhoneUI_menu_t));
        msgPtr.ui_data = NULL;
    }
    else if(strlen(data->image)>ADDR_LENGTH_MAX || strlen(data->title)>MAX_TITLE_LENGTH){
        UI_LOG("invalid data --- %d s",millis()/1000);
        return NULL;
    }
    else {
        memset(&_ui_menu_data,0,sizeof(PhoneUI_menu_t));
        memcpy(&_ui_menu_data,data,sizeof(PhoneUI_menu_t));
        memset(fpui_menu_title,0,sizeof(fpui_menu_title));
        if(data->title != NULL){
            strcpy(fpui_menu_title,data->title);
        }
        _ui_menu_data.title = fpui_menu_title;
        memset(fpui_menu_image,0,sizeof(fpui_menu_image));
        if(data->image != NULL){
            strcpy(fpui_menu_image,data->image);
        }
        _ui_menu_data.image = fpui_menu_image;
        msgPtr.ui_data = &_ui_menu_data;
    }
    msgPtr.refresh_ms = 10;
    msgPtr.ui_set = (void* (*)(void *))ui_menu_screen;
    msgPtr.ui_del = NULL;
    guiSendMsg(&msgPtr);
    osDelay(5); //绘图时间 10ms
    return &_ui_menu_data;
}
/**
  \fn
  \brief
  \return
*/
// static lv_obj_t *_screen_message = NULL;
static uint8_t messageKeepNum = POP_KEEP_NUM;
static lv_obj_t *_ui_message;
static uint32_t mes_message_ms = 0;
static uint32_t mes_message_cnt = 0;
lv_obj_t *ui_message_create(fpui_message_t *data) 
{
    uint32_t mark = millis();
    if(data == NULL) {
        uint8_t num = ui_obj_clean(_ui_message);
        num = num;
        #ifdef UI_TEST_ITEM
        UI_LOG("[%d:%d]clean 0x%X",num,i,_ui_message);
        #endif
        _ui_message = NULL;
        return NULL;
    }
    else if(data->screen == NULL || data->screen != lv_scr_act()){
        messageKeepNum = POP_KEEP_NUM;
        UI_WRN("invalid screen --- 0x%X != 0x%X(new)",data->screen,lv_scr_act());
        return NULL;
    }
    else if(data->width > LCD_WIDTH || data->height > LCD_HEIGHT){
        UI_ERR("invalid size --- W%d,H%d",data->width,data->height);
        return NULL;
    }
    else if(data->offsetX < -(LCD_WIDTH/2) || data->offsetY < -(LCD_HEIGHT/2) \
            || data->offsetX > LCD_WIDTH/2 || data->offsetY > LCD_HEIGHT/2){
        UI_ERR("invalid offset --- X%d,Y%d",data->offsetX,data->offsetY);
        return NULL;
    }
    if(data->overlay){
        messageKeepNum -= 1;
        // now = POP_KEEP_NUM-messageKeepNum;
        // #ifdef UI_TEST_ITEM
        // UI_LOG("overlay[%d] --- 0x%X,0x%X",now,data->screen,lv_scr_act());
        // #endif
    }
    else{
        ui_obj_clean(_ui_message);
        _ui_message = NULL;
    }
    if(lv_obj_is_valid(data->object) && data->object != NULL){
        #ifdef UI_TEST_ITEM
        UI_LOG("\t---0x%X = 0x%X,0x%X",_ui_message,data->object,data->screen);
        #endif
    }
    else{
        _ui_message = lv_obj_create(data->screen);
        data->object = _ui_message;
        lv_obj_set_size(data->object, data->width, data->height);
        lv_obj_set_style_radius(data->object, 8, LV_PART_MAIN); 
        lv_obj_set_style_border_post(data->object, true, LV_PART_MAIN); 
        lv_obj_set_style_bg_color(data->object, (lv_color_t)data->color, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(data->object, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_align(data->object, LV_ALIGN_CENTER, data->offsetX, data->offsetY);
    }
    if(lv_obj_is_valid(data->object) && strlen(data->context) < MESSAGE_LENGTH_MAX){
        lv_obj_t *_ui_message_text = lv_label_create(data->object);
        lv_obj_set_width(_ui_message_text, data->width);
        lv_obj_set_height(_ui_message_text, LV_SIZE_CONTENT);
        lv_obj_set_style_pad_left(_ui_message_text, 8, LV_PART_MAIN);
        lv_obj_set_style_pad_right(_ui_message_text, 8, LV_PART_MAIN);
        lv_obj_set_style_pad_top(_ui_message_text, 8, LV_PART_MAIN);
        lv_obj_set_style_pad_bottom(_ui_message_text, 8, LV_PART_MAIN);
        lv_label_set_long_mode(_ui_message_text, LV_LABEL_LONG_WRAP);
        lv_label_set_text(_ui_message_text, data->context);
        if((data->color & 0xF0F0) == 0xF0F0){
            lv_obj_set_style_text_color(_ui_message_text, lv_color_black(), LV_PART_MAIN);
        } 
        else lv_obj_set_style_text_color(_ui_message_text, lv_color_white(), LV_PART_MAIN);
    }
    
// labelEnd:
    mark = millis() - mark;
    mes_message_ms += mark;
    mes_message_cnt += 1;
    return _ui_message;
}
/**
  \fn
  \brief
  \return
*/
PLAT_FPSRAM_ZI_CUST fpui_message_t _ui_message_data;
PLAT_FPSRAM_ZI_CUST char _ui_message_context[MESSAGE_LENGTH_MAX+1];
void *ui_message_set(fpui_message_t *data)
{
    GuiMsgT msgPtr;
    if(data == NULL) {
        memset(&_ui_message_data,0,sizeof(fpui_message_t));
        msgPtr.ui_data = NULL;
    }
    else if(strlen(data->context) > MESSAGE_LENGTH_MAX){
        UI_LOG("invalid context --- %d,%s",strlen(data->context),data->context);
        return NULL;
    }
    else if(data->offsetX < -100 || data->offsetY < -100 || data->offsetX > 100 || data->offsetY > 100){
        UI_LOG("invalid offset --- X%d,Y%d",data->offsetX,data->offsetY);
        return NULL;
    }
    else if(data->width>LCD_WIDTH || data->height>LCD_HEIGHT || data->width < 40 || data->height < 40){
        UI_LOG("invalid size --- W%d,H%d",data->width,data->height);
        return NULL;
    }
    else if(lv_scr_act() == NULL) {
        UI_LOG("invalid screen --- %d",millis()/1000);
        return NULL;
    }
    else {
        memset(&_ui_message_data,0,sizeof(fpui_message_t));
        memcpy(&_ui_message_data,data,sizeof(fpui_message_t));

        memset(_ui_message_context,0,sizeof(_ui_message_context));
        if(data->context != NULL){
            strcpy(_ui_message_context,data->context);
        }
        _ui_message_data.context = _ui_message_context;
        _ui_message_data.screen = lv_scr_act();
        msgPtr.ui_data = &_ui_message_data;
    }
    msgPtr.refresh_ms = 10;
    msgPtr.ui_set = (void* (*)(void *))ui_message_create;
    msgPtr.ui_del = NULL;
    guiSendMsg(&msgPtr);
    osDelay(2); //绘图时间 <5ms
    return msgPtr.ui_data;
}
/**
  \fn
  \brief
  \return
*/
static lv_obj_t * _ui_HeadInfo = NULL;
static lv_obj_t *_screen_headinfo = NULL;
PLAT_FPSRAM_ZI_CUST char HeadInfoText[LABEL_LENGTH_MAX+1];
lv_obj_t *ui_headinfo_pop(char  *info)
{
    if(_ui_HeadInfo != NULL && lv_obj_is_valid(_ui_HeadInfo)){
        lv_obj_clean(_ui_HeadInfo);
        _ui_HeadInfo = NULL;
    }
    if(info == NULL) {
        return NULL;
    }
    if(_screen_headinfo == NULL || lv_scr_act() == NULL || lv_scr_act() != _screen_headinfo){
        UI_LOG("invalid screen --- %d.%03ds",millis()/1000,millis()%1000);
        return NULL;
    }
    _ui_HeadInfo = lv_obj_create(_screen_headinfo);
    lv_obj_set_size(_ui_HeadInfo, LV_PCT(100), 42);
    lv_obj_align(_ui_HeadInfo, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(_ui_HeadInfo, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(_ui_HeadInfo, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    if(strlen(info) && strlen(info)<LABEL_LENGTH_MAX){
        lv_obj_t * _ui_labeltext = lv_label_create(_ui_HeadInfo);
        lv_obj_set_width(_ui_labeltext, LV_PCT(100));
        lv_obj_set_height(_ui_labeltext, LV_SIZE_CONTENT);
        lv_obj_align(_ui_labeltext, LV_ALIGN_TOP_MID, 0, 2);
        lv_label_set_text(_ui_labeltext, info);
        // lv_obj_set_style_text_font(_ui_labeltext, &free36, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(_ui_labeltext, lv_color_white(), LV_PART_MAIN);
    }
    return _ui_HeadInfo;
}

/**
  \fn
  \brief
  \return
*/
lv_obj_t *ui_headinfo_set(char  *info)
{
    if (lv_scr_act() == NULL) {
        UI_LOG("invalid screen --- %d s",millis()/1000);
        return NULL;
    }
    else if(strlen(info) > LABEL_LENGTH_MAX){
        UI_LOG("invalid data --- %d s",millis()/1000);
        return NULL;
    }
    else if(info == NULL) {
        memset(HeadInfoText,0,sizeof(HeadInfoText));
        if(lv_obj_is_valid(_ui_HeadInfo)) lv_obj_add_flag(_ui_HeadInfo, LV_OBJ_FLAG_HIDDEN);
    }
    else {
        memset(HeadInfoText,0,sizeof(HeadInfoText));
        strcpy(HeadInfoText,info);
        _screen_headinfo = lv_scr_act();
    }
    GuiMsgT msgPtr;
    msgPtr.refresh_ms = 10;
    msgPtr.ui_set = (void* (*)(void *))ui_headinfo_pop;
    msgPtr.ui_del = NULL;
    msgPtr.ui_data = HeadInfoText;
    guiSendMsg(&msgPtr);
    return _ui_HeadInfo;
}
/**
  \fn
  \brief
  \return
*/
void *ui_menu_test(char *title,void * image)
{
   static char menu_btn[3][8] = {"确定", "test","返回"};
    PhoneUI_menu_t menu_test = {
        .title = title,
        .image= image,
        .bottomBtn = menu_btn,
    };
    #ifdef UI_TEST_ITEM
    UI_LOG("%d/%d,%s",mes_menu_ms,mes_menu_cnt,title);
    #endif
    return ui_menu_set(&menu_test);
}
/**
  \fn
  \brief
  \return
*/
void *ui_message_test(char *context,int16_t offsetX,int16_t offsetY,uint16_t width,uint16_t height,uint16_t color,bool overlay)
{
    uint8_t now = POP_KEEP_NUM-messageKeepNum;
    fpui_message_t message_test = {
        .object = NULL,
        .screen = NULL,
        .title = NULL,
        .context = context,
        .overlay = overlay,
        .offsetX = offsetY,
        .offsetY = offsetY,
        .width = width,
        .height = height,
        .color = color,
    };
    if(offsetX < -100 || offsetY < -100 || offsetX > 100 || offsetY > 100){
        UI_LOG("invalid offset ---%ds,X%d,Y%d",millis()/1000,offsetX,offsetY);
        return NULL;
    }
    if(width > LCD_WIDTH || height > LCD_HEIGHT || width < 40 || height < 40){
        UI_LOG("invalid size ---%ds,W%d,H%d",millis()/1000,width,height);
        return NULL;
    }
    char strbuffer[24] = {0};
    sprintf(strbuffer,"弹窗%d,%d/%d",now,mes_message_ms,mes_message_cnt);
    message_test.context = strbuffer;
    #ifdef UI_TEST_ITEM
    UI_LOG("%d/%d,[%d]%d,X%d,Y%d,W%d,H%d,0x%04X,%s",mes_message_ms,mes_message_cnt,\
            now,overlay,offsetX,offsetY,width,height,color,context);
    #endif
    return ui_message_set(&message_test);
}