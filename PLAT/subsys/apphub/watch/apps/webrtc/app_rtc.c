
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#include "ui.h"
#include "app_rtc.h"
#include DEBUG_LOG_HEADER_FILE
// #define EPAT_TRACE(subId, argLen, format, ...) ECOMM_TRACE(UNILOG_JRTC, subId, P_VALUE, argLen, format, ##__VA_ARGS__)
extern void sendQueueMsg(UINT32 msgId, UINT32 xTickstoWait);
lv_timer_t *test_timer = NULL;
lv_timer_t *talk_timer = NULL;
ui_rtc_obj_t *watch_rtc = NULL;
lv_obj_t *mid_btn_label = NULL;
/**
  \fn
  \brief
  \return
*/
static uint16_t time_data = 0;
static void timer_callback(lv_timer_t * timer) 
{
    if (lv_obj_has_flag(watch_rtc->stimer, LV_OBJ_FLAG_HIDDEN)) {
        time_data = 0;
    }
    else if(lv_obj_is_valid(watch_rtc->stimer)){
        time_data += 1;
        lv_label_set_text_fmt(watch_rtc->stimer, "%02d:%02d", time_data/60, time_data%60);
    }
}
/**
  \fn
  \brief
  \return
*/
void ec_rtc_usrlist(void* para)
{
    if(watch_rtc != NULL && lv_obj_is_valid(watch_rtc->screen)){
        lv_obj_clear_flag(watch_rtc->list, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(watch_rtc->header, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(watch_rtc->stimer, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(watch_rtc->contrl, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(watch_rtc->image, LV_OBJ_FLAG_HIDDEN);
        if (test_timer){
            lv_timer_del(test_timer);
            test_timer = NULL;
        } 
    }
}
/**
  \fn
  \brief
  \return
*/
void ec_rtc_talking(char *uid)
{
    lv_obj_add_flag(watch_rtc->list, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(watch_rtc->header, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(watch_rtc->header, uid);
    lv_obj_clear_flag(watch_rtc->stimer, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(watch_rtc->stimer, "00:00");
    time_data = 0;
    if (talk_timer) lv_timer_resume(talk_timer);
    lv_obj_clear_flag(watch_rtc->contrl, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(watch_rtc->bottom[0], LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(watch_rtc->bottom[2], LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(watch_rtc->bottom[1], LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(watch_rtc->image, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(mid_btn_label, "通话中");
    lv_obj_set_style_bg_color(watch_rtc->bottom[1], lv_color_hex(0xFF3B30), 0);
}
/**
  \fn
  \brief    用于模拟用户自动接听的流程
  \return
*/
void test_dialout_callback(lv_timer_t * timer) 
{
    char *name = (char *)timer->user_data;
    ec_rtc_talking(name);
    if (test_timer){
        lv_timer_del(test_timer);
        test_timer = NULL;
    } 
}
/**
  \fn
  \brief
  \return
*/
static void ec_rtc_dialout(char *uid)
{
    lv_obj_add_flag(watch_rtc->list, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(watch_rtc->header, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(watch_rtc->header, uid);
    lv_obj_clear_flag(watch_rtc->stimer, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(watch_rtc->stimer, "00:00");
    time_data = 0;
    if (talk_timer) lv_timer_resume(talk_timer);
    lv_obj_add_flag(watch_rtc->image, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(watch_rtc->contrl, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(watch_rtc->bottom[0], LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(watch_rtc->bottom[2], LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(watch_rtc->bottom[1], LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(watch_rtc->bottom[1], lv_color_hex(0x666666), 0);
    lv_label_set_text(mid_btn_label, "呼叫中");
    sendQueueMsg(12345, 0);
    //模拟对方操作的定时器
    // test_timer = lv_timer_create(test_dialout_callback, 5000, "eigencomm");
}
/**
  \fn
  \brief
  \return
*/
static void ec_rtc_ringing(char *uid)
{
    lv_obj_add_flag(watch_rtc->list, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(watch_rtc->header, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(watch_rtc->header, uid);
    lv_obj_clear_flag(watch_rtc->stimer, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(watch_rtc->stimer, "ring");
    if (talk_timer) lv_timer_pause(talk_timer);
    lv_obj_clear_flag(watch_rtc->contrl, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(watch_rtc->bottom[0], LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(watch_rtc->bottom[2], LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(watch_rtc->bottom[1], LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(watch_rtc->image, LV_OBJ_FLAG_HIDDEN);
    // lv_label_set_text(mid_btn_label, "挂断");
}
/**
  \fn
  \brief
  \return
*/
void ec_rtc_accept(void* uid)
{
    ec_rtc_talking(uid);
}
/**
  \fn
  \brief
  \return
*/
extern void jc_rtc_hangup(void);
void ec_rtc_hangup(void* para)
{
    ec_rtc_usrlist(NULL);
    jc_rtc_hangup();
}
/**
  \fn
  \brief    
  \return
*/
static void user_btn_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    if(target == watch_rtc->bottom[0]) {
        ec_rtc_accept("test");  //来电接听按键-左
    }
    else if(target == watch_rtc->bottom[1]) {
        // if (test_timer){
        //     ec_rtc_ringing("incoming"); //主动发起按键-中
        //     lv_timer_del(test_timer);
        //     test_timer = NULL;
        // } 
        // else 
        ec_rtc_hangup(NULL);
    }
    else if(target == watch_rtc->bottom[2]) {
        ec_rtc_hangup(NULL);    //挂断后返回列表    来电挂断按键-右
    }
    SYSLOG_INFO("code:%d\r\n", code);
}
/**
  \fn
  \brief
  \return
*/
void ui_event_screen_webrtc_cb(lv_event_t *e) 
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT)
    {
        if (lv_obj_has_flag(watch_rtc->list, LV_OBJ_FLAG_HIDDEN)) {
            ec_rtc_usrlist(NULL);
        }
        else{
            lv_scr_load_anim(ui_appListScreen, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
        }
    }
    else if (code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_TOP){

    }
    else if (code == LV_EVENT_SCREEN_LOADED) {

    }
}
/**
  \fn
  \brief
  \return
*/
static void contact_click_cb(lv_event_t * e)
{
    lv_obj_t * btn = lv_event_get_target(e);

    const char * name = lv_list_get_btn_text(watch_rtc->list, btn);
    if(watch_rtc->contact_selected_cb) {
        watch_rtc->contact_selected_cb(name);
    }
}

// 修改联系人按钮样式和交互
void add_contact_to_list(const char *name)
{
    lv_obj_t * btn = lv_list_add_btn(watch_rtc->list, LV_SYMBOL_CALL, name);
    
    // 基础样式
    lv_obj_set_style_text_font(btn, &lv_font_montserrat_30, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x333333), 0);
    lv_obj_set_style_text_color(btn, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(btn, 8, 0);  // 圆角
    
    // 边框样式
    lv_obj_set_style_border_color(btn, lv_color_hex(0x666666), 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_opa(btn, LV_OPA_100, 0);
    
    lv_obj_set_style_bg_color(btn, lv_color_darken(lv_color_hex(0x333333), 50), LV_STATE_PRESSED);

    // 添加点击事件
    lv_obj_add_event_cb(btn, contact_click_cb, LV_EVENT_CLICKED, NULL);
}
/**
  \fn
  \brief
  \return
*/
static void init_contact_list(lv_obj_t *parent)
{
    watch_rtc->list = lv_list_create(parent);
    lv_obj_set_size(watch_rtc->list, LV_PCT(98), LV_PCT(95)); // 留出边框空间
    lv_obj_align(watch_rtc->list, LV_ALIGN_TOP_MID, 0, 5);
    
    // 列表容器样式
    lv_obj_set_style_bg_color(watch_rtc->list, lv_color_hex(0x1a1a1a), 0);
    lv_obj_set_style_pad_all(watch_rtc->list, 5, 0);
    lv_obj_set_style_radius(watch_rtc->list, 10, 0);
    
    // 列表边框
    lv_obj_set_style_border_color(watch_rtc->list, lv_color_hex(0x4CAF50), 0);
    lv_obj_set_style_border_width(watch_rtc->list, 2, 0);
    lv_obj_set_style_border_opa(watch_rtc->list, LV_OPA_80, 0);

    // 添加示例联系人（实际开发中应从数据库读取）
    add_contact_to_list("zhang");
    add_contact_to_list("sister");
    add_contact_to_list("brother");
}

/**
  \fn
  \brief
  \return
*/
static void init_talking_page(lv_obj_t *parent)
{
    watch_rtc->header = lv_label_create(parent);
    lv_obj_align(watch_rtc->header, LV_ALIGN_TOP_MID, 0, 4);
    lv_obj_set_style_text_align(watch_rtc->header, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_size(watch_rtc->header, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(watch_rtc->header, LV_OPA_TRANSP, 0);
    lv_obj_set_style_text_color(watch_rtc->header, lv_color_white(), 0);
    lv_obj_set_style_text_font(watch_rtc->header, &lv_font_montserrat_20, 0);
    lv_label_set_text(watch_rtc->header, " ");
    lv_obj_add_flag(watch_rtc->header, LV_OBJ_FLAG_HIDDEN);
    // 在头部添加计时标签
    watch_rtc->stimer = lv_label_create(parent);
    lv_label_set_text(watch_rtc->stimer, "00:00");
    lv_obj_set_size(watch_rtc->stimer, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(watch_rtc->stimer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_text_color(watch_rtc->stimer, lv_color_make(0, 0xff, 0), 0);
    lv_obj_align_to(watch_rtc->stimer, watch_rtc->header, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);
    lv_obj_add_flag(watch_rtc->stimer, LV_OBJ_FLAG_HIDDEN);
    talk_timer = lv_timer_create(timer_callback, 1000, NULL);
    lv_timer_pause(talk_timer);

    // 创建底部按钮容器
    watch_rtc->contrl = lv_obj_create(parent);
    lv_obj_remove_style_all(watch_rtc->contrl);
    lv_obj_set_size(watch_rtc->contrl, LV_PCT(80), LV_SIZE_CONTENT);
    lv_obj_align(watch_rtc->contrl, LV_ALIGN_BOTTOM_MID, 0, -8);
    lv_obj_set_style_bg_opa(watch_rtc->contrl, LV_OPA_TRANSP, 0);
    lv_obj_add_flag(watch_rtc->contrl, LV_OBJ_FLAG_HIDDEN);

    // 接听按钮（左）
    watch_rtc->bottom[0] = lv_btn_create(watch_rtc->contrl);
    lv_obj_set_size(watch_rtc->bottom[0], 80, 40);
    lv_obj_align(watch_rtc->bottom[0], LV_ALIGN_LEFT_MID, 8, 0);
    lv_obj_set_style_bg_color(watch_rtc->bottom[0], lv_color_hex(0x007AFF), 0); // 蓝色
    lv_obj_set_style_border_color(watch_rtc->bottom[0], lv_color_white(), 0);
    lv_obj_set_style_border_width(watch_rtc->bottom[0], 2, 0);
    lv_obj_set_style_radius(watch_rtc->bottom[0], 10, 0);
    lv_obj_t *accept_label = lv_label_create(watch_rtc->bottom[0]);
    lv_label_set_text(accept_label, "接听");
    // lv_label_set_text(accept_label, LV_SYMBOL_CALL);
    lv_obj_center(accept_label);
    lv_obj_add_event_cb(watch_rtc->bottom[0], user_btn_cb, LV_EVENT_CLICKED, NULL);

    // 结束按钮（中）
    watch_rtc->bottom[1] = lv_btn_create(watch_rtc->contrl);
    lv_obj_set_size(watch_rtc->bottom[1], 80, 40);
    lv_obj_align(watch_rtc->bottom[1], LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(watch_rtc->bottom[1], lv_color_hex(0x666666), 0); // 灰色
    lv_obj_set_style_border_color(watch_rtc->bottom[1], lv_color_white(), 0);
    lv_obj_set_style_border_width(watch_rtc->bottom[1], 1, 0);
    lv_obj_set_style_radius(watch_rtc->bottom[1], 12, 0);
    mid_btn_label = lv_label_create(watch_rtc->bottom[1]);
    lv_label_set_text(mid_btn_label, "结束");
    lv_obj_center(mid_btn_label);
    lv_obj_add_event_cb(watch_rtc->bottom[1], user_btn_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(watch_rtc->bottom[1], LV_OBJ_FLAG_HIDDEN);

    // 挂断按钮（右）
    watch_rtc->bottom[2] = lv_btn_create(watch_rtc->contrl);
    lv_obj_set_size(watch_rtc->bottom[2], 80, 40);
    lv_obj_align(watch_rtc->bottom[2], LV_ALIGN_RIGHT_MID, -8, 0);
    lv_obj_set_style_bg_color(watch_rtc->bottom[2], lv_color_hex(0xFF3B30), 0);
    lv_obj_set_style_border_color(watch_rtc->bottom[2], lv_color_white(), 0);
    lv_obj_set_style_border_width(watch_rtc->bottom[2], 1, 0);
    lv_obj_set_style_radius(watch_rtc->bottom[2], 12, 0);
    lv_obj_t *reject_label = lv_label_create(watch_rtc->bottom[2]);
    lv_label_set_text(reject_label, "挂断");
    lv_obj_center(reject_label);
    lv_obj_add_event_cb(watch_rtc->bottom[2], user_btn_cb, LV_EVENT_CLICKED, NULL);
    
    // 在按钮样式中添加状态变化
    lv_obj_set_style_bg_color(watch_rtc->bottom[0], lv_color_darken(lv_color_hex(0x007AFF), 30), LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(watch_rtc->bottom[1], lv_color_darken(lv_color_hex(0x666666), 30), LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(watch_rtc->bottom[2], lv_color_darken(lv_color_hex(0xFF3B30), 30), LV_STATE_PRESSED);

}
/**
  \fn
  \brief
  \return
*/
void ui_webrtc_screen_init(void)
{
    if(watch_rtc == NULL){
        watch_rtc = (ui_rtc_obj_t *)pvPortZeroAssertMallocCust(sizeof(ui_rtc_obj_t));
        watch_rtc->parent = NULL;
    }
    watch_rtc->screen = lv_obj_create(watch_rtc->parent);
    lv_obj_set_size(watch_rtc->screen, LV_HOR_RES, LV_VER_RES);
    lv_obj_clear_flag(watch_rtc->screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_style_all(watch_rtc->screen);
    lv_obj_set_style_bg_opa(watch_rtc->screen, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(watch_rtc->screen, lv_color_black(), 0);
    lv_obj_add_event_cb(watch_rtc->screen, ui_event_screen_webrtc_cb, LV_EVENT_ALL, NULL);
    watch_rtc->image = lv_img_create(watch_rtc->screen); 
    // lv_obj_set_size(watch_rtc->image, LV_PCT(100), LV_PCT(100));
    if(ui_jpg_img_dsc.data == NULL){
        ui_jpg_img_dsc.data = pvPortZeroAssertMallocCust(2*240*240);
        ui_jpg_img_dsc.header.w = 240;
        ui_jpg_img_dsc.header.h = 240;
    }
    lv_img_set_src(watch_rtc->image, &ui_jpg_img_dsc);
    lv_obj_set_width(watch_rtc->image, LV_SIZE_CONTENT);
    lv_obj_set_height(watch_rtc->image, LV_SIZE_CONTENT);
    // 初始化联系人列表
    init_contact_list(watch_rtc->screen);
    watch_rtc->contact_selected_cb = ec_rtc_dialout;

    init_talking_page(watch_rtc->screen);
}
/**
  \fn
  \brief
  \return
*/
void webrtc_screen_load(uint8_t from)
{
    if(watch_rtc != NULL && lv_obj_is_valid(watch_rtc->screen)){
        switch (from)
        {
        case 1:
            lv_scr_load_anim(watch_rtc->screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
            break;
        default:
            lv_scr_load_anim(watch_rtc->screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
            break;
        }
    }
}
/**
  \fn
  \brief
  \return
*/
#include "hal_video.h"
extern uint8_t ui_rgb_buf[];
void lcd_show(struct jrtc_image_t*image)
{
    if (lv_obj_has_flag(watch_rtc->image, LV_OBJ_FLAG_HIDDEN)) {

    }
    else if(lv_obj_is_valid(watch_rtc->image)){
        if(ui_jpg_img_dsc.data != NULL){
            // ui_jpg_img_dsc.header.cf = LV_IMG_CF_TRUE_COLOR;
            // ui_jpg_img_dsc.header.w = image->width;
            // ui_jpg_img_dsc.header.h = image->height;
            // ui_jpg_img_dsc.data = (void*)image->data;
            Video_ScaleImageRgb565( image->data, image->width, image->height, ui_jpg_img_dsc.data, 240, 240);
            ui_jpg_img_dsc.header.w = 240;
            ui_jpg_img_dsc.header.h = 240;
            lv_img_set_src(watch_rtc->image, &ui_jpg_img_dsc);
            lv_obj_invalidate(watch_rtc->image);
            // uint32_t imgsize = lv_load_jpg("D:/face_75_2_dial_dial_img_0_1024_data_0.jpg",&ui_jpg_img_dsc);
        }
    }
}