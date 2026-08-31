#include "sctdef.h"
#include "gc6133.h"
#include "hal_cam.h"
SensorFuncObj_t gc6133_sns_func_obj;
uint32_t gc6133_1sdr_reg_list[] = {
    0xffff0032,
    0xfffe0032,
    0x00fe00a0,
    0x00fe00a0,
    0x00fe00a0,
    0x00f60000,
    0x00fa0011,
    0x00fc0012,
    0x00fe0000,
    0x00fe0000,
    0x00490070,
    0x004a0040,
    0x004b005d,
    // ANALOG & CISCTL
    0x00030000,
    0x000400fa,
    0x00010041,
    0x00020012,
    0x000f0001,
    0x000d0030,
    0x001200c8,
    0x00140054,
    0x00150032,
    0x00160004,
    0x00170019,
    0x001d00b9,
    0x001f0035,
    0x007a0000,
    0x007b0014,
    0x007d0036,
    0x00fe0010,
    /*ISP*/
    0x0020007e,
    0x002200b8,
    0x00240054,
    0x002600a7,
    0x00390000,
    0x003a0080,
    0x003b0001,
    0x003c0040,
    0x003e00f0,
    /*BLK*/
    0x002a002f,
    0x00370046,
    /*GAIN*/
    0x003f0018,
    /*DNDD*/
    0x0050003c,
    0x0052004f,
    0x00530081,
    0x00540043,
    0x00560078,
    0x005700aa,
    0x005800ff,
    /*ASDE*/
    0x005b0060,
    0x005c0080,
    0x00ab0028,
    0x00ac00b5,
    /*INTPEE*/
    0x00600045,
    0x00620068,
    0x00630018,
    0x00640043,
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
    0x00810040,
    0x00820040,
    0x0083004a,
    0x00850006,
    0x008d0078,
    0x008e0025,
    /*AEC*/
    0x00900038,
    0x00920048,
    0x009d0032,
    0x009e0061,
    0x009f0040,
    0x00a30028,
    0x00a40001,
    /*AWB*/
    0x00b1001e,
    0x00b30020,
    0x00bd0070,
    0x00be0058,
    0x00bf00a0,
    0x004300a8,
    0x00b000f2,
    0x00b50040,
    0x00b80005,
    0x00ba0060,
    /*Banding*/
    0x00010041,
    0x00020012,
    0x000f0001,
    0x009d0032,
    0x009e0061,
    0x009f00f4,
    /*SPI*/
    0x00fe0002,
    0x00010001,
    0x00020002,
    0x00030020,
    0x00040020,
    0x000a0000,
    0x00130010,
    0x00240000,
    0x00280003,
    /*OUTPUT*/
    0x00fe0000,
    0x002200f8,
    0x00f10003,
};

int gc6133_get_reg_list(uint8_t mode, uint32_t **reg_list, uint32_t *count)
{
    switch(mode)
    {
        case GC6133_IMG_MODE_SDR1BIT_240x320_15FPS: {
            *reg_list = gc6133_1sdr_reg_list;
            *count = (sizeof(gc6133_1sdr_reg_list) /
                      sizeof(gc6133_1sdr_reg_list[0]));
            gc6133_sns_func_obj.max_fps = 1500;
            break;
        }
        default: {
            return -1;
        }
    }
    return 0;
}

int gc6133_set_mirror_flip(uint8_t direct)
{
    if(direct == 3)
    {
        hal_cam_write_reg(GC6133_I2C_ADDR, 0x14, 0x57);
    }
    else if(direct == 1)
    {
        hal_cam_write_reg(GC6133_I2C_ADDR, 0x14, 0x55);
    }
    else if(direct == 2)
    {
        hal_cam_write_reg(GC6133_I2C_ADDR, 0x14, 0x56);
    }
    else
    {
        hal_cam_write_reg(GC6133_I2C_ADDR, 0x14, 0x54);
    }
    return 0;
}

int gc6133_set_fps(uint32_t fps)
{
    uint32_t vts = GC6133_DEFAULT_VTS;
    uint32_t dummy = 0;
    uint8_t regValue = 0;
    if(fps > GC6133_DEFAULT_MAXIM_FPS)
    {
        fps = GC6133_DEFAULT_MAXIM_FPS;
    }
    if(fps < GC6133_DEFAULT_MINUM_FPS)
    {
        fps = GC6133_DEFAULT_MINUM_FPS;
    }
    vts = GC6133_DEFAULT_VTS * GC6133_DEFAULT_MAXIM_FPS / fps;
    dummy = vts - GC6133_DEFAULT_START_OFFSET_LINE_NUMS -
            GC6133_DEFAULT_ALL_LINE_NUMS;
    hal_cam_read_reg(GC6133_I2C_ADDR, 0x0f, &regValue);
    regValue &= 0x0f;
    regValue += (uint8_t)((dummy & 0x0f00) >> 4);
    hal_cam_write_reg(GC6133_I2C_ADDR, 0x0f, regValue);
    regValue = (uint8_t)(dummy & 0xFF);
    hal_cam_write_reg(GC6133_I2C_ADDR, 0x02, regValue);
    return 0;
}

int gc6133_get_sensor_id(uint32_t *id)
{
    uint8_t idH = 0;
    uint8_t idL = 0;
    hal_cam_read_reg(GC6133_I2C_ADDR, 0xf0, &idL);
    idH = 0;
    *id = (((uint32_t)idH) << 8) + idL;
    return 0;
}

int gc6133_set_ev(uint8_t ev)
{
    uint8_t evValue = GC6133_DEFAULT_EV_VALUE;
    if(ev >= 50)
    {
        evValue =
            (GC6133_MAX_EV_VALUE - GC6133_DEFAULT_EV_VALUE) * (ev - 50) / 50 +
            GC6133_DEFAULT_EV_VALUE;
    }
    else
    {
        evValue = GC6133_MIN_EV_VALUE +
                  ((GC6133_DEFAULT_EV_VALUE - GC6133_MIN_EV_VALUE) * ev / 50);
    }
    hal_cam_write_reg(GC6133_I2C_ADDR, 0x92, evValue);
    return 0;
}

int gc6133_set_contrast(uint8_t contrast)
{
    uint8_t conValue = GC6133_DEFAULT_CONTRAST_VALUE;
    if(contrast >= 50)
    {
        conValue = (GC6133_MAX_CONTRAST_VALUE - GC6133_DEFAULT_CONTRAST_VALUE) *
                       (contrast - 50) / 50 +
                   GC6133_DEFAULT_CONTRAST_VALUE;
    }
    else
    {
        conValue =
            GC6133_MIN_CONTRAST_VALUE +
            ((GC6133_DEFAULT_CONTRAST_VALUE - GC6133_MIN_CONTRAST_VALUE) *
             contrast / 50);
    }
    hal_cam_write_reg(GC6133_I2C_ADDR, 0x83, conValue);
    return 0;
}

int gc6133_set_saturation(uint8_t sat)
{
    uint8_t satValue = GC6133_DEFAULT_SATURATION_VALUE;
    if(sat >= 50)
    {
        satValue =
            (GC6133_MAX_SATURATION_VALUE - GC6133_DEFAULT_SATURATION_VALUE) *
                (sat - 50) / 50 +
            GC6133_DEFAULT_SATURATION_VALUE;
    }
    else
    {
        satValue =
            GC6133_MIN_SATURATION_VALUE +
            ((GC6133_DEFAULT_SATURATION_VALUE - GC6133_MIN_SATURATION_VALUE) *
             sat / 50);
    }
    hal_cam_write_reg(GC6133_I2C_ADDR, 0x81, satValue);
    hal_cam_write_reg(GC6133_I2C_ADDR, 0x82, satValue);
    return 0;
}

int gc6133_set_sharpness(uint8_t sharp)
{
    uint8_t sharpValue = GC6133_DEFAULT_SHARPEN_VALUE;
    if(sharp >= 50)
    {
        sharpValue = (sharp - 50) *
                         ((GC6133_MAX_SHARPEN_VALUE & 0x0F) - sharpValue) / 50 +
                     (GC6133_DEFAULT_SHARPEN_VALUE & 0x0F);
    }
    else
    {
        sharpValue =
            (GC6133_MIN_SHARPEN_VALUE & 0x0F) +
            sharp * (sharpValue - (GC6133_MIN_SHARPEN_VALUE & 0x0F)) / 50;
    }
    uint8_t reg_value = 0;
    hal_cam_read_reg(GC6133_I2C_ADDR, 0x63, &reg_value);
    reg_value = (reg_value & 0xF0) + sharpValue;
    hal_cam_write_reg(GC6133_I2C_ADDR, 0x63, reg_value);
    return 0;
}

int gc6133_set_awb(bool awbEnable, uint8_t scene)
{
    uint8_t reg_value = 0;
    hal_cam_read_reg(GC6133_I2C_ADDR, 0x22, &reg_value);
    uint8_t rgain = 0;
    uint8_t ggain = 0;
    uint8_t bgain = 0;
    switch(scene)
    {
        case CAM_WB_AUTO:
            reg_value |= 0x40;
            break;
        case CAM_WB_CLOUD:
            reg_value &= ~0x40;
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
    hal_cam_write_reg(GC6133_I2C_ADDR, 0x22, reg_value);
    hal_cam_write_reg(GC6133_I2C_ADDR, 0x49, rgain);
    hal_cam_write_reg(GC6133_I2C_ADDR, 0x4a, ggain);
    hal_cam_write_reg(GC6133_I2C_ADDR, 0x4b, bgain);
    return 0;
}

int gc6133_set_gamma(uint8_t *table, uint32_t size) { return -1; }

int gc6133_set_ae(uint8_t aeMode)
{
    uint8_t reg_value = 0;
    reg_value = aeMode == 0 ? 0x01 : 0x00;
    hal_cam_write_reg(GC6133_I2C_ADDR, 0xa4, reg_value);
    return 0;
}

int gc6133_set_scene(uint8_t scene)
{
    switch(scene)
    {
        case CAM_SCENE_DAY: {
            hal_cam_write_reg(GC6133_I2C_ADDR, 0x9e, 0x61);
            hal_cam_write_reg(GC6133_I2C_ADDR, 0x9f, 0x40);
            break;
        }
        case CAM_SCENE_NIGHT: {
            hal_cam_write_reg(GC6133_I2C_ADDR, 0x9e, 0x66);
            hal_cam_write_reg(GC6133_I2C_ADDR, 0x9f, 0x40);
            break;
        }
        default:
            return -1;
    }
    return 0;
}

int gc6133_power_down(void)
{
    hal_cam_write_reg(GC6133_I2C_ADDR, 0xf1, 0x00);
    return 0;
}

int gc6133_power_up(void)
{
    hal_cam_write_reg(GC6133_I2C_ADDR, 0xf1, 0x03);
    return 0;
}

CamCfg_t gc6133_default_cfg = {
    .drv_id = CSPI_1,
    .int_mode = GC6133_IMG_MODE_SDR1BIT_240x320_15FPS,
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

SensorFuncObj_t gc6133_sns_func_obj = {
    .type = CAM_TYPE_GC6133,
    .sensor_id = 0x00ba,
    .img_width = 240,
    .img_height = 320,
    .max_fps = 1500,  // actural fps * 100
    .dev_addr = GC6133_I2C_ADDR,
    .reg_addr_size = 1,
    .reg_data_size = 1,
    .default_cfg = &gc6133_default_cfg,
    .pfn_set_mirror_flip = gc6133_set_mirror_flip,
    .pfn_set_fps = gc6133_set_fps,
    .pfn_get_sensor_id = gc6133_get_sensor_id,
    .pfn_set_ev = gc6133_set_ev,
    .pfn_set_contrast = gc6133_set_contrast,
    .pfn_set_saturation = gc6133_set_saturation,
    .pfn_set_sharp = gc6133_set_sharpness,
    .pfn_set_awb = gc6133_set_awb,
    .pfn_set_gamma = gc6133_set_gamma,
    .pfn_set_ae = gc6133_set_ae,
    .pfn_set_scene = gc6133_set_scene,
    .pfn_get_init_reg_list = gc6133_get_reg_list,
    .pfn_power_down = gc6133_power_down,
    .pfn_power_up = gc6133_power_up,
};
