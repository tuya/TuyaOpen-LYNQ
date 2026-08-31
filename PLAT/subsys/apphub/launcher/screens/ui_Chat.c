
#include "../ui.h"
#include <stdio.h>
#include <stdlib.h>
#include "ostask.h"
#include "osasys.h"
#include "bsp.h"
#include "bsp_custom.h"
#include "mode_config.h"
#include "sctdef.h"

lv_obj_t *ui_Chat = NULL;
lv_obj_t *ui_Chat_container;
lv_obj_t *ui_edit_container;

lv_obj_t *ui_sms_address;

lv_obj_t *ui_sms_rx = NULL;
lv_obj_t *ui_sms_rx_date = NULL;
lv_obj_t *ui_sms_rx_dock = NULL;
lv_obj_t *ui_sms_rx_text = NULL;
lv_obj_t *ui_sms_tts_btn = NULL;
lv_obj_t *ui_sms_rx_icon = NULL;
lv_obj_t *ui_sms_rx_size = NULL;

lv_obj_t *ui_sms_tx = NULL;
lv_obj_t *ui_sms_tx_date = NULL;
lv_obj_t *ui_sms_tx_dock = NULL;
lv_obj_t *ui_sms_tx_text = NULL;
lv_obj_t *ui_sms_tx_icon = NULL;
lv_obj_t *ui_sms_tx_flag = NULL;
lv_obj_t *ui_sms_keyboard;
lv_obj_t *sms_input;
lv_obj_t *send_btn;

static lv_style_t style_input;
static uint16_t sms_rx_panel_height = 120;

static uint8_t ucs_default[] = {0xFB,0x79,0xAF,0x82,0x4B,0x6D,0xD5,0x8B,0x8C,0x9A,0xC1,0x8B,0x01,0x78,0x31,0x00,0x32,0x00,0x33,0x00,0x34,0x00,0x35,0x00,0x36,0x00,0x37,0x00,0x38,0x00,0x39,0x00,0x30,0x00};
static uint8_t utf_default[] = {0xE7,0xA7,0xBB,0xE8,0x8A,0xAF,0xE6,0xB5,0x8B,0xE8,0xAF,0x95,0xE9,0xAA,0x8C,0xE8,0xAF,0x81,0xE7,0xA0,0x81,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x30};
static uint8_t text_default[] = "1234567890ABCDEFGH";
// static sms_data_t sms_show_buffer = {0};
sms_data_t sms_show_buffer = {
    .user = "10086xxxx",
    .date = "24/01/19,16:00:00",
    .text = "测试短信验证码:123456ABCD",
    .length = 18,
};

#ifdef FEATURE_IMS_ENABLE
extern PLAT_FPSRAM_ZI_CUST sms_data_t sms_mem_buffer[SMS_BUFF_NUM];
// extern UINT16 Ucs2ToUtf8(UINT8 *ucs2, UINT16 ucs2_size, UINT8 *utf8, UINT16 utf8_size);
int sms_get(uint8_t col, sms_data_t *sms)
{
    // uint16_t length = SMS_SIZE_MAX;
    if (sms_mem_buffer[col].length == 0)
    {
        // memcpy(sms->user,"10086",sizeof("10086"));
        // memcpy(sms->number,"10086",sizeof("10086"));
        // memcpy(sms->date,"24/01/19,16:00:00",sizeof("24/01/19,16:00:00"));
        // sms->length = Ucs2ToUtf8(ucs_default,sizeof(ucs_default),sms->text,&length);
        // ECPLAT_DUMP(UNILOG_MISC, Ucs2ToUtf8, P_INFO, "Ucs2ToUtf8:",length,sms->text);
    }
    else 
    {
        memset(sms,0,sizeof(sms_data_t));
        memcpy(sms, &sms_mem_buffer[col], sizeof(sms_data_t));
    }
    return sms->length;
}
int sms_load(int rank)
{
    sms_get(rank, &sms_show_buffer);
    // sms_rx_panel_height = 40 + 20 * (1 + / 10);
    if(ui_sms_rx_size) lv_label_set_text_fmt(ui_sms_rx_size, "%"LV_PRIu32,sms_show_buffer.length);
    // lv_obj_set_height( ui_sms_rx,sms_rx_panel_height);  
    if(ui_sms_address) lv_label_set_text(ui_sms_address,sms_show_buffer.user);
    if(ui_sms_rx_date) lv_label_set_text(ui_sms_rx_date,sms_show_buffer.date);
    if(ui_sms_rx_text) lv_label_set_text(ui_sms_rx_text,sms_show_buffer.text);
    lv_obj_update_layout(ui_Chat);
    // lv_label_set_text(ui_sms_rx_text,utf_default); //sms_show_buffer.text
    lv_obj_set_style_text_font(ui_sms_rx_text, &ui_font_song16, 0);
}
#endif
void ui_event_Chat(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    if ( event_code == LV_EVENT_SCREEN_LOADED) 
    {
        // // sms_rx_panel_height = 40 + 20 * (1 + / 10);
        // if(ui_sms_rx_size) lv_label_set_text_fmt(ui_sms_rx_size, "%"LV_PRIu32,sms_show_buffer.length);
        // // lv_obj_set_height( ui_sms_rx,sms_rx_panel_height);  
        // if(ui_sms_address) lv_label_set_text(ui_sms_address,sms_show_buffer.user);
        // if(ui_sms_rx_date) lv_label_set_text(ui_sms_rx_date,sms_show_buffer.date);
        // if(ui_sms_rx_text) lv_label_set_text(ui_sms_rx_text,sms_show_buffer.text);
        // lv_obj_update_layout(ui_Chat);
        // // lv_label_set_text(ui_sms_rx_text,utf_default); //sms_show_buffer.text
        // lv_obj_set_style_text_font(ui_sms_rx_text, &ui_font_song16, 0);
    }
    if ( event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT) 
    {
        lv_indev_wait_release(lv_indev_get_act());
        _ui_screen_change(&ui_Menu1, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Menu1_screen_init); 
    }
    if ( event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_LEFT) 
    {
        lv_indev_wait_release(lv_indev_get_act());
        _ui_screen_change(&ui_Menu1, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Menu1_screen_init); 
    }
}


static void kb_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    lv_obj_t * kb = lv_event_get_user_data(e);
    if(code == LV_EVENT_FOCUSED) {
        if(lv_indev_get_type(lv_indev_get_act()) != LV_INDEV_TYPE_KEYPAD) {
            lv_keyboard_set_textarea(kb, ta);
            lv_obj_set_style_max_height(kb, LV_HOR_RES * 2 / 3, 0);
            lv_obj_set_pos( ui_edit_container, 0, 288-lv_obj_get_height(kb));
            // lv_obj_update_layout(ui_Chat);
            // lv_obj_set_height(ui_Chat, LV_VER_RES - lv_obj_get_height(kb));
            lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
            lv_obj_scroll_to_view_recursive(ta, LV_ANIM_OFF);
        }
    }
    else if(code == LV_EVENT_DEFOCUSED) {
        lv_keyboard_set_textarea(kb, NULL);
        // lv_obj_set_height(ui_Chat, LV_VER_RES);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos( ui_edit_container, 0, 288);
        lv_indev_reset(NULL, ta);
    }
    else if(code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        // lv_obj_set_height(ui_Chat, LV_VER_RES);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos( ui_edit_container, 0, 288);
        lv_obj_clear_state(ta, LV_STATE_FOCUSED);
        lv_indev_reset(NULL, ta);  
    }
}
static void ttsCallback(void)
{
    lv_obj_set_style_text_color(ui_sms_rx_text, lv_color_hex(0x293062), LV_PART_MAIN | LV_STATE_DEFAULT );
}
static void sms_event_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    if(ta->user_data==1)
    {
        #ifdef FEATURE_SUBSYS_TTS_ENABLE
        LV_LOG_USER(sms_show_buffer.text);
        audioPlayTts(sms_show_buffer.text, ttsCallback);
        #endif
    }
    else if(ta->user_data==2)
    {
        // lv_obj_set_style_text_color(ui_sms_rx_text, lv_color_hex(0x293062), LV_PART_MAIN | LV_STATE_DEFAULT );
    }
    else if(ta->user_data==3)
    {
        if(ui_sms_tx_text) lv_label_set_text(ui_sms_tx_text,text_default);
        #ifdef FEATURE_IMS_ENABLE
        extern int ec_smsSend(UINT8 *number, UINT8 *content, int timeout);
        ec_smsSend(sms_show_buffer.user, text_default,0);
        #endif
        // lv_obj_set_style_text_font(ui_sms_tx_text, &ui_font_song16, LV_PART_MAIN| LV_STATE_DEFAULT);
        // lv_obj_clear_flag(ui_sms_tx, LV_OBJ_FLAG_HIDDEN);
        ui_sms_tx_create(ui_Chat_container);
    }
    // lv_label_set_text(dial_text, phone_text);
}

void ui_sms_rx_create(lv_obj_t * container)
{
    ui_sms_rx = lv_obj_create(container);
    lv_obj_set_width( ui_sms_rx, lv_pct(100));
    lv_obj_set_height( ui_sms_rx, sms_rx_panel_height); //> ui_sms_rx_dock+35
    lv_obj_align_to(ui_sms_rx, ui_sms_address, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);
    lv_obj_clear_flag( ui_sms_rx, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_obj_set_style_bg_color(ui_sms_rx, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(ui_sms_rx, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_sms_rx, 12, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_sms_rx, 12, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_sms_rx, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_sms_rx, 0, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_sms_rx_date = ui_Small_Label_create(ui_sms_rx);
    lv_obj_align(ui_sms_rx_date, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_label_set_text(ui_sms_rx_date,sms_show_buffer.date);
    lv_obj_set_style_text_color(ui_sms_rx_date, lv_color_hex(0x293062), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(ui_sms_rx_date, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_sms_rx_date, &lv_font_montserrat_14, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_sms_rx_dock = lv_obj_create(ui_sms_rx);
    lv_obj_set_height( ui_sms_rx_dock,sms_rx_panel_height-35);  
    lv_obj_set_width( ui_sms_rx_dock, lv_pct(90));
    lv_obj_align(ui_sms_rx_dock, LV_ALIGN_TOP_LEFT, 0, 20);
    lv_obj_clear_flag( ui_sms_rx_dock, LV_OBJ_FLAG_SCROLLABLE );  
    lv_obj_set_style_radius(ui_sms_rx_dock, 12, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_sms_rx_dock, lv_color_hex(0x9C9CD9), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(ui_sms_rx_dock, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_sms_rx_dock, 8, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_sms_rx_dock, 8, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_sms_rx_dock, 8, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_sms_rx_dock, 8, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_sms_tts_btn = lv_label_create(ui_sms_rx);
    lv_obj_set_size(ui_sms_tts_btn, 16, 16);
    lv_obj_set_style_text_font(ui_sms_tts_btn , &lv_font_montserrat_14, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_label_set_text(ui_sms_tts_btn , LV_SYMBOL_VOLUME_MAX);
    lv_obj_add_flag( ui_sms_tts_btn, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_ADV_HITTEST );   /// Flags
    lv_obj_clear_flag(ui_sms_tts_btn, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_obj_align_to(ui_sms_tts_btn , ui_sms_rx_dock , LV_ALIGN_OUT_RIGHT_MID, 4, 0);
    lv_obj_set_user_data(ui_sms_tts_btn, 1);
    lv_obj_add_event_cb(ui_sms_tts_btn, sms_event_cb, LV_EVENT_CLICKED, NULL);

    ui_sms_rx_text = ui_Small_Label_create(ui_sms_rx_dock);
    lv_obj_set_width( ui_sms_rx_text, lv_pct(100));
    lv_obj_set_height( ui_sms_rx_text, LV_SIZE_CONTENT); 
    lv_obj_align(ui_sms_rx_text, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_label_set_text(ui_sms_rx_text,utf_default); //sms_show_buffer.text
    lv_obj_set_style_text_color(ui_sms_rx_text, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(ui_sms_rx_text, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_sms_rx_text, &ui_font_song16, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_sms_rx_icon = lv_img_create(ui_sms_rx);
    lv_img_set_src(ui_sms_rx_icon, &ui_img_chatbox_png);
    lv_obj_set_width( ui_sms_rx_icon, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height( ui_sms_rx_icon, LV_SIZE_CONTENT);   /// 1
    lv_obj_align_to(ui_sms_rx_icon, ui_sms_rx_dock, LV_ALIGN_OUT_BOTTOM_LEFT, 10, 0);
    lv_obj_add_flag( ui_sms_rx_icon, LV_OBJ_FLAG_ADV_HITTEST );   /// Flags
    lv_obj_clear_flag( ui_sms_rx_icon, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_obj_set_style_img_recolor(ui_sms_rx_icon, lv_color_hex(0x9C9CD9), LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor_opa(ui_sms_rx_icon, 255, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_sms_rx_size = ui_Small_Label_create(ui_sms_rx);
    lv_obj_align_to(ui_sms_rx_size, ui_sms_rx_icon, LV_ALIGN_OUT_RIGHT_MID, 2, 2);
    lv_label_set_text_fmt(ui_sms_rx_size, "%"LV_PRIu32, sms_show_buffer.length);
    lv_obj_set_style_text_color(ui_sms_rx_size, lv_color_hex(0x293062), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(ui_sms_rx_size, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_sms_rx_size, &lv_font_montserrat_12, LV_PART_MAIN| LV_STATE_DEFAULT);
}

void ui_sms_tx_create(lv_obj_t * container)
{
    ui_sms_tx = lv_obj_create(container);
    lv_obj_set_height( ui_sms_tx, 120);
    lv_obj_set_width( ui_sms_tx, lv_pct(100));
    lv_obj_align_to(ui_sms_tx, ui_sms_rx, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_obj_clear_flag( ui_sms_tx, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_obj_set_style_bg_color(ui_sms_tx, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(ui_sms_tx, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_sms_tx, 12, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_sms_tx, 12, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_sms_tx, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_sms_tx, 0, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_sms_tx_date = ui_Small_Label_create(ui_sms_tx);
    lv_obj_align(ui_sms_tx_date, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_label_set_text(ui_sms_tx_date,"24/01/19,16:00:00");
    lv_obj_set_style_text_color(ui_sms_tx_date, lv_color_hex(0x293062), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(ui_sms_tx_date, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_sms_tx_date, &lv_font_montserrat_14, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_sms_tx_dock = lv_obj_create(ui_sms_tx);
    lv_obj_set_height( ui_sms_tx_dock, 85);
    lv_obj_set_width( ui_sms_tx_dock, lv_pct(90));
    lv_obj_align(ui_sms_tx_dock, LV_ALIGN_TOP_RIGHT, 0, 20);
    lv_obj_set_align( ui_sms_tx_dock, LV_ALIGN_TOP_RIGHT );
    lv_obj_clear_flag( ui_sms_tx_dock, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_obj_set_style_radius(ui_sms_tx_dock, 12, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_sms_tx_dock, lv_color_hex(0x293062), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(ui_sms_tx_dock, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_sms_tx_dock, 8, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_sms_tx_dock, 8, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_sms_tx_dock, 8, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_sms_tx_dock, 8, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_sms_tx_text = ui_Small_Label_create(ui_sms_tx_dock);
    lv_obj_set_width( ui_sms_tx_text, lv_pct(100));
    lv_obj_set_height( ui_sms_tx_text, LV_SIZE_CONTENT);   
    // lv_obj_set_x( ui_sms_tx_text, 0 );
    // lv_obj_set_y( ui_sms_tx_text, 0 );
    lv_obj_align(ui_sms_tx_text, LV_ALIGN_TOP_LEFT, 4, 4);
    // lv_obj_set_align( ui_sms_tx_text, LV_ALIGN_TOP_LEFT );
    lv_label_set_text(ui_sms_tx_text,text_default);
    lv_obj_set_style_text_font(ui_sms_tx_text, &ui_font_song16, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_sms_tx_text, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(ui_sms_tx_text, 255, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_sms_tx_icon = lv_img_create(ui_sms_tx);
    lv_img_set_src(ui_sms_tx_icon, &ui_img_chatbox2_png);
    lv_obj_set_width( ui_sms_tx_icon, LV_SIZE_CONTENT); 
    lv_obj_set_height( ui_sms_tx_icon, LV_SIZE_CONTENT); 

    lv_obj_align_to(ui_sms_tx_icon, ui_sms_tx_dock, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 0);
    // lv_obj_set_align( ui_sms_tx_icon, LV_ALIGN_TOP_RIGHT );
    lv_obj_add_flag( ui_sms_tx_icon, LV_OBJ_FLAG_ADV_HITTEST );   /// Flags
    lv_obj_clear_flag( ui_sms_tx_icon, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_obj_set_style_img_recolor(ui_sms_tx_icon, lv_color_hex(0x293062), LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor_opa(ui_sms_tx_icon, 255, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_sms_tx_flag = ui_Small_Label_create(ui_sms_tx);
    lv_obj_align_to(ui_sms_tx_flag, ui_sms_tx_dock, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_label_set_text(ui_sms_tx_flag,"已发送");
    lv_obj_set_style_text_color(ui_sms_tx_flag, lv_color_hex(0x9C9CD9), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(ui_sms_tx_flag, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_sms_tx_flag, &ui_font_song16, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_add_flag(ui_sms_tx_flag, LV_OBJ_FLAG_HIDDEN);
    // lv_obj_add_flag(ui_sms_tx, LV_OBJ_FLAG_HIDDEN);
}

// lv_obj_t *ime_pinyin;
// void ime_pinyin_test(lv_obj_t* main)
// {
//     lv_obj_t * cz_label = lv_label_create(main);
//     lv_label_set_text(cz_label,"\n上海移芯通信科技有限公司\nwww.eigencomm.com");
//     lv_obj_set_style_text_font(cz_label, &ui_font_song16, 0);
//     lv_obj_set_width(cz_label, LV_PCT(90));
//     lv_obj_align(cz_label, LV_ALIGN_TOP_MID, 0, 20);

//     lv_obj_t * ta1 = lv_textarea_create(main);
//     lv_textarea_set_one_line(ta1, true);
//     lv_obj_set_style_text_font(ta1, &ui_font_song16, 0);
//     // lv_obj_set_size(ta1, LV_PCT(160), LV_PCT(16));
//     lv_obj_set_width(ta1, LV_PCT(90));
//     // lv_obj_set_pos(ta1, 20, 20);
//     lv_textarea_set_placeholder_text(ta1, "请输入");
//     lv_obj_align(ta1, LV_ALIGN_TOP_LEFT, 12, 60);

//     /*Create a keyboard and add it to ime_pinyin*/
//     lv_obj_t * kb = lv_keyboard_create(main);
//     lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    
//     lv_ime_pinyin_set_keyboard(ime_pinyin, kb);
//     lv_keyboard_set_textarea(kb, ta1);
//     lv_obj_add_event_cb(ta1, kb_event_cb, LV_EVENT_ALL, kb);
// }

void ui_sms_edit_create(lv_obj_t * container)
{
    // lv_style_init(&style_input);
    // lv_style_set_radius(&style_input, 15);
    // lv_style_set_bg_opa(&style_input, LV_OPA_10);

    ui_edit_container = lv_obj_create(container);
    lv_obj_set_width( ui_edit_container, lv_pct(100));
    lv_obj_set_height( ui_edit_container, lv_pct(15));
    // lv_obj_set_x( ui_edit_container, 0 );
    // lv_obj_set_y( ui_edit_container, 288 );
    lv_obj_set_pos( ui_edit_container, 0, 288);
    lv_obj_set_style_bg_color(ui_edit_container, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(ui_edit_container, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
    // lv_obj_align_to(ui_edit_container, ui_Chat_container, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    ui_sms_keyboard = lv_keyboard_create(container);
    lv_obj_add_flag(ui_sms_keyboard, LV_OBJ_FLAG_HIDDEN);

    sms_input = lv_textarea_create(ui_edit_container);
    // lv_obj_add_style(sms_input, &style_input, 0);
    // lv_obj_set_width( sms_input, lv_pct(85));
    // lv_obj_set_height(sms_input, lv_pct(100));
    lv_obj_set_size(sms_input, 200, 40);
    // lv_obj_set_align( sms_input, LV_ALIGN_TOP_LEFT );
    // lv_obj_align(sms_input, LV_ALIGN_TOP_LEFT, 2, 2);
    // lv_obj_set_x( sms_input, 0 );
    // lv_obj_set_y( sms_input, 288 );
    lv_obj_set_pos( sms_input, 0, 0);
    // lv_obj_set_height( user_name, 60);
    // lv_textarea_set_one_line(sms_input, true);
    lv_textarea_set_placeholder_text(sms_input, text_default);
    // lv_obj_set_style_text_font(sms_input, &ui_font_song16, 0);
    // lv_obj_set_pos();
    // lv_obj_align_to(sms_input, send_btn, LV_ALIGN_OUT_LEFT_MID, 0, 0);
    lv_obj_add_event_cb(sms_input, kb_event_cb, LV_EVENT_ALL, ui_sms_keyboard);
    // lv_obj_set_style_bg_color(send_btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    // lv_obj_set_style_bg_opa(send_btn, 0, LV_PART_MAIN| LV_STATE_DEFAULT);

    send_btn = lv_btn_create(ui_edit_container);
    lv_obj_set_size(send_btn, 40, 40);
    lv_obj_align_to(send_btn, sms_input, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    // lv_obj_set_x( ui_edit_container, 208 );
    // lv_obj_set_y( ui_edit_container, 288 );
    //  lv_obj_add_style(send_btn, &style_input, 0);
    lv_obj_set_style_bg_img_src(send_btn, LV_SYMBOL_GPS, 0);
    // lv_obj_align(send_btn, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_set_user_data(send_btn, 3);
    lv_obj_add_event_cb(send_btn, sms_event_cb, LV_EVENT_CLICKED, NULL);

    // lv_obj_set_style_bg_color(send_btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    // lv_obj_set_style_bg_opa(send_btn, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
    // lv_obj_t * user_name_label = lv_label_create(ui_edit_container);
    // lv_label_set_text(ui_edit_container, "send SMS");
    // lv_obj_add_style(user_name_label, &style_text_muted, 0);
}

void ui_Chat_screen_init(void)
{
    ui_Chat = lv_obj_create(NULL);
    lv_obj_clear_flag( ui_Chat, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_obj_set_style_bg_img_src( ui_Chat, &ui_img_pattern_png, LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_img_tiled(ui_Chat, true, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_Chat_container = lv_obj_create(ui_Chat);
    lv_obj_set_width( ui_Chat_container, lv_pct(100));
    lv_obj_set_height( ui_Chat_container, lv_pct(80));
    lv_obj_set_align( ui_Chat_container, LV_ALIGN_CENTER );
    lv_obj_set_style_bg_color(ui_Chat_container, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(ui_Chat_container, 0, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_sms_address = ui_Small_Label_create(ui_Chat_container);
    lv_obj_align(ui_sms_address, LV_ALIGN_TOP_MID, 0, 0);
    lv_label_set_text(ui_sms_address,sms_show_buffer.user);
    lv_obj_set_style_text_color(ui_sms_address, lv_color_hex(0x000746), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(ui_sms_address, 255, LV_PART_MAIN| LV_STATE_DEFAULT);

    ui_sms_rx_create(ui_Chat_container);
    // ui_sms_tx_create(ui_Chat_container);
    ui_sms_edit_create(ui_Chat);
    #ifdef FEATURE_IMS_ENABLE
    sms_load(0);
    #endif
    // lv_obj_add_flag(ui_sms_tx_flag, LV_OBJ_FLAG_HIDDEN);
    // lv_obj_add_flag(ui_sms_tx, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(ui_Chat, ui_event_Chat, LV_EVENT_ALL, NULL);
}

