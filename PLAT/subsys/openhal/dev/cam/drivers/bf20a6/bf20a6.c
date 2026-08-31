#include "sctdef.h"
#include "bf20a6.h"
#include "hal_cam.h"

SensorFuncObj_t bf20a6_sns_func_obj;

uint32_t bf20a6_1sdr_reg_list[] = {
    /*System*/
    0xffff0032, 0xfffe0032, 0x00f20001, 0x001200a0, 0x00e10092, 0x00e30002,
    0x00e00000, 0x002a0098, 0x000e0047, 0x000f0060, 0x00100057, 0x00110060,
    0x00300061, 0x006200cd, 0x0063001a, 0x00640038, 0x00650052, 0x00660068,
    0x006700c2, 0x006800a7, 0x006900ab, 0x006a00ad, 0x006b00a9, 0x006c00c4,
    0x006d00c5, 0x006e0018, 0x00c00020, 0x00c10024, 0x00c20029, 0x00c30025,
    0x00c40028, 0x00c5002a, 0x00c60041, 0x00cd0034, 0x00ce0032, 0x00cf0035,
    0x00d0006c, 0x00d1006e, 0x00d200cb, 0x00e40073, 0x00e50022, 0x00e60024,
    0x00e70064, 0x00e800f2, 0x004a0000, 0x00000003, 0x001f0002, 0x00220002,
    0x000c0031, 0x00000000, 0x00600081, 0x00610081, 0x00a00008, 0x0001001a,
    0x0001001a, 0x0001001a, 0x00020015, 0x00020015, 0x00020015, 0x00130008,
    0x008a0096, 0x008b0006, 0x00870018, 0x00340048, 0x00350040, 0x00360040,
    0x00710044, 0x00720048, 0x007400a2, 0x007500a9, 0x00780012, 0x007900a0,
    0x007a0094, 0x007c0097, 0x00400030, 0x00410030, 0x00420028, 0x0043001f,
    0x0044001c, 0x00450016, 0x00460013, 0x00470010, 0x0048000D, 0x0049000C,
    0x004B000A, 0x004C000B, 0x004E0009, 0x004F0008, 0x00500008, 0x005f0029,
    0x00230033, 0x00a10010, 0x00a2000d, 0x00a30030, 0x00a40006, 0x00a50022,
    0x00a60056, 0x00a70018, 0x00a8001a, 0x00a90012, 0x00aa0012, 0x00ab0016,
    0x00ac00b1, 0x00ba0012, 0x00bb0012, 0x00ad0012, 0x00ae0056, 0x00af000a,
    0x003b0030, 0x003c0012, 0x003d0022, 0x003e003f, 0x003f0028, 0x00b800c3,
    0x00b900A3, 0x00390047, 0x00260013, 0x00270016, 0x00280014, 0x00290018,
    0x00ee000d, 0x00130005, 0x0024003C, 0x00810020, 0x00820040, 0x00830030,
    0x00840058, 0x00850030, 0x00920008, 0x008600A0, 0x008a004b, 0x009100ff,
    0x00940062, 0x009a0018, 0x00f0004e, 0x00510017, 0x00520003, 0x0053005F,
    0x00540047, 0x00550066, 0x0056000F, 0x007e0014, 0x00570036, 0x0058002A,
    0x005900AA, 0x005a00A8, 0x005b0043, 0x005c0010, 0x005d0000, 0x007d0036,
    0x005e0010, 0x00d60088, 0x00d50020, 0x00b00084, 0x00b50008, 0x00b100c8,
    0x00b200c0, 0x00b300d0, 0x00b400B0, 0x00320000, 0x00a00009, 0x000b0002,
    0x001200A0, 0x003a0001, 0x00e10093, 0x00e30012, 0x00150010, 0x00e2001B,
    0x00ca0000, 0x00160073, 0x008a004b,
};

uint32_t bf20a6_2sdr_reg_list[] = {
    0xffff03E8, 0xfffe0064, 0x00f20001, 0x001200a0, 0x00e10092, 0x00e30002,
    0x00e00000, 0x002a0098, 0x000e0047, 0x000f0060, 0x00100057, 0x00110060,
    0x00300061, 0x006200cd, 0x0063001a, 0x00640038, 0x00650052, 0x00660068,
    0x006700c2, 0x006800a7, 0x006900ab, 0x006a00ad, 0x006b00a9, 0x006c00c4,
    0x006d00c5, 0x006e0018, 0x00c00020, 0x00c10024, 0x00c20029, 0x00c30025,
    0x00c40028, 0x00c5002a, 0x00c60041, 0x00cd0034, 0x00ce0032, 0x00cf0035,
    0x00d0006c, 0x00d1006e, 0x00d200cb, 0x00e40073, 0x00e50022, 0x00e60024,
    0x00e70064, 0x00e800f2, 0x004a0000, 0x00000003, 0x001f0002, 0x00220002,
    0x000c0031, 0x00000000, 0x00600081, 0x00610081, 0x00a00008, 0x0001001a,
    0x0001001a, 0x0001001a, 0x00020015, 0x00020015, 0x00020015, 0x00130008,
    0x008a0096, 0x008b0006, 0x00870018, 0x00340048, 0x00350040, 0x00360040,
    0x00710044, 0x00720048, 0x007400a2, 0x007500a9, 0x00780012, 0x007900a0,
    0x007a0094, 0x007c0097, 0x00400030, 0x00410030, 0x00420028, 0x0043001f,
    0x0044001c, 0x00450016, 0x00460013, 0x00470010, 0x0048000D, 0x0049000C,
    0x004B000A, 0x004C000B, 0x004E0009, 0x004F0008, 0x00500008, 0x005f0029,
    0x00230033, 0x00a10010, 0x00a2000d, 0x00a30030, 0x00a40006, 0x00a50022,
    0x00a60056, 0x00a70018, 0x00a8001a, 0x00a90012, 0x00aa0012, 0x00ab0016,
    0x00ac00b1, 0x00ba0012, 0x00bb0012, 0x00ad0012, 0x00ae0056, 0x00af000a,
    0x003b0030, 0x003c0012, 0x003d0022, 0x003e003f, 0x003f0028, 0x00b800c3,
    0x00b900A3, 0x00390047, 0x00260013, 0x00270016, 0x00280014, 0x00290018,
    0x00ee000d, 0x00130005, 0x0024003C, 0x00810020, 0x00820040, 0x00830030,
    0x00840058, 0x00850030, 0x00920008, 0x008600A0, 0x008a004b, 0x009100ff,
    0x00940062, 0x009a0018, 0x00f0004e, 0x00510017, 0x00520003, 0x0053005F,
    0x00540047, 0x00550066, 0x0056000F, 0x007e0014, 0x00570036, 0x0058002A,
    0x005900AA, 0x005a00A8, 0x005b0043, 0x005c0010, 0x005d0000, 0x007d0036,
    0x005e0010, 0x00d60088, 0x00d50020, 0x00b00084, 0x00b50008, 0x00b100c8,
    0x00b200c0, 0x00b300d0, 0x00b400B0, 0x00320000, 0x00a00009, 0x000b0002,
    0x001200A0, 0x003a0002, 0x00e10092, 0x00e30012, 0x00150010, 0x00e2001B,
    0x00ca0000, 0x00160073, 0x008a004b,
};

uint32_t bf20a6_2sdr_mono_reg_list[] = {
    0xffff03E8, 0xfffe0064, 0x00f20001, 0x001200a0, 0x00e10092, 0x00e30002,
    0x00e00000, 0x002a0098, 0x000e0047, 0x000f0060, 0x00100057, 0x00110060,
    0x00300061, 0x006200cd, 0x0063001a, 0x00640038, 0x00650052, 0x00660068,
    0x006700c2, 0x006800a7, 0x006900ab, 0x006a00ad, 0x006b00a9, 0x006c00c4,
    0x006d00c5, 0x006e0018, 0x00c00020, 0x00c10024, 0x00c20029, 0x00c30025,
    0x00c40028, 0x00c5002a, 0x00c60041, 0x00cd0034, 0x00ce0032, 0x00cf0035,
    0x00d0006c, 0x00d1006e, 0x00d200cb, 0x00e40073, 0x00e50022, 0x00e60024,
    0x00e70064, 0x00e800f2, 0x004a0000, 0x00000003, 0x001f0002, 0x00220002,
    0x000c0031, 0x00000000, 0x00600081, 0x00610081, 0x00a00008, 0x0001001a,
    0x0001001a, 0x0001001a, 0x00020015, 0x00020015, 0x00020015, 0x00130008,
    0x008a0096, 0x008b0006, 0x00870018, 0x00340048, 0x00350040, 0x00360040,
    0x00710044, 0x00720048, 0x007400a2, 0x007500a9, 0x00780012, 0x007900a0,
    0x007a0094, 0x007c0097, 0x00400030, 0x00410030, 0x00420028, 0x0043001f,
    0x0044001c, 0x00450016, 0x00460013, 0x00470010, 0x0048000D, 0x0049000C,
    0x004B000A, 0x004C000B, 0x004E0009, 0x004F0008, 0x00500008, 0x005f0029,
    0x00230033, 0x00a10010, 0x00a2000d, 0x00a30030, 0x00a40006, 0x00a50022,
    0x00a60056, 0x00a70018, 0x00a8001a, 0x00a90012, 0x00aa0012, 0x00ab0016,
    0x00ac00b1, 0x00ba0012, 0x00bb0012, 0x00ad0012, 0x00ae0056, 0x00af000a,
    0x003b0030, 0x003c0012, 0x003d0022, 0x003e003f, 0x003f0028, 0x00b800c3,
    0x00b900A3, 0x00390047, 0x00260013, 0x00270016, 0x00280014, 0x00290018,
    0x00ee000d, 0x00130005, 0x0024003C, 0x00810020, 0x00820040, 0x00830030,
    0x00840058, 0x00850030, 0x00920008, 0x008600A0, 0x008a004b, 0x009100ff,
    0x00940062, 0x009a0018, 0x00f0004e, 0x00510017, 0x00520003, 0x0053005F,
    0x00540047, 0x00550066, 0x0056000F, 0x007e0014, 0x00570036, 0x0058002A,
    0x005900AA, 0x005a00A8, 0x005b0043, 0x005c0010, 0x005d0000, 0x007d0036,
    0x005e0010, 0x00d60088, 0x00d50020, 0x00b00084, 0x00b50008, 0x00b100c8,
    0x00b200c0, 0x00b300d0, 0x00b400B0, 0x00320000, 0x00a00009, 0x000b0002,
    0x001200A3, 0x003a0002, 0x00e10051, 0x00e30012, 0x00150010, 0x00e2001B,
    0x00ca0000, 0x00160073, 0x008a004b,
};

int bf20a6_get_reg_list(uint8_t mode, uint32_t **reg_list, uint32_t *count)
{
    switch(mode)
    {
        case BF20A6_IMG_MODE_SDR1BIT_640X480_10FPS: {
            *reg_list = bf20a6_1sdr_reg_list;
            *count = (sizeof(bf20a6_1sdr_reg_list) /
                      sizeof(bf20a6_1sdr_reg_list[0]));
            bf20a6_sns_func_obj.max_fps = 1000;
            break;
        }
        case BF20A6_IMG_MODE_SDR2BIT_640X480_15FPS: {
            *reg_list = bf20a6_2sdr_reg_list;
            *count = (sizeof(bf20a6_2sdr_reg_list) /
                      sizeof(bf20a6_2sdr_reg_list[0]));
            bf20a6_sns_func_obj.max_fps = 1500;
            break;
        }
        case BF20A6_IMG_MODE_SDR2BIT_640X480_30FPS_MONO: {
            *reg_list = bf20a6_2sdr_mono_reg_list;
            *count = (sizeof(bf20a6_2sdr_mono_reg_list) /
                      sizeof(bf20a6_2sdr_mono_reg_list[0]));
            bf20a6_sns_func_obj.max_fps = 3000;
            break;
        }
        default: {
            return -1;
        }
    }
    return 0;
}

int bf20a6_set_mirror_flip(uint8_t direct)
{
    uint8_t reg_value = 0;
    if(direct == 3)
    {
        reg_value = 0x0c;
    }
    else if(direct == 1)
    {
        reg_value = 0x04;
    }
    else if(direct == 2)
    {
        reg_value = 0x08;
    }
    return hal_cam_write_reg(BF20A6_I2C_ADDR, 0x4a, reg_value);
}

int bf20a6_set_fps(uint32_t fps)
{
    uint32_t vts = BF20A6_DEFAULT_VTS;
    uint32_t dummy = 0;
    if(fps >  bf20a6_sns_func_obj.max_fps)
    {
        fps =  bf20a6_sns_func_obj.max_fps;
    }
    if(fps < BF20A6_DEFAULT_MINUM_FPS)

    {
        fps = BF20A6_DEFAULT_MINUM_FPS;
    }
    vts = BF20A6_DEFAULT_VTS *  bf20a6_sns_func_obj.max_fps / fps;
    dummy = vts - BF20A6_DEFAULT_START_OFFSET_LINE_NUMS -
            BF20A6_DEFAULT_ALL_LINE_NUMS;
    hal_cam_write_reg(BF20A6_I2C_ADDR, 0x03, (uint8_t)((dummy & 0x0F00) >> 8));
    hal_cam_write_reg(BF20A6_I2C_ADDR, 0x2B, (uint8_t)(dummy & 0x00FF));
    return 0;
}

int bf20a6_get_sensor_id(uint32_t *id)
{
    uint8_t idH = 0;
    uint8_t idL = 0;
    hal_cam_read_reg(BF20A6_I2C_ADDR, 0xfc, &idH);
    hal_cam_read_reg(BF20A6_I2C_ADDR, 0xfd, &idL);
    *id = (((uint32_t)idH) << 8) + idL;
    return 0;
}

int bf20a6_set_ev(uint8_t ev)
{
    uint8_t evValue = BF20A6_DEFAULT_EV_VALUE;
    if(ev >= 50)
    {
        evValue =
            (BF20A6_MAX_EV_VALUE - BF20A6_DEFAULT_EV_VALUE) * (ev - 50) / 50 +
            BF20A6_DEFAULT_EV_VALUE;
    }
    else
    {
        evValue = BF20A6_MIN_EV_VALUE +
                  ((BF20A6_DEFAULT_EV_VALUE - BF20A6_MIN_EV_VALUE) * ev / 50);
    }
    hal_cam_write_reg(BF20A6_I2C_ADDR, 0x24, evValue);
    return 0;
}

int bf20a6_set_contrast(uint8_t contrast)
{
    uint8_t conValue = BF20A6_DEFAULT_CONTRAST_VALUE;
    if(contrast >= 50)
    {
        conValue = (BF20A6_MAX_CONTRAST_VALUE - BF20A6_DEFAULT_CONTRAST_VALUE) *
                       (contrast - 50) / 50 +
                   BF20A6_DEFAULT_CONTRAST_VALUE;
    }
    else
    {
        conValue =
            BF20A6_MIN_CONTRAST_VALUE +
            ((BF20A6_DEFAULT_CONTRAST_VALUE - BF20A6_MIN_CONTRAST_VALUE) *
             contrast / 50);
    }
    hal_cam_write_reg(BF20A6_I2C_ADDR, 0xd6, conValue);
    return 0;
}

int bf20a6_set_saturation(uint8_t sat)
{
    uint8_t satValue = BF20A6_DEFAULT_SATURATION_VALUE;
    if(sat >= 50)
    {
        satValue =
            (BF20A6_MAX_SATURATION_VALUE - BF20A6_DEFAULT_SATURATION_VALUE) *
                (sat - 50) / 50 +
            BF20A6_DEFAULT_SATURATION_VALUE;
    }
    else
    {
        satValue =
            BF20A6_MIN_SATURATION_VALUE +
            ((BF20A6_DEFAULT_SATURATION_VALUE - BF20A6_MIN_SATURATION_VALUE) *
             sat / 50);
    }
    hal_cam_write_reg(BF20A6_I2C_ADDR, 0xb1, satValue);
    hal_cam_write_reg(BF20A6_I2C_ADDR, 0xb2, satValue);
    return 0;
}

int bf20a6_set_sharpness(uint8_t sharp)
{
    uint8_t sharpValue = 0;
    if(sharp >= 50)
    {
        sharpValue =
            (sharp - 50) *
                (BF20A6_MAX_SHARPEN_VALUE - BF20A6_DEFAULT_SHARPEN_VALUE) / 50 +
            BF20A6_DEFAULT_SHARPEN_VALUE;
    }
    else
    {
        sharpValue =
            BF20A6_MIN_SHARPEN_VALUE +
            sharp * (BF20A6_DEFAULT_SHARPEN_VALUE - BF20A6_MIN_SHARPEN_VALUE) /
                50;
    }
    hal_cam_write_reg(BF20A6_I2C_ADDR, 0x78,
                      sharpValue + ((uint16_t)sharpValue << 4));
    return 0;
}

int bf20a6_set_awb(bool awbEnable, uint8_t scene)
{
    uint8_t rgain = 0;
    uint8_t ggain = 0;
    uint8_t bgain = 0;
    uint8_t reg_value = 0;
    hal_cam_read_reg(BF20A6_I2C_ADDR, 0xa0, &reg_value);
    switch(scene)
    {
        case CAM_WB_AUTO:
            reg_value |= 0x01;
            break;
        case CAM_WB_CLOUD:
            reg_value &= ~0x01;
            rgain = 0x35;
            ggain = 0x33;
            bgain = 0x08;
            break;
        case CAM_WB_DAYLIGHT:
            reg_value &= ~0x01;
            rgain = 0x40;
            ggain = 0x33;
            bgain = 0x08;
            break;
        case CAM_WB_INCANDESCENCE:
            reg_value &= ~0x01;
            rgain = 0x20;
            ggain = 0x33;
            bgain = 0x08;
            break;
        case CAM_WB_FLUORESCENT:
            reg_value &= ~0x01;
            rgain = 0x1E;
            ggain = 0x33;
            bgain = 0x0B;
            break;
        case CAM_WB_TUNGSTEN:
            reg_value &= ~0x01;
            rgain = 0x1E;
            ggain = 0x33;
            bgain = 0x10;
            break;
        default:
            return -1;
    }
    hal_cam_write_reg(BF20A6_I2C_ADDR, 0xa0, reg_value);
    hal_cam_write_reg(BF20A6_I2C_ADDR, 0x01, rgain);
    hal_cam_write_reg(BF20A6_I2C_ADDR, 0x23, ggain);
    hal_cam_write_reg(BF20A6_I2C_ADDR, 0x02, bgain);
    return 0;
}

int bf20a6_set_gamma(uint8_t *table, uint32_t size)
{
    uint8_t reg_addr = 0;
    uint8_t reg_value = 0;
    if(size < BF20A6_GAMMA_TABLE_SIZE)
    {
        return -1;
    }
    for(int i = 0; i < BF20A6_GAMMA_TABLE_SIZE; i++)
    {
        reg_addr = (i < 10) ? (0x40 + i) : (0x40 + i + 1);
        reg_value = *(table + i);
        hal_cam_write_reg(BF20A6_I2C_ADDR, reg_addr, reg_value);
    }
    return 0;
}

int bf20a6_set_ae(uint8_t aeMode)
{
    uint8_t reg_value = 0;
    hal_cam_read_reg(BF20A6_I2C_ADDR, 0x13, &reg_value);
    reg_value &= ~(0x04);
    reg_value = aeMode == 0 ? (reg_value + 0x04) : reg_value;
    hal_cam_write_reg(BF20A6_I2C_ADDR, 0x13, reg_value);
    return 0;
}

int bf20a6_set_scene(uint8_t scene)
{
    switch(scene)
    {
        case CAM_SCENE_DAY: {
            hal_cam_write_reg(BF20A6_I2C_ADDR, 0x86, 0xa0);
            hal_cam_write_reg(BF20A6_I2C_ADDR, 0xf0, 0x4e);
            break;
        }
        case CAM_SCENE_NIGHT: {
            hal_cam_write_reg(BF20A6_I2C_ADDR, 0x86, 0xff);
            hal_cam_write_reg(BF20A6_I2C_ADDR, 0xf0, 0x54);
            break;
        }
        default:
            return -1;
    }
    return 0;
}

int bf20a6_power_down(void)
{
    hal_cam_write_reg(BF20A6_I2C_ADDR, 0xe0, 0x01);
    return 0;
}

int bf20a6_power_up(void)
{
    hal_cam_write_reg(BF20A6_I2C_ADDR, 0xe0, 0x00);
    return 0;
}

CamCfg_t bf20a6_default_cfg = {
    .drv_id = CSPI_1,
    .int_mode = BF20A6_IMG_MODE_SDR2BIT_640X480_15FPS,
    .mclk_freq = CAM_24_M,
    .reso = CAM_8W_COLOR,
    .seq_cfg.clock_delay = 0,
    .img_out_wnd = false,
    .wnd_cfg = {.start_x = 0, .start_y = 0, .width = 0, .height = 0},
    .spi_cfg =
        {
            .endianMode = CAM_LSB_MODE,
            .wireNum = WIRE_2,
            .rxSeq = SEQ_0,
            .cpol = 0,
            .cpha = 0,
            .ddrMode = 0,
            .wordIdSeq = 0,
            .yOnly = 0,
            .rowScaleRatio = 1,
            .colScaleRatio = 1,
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

SensorFuncObj_t bf20a6_sns_func_obj = {
    .type = CAM_TYPE_BF20A6,
    .sensor_id = 0x20a6,
    .img_width = 640,
    .img_height = 480,
    .max_fps = 1500,  // actural fps * 100
    .dev_addr = BF20A6_I2C_ADDR,
    .reg_addr_size = 1,
    .reg_data_size = 1,
    .default_cfg = &bf20a6_default_cfg,
    .pfn_set_mirror_flip = bf20a6_set_mirror_flip,
    .pfn_set_fps = bf20a6_set_fps,
    .pfn_get_sensor_id = bf20a6_get_sensor_id,
    .pfn_set_ev = bf20a6_set_ev,
    .pfn_set_contrast = bf20a6_set_contrast,
    .pfn_set_saturation = bf20a6_set_saturation,
    .pfn_set_sharp = bf20a6_set_sharpness,
    .pfn_set_awb = bf20a6_set_awb,
    .pfn_set_gamma = bf20a6_set_gamma,
    .pfn_set_ae = bf20a6_set_ae,
    .pfn_set_scene = bf20a6_set_scene,
    .pfn_get_init_reg_list = bf20a6_get_reg_list,
    .pfn_power_down = bf20a6_power_down,
    .pfn_power_up = bf20a6_power_up,
};
