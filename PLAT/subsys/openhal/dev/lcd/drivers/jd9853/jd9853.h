#ifndef  LCD_JD9853_
#define  LCD_JD9853_
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t jd9853_set_window(uint16_t sx, uint16_t ex, uint16_t width,
                           uint16_t height);
int jd9853_set_direction(DisDirection_e dir);
int jd9853_set_display_mode(DisplayMode_e dir);
int jd9853_set_display_pix_mode(DisplayPixMode_e mode);
int jd9853_set_backlight(uint8_t level);
int jd9853_suspend(void);
int jd9853_resume(void);
int jd9853_set_display_data_fmt(DisplayDataFmt_e fmt);


#ifdef __cplusplus
}
#endif


#endif
