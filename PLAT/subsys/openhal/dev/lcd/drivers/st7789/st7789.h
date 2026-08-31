#ifndef __ST7789_H__
#define __ST7789_H__
#include <stdint.h>
#include <stdbool.h>

uint32_t st7789_set_window(uint16_t sx, uint16_t ex, uint16_t width,
                           uint16_t height);
int st7789_set_direction(DisDirection_e dir);
int st7789_set_display_mode(DisplayMode_e dir);
int st7789_set_display_pix_mode(DisplayPixMode_e mode);
int st7789_set_backlight(uint8_t level);
int st7789_suspend(void);
int st7789_resume(void);
int st7789_set_display_data_fmt(DisplayDataFmt_e fmt);
#endif
