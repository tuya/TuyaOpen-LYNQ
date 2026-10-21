
#include "ui.h"
static lv_obj_t * _ui_list = NULL;
PLAT_FPSRAM_ZI_CUST PhoneUI_list_t _ui_list_data;
PLAT_FPSRAM_ZI_CUST char _ui_list_title[MAX_TITLE_LENGTH+1];
PLAT_FPSRAM_ZI_CUST char _ui_list_image[ADDR_LENGTH_MAX+1];
PLAT_FPSRAM_ZI_CUST char _ui_list_items[MAX_LIST_ITEMS][LABEL_LENGTH_MAX+1];
static uint32_t _mes_list_ms = 0;
static uint32_t _mes_list_cnt = 0;
/**
  \fn
  \brief
  \return
*/
lv_obj_t *ui_list_screen(PhoneUI_list_t *data)
{
    uint32_t mark = millis();
    // if(data == NULL) {
    //     if(_ui_list != NULL){
    //         if(lv_obj_is_valid(_ui_list)){
    //             lv_obj_clean(_ui_list);
    //             lv_obj_del_async(_ui_list);
    //         }
    //         _ui_list = NULL;
    //     }
    //     goto labelEnd;
    // }
    if(lv_obj_is_valid(_ui_list) && _ui_list != NULL){
        lv_obj_clean(_ui_list);
        lv_obj_del_async(_ui_list);
        _ui_list = NULL;
    }
    if(data == NULL) {
        _mes_list_ms = 0;
        _mes_list_cnt = 0;
        goto labelEnd;
    }
    else if(((data->index)+(data->select))>=data->total || data->select>=MAX_LIST_ITEMS){
        UI_ERR("invalid index %d+%d>=%d",data->index,data->select,data->total);
        goto labelEnd;
    }

    _ui_list = lv_obj_create(NULL);
    lv_obj_clear_flag(_ui_list, LV_OBJ_FLAG_SCROLLABLE); 
    lv_obj_set_style_bg_color(_ui_list, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t * ui_ListTitle = lv_label_create(_ui_list);
    if(data->title != NULL && strlen(data->title) < MAX_TITLE_LENGTH) {
        lv_obj_set_width(ui_ListTitle, LV_SIZE_CONTENT);  
        lv_obj_set_height(ui_ListTitle, 40); 
        lv_label_set_text(ui_ListTitle, data->title);
        lv_obj_align(ui_ListTitle, LV_ALIGN_TOP_MID, 0, 8);
        lv_obj_set_style_bg_color(ui_ListTitle, lv_color_black(), LV_PART_MAIN);
        // lv_obj_set_style_text_font(ui_ListTitle, pLight36, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(ui_ListTitle, lv_color_make(0xff, 0xff, 0xff), LV_PART_MAIN);  
    }
    uint8_t total = (data->total > MAX_LIST_ITEMS) ? MAX_LIST_ITEMS : data->total;

    lv_obj_t * ui_ListPage = lv_list_create(_ui_list);
    lv_obj_set_size(ui_ListPage, LV_PCT(100), 216);
    lv_obj_remove_style(ui_ListPage, NULL, LV_PART_SCROLLBAR);
    lv_obj_align_to(ui_ListPage, ui_ListTitle, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);
    lv_obj_set_style_bg_color(ui_ListPage, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    
    for (uint8_t index = 0; index < total; index++) {
        if (data->context[index] != NULL && strlen(data->context[index]) < LABEL_LENGTH_MAX) {
            lv_obj_t *_ui_list_item = lv_list_add_btn(ui_ListPage, NULL, NULL);
            if(index==(data->select)) lv_obj_set_style_bg_color(_ui_list_item, lv_color_make(0xff, 0, 0), LV_PART_MAIN | LV_STATE_DEFAULT);
            else lv_obj_set_style_bg_color(_ui_list_item, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT); 
            lv_obj_t *_ui_list_text = lv_label_create(_ui_list_item);
            lv_label_set_text(_ui_list_text, data->context[index]);
            lv_obj_set_style_text_align(_ui_list_text, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            // lv_obj_set_style_text_font(_ui_list_text, pLight36, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(_ui_list_text, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    if (data->total > MAX_LIST_ITEMS) {
        lv_obj_t * list_number = lv_label_create(_ui_list);
        lv_obj_set_width(list_number, LV_SIZE_CONTENT);   
        lv_obj_set_height(list_number, LV_SIZE_CONTENT);    
        lv_obj_set_style_text_font(list_number, pSystem16, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(list_number, lv_color_make(0, 0xff, 0), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text_fmt(list_number, "%"LV_PRIu32"/%"LV_PRIu32, ((data->index)+(data->select)+1), data->total);
        lv_obj_align(list_number, LV_ALIGN_TOP_RIGHT, 0, 40);

        lv_obj_t * scrollbar = ui_scrollbar(_ui_list,data->index,data->total,200);
        lv_obj_align(scrollbar, LV_ALIGN_TOP_RIGHT, 0, 64);
    }
    if(data->bottomBtn){
        ui_btn_set(data->bottomBtn,lv_color_white(),lv_color_black());
    }
    lv_disp_load_scr(_ui_list);
labelEnd:
    mark = millis() - mark;
    _mes_list_ms += mark;
    _mes_list_cnt += 1;
    return _ui_list;
}
/**
  \fn
  \brief
  \return
*/
void *ui_list_set(PhoneUI_list_t *data)
{
    GuiMsgT msgPtr;
    if(data == NULL){
        memset(&_ui_list_data,0,sizeof(PhoneUI_list_t));
        msgPtr.ui_data = NULL;
    }
    else if(strlen(data->image)>ADDR_LENGTH_MAX || strlen(data->title)>MAX_TITLE_LENGTH){
        UI_LOG("invalid data --- %d s",millis()/1000);
        return NULL;
    }
    else if(((data->index+data->select) >= data->total) || data->select>=MAX_LIST_ITEMS){
        UI_LOG("invalid index %d,%d,%d",data->index,data->select,data->total);
        return NULL;
    }
    else {
        memset(&_ui_list_data,0,sizeof(PhoneUI_list_t));
        memset(_ui_list_title,0,sizeof(_ui_list_title));
        memset(_ui_list_image,0,sizeof(_ui_list_image));
        memcpy(&_ui_list_data,data,sizeof(PhoneUI_list_t));
        if(data->title != NULL){
            strcpy(_ui_list_title,data->title);
        }
        _ui_list_data.title = _ui_list_title;
        if(data->image != NULL){
            strcpy(_ui_list_image,data->image);
        }
        _ui_list_data.image = _ui_list_image;
        for (uint8_t i = 0; i < MAX_LIST_ITEMS; i++) {
            memset(_ui_list_items[i],0,sizeof(_ui_list_items[i]));
            if (data->context[i] != NULL && strlen(data->context[i])<LABEL_LENGTH_MAX) {
                strcpy(_ui_list_items[i],data->context[i]);
            }
            _ui_list_data.context[i] = _ui_list_items[i];
        }
        msgPtr.ui_data = &_ui_list_data;
    }
    msgPtr.refresh_ms = 10;
    msgPtr.ui_set = ui_list_screen;
    msgPtr.ui_del = NULL;
    guiSendMsg(&msgPtr);
    osDelay(10);        //绘图时间 27(≤29)ms
    return msgPtr.ui_data;
}
/**
  \fn
  \brief
  \return
*/
PLAT_FPSRAM_ZI_CUST fpui_listpop_t _ui_listpop_data;
PLAT_FPSRAM_ZI_CUST char _ui_listpop_items[MAX_LISTPOP_ITEMS][LABEL_LENGTH_MAX+1];
static uint8_t listKeepNum = POP_KEEP_NUM;  //30x1KB
PLAT_FPSRAM_ZI_CUST lv_obj_t *_ui_listp[POP_KEEP_NUM];
static uint32_t mes_listpop_time_ms = 0;
static uint32_t mes_listpop_cnt = 0;
lv_obj_t *ui_list_create(fpui_listpop_t *data) 
{
    uint32_t mark = millis();
    uint8_t now = POP_KEEP_NUM-listKeepNum;
    if(listKeepNum == 0){
        UI_ERR("all list %d used --- %ds",POP_KEEP_NUM,millis()/1000);
        return NULL;
    }
    if(data == NULL) {
        // UI_WRN("[%d]clean 0x%X\t0x%X,0x%X",now,_ui_listp[0],data->screen,lv_scr_act());
        for(int i = 0; i < now; i++){
            uint8_t num = ui_obj_clean(_ui_listp[i]);
            #ifdef UI_TEST_ITEM
            UI_LOG("[%d:%d]clean 0x%X",num,i,_ui_listp[i]);
            #endif
            _ui_listp[i] = NULL;
        }
        listKeepNum = POP_KEEP_NUM;
        now = POP_KEEP_NUM-listKeepNum;
        // goto labelEnd;
        return NULL;
    }
    if(data->screen == NULL || lv_scr_act() != data->screen){
        listKeepNum = POP_KEEP_NUM;
        UI_ERR("invalid screen --- %ds,0x%X != 0x%X(new)",millis()/1000,data->screen,lv_scr_act());
        return NULL;
    }
    else if(data->width > LCD_WIDTH || data->height > LCD_HEIGHT){
        UI_ERR("invalid size --- W%d,H%d",data->width,data->height);
        return NULL;
    }
    else if(data->offsetX < -(LCD_WIDTH/2) || data->offsetY < -(LCD_HEIGHT/2) || \
            data->offsetX > LCD_WIDTH/2 || data->offsetY > LCD_HEIGHT/2){
        UI_ERR("invalid offset --- X%d,Y%d",data->offsetX,data->offsetY);
        return NULL;
    }
    else if(data->overlay){
        listKeepNum -= 1;
        // now = POP_KEEP_NUM-listKeepNum;
        // _ui_listp[now] = NULL;
    }
    else {
        #ifdef UI_TEST_ITEM
        UI_LOG("clean %d --- 0x%X",now,_ui_listp[now]);
        #endif
        ui_obj_clean(_ui_listp[now]);
        _ui_listp[now] = NULL;
    }
    uint8_t total = (data->total > MAX_LISTPOP_ITEMS) ? MAX_LISTPOP_ITEMS : data->total;
    if(lv_obj_is_valid(data->object) && data->object != NULL){
        #ifdef UI_TEST_ITEM
        // UI_LOG("\t---0x%X = 0x%X,0x%X",_ui_listp[now],data->object,data->screen);
        #endif
    }
    else{
        data->object = lv_list_create(data->screen);
        lv_obj_set_width(data->object, data->width);
        lv_obj_set_height(data->object, LV_SIZE_CONTENT);
        lv_obj_align(data->object, LV_ALIGN_CENTER, data->offsetX, data->offsetY);
        lv_obj_set_style_border_width(data->object, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(data->object, lv_color_white(), LV_PART_MAIN);
        lv_obj_set_style_bg_color(data->object, (lv_color_t)data->color, LV_PART_MAIN | LV_STATE_DEFAULT); 
    }

    for (uint8_t index = 0; index < total; index++) {
        if (data->context[index] != NULL && strlen(data->context[index]) < LABEL_LENGTH_MAX) {
            lv_obj_t *_ui_list_item = lv_list_add_btn(data->object, NULL, NULL);
            if(index == (data->select)){
                if(data->color >= 0xF000){
                    lv_obj_set_style_text_color(_ui_list_item, lv_color_make(0,0xff,0), LV_PART_MAIN);
                }
                else lv_obj_set_style_bg_color(_ui_list_item, lv_color_make(0xff,0,0), LV_PART_MAIN);
            }
            else {
                lv_obj_set_style_bg_color(_ui_list_item, (lv_color_t)data->color, LV_PART_MAIN | LV_STATE_DEFAULT);
            }
            lv_obj_t *_ui_list_text = lv_label_create(_ui_list_item);
            lv_label_set_text(_ui_list_text, data->context[index]);
            lv_obj_set_style_text_align(_ui_list_text, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            // lv_obj_set_style_text_font(_ui_list_text, pLight36, LV_PART_MAIN | LV_STATE_DEFAULT);
            if((data->color & 0xF0F0) >= 0xF0F0){
                lv_obj_set_style_text_color(_ui_list_text, lv_color_black(), LV_PART_MAIN);
            } 
            else lv_obj_set_style_text_color(_ui_list_text, lv_color_white(), LV_PART_MAIN);
        }
    }
    _ui_listp[now] = data->object;
labelEnd:
    mark = millis() - mark;
    mes_listpop_time_ms += mark;
    mes_listpop_cnt += 1;
    #ifdef UI_TEST_ITEM
    for(int i = 0; i <= now; i++){
        UI_WRN("[%d] --- 0x%X",i,_ui_listp[i]);
    }
    #endif
    return &_ui_listp[now];
}
/**
  \fn
  \brief
  \return
*/
void *ui_listpop_set(fpui_listpop_t *data)
{
    uint8_t now = POP_KEEP_NUM-listKeepNum;
    if(lv_scr_act() == NULL){
        UI_LOG("invalid screen --- %ds",millis()/1000);
        return NULL;
    }
    if(listKeepNum == 0){
        UI_LOG("listKeepNum > %d --- %ds",POP_KEEP_NUM,millis()/1000);
        return NULL;
    }
    GuiMsgT msgPtr;
    if(data == NULL){
        memset(&_ui_listpop_data,0,sizeof(fpui_listpop_t));
        msgPtr.ui_data = NULL;
    }
    else if(((data->select) >= data->total) || (data->total) < 1 || data->select>= MAX_LISTPOP_ITEMS){
        UI_LOG("invalid index %d/%d",data->select,data->total);
        return NULL;
    }
    else if(data->offsetX < -100 || data->offsetY < -100 || data->offsetX > 100 || data->offsetY > 100){
        UI_LOG("invalid offset --- X%d,Y%d",data->offsetX,data->offsetY);
        return NULL;
    }
    else if(data->width > LCD_WIDTH || data->height > LCD_HEIGHT || data->width < 40 || data->height < 40){
        UI_LOG("invalid size --- W%d,H%d",data->width,data->height);
        return NULL;
    }
    else {
        memset(&_ui_listpop_data,0,sizeof(fpui_listpop_t));
        memcpy(&_ui_listpop_data,data,sizeof(fpui_listpop_t));
        for (uint8_t i = 0; i < MAX_LISTPOP_ITEMS; i++) {
            memset(_ui_listpop_items[i],0,sizeof(_ui_listpop_items[i]));
            if (data->context[i] != NULL && strlen(data->context[i])<LABEL_LENGTH_MAX) {
                strcpy(_ui_listpop_items[i],data->context[i]);
            }
            _ui_listpop_data.context[i] = _ui_listpop_items[i];
        }
        _ui_listpop_data.screen = lv_scr_act();
        msgPtr.ui_data = &_ui_listpop_data;
    }
    msgPtr.refresh_ms = 10; 
    msgPtr.ui_set = ui_list_create;
    msgPtr.ui_del = NULL;
    guiSendMsg(&msgPtr);
    osDelay(10);    //绘图时间 19(≤23)ms
    return &_ui_listpop_data;
}
/**
  \fn
  \brief
  \return
*/
void *ui_list_test(uint8_t start,uint8_t select,uint8_t total,char *title,void *image)
{
    static char list_btn[3][8] = {"确定", "test","返回"};
    static PhoneUI_list_t list_data = {
        .context[0] = "测试1",
        .context[1] = "测试2",
        .context[2] = "测试3",
        .context[3] = "测试4",
        .context[4] = "测试5",
        .context[5] = "测试6",
        .bottomBtn = list_btn,
    };
    if((start + select) >= total || total<1){
        // UI_LOG("%d/%d,%d",start,total,select);
        return NULL;
    }
    list_data.index = start;
    list_data.select = select;
    list_data.total = total;
    list_data.title = title;
    list_data.image = image;
    #ifdef UI_TEST_ITEM
    UI_LOG("%d,\t%d+%d<%d,%s,%s",millis()/1000,start,select,total,title,image);
    #endif
    return ui_list_set(&list_data);
}

/**
  \fn
  \brief
  \return
*/
lv_obj_t *ui_listpop_test(uint8_t total,uint8_t select,int16_t offsetX,int16_t offsetY,uint16_t width,uint16_t height,uint16_t color,bool overlay)
{
    uint8_t now = POP_KEEP_NUM - listKeepNum;
    fpui_listpop_t listpop = {
        .object = NULL,
        .screen = NULL,
        .total = total,
        .select = select,
        .overlay = overlay,
        .offsetX = offsetX,
        .offsetY = offsetY,
        .color = color,
        .width = width,
        .height = height,
        // .context[0] = "测试项1",
        // .context[1] = "测试项2",
        .context[2] = "测试项3",
        .context[3] = "测试项4",
        .context[4] = "测试项5",
    };
    if(select >= total || total<1){
        // UI_ERR("%d/%d,%d",start,total,select);
        return NULL;
    }
    if(offsetX < -100 || offsetY < -100 || offsetX > 100 || offsetY > 100){
        UI_LOG("invalid offset ---%ds,X%d,Y%d",millis()/1000,offsetX,offsetY);
        return NULL;
    }
    if(width > LCD_WIDTH || height > LCD_HEIGHT || width < 40 || height < 40){
        UI_LOG("invalid size ---%ds,W%d,H%d",millis()/1000,width,height);
        return NULL;
    }
    char strbuffer[24] = {0};
    sprintf(strbuffer,"POP-%04d",now);
    listpop.context[0] = strbuffer;
    memset(strbuffer,0,sizeof(strbuffer));
    sprintf(strbuffer,"%d/%d",mes_listpop_time_ms,mes_listpop_cnt);
    listpop.context[1] = strbuffer;
    #ifdef UI_TEST_ITEM
    // uint16_t mes_listpop = (mes_listpop_time_ms)/mes_listpop_cnt;
    UI_LOG("%d/%d,[%d]%d,%d/%d,X%d,Y%d,W%d,H%d,0x%04X",mes_listpop_time_ms,\
    mes_listpop_cnt,now,overlay,select,total,offsetX,offsetY,width,height,color);
    #endif
    return ui_listpop_set(&listpop);
}