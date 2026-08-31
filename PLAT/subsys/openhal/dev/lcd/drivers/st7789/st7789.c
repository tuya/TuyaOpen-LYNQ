#include "bsp.h"
#include "sctdef.h"
#include "hal_screen.h"
#include "st7789.h"
#include DEBUG_LOG_HEADER_FILE

#define MADCTRL_REG 0x36
#define MADCTRL_REG_INIT_VALUE 0x00
static uint8_t st7789MadCtrlRegValue = MADCTRL_REG_INIT_VALUE;
LcdDrvObj_t st7789Drv;
static ScrRegList_t st7789_init_table[] = {
    {0x11, 0},
    {0xff, 1, {120}},
    {0x36, 1, {MADCTRL_REG_INIT_VALUE}},
    {0x21, 1, {0}},  // display inverse
    {0x3a, 1, {5}},  // rgb565
    {0x35, 1, {0}},  // te on
    {0xe7, 1, {0x00}},
    {0xb2, 5, {0x0c, 0x0c, 0x00, 0x33, 0x33}},  // rate
    {0xb7, 1, {0x05}},
    {0xbb, 1, {0x20}},
    {0xc0, 1, {0x2c}},
    {0xc2, 1, {0x01}},
    {0xc3, 1, {0x15}},
    {0xc4, 1, {0x20}},
    {0xc6, 1, {0x0F}},        // try frame rate 105Hz
    {0xd0, 2, {0xa4, 0xa1}},  // pwctr
    {0xd6, 1, {0xa1}},
    {0xe0,
     14,
     {0xd0, 0x03, 0x09, 0x0e, 0x11, 0x3d, 0x47, 0x55, 0x53, 0x1a, 0x16, 0x14,
      0x1f, 0x22}},  // Positive voltage gamma
    {0xe1,
     14,
     {0xd0, 0x02, 0x08, 0x0d, 0x12, 0x2c, 0x43, 0x55, 0x53, 0x1e, 0x1b, 0x19,
      0x20, 0x22}},  // Negative voltage gamma
    {0x29, 0},
};

int st7789_suspend(void)
{
    hal_screen_send_cmd(0x10, NULL, 0);
    osDelay(5);
    return 0;
}

int st7789_resume(void)
{
    hal_screen_send_cmd(0x11, NULL, 0);
    osDelay(60);
    hal_screen_send_cmd(0x29, NULL, 0);
    return 0;
}

int st7789_set_backlight(uint8_t level) { return 0; }

uint32_t st7789_set_window(uint16_t sx, uint16_t sy, uint16_t width,
                           uint16_t height)
{
    uint16_t start_x = sx + st7789Drv.x_offset;
    uint16_t start_y = sy + st7789Drv.y_offset;
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
    return (width) * (height) * (st7789Drv.bpp / 8);
}

int st7789_set_direction(DisDirection_e dir)
{
    switch(dir)
    {
        case DIS_DIR_LRTB:
            st7789MadCtrlRegValue &= ~(BIT(2));
            st7789MadCtrlRegValue &= ~(BIT(4));
            st7789MadCtrlRegValue &= ~(BIT(5));
            break;
        case DIS_DIR_LRBT:
            st7789MadCtrlRegValue &= ~(BIT(2));
            st7789MadCtrlRegValue &= ~(BIT(5));
            st7789MadCtrlRegValue |= BIT(4);
            break;
        case DIS_DIR_RLTB:
            st7789MadCtrlRegValue &= ~(BIT(4));
            st7789MadCtrlRegValue &= ~(BIT(5));
            st7789MadCtrlRegValue |= BIT(2);
            break;
        case DIS_DIR_RLBT:
            st7789MadCtrlRegValue &= ~(BIT(5));
            st7789MadCtrlRegValue |= BIT(4);
            st7789MadCtrlRegValue |= BIT(2);
            break;
        case DIS_DIR_TBLR:
            st7789MadCtrlRegValue &= ~(BIT(2));
            st7789MadCtrlRegValue &= ~(BIT(4));
            st7789MadCtrlRegValue |= BIT(5);
            break;
        case DIS_DIR_BTLR:
            st7789MadCtrlRegValue &= ~(BIT(2));
            st7789MadCtrlRegValue |= BIT(4);
            st7789MadCtrlRegValue |= BIT(5);
            break;
        case DIS_DIR_TBRL:
            st7789MadCtrlRegValue &= ~(BIT(4));
            st7789MadCtrlRegValue |= BIT(2);
            st7789MadCtrlRegValue |= BIT(5);
            break;
        case DIS_DIR_BTRL:
            st7789MadCtrlRegValue |= BIT(2);
            st7789MadCtrlRegValue |= BIT(4);
            st7789MadCtrlRegValue |= BIT(5);
            break;
        default:
            break;
    }

    hal_screen_send_cmd(MADCTRL_REG, &st7789MadCtrlRegValue, 1);
    return 0;
}

int st7789_set_display_mode(DisplayMode_e mode)
{
    if((mode & DISPLAY_MODE_MIRROR_X) != 0)
    {
        st7789MadCtrlRegValue |= BIT(6);
    }
    else
    {
        st7789MadCtrlRegValue &= ~(BIT(6));
    }

    if((mode & DISPLAY_MODE_MIRROR_Y) != 0)
    {
        st7789MadCtrlRegValue |= BIT(7);
    }
    else
    {
        st7789MadCtrlRegValue &= ~(BIT(7));
    }

    if((mode & DISPLAY_MODE_SWAP_XY) != 0)
    {
        st7789MadCtrlRegValue |= BIT(5);
    }
    else
    {
        st7789MadCtrlRegValue &= ~(BIT(5));
    }
    printf("st7789 0x36 regvalue: 0x%x\r\n", st7789MadCtrlRegValue);
    hal_screen_send_cmd(MADCTRL_REG, &st7789MadCtrlRegValue, 1);
    return 0;
}

int st7789_set_display_pix_mode(DisplayPixMode_e mode)
{
    if(mode == DISPLAY_PIXMODE_RGB)
    {
        st7789MadCtrlRegValue &= ~(BIT(3));
    }
    else
    {
        st7789MadCtrlRegValue |= BIT(3);
    }
    hal_screen_send_cmd(MADCTRL_REG, &st7789MadCtrlRegValue, 1);
    return 0;
}

int st7789_set_display_data_fmt(DisplayDataFmt_e fmt)
{
    uint8_t value = 0x00;
    if(fmt == DISPLAY_DATA_FMT_RGB565)
    {
        value = 0x05;
        st7789Drv.bpp = 16;
    }
    else if(fmt == DISPLAY_DATA_FMT_RGB444)
    {
        value = 0x03;
        st7789Drv.bpp = 12;
    }
    else if(fmt == DISPLAY_DATA_FMT_RGB666)
    {
        value = 0x06;
        st7789Drv.bpp = 18;
    }
    hal_screen_send_cmd(0x3A, &value, 1);
    return 0;
}

ScrConfig_t st7789_default_cfg = {
    .type = SCREEN_TYPE_ST7789,
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

LcdDrvObj_t st7789Drv = {
    .id = 0x7789,
    .type = SCREEN_TYPE_ST7789,
    .width = 240,
    .height = 320,
    .x_offset = 0,
    .y_offset = 0,
    .bpp = 16,
    .data_lane_num = 1,
    .init_reg_list = st7789_init_table,
    .init_reg_list_len = sizeof(st7789_init_table) / sizeof(ScrRegList_t),
    .default_cfg = &st7789_default_cfg,
    .set_window = st7789_set_window,
    .set_direction = st7789_set_direction,
    .set_display_mode = st7789_set_display_mode,
    .set_display_pix_mode = st7789_set_display_pix_mode,
    .set_display_data_fmt = st7789_set_display_data_fmt,
    .set_backlight = st7789_set_backlight,
    .suspend = st7789_suspend,
    .resume = st7789_resume,
};
