
#include "ui.h"

extern uint32_t millis(void);

static lv_obj_t *_ui_player = NULL;
static lv_obj_t *_ui_player_label = NULL;
static lv_obj_t *_ui_player_image = NULL;
PLAT_FPSRAM_ZI_CUST fpui_player_t _ui_player_data;
PLAT_FPSRAM_ZI_CUST char _fpui_player_image[ADDR_LENGTH_MAX+1];
PLAT_FPSRAM_ZI_CUST char _fpui_player_label[MAX_TITLE_LENGTH+1];
PLAT_FPSRAM_ZI_CUST char _fpui_player_btn[3][8];
/**
  \fn
  \brief
  \return
*/
lv_obj_t *ui_play_screen(fpui_player_t *data)
{
    if(lv_obj_is_valid(data->object) && data->object != NULL){
        _ui_player = data->object;
    }
    else if(_ui_player != NULL && lv_obj_is_valid(_ui_player)){
        lv_obj_clean(_ui_player);
        lv_obj_del_async(_ui_player);
        _ui_player = NULL;
    }
    if(data == NULL) {
        goto labelEnd;
    }
    if(_ui_player == NULL){
        _ui_player = lv_obj_create(NULL);
        lv_obj_clear_flag(_ui_player, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(_ui_player, lv_color_make(0xff, 0xff, 0xff), LV_PART_MAIN | LV_STATE_DEFAULT );
        lv_obj_set_style_bg_opa(_ui_player, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t *ui_backward = lv_img_create(_ui_player);
        lv_img_set_src(ui_backward, LV_IMAGE_41);
        lv_obj_set_width(ui_backward, LV_SIZE_CONTENT);
        lv_obj_set_height(ui_backward, LV_SIZE_CONTENT);
        lv_obj_set_x(ui_backward, 30);
        lv_obj_set_y(ui_backward, -90);
        lv_obj_set_align(ui_backward, LV_ALIGN_BOTTOM_LEFT);
        lv_obj_add_flag(ui_backward, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_ADV_HITTEST);
        lv_obj_clear_flag(ui_backward, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_img_recolor(ui_backward, lv_color_hex(0x293062), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_img_recolor_opa(ui_backward, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_img_recolor(ui_backward, lv_color_hex(0x515EB5), LV_PART_MAIN | LV_STATE_PRESSED);
        lv_obj_set_style_img_recolor_opa(ui_backward, 255, LV_PART_MAIN | LV_STATE_PRESSED);

        lv_obj_t *ui_forward = lv_img_create(_ui_player);
        lv_img_set_src(ui_forward, LV_IMAGE_42);
        lv_obj_set_width(ui_forward, LV_SIZE_CONTENT); 
        lv_obj_set_height(ui_forward, LV_SIZE_CONTENT);
        lv_obj_set_x(ui_forward, -30);
        lv_obj_set_y(ui_forward, -90);
        lv_obj_set_align(ui_forward, LV_ALIGN_BOTTOM_RIGHT);
        lv_obj_add_flag(ui_forward, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_ADV_HITTEST);
        lv_obj_clear_flag(ui_forward, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_img_recolor(ui_forward, lv_color_hex(0x293062), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_img_recolor_opa(ui_forward, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_img_recolor(ui_forward, lv_color_hex(0x515EB5), LV_PART_MAIN | LV_STATE_PRESSED);
        lv_obj_set_style_img_recolor_opa(ui_forward, 255, LV_PART_MAIN | LV_STATE_PRESSED);

        lv_obj_t *ui_play_btn = lv_obj_create(_ui_player);
        lv_obj_set_width(ui_play_btn, 60);
        lv_obj_set_height(ui_play_btn, 60);
        lv_obj_set_x(ui_play_btn, 0);
        lv_obj_set_y(ui_play_btn, -72);
        lv_obj_set_align(ui_play_btn, LV_ALIGN_BOTTOM_MID);
        lv_obj_clear_flag(ui_play_btn, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_radius(ui_play_btn, 60, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(ui_play_btn, lv_color_hex(0x293062), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_play_btn, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_color(ui_play_btn, lv_color_hex(0x293062), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_opa(ui_play_btn, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_width(ui_play_btn, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_spread(ui_play_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_ofs_x(ui_play_btn, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_ofs_y(ui_play_btn, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(ui_play_btn, lv_color_hex(0x515EB5), LV_PART_MAIN | LV_STATE_PRESSED);
        lv_obj_set_style_bg_opa(ui_play_btn, 255, LV_PART_MAIN | LV_STATE_PRESSED);

        lv_obj_t *ui_play_img = lv_img_create(ui_play_btn);
        lv_img_set_src(ui_play_img, LV_IMAGE_43);
        lv_obj_set_width(ui_play_img, LV_SIZE_CONTENT);
        lv_obj_set_height(ui_play_img, LV_SIZE_CONTENT);
        lv_obj_set_x(ui_play_img, 2);
        lv_obj_set_y(ui_play_img, 0);
        lv_obj_set_align(ui_play_img, LV_ALIGN_CENTER);
        lv_obj_add_flag(ui_play_img, LV_OBJ_FLAG_ADV_HITTEST); 
        lv_obj_clear_flag(ui_play_img, LV_OBJ_FLAG_SCROLLABLE);
    }
    if(lv_obj_is_valid(_ui_player) && data->image != NULL && strlen(data->image) < ADDR_LENGTH_MAX){
        if(_ui_player_image != NULL && lv_obj_is_valid(_ui_player_image)){
            // lv_obj_clean(_ui_player_image);
            lv_obj_del(_ui_player_image);
        }
        _ui_player_image = lv_img_create(_ui_player);
        lv_img_set_src(_ui_player_image, data->image);
        lv_obj_set_width(_ui_player_image, LV_SIZE_CONTENT);
        lv_obj_set_height(_ui_player_image, LV_SIZE_CONTENT);
        lv_obj_set_x(_ui_player_image, 0);
        lv_obj_set_y(_ui_player_image, 20);
        lv_obj_set_align(_ui_player_image, LV_ALIGN_TOP_MID);
        lv_obj_add_flag(_ui_player_image, LV_OBJ_FLAG_ADV_HITTEST); 
        lv_obj_clear_flag(_ui_player_image, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_radius(_ui_player_image, 300, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_color(_ui_player_image, lv_color_hex(0xD5D2D5), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_opa(_ui_player_image, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_width(_ui_player_image, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_spread(_ui_player_image, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_ofs_x(_ui_player_image, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_ofs_y(_ui_player_image, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    if(lv_obj_is_valid(_ui_player) && data->title != NULL && strlen(data->title) < MAX_TITLE_LENGTH) {
        if(_ui_player_label != NULL && lv_obj_is_valid(_ui_player_label)){
            // lv_obj_clean(_ui_player_label);
            lv_obj_del(_ui_player_label);
        } 
        _ui_player_label = lv_label_create(_ui_player);
        lv_obj_set_x(_ui_player_label, 0);
        lv_obj_set_y(_ui_player_label, 0);
        lv_obj_set_align(_ui_player_label, LV_ALIGN_CENTER);
        lv_label_set_text(_ui_player_label, data->title);
        lv_obj_set_style_text_color(_ui_player_label, lv_color_hex(0x000746), LV_PART_MAIN | LV_STATE_DEFAULT );
        lv_obj_set_style_text_opa(_ui_player_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        // lv_obj_set_style_text_font(_ui_player_label, pLight36, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    if(data->bottomBtn != NULL && strlen(data->bottomBtn[0]) <= sizeof(data->bottomBtn[0])) {
        ui_btn_set(data->bottomBtn,lv_color_black(),lv_color_white());
    }
    lv_disp_load_scr(_ui_player);
labelEnd:
    // osEventFlagsSet(uiEvtHandle, (1U << PAGE_APP));
    data->object = _ui_player;
    return _ui_player;
}

/**
  \fn
  \brief
  \return
*/
void *ui_player_set(fpui_player_t *data)
{
    GuiMsgT msgPtr;
    if(data == NULL){
        memset(&_ui_player_data,0,sizeof(fpui_player_t));
        msgPtr.ui_data = NULL;
        // if(lv_obj_is_valid(_ui_player)) lv_obj_add_flag(_ui_player, LV_OBJ_FLAG_HIDDEN);
    }
    else if(strlen(data->image)>ADDR_LENGTH_MAX || strlen(data->title)>MAX_TITLE_LENGTH){
        UI_LOG("invalid data --- %d s",millis()/1000);
        return NULL;
    }
    else {
        memset(&_ui_player_data,0,sizeof(fpui_player_t));
        memcpy(&_ui_player_data,data,sizeof(fpui_player_t));
        memset(_fpui_player_label,0,sizeof(_fpui_player_label));
        if(data->title != NULL){
            strcpy(_fpui_player_label,data->title);
        }
        _ui_player_data.title = _fpui_player_label;
        memset(_fpui_player_image,0,sizeof(_fpui_player_image));
        if(data->image != NULL){
            strcpy(_fpui_player_image,data->image);
        }
        _ui_player_data.image = _fpui_player_image;
        memset(_fpui_player_btn,0,sizeof(_fpui_player_btn));
        if(data->bottomBtn != NULL && strlen(data->bottomBtn[0])<8){
            memcpy(_fpui_player_btn,data->bottomBtn,sizeof(_fpui_player_btn));
        }
        _ui_player_data.bottomBtn = _fpui_player_btn;
        msgPtr.ui_data = &_ui_player_data;
        // UI_LOG("%d-%s-%s,%d-%s-%s",strlen(data->title),data->title,_ui_player_data.title,strlen(data->image),data->image,_ui_player_data.image);
    }
    msgPtr.refresh_ms = 10;
    msgPtr.ui_set = (void* (*)(void *))ui_play_screen;
    msgPtr.ui_del = NULL;
    // osEventFlagsClear(uiEvtHandle, (1U << PAGE_APP));
    guiSendMsg(&msgPtr);
    osDelay(5);
    // osEventFlagsWait(uiEvtHandle, (1U << PAGE_APP), osFlagsWaitAll, 20);    //<18ms
    return &_ui_player_data;
}
/**
  \fn
  \brief
  \return
*/
void *ui_player_test(char *title, void *image)
{
    static char app_btn[3][8] = {"列表","test","返回"};
    fpui_player_t play_test = {
        .object = _ui_player,
        .title = title,
        .image = image,
        .bottomBtn = app_btn,
    };
    #ifdef UI_TEST_ITEM
    UI_LOG("%d,\t%s,%s",millis()/1000,title,image);
    #endif
    return ui_player_set(&play_test);
}