#include "bsp.h"
#include "sctdef.h"
#include "hal_screen.h"
#include "co5300.h"
#include DEBUG_LOG_HEADER_FILE

#define MADCTRL_REG 0x36
#define MADCTRL_REG_INIT_VALUE 0x08
static uint8_t co5300MadCtrlRegValue = MADCTRL_REG_INIT_VALUE;
LcdDrvObj_t co5300Drv;
/**
  \brief CO5300 LCD initialization command sequence
  \details This table contains all the commands needed to initialize the CO5300 LCD controller.
           Each entry consists of a command, data length, and data bytes. Special command 0xFF
           is used for delays where the delay time is specified in the first data byte.
*/
static ScrRegList_t co5300_init_table[] = {
    {0xFE, 1, {0x00}},                      // Set extended command set
    {0xC4, 1, {0x80}},                      // Display control
    {0x3A, 1, {0x55}},                      // Pixel format: RGB565 (16-bit)
    {0x35, 1, {0x00}},                      // Tearing effect line off
    {0x36, 1, {0x08}},                      // Memory data access control
    {0x53, 1, {0x20}},                      // Brightness control: BCTRL=1, Display dimming on
    {0x51, 1, {0xFF}},                      // Write display brightness: Maximum brightness
    {0x63, 1, {0xFF}},                      // Display enhancement control
    {0x11, 0, {}},                          // Sleep out
    {0xff, 1, {60}},                        // Delay 60ms
    {0x29, 0, {}},                          // Display on
};
int co5300_suspend(void)
{
    hal_screen_send_cmd(0x10, NULL, 0);
    osDelay(5);
    return 0;
}

int co5300_resume(void)
{
    hal_screen_send_cmd(0x11, NULL, 0);
    osDelay(60);
    hal_screen_send_cmd(0x29, NULL, 0);
    return 0;
}

int co5300_set_backlight(uint8_t level) { return 0; }

uint32_t co5300_set_window(uint16_t sx, uint16_t sy, uint16_t width,
                           uint16_t height)
{
    uint16_t start_x = sx + co5300Drv.x_offset;
    uint16_t start_y = sy + co5300Drv.y_offset;
    uint16_t end_x = start_x + width - 1;
    uint16_t end_y = start_y + height - 1;
    hal_screen_set_mspi(true, MSPI_DATA_LANE_1, MSPI_DATA_LANE_1, 0x02);
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
    hal_screen_set_mspi(true, MSPI_DATA_LANE_1, MSPI_DATA_LANE_4, 0x32);
    hal_screen_send_cmd(0x2C, NULL, 0);
    return (width) * (height) * (co5300Drv.bpp / 8);
}

int co5300_set_direction(DisDirection_e dir)
{
    switch(dir)
    {
        case DIS_DIR_LRTB:
            co5300MadCtrlRegValue &= ~(BIT(2));
            co5300MadCtrlRegValue &= ~(BIT(4));
            co5300MadCtrlRegValue &= ~(BIT(5));
            break;
        case DIS_DIR_LRBT:
            co5300MadCtrlRegValue &= ~(BIT(2));
            co5300MadCtrlRegValue &= ~(BIT(5));
            co5300MadCtrlRegValue |= BIT(4);
            break;
        case DIS_DIR_RLTB:
            co5300MadCtrlRegValue &= ~(BIT(4));
            co5300MadCtrlRegValue &= ~(BIT(5));
            co5300MadCtrlRegValue |= BIT(2);
            break;
        case DIS_DIR_RLBT:
            co5300MadCtrlRegValue &= ~(BIT(5));
            co5300MadCtrlRegValue |= BIT(4);
            co5300MadCtrlRegValue |= BIT(2);
            break;
        case DIS_DIR_TBLR:
            co5300MadCtrlRegValue &= ~(BIT(2));
            co5300MadCtrlRegValue &= ~(BIT(4));
            co5300MadCtrlRegValue |= BIT(5);
            break;
        case DIS_DIR_BTLR:
            co5300MadCtrlRegValue &= ~(BIT(2));
            co5300MadCtrlRegValue |= BIT(4);
            co5300MadCtrlRegValue |= BIT(5);
            break;
        case DIS_DIR_TBRL:
            co5300MadCtrlRegValue &= ~(BIT(4));
            co5300MadCtrlRegValue |= BIT(2);
            co5300MadCtrlRegValue |= BIT(5);
            break;
        case DIS_DIR_BTRL:
            co5300MadCtrlRegValue |= BIT(2);
            co5300MadCtrlRegValue |= BIT(4);
            co5300MadCtrlRegValue |= BIT(5);
            break;
        default:
            break;
    }
    hal_screen_set_mspi(true, MSPI_DATA_LANE_1, MSPI_DATA_LANE_1, 0x02);
    hal_screen_send_cmd(MADCTRL_REG, &co5300MadCtrlRegValue, 1);
    return 0;
}

int co5300_set_display_mode(DisplayMode_e mode)
{
    if((mode & DISPLAY_MODE_MIRROR_X) != 0)
    {
        co5300MadCtrlRegValue |= BIT(6);
    }
    else
    {
        co5300MadCtrlRegValue &= ~(BIT(6));
    }

    if((mode & DISPLAY_MODE_MIRROR_Y) != 0)
    {
        co5300MadCtrlRegValue |= BIT(7);
    }
    else
    {
        co5300MadCtrlRegValue &= ~(BIT(7));
    }

    if((mode & DISPLAY_MODE_SWAP_XY) != 0)
    {
        co5300MadCtrlRegValue |= BIT(5);
    }
    else
    {
        co5300MadCtrlRegValue &= ~(BIT(5));
    }
    hal_screen_set_mspi(true, MSPI_DATA_LANE_1, MSPI_DATA_LANE_1, 0x02);
    hal_screen_send_cmd(MADCTRL_REG, &co5300MadCtrlRegValue, 1);
    return 0;
}

int co5300_set_display_pix_mode(DisplayPixMode_e mode)
{
    if(mode == DISPLAY_PIXMODE_RGB)
    {
        co5300MadCtrlRegValue &= ~(BIT(3));
    }
    else
    {
        co5300MadCtrlRegValue |= BIT(3);
    }
    hal_screen_set_mspi(true, MSPI_DATA_LANE_1, MSPI_DATA_LANE_1, 0x02);
    hal_screen_send_cmd(MADCTRL_REG, &co5300MadCtrlRegValue, 1);
    return 0;
}

int co5300_set_display_data_fmt(DisplayDataFmt_e fmt)
{
    uint8_t value = 0x00;
    if(fmt == DISPLAY_DATA_FMT_RGB565)
    {
        value = 0x05;
        co5300Drv.bpp = 16;
    }
    else if(fmt == DISPLAY_DATA_FMT_RGB444)
    {
        value = 0x03;
        co5300Drv.bpp = 12;
    }
    else if(fmt == DISPLAY_DATA_FMT_RGB666)
    {
        value = 0x06;
        co5300Drv.bpp = 18;
    }
    hal_screen_set_mspi(true, MSPI_DATA_LANE_1, MSPI_DATA_LANE_1, 0x02);
    hal_screen_send_cmd(0x3A, &value, 1);
    return 0;
}

ScrConfig_t co5300_default_cfg = {
    .type = SCREEN_TYPE_CO5300,
    .drv_id = 0,
    .int_type = INF_MSPI_4W_II,
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

LcdDrvObj_t co5300Drv = {
    .id = 0x5300,
    .type = SCREEN_TYPE_CO5300,
    .width = 240,
    .height = 320,
    .x_offset = 0,
    .y_offset = 0,
    .bpp = 16,
    .data_lane_num = 1,
    .init_reg_list = co5300_init_table,
    .init_reg_list_len = sizeof(co5300_init_table) / sizeof(ScrRegList_t),
    .default_cfg = &co5300_default_cfg,
    .set_window = co5300_set_window,
    .set_direction = co5300_set_direction,
    .set_display_mode = co5300_set_display_mode,
    .set_display_pix_mode = co5300_set_display_pix_mode,
    .set_display_data_fmt = co5300_set_display_data_fmt,
    .set_backlight = co5300_set_backlight,
    .suspend = co5300_suspend,
    .resume = co5300_resume,
};

