#include "bsp.h"
#include "sctdef.h"
#include "hal_screen.h"
#include "jd9853.h"
#include DEBUG_LOG_HEADER_FILE

#define MADCTRL_REG 0x36
#define MADCTRL_REG_INIT_VALUE 0x00
static uint8_t jd9853MadCtrlRegValue = MADCTRL_REG_INIT_VALUE;

LcdDrvObj_t jd9853Drv;
static ScrRegList_t jd9853_init_table[] = {
    {0x01, 1, {0}},
    {0xff, 1, {40}},
    {0xDF, 2, {0x98, 0x53}},
    {0xDE, 1, {0x00}},
    {0xB2, 1, {0x24}},
    {0xB7, 4, {0x00, 0x21, 0x00, 0x49}},
    {0xBB, 6, {0x1E, 0x2F, 0x55, 0x71, 0x73, 0xF0}},
    {0xC0, 2, {0x22, 0xA2}},
    {0xC1, 1, {0x12}},
    {0xC3, 8, {0x7D, 0x08, 0x0A, 0x0C, 0xC4, 0x73, 0x22, 0x77}},
    {0xC4, 12, {0x00, 0x00, 0xA0, 0x79, 0x0A, 0x0B, 0x16, 0x79, 0x0A, 0x0B, 0x16, 0x82}},
    {0xC8, 32, {0x3F, 0x32, 0x2A, 0x24, 0x29, 0x2B, 0x26, 0x24,
                0x23, 0x22, 0x21, 0x15, 0x11, 0x0B, 0x04, 0x00,
                0x3F, 0x32, 0x2A, 0x24, 0x29, 0x2B, 0x26, 0x24,
                0x23, 0x22, 0x21, 0x15, 0x11, 0x0B, 0x04, 0x00}},
    {0xD0, 5, {0x04, 0x04, 0x6C, 0x1C, 0x03}},
    {0xD7, 2, {0x00, 0x20}},
    {0xE6, 1, {0x10}},
    {0xDE, 1, {0x01}},
    {0xBB, 1, {0x04}},
    {0xD7, 1, {0x12}},
    {0xB7, 5, {0x03, 0x13, 0xE5, 0x38, 0x38}},
    {0xC1, 3, {0x14, 0x15, 0xC0}},
    {0xC2, 2, {0x06, 0x3A}},
    {0xC4, 2, {0x72, 0x12}},
    {0xBE, 1, {0x00}},
    {0xDE, 1, {0x00}},
    {0x35, 1, {0x00}},
    {0x36, 1, {MADCTRL_REG_INIT_VALUE}},
    {0x3A, 1, {0x05}},
    {0x2A, 4, {0x00, 0x00, 0x00, 0xEF}},
    {0x2B, 4, {0x00, 0x00, 0x01, 0x3F}},
    {0x11, 1, {0x00}},
    {0xFF, 1, {120}},
    {0x29, 0, {0x00}},
    {0xFF, 1, {10}},
};

int jd9853_suspend(void)
{
    hal_screen_send_cmd(0x10, NULL, 0);
    osDelay(5);
    return 0;
}

int jd9853_resume(void)
{
    hal_screen_send_cmd(0x11, NULL, 0);
    osDelay(60);
    hal_screen_send_cmd(0x29, NULL, 0);
    return 0;
}

int jd9853_set_backlight(uint8_t level) { return 0; }

uint32_t jd9853_set_window(uint16_t sx, uint16_t sy, uint16_t width,
                           uint16_t height)
{
    uint16_t start_x = sx + jd9853Drv.x_offset;
    uint16_t start_y = sy + jd9853Drv.y_offset;
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
    return (width) * (height) * (jd9853Drv.bpp / 8);
}

int jd9853_set_direction(DisDirection_e dir)
{
    switch(dir)
    {
        case DIS_DIR_LRTB:
            jd9853MadCtrlRegValue &= ~(BIT(2));
            jd9853MadCtrlRegValue &= ~(BIT(4));
            jd9853MadCtrlRegValue &= ~(BIT(5));
            break;
        case DIS_DIR_LRBT:
            jd9853MadCtrlRegValue &= ~(BIT(2));
            jd9853MadCtrlRegValue &= ~(BIT(5));
            jd9853MadCtrlRegValue |= BIT(4);
            break;
        case DIS_DIR_RLTB:
            jd9853MadCtrlRegValue &= ~(BIT(4));
            jd9853MadCtrlRegValue &= ~(BIT(5));
            jd9853MadCtrlRegValue |= BIT(2);
            break;
        case DIS_DIR_RLBT:
            jd9853MadCtrlRegValue &= ~(BIT(5));
            jd9853MadCtrlRegValue |= BIT(4);
            jd9853MadCtrlRegValue |= BIT(2);
            break;
        case DIS_DIR_TBLR:
            jd9853MadCtrlRegValue &= ~(BIT(2));
            jd9853MadCtrlRegValue &= ~(BIT(4));
            jd9853MadCtrlRegValue |= BIT(5);
            break;
        case DIS_DIR_BTLR:
            jd9853MadCtrlRegValue &= ~(BIT(2));
            jd9853MadCtrlRegValue |= BIT(4);
            jd9853MadCtrlRegValue |= BIT(5);
            break;
        case DIS_DIR_TBRL:
            jd9853MadCtrlRegValue &= ~(BIT(4));
            jd9853MadCtrlRegValue |= BIT(2);
            jd9853MadCtrlRegValue |= BIT(5);
            break;
        case DIS_DIR_BTRL:
            jd9853MadCtrlRegValue |= BIT(2);
            jd9853MadCtrlRegValue |= BIT(4);
            jd9853MadCtrlRegValue |= BIT(5);
            break;
        default:
            break;
    }

    hal_screen_send_cmd(MADCTRL_REG, &jd9853MadCtrlRegValue, 1);
    return 0;
}

int jd9853_set_display_mode(DisplayMode_e mode)
{
    if((mode & DISPLAY_MODE_MIRROR_X) != 0)
    {
        jd9853MadCtrlRegValue |= BIT(6);
    }
    else
    {
        jd9853MadCtrlRegValue &= ~(BIT(6));
    }

    if((mode & DISPLAY_MODE_MIRROR_Y) != 0)
    {
        jd9853MadCtrlRegValue |= BIT(7);
    }
    else
    {
        jd9853MadCtrlRegValue &= ~(BIT(7));
    }

    if((mode & DISPLAY_MODE_SWAP_XY) != 0)
    {
        jd9853MadCtrlRegValue |= BIT(5);
    }
    else
    {
        jd9853MadCtrlRegValue &= ~(BIT(5));
    }

    hal_screen_send_cmd(MADCTRL_REG, &jd9853MadCtrlRegValue, 1);
    return 0;
}

int jd9853_set_display_pix_mode(DisplayPixMode_e mode)
{
    if(mode == DISPLAY_PIXMODE_RGB)
    {
        jd9853MadCtrlRegValue &= ~(BIT(3));
    }
    else
    {
        jd9853MadCtrlRegValue |= BIT(3);
    }
    hal_screen_send_cmd(MADCTRL_REG, &jd9853MadCtrlRegValue, 1);
    return 0;
}

int jd9853_set_display_data_fmt(DisplayDataFmt_e fmt)
{
    uint8_t value = 0x00;
    if(fmt == DISPLAY_DATA_FMT_RGB565)
    {
        value = 0x05;
        jd9853Drv.bpp = 16;
    }
    else if(fmt == DISPLAY_DATA_FMT_RGB444)
    {
        value = 0x03;
        jd9853Drv.bpp = 12;
    }
    else if(fmt == DISPLAY_DATA_FMT_RGB666)
    {
        value = 0x06;
        jd9853Drv.bpp = 18;
    }
    hal_screen_send_cmd(0x3A, &value, 1);
    return 0;
}

ScrConfig_t jd9853_default_cfg = {
    .type = SCREEN_TYPE_JD9853,
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

LcdDrvObj_t jd9853Drv = {
    .id = 0x9853,
    .type = SCREEN_TYPE_JD9853,
    .width = 240,
    .height = 320,
    .x_offset = 0,
    .y_offset = 0,
    .bpp = 16,
    .data_lane_num = 1,
    .init_reg_list = jd9853_init_table,
    .init_reg_list_len = sizeof(jd9853_init_table) / sizeof(ScrRegList_t),
    .default_cfg = &jd9853_default_cfg,
    .set_window = jd9853_set_window,
    .set_direction = jd9853_set_direction,
    .set_display_mode = jd9853_set_display_mode,
    .set_display_pix_mode = jd9853_set_display_pix_mode,
    .set_display_data_fmt = jd9853_set_display_data_fmt,
    .set_backlight = jd9853_set_backlight,
    .suspend = jd9853_suspend,
    .resume = jd9853_resume,
};
