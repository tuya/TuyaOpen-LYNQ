#include "device_parser.h"
#include "stdio.h"
#include "stdlib.h"
#include "syslog.h"
#include "cJSON.h"
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#ifdef FEATURE_SUBSYS_OPENHAL_ENABLE
#ifdef FEATURE_HAL_SCREEN_ENABLE
#include "hal_screen.h"
#endif
#ifdef FEATURE_HAL_CAM_ENABLE
#include "hal_cam.h"
#endif
#include "api_tp.h"
#endif

#if defined (FEATURE_SUBSYS_STORAGE_ENABLE) && defined (FEATURE_SUBSYS_OPENHAL_ENABLE)
static int safe_get_int(const cJSON* obj, const char* key, int default_value)
{
    const cJSON* item = cJSON_GetObjectItem(obj, key);
    if(item && cJSON_IsNumber(item))
    {
        return item->valueint;
    }
    SYSLOG_WARNING("'%s' is not a valid integer value\r\n", key);
    return default_value;
}

static bool safe_get_bool(const cJSON* obj, const char* key, bool default_value)
{
    const cJSON* item = cJSON_GetObjectItem(obj, key);
    if(item && cJSON_IsBool(item))
    {
        return item->valueint != 0;
    }
    SYSLOG_WARNING("'%s' is not a valid boolean value\r\n", key);
    return default_value;
}
#endif

#ifdef FEATURE_HAL_SCREEN_ENABLE
void dump_lcd_config(ScrConfig_t* lcd_cfg)
{
    SYSLOG_INFO("lcd settings:\r\n");
    SYSLOG_INFO("type: %d, drv_id: %d, int_type: %d, bk_light_mode: %d\r\n",
                lcd_cfg->type, lcd_cfg->drv_id, lcd_cfg->int_type,
                lcd_cfg->bk_light_mode);
    SYSLOG_INFO(
        "gpio_cfg: gpio_num: %d, pwm_cfg: pad: %d, pwm: %d, timer_port: %d\r\n",
        lcd_cfg->gpio_cfg.gpio_num, lcd_cfg->pwm_cfg.pad, lcd_cfg->pwm_cfg.pwm,
        lcd_cfg->pwm_cfg.timer_port);
    SYSLOG_INFO(
        "init_backlight_level: %d, ext_pwr_io: %d, reset_io_num: %d, "
        "reset_level: %d\r\n",
        lcd_cfg->init_backlight_level, lcd_cfg->ext_pwr_io,
        lcd_cfg->reset_io_num, lcd_cfg->reset_level);
    SYSLOG_INFO(
        "freq: %d, bpp: %d, data_lane_num: %d, "
        "width: %d, height: %d, "
        "x_offset: %d, y_offset: %d, "
        "te_circle: %d, display_mode: %d, pix_mode: %d\r\n",
        lcd_cfg->freq, lcd_cfg->bpp, lcd_cfg->data_lane_num, lcd_cfg->width,
        lcd_cfg->height, lcd_cfg->x_offset, lcd_cfg->y_offset,
        lcd_cfg->te_circle, lcd_cfg->display_mode, lcd_cfg->pix_mode);
}

static int parse_lcd_config(const cJSON* root, ScrConfig_t* lcd_cfg)
{
    if(!root || !lcd_cfg)
    {
        SYSLOG_ERR("Error: Invalid parameters for parse_lcd_config\r\n");
        return -1;
    }

    const cJSON* lcd_json = cJSON_GetObjectItem(root, "lcd");
    if(!lcd_json)
    {
        SYSLOG_WARNING(
            "'lcd' section not found in JSON, using default values\r\n");
        memset(lcd_cfg, 0, sizeof(ScrConfig_t));
        return -1;
    }

    bool enable = safe_get_bool(lcd_json, "enabled", false);
    if(!enable)
    {
        SYSLOG_WARNING("'enabled' is false, using default values\r\n");
        memset(lcd_cfg, 0, sizeof(ScrConfig_t));
        return -1;
    }

    // 解析基本字段，每个字段都提供默认值
    lcd_cfg->type = (ScreenType_e)safe_get_int(lcd_json, "type", 1);
    lcd_cfg->drv_id = (ScrlspiInstance_e)safe_get_int(lcd_json, "drv_id", 0);
    lcd_cfg->int_type =
        (lcdInterfaceType_e)safe_get_int(lcd_json, "int_type", 3);
    lcd_cfg->bk_light_mode =
        (ScreenBkLightMode_e)safe_get_int(lcd_json, "bk_light_mode", 0);
    lcd_cfg->reset_io_num = safe_get_int(lcd_json, "reset_io_num", 19);
    lcd_cfg->reset_level = safe_get_int(lcd_json, "reset_level", 0);
    lcd_cfg->init_backlight_level =
        safe_get_int(lcd_json, "init_backlight_level", 0);
    lcd_cfg->freq = safe_get_int(lcd_json, "freq", 75497472);
    lcd_cfg->bpp = safe_get_int(lcd_json, "bpp", 16);
    lcd_cfg->data_lane_num = safe_get_int(lcd_json, "data_lane_num", 1);
    lcd_cfg->width = safe_get_int(lcd_json, "width", 240);
    lcd_cfg->height = safe_get_int(lcd_json, "height", 320);
    lcd_cfg->x_offset = safe_get_int(lcd_json, "offset_x", 0);
    lcd_cfg->y_offset = safe_get_int(lcd_json, "offset_y", 0);
    lcd_cfg->te_circle = safe_get_int(lcd_json, "te_circle", 16742);
    lcd_cfg->te_wait_time = safe_get_int(lcd_json, "te_wait_time", 623);
    lcd_cfg->ext_pwr_io = safe_get_int(lcd_json, "ext_pwr_io", -1);
    lcd_cfg->display_mode = safe_get_int(lcd_json, "display_mode", -1);
    lcd_cfg->pix_mode = safe_get_int(lcd_json, "pix_mode", -1);

    const cJSON* gpio_cfg = cJSON_GetObjectItem(lcd_json, "gpio_cfg");
    if(gpio_cfg && cJSON_IsObject(gpio_cfg))
    {
        lcd_cfg->gpio_cfg.gpio_num = safe_get_int(gpio_cfg, "gpio_num", 0);
    }
    else
    {
        lcd_cfg->gpio_cfg.gpio_num = 0;
        SYSLOG_WARNING("'gpio_cfg' section missing, using default\r\n");
    }

    // 解析pwm_cfg子对象（带保护）
    const cJSON* pwm_cfg = cJSON_GetObjectItem(lcd_json, "pwm_cfg");
    if(pwm_cfg && cJSON_IsObject(pwm_cfg))
    {
        lcd_cfg->pwm_cfg.pad = safe_get_int(pwm_cfg, "pad", 0);
        lcd_cfg->pwm_cfg.pwm = safe_get_int(pwm_cfg, "pwm", 0);
        lcd_cfg->pwm_cfg.timer_port = safe_get_int(pwm_cfg, "timer_port", 0);
    }
    else
    {
        memset(&lcd_cfg->pwm_cfg, 0, sizeof(ScrBkLightPwmCfg_t));
        SYSLOG_WARNING("'pwm_cfg' section missing, using default\r\n");
    }
    dump_lcd_config(lcd_cfg);
    return 0;
}
#endif

#ifdef FEATURE_HAL_CAM_ENABLE
static void parse_cspi_config(const cJSON* spi_json, CamSpiCfg_t* spi_cfg)
{
    if(!spi_json || !spi_cfg) return;

    if(!cJSON_IsObject(spi_json))
    {
        SYSLOG_WARNING("'spi_cfg' is not a valid object\r\n");
        return;
    }

    spi_cfg->endianMode = safe_get_int(spi_json, "endianMode", 1);
    spi_cfg->wireNum = safe_get_int(spi_json, "wireNum", 1);
    spi_cfg->rxSeq = safe_get_int(spi_json, "rxSeq", 1);
    spi_cfg->cpol = safe_get_int(spi_json, "cpol", 0);
    spi_cfg->cpha = safe_get_int(spi_json, "cpha", 1);
    spi_cfg->ddrMode = safe_get_bool(spi_json, "ddrMode", false) ? 1 : 0;
    spi_cfg->wordIdSeq = safe_get_int(spi_json, "wordIdSeq", 1);
    spi_cfg->yOnly = safe_get_bool(spi_json, "yOnly", false) ? 1 : 0;
    spi_cfg->rowScaleRatio = safe_get_int(spi_json, "rowScaleRatio", 1);
    spi_cfg->colScaleRatio = safe_get_int(spi_json, "colScaleRatio", 1);
    spi_cfg->scaleBytes = safe_get_int(spi_json, "scaleBytes", 3);
    spi_cfg->dummyAllowed =
        safe_get_bool(spi_json, "dummyAllowed", true) ? 1 : 0;
}

void dump_camera_config(CamCfg_t* cam_cfg)
{
    SYSLOG_INFO("camera settings:\r\n");
    SYSLOG_INFO(
        "drv_id: %d, type: %d, int_mode: %d, mclk_freq: %d, reso: %d\r\n",
        cam_cfg->drv_id, cam_cfg->type, cam_cfg->int_mode, cam_cfg->mclk_freq,
        cam_cfg->reso);
    SYSLOG_INFO("img_out_wnd(%d): %d, %d, %d, %d\r\n", cam_cfg->img_out_wnd,
                cam_cfg->wnd_cfg.start_x, cam_cfg->wnd_cfg.start_y,
                cam_cfg->wnd_cfg.width, cam_cfg->wnd_cfg.height);
    SYSLOG_INFO("spi_cfg: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n",
                cam_cfg->spi_cfg.endianMode, cam_cfg->spi_cfg.wireNum,
                cam_cfg->spi_cfg.rxSeq, cam_cfg->spi_cfg.cpol,
                cam_cfg->spi_cfg.cpha, cam_cfg->spi_cfg.ddrMode,
                cam_cfg->spi_cfg.wordIdSeq, cam_cfg->spi_cfg.yOnly,
                cam_cfg->spi_cfg.rowScaleRatio, cam_cfg->spi_cfg.colScaleRatio,
                cam_cfg->spi_cfg.scaleBytes, cam_cfg->spi_cfg.dummyAllowed);
    SYSLOG_INFO(
        "ext_pwr_cfg: io_num: %d, enable: %d, rst_pin_cfg: io_num: %d, "
        "reset_level: %d\r\n",
        cam_cfg->ext_pwr_cfg.io_num, cam_cfg->ext_pwr_cfg.enable,
        cam_cfg->rst_pin_cfg.io_num, cam_cfg->rst_pin_cfg.reset_level);
    SYSLOG_INFO("i2c_cfg: i2c_port: %d, speed: %d, addr: %d\r\n",
                cam_cfg->i2c_cfg.i2c_port, cam_cfg->i2c_cfg.speed,
                cam_cfg->i2c_cfg.dev_addr);
    SYSLOG_INFO("qbuf_cfg: item_count: %d, item_size: %d\r\n",
                cam_cfg->qbuf_cfg.item_count, cam_cfg->qbuf_cfg.item_size);
}

int parse_camera_config(const cJSON* root, CamCfg_t* cam_cfg)
{
    if(!root || !cam_cfg)
    {
        SYSLOG_ERR("Invalid parameters for parse_camera_config\r\n");
        return -1;
    }

    const cJSON* cam_json = cJSON_GetObjectItem(root, "camera");
    if(!cam_json)
    {
        SYSLOG_WARNING(
            "'camera' section not found in JSON, using default "
            "values\r\n");
        memset(cam_cfg, 0, sizeof(CamCfg_t));
        return -1;
    }

    bool enable = safe_get_bool(cam_json, "enabled", false);
    if(!enable)
    {
        SYSLOG_WARNING("'enable' is false, using default values\r\n");
        memset(cam_cfg, 0, sizeof(CamCfg_t));
        return -1;
    }

    // 解析基本字段
    cam_cfg->type = (CamType_e)safe_get_int(cam_json, "type", 3);
    cam_cfg->drv_id = (CamSpiCspiInstance_e)safe_get_int(cam_json, "drv_id", 1);
    cam_cfg->int_mode = safe_get_int(cam_json, "int_mode", 2);
    cam_cfg->mclk_freq = (camFrequence_e)safe_get_int(cam_json, "mclk_freq", 3);
    cam_cfg->reso = (camResolution_e)safe_get_int(cam_json, "reso", 1);
    if(cam_cfg->reso == 0)
    {
        cam_cfg->reso = CAM_8W_Y;
    }
    else if(cam_cfg->reso == 1)
    {
        cam_cfg->reso = CAM_8W_COLOR;
    }
    else if(cam_cfg->reso == 2)
    {
        cam_cfg->reso = CAM_30W_Y;
    }
    else
    {
        cam_cfg->reso = CAM_30W_COLOR;
    }
    cam_cfg->img_out_wnd = safe_get_bool(cam_json, "img_out_wnd", false);

    // 解析seq_cfg（带保护）
    const cJSON* seq_cfg = cJSON_GetObjectItem(cam_json, "seq_cfg");
    if(seq_cfg && cJSON_IsObject(seq_cfg))
    {
        cam_cfg->seq_cfg.clock_delay = safe_get_int(seq_cfg, "clock_delay", 2);
    }
    else
    {
        cam_cfg->seq_cfg.clock_delay = 2;
        SYSLOG_WARNING("'seq_cfg' section missing, using default\r\n");
    }

    // 解析wnd_cfg（带保护）
    const cJSON* wnd_cfg = cJSON_GetObjectItem(cam_json, "wnd_cfg");
    if(wnd_cfg && cJSON_IsObject(wnd_cfg))
    {
        cam_cfg->wnd_cfg.start_x = safe_get_int(wnd_cfg, "start_x", 0);
        cam_cfg->wnd_cfg.start_y = safe_get_int(wnd_cfg, "start_y", 0);
        cam_cfg->wnd_cfg.width = safe_get_int(wnd_cfg, "width", 0);
        cam_cfg->wnd_cfg.height = safe_get_int(wnd_cfg, "height", 0);
    }
    else
    {
        memset(&cam_cfg->wnd_cfg, 0, sizeof(CamWndCfg_t));
        SYSLOG_WARNING("'wnd_cfg' section missing, using default\r\n");
    }

    // 解析spi_cfg（带保护）
    const cJSON* spi_cfg = cJSON_GetObjectItem(cam_json, "spi_cfg");
    if(spi_cfg && cJSON_IsObject(spi_cfg))
    {
        parse_cspi_config(spi_cfg, &cam_cfg->spi_cfg);
    }
    else
    {
        memset(&cam_cfg->spi_cfg, 0, sizeof(CamSpiCfg_t));
        SYSLOG_WARNING("'spi_cfg' section missing, using default\r\n");
    }

    // 解析ext_pwr_cfg（带保护）
    const cJSON* ext_pwr = cJSON_GetObjectItem(cam_json, "ext_pwr_cfg");
    if(ext_pwr && cJSON_IsObject(ext_pwr))
    {
        cam_cfg->ext_pwr_cfg.enable =
            safe_get_bool(ext_pwr, "enabled", true) ? 1 : 0;
        cam_cfg->ext_pwr_cfg.io_num = safe_get_int(ext_pwr, "io_num", 16);
    }
    else
    {
        memset(&cam_cfg->ext_pwr_cfg, 0, sizeof(CamExtPwrCfg_t));
        SYSLOG_WARNING("'ext_pwr_cfg' section missing, using default\r\n");
    }

    // 解析rst_pin_cfg（带保护）
    const cJSON* rst_pin = cJSON_GetObjectItem(cam_json, "rst_pin_cfg");
    if(rst_pin && cJSON_IsObject(rst_pin))
    {
        cam_cfg->rst_pin_cfg.io_num = safe_get_int(rst_pin, "io_num", 20);
        cam_cfg->rst_pin_cfg.reset_level =
            safe_get_int(rst_pin, "reset_level", 1);
    }
    else
    {
        memset(&cam_cfg->rst_pin_cfg, 0, sizeof(CamRstPinCfg_t));
        SYSLOG_WARNING("'rst_pin_cfg' section missing, using default\r\n");
    }

    // 解析i2c_cfg（带保护）
    const cJSON* i2c_cfg = cJSON_GetObjectItem(cam_json, "i2c_cfg");
    if(i2c_cfg && cJSON_IsObject(i2c_cfg))
    {
        cam_cfg->i2c_cfg.i2c_port = safe_get_int(i2c_cfg, "i2c_port", 0);
        cam_cfg->i2c_cfg.speed =
            (I2cSpeed_e)safe_get_int(i2c_cfg, "i2c_speed", 1);
        cam_cfg->i2c_cfg.dev_addr = safe_get_int(i2c_cfg, "addr", 0x33);
    }
    else
    {
        memset(&cam_cfg->i2c_cfg, 0, sizeof(CamI2cCfg_t));
        SYSLOG_WARNING("'i2c_cfg' section missing, using default\r\n");
    }

    // 解析qbuf_cfg（带保护）
    const cJSON* qbuf = cJSON_GetObjectItem(cam_json, "qbuf_cfg");
    if(qbuf && cJSON_IsObject(qbuf))
    {
        cam_cfg->qbuf_cfg.item_count = safe_get_int(qbuf, "item_count", 3);
        cam_cfg->qbuf_cfg.item_size = safe_get_int(qbuf, "item_size", 153600);
        cam_cfg->qbuf_cfg.pool_addr = NULL;  // 需要外部分配
    }
    else
    {
        cam_cfg->qbuf_cfg.item_count = 3;
        cam_cfg->qbuf_cfg.item_size = 153600;
        cam_cfg->qbuf_cfg.pool_addr = NULL;
        SYSLOG_WARNING("'qbuf_cfg' section missing, using default\r\n");
    }
    dump_camera_config(cam_cfg);
    return 0;
}
#endif

#if FEATURE_SUBSYS_OPENHAL_ENABLE
void dump_tp_config(api_tp_infp tp_cfg)
{
    SYSLOG_INFO("tp settings:\r\n");
    SYSLOG_INFO("type: %d, i2cId: %d, GpioId: %d, fw_id: %d\r\n", tp_cfg->type,
                tp_cfg->i2cId, tp_cfg->GpioId, tp_cfg->fw_id);
}

int parse_tp_config(const cJSON* root, api_tp_infp tp_cfg)
{
    if(!root || !tp_cfg)
    {
        SYSLOG_ERR("Invalid parameters for parse_tp_config\r\n");
        return -1;
    }
    const cJSON* tp_json = cJSON_GetObjectItem(root, "tp");
    if(!tp_json)
    {
        SYSLOG_WARNING(
            "'tp' section not found in JSON, using default "
            "values\n");
        memset(tp_cfg, 0, sizeof(api_tp_inf));
        return -1;
    }
    bool enable = safe_get_bool(tp_json, "enabled", false);
    if(!enable)
    {
        SYSLOG_WARNING("'enabled' is false, using default values\r\n");
        memset(tp_cfg, 0, sizeof(api_tp_inf));
        return -1;
    }

    tp_cfg->type = (tpSupportList_e)safe_get_int(tp_json, "type", 0);
    tp_cfg->i2cId = safe_get_int(tp_json, "i2c_id", 0);
    tp_cfg->GpioId = safe_get_int(tp_json, "gpio_num", 36);
    tp_cfg->fw_id = safe_get_int(tp_json, "fw_id", 0);
    dump_tp_config(tp_cfg);
    return 0;
}
#endif

int device_parser_parse(const char* dev_path, void** cfg, uint32_t* size,
                        void* dev_cfg)
{
    int ret = -1;
#if defined (FEATURE_SUBSYS_STORAGE_ENABLE) && defined (FEATURE_SUBSYS_OPENHAL_ENABLE)
    FILE* fp = NULL;
    if(!dev_path || !cfg || !size)
    {
        SYSLOG_ERR("Invalid parameters for device_parser_parse\r\n");
        return -1;
    }

    // 提前检查设备路径是否合法
    if(strcasecmp("dev:/lcd", dev_path) != 0 &&
       strcasecmp("dev:/tp", dev_path) != 0 &&
       strcasecmp("dev:/camera", dev_path) != 0)
    {
        SYSLOG_ERR("Invalid device path %s\r\n", dev_path);
        return -1;
    }

    fp = file_fopen(DEV_CFG_PATH, "rb");
    if(!fp)
    {
        SYSLOG_ERR("Failed to open file %s, can not use configuration file\r\n",
                   dev_path);
        return -1;
    }
    struct stat file_stat = {0};
    file_fstat((int)fp, &file_stat);
    char* dev_json = malloc(file_stat.st_size + 1);
    if(!dev_json)
    {
        SYSLOG_ERR("Failed to allocate memory for device config\r\n");
        file_fclose(fp);
        return -1;
    }
    file_fread(dev_json, 1, file_stat.st_size, fp);
    file_fclose(fp);
    dev_json[file_stat.st_size] = '\0';  // 确保字符串结束

    cJSON* root = cJSON_Parse(dev_json);
    if(!root)
    {
        SYSLOG_ERR("Error parsing JSON: %s\r\n", cJSON_GetErrorPtr());
        free(dev_json);
        return -1;
    }
    if(strcasecmp("dev:/lcd", dev_path) == 0)
    {
#ifdef FEATURE_HAL_SCREEN_ENABLE
        ScrConfig_t* lcd_cfg = malloc(sizeof(ScrConfig_t));
        if(!lcd_cfg)
        {
            SYSLOG_ERR("Failed to allocate memory for lcd config\r\n");
            goto cleanup;
        }
        memset(lcd_cfg, 0, sizeof(ScrConfig_t));
        if(dev_cfg)
        {
            memcpy(lcd_cfg, dev_cfg, sizeof(ScrConfig_t));
        }
        if(0 == parse_lcd_config(root, lcd_cfg))
        {
            *cfg = lcd_cfg;
            *size = sizeof(ScrConfig_t);
            ret = 0;
        }
        else
        {
            SYSLOG_ERR("Failed to parse lcd config file %s\r\n", dev_path);
            free(lcd_cfg);
        }
#endif
    }
    else if(strcasecmp("dev:/tp", dev_path) == 0)
    {
        api_tp_infp tp_cfg = malloc(sizeof(api_tp_inf));
        if(!tp_cfg)
        {
            SYSLOG_ERR("Failed to allocate memory for tp config\r\n");
            goto cleanup;
        }
        memset(tp_cfg, 0, sizeof(api_tp_inf));
        if(dev_cfg)
        {
            memcpy(tp_cfg, dev_cfg, sizeof(api_tp_inf));
        }
        if(0 == parse_tp_config(root, tp_cfg))
        {
            *cfg = tp_cfg;
            *size = sizeof(api_tp_inf);
            ret = 0;
        }
        else
        {
            SYSLOG_ERR("Failed to parse tp config file %s\r\n", dev_path);
            free(tp_cfg);
        }
    }
    else if(strcasecmp("dev:/camera", dev_path) == 0)
    {
#ifdef FEATURE_HAL_CAM_ENABLE
        CamCfg_t* cam_cfg = malloc(sizeof(CamCfg_t));
        if(!cam_cfg)
        {
            SYSLOG_ERR("Failed to allocate memory for cam config\r\n");
            free(dev_json);
            return -1;
        }
        memset(cam_cfg, 0, sizeof(CamCfg_t));
        if(dev_cfg)
        {
            memcpy(cam_cfg, dev_cfg, sizeof(CamCfg_t));
        }
        if(0 == parse_camera_config(root, cam_cfg))
        {
            *cfg = cam_cfg;
            *size = sizeof(CamCfg_t);
            ret = 0;
        }
        else
        {
            SYSLOG_ERR("Failed to parse camera config file %s\r\n", dev_path);
            free(cam_cfg);
        }
#endif
    }
cleanup:

    if(dev_json)
    {
        free(dev_json);
    }
    if(root)
    {
        cJSON_Delete(root);
    }
#endif
    return ret;
}

int device_parser_free(void* cfg)
{
    if(cfg)
    {
        free(cfg);
    }
    return 0;
}
