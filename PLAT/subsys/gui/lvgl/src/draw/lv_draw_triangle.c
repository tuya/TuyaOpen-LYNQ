/**
 * @file lv_draw_triangle.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_draw.h"
#include "lv_draw_triangle.h"
#include "../misc/lv_math.h"
#include "../misc/lv_mem.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
#ifdef FEATURE_MES_LCD_ENABLE
#include "syslog.h"
static uint32_t cost_max1 = 0;
#endif
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_draw_polygon(struct _lv_draw_ctx_t * draw_ctx, const lv_draw_rect_dsc_t * draw_dsc, const lv_point_t points[],
                     uint16_t point_cnt)
{
    draw_ctx->draw_polygon(draw_ctx, draw_dsc, points, point_cnt);
}

void lv_draw_triangle(struct _lv_draw_ctx_t * draw_ctx, const lv_draw_rect_dsc_t * draw_dsc, const lv_point_t points[])
{
    #ifdef FEATURE_MES_LCD_ENABLE
    uint32_t cost_tick = 0;
    measure_time(NULL);
    #endif
    draw_ctx->draw_polygon(draw_ctx, draw_dsc, points, 3);
    #ifdef FEATURE_MES_LCD_ENABLE
    uint32_t cost_us = measure_time(&cost_tick);    //
    if(cost_us > cost_max1){
        cost_max1 = cost_us;
        SYSLOG_INFO("%d\r\n",cost_us);
    }
    #endif
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
