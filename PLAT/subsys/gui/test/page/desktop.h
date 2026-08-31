/**
 * @file desktop.h
 *
 */

#ifndef __DESKTOP_H__
#define __DESKTOP_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
 * Create a desktop object, desktop is a page
 * @param frame_count         可左右活动的页面数量
 * @param home_index          home页的索引
 * @return                    pointer to the created page
 */
lv_obj_t* ui_desktop_create(uint32_t frame_count, uint32_t home_index);

uint32_t ui_desktop_get_frame_custom_height(lv_obj_t* obj);
lv_obj_t* ui_desktop_get_frame(lv_obj_t* obj, uint32_t frame_index);
uint32_t ui_desktop_get_frame_count(lv_obj_t* obj);
uint32_t ui_desktop_get_home_index(lv_obj_t* obj);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*__DESKTOP_H__*/
