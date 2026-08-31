#ifndef _GAME_RACING_H
#define _GAME_RACING_H

#ifdef __cplusplus
extern "C"
{
#endif
#include "lvgl.h"
#include "app_hal.h"

//#define ENABLE_GAME_RACING // (Racing) uncomment to enable or define it elsewhere

#ifdef ENABLE_GAME_RACING
extern lv_obj_t *ui_raceScreen;
#define ui_img_road_png         "D:/ui_img_road.png"
#define ui_img_car_png          "D:/ui_img_car.png"
#define ui_img_car_green_png    "D:/ui_img_car_green.png"
#define ui_img_car_red_png      "D:/ui_img_car_red.png"
#define ui_img_car_yellow_png   "D:/ui_img_car_yellow.png"
// LV_IMG_DECLARE(ui_img_road_png);       // assets\road.png
// LV_IMG_DECLARE(ui_img_car_png);        // assets\car.png
// LV_IMG_DECLARE(ui_img_car_green_png);  // assets\car_green.png
// LV_IMG_DECLARE(ui_img_car_red_png);    // assets\car_red.png
// LV_IMG_DECLARE(ui_img_car_yellow_png); // assets\car_yellow.png
#endif
void ui_raceScreen_screen_init(void (*callback)(const char*, const lv_img_dsc_t *, lv_obj_t **));
void ui_raceScreen_screen_loop(void *arg);
void ui_gameExit(void);
void onGameOpened(void);
void onGameClosed(void);
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif