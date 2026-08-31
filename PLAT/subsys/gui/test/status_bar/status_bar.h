/**
 * @file status_bar.h
 *
 */

#ifndef __STATUS_BAR_H__
#define __STATUS_BAR_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"
#include "page/page.h"

/*********************
 *      DEFINES
 *********************/
#define UI_DESKTOP_FRAME_COUNT_MAX      3
#define UI_STATUS_BAR_HEIGHT            32


/**********************
 *      TYPEDEFS
 **********************/
typedef lv_obj_t ui_page_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void ui_init(uint8_t desktop_frame_count, uint8_t home_index);


/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*__STATUS_BAR_H__*/
