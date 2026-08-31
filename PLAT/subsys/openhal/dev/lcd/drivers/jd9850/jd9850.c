#include "bsp.h"
#include "sctdef.h"
#include "hal_screen.h"
#include "jd9850.h"
#include DEBUG_LOG_HEADER_FILE

#define MADCTRL_REG 0x36
#define MADCTRL_REG_INIT_VALUE 0xD0
static uint8_t jd9850MadCtrlRegValue = MADCTRL_REG_INIT_VALUE;

LcdDrvObj_t jd9850Drv;
static ScrRegList_t jd9850_init_table[] = {
    {0x01, 1, {0}},
    {0xff, 1, {20}},
    {0xDF, 2, {0x98, 0x50}},
    {0xDE, 1, {0x00}},
    {0xB2, 1, {0x04}},
    {0xB7, 2, {0x16, 0x16}},
    {0xB9, 3, {0x17, 0x03, 0x00}},
    {0xBB, 6, {0x2A, 0x42, 0x80, 0xBB, 0x7F, 0xEF}},
    {0xC1, 5, {0x26, 0x03, 0x82, 0x03, 0x82}},
    {0xC3, 8, {0x01, 0xA8, 0xFE, 0x3F, 0x87, 0xF2, 0xA4, 0xA4}},
    {0xC4, 7, {0x00, 0x00, 0x40, 0x49, 0x50, 0x39, 0x16}},
    {0xC8, 32, {0x7F, 0x7D, 0x79, 0x70, 0x2B, 0x28, 0x22, 0x23,
                0x24, 0x26, 0x26, 0x24, 0x25, 0x24, 0x23, 0x80,
                0x3F, 0x3D, 0x39, 0x30, 0x2B, 0x28, 0x22, 0x23,
                0x24, 0x26, 0x26, 0x24, 0x25, 0x24, 0x23, 0x80}},
    {0xD0, 4, {0x06, 0x84, 0x98, 0x0F}},
    {0xD3, 2, {0x13, 0xFF}},
    {0xD7, 2, {0x6B, 0xE0}},
    {0xDE, 1, {0x01}},
    {0xB2, 2, {0x10, 0xA2}},
    {0xB7, 4, {0x19, 0x15, 0x1D, 0x20}},
    {0xC2, 3, {0x16, 0x00, 0xEE}},
    {0xC5, 2, {0x11, 0x00}},
    {0x2A, 4, {0x00, 0x00, 0x00, 0x7F}},
    {0x2B, 4, {0x00, 0x00, 0x00, 0x7F}},
    {0x35, 1, {0x00}},
    {0x36, 1, {MADCTRL_REG_INIT_VALUE}},
    {0x3A, 1, {0x05}},
    {0x11, 1, {0x00}},
    {0xFF, 1, {120}},
    {0x29, 0, {0x00}},
    {0xFF, 1, {20}},
    {0x2C, 1, {0x00}},
};

int jd9850_suspend(void)
{
    hal_screen_send_cmd(0x10, NULL, 0);
    osDelay(5);
    return 0;
}

int jd9850_resume(void)
{
    hal_screen_send_cmd(0x11, NULL, 0);
    osDelay(60);
    hal_screen_send_cmd(0x29, NULL, 0);
    return 0;
}

int jd9850_set_backlight(uint8_t level) { return 0; }

uint32_t jd9850_set_window(uint16_t sx, uint16_t sy, uint16_t width,
                           uint16_t height)
{
    uint16_t start_x = sx + jd9850Drv.x_offset;
    uint16_t start_y = sy + jd9850Drv.y_offset;
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
    return (width) * (height) * (jd9850Drv.bpp / 8);
}

int jd9850_set_direction(DisDirection_e dir)
{
    switch(dir)
    {
        case DIS_DIR_LRTB:
            jd9850MadCtrlRegValue &= ~(BIT(2));
            jd9850MadCtrlRegValue &= ~(BIT(4));
            jd9850MadCtrlRegValue &= ~(BIT(5));
            break;
        case DIS_DIR_LRBT:
            jd9850MadCtrlRegValue &= ~(BIT(2));
            jd9850MadCtrlRegValue &= ~(BIT(5));
            jd9850MadCtrlRegValue |= BIT(4);
            break;
        case DIS_DIR_RLTB:
            jd9850MadCtrlRegValue &= ~(BIT(4));
            jd9850MadCtrlRegValue &= ~(BIT(5));
            jd9850MadCtrlRegValue |= BIT(2);
            break;
        case DIS_DIR_RLBT:
            jd9850MadCtrlRegValue &= ~(BIT(5));
            jd9850MadCtrlRegValue |= BIT(4);
            jd9850MadCtrlRegValue |= BIT(2);
            break;
        case DIS_DIR_TBLR:
            jd9850MadCtrlRegValue &= ~(BIT(2));
            jd9850MadCtrlRegValue &= ~(BIT(4));
            jd9850MadCtrlRegValue |= BIT(5);
            break;
        case DIS_DIR_BTLR:
            jd9850MadCtrlRegValue &= ~(BIT(2));
            jd9850MadCtrlRegValue |= BIT(4);
            jd9850MadCtrlRegValue |= BIT(5);
            break;
        case DIS_DIR_TBRL:
            jd9850MadCtrlRegValue &= ~(BIT(4));
            jd9850MadCtrlRegValue |= BIT(2);
            jd9850MadCtrlRegValue |= BIT(5);
            break;
        case DIS_DIR_BTRL:
            jd9850MadCtrlRegValue |= BIT(2);
            jd9850MadCtrlRegValue |= BIT(4);
            jd9850MadCtrlRegValue |= BIT(5);
            break;
        default:
            break;
    }

    hal_screen_send_cmd(MADCTRL_REG, &jd9850MadCtrlRegValue, 1);
    return 0;
}

int jd9850_set_display_mode(DisplayMode_e mode)
{
    if((mode & DISPLAY_MODE_MIRROR_X) != 0)
    {
        jd9850MadCtrlRegValue |= BIT(6);
    }
    else
    {
        jd9850MadCtrlRegValue &= ~(BIT(6));
    }

    if((mode & DISPLAY_MODE_MIRROR_Y) != 0)
    {
        jd9850MadCtrlRegValue |= BIT(7);
    }
    else
    {
        jd9850MadCtrlRegValue &= ~(BIT(7));
    }

    if((mode & DISPLAY_MODE_SWAP_XY) != 0)
    {
        jd9850MadCtrlRegValue |= BIT(5);
    }
    else
    {
        jd9850MadCtrlRegValue &= ~(BIT(5));
    }

    hal_screen_send_cmd(MADCTRL_REG, &jd9850MadCtrlRegValue, 1);
    return 0;
}

int jd9850_set_display_pix_mode(DisplayPixMode_e mode)
{
    if(mode == DISPLAY_PIXMODE_RGB)
    {
        jd9850MadCtrlRegValue &= ~(BIT(3));
    }
    else
    {
        jd9850MadCtrlRegValue |= BIT(3);
    }
    hal_screen_send_cmd(MADCTRL_REG, &jd9850MadCtrlRegValue, 1);
    return 0;
}

int jd9850_set_display_data_fmt(DisplayDataFmt_e fmt)
{
    uint8_t value = 0x00;
    if(fmt == DISPLAY_DATA_FMT_RGB565)
    {
        value = 0x05;
        jd9850Drv.bpp = 16;
    }
    else if(fmt == DISPLAY_DATA_FMT_RGB444)
    {
        value = 0x03;
        jd9850Drv.bpp = 12;
    }
    else if(fmt == DISPLAY_DATA_FMT_RGB666)
    {
        value = 0x06;
        jd9850Drv.bpp = 18;
    }
    hal_screen_send_cmd(0x3A, &value, 1);
    return 0;
}

ScrConfig_t jd9850_default_cfg = {
    .type = SCREEN_TYPE_JD9850,
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

LcdDrvObj_t jd9850Drv = {
    .id = 0x9850,
    .type = SCREEN_TYPE_JD9850,
    .width = 240,
    .height = 320,
    .x_offset = 0,
    .y_offset = 0,
    .bpp = 16,
    .data_lane_num = 1,
    .init_reg_list = jd9850_init_table,
    .init_reg_list_len = sizeof(jd9850_init_table) / sizeof(ScrRegList_t),
    .default_cfg = &jd9850_default_cfg,
    .set_window = jd9850_set_window,
    .set_direction = jd9850_set_direction,
    .set_display_mode = jd9850_set_display_mode,
    .set_display_pix_mode = jd9850_set_display_pix_mode,
    .set_display_data_fmt = jd9850_set_display_data_fmt,
    .set_backlight = jd9850_set_backlight,
    .suspend = jd9850_suspend,
    .resume = jd9850_resume,
};
