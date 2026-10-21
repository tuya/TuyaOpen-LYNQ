#ifndef _UI_EVENTS_H
#define _UI_EVENTS_H
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    char numstr[20];
    char name[32];
    char icon[32];
    char button[16];
} ui_label_t;

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
void ui_event_resp_ringing(ui_label_t *label);
void onWatchfaceChange(lv_event_t * e);
void ui_event_messageClick(lv_event_t *e);
void ui_event_btnmatrix_dialpad(lv_event_t* e);
void ui_event_brightness_slider_cb(lv_event_t * e);
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
