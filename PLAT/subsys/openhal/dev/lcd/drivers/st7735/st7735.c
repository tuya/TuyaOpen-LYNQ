#include "bsp.h"
#include "sctdef.h"
#include "hal_screen.h"
#include "st7735.h"
#include DEBUG_LOG_HEADER_FILE

#define MADCTRL_REG 0x36
#define MADCTRL_REG_INIT_VALUE 0xC8
static uint8_t st7735MadCtrlRegValue = MADCTRL_REG_INIT_VALUE;

LcdDrvObj_t st7735Drv;
static ScrRegList_t st7735_init_table[] = {
    {0x11, 0, {0}},
    {0xff, 1, {120}},
    {0xb1, 3, {0x05, 0x3c, 0x3c}},                    // normal mode
    {0xb2, 3, {0x05, 0x3c, 0x3c}},                    // Idle mode
    {0xb3, 6, {0x05, 0x3c, 0x3c, 0x05, 0x3c, 0x3c}},  // Partial mode
    {0xb4, 1, {0x0}},                                 // Dot inersion
    {0xc0, 3, {0xab, 0x0b, 0x04}},                    // AVDD GVDD
    {0xc1, 1, {0xc5}},                                // c0
    {0xc2, 2, {0x0d, 0x00}},                          // Normal Mode
    {0xc3, 2, {0x8d, 0x6a}},                          // Idle
    {0xc4, 2, {0x8d, 0xee}},                          // Partial+Full
    {0xc5, 1, {0x0f}},                                // VCOM
    {0xe0,
     16,
     {0x07, 0x1B, 0x0F, 0x1E, 0x35, 0x2D, 0x27, 0x2A, 0x27, 0x25, 0x2E, 0x37,
      0x00, 0x05, 0x00, 0x10}},  // Positive voltage gamma
    {0xe1,
     16,
     {0x09, 0x18, 0x09, 0x1A, 0x27, 0x32, 0x2B, 0x2F, 0x2E, 0x2A, 0x31, 0x3E,
      0x00, 0x04, 0x05, 0x10}},  // Negative voltage gamma
    {0xfc, 1, {0x80}},
    {0x3a, 1, {5}},  // rgb565
    {0x36, 1, {MADCTRL_REG_INIT_VALUE}},
    //{0x21, 1, {0}},
    {0x29, 0, {0}},
    {0x2a, 4, {0x00, 0x1A, 0x00, 0x69}},
    {0x2b, 4, {0x00, 0x01, 0x00, 0xA0}},
    {0x2c, 0, {0}},
};

int st7735_suspend(void)
{
    hal_screen_send_cmd(0x10, NULL, 0);
    osDelay(5);
    return 0;
}

int st7735_resume(void)
{
    hal_screen_send_cmd(0x11, NULL, 0);
    osDelay(60);
    hal_screen_send_cmd(0x29, NULL, 0);
    return 0;
}

int st7735_set_backlight(uint8_t level) { return 0; }

uint32_t st7735_set_window(uint16_t sx, uint16_t sy, uint16_t width,
                           uint16_t height)
{
    uint16_t start_x = sx + st7735Drv.x_offset;
    uint16_t start_y = sy + st7735Drv.y_offset;
    uint16_t end_x = start_x + width - 1;
    uint16_t end_y = start_y + height - 1;
    uint8_t set_x_cmd[4] = {0};
    set_x_cmd[0] = start_x >> 8;
    set_x_cmd[1] = start_x & 0xFF;
    set_x_cmd[2] = end_x >> 8;
    set_x_cmd[3] = end_x & 0xFF;
    hal_screen_send_cmd(0x2A, &set_x_cmd[0], 4);

    uint8_t set_y_cmd[4] = {0};
    set_y_cmd[0] = start_y >> 8;
    set_y_cmd[1] = start_y & 0xFF;
    set_y_cmd[2] = end_y >> 8;
    set_y_cmd[3] = end_y & 0xFF;
    hal_screen_send_cmd(0x2B, &set_y_cmd[0], 4);
    hal_screen_send_cmd(0x2C, NULL, 0);
    return (width) * (height) * (st7735Drv.bpp / 8);
}

int st7735_set_direction(DisDirection_e dir)
{
    switch(dir)
    {
        case DIS_DIR_LRTB:
            st7735MadCtrlRegValue &= ~(BIT(2));
            st7735MadCtrlRegValue &= ~(BIT(4));
            st7735MadCtrlRegValue &= ~(BIT(5));
            break;
        case DIS_DIR_LRBT:
            st7735MadCtrlRegValue &= ~(BIT(2));
            st7735MadCtrlRegValue &= ~(BIT(5));
            st7735MadCtrlRegValue |= BIT(4);
            break;
        case DIS_DIR_RLTB:
            st7735MadCtrlRegValue &= ~(BIT(4));
            st7735MadCtrlRegValue &= ~(BIT(5));
            st7735MadCtrlRegValue |= BIT(2);
            break;
        case DIS_DIR_RLBT:
            st7735MadCtrlRegValue &= ~(BIT(5));
            st7735MadCtrlRegValue |= BIT(4);
            st7735MadCtrlRegValue |= BIT(2);
            break;
        case DIS_DIR_TBLR:
            st7735MadCtrlRegValue &= ~(BIT(2));
            st7735MadCtrlRegValue &= ~(BIT(4));
            st7735MadCtrlRegValue |= BIT(5);
            break;
        case DIS_DIR_BTLR:
            st7735MadCtrlRegValue &= ~(BIT(2));
            st7735MadCtrlRegValue |= BIT(4);
            st7735MadCtrlRegValue |= BIT(5);
            break;
        case DIS_DIR_TBRL:
            st7735MadCtrlRegValue &= ~(BIT(4));
            st7735MadCtrlRegValue |= BIT(2);
            st7735MadCtrlRegValue |= BIT(5);
            break;
        case DIS_DIR_BTRL:
            st7735MadCtrlRegValue |= BIT(2);
            st7735MadCtrlRegValue |= BIT(4);
            st7735MadCtrlRegValue |= BIT(5);
            break;
        default:
            break;
    }

    hal_screen_send_cmd(MADCTRL_REG, &st7735MadCtrlRegValue, 1);
    return 0;
}

int st7735_set_display_mode(DisplayMode_e mode)
{
    if((mode & DISPLAY_MODE_MIRROR_X) != 0)
    {
        st7735MadCtrlRegValue |= BIT(6);
    }
    else
    {
        st7735MadCtrlRegValue &= ~(BIT(6));
    }

    if((mode & DISPLAY_MODE_MIRROR_Y) != 0)
    {
        st7735MadCtrlRegValue |= BIT(7);
    }
    else
    {
        st7735MadCtrlRegValue &= ~(BIT(7));
    }

    if((mode & DISPLAY_MODE_SWAP_XY) != 0)
    {
        st7735MadCtrlRegValue |= BIT(5);
    }
    else
    {
        st7735MadCtrlRegValue &= ~(BIT(5));
    }

    hal_screen_send_cmd(MADCTRL_REG, &st7735MadCtrlRegValue, 1);
    return 0;
}

int st7735_set_display_pix_mode(DisplayPixMode_e mode)
{
    if(mode == DISPLAY_PIXMODE_RGB)
    {
        st7735MadCtrlRegValue &= ~(BIT(3));
    }
    else
    {
        st7735MadCtrlRegValue |= BIT(3);
    }
    hal_screen_send_cmd(MADCTRL_REG, &st7735MadCtrlRegValue, 1);
    return 0;
}

int st7735_set_display_data_fmt(DisplayDataFmt_e fmt)
{
    uint8_t value = 0x00;
    if(fmt == DISPLAY_DATA_FMT_RGB565)
    {
        value = 0x05;
        st7735Drv.bpp = 16;
    }
    else if(fmt == DISPLAY_DATA_FMT_RGB444)
    {
        value = 0x03;
        st7735Drv.bpp = 12;
    }
    else if(fmt == DISPLAY_DATA_FMT_RGB666)
    {
        value = 0x06;
        st7735Drv.bpp = 18;
    }
    hal_screen_send_cmd(0x3A, &value, 1);
    return 0;
}

ScrConfig_t st7735_default_cfg = {
    .type = SCREEN_TYPE_ST7735,
    .drv_id = 0,
    .int_type = INF_SPI_4W_II,
    .bk_light_mode = SCREEN_BK_LIGHT_MODE_PWM,
    .pwm_cfg =
        {
            .pwm = 0,
            .pad = 16,
            .timer_port = 0,
        },
    .gpio_cfg =
        {
            .gpio_num = 0,
        },
    .reset_io_num = 19,
    .reset_level = 0,
    .init_backlight_level = 0,
    .freq = 72 * 1024 * 1024,
    .bpp = 16,
    .data_lane_num = 1,
    .width = 240,
    .height = 320,
    .x_offset = 0,
    .y_offset = 0,
    .te_circle = 16742,
    .te_wait_time = 623,
};

LcdDrvObj_t st7735Drv = {
    .id = 0x7735,
    .type = SCREEN_TYPE_ST7735,
    .width = 240,
    .height = 320,
    .x_offset = 0,
    .y_offset = 0,
    .bpp = 16,
    .data_lane_num = 1,
    .init_reg_list = st7735_init_table,
    .init_reg_list_len = sizeof(st7735_init_table) / sizeof(ScrRegList_t),
    .default_cfg = &st7735_default_cfg,
    .set_window = st7735_set_window,
    .set_direction = st7735_set_direction,
    .set_display_mode = st7735_set_display_mode,
    .set_display_pix_mode = st7735_set_display_pix_mode,
    .set_display_data_fmt = st7735_set_display_data_fmt,
    .set_backlight = st7735_set_backlight,
    .suspend = st7735_suspend,
    .resume = st7735_resume,
};

