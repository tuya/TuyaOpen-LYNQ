/**
 * @file lv_draw_transform.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_draw.h"
#include "lv_draw_transform.h"
#include "../misc/lv_assert.h"
#include "../misc/lv_area.h"

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
void lv_draw_transform(lv_draw_ctx_t * draw_ctx, const lv_area_t * dest_area, const void * src_buf, lv_coord_t src_w,
                       lv_coord_t src_h,
                       lv_coord_t src_stride, const lv_draw_img_dsc_t * draw_dsc, lv_img_cf_t cf, lv_color_t * cbuf, lv_opa_t * abuf)
{
    LV_ASSERT_NULL(draw_ctx);
    if(draw_ctx->draw_transform == NULL) {
        LV_LOG_WARN("draw_ctx->draw_transform == NULL");
        return;
    }
    #ifdef FEATURE_MES_LCD_ENABLE
    uint32_t cost_tick = 0;
    measure_time(NULL);
    #endif
    draw_ctx->draw_transform(draw_ctx, dest_area, src_buf, src_w, src_h, src_stride, draw_dsc, cf, cbuf, abuf);
    #ifdef FEATURE_MES_LCD_ENABLE
    uint32_t cost_us = measure_time(&cost_tick);    //
    if(cost_us > cost_max1){
        cost_max1 = cost_us;
        SYSLOG_INFO("[%d,%d],%d\r\n",src_w, src_h,cost_us);
    }
    #endif
}


/**********************
 *   STATIC FUNCTIONS
 **********************/
