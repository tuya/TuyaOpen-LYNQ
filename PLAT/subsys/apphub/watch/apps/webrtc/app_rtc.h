#ifndef __APP_RTC_H__
#define __APP_RTC_H__
#include "lvgl.h"
#include "ui.h"

typedef struct {
    lv_obj_t *parent;
    lv_obj_t *screen;
    lv_obj_t *image;
    lv_obj_t *list;     // 联系人列表
    void (*contact_selected_cb)(char *uid);
    lv_obj_t *header;
	lv_obj_t *stimer;
    lv_obj_t *window;
	lv_obj_t *contrl;
    lv_obj_t *bottom[3];
    lv_style_t normal;
} ui_rtc_obj_t;
extern ui_rtc_obj_t *watch_rtc;

void ui_webrtc_screen_init(void);
void webrtc_screen_load(uint8_t from);
#endif
