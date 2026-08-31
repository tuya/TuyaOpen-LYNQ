#include "sctdef.h"
#include "gc6153.h"
#include "hal_cam.h"
SensorFuncObj_t gc6153_sns_func_obj;
uint32_t gc6153_1sdr_reg_list[] = {
    0xffff0032,
    0xfffe0032,
    // SYS
    0x00fe00a0,
    0x00fe00a0,
    0x00fe00a0,
    0x00fa0011,
    0x00fc0000,
    0x00f60000,
    0x00fc0012,
    // ANALOG & CISCTL
    0x00fe0000,
    0x00010040,
    0x00020012,
    0x000d0040,
    0x0014007c,  // 0x7e
    0x00160005,  // 0x05
    0x00170018,  // 0x18
    0x001c0031,
    0x001d00bb,
    0x001f003f,
    0x00730020,
    0x00740071,
    0x00770022,
    0x007a0008,
    0x00110018,
    0x00130048,
    0x001200c8,
    0x007000c8,
    0x007b0018,
    0x007d0030,
    0x007e0002,

    0x00fe0010,
    0x00fe0000,
    0x00fe0000,
    0x00fe0000,
    0x00fe0000,
    0x00fe0000,
    0x00fe0010,
    0x00fe0000,
    0x00490061,
    0x004a0040,
    0x004b0058,
    /*ISP*/
    0x00fe0000,
    0x00390002,
    0x003a0080,
    0x0020007e,
    0x002600a7,
    /*BLK*/
    0x00330010,
    0x00370006,
    0x002a0021,
    /*GAIN*/
    0x003f0016,
    /*DNDD*/
    0x005200a6,
    0x00530081,
    0x00540043,
    0x00560078,
    0x005700aa,
    0x005800ff,
    /*ASDE*/
    0x005b0060,
    0x005c0050,
    0x00ab002a,
    0x00ac00b5,
    /*INTPEE*/
    0x005e0006,
    0x005f0006,
    0x00600044,
    0x006100ff,
    0x00620069,
    0x00630018,
    /*CC*/
    0x00650013,
    0x00660026,
    0x00670007,
    0x006800f5,
    0x006900ea,
    0x006a0021,
    0x006b0021,
    0x006c00e4,
    0x006d00fb,
    /*YCP*/
    0x00810040,  // 0
    0x00820040,  // 0 : uyvy �ڰ�
    0x0083004b,
    0x00840090,
    0x008600f0,
    0x0087001d,
    0x00880016,
    0x008d0074,
    0x008e0025,
    /*AEC*/
    0x00900036,
    0x00920048,
    0x009d0032,
    0x009e0081,
    0x009f00f4,
    0x00a000a0,
    0x00a10004,
    0x00a3002d,
    0x00a40001,
    /*AWB*/
    0x00b000c2,
    0x00b1001e,
    0x00b20010,
    0x00b30020,
    0x00b4002d,
    0x00b5001b,
    0x00b6002e,
    0x00b80013,
    0x00ba0060,
    0x00bb0062,
    0x00bd0078,
    0x00be0055,
    0x00bf00a0,
    0x00c400e7,
    0x00c50015,
    0x00c60016,
    0x00c700eb,
    0x00c800e4,
    0x00c90016,
    0x00ca0016,
    0x00cb00e9,
    0x002200f8,
    /*SPI*/
    0x00fe0002,
    0x00010001,
    0x00020002,
    0x00030020,
    0x00040020,
    0x000a0000,
    //{0x13, 0x10},
    0x00240000,
    0x00280003,
    0x00fe0000,
    /*OUTPUT*/
    0x00f20003,
    0x00fe0000,
};

int gc6153_get_reg_list(uint8_t mode, uint32_t **reg_list, uint32_t *count)
{
    switch(mode)
    {
        case GC6153_IMG_MODE_SDR1BIT_240x320_15FPS: {
            *reg_list = gc6153_1sdr_reg_list;
            *count = (sizeof(gc6153_1sdr_reg_list) /
                      sizeof(gc6153_1sdr_reg_list[0]));
            gc6153_sns_func_obj.max_fps = 1500;
            break;
        }
        default: {
            return -1;
        }
    }
    return 0;
}

int gc6153_set_mirror_flip(uint8_t direct)
{
    if(direct == 3)
    {
        hal_cam_write_reg(GC6153_I2C_ADDR, 0x14, 0x6b);
        hal_cam_write_reg(GC6153_I2C_ADDR, 0x16, 0x04);
        hal_cam_write_reg(GC6153_I2C_ADDR, 0x17, 0x19);
    }
    else if(direct == 1)
    {
        hal_cam_write_reg(GC6153_I2C_ADDR, 0x14, 0x69);
        hal_cam_write_reg(GC6153_I2C_ADDR, 0x16, 0x04);
        hal_cam_write_reg(GC6153_I2C_ADDR, 0x17, 0x19);
    }
    else if(direct == 2)
    {
        hal_cam_write_reg(GC6153_I2C_ADDR, 0x14, 0x7e);
        hal_cam_write_reg(GC6153_I2C_ADDR, 0x16, 0x05);
        hal_cam_write_reg(GC6153_I2C_ADDR, 0x17, 0x18);
    }
    else
    {
        hal_cam_write_reg(GC6153_I2C_ADDR, 0x14, 0x7c);
        hal_cam_write_reg(GC6153_I2C_ADDR, 0x16, 0x05);
        hal_cam_write_reg(GC6153_I2C_ADDR, 0x17, 0x18);
    }
    return 0;
}

int gc6153_set_fps(uint32_t fps)
{
    uint32_t vts = GC6153_DEFAULT_VTS;
    uint32_t dummy = 0;
    uint8_t regValue = 0;
    if(fps > GC6153_DEFAULT_MAXIM_FPS)
    {
        fps = GC6153_DEFAULT_MAXIM_FPS;
    }
    if(fps < GC6153_DEFAULT_MINUM_FPS)
    {
        fps = GC6153_DEFAULT_MINUM_FPS;
    }
    vts = GC6153_DEFAULT_VTS * GC6153_DEFAULT_MAXIM_FPS / fps;
    dummy = vts - GC6153_DEFAULT_START_OFFSET_LINE_NUMS -
            GC6153_DEFAULT_ALL_LINE_NUMS;
    hal_cam_read_reg(GC6153_I2C_ADDR, 0x0f, &regValue);
    regValue &= 0x0f;
    regValue += (uint8_t)((dummy & 0x0f00) >> 4);
    hal_cam_write_reg(GC6153_I2C_ADDR, 0x0f, regValue);
    regValue = (uint8_t)(dummy & 0xFF);
    hal_cam_write_reg(GC6153_I2C_ADDR, 0x02, regValue);
    return 0;
}

int gc6153_get_sensor_id(uint32_t *id)
{
    uint8_t idH = 0;
    uint8_t idL = 0;
    hal_cam_read_reg(GC6153_I2C_ADDR, 0xf0, &idH);
    hal_cam_read_reg(GC6153_I2C_ADDR, 0xf1, &idL);
    *id = (((uint32_t)idH) << 8) + idL;
    return 0;
}
int gc6153_set_ev(uint8_t ev)
{
    uint8_t evValue = GC6153_DEFAULT_EV_VALUE;
    if(ev >= 50)
    {
        evValue =
            (GC6153_MAX_EV_VALUE - GC6153_DEFAULT_EV_VALUE) * (ev - 50) / 50 +
            GC6153_DEFAULT_EV_VALUE;
    }
    else
    {
        evValue = GC6153_MIN_EV_VALUE +
                  ((GC6153_DEFAULT_EV_VALUE - GC6153_MIN_EV_VALUE) * ev / 50);
    }
    hal_cam_write_reg(GC6153_I2C_ADDR, 0x92, evValue);
    return 0;
}

int gc6153_set_contrast(uint8_t contrast)
{
    uint8_t conValue = GC6153_DEFAULT_CONTRAST_VALUE;
    if(contrast >= 50)
    {
        conValue = (GC6153_MAX_CONTRAST_VALUE - GC6153_DEFAULT_CONTRAST_VALUE) *
                       (contrast - 50) / 50 +
                   GC6153_DEFAULT_CONTRAST_VALUE;
    }
    else
    {
        conValue =
            GC6153_MIN_CONTRAST_VALUE +
            ((GC6153_DEFAULT_CONTRAST_VALUE - GC6153_MIN_CONTRAST_VALUE) *
             contrast / 50);
    }
    hal_cam_write_reg(GC6153_I2C_ADDR, 0x83, conValue);
    return 0;
}

int gc6153_set_saturation(uint8_t sat)
{
    uint8_t satValue = GC6153_DEFAULT_SATURATION_VALUE;
    if(sat >= 50)
    {
        satValue =
            (GC6153_MAX_SATURATION_VALUE - GC6153_DEFAULT_SATURATION_VALUE) *
                (sat - 50) / 50 +
            GC6153_DEFAULT_SATURATION_VALUE;
    }
    else
    {
        satValue =
            GC6153_MIN_SATURATION_VALUE +
            ((GC6153_DEFAULT_SATURATION_VALUE - GC6153_MIN_SATURATION_VALUE) *
             sat / 50);
    }
    hal_cam_write_reg(GC6153_I2C_ADDR, 0x81, satValue);
    hal_cam_write_reg(GC6153_I2C_ADDR, 0x82, satValue);
    return 0;
}

int gc6153_set_sharpness(uint8_t sharp)
{
    uint8_t sharpValue = GC6153_DEFAULT_SHARPEN_VALUE;
    if(sharp >= 50)
    {
        sharpValue = (sharp - 50) *
                         ((GC6153_MAX_SHARPEN_VALUE & 0x0F) - sharpValue) / 50 +
                     (GC6153_DEFAULT_SHARPEN_VALUE & 0x0F);
    }
    else
    {
        sharpValue =
            (GC6153_MIN_SHARPEN_VALUE & 0x0F) +
            sharp * (sharpValue - (GC6153_MIN_SHARPEN_VALUE & 0x0F)) / 50;
    }
    uint8_t reg_value = 0;
    hal_cam_read_reg(GC6153_I2C_ADDR, 0x63, &reg_value);
    reg_value = (reg_value & 0xF0) + sharpValue;
    hal_cam_write_reg(GC6153_I2C_ADDR, 0x63, reg_value);
    return 0;
}

int gc6153_set_awb(bool awbEnable, uint8_t scene)
{
    uint8_t reg_value = 0;
    hal_cam_read_reg(GC6153_I2C_ADDR, 0x22, &reg_value);
    uint8_t rgain = 0;
    uint8_t ggain = 0;
    uint8_t bgain = 0;
    switch(scene)
    {
        case CAM_WB_AUTO:
            reg_value |= 0x40;
            break;
        case CAM_WB_CLOUD:
            reg_value &= ~0x60;
            rgain = 0xb0;
            ggain = 0x40;
            bgain = 0x40;
            break;
        case CAM_WB_DAYLIGHT:
            reg_value &= ~0x40;
            rgain = 0x7b;
            ggain = 0x40;
            bgain = 0x40;
            break;
        case CAM_WB_INCANDESCENCE:
            reg_value &= ~0x40;
            rgain = 0x58;
            ggain = 0x40;
            bgain = 0x98;
            break;
        case CAM_WB_FLUORESCENT:
            reg_value &= ~0x40;
            rgain = 0x40;
            ggain = 0x42;
            bgain = 0x80;
            break;
        case CAM_WB_TUNGSTEN:
            reg_value &= ~0x40;
            rgain = 0x40;
            ggain = 0x4e;
            bgain = 0xa8;
            break;
        default:
            return -1;
    }
    hal_cam_write_reg(GC6153_I2C_ADDR, 0x22, reg_value);
    hal_cam_write_reg(GC6153_I2C_ADDR, 0x49, rgain);
    hal_cam_write_reg(GC6153_I2C_ADDR, 0x4a, ggain);
    hal_cam_write_reg(GC6153_I2C_ADDR, 0x4b, bgain);
    return 0;
}

int gc6153_set_gamma(uint8_t *table, uint32_t size) { return -1; }

int gc6153_set_ae(uint8_t aeMode)
{
    uint8_t reg_value = 0;
    reg_value = aeMode == 0 ? 0x01 : 0x00;
    hal_cam_write_reg(GC6153_I2C_ADDR, 0xa4, reg_value);
    return 0;
}

int gc6153_set_scene(uint8_t scene)
{
    switch(scene)
    {
        case CAM_SCENE_DAY: {
            hal_cam_write_reg(GC6153_I2C_ADDR, 0x9e, 0x61);
            hal_cam_write_reg(GC6153_I2C_ADDR, 0x9f, 0x40);
            break;
        }
        case CAM_SCENE_NIGHT: {
            hal_cam_write_reg(GC6153_I2C_ADDR, 0x9e, 0x66);
            hal_cam_write_reg(GC6153_I2C_ADDR, 0x9f, 0x40);
            break;
        }
        default:
            return -1;
    }
    return 0;
}

int gc6153_power_down(void)
{
    hal_cam_write_reg(GC6153_I2C_ADDR, 0xf2, 0x00);
    return 0;
}

int gc6153_power_up(void)
{
    hal_cam_write_reg(GC6153_I2C_ADDR, 0xf2, 0x03);
    return 0;
}

CamCfg_t gc6153_default_cfg = {
    .drv_id = CSPI_1,
    .int_mode = GC6153_IMG_MODE_SDR1BIT_240x320_15FPS,
    .mclk_freq = CAM_24_M,
    .reso = CAM_8W_COLOR,
    .img_out_wnd = false,
    .wnd_cfg = {.start_x = 0, .start_y = 0, .width = 0, .height = 0},
    .seq_cfg.clock_delay = 0,
    .spi_cfg =
        {
            .endianMode = CAM_LSB_MODE,
            .wireNum = WIRE_1,
            .rxSeq = SEQ_1,
            .cpol = 0,
            .cpha = 1,
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

SensorFuncObj_t gc6153_sns_func_obj = {
    .type = CAM_TYPE_GC6153,
    .sensor_id = 0x6153,
    .img_width = 240,
    .img_height = 320,
    .max_fps = 1500,  // actural fps * 100
    .dev_addr = GC6153_I2C_ADDR,
    .reg_addr_size = 1,
    .reg_data_size = 1,
    .default_cfg = &gc6153_default_cfg,
    .pfn_set_mirror_flip = gc6153_set_mirror_flip,
    .pfn_set_fps = gc6153_set_fps,
    .pfn_get_sensor_id = gc6153_get_sensor_id,
    .pfn_set_ev = gc6153_set_ev,
    .pfn_set_contrast = gc6153_set_contrast,
    .pfn_set_saturation = gc6153_set_saturation,
    .pfn_set_sharp = gc6153_set_sharpness,
    .pfn_set_awb = gc6153_set_awb,
    .pfn_set_gamma = gc6153_set_gamma,
    .pfn_set_ae = gc6153_set_ae,
    .pfn_set_scene = gc6153_set_scene,
    .pfn_get_init_reg_list = gc6153_get_reg_list,
    .pfn_power_down = gc6153_power_down,
    .pfn_power_up = gc6153_power_up,
};
