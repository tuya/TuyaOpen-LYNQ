
#include "ui.h"

extern uint32_t millis(void);

static lv_obj_t * _ui_list = NULL;
PLAT_FPSRAM_ZI_CUST PhoneUI_list_t _ui_list_data;
PLAT_FPSRAM_ZI_CUST char _ui_list_title[MAX_TITLE_LENGTH+1];
PLAT_FPSRAM_ZI_CUST char _ui_list_image[ADDR_LENGTH_MAX+1];
PLAT_FPSRAM_ZI_CUST char _ui_list_items[MAX_LIST_ITEMS][LABEL_LENGTH_MAX+1];
static uint32_t _mes_list_ms = 0;
static uint32_t _mes_list_cnt = 0;

#include DEBUG_LOG_HEADER_FILE
#define EPAT_TRACE(subId, argLen, format,  ...) ECOMM_TRACE(UNILOG_TEST, subId, P_VALUE, argLen, format,  ##__VA_ARGS__) 
/**
  \fn
  \brief
  \return
*/
lv_obj_t *ui_list_screen(PhoneUI_list_t *data)
{
    uint32_t mark = millis();
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
    msgPtr.ui_set = (void* (*)(void *))ui_list_screen;
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
            num = num;
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
        lv_obj_move_foreground(data->object);
        lv_obj_align(data->object, LV_ALIGN_CENTER, data->offsetX, data->offsetY);
        lv_obj_set_style_border_width(data->object, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(data->object, lv_color_white(), LV_PART_MAIN);
        lv_obj_set_style_bg_color(data->object, (lv_color_t)data->color, LV_PART_MAIN | LV_STATE_DEFAULT); 
        lv_obj_set_style_bg_opa(data->object, LV_OPA_COVER, 0);
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
// labelEnd:
    mark = millis() - mark;
    mes_listpop_time_ms += mark;
    mes_listpop_cnt += 1;
    #ifdef UI_TEST_ITEM
    for(int i = 0; i <= now; i++){
        UI_WRN("[%d] --- 0x%X",i,_ui_listp[i]);
    }
    #endif
    return (lv_obj_t *)&_ui_listp[now];
}
/**
  \fn
  \brief
  \return
*/
void *ui_listpop_set(fpui_listpop_t *data)
{
    // uint8_t now = POP_KEEP_NUM-listKeepNum;
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
    msgPtr.ui_set = (void* (*)(void *))ui_list_create;
    msgPtr.ui_del = NULL;
    osDelay(2);
    guiSendMsg(&msgPtr);
    osDelay(2);    //绘图时间 19(≤23)ms
    return &_ui_listpop_data;
}
/**
  \fn
  \brief
  \return
*/
static lv_obj_t * _ui_imebar = NULL;
static lv_obj_t * _ui_imebar_pinyin = NULL;
static lv_obj_t * _ui_imebar_hanzi = NULL;
AP_PLAT_COMMON_DATA fpui_imebar_t _ui_imebar_data;
lv_obj_t *ui_imebar_create(fpui_imebar_t *data) 
{
    static lv_style_t pinyin_style;
    static lv_style_t hanzi_style;
    static lv_style_t focus_style;
    EPAT_TRACE(ui_imebar_create, 3, "data 0x%X=0x%X, 0x%X", &_ui_imebar_data, data, _ui_imebar);
    /* 初始化样式 */
    if(pinyin_style.prop_cnt == 0){
        lv_style_init(&pinyin_style);
        lv_style_set_pad_all(&pinyin_style, 2);
        lv_style_set_bg_opa(&pinyin_style, LV_OPA_TRANSP);
        lv_style_set_text_font(&pinyin_style, &lv_font_montserrat_16);
        lv_style_set_text_color(&pinyin_style, lv_color_hex(0x333333));
        
        lv_style_init(&hanzi_style);
        lv_style_set_pad_all(&hanzi_style, 2);
        lv_style_set_bg_opa(&hanzi_style, LV_OPA_TRANSP);
        lv_style_set_text_color(&hanzi_style, lv_color_hex(0x444444));

        lv_style_init(&focus_style);
        lv_style_set_radius(&focus_style, 2);
        lv_style_set_bg_color(&focus_style, lv_color_hex(0x0099FF));
        lv_style_set_text_color(&focus_style, lv_color_white());
        lv_style_set_border_width(&focus_style, 2);
        lv_style_set_border_color(&focus_style, lv_color_hex(0x0066CC));
        lv_style_set_bg_opa(&focus_style, LV_OPA_COVER);
    }
    if(data == NULL){
        if(_ui_imebar != NULL && lv_obj_is_valid(_ui_imebar)){
            lv_obj_add_flag(_ui_imebar, LV_OBJ_FLAG_HIDDEN);
        }
        return NULL;
    }
    if(!lv_obj_is_valid(data->screen) && data->screen != NULL){
        UI_LOG("Invalid screen 0x%X",data->screen);
        return NULL;
    }
    /* 设置选中颜色 */
    if(data->highlight < 0xFFFFFF && data->highlight > 0)
    {
        lv_style_set_bg_color(&focus_style, lv_color_hex(data->highlight));
        UI_LOG("select 0x%x,highlight=0x%X",data->select, data->highlight);
    } 
    /* 创建/更新候选栏容器 */
    if(_ui_imebar == NULL){
        _ui_imebar = lv_obj_create(data->screen);
        lv_obj_set_size(_ui_imebar, 240, 60);
        lv_obj_clear_flag(_ui_imebar, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_opa(_ui_imebar, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_border_width(_ui_imebar, 1, LV_PART_MAIN);
        lv_obj_align(_ui_imebar, LV_ALIGN_BOTTOM_MID, 0, 0);
    }
    lv_obj_clear_flag(_ui_imebar, LV_OBJ_FLAG_HIDDEN);
    /* 创建拼音候选栏 */
    if(_ui_imebar_pinyin == NULL){
        _ui_imebar_pinyin = lv_btnmatrix_create(_ui_imebar);
        lv_btnmatrix_set_btn_width(_ui_imebar_pinyin, 0, 48);
        lv_obj_set_size(_ui_imebar_pinyin, 240, 20);
        lv_obj_align(_ui_imebar_pinyin, LV_ALIGN_TOP_LEFT, 0, 0);
        lv_obj_add_style(_ui_imebar_pinyin, &pinyin_style, LV_PART_ITEMS);
        lv_obj_add_style(_ui_imebar_pinyin, &focus_style, LV_PART_ITEMS | LV_STATE_CHECKED);
        lv_btnmatrix_set_one_checked(_ui_imebar_pinyin, true);  // 启用单选模式
    }
    // static uint8_t last_select_type = 0;
    // static uint8_t last_select_idx = 0;
    uint8_t select_type = (data->select & 0x80) ? IME_SELECT_PINYIN : IME_SELECT_HANZI;
    uint8_t select_idx = (data->select & 0x7F) - 1 ;
    /* 更新拼音内容 */
    static char *pinyin_map[MAX_IMEBAR_NUM + 1];
    for(uint8_t i=0; i<MAX_IMEBAR_NUM; i++){
        pinyin_map[i] = (data->pinyin[i][0] != '\0') ? data->pinyin[i] : " ";
        /* 设置选中状态 */
        if(select_type == IME_SELECT_PINYIN && select_idx == i){
            // lv_btnmatrix_set_selected_btn(_ui_imebar_pinyin, select_idx);
            lv_btnmatrix_set_btn_ctrl(_ui_imebar_pinyin, select_idx, LV_BTNMATRIX_CTRL_CHECKED);
        }
        else if(lv_btnmatrix_has_btn_ctrl(_ui_imebar_pinyin, i, LV_BTNMATRIX_CTRL_CHECKED)) {
            lv_btnmatrix_clear_btn_ctrl(_ui_imebar_pinyin, i, LV_BTNMATRIX_CTRL_CHECKED);
        }
    }
    pinyin_map[MAX_IMEBAR_NUM] = "";
    lv_btnmatrix_set_map(_ui_imebar_pinyin, (const char **)pinyin_map);

    /* 创建汉字候选栏 */
    if(_ui_imebar_hanzi == NULL){
        _ui_imebar_hanzi = lv_btnmatrix_create(_ui_imebar);
        lv_btnmatrix_set_btn_width(_ui_imebar_hanzi, 0, 48);
        lv_obj_set_size(_ui_imebar_hanzi, 240, 40);
        lv_obj_align(_ui_imebar_hanzi, LV_ALIGN_BOTTOM_LEFT, 0, 0);
        lv_obj_add_style(_ui_imebar_hanzi, &hanzi_style, LV_PART_ITEMS);
        // lv_obj_add_style(_ui_imebar_hanzi, &focus_style, LV_PART_ITEMS | LV_STATE_FOCUSED);
        lv_obj_add_style(_ui_imebar_hanzi, &focus_style, LV_PART_ITEMS | LV_STATE_CHECKED);
        lv_btnmatrix_set_one_checked(_ui_imebar_hanzi, true);  // 启用单选模式
    }

    /* 更新汉字内容 */
    static char *hanzi_map[MAX_IMEBAR_NUM + 1];
    for(uint8_t i=0; i<MAX_IMEBAR_NUM; i++){
        hanzi_map[i] = (data->hanzi[i][0] != '\0') ? data->hanzi[i] : " ";
        if(select_type == IME_SELECT_HANZI && select_idx == i){
            lv_btnmatrix_set_btn_ctrl(_ui_imebar_hanzi, select_idx, LV_BTNMATRIX_CTRL_CHECKED);
        }
        else if(lv_btnmatrix_has_btn_ctrl(_ui_imebar_hanzi, i, LV_BTNMATRIX_CTRL_CHECKED)) {
            lv_btnmatrix_clear_btn_ctrl(_ui_imebar_hanzi, i, LV_BTNMATRIX_CTRL_CHECKED);
        }
    }
    hanzi_map[MAX_IMEBAR_NUM] = "";
    lv_btnmatrix_set_map(_ui_imebar_hanzi, (const char **)hanzi_map);
    // uint16_t ret_id = lv_btnmatrix_get_selected_btn(_ui_imebar_pinyin);
    // EPAT_TRACE(ui_imebar_create_end, 4, "id=0x%x,select=%d,%s,%s",ret_id ,select_idx, pinyin_map[select_idx-1], hanzi_map[select_idx-1]);
    // UI_LOG("id=0x%x,select=%d,%s,%s",ret_id ,select_idx, pinyin_map[select_idx-1], hanzi_map[select_idx-1]);
    // if (lv_obj_has_state(_ui_imebar_pinyin,LV_STATE_FOCUSED)) UI_LOG("pinyin focus %d,%s",select_idx-1, pinyin_map[select_idx-1]);
    return _ui_imebar;
}

/**
  \fn ui_imebar_set
  \brief 设置输入法候选栏内容
  \param type 输入类型 0:拼音 1:汉字 2:隐藏
  \param index 候选位置索引(1~MAX_IMEBAR_NUM) select为0不选中任何项
  \param p_pinyin 拼音候选数组指针
  \param p_hanzi 汉字候选数组指针
  \param color 高亮颜色值
  \return 操作结果指针
*/
void *ui_imebar_set(uint8_t type, uint8_t index, uint32_t color, char (*p_pinyin)[MAX_IMEBAR_STRLEN], char (*p_hanzi)[MAX_IMEBAR_STRLEN])
{
    GuiMsgT msgPtr;
    // UI_LOG("type %d,index %d,%s,%s,color=0x%x",type,index,p_pinyin[3],p_hanzi[3],color);
    /* 参数有效性验证 */
    if(_ui_imebar == NULL){
        memset(&_ui_imebar_data, 0, sizeof(fpui_imebar_t));
    }
    if(type >= IME_INVALID || index > MAX_IMEBAR_NUM){
        UI_LOG("Invalid type:%u or index:%u", type, index);
        return NULL;
    }
    else if(type == IME_INPUT_EXIT){
        if(_ui_imebar != NULL && lv_obj_is_valid(_ui_imebar)){
            lv_obj_add_flag(_ui_imebar, LV_OBJ_FLAG_HIDDEN);
        }
        return NULL;
        // msgPtr.ui_data = NULL;
    }
    else 
    {
        for(uint8_t i = 0; i < MAX_IMEBAR_NUM; i++){
            if(p_pinyin != NULL){
                // 处理拼音数据
                memset(_ui_imebar_data.pinyin[i],0,MAX_IMEBAR_STRLEN);
                if(p_pinyin != NULL && (strlen(p_pinyin[i]) < MAX_IMEBAR_STRLEN)){
                    strncpy(_ui_imebar_data.pinyin[i], p_pinyin[i], MAX_IMEBAR_STRLEN-1);
                }
            }
            if(p_hanzi != NULL){
                // 处理汉字数据
                memset(_ui_imebar_data.hanzi[i],0,MAX_IMEBAR_STRLEN);
                if(p_hanzi != NULL && (strlen(p_hanzi[i]) < MAX_IMEBAR_STRLEN)){
                    strncpy(_ui_imebar_data.hanzi[i], p_hanzi[i], MAX_IMEBAR_STRLEN-1);
                }
            }
        }
        /* 设置选中状态 */
        _ui_imebar_data.select = (type == IME_SELECT_PINYIN) ? 
            (index | 0x80) :  // 拼音选中标志位最高位置1
            (index & 0x7F);   // 汉字选中标志位最高位清零
        
        _ui_imebar_data.highlight = color;
        _ui_imebar_data.object = _ui_imebar;
        _ui_imebar_data.screen = lv_layer_sys();    //lv_scr_act();
        msgPtr.ui_data = &_ui_imebar_data;
        UI_LOG("select 0x%X, color=0x%X", _ui_imebar_data.select, color);
        EPAT_TRACE(ui_imebar_set, 2, "select 0x%X, color=0x%X", _ui_imebar_data.select, color);
    }
    msgPtr.refresh_ms = 10; 
    msgPtr.ui_set = (void* (*)(void *))ui_imebar_create;
    msgPtr.ui_del = NULL;
    guiSendMsg(&msgPtr);
    osDelay(1);
    return &_ui_imebar_data;
}
/**
  \fn
  \brief
  \return
*/
/**
 * \brief 输入法候选栏测试入口
 * \param 
 */
void ui_imebar_test(ui_test_case_t case_type)
{
    char pinyin[MAX_IMEBAR_NUM][MAX_IMEBAR_STRLEN] = {0};
    char hanzi[MAX_IMEBAR_NUM][MAX_IMEBAR_STRLEN] = {0};
    // uint16_t color = 0x0;

    switch((uint32_t)case_type) {
    case TEST_CASE_NORMAL: // 正常情况
        strncpy(pinyin[0], "ni", sizeof(pinyin[0])-1);
        strncpy(pinyin[1], "hao", sizeof(pinyin[1])-1);
        strncpy(pinyin[2], "yi", sizeof(pinyin[2])-1);
        strncpy(pinyin[3], "xin", sizeof(pinyin[3])-1);
        strncpy(hanzi[0], "你", sizeof(hanzi[0])-1);
        strncpy(hanzi[1], "好", sizeof(hanzi[1])-1);
        strncpy(hanzi[2], "移", sizeof(hanzi[2])-1);
        strncpy(hanzi[3], "芯", sizeof(hanzi[3])-1);
        ui_imebar_set(
            IME_SELECT_PINYIN,  // 类型：拼音
            2,  // 选中
            0x00FF00,
            pinyin,
            hanzi
        );
        memset(pinyin, 0, sizeof(pinyin)); // 清空旧数据
        memset(hanzi, 0, sizeof(hanzi));
        strncpy(pinyin[0], "yi", sizeof(pinyin[0])-1);
        strncpy(pinyin[1], "xin", sizeof(pinyin[1])-1);
        strncpy(pinyin[2], "shu", sizeof(pinyin[2])-1);
        strncpy(pinyin[3], "ru", sizeof(pinyin[3])-1);
        strncpy(pinyin[4], "fa", sizeof(pinyin[4])-1);
        strncpy(hanzi[0], "移", sizeof(hanzi[0])-1);
        strncpy(hanzi[1], "芯", sizeof(hanzi[1])-1);
        strncpy(hanzi[2], "输", sizeof(hanzi[2])-1);
        strncpy(hanzi[3], "入", sizeof(hanzi[3])-1);
        strncpy(hanzi[4], "法", sizeof(hanzi[4])-1);
        osDelay(2000);
        ui_imebar_set(IME_SELECT_HANZI, 1, 0xFF0000, pinyin, hanzi);
        break;
    case TEST_CASE_BUFFER_OVERFLOW: // 超长字符串测试
        memset(pinyin[0], 'a', MAX_IMEBAR_STRLEN+5);
#if 0 // 此处编译报错，猜测是复制“测”字到hanzi[0]，但这样写达不到效果，遂改成memcpy，不知猜测是否正确，所以保留原文
        memset(hanzi[0], '测', MAX_IMEBAR_STRLEN+5);
#else
        memcpy(hanzi[0], "测", strlen("测"));
#endif
        break;

    case TEST_CASE_BOUNDARY_VALUE: // 边界值测试
        for(int i=0; i<MAX_IMEBAR_NUM; i++){
            snprintf(pinyin[i], MAX_IMEBAR_STRLEN-1, "py%d", i+1);
            snprintf(hanzi[i], MAX_IMEBAR_STRLEN-1, "汉%d", i+1);
        }
        break;

    case TEST_CASE_INVALID_PARAM: // 非法参数测试
        ui_imebar_set(2, 0, 0, NULL, 0); // 错误type
        ui_imebar_set(0, MAX_IMEBAR_NUM+1, 0, NULL, 0); // 越界index
        return;
    }
    osDelay(3000);
    ui_imebar_set(IME_INPUT_EXIT, 0, 0, NULL, NULL);
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
void ui_listpop_test(ui_test_case_t case_type)
{
    static fpui_listpop_t listpop = {
        .object = NULL,
        .screen = NULL,
        .total = 3,
        .select = 1,
        .overlay = true,
        .offsetX = 0,
        .offsetY = 0,
        .color = 0x0,
        .width = 200,
        .height = 80,
        .context[0] = "测试项1",
        .context[1] = "测试项2",
        .context[2] = "测试项3",
        .context[3] = "测试项4",
        .context[4] = "测试项5",
    };
    switch((uint32_t)case_type) {
        case TEST_CASE_NORMAL:
            ui_listpop_set(&listpop);
            break;
        case TEST_CASE_BOUNDARY_VALUE:
            listpop.select = 1;
            ui_listpop_set(&listpop);
            osDelay(1000);
            listpop.select = 2;
            ui_listpop_set(&listpop);
            osDelay(500);
            listpop.select = 0;
            ui_listpop_set(&listpop);
            osDelay(500);
            listpop.select = 1;
            ui_listpop_set(&listpop);
            osDelay(500);
            listpop.select = 2;
            ui_listpop_set(&listpop);
            osDelay(500);
            listpop.select = 0;
            ui_listpop_set(&listpop);
            break;
        case TEST_CASE_ITEM_SELECTION:
            osDelay(100);
            listpop.select = 1;
            listpop.offsetX = -40;
            listpop.offsetY = -40;
            listpop.width = 160;
            listpop.height = 120;
            listpop.color = 0x00F1;
            // listpop.overlay = false;
            ui_listpop_set(&listpop);
            osDelay(100);
            listpop.select = 0;
            listpop.offsetX = 20;
            listpop.offsetY = 20;
            listpop.width = 160;
            listpop.height = 120;
            // listpop.overlay = false;
            listpop.color = 0xF100;
            ui_listpop_set(&listpop);
            // osDelay(500);
            break;
        case TEST_CASE_UI_CLOSE:
            // ui_listpop_set(NULL);
            break;
        default:
            break;
    }
}