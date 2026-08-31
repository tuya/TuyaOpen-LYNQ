#include "bsp.h"
#include "sctdef.h"
#include "hal_screen.h"
#include "jd9855.h"
#include DEBUG_LOG_HEADER_FILE

#define MADCTRL_REG 0x36
#define MADCTRL_REG_INIT_VALUE 0x00
static uint8_t jd9855MadCtrlRegValue = MADCTRL_REG_INIT_VALUE;

LcdDrvObj_t jd9855Drv;
static ScrRegList_t jd9855_init_table[] = {
    {0xDE, 0},
    {0xDF, 2, {0x98, 0x55}},
    {0xCE, 2, {0x0D, 0x00}}, //00=D1 read；40=D0 read
    {0xB2, 1, {0x1F}},
    {0xB7, 4, {0x01, 0x2D, 0x01, 0x55}},
    {0xBB, 6, {0x1B, 0x64, 0xC4, 0x0E, 0x3E, 0xF5}},
    {0xBC, 4, {0x03, 0x22, 0xF3, 0xC0}},
    {0xC0, 2, {0x22, 0xA4}},
    {0xC3, 11, {0x00, 0x02, 0x2A, 0x0B, 0x08, 0x48, 0x08, 0x04, 0x62, 0x30, 0x30}},
    {0xC4, 11, {0x40, 0x00, 0xAD, 0x68, 0x43, 0x07, 0x04, 0x16, 0x43, 0x07, 0x04}},
    {0xC8, 32, {0x3F, 0x31, 0x28, 0x25, 0x25, 0x27, 0x22, 0x22,
                0x20, 0x1F, 0x1C, 0x12, 0x0F, 0x0B, 0x02, 0x02,
                0x3F, 0x31, 0x28, 0x25, 0x25, 0x27, 0x22, 0x22,
                0x20, 0x1F, 0x1C, 0x12, 0x0F, 0x0B, 0x02, 0x02}},
    {0xD3, 2, {0x28, 0x13}},
    {0xD7, 2, {0x00, 0x30}},
    {0xD9, 6, {0x00, 0x00, 0xFF, 0x00, 0xF0, 0x00}},
    {0xDE, 1, {0x01}},
    {0xB7, 8, {0x13, 0xE7, 0x64, 0x39, 0x06, 0x36, 0x19, 0x1C}},
    {0xBE, 1, {0x00}},
    {0xC1, 3, {0x05, 0x4A, 0x80}},
    {0xC2, 4, {0x00, 0x16, 0xDA, 0xE7}},
    {0xC7, 8, {0x00, 0x00, 0x00, 0x38, 0x08, 0x08, 0x00, 0x01}},
    {0xC8, 6, {0x00, 0x00, 0x00, 0x00, 0x15, 0x3B}},
    {0xC9, 5, {0x06, 0x04, 0x0A, 0x08, 0x1E}},
    {0xCA, 5, {0x00, 0x35, 0x15, 0x1F, 0x1F}},
    {0xCB, 5, {0x07, 0x05, 0x0B, 0x09, 0x1E}},
    {0xCC, 5, {0x01, 0x35, 0x15, 0x1F, 0x1F}},
    {0xCD, 5, {0x09, 0x0B, 0x05, 0x07, 0x1E}},
    {0xCE, 5, {0x01, 0x35, 0x1F, 0x15, 0x1F}},
    {0xCF, 5, {0x08, 0x0A, 0x04, 0x06, 0x1E}},
    {0xD0, 5, {0x00, 0x35, 0x1F, 0x15, 0x1F}},
    {0xD1, 2, {0x02, 0x30}},   
    {0xD2, 5, {0x02, 0x03, 0x52, 0xDF, 0xDD}},
    {0xD3, 3, {0x3B, 0x04, 0x48}},
    {0xD5, 7, {0x10, 0x10, 0x07, 0x07, 0x0F, 0x94, 0x26}},
    {0xD6, 3, {0x00, 0x00, 0x40}},
    {0xD7, 3, {0x00, 0x00, 0x20}},
    {0xDE, 1, {0x02}},
    {0xB6, 1, {0x1C}},
    {0xDE, 1, {0x00}},
    {0x4D, 1, {0x00}},
    {0x4E, 1, {0x00}},
    {0x4F, 1, {0x00}},
    {0x4C, 1, {0x01}},  
    {0xff, 1, {10}},
    {0x4C, 1, {0x00}},
    {MADCTRL_REG, 1, {MADCTRL_REG_INIT_VALUE}},
    {0x35, 0},
    {0x2A, 4, {0x00, 0x00, 0x01, 0x68}},
    {0x2B, 4, {0x00, 0x00, 0x01, 0x68}},
    {0x3A, 1, {0x55}},
    {0x11, 0},
    {0xff, 1, {120}},
    {0x29, 0},
    {0xff, 1, {20}},  
};

int jd9855_suspend(void)
{
    hal_screen_set_mspi(true, MSPI_DATA_LANE_1, MSPI_DATA_LANE_1, 0x02);
    hal_screen_send_cmd(0x10, NULL, 0);
    osDelay(5);
    return 0;
}

int jd9855_resume(void)
{
    hal_screen_set_mspi(true, MSPI_DATA_LANE_1, MSPI_DATA_LANE_1, 0x02);
    hal_screen_send_cmd(0x11, NULL, 0);
    osDelay(60);
    hal_screen_send_cmd(0x29, NULL, 0);
    return 0;
}

int jd9855_set_backlight(uint8_t level) { return 0; }

uint32_t jd9855_set_window(uint16_t sx, uint16_t sy, uint16_t width,
                           uint16_t height)
{
    uint16_t start_x = sx + jd9855Drv.x_offset;
    uint16_t start_y = sy + jd9855Drv.y_offset;
    uint16_t end_x = start_x + width - 1;
    uint16_t end_y = start_y + height - 1;
    uint8_t set_x_cmd[4] = {0};
    hal_screen_set_mspi(true, MSPI_DATA_LANE_1, MSPI_DATA_LANE_1, 0x02);
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

    hal_screen_set_mspi(true, MSPI_DATA_LANE_1, MSPI_DATA_LANE_4, 0x32);
    hal_screen_send_cmd(0x2C, NULL, 0);
    return (width) * (height) * (jd9855Drv.bpp / 8);
}

int jd9855_set_direction(DisDirection_e dir)
{
    switch(dir)
    {
        case DIS_DIR_LRTB:
            jd9855MadCtrlRegValue &= ~(BIT(2));
            jd9855MadCtrlRegValue &= ~(BIT(4));
            jd9855MadCtrlRegValue &= ~(BIT(5));
            break;
        case DIS_DIR_LRBT:
            jd9855MadCtrlRegValue &= ~(BIT(2));
            jd9855MadCtrlRegValue &= ~(BIT(5));
            jd9855MadCtrlRegValue |= BIT(4);
            break;
        case DIS_DIR_RLTB:
            jd9855MadCtrlRegValue &= ~(BIT(4));
            jd9855MadCtrlRegValue &= ~(BIT(5));
            jd9855MadCtrlRegValue |= BIT(2);
            break;
        case DIS_DIR_RLBT:
            jd9855MadCtrlRegValue &= ~(BIT(5));
            jd9855MadCtrlRegValue |= BIT(4);
            jd9855MadCtrlRegValue |= BIT(2);
            break;
        case DIS_DIR_TBLR:
            jd9855MadCtrlRegValue &= ~(BIT(2));
            jd9855MadCtrlRegValue &= ~(BIT(4));
            jd9855MadCtrlRegValue |= BIT(5);
            break;
        case DIS_DIR_BTLR:
            jd9855MadCtrlRegValue &= ~(BIT(2));
            jd9855MadCtrlRegValue |= BIT(4);
            jd9855MadCtrlRegValue |= BIT(5);
            break;
        case DIS_DIR_TBRL:
            jd9855MadCtrlRegValue &= ~(BIT(4));
            jd9855MadCtrlRegValue |= BIT(2);
            jd9855MadCtrlRegValue |= BIT(5);
            break;
        case DIS_DIR_BTRL:
            jd9855MadCtrlRegValue |= BIT(2);
            jd9855MadCtrlRegValue |= BIT(4);
            jd9855MadCtrlRegValue |= BIT(5);
            break;
        default:
            break;
    }
    hal_screen_set_mspi(true, MSPI_DATA_LANE_1, MSPI_DATA_LANE_1, 0x02);
    hal_screen_send_cmd(MADCTRL_REG, &jd9855MadCtrlRegValue, 1);
    return 0;
}

int jd9855_set_display_mode(DisplayMode_e mode)
{
    if((mode & DISPLAY_MODE_MIRROR_X) != 0)
    {
        jd9855MadCtrlRegValue |= BIT(6);
    }
    else
    {
        jd9855MadCtrlRegValue &= ~(BIT(6));
    }

    if((mode & DISPLAY_MODE_MIRROR_Y) != 0)
    {
        jd9855MadCtrlRegValue |= BIT(7);
    }
    else
    {
        jd9855MadCtrlRegValue &= ~(BIT(7));
    }

    if((mode & DISPLAY_MODE_SWAP_XY) != 0)
    {
        jd9855MadCtrlRegValue |= BIT(5);
    }
    else
    {
        jd9855MadCtrlRegValue &= ~(BIT(5));
    }
    hal_screen_set_mspi(true, MSPI_DATA_LANE_1, MSPI_DATA_LANE_1, 0x02);
    hal_screen_send_cmd(MADCTRL_REG, &jd9855MadCtrlRegValue, 1);
    return 0;
}

int jd9855_set_display_pix_mode(DisplayPixMode_e mode)
{
    if(mode == DISPLAY_PIXMODE_RGB)
    {
        jd9855MadCtrlRegValue &= ~(BIT(3));
    }
    else
    {
        jd9855MadCtrlRegValue |= BIT(3);
    }
    hal_screen_set_mspi(true, MSPI_DATA_LANE_1, MSPI_DATA_LANE_1, 0x02);
    hal_screen_send_cmd(MADCTRL_REG, &jd9855MadCtrlRegValue, 1);
    return 0;
}

int jd9855_set_display_data_fmt(DisplayDataFmt_e fmt)
{
    uint8_t value = 0x00;
    if(fmt == DISPLAY_DATA_FMT_RGB565)
    {
        value = 0x05;
        jd9855Drv.bpp = 16;
    }
    else if(fmt == DISPLAY_DATA_FMT_RGB444)
    {
        value = 0x03;
        jd9855Drv.bpp = 12;
    }
    else if(fmt == DISPLAY_DATA_FMT_RGB666)
    {
        value = 0x06;
        jd9855Drv.bpp = 18;
    }
    hal_screen_set_mspi(true, MSPI_DATA_LANE_1, MSPI_DATA_LANE_1, 0x02);
    hal_screen_send_cmd(0x3A, &value, 1);
    return 0;
}

ScrConfig_t jd9855_default_cfg = {
 .type = SCREEN_TYPE_JD9855,
    .drv_id = 0,
    .int_type = INF_MSPI_4W_II,
    .bk_light_mode = SCREEN_BK_LIGHT_MODE_PWM,
    .pwm_cfg =
        {
            .pwm = 2,
            .pad = 18,
            .timer_port = 2,
        },
    .gpio_cfg =
        {
            .gpio_num = 0,
        },
    .reset_io_num = 26,
    .reset_level = 0,
    .init_backlight_level = 0,
    .freq = 72 * 1024 * 1024,
    .bpp = 16,
    .data_lane_num = 1,
    .width = 360,
    .height = 360,
    .x_offset = 0,
    .y_offset = 0,
    .te_circle = 16742,
    .te_wait_time = 623,
};

LcdDrvObj_t jd9855Drv = {
    .id = 0x9855,
    .type = SCREEN_TYPE_JD9855,
    .width = 360,
    .height = 360,
    .x_offset = 0,
    .y_offset = 0,
    .bpp = 16,
    .data_lane_num = 1,
    .init_reg_list = jd9855_init_table,
    .init_reg_list_len = sizeof(jd9855_init_table) / sizeof(ScrRegList_t),
    .default_cfg = &jd9855_default_cfg,
    .set_window = jd9855_set_window,
    .set_direction = jd9855_set_direction,
    .set_display_mode = jd9855_set_display_mode,
    .set_display_pix_mode = jd9855_set_display_pix_mode,
    .set_display_data_fmt = jd9855_set_display_data_fmt,
    .set_backlight = jd9855_set_backlight,
    .suspend = jd9855_suspend,
    .resume = jd9855_resume,
};
