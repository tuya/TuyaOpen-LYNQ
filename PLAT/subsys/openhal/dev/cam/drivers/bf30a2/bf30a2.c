#include "sctdef.h"
#include "bf30a2.h"
#include "hal_cam.h"

SensorFuncObj_t bf30a2_sns_func_obj;

uint32_t bf30a2_1sdr_reg_list[] = {
    0xffff0032, 0xfffe0032,
    0x00f20001,  // soft reset
    0x00cf00b0,  // power up
    0x00120020,  // MTK:20 ZX:10 RDA:40
    0x00150080, 0x006b0071, 0x00000040, 0x00040000, 0x00060026, 0x00080007,
    0x001c0012, 0x00200020, 0x00210020, 0x00340002, 0x00350002, 0x00360021,
    0x00370013, 0x00ca0023, 0x00cb0022, 0x00cc0089, 0x00cd004c, 0x00ce006b,
    0x00a0008e, 0x0001001b, 0x0002001d, 0x00130008, 0x00870013, 0x008b0008,
    0x0070001f, 0x00710040, 0x0072000a, 0x00730062, 0x007400a2, 0x007500bf,
    0x00760024, 0x007700cc, 0x00400032, 0x00410028, 0x00420026, 0x0043001d,
    0x0044001a, 0x00450014, 0x00460011, 0x0047000f, 0x0048000e, 0x0049000d,
    0x004B000c, 0x004C000b, 0x004E000a, 0x004F0009, 0x00500009, 0x00240042,
    0x00250036, 0x00800000, 0x00810020, 0x00820040, 0x00830030, 0x00840050,
    0x00850030, 0x008600D8, 0x00890045, 0x008a0033, 0x008f0081, 0x009100ff,
    0x00920008, 0x00940082, 0x009500fd, 0x009a0020, 0x009e00bc, 0x00f0008f,
    0x00510006, 0x00520025, 0x0053002b, 0x0054000F, 0x0057002A, 0x00580022,
    0x0059002c, 0x00230033, 0x00a10093, 0x00a2000f, 0x00a3002a, 0x00a40008,
    0x00a50026, 0x00a70080, 0x00a80080, 0x00a9001e, 0x00aa0019, 0x00ab0018,
    0x00ae0050, 0x00af0004, 0x00c80010, 0x00c90015, 0x00d3000c, 0x00d40016,
    0x00ee0006, 0x00ef0004, 0x00550034, 0x00560080, 0x00b10098, 0x00b20098,
    0x00b300c4, 0x00b4000C, 0x00a0008f, 0x00130007,
};

int bf30a2_get_reg_list(uint8_t mode, uint32_t **reg_list, uint32_t *count)
{
    switch(mode)
    {
        case BF30A2_IMG_MODE_SDR1BIT_240x320_15FPS: {
            *reg_list = bf30a2_1sdr_reg_list;
            *count = (sizeof(bf30a2_1sdr_reg_list) /
                      sizeof(bf30a2_1sdr_reg_list[0]));
            bf30a2_sns_func_obj.max_fps = 1500;
            break;
        }
        default: {
            return -1;
        }
    }
    return 0;
}

int bf30a2_set_mirror_flip(uint8_t direct)
{
    uint8_t reg_value = 0;
    hal_cam_read_reg(BF30A2_I2C_ADDR, 0x00, &reg_value);
    reg_value = reg_value & (~0x0C);
    if(direct == 3)
    {
        reg_value += 0x0C;
    }
    else if(direct == 1)
    {
        reg_value += 0x08;
    }
    else if(direct == 2)
    {
        reg_value += 0x04;
    }
    hal_cam_write_reg(BF30A2_I2C_ADDR, 0x00, reg_value);
    return 0;
}

int bf30a2_set_fps(uint32_t fps) { return 0; }

int bf30a2_get_sensor_id(uint32_t *id)
{
    uint8_t idH = 0;
    uint8_t idL = 0;
    hal_cam_read_reg(BF30A2_I2C_ADDR, 0xfc, &idH);
    hal_cam_read_reg(BF30A2_I2C_ADDR, 0xfd, &idL);
    *id = (((uint32_t)idH) << 8) + idL;
    return 0;
}

int bf30a2_set_ev(uint8_t ev)
{
    uint8_t evValue = BF30A2_DEFAULT_EV_VALUE;
    if(ev >= 50)
    {
        evValue =
            (BF30A2_MAX_EV_VALUE - BF30A2_DEFAULT_EV_VALUE) * (ev - 50) / 50 +
            BF30A2_DEFAULT_EV_VALUE;
    }
    else
    {
        evValue = BF30A2_MIN_EV_VALUE +
                  ((BF30A2_DEFAULT_EV_VALUE - BF30A2_MIN_EV_VALUE) * ev / 50);
    }
    hal_cam_write_reg(BF30A2_I2C_ADDR, 0x24, evValue);
    return 0;
}

int bf30a2_set_contrast(uint8_t contrast)
{
    uint8_t conValue = BF30A2_DEFAULT_CONTRAST_VALUE;
    if(contrast >= 50)
    {
        conValue = (BF30A2_MAX_CONTRAST_VALUE - BF30A2_DEFAULT_CONTRAST_VALUE) *
                       (contrast - 50) / 50 +
                   BF30A2_DEFAULT_CONTRAST_VALUE;
    }
    else
    {
        conValue =
            BF30A2_MIN_CONTRAST_VALUE +
            ((BF30A2_DEFAULT_CONTRAST_VALUE - BF30A2_MIN_CONTRAST_VALUE) *
             contrast / 50);
    }
    hal_cam_write_reg(BF30A2_I2C_ADDR, 0x56, conValue);
    return 0;
}

int bf30a2_set_saturation(uint8_t sat)
{
    uint8_t satValue = BF30A2_DEFAULT_SATURATION_VALUE;
    if(sat >= 50)
    {
        satValue =
            (BF30A2_MAX_SATURATION_VALUE - BF30A2_DEFAULT_SATURATION_VALUE) *
                (sat - 50) / 50 +
            BF30A2_DEFAULT_SATURATION_VALUE;
    }
    else
    {
        satValue =
            BF30A2_MIN_SATURATION_VALUE +
            ((BF30A2_DEFAULT_SATURATION_VALUE - BF30A2_MIN_SATURATION_VALUE) *
             sat / 50);
    }

    hal_cam_write_reg(BF30A2_I2C_ADDR, 0xb1, satValue);
    hal_cam_write_reg(BF30A2_I2C_ADDR, 0xb2, satValue);
    return 0;
}

int bf30a2_set_sharpness(uint8_t sharp)
{
    uint8_t sharpValue = 0;
    if(sharp >= 50)
    {
        sharpValue =
            (sharp - 50) *
                (BF30A2_MAX_SHARPEN_VALUE - BF30A2_DEFAULT_SHARPEN_VALUE) / 50 +
            BF30A2_DEFAULT_SHARPEN_VALUE;
    }
    else
    {
        sharpValue =
            BF30A2_MIN_SHARPEN_VALUE +

            sharp * (BF30A2_DEFAULT_SHARPEN_VALUE - BF30A2_MIN_SHARPEN_VALUE) /
                50;
    }
    uint8_t reg_value = 0;
    hal_cam_read_reg(BF30A2_I2C_ADDR, 0x63, &reg_value);
    reg_value &= ~(0x3F);
    reg_value = reg_value + sharpValue + (sharpValue << 3);
    hal_cam_write_reg(BF30A2_I2C_ADDR, 0x63, reg_value);
    return 0;
}

int bf30a2_set_awb(bool awbEnable, uint8_t scene)
{
    uint8_t reg_value = 0;
    uint8_t rgain = 0;
    uint8_t bgain = 0;
    hal_cam_read_reg(BF30A2_I2C_ADDR, 0xa0, &reg_value);
    switch(scene)
    {
        case CAM_WB_AUTO:
            reg_value |= 0x01;
            break;
        case CAM_WB_CLOUD:
            reg_value &= ~0x01;
            rgain = 0x0b;
            bgain = 0x2e;
            break;
        case CAM_WB_DAYLIGHT:
            reg_value &= ~0x01;
            rgain = 0x0f;
            bgain = 0x26;
            break;
        case CAM_WB_INCANDESCENCE:
            reg_value &= ~0x01;
            rgain = 0x2a;
            bgain = 0x0a;
            break;
        case CAM_WB_FLUORESCENT:
            reg_value &= ~0x01;
            rgain = 0x13;
            bgain = 0x1e;
            break;
        case CAM_WB_TUNGSTEN:
            reg_value &= ~0x01;
            rgain = 0x24;
            bgain = 0x10;
            break;
        default:
            return -1;
    }
    hal_cam_write_reg(BF30A2_I2C_ADDR, 0xa0, reg_value);
    hal_cam_write_reg(BF30A2_I2C_ADDR, 0x01, rgain);
    hal_cam_write_reg(BF30A2_I2C_ADDR, 0x02, bgain);
    return 0;
}

int bf30a2_set_gamma(uint8_t *table, uint32_t size)
{
    uint8_t reg_addr = 0;
    uint8_t reg_value = 0;
    if(size < BF30A2_GAMMA_TABLE_SIZE)
    {
        return -1;
    }
    for(int i = 0; i < BF30A2_GAMMA_TABLE_SIZE; i++)
    {
        reg_addr = (i < 10) ? (0x40 + i) : (0x40 + i + 1);
        reg_value = *(table + i);
        hal_cam_write_reg(BF30A2_I2C_ADDR, reg_addr, reg_value);
    }
    return 0;
}

int bf30a2_set_ae(uint8_t aeMode)
{
    uint8_t reg_value = 0;
    hal_cam_read_reg(BF30A2_I2C_ADDR, 0x13, &reg_value);
    reg_value &= ~(0x07);
    reg_value = aeMode == 0 ? (reg_value + 0x07) : reg_value;
    hal_cam_write_reg(BF30A2_I2C_ADDR, 0x13, reg_value);
    return 0;
}

int bf30a2_power_down(void)
{
    hal_cam_write_reg(BF30A2_I2C_ADDR, 0xcf, 0xb1);
    return 0;
}

int bf30a2_power_up(void)
{
    hal_cam_write_reg(BF30A2_I2C_ADDR, 0xcf, 0xb0);
    return 0;
}

int bf30a2_set_scene(uint8_t scene) { return 0; }

CamCfg_t bf30a2_default_cfg = {
    .drv_id = CSPI_1,
    .int_mode = BF30A2_IMG_MODE_SDR1BIT_240x320_15FPS,
    .mclk_freq = CAM_24_M,
    .reso = CAM_8W_COLOR,
    .img_out_wnd = false,
    .wnd_cfg = {.start_x = 0, .start_y = 0, .width = 0, .height = 0},
    .seq_cfg.clock_delay = 0,
    .spi_cfg =
        {
            .endianMode = CAM_LSB_MODE,
            .wireNum = WIRE_1,
            .rxSeq = SEQ_0,
            .cpol = 0,
            .cpha = 0,
            .ddrMode = 0,
            .wordIdSeq = 0,
            .yOnly = 0,
            .rowScaleRatio = 0,
            .colScaleRatio = 0,
            .scaleBytes = 3,
            .dummyAllowed = 0,
        },
    .ext_pwr_cfg =
        {
            .enable = 1,
            .pad_num = 11,
            .mux = PAD_MUX_ALT4,
            .io_num = 16,
        },
    .rst_pin_cfg =
        {
            .pad_num = 20,
            .mux = PAD_MUX_ALT0,
            .io_num = 5,
            .reset_level = 1,
        },
    .i2c_cfg =
        {
            .i2c_port = 0,
            .speed = OPEN_I2C_SPEED_100KHZ,
        },
    .qbuf_cfg = {
        .item_count = 3,
        .item_size = 320 * 240 * 2,
        .pool_addr = NULL,
    }
};

SensorFuncObj_t bf30a2_sns_func_obj = {
    .type = CAM_TYPE_BF30A2,
    .sensor_id = 0x3b02,
    .img_width = 240,
    .img_height = 320,
    .max_fps = 1500,  // actural fps * 100
    .dev_addr = BF30A2_I2C_ADDR,
    .reg_addr_size = 1,
    .reg_data_size = 1,
    .default_cfg = &bf30a2_default_cfg,
    .pfn_set_mirror_flip = bf30a2_set_mirror_flip,
    .pfn_set_fps = bf30a2_set_fps,
    .pfn_get_sensor_id = bf30a2_get_sensor_id,
    .pfn_set_ev = bf30a2_set_ev,
    .pfn_set_contrast = bf30a2_set_contrast,
    .pfn_set_saturation = bf30a2_set_saturation,
    .pfn_set_sharp = bf30a2_set_sharpness,
    .pfn_set_awb = bf30a2_set_awb,
    .pfn_set_gamma = bf30a2_set_gamma,
    .pfn_set_ae = bf30a2_set_ae,
    .pfn_set_scene = bf30a2_set_scene,
    .pfn_get_init_reg_list = bf30a2_get_reg_list,
    .pfn_power_down = bf30a2_power_down,
    .pfn_power_up = bf30a2_power_up,
};
