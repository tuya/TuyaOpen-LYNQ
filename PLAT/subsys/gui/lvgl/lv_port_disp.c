/**
 * @file lv_port_disp.c
 *
 */

/*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp.h"
#include <stdbool.h>
#include "bsp.h"

#include "osasys.h"
#define EPAT_LOG(subId, debugLevel, format, ...)  ECPLAT_PRINTF(UNILOG_LVGL, subId, debugLevel, format, ##__VA_ARGS__)

#ifdef FEATURE_HAL_SCREEN_ENABLE
#include "api_scr.h"
#include "api_comm.h"
#else
#include "lcdDrv.h"
#endif

#ifdef FEATURE_SUBSYS_DISPLAY_ENABLE
#include "display.h"
#endif

#ifdef LCD_WIDTH
#define MY_DISP_HOR_RES         LCD_WIDTH               //horizontally  横向
#endif
#ifdef LCD_HEIGHT
#define MY_DISP_VER_RES         LCD_HEIGHT              //vertically    纵向
#endif
#ifdef FEATURE_SUBSYS_LAUNCHER_ENABLE
#define MY_DISP_HOR_RES    240
#define MY_DISP_VER_RES    320
#elif FEATURE_SUBSYS_WATCH_ENABLE
#define MY_DISP_HOR_RES    240
#define MY_DISP_VER_RES    240
#elif (ONLY_TEST_LV_SIZE == 1)
#define MY_DISP_HOR_RES    240
#define MY_DISP_VER_RES    320
#endif
/*********************
 *      DEFINES
 *********************/
#ifndef MY_DISP_HOR_RES
#warning Please define or replace the macro MY_DISP_HOR_RES with the actual screen width, default value 320 is used for now.
#define MY_DISP_HOR_RES    240  //LCD_WIDTH
#endif  

#ifndef MY_DISP_VER_RES
#warning Please define or replace the macro MY_DISP_HOR_RES with the actual screen height, default value 240 is used for now.
#define MY_DISP_VER_RES    320  //LCD_HEIGHT
#endif
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);

static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);

/**********************
 *  STATIC VARIABLES
 **********************/
lv_disp_drv_t disp_drv;                        
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
LV_ATTRIBUTE_LARGE_RAM_ARRAY lv_color_t s_disp_buf[MY_DISP_HOR_RES*MY_DISP_VER_RES];
LV_ATTRIBUTE_LARGE_RAM_ARRAY lv_color_t p_disp_buf[MY_DISP_HOR_RES*MY_DISP_VER_RES];
static lv_disp_draw_buf_t draw_buf_dma = {0};

static void lcd_rounder_cb(lv_disp_drv_t * drv, lv_area_t * area)
{
    area->x1 -= (area->x1)%4;
    area->x2 += (4-(area->x2+1)%4);
    if(area->x2>(MY_DISP_HOR_RES-1)) area->x2 = (MY_DISP_HOR_RES-1);
    area->y1 -= (area->y1)%4;
    area->y2 += (4-(area->y2+1)%4);
    if(area->y2>(MY_DISP_VER_RES-1)) area->y2 = (MY_DISP_VER_RES-1);
}

void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();
    // s_disp_buf = OsaAllocZeroMemoryNoAssert_Psram(MY_DISP_HOR_RES*MY_PART_VER_RES*2);
    // p_disp_buf = OsaAllocZeroMemoryNoAssert_Psram(MY_DISP_HOR_RES*MY_PART_VER_RES*2);
    /*-----------------------------
     * Create a buffer for drawing
     *----------------------------*/

    /**
     * LVGL requires a buffer where it internally draws the widgets.
     * Later this buffer will passed to your display driver's `flush_cb` to copy its content to your display.
     * The buffer has to be greater than 1 display row
     *
     * There are 3 buffering configurations:
     * 1. Create ONE buffer:
     *      LVGL will draw the display's content here and writes it to your display
     *
     * 2. Create TWO buffer:
     *      LVGL will draw the display's content to a buffer and writes it your display.
     *      You should use DMA to write the buffer's content to the display.
     *      It will enable LVGL to draw the next part of the screen to the other buffer while
     *      the data is being sent form the first buffer. It makes rendering and flushing parallel.
     *
     * 3. Double buffering
     *      Set 2 screens sized buffers and set disp_drv.full_refresh = 1.
     *      This way LVGL will always provide the whole rendered screen in `flush_cb`
     *      and you only need to change the frame buffer's address.
     */
    lv_disp_draw_buf_init(&draw_buf_dma, s_disp_buf, p_disp_buf,MY_DISP_HOR_RES*MY_DISP_VER_RES);
    /*-----------------------------------
     * Register the display in LVGL
     *----------------------------------*/
    lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/

    /*Set up the functions to access to your display*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = MY_DISP_HOR_RES;
    disp_drv.ver_res = MY_DISP_VER_RES;

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = disp_flush;
    disp_drv.rounder_cb = lcd_rounder_cb;
    /*Set a display buffer*/
    disp_drv.draw_buf = &draw_buf_dma;

    // #ifndef MY_PART_VER_RES
    // disp_drv.full_refresh = 1;
    // #endif

    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/
static void disp_init(void)
{
    /*You code here*/
}

volatile bool disp_flush_enabled = true;

/* Enable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_enable_update(void)
{
    disp_flush_enabled = true;
}

/* Disable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_disable_update(void)
{
    disp_flush_enabled = false;
}
void bsp_lvgl_flush_ready(void)
{
	lv_disp_flush_ready(&disp_drv);
}

/*Flush the content of the internal buffer the specific area on the display
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_disp_flush_ready()' has to be called when finished.*/
uint8_t g_lv_disp_disable_flag = 0;
static uint8_t disp_disable_flag_checked = 0;
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    if(g_lv_disp_disable_flag && disp_disable_flag_checked){
        bsp_lvgl_flush_ready();
        EPAT_LOG(disp_flush, P_ERROR, "disable %d,%d,%d,%d",(area->x1),(area->y1),(area->x2),(area->y2));
        return;
    } 
    #ifdef FEATURE_HAL_SCREEN_ENABLE
    #ifdef FEATURE_SUBSYS_DISPLAY_ENABLE
    DisplayRegion_t region = {0};
    region.x = area->x1;
    region.y = area->y1;
    region.width = area->x2 - area->x1 + 1;
    region.height = area->y2 - area->y1 + 1;
    int ret = 0;
    do{
        ret = displayWriteData((uint8_t*)color_p,  region.width * region.height * 2, &region);
        if(ret == -2)
        {
            osDelay(5);
            printf("displayWriteData retry %d,%d,%d,%d\r\n",(area->x1),(area->y1),(area->x2),(area->y2));
        }
    } while(ret == -2);
    #else
        ScrWriteParam_t trans;
        trans.start_x = area->x1;
        trans.start_y = area->y1;
        trans.width = area->x2 - area->x1 + 1;
        trans.height = area->y2 - area->y1 + 1;
        trans.data = (void*)color_p;
        trans.fmt = OPEN_SCREEN_DATA_FMT_RGB565;
        trans.size = (trans.width * trans.height * 2);
        Device_write("dev:/lcd", &trans, sizeof(ScrWriteParam_t));
    #endif
    #elif defined FEATURE_DRIVER_LCD_ENABLE
    extern lcdDrvFunc_t* lcdDev;
    if (lcdDev != NULL) 
    {
        uint32_t fillLen = lcdSetWindow(lcdDev,area->x1,area->y1,area->x2,area->y2);
        lcdFill(lcdDev, fillLen, (uint8_t*)color_p);
    }
    #endif
    if(g_lv_disp_disable_flag != disp_disable_flag_checked){
        disp_disable_flag_checked = g_lv_disp_disable_flag;
        EPAT_LOG(disp_flush_end, P_WARNING, "%d,%d,%d,%d",(area->x1),(area->y1),(area->x2),(area->y2));
    } 
}

/*OPTIONAL: GPU INTERFACE*/

/*If your MCU has hardware accelerator (GPU) then you can use it to fill a memory with a color*/
//static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//                    const lv_area_t * fill_area, lv_color_t color)
//{
//    /*It's an example code which should be done by your GPU*/
//    int32_t x, y;
//    dest_buf += dest_width * fill_area->y1; /*Go to the first line*/
//
//    for(y = fill_area->y1; y <= fill_area->y2; y++) {
//        for(x = fill_area->x1; x <= fill_area->x2; x++) {
//            dest_buf[x] = color;
//        }
//        dest_buf+=dest_width;    /*Go to the next line*/
//    }
//}


#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
