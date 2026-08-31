#ifndef  _SCR_CO5300_
#define  _SCR_CO5300_
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t co5300_set_window(uint16_t sx, uint16_t ex, uint16_t width,
                           uint16_t height);
int co5300_set_direction(DisDirection_e dir);
int co5300_set_display_mode(DisplayMode_e dir);
int co5300_set_display_pix_mode(DisplayPixMode_e mode);
int co5300_set_backlight(uint8_t level);
int co5300_suspend(void);
int co5300_resume(void);
int co5300_set_display_data_fmt(DisplayDataFmt_e fmt);

#ifdef __cplusplus
}
#endif
#endif
