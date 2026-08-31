#include DEBUG_LOG_HEADER_FILE
#include "ui.h"
#include "ui_helpers.h"
#include "bsp.h"
#include "dbversion.h"
#ifdef ENABLE_FACE_75_2_DIAL
#include "./faces/75_2_dial/75_2_dial.h"
#endif
#ifdef ENABLE_FACE_SMART_RESIZED
#include "./faces/smart_resized/smart_resized.h"
#endif
#include "watchMenu.h"
#include "main_alipay.h"
#if (CAMERA_ENABLE_GC6153 == 1) && (defined USED_WATCH_WEBRTC)
#include "app_rtc.h"
#endif
#ifdef USED_WATCH_ALIPAY
#include "app_alipay.h"
#include "main_alipay.h"
#endif
#include "syslog.h"
#include "systime.h"
#include "ui_events.h"
#define EPAT_LOG(subId, debugLevel, format, ...)  \
    ECPLAT_PRINTF(UNILOG_LVGL, subId, debugLevel, format, ##__VA_ARGS__)
#ifdef USED_WATCH_MPLAYER
#include "watch_mplayer.h"
#endif

#ifdef USED_WATCH_CAMERA
#include "watch_camera.h"
#endif
#ifdef USED_WATCH_ALBUM
#include "watch_album.h"
#endif
char *ui_settings_EN[8] = {
"Settings",
"Screen Brightness",
"Circular Scroll",
"Screen Timeout",
"Show Alerts",
"Language"
};
char *ui_settings_CN[8] = {
"设置",
"屏幕亮度",
"环形滚动",
"息屏时间",
"显示告警",
"语言"
};
ui_label_t call_label = {
.numstr = " ",
.name = "呼叫中",
.icon = "D:/ui_img_answer.png",
.button = "结束"
};
ui_label_t ring_label = {
.numstr = "10086",
.name = "测试",
.icon = "D:/ui_img_ring.png",
.button = "接听"
};
ui_language_label_t watch_lable;
lv_font_t *pFontNumber = NULL;
lv_font_t *pFontExtern_28 = NULL;
lv_font_t *pFontExtern_48 = NULL;
lv_img_dsc_t ui_jpg_img_dsc = {
    .header.cf = LV_IMG_CF_TRUE_COLOR,
    .header.always_zero = 0,
    .header.reserved = 0,
    .header.w = 0,
    .header.h = 0,
    .data = NULL,
};
lv_img_dsc_t ui_jpg_img_preview_dsc = {
    .header.cf = LV_IMG_CF_TRUE_COLOR,
    .header.always_zero = 0,
    .header.reserved = 0,
    .header.w = 0,
    .header.h = 0,
    .data = NULL,
};
///////////////////// VARIABLES ////////////////////
void pulseCall_Animation(lv_obj_t *TargetObject, int delay);
void analogSecond_Animation(lv_obj_t * TargetObject, int delay);
lv_anim_t secondsAnimation_0;
void ui_event_clockScreen(lv_event_t *e);
lv_obj_t *ui_clockScreen;
lv_obj_t *ui_hourLabel;
lv_obj_t *ui_minuteLabel;
lv_obj_t *ui_dateLabel;
lv_obj_t *ui_weatherIcon;
lv_obj_t *ui_weatherTemp;
lv_obj_t *ui_dayLabel;
lv_obj_t *ui_amPmLabel;
void ui_event_alertPanel(lv_event_t *e);
lv_obj_t *ui_alertPanel;
lv_obj_t *ui_alertIcon;
lv_obj_t *ui_alertText;
void ui_event_weatherScreen(lv_event_t *e);
lv_obj_t *ui_weatherScreen;
lv_obj_t *ui_weatherPanel;
lv_obj_t *ui_weatherCity;
lv_obj_t *ui_weatherCurrentIcon;
lv_obj_t *ui_weatherCurrentTemp;
lv_obj_t *ui_weatherUpdateTime;
lv_obj_t *ui_forecastPanel;
lv_obj_t *ui_forecastTitle;
lv_obj_t *ui_forecastList;

void ui_event_appListScreen(lv_event_t *e);
lv_obj_t *ui_appListScreen;
lv_obj_t *ui_appList;

void ui_event_gameListScreen(lv_event_t *e);
lv_obj_t *ui_gameListScreen;
lv_obj_t *ui_gameList;

void ui_event_notificationScreen(lv_event_t *e);
lv_obj_t *ui_notificationScreen;
lv_obj_t *ui_messagePanel;
lv_obj_t *ui_messageIcon;
lv_obj_t *ui_messageTime;
lv_obj_t *ui_messageContent;
lv_obj_t *ui_messageList;
void ui_event_settingsScreen(lv_event_t *e);
lv_obj_t *ui_settingsScreen;
lv_obj_t *ui_settingsList;
lv_obj_t *ui_settingsTitle;
lv_obj_t *ui_brightnessPanel;
void ui_event_brightnessSlider(lv_event_t *e);
lv_obj_t *ui_brightnessSlider;
lv_obj_t *ui_brightnessIcon;
lv_obj_t *ui_brightnessLabel;
lv_obj_t *ui_scrollingPanel;
lv_obj_t *ui_icon_scrolling;
void ui_event_scrollMode(lv_event_t *e);
lv_obj_t *ui_Switch2;
lv_obj_t *ui_scrollLabel;
lv_obj_t *ui_timeoutPanel;

void ui_event_alertStateSwitch(lv_event_t *e);
lv_obj_t *ui_alertStateLabel;
lv_obj_t *ui_alertStateSwitch;
lv_obj_t *ui_alertStateIcon;
lv_obj_t *ui_alertStatePanel;
void ui_event_timeoutSelect(lv_event_t *e);
lv_obj_t *ui_timeoutSelect;
lv_obj_t *ui_timeoutIcon;
lv_obj_t *ui_timeoutLabel;
lv_obj_t *ui_languagePanel;
void ui_event_languageSelect(lv_event_t *e);
lv_obj_t *ui_languageSelect;
lv_obj_t *ui_languageIcon;
lv_obj_t *ui_languageLabel;
lv_obj_t *ui_batteryPanel;
void ui_event_batterySlider(lv_event_t *e);
lv_obj_t *ui_batterySlider;
lv_obj_t *ui_batteryIcon;
lv_obj_t *ui_batteryLabel;
lv_obj_t *ui_aboutPanel;
lv_obj_t *ui_aboutIcon;
lv_obj_t *ui_aboutText;
void ui_event_kenyaPanel(lv_event_t *e);
lv_obj_t *ui_kenyaPanel;
lv_obj_t *ui_kenyaIcon;
lv_obj_t *ui_kenyaText;
void ui_event_screen_control(lv_event_t *e);
lv_obj_t *ui_controlTitle;
void ui_event_musicPlayButton(lv_event_t *e);
lv_obj_t *ui_musicPlayButton;
void ui_event_musicPrevButton(lv_event_t *e);
lv_obj_t *ui_musicPrevButton;
void ui_event_musicNextButton(lv_event_t *e);
lv_obj_t *ui_musicNextButton;
lv_obj_t *ui_btStateButton;
lv_obj_t *ui_searchPanel;
void ui_event_button_reserve(lv_event_t *e);
lv_obj_t *ui_reserveButton;
lv_obj_t *ui_volumeUpButton;
lv_obj_t *ui_volumeDownButton;
void ui_event_button_dial(lv_event_t *e);
lv_obj_t *ui_icon_btn_dial;

void ui_event_appInfoScreen(lv_event_t *e);
lv_obj_t *ui_appInfoScreen;
lv_obj_t *ui_appInfoPanel;
lv_obj_t *ui_appInfoTitle;
lv_obj_t *ui_appDetailsPanel;
lv_obj_t *ui_appDetailsIcon;
lv_obj_t *ui_appDetailsText;
lv_obj_t *ui_appConnectionPanel;
lv_obj_t *ui_appConnectionIcon;
lv_obj_t *ui_appConnectionText;
lv_obj_t *ui_appBatteryPanel;
lv_obj_t *ui_appBatteryIcon;
lv_obj_t *ui_appBatteryText;
lv_obj_t *ui_appBatteryLevel;

void ui_event_screen_dialpad(lv_event_t *e);
lv_obj_t *ui_dialpadScreen;
lv_obj_t *ui_dialpadLabel;
lv_obj_t *ui_dialMatrix;

void ui_event_screen_call(lv_event_t *e);
lv_obj_t *ui_callScreen;
lv_obj_t *ui_callNumber;
lv_obj_t *ui_callLabel;
lv_obj_t *ui_callButton;
lv_obj_t *ui_callButtonText;
lv_obj_t *ui_callIcon;
lv_obj_t *ui_callPanel;

void ui_event_screen_ringing(lv_event_t *e);
lv_obj_t *ui_ringScreen;
lv_obj_t *ui_ringIcon;
lv_obj_t *ui_ringButton;
lv_obj_t *ui_ringButtonText;
lv_obj_t *ui_ringNumber;
lv_obj_t *ui_ringName;




void ui_filesScreen_screen_init(void);


void ui_photoAlbum_screen_init(void);


lv_obj_t *ui_filesScreen;
lv_obj_t *ui_fileManagerPanel;
lv_obj_t *ui_driveInfoPanel;
lv_obj_t *ui_driveInfoIcon;
lv_obj_t *ui_driveInfoLabel;
lv_obj_t *ui_driveInfoBar;
lv_obj_t *ui_fileInfoPanel;
lv_obj_t *ui_fileInfoIcon;
lv_obj_t *ui_fileInfoName;
lv_obj_t *ui_fileInfoSize;
lv_obj_t *ui_folderInfoPanel;
lv_obj_t *ui_folderInfoIcon;
lv_obj_t *ui_folderInfoName;
lv_obj_t *ui_driveBackPanel;
lv_obj_t *ui_driveBackIcon;
lv_obj_t *ui_driveBackLabel;

lv_obj_t *ui_errorWindow;
lv_obj_t *ui_errorPanel;
lv_obj_t *ui_errorTitle;
lv_obj_t *ui_errorMessage;
void ui_event_errorClose(lv_event_t *e);
lv_obj_t *ui_errorClose;
lv_obj_t *ui_errorCloseText;

lv_obj_t *ui_home;
lv_obj_t *ui_game;
lv_obj_t *ui_faceSelect;

lv_obj_t *face_custom_root;

bool isRinging;
bool toAppList; // flag open from app list
bool circular;
int numFaces;
int numGames;
int currentIndex;
lv_timer_t *time_update_timer = NULL;
void ui_event____initial_actions0(lv_event_t *e);
lv_obj_t *ui____initial_actions0;

Face faces[MAX_FACES];
Face games[MAX_GAMES];

Drag logoEv;

void registerWatchface_cb(const char *name, const lv_img_dsc_t *preview, lv_obj_t **watchface);
void addNotificationList(int appId, const char *message, int index);
void addForecast(int day, int temp, int icon);
void setWeatherIcon(lv_obj_t *obj, int id, bool day);
void setNotificationIcon(lv_obj_t *obj, int appId);
void ui_update_watchfaces(int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday,
                          int temp, int icon, int battery, bool connection, int steps, int distance, int kcal, int bpm, int oxygen);
void addListDrive(const char *name, int total, int used, lv_event_cb_t event_cb);
void addListDir(const char *name);
void addListFile(const char *name, int size);
void addListBack(lv_event_cb_t event_cb);
void addSelectItem(lv_obj_t *parent);

const lv_img_dsc_t *notificationIcons[] = {
    "D:/ui_img_sms.png",      // SMS
    "D:/ui_img_mail.png",     // Mail
    "D:/ui_img_weibo.png",     // Weibo
    "D:/ui_img_wechat.png"      // Wechat
};

const char *days[7] = {"星期天", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六"};
const char *months[12] = {"一月", "二月", "三月", "四月", "五月", "六月", "七月", "八月", "九月", "十月", "十月", "十二月"};
///////////////////// TEST LVGL SETTINGS ////////////////////
#if LV_COLOR_DEPTH != 16
#error "LV_COLOR_DEPTH should be 16bit to match SquareLine Studio's settings"
#endif

///////////////////// ANIMATIONS ////////////////////
void pulseCall_Animation(lv_obj_t *TargetObject, int delay)
{
    ui_anim_user_data_t *PropertyAnimation_0_user_data = lv_mem_alloc(sizeof(ui_anim_user_data_t));
    PropertyAnimation_0_user_data->target = TargetObject;
    PropertyAnimation_0_user_data->val = -1;
    lv_anim_t PropertyAnimation_0;
    lv_anim_init(&PropertyAnimation_0);
    lv_anim_set_time(&PropertyAnimation_0, 1000);
    lv_anim_set_user_data(&PropertyAnimation_0, PropertyAnimation_0_user_data);
    lv_anim_set_custom_exec_cb(&PropertyAnimation_0, _ui_anim_callback_set_image_zoom);
    lv_anim_set_values(&PropertyAnimation_0, 0, 100);
    lv_anim_set_path_cb(&PropertyAnimation_0, lv_anim_path_ease_out);
    lv_anim_set_delay(&PropertyAnimation_0, delay + 0);
    lv_anim_set_deleted_cb(&PropertyAnimation_0, _ui_anim_callback_free_user_data);
    lv_anim_set_playback_time(&PropertyAnimation_0, 1000);
    lv_anim_set_playback_delay(&PropertyAnimation_0, 0);
    lv_anim_set_repeat_count(&PropertyAnimation_0, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_repeat_delay(&PropertyAnimation_0, 0);
    lv_anim_set_early_apply(&PropertyAnimation_0, false);
    lv_anim_set_get_value_cb(&PropertyAnimation_0, &_ui_anim_callback_get_image_zoom);
    lv_anim_start(&PropertyAnimation_0);
    ui_anim_user_data_t *PropertyAnimation_1_user_data = lv_mem_alloc(sizeof(ui_anim_user_data_t));
    PropertyAnimation_1_user_data->target = TargetObject;
    PropertyAnimation_1_user_data->val = -1;
    lv_anim_t PropertyAnimation_1;
    lv_anim_init(&PropertyAnimation_1);
    lv_anim_set_time(&PropertyAnimation_1, 2000);
    lv_anim_set_user_data(&PropertyAnimation_1, PropertyAnimation_1_user_data);
    lv_anim_set_custom_exec_cb(&PropertyAnimation_1, _ui_anim_callback_set_y);
    lv_anim_set_values(&PropertyAnimation_1, 0, 20);
    lv_anim_set_path_cb(&PropertyAnimation_1, lv_anim_path_ease_out);
    lv_anim_set_delay(&PropertyAnimation_1, delay + 0);
    lv_anim_set_deleted_cb(&PropertyAnimation_1, _ui_anim_callback_free_user_data);
    lv_anim_set_playback_time(&PropertyAnimation_1, 2000);
    lv_anim_set_playback_delay(&PropertyAnimation_1, 0);
    lv_anim_set_repeat_count(&PropertyAnimation_1, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_repeat_delay(&PropertyAnimation_1, 0);
    lv_anim_set_early_apply(&PropertyAnimation_1, false);
    lv_anim_set_get_value_cb(&PropertyAnimation_1, &_ui_anim_callback_get_y);
    lv_anim_start(&PropertyAnimation_1);
}
/**
  \fn
  \brief
  \return
*/
void findPhone_Animation(lv_obj_t *TargetObject, int delay)
{
    ui_anim_user_data_t *PropertyAnimation_0_user_data = lv_mem_alloc(sizeof(ui_anim_user_data_t));
    PropertyAnimation_0_user_data->target = TargetObject;
    PropertyAnimation_0_user_data->val = -1;
    lv_anim_t PropertyAnimation_0;
    lv_anim_init(&PropertyAnimation_0);
    lv_anim_set_time(&PropertyAnimation_0, 500);
    lv_anim_set_user_data(&PropertyAnimation_0, PropertyAnimation_0_user_data);
    lv_anim_set_custom_exec_cb(&PropertyAnimation_0, _ui_anim_callback_set_width);
    lv_anim_set_values(&PropertyAnimation_0, 64, 110);
    lv_anim_set_path_cb(&PropertyAnimation_0, lv_anim_path_linear);
    lv_anim_set_delay(&PropertyAnimation_0, delay + 0);
    lv_anim_set_deleted_cb(&PropertyAnimation_0, _ui_anim_callback_free_user_data);
    lv_anim_set_playback_time(&PropertyAnimation_0, 500);
    lv_anim_set_playback_delay(&PropertyAnimation_0, 0);
    lv_anim_set_repeat_count(&PropertyAnimation_0, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_repeat_delay(&PropertyAnimation_0, 0);
    lv_anim_set_early_apply(&PropertyAnimation_0, false);
    lv_anim_start(&PropertyAnimation_0);
    ui_anim_user_data_t *PropertyAnimation_1_user_data = lv_mem_alloc(sizeof(ui_anim_user_data_t));
    PropertyAnimation_1_user_data->target = TargetObject;
    PropertyAnimation_1_user_data->val = -1;
    lv_anim_t PropertyAnimation_1;
    lv_anim_init(&PropertyAnimation_1);
    lv_anim_set_time(&PropertyAnimation_1, 500);
    lv_anim_set_user_data(&PropertyAnimation_1, PropertyAnimation_1_user_data);
    lv_anim_set_custom_exec_cb(&PropertyAnimation_1, _ui_anim_callback_set_height);
    lv_anim_set_values(&PropertyAnimation_1, 64, 110);
    lv_anim_set_path_cb(&PropertyAnimation_1, lv_anim_path_linear);
    lv_anim_set_delay(&PropertyAnimation_1, delay + 0);
    lv_anim_set_deleted_cb(&PropertyAnimation_1, _ui_anim_callback_free_user_data);
    lv_anim_set_playback_time(&PropertyAnimation_1, 500);
    lv_anim_set_playback_delay(&PropertyAnimation_1, 0);
    lv_anim_set_repeat_count(&PropertyAnimation_1, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_repeat_delay(&PropertyAnimation_1, 0);
    lv_anim_set_early_apply(&PropertyAnimation_1, false);
    lv_anim_start(&PropertyAnimation_1);
}
/**
  \fn
  \brief
  \return
*/
void analogSecond_Animation(lv_obj_t * TargetObject, int delay)
{
    ui_anim_user_data_t * secondsAnimation_0_user_data = lv_mem_alloc(sizeof(ui_anim_user_data_t));
    secondsAnimation_0_user_data->target = TargetObject;
    secondsAnimation_0_user_data->val = -1;
    lv_anim_init(&secondsAnimation_0);
    lv_anim_set_time(&secondsAnimation_0, 60000);
    lv_anim_set_user_data(&secondsAnimation_0, secondsAnimation_0_user_data);
    lv_anim_set_custom_exec_cb(&secondsAnimation_0, _ui_anim_callback_set_image_angle);
    lv_anim_set_values(&secondsAnimation_0, 0, 3600);
    lv_anim_set_path_cb(&secondsAnimation_0, lv_anim_path_linear);
    lv_anim_set_delay(&secondsAnimation_0, delay + 0);
    lv_anim_set_deleted_cb(&secondsAnimation_0, _ui_anim_callback_free_user_data);
    lv_anim_set_playback_time(&secondsAnimation_0, 0);
    lv_anim_set_playback_delay(&secondsAnimation_0, 0);
    lv_anim_set_repeat_count(&secondsAnimation_0, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_repeat_delay(&secondsAnimation_0, 0);
    lv_anim_set_early_apply(&secondsAnimation_0, false);
    lv_anim_set_get_value_cb(&secondsAnimation_0, &_ui_anim_callback_get_image_angle);
    lv_anim_start(&secondsAnimation_0);
}

///////////////////// FUNCTIONS ////////////////////

void ui_event____initial_actions0(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    //lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_SCREEN_LOAD_START)
    {
        pulseCall_Animation(ui_ringIcon, 0);
        findPhone_Animation(ui_callPanel, 0);
    }
}
/**
  \fn
  \brief
  \return
*/
void ui_event_clockScreen(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    //lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT)
    {
#if USED_WATCH_CAMERA
        _ui_screen_change(ui_cameraScreen, LV_SCR_LOAD_ANIM_NONE, 0, 0);
#endif
    }
    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_LEFT)
    {
        _ui_screen_change(ui_appListScreen, LV_SCR_LOAD_ANIM_NONE, 0, 0);
    }
    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_TOP)
    {
        toAppList = false; // flag was not open from app list
        _ui_screen_change(ui_weatherScreen, LV_SCR_LOAD_ANIM_NONE, 0, 0);
    }
    if (event_code == LV_EVENT_SCREEN_LOAD_START)
    {
        onLoadHome(e);
    }
    if (event_code == LV_EVENT_LONG_PRESSED)
    {
        // ui_home = ui_pcbScreen;
        // _ui_screen_change(ui_home, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0);
        onWatchfaceChange(e);
    }
}
/**
  \fn
  \brief
  \return
*/
void ui_event_appListScreen(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    //lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT)
    {
        _ui_screen_change(ui_home, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0);
    }
}
/**
  \fn
  \brief
  \return
*/
void ui_event_gameListScreen(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    //lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT)
    {
        _ui_screen_change(ui_appListScreen, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0);
    }
}
/**
  \fn
  \brief
  \return
*/
#if 0
void ui_event_alertPanel(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        onClickAlert(e);
        _ui_flag_modify(ui_alertPanel, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_ADD);
    }
    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_LEFT)
    {
        _ui_flag_modify(ui_alertPanel, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_ADD);
    }
}
#endif
/**
  \fn
  \brief
  \return
*/
void ui_event_weatherScreen(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_BOTTOM)
    {
        if (toAppList)
        {
            // do nothing
        }
        else
        {
            _ui_screen_change(ui_home, LV_SCR_LOAD_ANIM_MOVE_BOTTOM, 500, 0); // load home
        }
    }
    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_LEFT)
    {
        _ui_flag_modify(ui_weatherPanel, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_ADD);
        _ui_flag_modify(ui_forecastPanel, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_REMOVE);
        onForecastOpen(e);
    }
    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT)
    {
        if (!lv_obj_has_flag(ui_weatherPanel, LV_OBJ_FLAG_HIDDEN))
        {
                if (toAppList)
                {
                    _ui_screen_change(ui_appListScreen, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0);
                    return;
                }
        }
        _ui_flag_modify(ui_weatherPanel, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_REMOVE);
        _ui_flag_modify(ui_forecastPanel, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_ADD);
    }
    if (event_code == LV_EVENT_SCREEN_LOAD_START)
    {
        onWeatherLoad(e);
    }
}
/**
  \fn
  \brief
  \return
*/
void ui_event_notificationScreen(lv_event_t *e)
{
      lv_event_code_t event_code = lv_event_get_code(e);
      //lv_obj_t *target = lv_event_get_target(e);
      if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_LEFT)
      {
            if (lv_obj_has_flag(ui_messageList, LV_OBJ_FLAG_HIDDEN))
            {
                  _ui_flag_modify(ui_messageList, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_REMOVE);
                  _ui_flag_modify(ui_messagePanel, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_ADD);
            }
            else
            {
                _ui_flag_modify(ui_messageList, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_REMOVE);
                _ui_flag_modify(ui_messagePanel, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_ADD);
                if (toAppList)
                {
                }
                else
                {
                    _ui_screen_change(ui_home, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0);
                }
            }
      }
      if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT)
      {
            if (toAppList)
            {
                  if (!lv_obj_has_flag(ui_messageList, LV_OBJ_FLAG_HIDDEN))
                  {
                        _ui_screen_change(ui_appListScreen, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0);
                        return;
                  }
            }
            _ui_flag_modify(ui_messagePanel, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_ADD);
            _ui_flag_modify(ui_messageList, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_REMOVE);
      }
      if (event_code == LV_EVENT_SCREEN_LOAD_START)
      {
            onNotificationsOpen(e);
      }
}

void ui_event_settingsScreen(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    //lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT)
    {
        _ui_screen_change(ui_appListScreen, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0);
    }
}

void ui_event_appInfoScreen(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    //lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT)
    {
        _ui_screen_change(ui_appListScreen, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0);
    }
}
/**
  \fn
  \brief
  \return
*/
void ui_event_screen_ringing(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    //lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT)
    {
        _ui_screen_change(ui_home, LV_SCR_LOAD_ANIM_FADE_OUT, 500, 0);
        onHangUp();
        // LV_LOG_WARN("%d",event_code);
    }
    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_TOP)
    {
        onHangUp();
        if (toAppList)
        {
            _ui_screen_change(ui_appListScreen, LV_SCR_LOAD_ANIM_MOVE_TOP, 500, 0);
        }
        // LV_LOG_USER("%d",event_code);
    }
}
/**
  \fn
  \brief
  \return
*/
void ui_event_screen_dialpad(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_TOP)
    {
        if (toAppList)  // flag was open from app list
        {
            _ui_screen_change(ui_appListScreen, LV_SCR_LOAD_ANIM_MOVE_TOP, 500, 0);
        }
        LV_LOG_WARN("%d",event_code);
    }
    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT)
    {
        // _ui_screen_change(ui_home, LV_SCR_LOAD_ANIM_FADE_OUT, 500, 0);
        _ui_screen_change(ui_appListScreen, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0);
        phone_text_index=0;
        memset(call_label.numstr,0,sizeof(call_label.numstr));
        // LV_LOG_WARN("--%d",event_code);
    }
}
/**
  \fn
  \brief
  \return
*/
void ui_event_brightnessSlider(lv_event_t *e)
{
    lv_disp_t *display = lv_disp_get_default();
    lv_obj_t *actScr = lv_disp_get_scr_act(display);
    if (actScr != ui_settingsScreen)
    {
        return;
    }
    lv_event_code_t event_code = lv_event_get_code(e);
    //lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        onBrightnessChange(e);
    }
}

void ui_event_scrollMode(lv_event_t *e)
{
    lv_disp_t *display = lv_disp_get_default();
    lv_obj_t *actScr = lv_disp_get_scr_act(display);
    if (actScr != ui_settingsScreen)
    {
        return;
    }
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_VALUE_CHANGED)
    {

        circular = lv_obj_has_state(target, LV_STATE_CHECKED);
        lv_obj_scroll_by(ui_settingsList, 0, circular ? 1 : -1, LV_ANIM_ON);
        lv_obj_scroll_by(ui_appList, 0, circular ? 1 : -1, LV_ANIM_OFF);
        lv_obj_scroll_by(ui_messageList, 0, circular ? 1 : -1, LV_ANIM_OFF);
        lv_obj_scroll_by(ui_appInfoPanel, 0, circular ? 1 : -1, LV_ANIM_OFF);
        lv_obj_scroll_by(ui_gameList, 0, circular ? 1 : -1, LV_ANIM_OFF);
        lv_obj_scroll_by(ui_fileManagerPanel, 0, circular ? 1 : -1, LV_ANIM_OFF);
        onScrollMode(e);
    }
}

void ui_event_alertStateSwitch(lv_event_t *e)
{
      lv_disp_t *display = lv_disp_get_default();
      lv_obj_t *actScr = lv_disp_get_scr_act(display);
      if (actScr != ui_settingsScreen)
      {
            return;
      }
      lv_event_code_t event_code = lv_event_get_code(e);
      lv_obj_t *target = lv_event_get_target(e);
      if (event_code == LV_EVENT_VALUE_CHANGED)
      {
            onAlertState(e);
      }
}
/**
  \fn
  \brief    设置熄屏时间
  \return
*/
void ui_event_timeoutSelect(lv_event_t *e)
{
    lv_disp_t *display = lv_disp_get_default();
    lv_obj_t *actScr = lv_disp_get_scr_act(display);
    if (actScr != ui_settingsScreen)
    {
        return;
    }
    lv_event_code_t event_code = lv_event_get_code(e);
    //lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        onTimeoutChange(e);
    }
}
/**
  \fn
  \brief
  \return
*/
void ui_event_languageSelect(lv_event_t *e)
{
    lv_disp_t *display = lv_disp_get_default();
    lv_obj_t *actScr = lv_disp_get_scr_act(display);
    if (actScr != ui_settingsScreen)
    {
        return;
    }
    lv_event_code_t event_code = lv_event_get_code(e);
    //lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        onLanguageChange(e);
    }
}
/**
  \fn
  \brief
  \return
*/
void ui_event_batterySlider(lv_event_t *e)
{
    lv_disp_t *display = lv_disp_get_default();
    lv_obj_t *actScr = lv_disp_get_scr_act(display);
    if (actScr != ui_settingsScreen)
    {
        return;
    }
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        _ui_slider_set_text_value(ui_batteryLabel, target, "电量 ", "%");
    }
    if (event_code == LV_EVENT_RELEASED)
    {
        onBatteryChange(e);
    }
}
/**
  \fn
  \brief
  \return
*/
void ui_event_kenyaPanel(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    //lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        _ui_screen_change(ui_appListScreen, LV_SCR_LOAD_ANIM_MOVE_TOP, 500, 0);
    }
}


/**
  \fn
  \brief
  \return
*/
void ui_event_screen_call(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    //lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT)
    {
        toAppList = true; // flag was open from app list
        phone_text_index=0;
        memset(call_label.numstr,0,sizeof(call_label.numstr));
        _ui_screen_change(ui_appListScreen, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0);
        lv_label_set_text(ui_dialpadLabel, call_label.numstr);
        onHangUp();
    }
    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_BOTTOM)
    {
        toAppList = false; // flag was not open from app list
        phone_text_index=0;
        memset(call_label.numstr,0,sizeof(call_label.numstr));
        _ui_screen_change(ui_appListScreen, LV_SCR_LOAD_ANIM_MOVE_BOTTOM, 500, 0);
        lv_label_set_text(ui_dialpadLabel, call_label.numstr);
        onHangUp();
        // LV_LOG_USER("%d",event_code);
    }
    if (event_code == LV_EVENT_SCREEN_UNLOAD_START)
    {
        LV_LOG_USER("%d",event_code);
        phone_text_index=0;
        memset(call_label.numstr,0,sizeof(call_label.numstr));
        // lv_label_set_text(ui_dialpadLabel, call_label.numstr);
        _ui_label_set_property(ui_dialpadLabel, _UI_LABEL_PROPERTY_TEXT, " ");
        // _ui_label_set_property(ui_callLabel, _UI_LABEL_PROPERTY_TEXT,watch_lable.dialing[3]);
        // _ui_label_set_property(ui_callButtonText, _UI_LABEL_PROPERTY_TEXT, watch_lable.dialing[0]);
        // _ui_state_modify(ui_callButton, LV_STATE_CHECKED, _UI_MODIFY_STATE_REMOVE);
        _ui_flag_modify(ui_callPanel, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_REMOVE);
    }
}

/**
  \fn
  \brief
  \return
*/
void ui_event_faceSelected(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    int index = (int)lv_event_get_user_data(e);
    if (target == ui_home)
    {
        return;
    }
    if (event_code == LV_EVENT_CLICKED)
    {
        if (index >= numFaces)
        {
            showError("Watchface Error", "Watchface out of bounds");
            return;
        }
        if (currentIndex != index)
        {
            currentIndex = index;
            if (faces[index].custom)
            {
                onCustomFaceSelected(faces[index].customIndex);
                onFaceSelected(e);

                return;
            }
            else
            {
                ui_home = *faces[index].watchface;
                onCustomFaceSelected(-1);
            }
        }
        lv_scr_load_anim(ui_home, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
        onFaceSelected(e);
    }
}

/**
  \fn
  \brief
  \return
*/
void onAppListClicked(lv_event_t *e)
{
    lv_disp_t *display = lv_disp_get_default();
    lv_obj_t *actScr = lv_disp_get_scr_act(display);
    if (actScr != ui_appListScreen)
    {
        return;
    }
    int index = (int)lv_event_get_user_data(e);
    switch (index)
    {
      case 0:
            toAppList = true; // flag was open from app list
            _ui_screen_change(ui_notificationScreen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0);
            break;
      case 1:
            toAppList = true; // flag was open from app list
            _ui_screen_change(ui_weatherScreen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0);
            break;
      case 2:
            _ui_screen_change(ui_settingsScreen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0);
            break;
      case 3:
            _ui_screen_change(ui_appInfoScreen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0);
            break;
      case 4:
            toAppList = true; // flag was open from app list
            // extern aliAppInfo_t alipayInfo;
	  		// if(alipayInfo.status >= E_ALI_STA_BINDIND_SUCCESS)
            //  	_ui_screen_change(ui_alipayScreen,LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0);
			// else
			// _ui_screen_change(ui_bindCodeScreen,LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0);
            #if (CAMERA_ENABLE_GC6153 == 1) && (defined USED_WATCH_WEBRTC)
            webrtc_screen_load(1);
            // _ui_screen_change(watch_rtc->screen,LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0);
            #endif
            break;
      case 5:
            lv_obj_clean(ui_fileManagerPanel);
            addSelectItem(ui_fileManagerPanel);
            for (int i = 0; i < numFaces; i++)
            {
                addFaceList(ui_fileManagerPanel, faces[i]);
            }
            _ui_screen_change(ui_filesScreen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0);
            break;
      case 6:
            toAppList = true; // flag was open from app list
            phone_text_index=0;
            memset(call_label.numstr,0,sizeof(call_label.numstr));
            _ui_screen_change(ui_dialpadScreen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0);
            break;
      case 7:
            toAppList = true; // flag was open from app list
            _ui_screen_change(ui_gameListScreen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0);
            break;
      case 8:
        	{
#ifdef USED_WATCH_ALIPAY
        		extern aliAppInfo_t alipayInfo;
				extern lv_obj_t *ui_alipayScreen;
				extern lv_obj_t *ui_bindCodeScreen;
				ECPLAT_PRINTF(UNILOG_PLAT_ALIPAY, onAppListClicked, P_DEBUG, "status[%d]",alipayInfo.status);
				if(ui_bindCodeScreen && ui_alipayScreen)
				{
					if(alipayInfo.status >= E_ALI_STA_BINDIND_SUCCESS)
					{
						ui_paymnet_state_show(0,alipayInfo.paymentStatus);
						_ui_screen_change(ui_alipayScreen,LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0);
					}
					else
					{
						ui_binding_stat_show(alipayInfo.status);
						_ui_screen_change(ui_bindCodeScreen,LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0);
					}
				}
#endif
        	}
            break;
        case 9:
#ifdef USED_WATCH_CAMERA
            _ui_screen_change(ui_cameraScreen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0);
#endif
            break;
        case 10:
#ifdef USED_WATCH_ALBUM
            _ui_screen_change(ui_photoAlbumScreen, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0);
#endif
            break;
        case 11:
#ifdef USED_WATCH_MPLAYER
            _ui_screen_change(ui_scrFileList, LV_SCR_LOAD_ANIM_MOVE_LEFT,
                              500, 0);
#endif
            break;
      }
}
/**
  \fn
  \brief
  \return
*/
static void onScroll(lv_event_t *e)
{
    lv_obj_t *list = lv_event_get_target(e);
    lv_area_t list_a;
    lv_obj_get_coords(list, &list_a);
    lv_coord_t list_y_center = list_a.y1 + lv_area_get_height(&list_a) / 2;
    lv_coord_t r = lv_obj_get_height(list) * 7 / 10;
    uint32_t i;
    uint32_t child_cnt = lv_obj_get_child_cnt(list);
    for (i = 0; i < child_cnt; i++)
    {
        lv_obj_t *child = lv_obj_get_child(list, i);
        lv_area_t child_a;
        lv_obj_get_coords(child, &child_a);
        lv_coord_t child_y_center = child_a.y1 + lv_area_get_height(&child_a) / 2;
        lv_coord_t diff_y = child_y_center - list_y_center;
        diff_y = LV_ABS(diff_y);
        /*Get the x of diff_y on a circle.*/
        lv_coord_t x;
        /*If diff_y is out of the circle use the last point of the circle (the radius)*/
        if (diff_y >= r)
        {
            x = r;
        }
        else
        {
            /*Use Pythagoras theorem to get x from radius and y*/
            uint32_t x_sqr = r * r - diff_y * diff_y;
            lv_sqrt_res_t res;
            lv_sqrt(x_sqr, &res, 0x8000); /*Use lvgl's built in sqrt root function*/
            x = r - res.i;
        }
        /*Translate the item by the calculated X coordinate*/
        lv_obj_set_style_translate_x(child, circular ? x : 0, 0);
        /*Use some opacity with larger translations*/
        // lv_opa_t opa = lv_map(x, 0, r, LV_OPA_TRANSP, LV_OPA_COVER);
        // lv_obj_set_style_opa(child, LV_OPA_COVER - opa, 0);
    }
}
/**
  \fn
  \brief
  \return
*/

static void update_time_callback(lv_timer_t *timer)
{
    time_t current_time;
    struct tm *local_time;
    char time_str[6];
    char date_str[20];

    // Get current system time
    current_time = time_time(NULL);
    local_time = time_localtime(&current_time);
    sprintf(time_str, "%02d", local_time->tm_hour);
    lv_label_set_text(ui_hourLabel, time_str);
    sprintf(time_str, "%02d", local_time->tm_min);
    lv_label_set_text(ui_minuteLabel, time_str);
    sprintf(date_str, "%02d\n%s", local_time->tm_mday, months[local_time->tm_mon]);
    lv_label_set_text(ui_dateLabel, date_str);
    sprintf(time_str, "%s", days[local_time->tm_wday]);
    lv_label_set_text(ui_dayLabel, time_str);
}
void watchfaceEvents(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    //lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_LEFT)
    {
        lv_scr_load_anim(ui_appListScreen, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
    }
    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_TOP)
    {
        toAppList = false; // flag was not open from app list
        lv_scr_load_anim(ui_weatherScreen, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
    }
    if (event_code == LV_EVENT_LONG_PRESSED_REPEAT)
    {
        lv_disp_t *display = lv_disp_get_default();
        lv_obj_t *actScr = lv_disp_get_scr_act(display);
        if (actScr != ui_home)
        {
            return;
        }
        // ui_home = ui_clockScreen;
        lv_scr_load_anim(ui_faceSelect, LV_SCR_LOAD_ANIM_FADE_ON, 50, 0, false);
    }
    if(event_code == LV_EVENT_SCREEN_LOADED)
    {
        time_update_timer = lv_timer_create(update_time_callback, 1000, NULL);
    }
    if(event_code == LV_EVENT_SCREEN_UNLOADED)
    {
        lv_timer_del(time_update_timer);
        time_update_timer = NULL;
    }
}
/**
  \fn
  \brief
  \return
*/
void onFaceEvent(lv_event_t *e)
{
    watchfaceEvents(e);
}
/**
  \fn
  \brief
  \return
*/
void ui_event_gameSelected(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    //lv_obj_t *target = lv_event_get_target(e);
    int index = (int)lv_event_get_user_data(e);

    lv_disp_t *display = lv_disp_get_default();
    lv_obj_t *actScr = lv_disp_get_scr_act(display);
    if (actScr != ui_gameListScreen)
    {
        return;
    }
    if (event_code == LV_EVENT_CLICKED)
    {
        if (index >= numGames)
        {
            showError("Game Error", "Could not lauch the game, index out of bounds");
            return;
        }
        // ui_game = *games[index].watchface;
        if (!*games[index].watchface)
        {
            showError("Game Error", "Game root object not initialized");
            return;
        }
        lv_scr_load_anim(*games[index].watchface, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0, false);
        // lv_scr_load_anim(ui_raceScreen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0, false);
    }
}

/**
  \fn
  \brief
  \return
*/
void ui_event_errorClose(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    //lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        _ui_flag_modify(ui_errorWindow, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_ADD);
    }
}
/**
  \fn
  \brief
  \return
*/
void ui_event_face_select(lv_event_t *e)
{
    lv_disp_t *display = lv_disp_get_default();
    lv_obj_t *actScr = lv_disp_get_scr_act(display);
    if (actScr != ui_filesScreen)
    {
        return;
    }
    lv_scr_load_anim(ui_faceSelect, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, false);
}

uint8_t getWeatherIconIndex(uint8_t id)
{
      if (id > 7)
      {
            return 0;
      }
      return id;
}

int getNotificationIconIndex(int id)
{
      switch (id)
      {
      case 0x03:
            return 0;
      case 0x04:
            return 1;
      case 0x07:
            return 2;
      case 0x08:
            return 3;
      case 0x0A:
            return 4;
      case 0x0B:
            return 5;
      case 0x0E:
            return 6;
      case 0x0F:
            return 7;
      case 0x10:
            return 8;
      case 0x11:
            return 9;
      case 0x12:
            return 10;
      case 0x13:
            return 11;
      case 0x14:
            return 12;
      case 0x16:
            return 13;
      case 0x17:
            return 14;
      case 0x18:
            return 15;
      case 0xC0:
            return 16;
      case 0x09:
            return 17;
      default:
            return 0;
      }
}

void setWeatherIcon(lv_obj_t *obj, int id, bool day)
{
      // lv_img_set_src(obj, day ? weatherIcons[getWeatherIconIndex(id)] : weatherNtIcons[getWeatherIconIndex(id)]);
}

void setNotificationIcon(lv_obj_t *obj, int appId)
{
    lv_img_set_src(obj, notificationIcons[getNotificationIconIndex(appId)]);
}
/**
  \fn
  \brief
  \return
*/
void add_appList(const char *appName, int index, const void *img)
{
    lv_obj_t *ui_appListPanel = lv_obj_create(ui_appList);
    lv_obj_set_width(ui_appListPanel, 200);
    lv_obj_set_height(ui_appListPanel, 64);
    lv_obj_set_align(ui_appListPanel, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_appListPanel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(ui_appListPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_appListPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_appListPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_appListPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_appListPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_appListPanel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_appListPanel, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_appListPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_appListPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_appListPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_appListPanel, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_appListIcon = lv_img_create(ui_appListPanel);
    lv_img_set_src(ui_appListIcon, img);
    lv_obj_set_width(ui_appListIcon, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_appListIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_appListIcon, -74);
    lv_obj_set_y(ui_appListIcon, 2);
    lv_obj_set_align(ui_appListIcon, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_appListIcon, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_appListIcon, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_img_set_zoom(ui_appListIcon, 150);

    lv_obj_t *ui_appListLabel = lv_label_create(ui_appListPanel);
    lv_obj_set_width(ui_appListLabel, 160);
    lv_obj_set_height(ui_appListLabel, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_appListLabel, 54);
    lv_obj_set_y(ui_appListLabel, 3);
    lv_obj_set_align(ui_appListLabel, LV_ALIGN_LEFT_MID);
    lv_label_set_text(ui_appListLabel, appName);
    lv_label_set_long_mode(ui_appListLabel, LV_LABEL_LONG_CLIP);
    lv_obj_add_event_cb(ui_appListPanel, onAppListClicked, LV_EVENT_CLICKED, (void *)index);
}
/**
  \fn
  \brief
  \return
*/
void add_gameList(const char *appName, int index, const void *img)
{
    lv_obj_t *ui_gameListPanel = lv_obj_create(ui_gameList);
    lv_obj_set_width(ui_gameListPanel, 200);
    lv_obj_set_height(ui_gameListPanel, 64);
    lv_obj_set_align(ui_gameListPanel, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_gameListPanel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(ui_gameListPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_gameListPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_gameListPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_gameListPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_gameListPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_gameListPanel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_gameListPanel, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_gameListPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_gameListPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_gameListPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_gameListPanel, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_gameListIcon = lv_img_create(ui_gameListPanel);
    lv_img_set_src(ui_gameListIcon, img);
    lv_obj_set_width(ui_gameListIcon, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_gameListIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_gameListIcon, -74);
    lv_obj_set_y(ui_gameListIcon, 2);
    lv_obj_set_align(ui_gameListIcon, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_gameListIcon, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_gameListIcon, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_img_set_zoom(ui_gameListIcon, 150);

    lv_obj_t *ui_gameListLabel = lv_label_create(ui_gameListPanel);
    lv_obj_set_width(ui_gameListLabel, 160);
    lv_obj_set_height(ui_gameListLabel, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_gameListLabel, 54);
    lv_obj_set_y(ui_gameListLabel, 3);
    lv_obj_set_align(ui_gameListLabel, LV_ALIGN_LEFT_MID);
    lv_label_set_text(ui_gameListLabel, appName);
    lv_label_set_long_mode(ui_gameListLabel, LV_LABEL_LONG_CLIP);
    // lv_obj_set_style_text_font(ui_gameListLabel, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(ui_gameListPanel, ui_event_gameSelected, LV_EVENT_CLICKED, (void *)index);
}
/**
  \fn
  \brief
  \return
*/
void addWatchface(const char *name, const lv_img_dsc_t *src, int index)
{
    lv_obj_t *ui_faceItem = lv_obj_create(ui_faceSelect);
    lv_obj_set_width(ui_faceItem, 160);
    lv_obj_set_height(ui_faceItem, 180);
    lv_obj_set_align(ui_faceItem, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_faceItem, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_add_flag(ui_faceItem, LV_OBJ_FLAG_SNAPPABLE);
    lv_obj_clear_flag(ui_faceItem, LV_OBJ_FLAG_SCROLL_ONE);
    lv_obj_set_style_radius(ui_faceItem, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_faceItem, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_faceItem, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_faceItem, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_faceItem, lv_color_hex(0x142ABC), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(ui_faceItem, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(ui_faceItem, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_pad(ui_faceItem, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_faceItem, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_faceItem, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_faceItem, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_faceItem, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_facePreview = lv_img_create(ui_faceItem);
    lv_img_set_src(ui_facePreview, src);
    lv_obj_set_width(ui_facePreview, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_facePreview, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_facePreview, LV_ALIGN_TOP_MID);
    lv_obj_add_flag(ui_facePreview, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_facePreview, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    lv_obj_t *ui_faceLabel = lv_label_create(ui_faceItem);
    lv_obj_set_width(ui_faceLabel, 160);
    lv_obj_set_height(ui_faceLabel, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_faceLabel, LV_ALIGN_BOTTOM_MID);
    lv_label_set_long_mode(ui_faceLabel, LV_LABEL_LONG_DOT);
    lv_label_set_text(ui_faceLabel, name);
    lv_obj_set_style_text_align(ui_faceLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(ui_faceItem, ui_event_faceSelected, LV_EVENT_ALL, (void *)index);
}
/**
  \fn
  \brief
  \return
*/
void addSelectItem(lv_obj_t *parent)
{
    lv_obj_t *ui_faceSelectItem = lv_obj_create(parent);
    lv_obj_set_width(ui_faceSelectItem, 240);
    lv_obj_set_height(ui_faceSelectItem, 50);
    lv_obj_set_align(ui_faceSelectItem, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_faceSelectItem, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(ui_faceSelectItem, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_faceSelectItem, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_faceSelectItem, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_faceSelectItem, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_faceSelectItem, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_faceSelectItem, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_faceSelectItem, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_faceItemIcon = lv_img_create(ui_faceSelectItem);
    lv_img_set_src(ui_faceItemIcon, "D:/ui_img_clock.png");
    lv_obj_set_width(ui_faceItemIcon, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_faceItemIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_faceItemIcon, 10);
    lv_obj_set_y(ui_faceItemIcon, 0);
    lv_obj_set_align(ui_faceItemIcon, LV_ALIGN_LEFT_MID);
    lv_obj_add_flag(ui_faceItemIcon, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_faceItemIcon, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_img_recolor(ui_faceItemIcon, lv_color_hex(0x2062E3), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor_opa(ui_faceItemIcon, 200, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_faceItemName = lv_label_create(ui_faceSelectItem);
    lv_obj_set_width(ui_faceItemName, 117);
    lv_obj_set_height(ui_faceItemName, LV_SIZE_CONTENT);
    lv_obj_set_x(ui_faceItemName, 50);
    lv_obj_set_y(ui_faceItemName, 0);
    lv_obj_set_align(ui_faceItemName, LV_ALIGN_LEFT_MID);
    lv_label_set_long_mode(ui_faceItemName, LV_LABEL_LONG_CLIP);
    watch_lable.system[1] = "选择表盘"; //"Change Current"
    lv_label_set_text(ui_faceItemName, watch_lable.system[1]);
    lv_obj_add_event_cb(ui_faceSelectItem, ui_event_face_select, LV_EVENT_CLICKED, NULL);
}
/**
  \fn
  \brief
  \return
*/
void addForecast(int day, int temp, int icon)
{
    lv_obj_t *forecastItem = lv_obj_create(ui_forecastList);
    lv_obj_set_width(forecastItem, 200);
    lv_obj_set_height(forecastItem, 40);
    lv_obj_set_align(forecastItem, LV_ALIGN_CENTER);
    lv_obj_clear_flag(forecastItem, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(forecastItem, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(forecastItem, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(forecastItem, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(forecastItem, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(forecastItem, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(forecastItem, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(forecastItem, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(forecastItem, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(forecastItem, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(forecastItem, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(forecastItem, 10, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *forecastIcon = lv_img_create(forecastItem);
    lv_obj_set_width(forecastIcon, LV_SIZE_CONTENT);  /// 40
    lv_obj_set_height(forecastIcon, LV_SIZE_CONTENT); /// 32
    lv_obj_set_align(forecastIcon, LV_ALIGN_CENTER);
    lv_obj_add_flag(forecastIcon, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(forecastIcon, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_img_set_zoom(forecastIcon, 150);
    setWeatherIcon(forecastIcon, icon, true);

    lv_obj_t *forecastTemp = lv_label_create(forecastItem);
    lv_obj_set_width(forecastTemp, 58);
    lv_obj_set_height(forecastTemp, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(forecastTemp, 68);
    lv_obj_set_y(forecastTemp, 3);
    lv_obj_set_align(forecastTemp, LV_ALIGN_CENTER);
    lv_label_set_long_mode(forecastTemp, LV_LABEL_LONG_CLIP);
    lv_label_set_text_fmt(forecastTemp, "%d°C", temp);
    // lv_obj_set_style_text_font(forecastTemp, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *forecastDay = lv_label_create(forecastItem);
    lv_obj_set_width(forecastDay, 68);
    lv_obj_set_height(forecastDay, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(forecastDay, -56);
    lv_obj_set_y(forecastDay, 3);
    lv_obj_set_align(forecastDay, LV_ALIGN_CENTER);
    lv_label_set_long_mode(forecastDay, LV_LABEL_LONG_CLIP);
    lv_label_set_text(forecastDay, days[day % 7]);
    // lv_obj_set_style_text_font(forecastDay, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
}

void addNotificationList(int appId, const char *message, int index)
{
    lv_obj_t *notificationItem = lv_obj_create(ui_messageList);
    lv_obj_set_width(notificationItem, 200);
    lv_obj_set_height(notificationItem, LV_SIZE_CONTENT); /// 50
    lv_obj_set_align(notificationItem, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow(notificationItem, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(notificationItem, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(notificationItem, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(notificationItem, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(notificationItem, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(notificationItem, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(notificationItem, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(notificationItem, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(notificationItem, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(notificationItem, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(notificationItem, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(notificationItem, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(notificationItem, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(notificationItem, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *notificationIcon = lv_img_create(notificationItem);

    lv_obj_set_width(notificationIcon, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(notificationIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(notificationIcon, LV_ALIGN_CENTER);
    lv_obj_add_flag(notificationIcon, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(notificationIcon, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    setNotificationIcon(notificationIcon, appId);

    lv_obj_t *notificationText = lv_label_create(notificationItem);
    lv_obj_set_width(notificationText, 140);
    lv_obj_set_height(notificationText, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(notificationText, LV_ALIGN_CENTER);
    lv_label_set_long_mode(notificationText, LV_LABEL_LONG_DOT);
    lv_label_set_text(notificationText, message);

    lv_obj_add_event_cb(notificationItem, ui_event_messageClick, LV_EVENT_CLICKED, (void *)index);
}
/**
  \fn
  \brief
  \return
*/
void addAppInfo(const void *src, const char *txt)
{
    lv_obj_t *ui_appDetailsPanel = lv_obj_create(ui_appInfoPanel);
    lv_obj_set_width(ui_appDetailsPanel, 190);
    lv_obj_set_height(ui_appDetailsPanel, 50);
    lv_obj_set_align(ui_appDetailsPanel, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_appDetailsPanel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(ui_appDetailsPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_appDetailsPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_appDetailsPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_appDetailsPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_appDetailsPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_appDetailsPanel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_appDetailsPanel, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_appDetailsPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_appDetailsPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_appDetailsPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_appDetailsPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_appDetailsIcon = lv_img_create(ui_appDetailsPanel);
    lv_img_set_src(ui_appDetailsIcon, src);
    lv_obj_set_width(ui_appDetailsIcon, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_appDetailsIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_appDetailsIcon, LV_ALIGN_LEFT_MID);
    lv_obj_add_flag(ui_appDetailsIcon, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_appDetailsIcon, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    lv_obj_t *ui_appDetailsText = lv_label_create(ui_appDetailsPanel);
    lv_obj_set_width(ui_appDetailsText, 139);
    lv_obj_set_height(ui_appDetailsText, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_appDetailsText, 43);
    lv_obj_set_y(ui_appDetailsText, 8);
    lv_label_set_text(ui_appDetailsText, txt);
    lv_label_set_recolor(ui_appDetailsText, true);
}
/**
  \fn
  \brief
  \return
*/
void addListDrive(const char *name, int total, int used, lv_event_cb_t event_cb)
{
    ui_driveInfoPanel = lv_obj_create(ui_fileManagerPanel);
    lv_obj_set_width(ui_driveInfoPanel, 240);
    lv_obj_set_height(ui_driveInfoPanel, 60);
    lv_obj_set_align(ui_driveInfoPanel, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_driveInfoPanel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(ui_driveInfoPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_driveInfoPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_driveInfoPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_driveInfoPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_driveInfoPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_driveInfoPanel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_driveInfoPanel, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_bg_color(ui_driveInfoPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_PRESSED);
    // lv_obj_set_style_bg_opa(ui_driveInfoPanel, 100, LV_PART_MAIN | LV_STATE_PRESSED);

    ui_driveInfoIcon = lv_img_create(ui_driveInfoPanel);
    lv_img_set_src(ui_driveInfoIcon, "D:/ui_img_drive.png");
    lv_obj_set_width(ui_driveInfoIcon, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_driveInfoIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_driveInfoIcon, 10);
    lv_obj_set_y(ui_driveInfoIcon, 0);
    lv_obj_set_align(ui_driveInfoIcon, LV_ALIGN_LEFT_MID);
    lv_obj_add_flag(ui_driveInfoIcon, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_driveInfoIcon, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    ui_driveInfoLabel = lv_label_create(ui_driveInfoPanel);
    lv_obj_set_width(ui_driveInfoLabel, 150);
    lv_obj_set_height(ui_driveInfoLabel, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_driveInfoLabel, 50);
    lv_obj_set_y(ui_driveInfoLabel, -7);
    lv_obj_set_align(ui_driveInfoLabel, LV_ALIGN_LEFT_MID);

    if (total >= 1048576)
    {
        lv_label_set_text_fmt(ui_driveInfoLabel, "%s - %dMB", name, (total / 1048576));
    }
    else if (total >= 1024)
    {
        lv_label_set_text_fmt(ui_driveInfoLabel, "%s - %dkB", name, (total / 1024));
    }
    else if (total == 0)
    {
        lv_label_set_text_fmt(ui_driveInfoLabel, "%s - (n/a)", name);
    }
    else
    {
        lv_label_set_text_fmt(ui_driveInfoLabel, "%s - %dB", name, total);
    }
    // lv_obj_set_style_text_font(ui_driveInfoLabel, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_driveInfoBar = lv_bar_create(ui_driveInfoPanel);
    if (total == 0)
    {
        lv_bar_set_value(ui_driveInfoBar, 0, LV_ANIM_OFF);
    }
    else
    {
        lv_bar_set_value(ui_driveInfoBar, ((used * 100) / total), LV_ANIM_OFF);
    }

    lv_bar_set_start_value(ui_driveInfoBar, 0, LV_ANIM_OFF);
    lv_obj_set_width(ui_driveInfoBar, 150);
    lv_obj_set_height(ui_driveInfoBar, 10);
    lv_obj_set_x(ui_driveInfoBar, 50);
    lv_obj_set_y(ui_driveInfoBar, 11);
    lv_obj_set_align(ui_driveInfoBar, LV_ALIGN_LEFT_MID);

    lv_obj_add_event_cb(ui_driveInfoPanel, event_cb, LV_EVENT_CLICKED, NULL);
}
/**
  \fn
  \brief
  \return
*/
void addListDir(const char *name)
{
    ui_folderInfoPanel = lv_obj_create(ui_fileManagerPanel);
    lv_obj_set_width(ui_folderInfoPanel, 240);
    lv_obj_set_height(ui_folderInfoPanel, 50);
    lv_obj_set_align(ui_folderInfoPanel, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_folderInfoPanel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(ui_folderInfoPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_folderInfoPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_folderInfoPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_folderInfoPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_folderInfoPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_folderInfoPanel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_folderInfoPanel, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_bg_color(ui_folderInfoPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_PRESSED);
    // lv_obj_set_style_bg_opa(ui_folderInfoPanel, 100, LV_PART_MAIN | LV_STATE_PRESSED);

    ui_folderInfoIcon = lv_img_create(ui_folderInfoPanel);
    lv_img_set_src(ui_folderInfoIcon, "D:/ui_img_directory.png");
    lv_obj_set_width(ui_folderInfoIcon, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_folderInfoIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_folderInfoIcon, 10);
    lv_obj_set_y(ui_folderInfoIcon, 0);
    lv_obj_set_align(ui_folderInfoIcon, LV_ALIGN_LEFT_MID);
    lv_obj_add_flag(ui_folderInfoIcon, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_folderInfoIcon, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    ui_folderInfoName = lv_label_create(ui_folderInfoPanel);
    lv_obj_set_width(ui_folderInfoName, 150);
    lv_obj_set_height(ui_folderInfoName, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_folderInfoName, 50);
    lv_obj_set_y(ui_folderInfoName, 0);
    lv_obj_set_align(ui_folderInfoName, LV_ALIGN_LEFT_MID);
    lv_label_set_text(ui_folderInfoName, name);
    // lv_obj_set_style_text_font(ui_folderInfoName, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
}
/**
  \fn
  \brief
  \return
*/
void addListFile(const char *name, int size)
{
    ui_fileInfoPanel = lv_obj_create(ui_fileManagerPanel);
    lv_obj_set_width(ui_fileInfoPanel, 240);
    lv_obj_set_height(ui_fileInfoPanel, 50);
    lv_obj_set_align(ui_fileInfoPanel, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_fileInfoPanel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(ui_fileInfoPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_fileInfoPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_fileInfoPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_fileInfoPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_fileInfoPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_fileInfoPanel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_fileInfoPanel, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_bg_color(ui_fileInfoPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_PRESSED);
    // lv_obj_set_style_bg_opa(ui_fileInfoPanel, 100, LV_PART_MAIN | LV_STATE_PRESSED);

    ui_fileInfoIcon = lv_img_create(ui_fileInfoPanel);
    // lv_img_set_src(ui_fileInfoIcon, &ui_img_file_png);
    lv_img_set_src(ui_fileInfoIcon, "D:/ui_img_file.png");
    lv_obj_set_width(ui_fileInfoIcon, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_fileInfoIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_fileInfoIcon, 10);
    lv_obj_set_y(ui_fileInfoIcon, 0);
    lv_obj_set_align(ui_fileInfoIcon, LV_ALIGN_LEFT_MID);
    lv_obj_add_flag(ui_fileInfoIcon, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_fileInfoIcon, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    ui_fileInfoName = lv_label_create(ui_fileInfoPanel);
    lv_obj_set_width(ui_fileInfoName, 150);
    lv_obj_set_height(ui_fileInfoName, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_fileInfoName, 50);
    lv_obj_set_y(ui_fileInfoName, -7);
    lv_obj_set_align(ui_fileInfoName, LV_ALIGN_LEFT_MID);
    lv_label_set_text(ui_fileInfoName, name);
    // lv_obj_set_style_text_font(ui_fileInfoName, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_fileInfoSize = lv_label_create(ui_fileInfoPanel);
    lv_obj_set_width(ui_fileInfoSize, 150);
    lv_obj_set_height(ui_fileInfoSize, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_fileInfoSize, 50);
    lv_obj_set_y(ui_fileInfoSize, 11);
    lv_obj_set_align(ui_fileInfoSize, LV_ALIGN_LEFT_MID);
    if (size >= 1048576)
    {
        lv_label_set_text_fmt(ui_fileInfoSize, "%dmb", (size / 1048576));
    }
    else if (size >= 1024)
    {
        lv_label_set_text_fmt(ui_fileInfoSize, "%dkb", (size / 1024));
    }
    else
    {
        lv_label_set_text_fmt(ui_fileInfoSize, "%db", size);
    }
    lv_obj_set_style_text_align(ui_fileInfoSize, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_text_font(ui_fileInfoSize, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
}
/**
  \fn
  \brief
  \return
*/
void addListBack(lv_event_cb_t event_cb)
{
    ui_driveBackPanel = lv_obj_create(ui_fileManagerPanel);
    lv_obj_set_width(ui_driveBackPanel, 240);
    lv_obj_set_height(ui_driveBackPanel, 50);
    lv_obj_set_align(ui_driveBackPanel, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_driveBackPanel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(ui_driveBackPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_driveBackPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_driveBackPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_driveBackPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_driveBackPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_driveBackPanel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_driveBackPanel, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_bg_color(ui_driveBackPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_PRESSED);
    // lv_obj_set_style_bg_opa(ui_driveBackPanel, 100, LV_PART_MAIN | LV_STATE_PRESSED);

    ui_driveBackIcon = lv_img_create(ui_driveBackPanel);
    lv_img_set_src(ui_driveBackIcon, "D:/ui_img_back.png");
    lv_obj_set_width(ui_driveBackIcon, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_driveBackIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_driveBackIcon, 10);
    lv_obj_set_y(ui_driveBackIcon, 0);
    lv_obj_set_align(ui_driveBackIcon, LV_ALIGN_LEFT_MID);
    lv_obj_add_flag(ui_driveBackIcon, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_driveBackIcon, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    ui_driveBackLabel = lv_label_create(ui_driveBackPanel);
    lv_obj_set_width(ui_driveBackLabel, 150);
    lv_obj_set_height(ui_driveBackLabel, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_driveBackLabel, 50);
    lv_obj_set_y(ui_driveBackLabel, 0);
    lv_obj_set_align(ui_driveBackLabel, LV_ALIGN_LEFT_MID);
    lv_label_set_text(ui_driveBackLabel,  "返回");//"Back"
    lv_obj_add_event_cb(ui_driveBackPanel, event_cb, LV_EVENT_CLICKED, NULL);
}
/**
  \fn
  \brief
  \return
*/
void registerWatchface_cb(const char *name, const lv_img_dsc_t *preview, lv_obj_t **watchface)
{
    if (numFaces >= MAX_FACES)
    {
        return;
    }
    faces[numFaces].name = name;
    faces[numFaces].preview = preview;
    faces[numFaces].watchface = watchface;
    addWatchface(faces[numFaces].name, faces[numFaces].preview, numFaces);
    numFaces++;
}
/**
  \fn
  \brief
  \return
*/
void registerGame_cb(const char *name, const lv_img_dsc_t *icon, lv_obj_t **game)
{
    if (numGames >= MAX_GAMES)
    {
        return;
    }
    games[numGames].name = name;
    games[numGames].preview = icon;
    games[numGames].watchface = game;
    // addGame
    add_gameList(games[numGames].name, numGames, games[numGames].preview);
    numGames++;
}
/**
  \fn
  \brief
  \return
*/
void init_face_select()
{
    ui_faceSelect = lv_obj_create(NULL);
    lv_obj_set_width(ui_faceSelect, 240);
    lv_obj_set_height(ui_faceSelect, 240);
    lv_obj_set_align(ui_faceSelect, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow(ui_faceSelect, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ui_faceSelect, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(ui_faceSelect, LV_OBJ_FLAG_SNAPPABLE); /// Flags
    lv_obj_set_scrollbar_mode(ui_faceSelect, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_snap_x(ui_faceSelect, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_style_radius(ui_faceSelect, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_faceSelect, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_faceSelect, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_faceSelect, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_faceSelect, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_faceSelect, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_faceSelect, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_faceSelect, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(ui_faceSelect, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(ui_faceSelect, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
}
/**
  \fn
  \brief
  \return
*/
void showError(const char *title, const char *message)
{
    lv_disp_t *display = lv_disp_get_default();
    lv_obj_t *actScr = lv_disp_get_scr_act(display);

    // attach the error windows to current screen
    lv_obj_set_parent(ui_errorWindow, actScr);

    lv_label_set_text(ui_errorTitle, title);
    lv_label_set_text(ui_errorMessage, message);
    lv_obj_clear_flag(ui_errorWindow, LV_OBJ_FLAG_HIDDEN); // show the error
    lv_obj_scroll_to_y(ui_errorWindow, 0, LV_ANIM_ON);
}

///////////////////// SCREENS ////////////////////
void ui_clockScreen_screen_init(void)
{
    ui_clockScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_clockScreen, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_bg_color(ui_clockScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_clockScreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_radius(ui_clockScreen, 120, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_bg_img_src(ui_clockScreen, "D:/night-sky.png", LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_outline_color(ui_clockScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_outline_opa(ui_clockScreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_outline_width(ui_clockScreen, 55, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_outline_pad(ui_clockScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_hourLabel = lv_label_create(ui_clockScreen);
    lv_obj_set_width(ui_hourLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_hourLabel, LV_SIZE_CONTENT);
    // lv_obj_set_x(ui_hourLabel, -50);
    // lv_obj_set_y(ui_hourLabel, -32);
    // lv_obj_set_align(ui_hourLabel, LV_ALIGN_CENTER);
    lv_obj_align(ui_hourLabel,LV_ALIGN_CENTER,-48,-40);
    lv_label_set_text(ui_hourLabel, "12");
    lv_obj_set_style_text_align(ui_hourLabel, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_hourLabel, pFontNumber, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_minuteLabel = lv_label_create(ui_clockScreen);
    lv_obj_set_width(ui_minuteLabel, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_minuteLabel, LV_SIZE_CONTENT); /// 1
    // lv_obj_set_x(ui_minuteLabel, 32);
    // lv_obj_set_y(ui_minuteLabel, 60);
    // lv_obj_set_align(ui_minuteLabel, LV_ALIGN_CENTER);
    lv_obj_align(ui_minuteLabel,LV_ALIGN_CENTER,32,48);
    lv_label_set_text(ui_minuteLabel, "28");
    lv_obj_set_style_text_font(ui_minuteLabel, pFontNumber, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_dateLabel = lv_label_create(ui_clockScreen);
    lv_obj_set_width(ui_dateLabel, 113);
    lv_obj_set_height(ui_dateLabel, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_dateLabel, 89);
    lv_obj_set_y(ui_dateLabel, -24);
    lv_obj_set_align(ui_dateLabel, LV_ALIGN_CENTER);
    lv_label_set_long_mode(ui_dateLabel, LV_LABEL_LONG_CLIP);
    lv_label_set_text(ui_dateLabel, "08\nJuly");
    // lv_obj_set_style_text_font(ui_dateLabel, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_weatherIcon = lv_img_create(ui_clockScreen);
    lv_img_set_src(ui_weatherIcon, "D:/ui_img_weather_1.png");
    lv_obj_set_width(ui_weatherIcon, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_weatherIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_weatherIcon, -63);
    lv_obj_set_y(ui_weatherIcon, 29);
    lv_obj_set_align(ui_weatherIcon, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_weatherIcon, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_weatherIcon, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    ui_weatherTemp = lv_label_create(ui_clockScreen);
    lv_obj_set_width(ui_weatherTemp, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_weatherTemp, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_weatherTemp, -61);
    lv_obj_set_y(ui_weatherTemp, 72);
    lv_obj_set_align(ui_weatherTemp, LV_ALIGN_CENTER);
    lv_label_set_text(ui_weatherTemp, "15 °C");
    lv_obj_set_style_text_font(ui_weatherTemp, pFontExtern_28, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_dayLabel = lv_label_create(ui_clockScreen);
    lv_obj_set_width(ui_dayLabel, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_dayLabel, LV_SIZE_CONTENT); /// 1
    // lv_obj_set_x(ui_dayLabel, 0);
    // lv_obj_set_y(ui_dayLabel, -100);
    // lv_obj_set_align(ui_dayLabel, LV_ALIGN_CENTER);
    lv_obj_align(ui_dayLabel,LV_ALIGN_TOP_MID,0,8);
    lv_label_set_text(ui_dayLabel, "星期天");

    ui_amPmLabel = lv_label_create(ui_clockScreen);
    lv_obj_set_width(ui_amPmLabel, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_amPmLabel, LV_SIZE_CONTENT); /// 1
    // lv_obj_set_x(ui_amPmLabel, 12);
    // lv_obj_set_y(ui_amPmLabel, 105);
    // lv_obj_set_align(ui_amPmLabel, LV_ALIGN_CENTER);
    lv_obj_align(ui_amPmLabel,LV_ALIGN_BOTTOM_MID,0,-8);
    lv_label_set_text(ui_amPmLabel, "PM");
#if 0
    ui_alertPanel = lv_obj_create(ui_clockScreen);
    lv_obj_set_width(ui_alertPanel, 200);
    lv_obj_set_height(ui_alertPanel, 55);
    lv_obj_set_align(ui_alertPanel, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_alertPanel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ui_alertPanel, LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_bg_color(ui_alertPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_alertPanel, 240, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_alertPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_alertPanel, 240, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_alertPanel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_alertPanel, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_alertPanel, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_alertPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_alertPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_alertIcon = lv_img_create(ui_alertPanel);
    lv_img_set_src(ui_alertIcon, "D:/ui_img_wechat.png");
    lv_obj_set_width(ui_alertIcon, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_alertIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_alertIcon, LV_ALIGN_LEFT_MID);
    lv_obj_add_flag(ui_alertIcon, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_alertIcon, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    ui_alertText = lv_label_create(ui_alertPanel);
    lv_obj_set_width(ui_alertText, 142);
    lv_obj_set_height(ui_alertText, 40);
    lv_obj_set_x(ui_alertText, 39);
    lv_obj_set_y(ui_alertText, 0);
    lv_obj_set_align(ui_alertText, LV_ALIGN_LEFT_MID);
    lv_label_set_long_mode(ui_alertText, LV_LABEL_LONG_DOT);
    lv_label_set_text(ui_alertText,"收到一条消息");

    lv_obj_add_event_cb(ui_alertPanel, ui_event_alertPanel, LV_EVENT_ALL, NULL);
#endif
    lv_obj_add_event_cb(ui_clockScreen, watchfaceEvents, LV_EVENT_ALL, NULL);
}
/**
  \fn
  \brief
  \return
*/
void ui_weatherScreen_screen_init(void)
{
    ui_weatherScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_weatherScreen, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    // lv_obj_set_style_radius(ui_weatherScreen, 120, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_weatherScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_weatherScreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_bg_img_src(ui_weatherScreen, &ui_img_857483832, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_bg_img_opa(ui_weatherScreen, 150, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_outline_color(ui_weatherScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_outline_opa(ui_weatherScreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_outline_width(ui_weatherScreen, 55, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_outline_pad(ui_weatherScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_weatherPanel = lv_obj_create(ui_weatherScreen);
    lv_obj_set_width(ui_weatherPanel, lv_pct(100));
    lv_obj_set_height(ui_weatherPanel, lv_pct(100));
    lv_obj_set_align(ui_weatherPanel, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_weatherPanel, LV_OBJ_FLAG_HIDDEN);       /// Flags
    lv_obj_clear_flag(ui_weatherPanel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(ui_weatherPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_weatherPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_weatherPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_weatherPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_weatherCity = lv_label_create(ui_weatherPanel);
    lv_obj_set_width(ui_weatherCity, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_weatherCity, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_weatherCity, -3);
    lv_obj_set_y(ui_weatherCity, -85);
    lv_obj_set_align(ui_weatherCity, LV_ALIGN_CENTER);
    lv_label_set_text(ui_weatherCity, "");

    ui_weatherCurrentIcon = lv_img_create(ui_weatherPanel);
    lv_img_set_src(ui_weatherCurrentIcon, "D:/ui_img_weather_8.png");
    lv_obj_set_width(ui_weatherCurrentIcon, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_weatherCurrentIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_weatherCurrentIcon, 0);
    lv_obj_set_y(ui_weatherCurrentIcon, -29);
    lv_obj_set_align(ui_weatherCurrentIcon, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_weatherCurrentIcon, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_weatherCurrentIcon, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_img_set_zoom(ui_weatherCurrentIcon, 400);

    ui_weatherCurrentTemp = lv_label_create(ui_weatherPanel);
    lv_obj_set_width(ui_weatherCurrentTemp, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_weatherCurrentTemp, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_weatherCurrentTemp, 0);
    lv_obj_set_y(ui_weatherCurrentTemp, 44);
    lv_obj_set_align(ui_weatherCurrentTemp, LV_ALIGN_CENTER);
    lv_label_set_text(ui_weatherCurrentTemp, "--°C");
    lv_obj_set_style_text_font(ui_weatherCurrentTemp, pFontExtern_48, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_weatherUpdateTime = lv_label_create(ui_weatherPanel);
    lv_obj_set_width(ui_weatherUpdateTime, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_weatherUpdateTime, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_weatherUpdateTime, 0);
    lv_obj_set_y(ui_weatherUpdateTime, 97);
    lv_obj_set_align(ui_weatherUpdateTime, LV_ALIGN_CENTER);
    watch_lable.weather[0] = "更新于\n--:--";    //"updated at\n--:--"
    lv_label_set_text(ui_weatherUpdateTime, watch_lable.weather[0]);
    lv_obj_set_style_text_align(ui_weatherUpdateTime, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_forecastPanel = lv_obj_create(ui_weatherScreen);
    lv_obj_set_width(ui_forecastPanel, lv_pct(100));
    lv_obj_set_height(ui_forecastPanel, lv_pct(100));
    // lv_obj_set_style_radius(ui_forecastPanel, 120, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_align(ui_forecastPanel, LV_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(ui_forecastPanel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(ui_forecastPanel, LV_DIR_VER);
    lv_obj_set_style_radius(ui_forecastPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_forecastPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_forecastPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_forecastPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_forecastTitle = lv_label_create(ui_forecastPanel);
    lv_obj_set_width(ui_forecastTitle, 150);
    lv_obj_set_height(ui_forecastTitle, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_forecastTitle, LV_ALIGN_TOP_MID);
    watch_lable.weather[1] = "天气预报";    //"Forecast"
    lv_label_set_text(ui_forecastTitle, watch_lable.weather[1]);
    lv_obj_set_style_text_align(ui_forecastTitle, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_forecastTitle, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_forecastTitle, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_forecastTitle, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_forecastTitle, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_forecastTitle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_forecastTitle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_forecastTitle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_forecastTitle, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_forecastList = lv_obj_create(ui_forecastPanel);
    lv_obj_set_width(ui_forecastList, 240);
    lv_obj_set_height(ui_forecastList, LV_SIZE_CONTENT); /// 50
    lv_obj_set_x(ui_forecastList, 0);
    lv_obj_set_y(ui_forecastList, 31);
    lv_obj_set_align(ui_forecastList, LV_ALIGN_TOP_MID);
    lv_obj_set_flex_flow(ui_forecastList, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_forecastList, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(ui_forecastList, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(ui_forecastList, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_forecastList, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_forecastList, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_forecastList, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_forecastList, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_forecastList, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_forecastList, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_forecastList, 50, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *info = lv_label_create(ui_forecastList);
    lv_obj_set_width(info, 180);
    lv_obj_set_height(info, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(info, LV_ALIGN_CENTER);
    // lv_label_set_text(info, "Weather information has not yet been synced. Make sure to enable it in the app settings.");
    lv_label_set_text(info, "天气信息未同步. 请确保已经打开了相关配置.");

    lv_obj_add_event_cb(ui_weatherScreen, ui_event_weatherScreen, LV_EVENT_ALL, NULL);
}
/**
  \fn
  \brief
  \return
*/
void ui_appList_screen_init(void)
{
    ui_appListScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_appListScreen, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_bg_color(ui_appListScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_appListScreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_appList = lv_obj_create(ui_appListScreen);
    lv_obj_set_width(ui_appList, lv_pct(100));
    lv_obj_set_height(ui_appList, lv_pct(100));
    lv_obj_set_align(ui_appList, LV_ALIGN_TOP_MID);
    lv_obj_set_flex_flow(ui_appList, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_appList, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(ui_appList, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(ui_appList, LV_DIR_VER);
    lv_obj_set_style_radius(ui_appList, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_appList, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_appList, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_appList, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_appList, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_appList, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_appList, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_appList, 70, LV_PART_MAIN | LV_STATE_DEFAULT);
    for (uint32_t i=0; i<gFirstMenuCount; i++)
    {
        add_appList(gFirstMenuName[i], gFirstMenuId[i], gFirstMenuImage[i]);
    }
    lv_obj_add_event_cb(ui_appList, onScroll, LV_EVENT_SCROLL, NULL);
    lv_obj_add_event_cb(ui_appListScreen, ui_event_appListScreen, LV_EVENT_ALL, NULL);
}
/**
  \fn
  \brief
  \return
*/
void ui_gameListScreen_screen_init(void)
{
    ui_gameListScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_gameListScreen, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_bg_color(ui_gameListScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_gameListScreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_gameList = lv_obj_create(ui_gameListScreen);
    lv_obj_set_width(ui_gameList, lv_pct(100));
    lv_obj_set_height(ui_gameList, lv_pct(100));
    lv_obj_set_align(ui_gameList, LV_ALIGN_TOP_MID);
    lv_obj_set_flex_flow(ui_gameList, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_gameList, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(ui_gameList, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(ui_gameList, LV_DIR_VER);
    lv_obj_set_style_radius(ui_gameList, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_gameList, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_gameList, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_gameList, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_gameList, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_gameList, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_gameList, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_gameList, 70, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(ui_gameList, onScroll, LV_EVENT_SCROLL, NULL);
    lv_obj_add_event_cb(ui_gameListScreen, ui_event_gameListScreen, LV_EVENT_ALL, NULL);
}
/**
  \fn
  \brief
  \return
*/
void ui_notificationScreen_screen_init(void)
{
    ui_notificationScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_notificationScreen, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_bg_color(ui_notificationScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_notificationScreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_messagePanel = lv_obj_create(ui_notificationScreen);
    lv_obj_set_width(ui_messagePanel, lv_pct(100));
    lv_obj_set_height(ui_messagePanel, lv_pct(100));
    lv_obj_set_align(ui_messagePanel, LV_ALIGN_TOP_MID);
    lv_obj_set_flex_flow(ui_messagePanel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_messagePanel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(ui_messagePanel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(ui_messagePanel, LV_DIR_VER);
    lv_obj_set_style_radius(ui_messagePanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_messagePanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_messagePanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_messagePanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_messagePanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_messagePanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_messagePanel, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_messagePanel, 70, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_messageIcon = lv_img_create(ui_messagePanel);
    lv_img_set_src(ui_messageIcon, "D:/ui_img_chrns.png");
    lv_obj_set_width(ui_messageIcon, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_messageIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_messageIcon, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_messageIcon, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_messageIcon, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    ui_messageTime = lv_label_create(ui_messagePanel);
    lv_obj_set_width(ui_messageTime, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_messageTime, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_messageTime, LV_ALIGN_CENTER);
    lv_label_set_text(ui_messageTime, "Chronos");

    ui_messageContent = lv_label_create(ui_messagePanel);
    lv_obj_set_width(ui_messageContent, 180);
    lv_obj_set_height(ui_messageContent, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_messageContent, LV_ALIGN_CENTER);
    lv_label_set_text(ui_messageContent, "Download from Google Play to sync time and receive notifications");

    ui_messageList = lv_obj_create(ui_notificationScreen);
    lv_obj_set_width(ui_messageList, lv_pct(100));
    lv_obj_set_height(ui_messageList, lv_pct(100));
    lv_obj_set_align(ui_messageList, LV_ALIGN_TOP_MID);
    lv_obj_set_flex_flow(ui_messageList, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_messageList, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(ui_messageList, LV_OBJ_FLAG_HIDDEN); /// Flags
    lv_obj_set_scrollbar_mode(ui_messageList, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(ui_messageList, LV_DIR_VER);
    lv_obj_set_style_radius(ui_messageList, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_messageList, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_messageList, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_messageList, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_messageList, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_messageList, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_messageList, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_messageList, 70, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(ui_messageList, onScroll, LV_EVENT_SCROLL, NULL);

    lv_obj_add_event_cb(ui_notificationScreen, ui_event_notificationScreen, LV_EVENT_ALL, NULL);
}
/**
  \fn
  \brief    三级目录，设置页面
  \return
*/
void ui_settings_screen_init(void)
{
    ui_settingsScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_settingsScreen, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(ui_settingsScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_settingsScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_settingsScreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_settingsList = lv_obj_create(ui_settingsScreen);
    lv_obj_set_width(ui_settingsList, lv_pct(100));
    lv_obj_set_height(ui_settingsList, lv_pct(100));
    lv_obj_set_align(ui_settingsList, LV_ALIGN_TOP_MID);
    lv_obj_set_flex_flow(ui_settingsList, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_settingsList, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(ui_settingsList, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(ui_settingsList, LV_DIR_VER);
    lv_obj_set_style_radius(ui_settingsList, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_settingsList, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_settingsList, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_settingsList, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_settingsList, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_settingsList, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_settingsList, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_settingsList, 70, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_settingsTitle = lv_label_create(ui_settingsList);
    lv_obj_set_width(ui_settingsTitle, 150);
    lv_obj_set_height(ui_settingsTitle, LV_SIZE_CONTENT);
    // lv_obj_set_align(ui_settingsTitle, LV_ALIGN_CENTER);
    lv_label_set_text(ui_settingsTitle, watch_lable.settings[0]);
    lv_obj_align(ui_settingsTitle,LV_ALIGN_CENTER,0,-4);
    lv_obj_set_style_text_align(ui_settingsTitle, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_text_font(ui_settingsTitle, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_settingsTitle, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_settingsTitle, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_settingsTitle, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_settingsTitle, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_settingsTitle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_settingsTitle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_settingsTitle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_settingsTitle, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_brightnessPanel = lv_obj_create(ui_settingsList);
    lv_obj_set_width(ui_brightnessPanel, 200);
    lv_obj_set_height(ui_brightnessPanel, 64);
    lv_obj_set_align(ui_brightnessPanel, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_brightnessPanel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(ui_brightnessPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_brightnessPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_brightnessPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_brightnessPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_brightnessPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_brightnessPanel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_brightnessPanel, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_brightnessPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_brightnessPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_brightnessPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_brightnessPanel, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_brightnessSlider = lv_slider_create(ui_brightnessPanel);
    lv_slider_set_range(ui_brightnessSlider, 1, 100);
    lv_slider_set_value(ui_brightnessSlider, lcdBackLightVal(), LV_ANIM_OFF);
    if (lv_slider_get_mode(ui_brightnessSlider) == LV_SLIDER_MODE_RANGE)
        lv_slider_set_left_value(ui_brightnessSlider, 0, LV_ANIM_OFF);
    lv_obj_set_width(ui_brightnessSlider, 123);
    lv_obj_set_height(ui_brightnessSlider, 10);
    lv_obj_set_x(ui_brightnessSlider, 24);
    lv_obj_set_y(ui_brightnessSlider, 12);
    lv_obj_set_align(ui_brightnessSlider, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_brightnessSlider, LV_OBJ_FLAG_GESTURE_BUBBLE); /// Flags
    lv_obj_add_event_cb(ui_brightnessSlider, ui_event_brightness_slider_cb, LV_EVENT_ALL, NULL);

    ui_brightnessIcon = lv_img_create(ui_brightnessPanel);
    lv_img_set_src(ui_brightnessIcon, "D:/ui_img_brightness.png");
    lv_obj_set_width(ui_brightnessIcon, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_brightnessIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_brightnessIcon, -74);
    lv_obj_set_y(ui_brightnessIcon, 2);
    lv_obj_set_align(ui_brightnessIcon, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_brightnessIcon, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_brightnessIcon, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_img_set_zoom(ui_brightnessIcon, 150);

    ui_brightnessLabel = lv_label_create(ui_brightnessPanel);
    lv_obj_set_width(ui_brightnessLabel, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_brightnessLabel, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_brightnessLabel, 53);
    lv_obj_set_y(ui_brightnessLabel, 3);
    lv_label_set_text(ui_brightnessLabel, watch_lable.settings[1]);

    ui_scrollingPanel = lv_obj_create(ui_settingsList);
    lv_obj_set_width(ui_scrollingPanel, 200);
    lv_obj_set_height(ui_scrollingPanel, 64);
    lv_obj_set_align(ui_scrollingPanel, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_scrollingPanel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(ui_scrollingPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_scrollingPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_scrollingPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_scrollingPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_scrollingPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_scrollingPanel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_scrollingPanel, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_scrollingPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_scrollingPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_scrollingPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_scrollingPanel, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_icon_scrolling = lv_img_create(ui_scrollingPanel);
    lv_img_set_src(ui_icon_scrolling, "D:/ui_icon_scrolling.png");
    lv_obj_set_width(ui_icon_scrolling, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_icon_scrolling, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_icon_scrolling, -74);
    lv_obj_set_y(ui_icon_scrolling, 2);
    lv_obj_set_align(ui_icon_scrolling, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_icon_scrolling, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_icon_scrolling, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_img_set_zoom(ui_icon_scrolling, 150);

    ui_Switch2 = lv_switch_create(ui_scrollingPanel);
    lv_obj_set_width(ui_Switch2, 50);
    lv_obj_set_height(ui_Switch2, 25);
    lv_obj_set_x(ui_Switch2, 57);
    lv_obj_set_y(ui_Switch2, 29);

    ui_scrollLabel = lv_label_create(ui_scrollingPanel);
    lv_obj_set_width(ui_scrollLabel, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_scrollLabel, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_scrollLabel, 54);
    lv_obj_set_y(ui_scrollLabel, 3);
    lv_label_set_text(ui_scrollLabel, watch_lable.settings[2]);

    ui_timeoutPanel = lv_obj_create(ui_settingsList);
    lv_obj_set_width(ui_timeoutPanel, 200);
    lv_obj_set_height(ui_timeoutPanel, 64);
    lv_obj_set_x(ui_timeoutPanel, 37);
    lv_obj_set_y(ui_timeoutPanel, 7);
    lv_obj_set_align(ui_timeoutPanel, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_timeoutPanel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(ui_timeoutPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_timeoutPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_timeoutPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_timeoutPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_timeoutPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_timeoutPanel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_timeoutPanel, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_timeoutPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_timeoutPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_timeoutPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_timeoutPanel, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_timeoutSelect = lv_dropdown_create(ui_timeoutPanel);
    lv_dropdown_set_options(ui_timeoutSelect, "5 S\n10 S\n20 S\n30 S\nAlways On");  // 熄屏延时: 5秒 10秒 20秒 30秒 常亮
    lv_obj_set_width(ui_timeoutSelect, 120);
    lv_obj_set_height(ui_timeoutSelect, LV_SIZE_CONTENT);
    lv_obj_set_x(ui_timeoutSelect, 20);
    lv_obj_set_y(ui_timeoutSelect, 10);
    lv_obj_set_align(ui_timeoutSelect, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_timeoutSelect, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    lv_obj_set_style_bg_color(ui_timeoutSelect, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_timeoutSelect, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_timeoutSelect, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_timeoutSelect, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_timeoutSelect, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(lv_dropdown_get_list(ui_timeoutSelect), lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(lv_dropdown_get_list(ui_timeoutSelect), 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_timeoutIcon = lv_img_create(ui_timeoutPanel);
    lv_img_set_src(ui_timeoutIcon, "D:/ui_img_timeout.png");
    lv_obj_set_width(ui_timeoutIcon, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_timeoutIcon, LV_SIZE_CONTENT);
    lv_obj_set_x(ui_timeoutIcon, -74);
    lv_obj_set_y(ui_timeoutIcon, 2);
    lv_obj_set_align(ui_timeoutIcon, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_timeoutIcon, LV_OBJ_FLAG_ADV_HITTEST);
    lv_obj_clear_flag(ui_timeoutIcon, LV_OBJ_FLAG_SCROLLABLE);
    lv_img_set_zoom(ui_timeoutIcon, 150);

    ui_timeoutLabel = lv_label_create(ui_timeoutPanel);
    lv_obj_set_width(ui_timeoutLabel, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_timeoutLabel, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_timeoutLabel, 61);
    lv_obj_set_y(ui_timeoutLabel, 0);
    lv_label_set_text(ui_timeoutLabel, watch_lable.settings[3]);

    ui_languagePanel = lv_obj_create(ui_settingsList);
    lv_obj_set_width(ui_languagePanel, 200);
    lv_obj_set_height(ui_languagePanel, 64);
    lv_obj_set_x(ui_languagePanel, 37);
    lv_obj_set_y(ui_languagePanel, 7);
    lv_obj_set_align(ui_languagePanel, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_languagePanel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_languagePanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_languagePanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_languagePanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_languagePanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_languagePanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_languagePanel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_languagePanel, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_languagePanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_languagePanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_languagePanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_languagePanel, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_languageSelect = lv_dropdown_create(ui_languagePanel);
    lv_dropdown_set_options(ui_languageSelect, "中文\nEnglish");
    lv_obj_set_width(ui_languageSelect, 120);
    lv_obj_set_height(ui_languageSelect, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_languageSelect, 20);
    lv_obj_set_y(ui_languageSelect, 10);
    lv_obj_set_align(ui_languageSelect, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_languageSelect, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    lv_obj_set_style_bg_color(ui_languageSelect, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_languageSelect, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_languageSelect, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_languageSelect, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_languageSelect, 1, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(lv_dropdown_get_list(ui_languageSelect), lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(lv_dropdown_get_list(ui_languageSelect), 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_languageIcon = lv_img_create(ui_languagePanel);
    lv_img_set_src(ui_languageIcon, "D:/ui_img_language.png");
    lv_obj_set_width(ui_languageIcon, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_languageIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_languageIcon, -74);
    lv_obj_set_y(ui_languageIcon, 2);
    lv_obj_set_align(ui_languageIcon, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_languageIcon, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_languageIcon, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_img_set_zoom(ui_languageIcon, 150);

    ui_languageLabel = lv_label_create(ui_languagePanel);
    lv_obj_set_width(ui_languageLabel, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_languageLabel, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_languageLabel, 61);
    lv_obj_set_y(ui_languageLabel, 0);
    lv_label_set_text(ui_languageLabel, watch_lable.settings[5]); 
    // lv_label_set_text(ui_languageLabel, "语言"); //"Language"

    ui_alertStatePanel = lv_obj_create(ui_settingsList);
    lv_obj_set_width(ui_alertStatePanel, 200);
    lv_obj_set_height(ui_alertStatePanel, 64);
    lv_obj_set_align(ui_alertStatePanel, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_alertStatePanel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(ui_alertStatePanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_alertStatePanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_alertStatePanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_alertStatePanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_alertStatePanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_alertStatePanel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_alertStatePanel, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_alertStatePanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_alertStatePanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_alertStatePanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_alertStatePanel, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_alertStateIcon = lv_img_create(ui_alertStatePanel);
    lv_img_set_src(ui_alertStateIcon, "D:/ui_icon_alert.png");
    lv_obj_set_width(ui_alertStateIcon, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_alertStateIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_alertStateIcon, -74);
    lv_obj_set_y(ui_alertStateIcon, 2);
    lv_obj_set_align(ui_alertStateIcon, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_alertStateIcon, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_alertStateIcon, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_img_set_zoom(ui_alertStateIcon, 150);

    ui_alertStateSwitch = lv_switch_create(ui_alertStatePanel);
    lv_obj_set_width(ui_alertStateSwitch, 50);
    lv_obj_set_height(ui_alertStateSwitch, 25);
    lv_obj_set_x(ui_alertStateSwitch, 57);
    lv_obj_set_y(ui_alertStateSwitch, 29);

    ui_alertStateLabel = lv_label_create(ui_alertStatePanel);
    lv_obj_set_width(ui_alertStateLabel, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_alertStateLabel, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_alertStateLabel, 54);
    lv_obj_set_y(ui_alertStateLabel, 3);
    lv_label_set_text(ui_alertStateLabel, watch_lable.settings[4]);

    ui_batteryPanel = lv_obj_create(ui_settingsList);
    lv_obj_set_width(ui_batteryPanel, 200);
    lv_obj_set_height(ui_batteryPanel, 64);
    lv_obj_set_align(ui_batteryPanel, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_batteryPanel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(ui_batteryPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_batteryPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_batteryPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_batteryPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_batteryPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_batteryPanel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_batteryPanel, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_batteryPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_batteryPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_batteryPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_batteryPanel, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_batterySlider = lv_slider_create(ui_batteryPanel);
    lv_slider_set_value(ui_batterySlider, 50, LV_ANIM_OFF);
    if (lv_slider_get_mode(ui_batterySlider) == LV_SLIDER_MODE_RANGE)
        lv_slider_set_left_value(ui_batterySlider, 0, LV_ANIM_OFF);
    lv_obj_set_width(ui_batterySlider, 130);
    lv_obj_set_height(ui_batterySlider, 10);
    lv_obj_set_x(ui_batterySlider, 22);
    lv_obj_set_y(ui_batterySlider, 12);
    lv_obj_set_align(ui_batterySlider, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_batterySlider, LV_OBJ_FLAG_GESTURE_BUBBLE); /// Flags

    lv_obj_set_style_bg_color(ui_batterySlider, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_batterySlider, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

    ui_batteryIcon = lv_img_create(ui_batteryPanel);
    lv_img_set_src(ui_batteryIcon, "D:/ui_img_bat.png");
    lv_obj_set_width(ui_batteryIcon, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_batteryIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_batteryIcon, -74);
    lv_obj_set_y(ui_batteryIcon, 2);
    lv_obj_set_align(ui_batteryIcon, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_batteryIcon, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_batteryIcon, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_img_set_zoom(ui_batteryIcon, 150);

    ui_batteryLabel = lv_label_create(ui_batteryPanel);
    lv_obj_set_width(ui_batteryLabel, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_batteryLabel, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_batteryLabel, 53);
    lv_obj_set_y(ui_batteryLabel, 3);
    // lv_label_set_text(ui_batteryLabel, "Battery 50%");
    lv_label_set_text_fmt(ui_batteryLabel, "电量 %d%%", 50);

    ui_aboutPanel = lv_obj_create(ui_settingsList);
    lv_obj_set_width(ui_aboutPanel, 200);
    lv_obj_set_height(ui_aboutPanel, LV_SIZE_CONTENT); /// 64
    lv_obj_set_align(ui_aboutPanel, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_aboutPanel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(ui_aboutPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_aboutPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_aboutPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_aboutPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_aboutPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_aboutPanel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_aboutPanel, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_aboutPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_aboutPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_aboutPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_aboutPanel, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_aboutIcon = lv_img_create(ui_aboutPanel);
    lv_img_set_src(ui_aboutIcon, "D:/ui_img_info.png");
    lv_obj_set_width(ui_aboutIcon, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_aboutIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_aboutIcon, -5);
    lv_obj_set_y(ui_aboutIcon, 2);
    lv_obj_add_flag(ui_aboutIcon, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_aboutIcon, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_img_set_zoom(ui_aboutIcon, 150);

    ui_aboutText = lv_label_create(ui_aboutPanel);
    lv_obj_set_width(ui_aboutText, 130);
    lv_obj_set_height(ui_aboutText, LV_SIZE_CONTENT);
    lv_obj_set_x(ui_aboutText, 60);
    lv_obj_set_y(ui_aboutText, 7);
    lv_label_set_text_fmt(ui_aboutText, "%s\n%s\n%s", CHIP_TYPE, __DATE__,__TIME__);   //

    ui_kenyaPanel = lv_obj_create(ui_settingsList);
    lv_obj_set_width(ui_kenyaPanel, 200);
    lv_obj_set_height(ui_kenyaPanel, LV_SIZE_CONTENT); /// 64
    lv_obj_set_align(ui_kenyaPanel, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_kenyaPanel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(ui_kenyaPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_kenyaPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_kenyaPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_kenyaPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_kenyaPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_kenyaPanel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_kenyaPanel, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_kenyaPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_kenyaPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_kenyaPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_kenyaPanel, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_kenyaIcon = lv_img_create(ui_kenyaPanel);
    lv_img_set_src(ui_kenyaIcon, "D:/ui_img_notify.png");
    lv_obj_set_width(ui_kenyaIcon, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_kenyaIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_kenyaIcon, -5);
    lv_obj_set_y(ui_kenyaIcon, 2);
    lv_obj_add_flag(ui_kenyaIcon, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_kenyaIcon, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_img_set_zoom(ui_kenyaIcon, 150);

    ui_kenyaText = lv_label_create(ui_kenyaPanel);
    lv_obj_set_width(ui_kenyaText, 130);
    lv_obj_set_height(ui_kenyaText, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_kenyaText, 60);
    lv_obj_set_y(ui_kenyaText, 7);
    lv_label_set_recolor(ui_kenyaText, true);
    lv_label_set_text_fmt(ui_kenyaText, "#f54278 手表#参考方案\nSDK v%s%s\nUNIQ=%s",SDK_MINOR_VERSION,SDK_PATCH_VERSION,DB_VERSION_UNIQ_ID);
    // lv_label_set_text(ui_kenyaText, "#f54278 手表#参考方案\nEigencomm \nusing LVGL v8.4");
    lv_obj_add_event_cb(ui_kenyaPanel, ui_event_kenyaPanel, LV_EVENT_ALL, NULL);

    lv_obj_add_event_cb(ui_settingsList, onScroll, LV_EVENT_SCROLL, NULL);
    lv_obj_add_event_cb(ui_brightnessSlider, ui_event_brightnessSlider, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_Switch2, ui_event_scrollMode, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_alertStateSwitch, ui_event_alertStateSwitch, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_timeoutSelect, ui_event_timeoutSelect, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_languageSelect, ui_event_languageSelect, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_batterySlider, ui_event_batterySlider, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_settingsScreen, ui_event_settingsScreen, LV_EVENT_ALL, NULL);
}
/**
  \fn
  \brief
  \return
*/
void ui_appInfo_screen_init(void)
{
    ui_appInfoScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_appInfoScreen, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_bg_color(ui_appInfoScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_appInfoScreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_appInfoPanel = lv_obj_create(ui_appInfoScreen);
    lv_obj_set_width(ui_appInfoPanel, lv_pct(100));
    lv_obj_set_height(ui_appInfoPanel, lv_pct(100));
    lv_obj_set_align(ui_appInfoPanel, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow(ui_appInfoPanel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_appInfoPanel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(ui_appInfoPanel, LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(ui_appInfoPanel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(ui_appInfoPanel, LV_DIR_VER);
    lv_obj_set_style_radius(ui_appInfoPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_appInfoPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_appInfoPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_appInfoPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_appInfoPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_appInfoPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_appInfoPanel, 25, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_appInfoPanel, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(ui_appInfoPanel, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(ui_appInfoPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_appInfoTitle = lv_label_create(ui_appInfoPanel);
    lv_obj_set_width(ui_appInfoTitle, 150);
    lv_obj_set_height(ui_appInfoTitle, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_appInfoTitle, LV_ALIGN_TOP_MID);
    watch_lable.system[2] = "设备信息";
    lv_label_set_text(ui_appInfoTitle, watch_lable.system[2]);
    lv_obj_set_style_text_align(ui_appInfoTitle, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_appInfoTitle, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_appInfoTitle, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_appInfoTitle, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_appInfoTitle, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_appInfoTitle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_appInfoTitle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_appInfoTitle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_appInfoTitle, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_appDetailsPanel = lv_obj_create(ui_appInfoPanel);
    lv_obj_set_width(ui_appDetailsPanel, 190);
    lv_obj_set_height(ui_appDetailsPanel, 50);
    lv_obj_set_align(ui_appDetailsPanel, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_appDetailsPanel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(ui_appDetailsPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_appDetailsPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_appDetailsPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_appDetailsPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_appDetailsPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_appDetailsPanel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_appDetailsPanel, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_appDetailsPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_appDetailsPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_appDetailsPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_appDetailsPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_appDetailsIcon = lv_img_create(ui_appDetailsPanel);
    lv_img_set_src(ui_appDetailsIcon, "D:/ui_img_chrns.png");
    lv_obj_set_width(ui_appDetailsIcon, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_appDetailsIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_appDetailsIcon, LV_ALIGN_LEFT_MID);
    lv_obj_add_flag(ui_appDetailsIcon, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_appDetailsIcon, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    ui_appDetailsText = lv_label_create(ui_appDetailsPanel);
    lv_obj_set_width(ui_appDetailsText, 139);
    lv_obj_set_height(ui_appDetailsText, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_appDetailsText, 43);
    lv_obj_set_y(ui_appDetailsText, 8);
    lv_label_set_text_fmt(ui_appDetailsText, "%s %s\n%s", SDK_MINOR_VERSION,SDK_PATCH_VERSION,DB_VERSION_UNIQ_ID);

    ui_appConnectionPanel = lv_obj_create(ui_appInfoPanel);
    lv_obj_set_width(ui_appConnectionPanel, 190);
    lv_obj_set_height(ui_appConnectionPanel, 50);
    lv_obj_set_align(ui_appConnectionPanel, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_appConnectionPanel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(ui_appConnectionPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_appConnectionPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_appConnectionPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_appConnectionPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_appConnectionPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_appConnectionPanel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_appConnectionPanel, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_appConnectionPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_appConnectionPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_appConnectionPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_appConnectionPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_appConnectionIcon = lv_img_create(ui_appConnectionPanel);
    lv_img_set_src(ui_appConnectionIcon, "D:/ui_img_ble_app.png");
    lv_obj_set_width(ui_appConnectionIcon, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_appConnectionIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_appConnectionIcon, LV_ALIGN_LEFT_MID);
    lv_obj_add_flag(ui_appConnectionIcon, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_appConnectionIcon, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    ui_appConnectionText = lv_label_create(ui_appConnectionPanel);
    lv_obj_set_width(ui_appConnectionText, 139);
    lv_obj_set_height(ui_appConnectionText, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_appConnectionText, 43);
    lv_obj_set_y(ui_appConnectionText, 8);
    // lv_label_set_text(ui_appConnectionText, "Status\n#f73619 Disconnected#");
    lv_label_set_text(ui_appConnectionText, "状态\n#f73619 未连接#");
    lv_label_set_recolor(ui_appConnectionText, true);
    // lv_obj_set_style_text_font(ui_appConnectionText, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_appBatteryPanel = lv_obj_create(ui_appInfoPanel);
    lv_obj_set_width(ui_appBatteryPanel, 190);
    lv_obj_set_height(ui_appBatteryPanel, 50);
    lv_obj_set_align(ui_appBatteryPanel, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_appBatteryPanel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(ui_appBatteryPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_appBatteryPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_appBatteryPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_appBatteryPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_appBatteryPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_appBatteryPanel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_appBatteryPanel, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_appBatteryPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_appBatteryPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_appBatteryPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_appBatteryPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_appBatteryIcon = lv_img_create(ui_appBatteryPanel);
    lv_img_set_src(ui_appBatteryIcon, "D:/ui_img_battery_state.png");
    lv_obj_set_width(ui_appBatteryIcon, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_appBatteryIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_appBatteryIcon, LV_ALIGN_LEFT_MID);
    lv_obj_add_flag(ui_appBatteryIcon, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_appBatteryIcon, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    ui_appBatteryText = lv_label_create(ui_appBatteryPanel);
    lv_obj_set_width(ui_appBatteryText, 148);
    lv_obj_set_height(ui_appBatteryText, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_appBatteryText, 43);
    lv_obj_set_y(ui_appBatteryText, 5);
    lv_label_set_text_fmt(ui_appBatteryText, "电量 %d%%", 45);  //Battery

    ui_appBatteryLevel = lv_bar_create(ui_appBatteryPanel);
    lv_bar_set_value(ui_appBatteryLevel, 25, LV_ANIM_OFF);
    lv_obj_set_width(ui_appBatteryLevel, 136);
    lv_obj_set_height(ui_appBatteryLevel, 10);
    lv_obj_set_x(ui_appBatteryLevel, 44);
    lv_obj_set_y(ui_appBatteryLevel, 28);

    lv_obj_add_event_cb(ui_appInfoPanel, onScroll, LV_EVENT_SCROLL, NULL);
    lv_obj_add_event_cb(ui_appInfoScreen, ui_event_appInfoScreen, LV_EVENT_ALL, NULL);
}
/**
  \fn
  \brief
  \return
*/
void ui_ringing_screen_init(void)
{
    ui_ringScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_ringScreen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_ringScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_ringScreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_ringNumber = lv_label_create(ui_ringScreen);
    lv_obj_set_width(ui_ringNumber, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_ringNumber, LV_SIZE_CONTENT);
    lv_obj_align(ui_ringNumber,LV_ALIGN_TOP_MID,0,16);
    lv_label_set_text(ui_ringNumber, ring_label.numstr);
    lv_obj_set_style_text_font(ui_ringNumber, pFontExtern_28, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_ringName = lv_label_create(ui_ringScreen);
    lv_obj_set_width(ui_ringName, 200);
    lv_obj_set_height(ui_ringName, LV_SIZE_CONTENT);
    lv_obj_align(ui_ringName,LV_ALIGN_TOP_MID,0,60);
    lv_label_set_text(ui_ringName, ring_label.name);
    lv_obj_set_style_text_align(ui_ringName, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_ringIcon = lv_img_create(ui_ringScreen);
    lv_img_set_src(ui_ringIcon, ring_label.icon);
    lv_obj_set_width(ui_ringIcon, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_ringIcon, LV_SIZE_CONTENT);
    lv_obj_align(ui_ringIcon,LV_ALIGN_CENTER,0,0);
    lv_obj_add_flag(ui_ringIcon, LV_OBJ_FLAG_ADV_HITTEST);
    lv_obj_clear_flag(ui_ringIcon, LV_OBJ_FLAG_SCROLLABLE);

    ui_ringButton = lv_btn_create(ui_ringScreen);
    lv_obj_set_width(ui_ringButton, 80);
    lv_obj_set_height(ui_ringButton,30);
    lv_obj_align(ui_ringButton,LV_ALIGN_BOTTOM_MID,0,-16);
    lv_obj_add_flag(ui_ringButton, LV_OBJ_FLAG_CHECKABLE | LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_ringButton, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_ringButton, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(ui_ringButton, ui_event_button_ringing, LV_EVENT_ALL, NULL);

    ui_ringButtonText = lv_label_create(ui_ringButton);
    lv_obj_set_width(ui_ringButtonText, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_ringButtonText, LV_SIZE_CONTENT);
    lv_obj_align(ui_ringButtonText,LV_ALIGN_CENTER,0,0);
    lv_label_set_text(ui_ringButtonText, ring_label.button);

    lv_obj_add_event_cb(ui_ringScreen, ui_event_screen_ringing, LV_EVENT_ALL, NULL);
}



// void ui_event_screen_camera_preview(lv_event_t *e) 
// {
//     lv_event_code_t code = lv_event_get_code(e);
//     if (code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_BOTTOM)
//     {

//         // EPAT_LOG(ui_event_camera_preview_stop, P_INFO, "code %d",code);
//     }

// }


/**
  \fn
  \brief  拨号界面，数字键盘
  \return
*/
void ui_dialpad_screen_init(void)
{
    ui_dialpadScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_dialpadScreen, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    ui_dialpadLabel = lv_label_create(ui_dialpadScreen);
    lv_obj_set_style_text_font(ui_dialpadLabel, pFontExtern_28, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_size(ui_dialpadLabel, 224, 30);
    lv_obj_align(ui_dialpadLabel,LV_ALIGN_TOP_LEFT,8,4);
    lv_label_set_text(ui_dialpadLabel,call_label.numstr);

    ui_dialMatrix = lv_btnmatrix_create(ui_dialpadScreen);
    lv_btnmatrix_set_map(ui_dialMatrix, btn_dial_list);
    lv_obj_set_size(ui_dialMatrix, 220, 200);
    lv_obj_align(ui_dialMatrix,LV_ALIGN_BOTTOM_MID,0,-4);
    // lv_obj_set_user_data(ui_Call_Incon1, (uint16_t)1);
    lv_obj_add_event_cb(ui_dialMatrix, ui_event_btnmatrix_dialpad, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ui_dialpadScreen, ui_event_screen_dialpad, LV_EVENT_ALL, NULL);
}
/**
  \fn
  \brief
  \return
*/
void ui_calling_screen_init(void)
{
    ui_callScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_callScreen, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_bg_color(ui_callScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_callScreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_callNumber = lv_label_create(ui_callScreen);
    lv_obj_set_width(ui_callNumber, 150);
    lv_obj_set_height(ui_callNumber, 30);
    lv_obj_align(ui_callNumber,LV_ALIGN_TOP_MID,0,8);
    lv_label_set_text(ui_callNumber, call_label.numstr);
    lv_obj_set_style_text_align(ui_callNumber, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_callNumber, pFontExtern_28, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_callPanel = lv_obj_create(ui_callScreen);
    lv_obj_set_width(ui_callPanel, 64);
    lv_obj_set_height(ui_callPanel, 64);
    lv_obj_align(ui_callPanel,LV_ALIGN_CENTER,0,-16);
    lv_obj_add_flag(ui_callPanel, LV_OBJ_FLAG_HIDDEN);       /// Flags
    lv_obj_clear_flag(ui_callPanel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(ui_callPanel, 64, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_callPanel, lv_color_hex(0xFF6910), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_callPanel, 100, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_callPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_callIcon = lv_img_create(ui_callScreen);
    lv_img_set_src(ui_callIcon, call_label.icon);
    lv_obj_set_width(ui_callIcon, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_callIcon, LV_SIZE_CONTENT);
    lv_obj_align(ui_callIcon,LV_ALIGN_CENTER,0,-16);
    lv_obj_add_flag(ui_callIcon, LV_OBJ_FLAG_ADV_HITTEST);
    lv_obj_clear_flag(ui_callIcon, LV_OBJ_FLAG_SCROLLABLE);

    ui_callLabel = lv_label_create(ui_callScreen);
    lv_obj_set_width(ui_callLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_callLabel, LV_SIZE_CONTENT);
    lv_obj_align(ui_callLabel,LV_ALIGN_CENTER,0,40);
    lv_label_set_text(ui_callLabel, call_label.name);

    ui_callButton = lv_btn_create(ui_callScreen);
    lv_obj_set_width(ui_callButton, 80);
    lv_obj_set_height(ui_callButton,30);
    lv_obj_align(ui_callButton,LV_ALIGN_BOTTOM_MID,0,-16);
    lv_obj_add_flag(ui_callButton, LV_OBJ_FLAG_CHECKABLE | LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    lv_obj_clear_flag(ui_callButton, LV_OBJ_FLAG_SCROLLABLE);                            /// Flags
    lv_obj_set_style_radius(ui_callButton, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(ui_callButton, ui_event_button_call, LV_EVENT_ALL, NULL);

    ui_callButtonText = lv_label_create(ui_callButton);
    lv_obj_set_width(ui_callButtonText, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_callButtonText, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_callButtonText, LV_ALIGN_CENTER);
    lv_label_set_text(ui_callButtonText, call_label.button);

    lv_obj_add_event_cb(ui_callScreen, ui_event_screen_call, LV_EVENT_ALL, NULL);
}
/**
  \fn
  \brief
  \return
*/
void ui_filesScreen_screen_init(void)
{
    ui_filesScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_filesScreen, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    ui_fileManagerPanel = lv_obj_create(ui_filesScreen);
    lv_obj_set_width(ui_fileManagerPanel, lv_pct(100));
    lv_obj_set_height(ui_fileManagerPanel, lv_pct(100));
    lv_obj_set_align(ui_fileManagerPanel, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow(ui_fileManagerPanel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_fileManagerPanel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_scrollbar_mode(ui_fileManagerPanel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(ui_fileManagerPanel, LV_DIR_VER);
    lv_obj_set_style_radius(ui_fileManagerPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_fileManagerPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_fileManagerPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_fileManagerPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_fileManagerPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_fileManagerPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_fileManagerPanel, 40, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_fileManagerPanel, 40, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(ui_fileManagerPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(ui_fileManagerPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(ui_fileManagerPanel, onScroll, LV_EVENT_SCROLL, NULL);
    lv_obj_add_event_cb(ui_filesScreen, ui_event_appInfoScreen, LV_EVENT_ALL, NULL);
}
/**
  \fn
  \brief
  \return
*/
void ui_watchfaces_init(void)
{
    numFaces = 0;
    registerWatchface_cb("Default", "D:/digital_preview_map.png", &ui_clockScreen);
    // register other watchfaces by initializing them and passing the register callback
    #ifdef ENABLE_FACE_75_2_DIAL
    init_face_75_2_dial(registerWatchface_cb);
    #endif
    #ifdef ENABLE_FACE_SMART_RESIZED
    init_face_smart_resized(registerWatchface_cb);
    #endif
}

/**
  \fn
  \brief
  \return
*/
void ui_update_watchfaces(int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday,
                          int temp, int icon, int battery, bool connection, int steps, int distance, int kcal, int bpm, int oxygen)
{
    #ifdef ENABLE_FACE_75_2_DIAL
    update_check_75_2_dial(ui_home, second, minute, hour, mode, am, day, month, year, weekday, temp, icon, battery, connection, steps, distance, kcal, bpm, oxygen);
    #endif
    #ifdef ENABLE_FACE_SMART_RESIZED
    update_check_smart_resized(ui_home, second, minute, hour, mode, am, day, month, year, weekday, temp, icon, battery, connection, steps, distance, kcal, bpm, oxygen);
    #endif
}
/**
  \fn
  \brief
  \return
*/
void ui_errorWindow_init(void)
{
    ui_errorWindow = lv_obj_create(ui_clockScreen);
    lv_obj_set_width(ui_errorWindow, lv_pct(100));
    lv_obj_set_height(ui_errorWindow, lv_pct(100));
    lv_obj_set_align(ui_errorWindow, LV_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(ui_errorWindow, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(ui_errorWindow, LV_OBJ_FLAG_HIDDEN);           /// Flags
    lv_obj_clear_flag(ui_errorWindow, LV_OBJ_FLAG_GESTURE_BUBBLE); /// block gesture events when active
    lv_obj_set_scroll_dir(ui_errorWindow, LV_DIR_VER);
    lv_obj_set_style_radius(ui_errorWindow, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_errorWindow, lv_color_hex(0xB10000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_errorWindow, 150, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_errorWindow, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_errorWindow, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_errorWindow, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_errorWindow, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_errorWindow, 60, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_errorPanel = lv_obj_create(ui_errorWindow);
    lv_obj_set_width(ui_errorPanel, 167);
    lv_obj_set_height(ui_errorPanel, LV_SIZE_CONTENT); /// 139
    lv_obj_set_x(ui_errorPanel, 0);
    lv_obj_set_y(ui_errorPanel, 60);
    lv_obj_set_align(ui_errorPanel, LV_ALIGN_TOP_MID);
    lv_obj_set_flex_flow(ui_errorPanel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_errorPanel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(ui_errorPanel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_bg_color(ui_errorPanel, lv_color_hex(0x080404), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_errorPanel, 235, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_errorPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_errorPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_errorPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_errorPanel, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_errorPanel, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_errorPanel, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_errorPanel, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_errorTitle = lv_label_create(ui_errorPanel);
    lv_obj_set_width(ui_errorTitle, 140);
    lv_obj_set_height(ui_errorTitle, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_errorTitle, LV_ALIGN_TOP_MID);
    lv_label_set_long_mode(ui_errorTitle, LV_LABEL_LONG_SCROLL_CIRCULAR);
    // lv_label_set_text(ui_errorTitle, "Error");
    lv_label_set_text(ui_errorTitle, "错误");
    lv_obj_set_style_text_align(ui_errorTitle, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_text_font(ui_errorTitle, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_errorTitle, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_errorTitle, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_errorTitle, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_errorTitle, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_errorTitle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_errorTitle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_errorTitle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_errorTitle, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_errorMessage = lv_label_create(ui_errorPanel);
    lv_obj_set_width(ui_errorMessage, 140);
    lv_obj_set_height(ui_errorMessage, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_errorMessage, LV_ALIGN_CENTER);
    // lv_label_set_text(ui_errorMessage, "Error message");
    lv_label_set_text(ui_errorMessage, "错误消息");
    // lv_obj_set_style_text_font(ui_errorMessage, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_errorClose = lv_btn_create(ui_errorPanel);
    lv_obj_set_width(ui_errorClose, 100);
    lv_obj_set_height(ui_errorClose, LV_SIZE_CONTENT); /// 40
    lv_obj_set_align(ui_errorClose, LV_ALIGN_BOTTOM_MID);
    lv_obj_add_flag(ui_errorClose, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    lv_obj_clear_flag(ui_errorClose, LV_OBJ_FLAG_SCROLLABLE);    /// Flags
    lv_obj_set_style_radius(ui_errorClose, 20, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_errorCloseText = lv_label_create(ui_errorClose);
    lv_obj_set_width(ui_errorCloseText, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_errorCloseText, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_errorCloseText, LV_ALIGN_CENTER);
    // lv_label_set_text(ui_errorCloseText, "Close");
    lv_label_set_text(ui_errorCloseText, "关闭");
    // lv_obj_set_style_text_font(ui_errorCloseText, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(ui_errorClose, ui_event_errorClose, LV_EVENT_ALL, NULL);
}
/**
  \fn
  \brief
  \return
*/
void init_custom_face(void)
{
    face_custom_root = lv_obj_create(NULL);
    lv_obj_clear_flag(face_custom_root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(face_custom_root, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(face_custom_root, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(face_custom_root, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(face_custom_root, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(face_custom_root, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(face_custom_root, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(face_custom_root, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(face_custom_root, onFaceEvent, LV_EVENT_ALL, NULL);
}
/**
  \fn
  \brief
  \return
*/
static void ui_data(void)
{
    for(uint8_t i=0; i<8; i++){
        watch_lable.settings[i] = ui_settings_CN[i];
    }
    // for(uint8_t i=0; i<4; i++){
    //     watch_lable.dialing[i] = ui_dialing_CN[i];
    // }
    // for(uint8_t i=0; i<4; i++){
    //     watch_lable.ringing[i] = ui_ringing_CN[i];
    // }
    watchMenuRead(watchMenuFilePathGet());
    pFontNumber = lv_font_load("D:/lv_font_number.bin");
    if(pFontNumber == NULL) {
        LV_LOG_ERROR("Failed to load lv_font_number.bin");
    }
    pFontExtern_28 = lv_font_load("D:/lv_font_extern_28.bin");
    if(pFontExtern_28 == NULL) {
        LV_LOG_ERROR("Failed to load lv_font_extern_28.bin");
    }
    pFontExtern_48 = lv_font_load("D:/lv_font_extern_48.bin");
    if(pFontExtern_48 == NULL) {
        LV_LOG_ERROR("Failed to load lv_font_extern_48.bin");
    }
    extern uint32_t lv_font_extern_init(void);
    lv_font_extern_init();
}
/**
  \fn
  \brief
  \return
*/
void ui_games_init(void)
{
    numGames = 0;
    ui_raceScreen_screen_init(registerGame_cb);
    if (numGames == 0)
    {
        lv_obj_t *info = lv_label_create(ui_gameList);
        lv_obj_set_width(info, 180);
        lv_obj_set_height(info, LV_SIZE_CONTENT); /// 1
        lv_obj_set_align(info, LV_ALIGN_CENTER);
        lv_label_set_text(info, "No apps or games available currently. Check whether it was enabled in the code");
    }
}
/**
  \fn
  \brief
  \return
*/
#define MSGBOX_AUTO_CLOSE_DELAY 1000  // 2秒后自动关闭
// 关闭消息框的回调函数
void close_msgbox_callback(lv_timer_t * timer) {
    lv_obj_t * mbox = (lv_obj_t *)timer->user_data;  // 获取消息框对象
    lv_msgbox_close(mbox);  // 关闭消息框
    lv_timer_del(timer);    // 删除定时器
}

// 定时器回调函数
void timer_callback(lv_timer_t * timer) {
    char time_str[64];
    time_t now = time_time(NULL);
    struct tm *timeinfo = time_localtime(&now);
    memset(time_str, 0, sizeof(time_str));
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);
    lv_obj_t * mbox = lv_msgbox_create(lv_layer_sys(), NULL, time_str, NULL, false);
    lv_obj_set_width(mbox, 200); 
    lv_obj_align(mbox, LV_ALIGN_TOP_MID, 0, 0);
    // lv_obj_set_style_bg_color(mbox, lv_color_hex(0x333333), 0);
    lv_obj_set_style_bg_opa(mbox, LV_OPA_30, 0);
    // lv_obj_set_style_radius(mbox, 10, 0); 
    // 创建一个定时器，用于延迟关闭消息框
    lv_timer_t * close_timer = lv_timer_create(close_msgbox_callback, MSGBOX_AUTO_CLOSE_DELAY, mbox);
}
/**
  \fn
  \brief
  \return
*/
void ui_init(void)
{
    osThreadAttr_t coTaskAttr = {
        .name = "ui_coTask",
        .stack_size = 2*1024,
        .priority = osPriorityNormal,
    };
    coTaskAttr.stack_mem = pvPortZeroAssertMallocCust(2*1024);
    ui_coTask_init(NULL,&coTaskAttr);   //创建coTask但是不单独指定链表，和guiTask共享任务链表；
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), true, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    ui_data();
    ui_clockScreen_screen_init();
    ui_weatherScreen_screen_init();
    ui_notificationScreen_screen_init();
    ui_appList_screen_init();
    ui_gameListScreen_screen_init();
    ui_settings_screen_init();
    ui_appInfo_screen_init();
    ui_ringing_screen_init();
    ui_dialpad_screen_init();
    ui_calling_screen_init();
    ui_errorWindow_init();
    ui_filesScreen_screen_init();
    #if (CAMERA_ENABLE_GC6153 == 1) && (defined USED_WATCH_WEBRTC)
    ui_webrtc_screen_init();
    #endif
    init_face_select();
    ui_watchfaces_init();
    init_custom_face();
    ui_games_init();
#ifdef USED_WATCH_ALIPAY
	ui_alipay_init();
	#endif
#ifdef USED_WATCH_CAMERA
    ui_watch_camera_init();
#endif
#ifdef USED_WATCH_ALBUM
    ui_watch_album_init();
#endif
#ifdef USED_WATCH_MPLAYER
    ui_mplayer_init();
#endif
    // lv_timer_t * timer = lv_timer_create(timer_callback, 4000, NULL);
    ui_home = ui_clockScreen;
    ui____initial_actions0 = lv_obj_create(NULL);
    lv_obj_add_event_cb(ui____initial_actions0,
                        ui_event____initial_actions0, LV_EVENT_ALL, NULL);
    lv_disp_load_scr(ui____initial_actions0);
    lv_disp_load_scr(ui_home);
    extern void audioFileListGet(void);
    audioFileListGet();
}
