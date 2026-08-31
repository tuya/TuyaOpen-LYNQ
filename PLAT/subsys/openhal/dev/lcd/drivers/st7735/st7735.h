#ifndef __ST7735_H__
#define __ST7735_H__
#include <stdint.h>
#include <stdbool.h>

uint32_t st7735_set_window(uint16_t sx, uint16_t ex, uint16_t width,
                           uint16_t height);
int st7735_set_direction(DisDirection_e dir);
int st7735_set_display_mode(DisplayMode_e dir);
int st7735_set_display_pix_mode(DisplayPixMode_e mode);
int st7735_set_backlight(uint8_t level);
int st7735_suspend(void);
int st7735_resume(void);
int st7735_set_display_data_fmt(DisplayDataFmt_e fmt);

#endif
