#ifndef _UI_EVENTS_H
#define _UI_EVENTS_H

#include <stdint.h>
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    char numstr[20];
    char name[32];
    char icon[32];
    char button[16];
} ui_label_t;


// 定义回调函数类型
typedef void (*task_callback_t)(void *arg);
// 定义链表节点结构
typedef struct task_node {
    lv_obj_t *obj;        
    task_callback_t callback;
    void *arg;
    uint32_t period;    //ms
    struct task_node *next;
} task_node_t;

void* ui_coTask_init(void *para, void *task);
void ui_coTask_stop(void);  //task sleep
uint32_t ui_app_loop(uint32_t timeout);
void task_list_print(void);
void task_list_add(lv_obj_t *obj, task_callback_t callback, void *arg, uint32_t period) ;
void task_list_remove(task_callback_t callback) ;
void onLoadHome(lv_event_t * e);
void onClickAlert(lv_event_t * e);
void onForecastOpen(lv_event_t * e);
void onWeatherLoad(lv_event_t * e);
void onNotificationsOpen(lv_event_t * e);
void onBrightnessChange(lv_event_t * e);
void onScrollMode(lv_event_t * e);
void onTimeoutChange(lv_event_t * e);
void onLanguageChange(lv_event_t * e);
void onBatteryChange(lv_event_t * e);
void onMusicPlay(lv_event_t * e);
void onMusicPrevious(lv_event_t * e);
void onMusicNext(lv_event_t * e);
void onAlertState(lv_event_t *e);
void onFaceSelected(lv_event_t *e);
void onCustomFaceSelected(int pathIndex);
void onOpenAlbum();
void onCloseAlbum();
void onAlbumPageUpDown(bool up);
void ui_event_resp_ringing(ui_label_t *label);
void onWatchfaceChange(lv_event_t * e);
void ui_event_messageClick(lv_event_t *e);
void ui_event_btnmatrix_dialpad(lv_event_t* e);
void ui_event_brightness_slider_cb(lv_event_t * e);
void ui_CamCaptureExit();
void onThumbnailPreview(int index);
void onPhotoDel();
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
