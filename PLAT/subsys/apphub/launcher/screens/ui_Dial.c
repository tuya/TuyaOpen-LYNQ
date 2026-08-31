
#include "../ui.h"
#include <stdio.h>
#include <stdlib.h>
#include "ostask.h"
#include "osasys.h"
#include "bsp.h"
#include "bsp_custom.h"

#define APP_TRACE(subId, argLen, format,  ...)  \
    ECOMM_TRACE(UNILOG_REF_APP, subId, P_VALUE, argLen, format,  ##__VA_ARGS__) 


extern char phone_text[20];
extern int phone_text_index;

lv_obj_t *ui_Dial;
lv_obj_t *ui_Dial_container;
lv_obj_t *dial_text;
lv_obj_t *dial_btnm;

static const char *btn_dial_list[20] = {
    "1", "2", "3", "\n",
    "4", "5", "6", "\n",
    "7", "8", "9", "\n",
    "*", "0", "#", "\n",
    LV_SYMBOL_CALL, LV_SYMBOL_STOP, LV_SYMBOL_BACKSPACE};

static void dial_btn_event_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    uint16_t id = lv_btnmatrix_get_selected_btn(obj);
    char* txt = lv_btnmatrix_get_btn_text(obj,id);
    if(code == LV_EVENT_CLICKED)
    {
        if(id >=0 && id<12)
        {
            NumberProc(txt[0]);
        }
        else if(id == 12) //dialout
        {
            if(phone_text_index>2 && code)
            {
                #ifdef FEATURE_IMS_ENABLE
                phoneCall(phone_text);
                #endif
                lv_label_set_text(ui_phone_number,phone_text);
            } 
        }
        else if(id == 13) //hang up
        {
            #ifdef FEATURE_IMS_ENABLE
            phoneHangUp();
            #endif
            NumberDelProc(phone_text_index);
        }
        else if(id == 14)
        {
            NumberDelProc(1);
        }
    }
    // lv_label_set_text(dial_text, phone_text);
    // APP_TRACE(dial_btn_event_cb, 4,"0x%X,%d,%d,%s",code,id,phone_text_index,phone_text);
}



void ui_Dial_screen_init(void)
{
    ui_Dial = lv_obj_create(NULL);
    lv_obj_clear_flag( ui_Dial, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_obj_set_style_bg_img_src( ui_Dial, &ui_img_pattern_png, LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_img_tiled(ui_Dial, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_Dial_container = lv_obj_create(ui_Dial);
    // lv_obj_set_width( ui_Dial_container, lv_pct(100));
    // lv_obj_set_height( ui_Dial_container, lv_pct(100));
    // lv_obj_set_align( ui_Dial_container, LV_ALIGN_CENTER );
    // lv_obj_set_style_bg_color(ui_Dial_container, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    // lv_obj_set_style_bg_opa(ui_Dial_container, 0, LV_PART_MAIN| LV_STATE_DEFAULT);

    dial_text = lv_label_create(ui_Dial);
    lv_obj_set_style_text_font(dial_text, &lv_font_montserrat_34, 0);
    lv_label_set_text(dial_text, phone_text);
    lv_obj_set_size(dial_text, LV_PCT(100), LV_PCT(10));
    lv_obj_set_pos(dial_text, 12, 40);
    // lv_obj_align(dial_text, LV_ALIGN_CENTER, 0, 40);
    // lv_obj_add_flag(dial_text, LV_OBJ_FLAG_HIDDEN);
    // lv_obj_clear_flag(dial_text, LV_OBJ_FLAG_SCROLLABLE);   

    
    dial_btnm = lv_btnmatrix_create(ui_Dial);
    lv_btnmatrix_set_map(dial_btnm, btn_dial_list);
    lv_obj_set_size(dial_btnm, LV_PCT(100), LV_PCT(75));
    // lv_obj_align(dial_btnm, LV_ALIGN_CENTER, 0, 55);
    lv_obj_align(dial_btnm,LV_ALIGN_BOTTOM_MID,0,0);
    // lv_btnmatrix_set_btn_ctrl(dial_btnm,12,LV_BTNMATRIX_CTRL_CHECKABLE);
    // lv_btnmatrix_set_btn_ctrl(dial_btnm,16,LV_BTNMATRIX_CTRL_CHECKABLE);
    lv_obj_add_event_cb(dial_btnm, dial_btn_event_cb,LV_EVENT_CLICKED,NULL);
    // lv_obj_add_flag(dial_btnm, LV_OBJ_FLAG_HIDDEN); 
    // lv_obj_clear_flag(dial_btnm, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_add_event_cb(ui_Dial, ui_event_Dial, LV_EVENT_ALL, NULL);

}
extern void kpcInputHandle(bool entry);

void ui_event_Dial(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    // if(event_code<LV_EVENT_COVER_CHECK && event_code>LV_EVENT_DRAW_PART_END) APP_TRACE(ui_event_Dial, 1,"%d",event_code);
    if ( event_code == LV_EVENT_SCREEN_LOADED) 
    {
        // Up_Animation(ui_Dial_date, 100);
        // Up_Animation(ui_C1, 200);
        // Up_Animation(ui_C2, 300);
        // Up_Animation(net_label, 10);
        // Up_Animation(dial_text, 100);
        // Up_Animation(dial_btnm, 300); 
        // phone_text_index=0;
        // memset(phone_text,0,sizeof(phone_text));
        lv_obj_clear_flag(dial_btnm, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(dial_text, LV_OBJ_FLAG_HIDDEN);
        // scrolldot_Animation(ui_Scrolldots2, 0);
    }
    if ( event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT) 
    {
        lv_indev_wait_release(lv_indev_get_act());
        kpcInputHandle(false);
        _ui_screen_change(&ui_Menu1, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Menu1_screen_init); 
    }
    else if( event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_LEFT) 
    {
        lv_indev_wait_release(lv_indev_get_act());
        kpcInputHandle(false);
        _ui_screen_change(&ui_Menu1, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Menu1_screen_init); 
    }
    else if ( event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_TOP ) 
    {
        lv_indev_wait_release(lv_indev_get_act());
        lv_obj_clear_flag(dial_btnm, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(dial_text, LV_OBJ_FLAG_HIDDEN);
    }
    else if ( event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_BOTTOM ) 
    {
        lv_indev_wait_release(lv_indev_get_act());
        lv_obj_add_flag(dial_btnm, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(dial_text, LV_OBJ_FLAG_HIDDEN); 
    }
}