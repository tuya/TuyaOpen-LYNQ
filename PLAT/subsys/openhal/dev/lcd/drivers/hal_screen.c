#include "hal_screen.h"
#include "api_lspi.h"
#include <string.h>
#include "timer.h"
#include "api_pwm.h"
#include "api_gpio.h"
#include DEBUG_LOG_HEADER_FILE

#if defined(HAL_SCR_CO5300_BUILTIN)
extern LcdDrvObj_t co5300Drv;
#endif
#if defined(HAL_SCR_JD9850_BUILTIN)
extern LcdDrvObj_t jd9850Drv;
#endif
#if defined(HAL_SCR_ST7735_BUILTIN)
extern LcdDrvObj_t st7735Drv;
#endif
#if defined(HAL_SCR_ST7789_BUILTIN)
extern LcdDrvObj_t st7789Drv;
#endif
#if defined(HAL_SCR_JD9855_BUILTIN)
extern LcdDrvObj_t jd9855Drv;
#endif
#if defined(HAL_SCR_JD9853_BUILTIN)
extern LcdDrvObj_t jd9853Drv;
#endif
static LcdDrvObj_t* s_scr_list[] = {
#if defined(HAL_SCR_CO5300_BUILTIN)
    &co5300Drv,
#endif
#if defined(HAL_SCR_JD9850_BUILTIN)
    &jd9850Drv,
#endif
#if defined(HAL_SCR_ST7735_BUILTIN)
    &st7735Drv,
#endif
#if defined(HAL_SCR_ST7789_BUILTIN)
    &st7789Drv,
#endif
#if defined(HAL_SCR_JD9855_BUILTIN)
    &jd9855Drv,
#endif
#if defined(HAL_SCR_JD9853_BUILTIN)
    &jd9853Drv,
#endif
};

typedef struct HalScrCtx_
{
    bool init;
    ScreenType_e scr_type;
    int drv_id;
    int32_t lspi_usrid;
    LcdDrvObj_t* drv_obj;
    uint32_t bl_pwm_usr_id;
    uint32_t bl_gpio_usr_id;
    uint32_t ext_pwr_usr_id;
    int32_t dma_tx_ch;
    lspiCtrl_t lspi_ctrl;
    lspiCmdAddr_t lspi_cmd_addr;
    ScrUspEventCb usp_cb;
    DmaTransferConfig_t dma_tx_cfg;
    uint32_t prev_width;
    uint32_t prev_height;
    uint8_t backlight;
} HalScrCtx_t;

PLAT_FM_ZI DmaDescriptor_t __ALIGNED(16) lcdDmaTxDesc[DMA_DESC_MAX];

static HalScrCtx_t* get_hal_scr_ctx()
{
    static HalScrCtx_t* ctx = NULL;
    if(ctx == NULL)
    {
        ctx = (HalScrCtx_t*)malloc(sizeof(HalScrCtx_t));
        memset(ctx, 0, sizeof(HalScrCtx_t));
        ctx->scr_type = SCREEN_TYPE_UNKNOWN;
    }

    return ctx;
}

static LcdDrvObj_t* get_drv_obj(ScreenType_e type)
{
    int type_num = sizeof(s_scr_list) / sizeof(s_scr_list[0]);
    for(int i = 0; i < type_num; i++)
    {
        if(s_scr_list[i]->type == type)
        {
            return s_scr_list[i];
        }
    }
    return NULL;
}

static int hal_screen_init_back_light(ScrConfig_t* cfg)
{
    HalScrCtx_t* ctx = get_hal_scr_ctx();
    if(cfg->bk_light_mode == SCREEN_BK_LIGHT_MODE_PWM)
    {
        pwm_config_t pwm_cfg = {0};
        pwm_cfg.pin = cfg->pwm_cfg.pad;
        pwm_cfg.act = TIMER_PWM_STOP_LOW;
        pwm_cfg.freq = 1;  // 频率1KHz
        pwm_cfg.port = cfg->pwm_cfg.timer_port;
        pwm_cfg.duty = 0;
        int ret =
            api_pwm_create(cfg->pwm_cfg.pwm, &pwm_cfg, &ctx->bl_pwm_usr_id);
        if((ctx->bl_pwm_usr_id == 0) || (ret != OPEN_HAL_DONE))
        {
            ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_screen_init_back_light_failed0,
                          P_INFO, "can not create pwm %d, usr id %d, ret %d",
                          cfg->pwm_cfg.pwm, ctx->bl_pwm_usr_id, ret);
            return -1;
        }
        ret = api_pwm_open(ctx->bl_pwm_usr_id, NULL, 0);
        if(ret != OPEN_HAL_DONE)
        {
            ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_screen_init_back_light_failed1,
                          P_INFO, "can not open pwm %d, usr id %d, ret %d",
                          cfg->pwm_cfg.pwm, ctx->bl_pwm_usr_id, ret);
            return -1;
        }
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_screen_init_back_light_pwm, P_INFO,
                      "init backlight pwm %d, usr_id %d", cfg->pwm_cfg.pwm,
                      ctx->bl_pwm_usr_id);
    }
    else if(cfg->bk_light_mode == SCREEN_BK_LIGHT_MODE_GPIO)
    {
        GpioPinConfig_t pinConfig = {0};
        pinConfig.pinDirection = GPIO_DIRECTION_OUTPUT;
        pinConfig.misc.initOutput = 1;
        api_gpio_create(cfg->gpio_cfg.gpio_num, &pinConfig,
                        &ctx->bl_gpio_usr_id);
        api_gpio_open(ctx->bl_gpio_usr_id, &pinConfig, 0);
        uint8_t active = 0;
        api_gpio_ioctl(ctx->bl_gpio_usr_id, OPEN_GPIO_IOCTL_OUT_ACT, &active);
        uint16_t level = 1;
        api_gpio_write(ctx->bl_gpio_usr_id, &level, 1);
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_screen_init_back_gpio, P_INFO,
                      "init backlight gpio %d, usr_id %d",
                      cfg->gpio_cfg.gpio_num, ctx->bl_gpio_usr_id);
    }
    else
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_screen_init_back_reg, P_INFO,
                      "init backlight register");
    }
    return 0;
}

static int hal_screen_uninit_back_light(void)
{
    HalScrCtx_t* ctx = get_hal_scr_ctx();
    if(ctx->bl_pwm_usr_id != 0)
    {
        api_pwm_close(ctx->bl_pwm_usr_id);
        ctx->bl_pwm_usr_id = 0;
    }
    return 0;
}

int hal_screen_reset(uint32_t io_num, uint8_t reset_level,
                     uint32_t reset_period_in_us)
{
    GpioPinConfig_t pinConfig = {0};
    uint32_t id = 0;
    pinConfig.pinDirection = GPIO_DIRECTION_OUTPUT;
    pinConfig.misc.initOutput = 1;
    api_gpio_create(io_num, &pinConfig, &id);
    api_gpio_open(id, &pinConfig, 0);
    uint8_t active = 0;
    api_gpio_ioctl(id, OPEN_GPIO_IOCTL_OUT_ACT, &active);
    uint16_t level = reset_level == 0 ? 0 : 1;
    api_gpio_write(id, &level, 1);
    delay_us(reset_period_in_us);
    level = reset_level == 1 ? 0 : 1;
    api_gpio_write(id, &level, 1);
    delay_us(reset_period_in_us);
    return 0;
}

static int hal_screen_dma_init(ScrDmaEventCb dma_cb)
{
    HalScrCtx_t* ctx = get_hal_scr_ctx();
    if(ctx->dma_tx_ch != 0)
    {
        DMA_closeChannel(DMA_INSTANCE_MP, ctx->dma_tx_ch);
    }
    ctx->dma_tx_ch = DMA_openChannel(DMA_INSTANCE_MP);
    if(ARM_DMA_ERROR_CHANNEL_ALLOC == ctx->dma_tx_ch)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_screen_dma_init_failed0, P_INFO,
                      "can not alloc dma channel");
        return -1;
    }
    DMA_setChannelRequestSource(DMA_INSTANCE_MP, ctx->dma_tx_ch,
                                DMA_REQUEST_USP2_TX);
    DMA_rigisterChannelCallback(DMA_INSTANCE_MP, ctx->dma_tx_ch, dma_cb);
    DMA_enableChannelInterrupts(DMA_INSTANCE_MP, ctx->dma_tx_ch,
                                DMA_END_INTERRUPT_ENABLE);
    return 0;
}

static void hal_screen_usp_irq_cb(void)
{
    HalScrCtx_t* ctx = get_hal_scr_ctx();
    api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_CLEAR_WREND, NULL);
    if(ctx->usp_cb)
    {
        ctx->usp_cb();
    }
}

int hal_screen_set_mspi(bool enable, MspiDataLane_e addr_lane,
                        MspiDataLane_e data_lane, uint8_t inst)
{
    HalScrCtx_t* ctx = get_hal_scr_ctx();
    lspiMspiCtrl_t lspi_mspictrl = {0};
    lspi_mspictrl.mspiEn = enable;
    lspi_mspictrl.mspiAddrLane = (uint8_t)addr_lane;
    lspi_mspictrl.mspiDataLane = (uint8_t)data_lane;
    lspi_mspictrl.mspiInst = inst;
    api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_MSPI_CTRL, &lspi_mspictrl);
    return 0;
}

static int hal_screan_set_lspi_type(lcdInterfaceType_e type,
                                    uint8_t data_lane_num)
{
    HalScrCtx_t* ctx = get_hal_scr_ctx();
    lspi8080Ctrl_t lspi_8080ctrl = {0};
    memset(&ctx->lspi_cmd_addr, 0, sizeof(lspiCmdAddr_t));
    switch(type)
    {
        case INF_SPI_3W_I: {
            ctx->lspi_cmd_addr.busType = 0;
            ctx->lspi_ctrl.line4 = 0;
            break;
        }
        case INF_SPI_3W_II: {
            ctx->lspi_cmd_addr.busType = 1;
            ctx->lspi_ctrl.line4 = 0;
            break;
        }
        case INF_SPI_4W_I: {
            ctx->lspi_cmd_addr.busType = 0;
            ctx->lspi_ctrl.line4 = 1;
            break;
        }
        case INF_SPI_4W_II: {
            ctx->lspi_cmd_addr.busType = 1;
            ctx->lspi_ctrl.line4 = 1;
            break;
        }
        case INF_MSPI_4W_II: {
            hal_screen_set_mspi(true, MSPI_DATA_LANE_1, MSPI_DATA_LANE_1, 0x02);
            break;
        }
        case INF_8080: {
            // config cpol cpha
            //*(uint32_t *)0x4d042028 |= (1 << 4) | (1 << 5);
            lspi_8080ctrl.lspi8080En = 1;
            break;
        }
        default:
            ctx->lspi_cmd_addr.busType = 1;
            ctx->lspi_ctrl.line4 = 1;
    }
    ctx->lspi_ctrl.dspiEn = data_lane_num == 2 ? 1 : 0;
    ctx->lspi_ctrl.dspiCfg = data_lane_num == 2 ? 2 : 0;
    api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_CTRL, &ctx->lspi_ctrl);
    api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_CMD_ADDR,
                   &ctx->lspi_cmd_addr);
    api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_8080_CTRL, &lspi_8080ctrl);
    return 0;
}

int hal_screen_send_cmd(uint8_t cmd, uint8_t* data, uint32_t len)
{
    HalScrCtx_t* ctx = get_hal_scr_ctx();
    WriteParam_t param = {0};
    param.type = TYPE_CMD;
    param.cmd = cmd;
    param.data = data;
    param.len = len;
    return api_lspi_write(ctx->lspi_usrid, &param, sizeof(WriteParam_t));
}

static int hal_screen_init_reg_list(ScrRegList_t* reg_list, uint32_t list_len)
{
    int ret = 0;
    for(int i = 0; i < list_len; i++)
    {
        ScrRegList_t* reg = &reg_list[i];
        if(reg->cmd == 0xFF)
        {
            osDelay(reg->data[0]);
            continue;
        }
        ret = hal_screen_send_cmd(reg->cmd, reg->data, reg->len);
        if(ret != 0)
        {
            break;
        }
    }
    return ret;
}

static int hal_screen_ext_pwr_init(int io_num)
{
    HalScrCtx_t* ctx = get_hal_scr_ctx();
    if(io_num < 0)
    {
        return 0;
    }

    GpioPinConfig_t pinConfig = {0};
    pinConfig.pinDirection = GPIO_DIRECTION_OUTPUT;
    pinConfig.misc.initOutput = 0;
    api_gpio_create(io_num, &pinConfig, &ctx->ext_pwr_usr_id);
    if(ctx->ext_pwr_usr_id == 0)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_screen_ext_pwr_init_failed, P_INFO,
                      "create gpio ext pwr pin failed");
        return -1;
    }
    api_gpio_open(ctx->ext_pwr_usr_id, &pinConfig, 0);
    uint8_t active = 0;
    api_gpio_ioctl(ctx->ext_pwr_usr_id, OPEN_GPIO_IOCTL_OUT_ACT, &active);
    uint16_t level = 1;
    api_gpio_write(ctx->ext_pwr_usr_id, &level, 1);
    return 0;
}

int hal_screen_ext_pwr_set(bool on)
{
    HalScrCtx_t* ctx = get_hal_scr_ctx();
    if(ctx->ext_pwr_usr_id == 0)
    {
        return -1;
    }
    uint16_t level = on ? 1 : 0;
    api_gpio_write(ctx->ext_pwr_usr_id, &level, 1);
    return 0;
}

int hal_screen_init_lspi(uint32_t drv_id, ScrConfig_t* cfg,
                         ScrUspEventCb usp_cb)
{
    HalScrCtx_t* ctx = get_hal_scr_ctx();
    ctx->lspi_usrid = api_lspi_create(drv_id, NULL);
    if(ctx->lspi_usrid == 0)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_screen_init_lspi_failed0, P_INFO,
                      "can not create lspi %d", drv_id);
        return -1;
    }
    LspiConfig_t lspi_cfg = {0};
    lspi_cfg.freq = cfg->freq;
    api_lspi_open(ctx->lspi_usrid, &lspi_cfg, 0);
    ctx->drv_id = drv_id;
    memset(&ctx->lspi_ctrl, 0, sizeof(lspiCtrl_t));
    ctx->lspi_ctrl.enable = 1;
    ctx->lspi_ctrl.datSrc = 1;
    if(cfg->bpp == 12)
    {
        ctx->lspi_ctrl.colorModeIn = 3;
        ctx->lspi_ctrl.colorModeOut = 0;
    }
    else if(cfg->bpp == 16)
    {
        ctx->lspi_ctrl.colorModeIn = 3;
        ctx->lspi_ctrl.colorModeOut = 1;
    }
    else if(cfg->bpp == 18)
    {
        ctx->lspi_ctrl.colorModeIn = 3;
        ctx->lspi_ctrl.colorModeOut = 2;
    }
    else if(cfg->bpp == 24)
    {
        ctx->lspi_ctrl.colorModeIn = 4;
        ctx->lspi_ctrl.colorModeOut = 6;
    }
    else
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_screen_init_lspi_failed1, P_INFO,
                      "bpp %d not support", cfg->bpp);
        return -1;
    }
    api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_CTRL, &ctx->lspi_ctrl);

#if defined TYPE_EC718M
    lspiCmdPreParam0_t param0 = {0};
    api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_PRE_PARA0_CTRL, &param0);
    lspiCmdPreParam2_t param2 = {0};
    api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_PRE_PARA2_CTRL, &param2);
    lspiCmdPreParam3_t param3 = {0};
    api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_PRE_PARA3_CTRL, &param3);
    lspiCmdPostParam0_t post_param0 = {0};
    api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_POST_PARA0_CTRL,
                   &post_param0);
#endif
    lspiBusSel_t lspi_bus_sel = {0};
    lspi_bus_sel.lspiBusEn = 1;
    api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_BUS_SEL, &lspi_bus_sel);

    if(usp_cb)
    {
        lspiIntCtrl_t lspi_int_ctrl = {0};
        lspi_int_ctrl.lspiRamWrEndEn = 1;
        lspi_int_ctrl.txIntThreshHold = 8;
        lspi_int_ctrl.rxIntThreshHold = 8;
        ctx->usp_cb = usp_cb;
        api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_INT_CTRL,
                       &lspi_int_ctrl);
        XIC_SetVector(PXIC0_USP2_IRQn, hal_screen_usp_irq_cb);
        XIC_EnableIRQ(PXIC0_USP2_IRQn);
    }

    lspiDataFmt_t lspi_data_fmt = {0};
    lspi_data_fmt.wordSize = 31;
    api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_DATA_FORMAT,
                   &lspi_data_fmt);

    lspiInfo_t lspi_info = {0};
    lspi_info.frameHeight = cfg->height;
    lspi_info.frameWidth = cfg->width;
    api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_FRAME_INFO, &lspi_info);

    lspiFrameInfoOut_t lspi_frame_info_out = {0};
    lspi_frame_info_out.frameHeightOut = cfg->height;
    lspi_frame_info_out.frameWidthOut = cfg->width;
    api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_FRAME_INFO_OUT,
                   &lspi_frame_info_out);
    hal_screan_set_lspi_type(cfg->int_type, cfg->data_lane_num);
    return 0;
}

int hal_screen_open(uint32_t drv_id, ScrConfig_t* cfg, ScrDmaEventCb dma_cb,
                    ScrUspEventCb usp_cb)
{
    HalScrCtx_t* ctx = get_hal_scr_ctx();
    int ret = 0;
    if(ctx == NULL)
    {
        return -1;
    }
    ctx->drv_obj = get_drv_obj(cfg->type);
    if(ctx->drv_obj == NULL)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_screen_open_failed0, P_INFO,
                      "can not find screen type %d", cfg->type);
        return -1;
    }
    ctx->scr_type = cfg->type;
    ret = hal_screen_init_back_light(cfg);
    if(ret != 0)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_screen_open_failed1, P_INFO,
                      "can not init back light");
        return ret;
    }

    hal_screen_ext_pwr_init(cfg->ext_pwr_io);
    hal_screen_ext_pwr_set(true);
    hal_screen_reset(cfg->reset_io_num, cfg->reset_level, 200 * 1000);
    ret = hal_screen_dma_init(dma_cb);
    if(ret != 0)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_screen_open_failed2, P_INFO,
                      "can not init dma");
        return ret;
    }

    ret = hal_screen_init_lspi(drv_id, cfg, usp_cb);
    if(ret != 0)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_screen_open_failed4, P_INFO,
                      "can not init lspi");
        return ret;
    }

    ret = hal_screen_init_reg_list(ctx->drv_obj->init_reg_list,
                                   ctx->drv_obj->init_reg_list_len);
    if(ret != 0)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_screen_open_failed3, P_INFO,
                      "can not init reg list");
        return ret;
    }
    ctx->drv_obj->bpp = cfg->bpp;
    ctx->drv_obj->data_lane_num = cfg->data_lane_num;
    ctx->drv_obj->width = cfg->width;
    ctx->drv_obj->height = cfg->height;
    ctx->drv_obj->x_offset = cfg->x_offset;
    ctx->drv_obj->y_offset = cfg->y_offset;
    hal_screen_set_backlight(drv_id, cfg->init_backlight_level);
    ctx->drv_obj->set_display_mode(cfg->display_mode);
    ctx->drv_obj->set_display_pix_mode(cfg->pix_mode);
    return 0;
}

static void hal_screen_dma_start()
{
    HalScrCtx_t* ctx = get_hal_scr_ctx();
    DMA_loadChannelDescriptorAndRun(DMA_INSTANCE_MP, ctx->dma_tx_ch,
                                    lcdDmaTxDesc);
}

static void hal_screen_dma_stop()
{
    HalScrCtx_t* ctx = get_hal_scr_ctx();
    DMA_stopChannel(DMA_INSTANCE_MP, ctx->dma_tx_ch, false);
}

int hal_screen_close(uint32_t drv_id)
{
    HalScrCtx_t* ctx = get_hal_scr_ctx();
    hal_screen_dma_stop();
    api_lspi_close(ctx->lspi_usrid);
    hal_screen_uninit_back_light();
    hal_screen_ext_pwr_set(false);
    return 0;
}

int hal_screen_set_backlight(uint32_t drv_id, uint8_t level)
{
    uint8_t act_level = level;
    HalScrCtx_t* ctx = get_hal_scr_ctx();
    if(ctx->bl_pwm_usr_id != 0)
    {
        if(act_level == 0)
        {
            api_pwm_write(ctx->bl_pwm_usr_id, &act_level, sizeof(uint8_t));
            api_pwm_close(ctx->bl_pwm_usr_id);
        }
        else
        {
            api_pwm_open(ctx->bl_pwm_usr_id, NULL, 0);
            api_pwm_write(ctx->bl_pwm_usr_id, &act_level, sizeof(uint8_t));
        }
    }
    else if(ctx->bl_gpio_usr_id != 0)
    {
        uint16_t gpio_level = (act_level != 0) ? 1 : 0;
        api_gpio_write(ctx->bl_gpio_usr_id, &gpio_level, sizeof(uint16_t));
    }
    else if(ctx->drv_obj->set_backlight)
    {
        ctx->drv_obj->set_backlight(level);
    }
    ctx->backlight = level;
    return 0;
}

int hal_screen_get_backlight(uint32_t drv_id, uint8_t* level)
{
    HalScrCtx_t* ctx = get_hal_scr_ctx();
    if(ctx->bl_pwm_usr_id != 0)
    {
        api_pwm_read(ctx->bl_pwm_usr_id, level, sizeof(uint8_t));
    }
    else if(ctx->bl_gpio_usr_id != 0)
    {
        api_gpio_read(ctx->bl_gpio_usr_id, level, sizeof(uint8_t));
    }
    else if(ctx->drv_obj->set_backlight)
    {
        *level = ctx->backlight;
    }
    return 0;
}

int hal_screen_set_direction(uint32_t drv_id, DisDirection_e direction)
{
    return 0;
}

int hal_screen_rst_and_clear_fifo(uint32_t drv_id)
{
    HalScrCtx_t* ctx = get_hal_scr_ctx();
    if(ctx->lspi_usrid == 0)
    {
        return -1;
    }
    api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_RST_CLERA_FIFO, NULL);
    return 0;
}

int hal_screen_cam_preview(uint32_t drv_id, ScrPrevInfo_t* prev_info)
{
    HalScrCtx_t* ctx = get_hal_scr_ctx();

    if(prev_info->enable)
    {
        if((ctx->prev_width > prev_info->img_width) ||
           (ctx->prev_height > prev_info->img_height))
        {
            ECPLAT_PRINTF(
                UNILOG_OPEN_HAL, hal_screen_cam_preview_failed1, P_INFO,
                "LCD resolution(%dx%d) is bigger than that of image(%dx%d)",
                ctx->prev_width, ctx->prev_height, prev_info->img_width,
                prev_info->img_height);
            return -1;
        }

        lspiDataFmt_t lspi_data_fmt = {0};
        lspi_data_fmt.wordSize = 7;
        lspi_data_fmt.txPack = 0;
        api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_DATA_FORMAT,
                       &lspi_data_fmt);

        lspiDmaCtrl_t lspi_dma_ctrl = {0};
        lspi_dma_ctrl.txDmaReqEn = 0;
        lspi_dma_ctrl.dmaWorkWaitCycle = 31;
        lspi_dma_ctrl.rxDmaBurstSizeSub1 = 7;
        lspi_dma_ctrl.txDmaBurstSizeSub1 = 7;
        lspi_dma_ctrl.rxDmaThreadHold = 7;
        lspi_dma_ctrl.txDmaThreadHold = 7;
        api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_DMA_CTRL,
                       &lspi_dma_ctrl);

        lspiInfo_t lspi_info = {0};
        lspi_info.frameHeight = prev_info->img_height;
        lspi_info.frameWidth = prev_info->img_width;

        lspiFrameInfoOut_t lspi_frame_info_out = {0};
        lspi_frame_info_out.frameHeightOut = ctx->prev_height;
        lspi_frame_info_out.frameWidthOut = ctx->prev_width;

        uint8_t div = 0;
        uint16_t div_remain = 0;
        lspiTailorInfo0_t lspi_tailor_info0 = {0};
        lspiTailorInfo_t lspi_tailor_info = {0};
        lspiScaleInfo_t lspi_scale_info = {0};
        div = lspi_info.frameHeight / lspi_frame_info_out.frameHeightOut;
        div_remain = lspi_info.frameHeight % lspi_frame_info_out.frameHeightOut;

        lspi_tailor_info0.tailorTop = div_remain >> 1;
        lspi_tailor_info0.tailorBottom = div_remain >> 1;
        lspi_scale_info.colScaleFrac = (div == 1) ? 0 : (128 / div);

        div = lspi_info.frameWidth / lspi_frame_info_out.frameWidthOut;
        div_remain = lspi_info.frameWidth % lspi_frame_info_out.frameWidthOut;
        lspi_tailor_info.tailorLeft = div_remain >> 1;
        lspi_tailor_info.tailorRight = div_remain >> 1;
        lspi_scale_info.rowScaleFrac = (div == 1) ? 0 : (128 / div);

        api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_SCALE_INFO,
                       &lspi_scale_info);
        api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_TAILOR_INFO0,
                       &lspi_tailor_info0);
        api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_FRAME_INFO, &lspi_info);
        api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_FRAME_INFO_OUT,
                       &lspi_frame_info_out);
        api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_TAILOR_INFO1,
                       &lspi_tailor_info);

        ctx->lspi_ctrl.datSrc = 0;
        ctx->lspi_ctrl.colorModeIn = 0;
        ctx->lspi_ctrl.colorModeOut = 1;
        api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_CTRL, &ctx->lspi_ctrl);

        lspiCmdCtrl_t lspi_cmd_ctrl = {0};
        lspi_cmd_ctrl.wrRdn = 1;
        lspi_cmd_ctrl.ramWr = 1;
        lspi_cmd_ctrl.ramWrHaltMode = 1;
        lspi_cmd_ctrl.dataLen = 0x3FFFFF;
        api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_CMD_CTRL,
                       &lspi_cmd_ctrl);
    }
    else
    {
        lspiDmaCtrl_t lspi_dma_ctrl = {0};
        lspi_dma_ctrl.txDmaReqEn = 0;
        lspi_dma_ctrl.dmaWorkWaitCycle = 31;
        lspi_dma_ctrl.rxDmaBurstSizeSub1 = 7;
        lspi_dma_ctrl.txDmaBurstSizeSub1 = 7;
        lspi_dma_ctrl.rxDmaThreadHold = 7;
        lspi_dma_ctrl.txDmaThreadHold = 7;
        api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_DMA_CTRL,
                       &lspi_dma_ctrl);

        lspiDataFmt_t lspi_data_fmt = {0};
        lspi_data_fmt.wordSize = 31;
        lspi_data_fmt.txPack = 0;
        api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_DATA_FORMAT,
                       &lspi_data_fmt);

        lspiInfo_t lspi_info = {0};
        lspi_info.frameHeight = ctx->drv_obj->height;
        lspi_info.frameWidth = ctx->drv_obj->width;
        api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_FRAME_INFO, &lspi_info);

        lspiFrameInfoOut_t lspi_frame_info_out = {0};
        lspi_frame_info_out.frameHeightOut = ctx->drv_obj->height;
        lspi_frame_info_out.frameWidthOut = ctx->drv_obj->width;
        api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_FRAME_INFO_OUT,
                       &lspi_frame_info_out);

        ctx->lspi_ctrl.datSrc = 1;
        ctx->lspi_ctrl.colorModeIn = 3;
        ctx->lspi_ctrl.colorModeOut = 1;
        api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_CTRL, &ctx->lspi_ctrl);

        lspiScaleInfo_t lspi_scale_info = {0};
        api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_SCALE_INFO,
                       &lspi_scale_info);

        lspiTailorInfo0_t lspi_tailor_info0 = {0};
        api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_TAILOR_INFO0,
                       &lspi_tailor_info0);

        lspiTailorInfo_t lspi_tailor_info = {0};
        api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_TAILOR_INFO1,
                       &lspi_tailor_info);
        uint8_t data = 0x00;
        hal_screen_send_cmd(0xFF, &data, sizeof(uint8_t));
        api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_TAILOR_INFO1,
                       &lspi_tailor_info);
    }

    return 0;
}

int hal_screen_set_refresh_dir(uint32_t drv_id, DisDirection_e direction)
{
    HalScrCtx_t* ctx = get_hal_scr_ctx();
    if(ctx->drv_obj == NULL)
    {
        return -1;
    }
    return ctx->drv_obj->set_direction(direction);
}

int hal_screen_set_display_mode(uint32_t drv_id, DisplayMode_e mode)
{
    HalScrCtx_t* ctx = get_hal_scr_ctx();
    if(ctx->drv_obj == NULL)
    {
        return -1;
    }
    return ctx->drv_obj->set_display_mode(mode);
}

int hal_screen_set_display_pix_mode(uint32_t drv_id, DisplayPixMode_e mode)
{
    HalScrCtx_t* ctx = get_hal_scr_ctx();
    if(ctx->drv_obj == NULL)
    {
        return -1;
    }
    return ctx->drv_obj->set_display_pix_mode(mode);
}

int hal_screen_suspend(uint32_t drv_id)
{
    HalScrCtx_t* ctx = get_hal_scr_ctx();
    if(ctx->drv_obj == NULL)
    {
        return -1;
    }
    return ctx->drv_obj->suspend();
}

int hal_screen_resume(uint32_t drv_id)
{
    HalScrCtx_t* ctx = get_hal_scr_ctx();
    if(ctx->drv_obj == NULL)
    {
        return -1;
    }
    return ctx->drv_obj->resume();
}

uint32_t hal_screen_set_refresh_loc(uint32_t drv_id, uint32_t start_x,
                                    uint32_t start_y, uint32_t width,
                                    uint32_t height)
{
    HalScrCtx_t* ctx = get_hal_scr_ctx();
    if(ctx->drv_obj == NULL)
    {
        return -1;
    }
    ctx->prev_width = width;
    ctx->prev_height = height;
    return ctx->drv_obj->set_window(start_x, start_y, width, height);
}

int hal_screen_refresh(uint32_t drv_id, uint8_t* data, uint32_t size)
{
    uint32_t res = 0, patch = 0;
    int dmaChainCnt = 0, ret = 0;
    HalScrCtx_t* ctx = get_hal_scr_ctx();
    res = size;
    if(size % 4)
    {
        res += (4 - size % 4);
    }

    lspiDmaCtrl_t lspi_dma_ctrl = {0};
    lspi_dma_ctrl.txDmaReqEn = 1;
    lspi_dma_ctrl.dmaWorkWaitCycle = 31;
    lspi_dma_ctrl.rxDmaBurstSizeSub1 = 7;
    lspi_dma_ctrl.txDmaBurstSizeSub1 = 7;
    lspi_dma_ctrl.rxDmaThreadHold = 7;
    lspi_dma_ctrl.txDmaThreadHold = 7;
    api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_DMA_CTRL, &lspi_dma_ctrl);

    lspiCmdCtrl_t lspi_cmd_ctrl = {0};
    lspi_cmd_ctrl.wrRdn = 1;  // 1: wr   0: rd
    lspi_cmd_ctrl.ramWr = 1;  // start ramwr

#if(defined TYPE_EC718M)
    if(ctx->drv_obj->data_lane_num == 2)
    {
        lspi_cmd_ctrl.ramWrHaltMode = 0;
    }
    else
    {
        lspi_cmd_ctrl.ramWrHaltMode = 1;
    }
#endif

#if((defined CHIP_EC718) && !(defined TYPE_EC718M)) || (defined CHIP_EC716)
    lspi_cmd_ctrl.dataLen =
        (ctx->drv_obj->data_lane_num == 2) ? (res >> 1) : res;
#elif(defined TYPE_EC718M)
    if(ctx->drv_obj->data_lane_num == 2)
    {
        lspi_cmd_ctrl.dataLen = (res >> 1);
    }
    else
    {
        if(ctx->drv_obj->bpp == 24)
        {
            lspi_cmd_ctrl.dataLen = res / (ctx->drv_obj->bpp / 8 + 1);
        }
        else
        {
            lspi_cmd_ctrl.dataLen = res / (ctx->drv_obj->bpp / 8);
        }
    }
#endif
    lspi_cmd_ctrl.dataLen =
        lspi_cmd_ctrl.dataLen > 0x3ffff ? 0x3ffff : lspi_cmd_ctrl.dataLen;
    api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_CMD_CTRL, &lspi_cmd_ctrl);

    // step2: config DMA
    uint32_t* sourceAddress = (uint32_t*)data;
    uint32_t fifo_addr = 0;
    api_lspi_ioctl(ctx->lspi_usrid, OPEN_LSPI_IOCTL_GET_FIFO_ADDR, &fifo_addr);
    ctx->dma_tx_cfg.sourceAddress = (void*)data;
    ctx->dma_tx_cfg.targetAddress = (void*)fifo_addr;
    ctx->dma_tx_cfg.flowControl = DMA_FLOW_CONTROL_TARGET;
    ctx->dma_tx_cfg.addressIncrement = DMA_ADDRESS_INCREMENT_SOURCE;
    ctx->dma_tx_cfg.dataWidth = DMA_DATA_WIDTH_FOUR_BYTES;
    ctx->dma_tx_cfg.burstSize = DMA_BURST_32_BYTES;
    ctx->dma_tx_cfg.totalLength = DMA_BULK_NUM;
    patch = DMA_BULK_NUM - res % DMA_BULK_NUM;
    dmaChainCnt = (res + patch) / DMA_BULK_NUM;
    uint32_t package = DMA_BULK_NUM;
    if(patch % dmaChainCnt == 0)
    {
        package -= (patch / dmaChainCnt);
        ctx->dma_tx_cfg.totalLength = package;
        DMA_buildDescriptorChain(lcdDmaTxDesc, &ctx->dma_tx_cfg, dmaChainCnt,
                                 true, true, true);
        ret += dmaChainCnt;
        res -= dmaChainCnt * package;
    }
    else
    {
        dmaChainCnt -= 1;
        res -= dmaChainCnt * DMA_BULK_NUM;
        ctx->dma_tx_cfg.totalLength = res;
        DMA_buildDescriptorChain(lcdDmaTxDesc, &ctx->dma_tx_cfg, 1, false,
                                 false, false);
        ctx->dma_tx_cfg.sourceAddress =
            (void*)sourceAddress + ret * DMA_BULK_NUM + res;
        ctx->dma_tx_cfg.totalLength = DMA_BULK_NUM;
        DMA_buildDescriptorChain(lcdDmaTxDesc + 1, &ctx->dma_tx_cfg,
                                 dmaChainCnt, true, true, true);
        ret += dmaChainCnt;
    }
    hal_screen_dma_start();
    return ret;
}

ScrConfig_t* hal_scr_get_default_config(ScreenType_e type)
{
    int type_num = sizeof(s_scr_list) / sizeof(s_scr_list[0]);
    for(int i = 0; i < type_num; i++)
    {
        if(s_scr_list[i]->type == type)
        {
            return s_scr_list[i]->default_cfg;
        }
    }
    return NULL;
}

int hal_screen_draw_piont(uint32_t drv_id, uint32_t x, uint32_t y,
                          uint32_t color)
{
    HalScrCtx_t* ctx = get_hal_scr_ctx();
    if(ctx->lspi_usrid == 0)
    {
        return -1;
    }
    hal_screen_set_refresh_loc(drv_id, x, y, 1, 1);
    WriteParam_t param = {0};
    param.type = TYPE_DATA;
    param.data = (uint8_t*)&color;
    param.len = sizeof(uint32_t);
    return api_lspi_write(ctx->lspi_usrid, &param, sizeof(WriteParam_t));
}

#define HW_COUNTER *(volatile uint32_t*)(0x4f020344)
uint32_t millis(void) { return (HW_COUNTER / 38); }
