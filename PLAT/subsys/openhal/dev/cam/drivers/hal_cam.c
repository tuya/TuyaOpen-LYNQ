#include "hal_cam.h"
#include "hal_i2c.h"
#include "pad.h"
#include "sctdef.h"
#include "api_pad.h"
#include "api_gpio.h"
#include "api_i2c.h"
#include "api_cspi.h"
#include "rbuffer.h"

#include DEBUG_LOG_HEADER_FILE

#if defined(HAL_CAM_BF20A6_BUILTIN)
extern SensorFuncObj_t bf20a6_sns_func_obj;
extern CamCfg_t bf20a6_default_cfg;
#endif
#if defined(HAL_CAM_BF30A2_BUILTIN)
extern SensorFuncObj_t bf30a2_sns_func_obj;
extern CamCfg_t bf30a2_default_cfg;
#endif
#if defined(HAL_CAM_GC032A_BUILTIN)
extern SensorFuncObj_t gc032a_sns_func_obj;
extern CamCfg_t gc032a_default_cfg;
#endif
#if defined(HAL_CAM_GC6133_BUILTIN)
extern SensorFuncObj_t gc6133_sns_func_obj;
extern CamCfg_t gc6133_default_cfg;
#endif
#if defined(HAL_CAM_GC6153_BUILTIN)
extern SensorFuncObj_t gc6153_sns_func_obj;
extern CamCfg_t gc6153_default_cfg;
#endif

static SensorFuncObj_t *s_cam_list[] = {
#if defined(HAL_CAM_BF20A6_BUILTIN)
    &bf20a6_sns_func_obj,
#endif
#if defined(HAL_CAM_BF30A2_BUILTIN)
    &bf30a2_sns_func_obj,
#endif
#if defined(HAL_CAM_GC032A_BUILTIN)
    &gc032a_sns_func_obj,
#endif
#if defined(HAL_CAM_GC6133_BUILTIN)
    &gc6133_sns_func_obj,
#endif
#if defined(HAL_CAM_GC6153_BUILTIN)
    &gc6153_sns_func_obj,
#endif
};

#define EIGEN_CSPI(n) ((CSPI_TypeDef *)(MP_USP0_BASE_ADDR + 0x1000 * n))

typedef struct HalCamCtx_
{
    bool init;
    int drv_id;
    int32_t cspi_usrid;
    CamType_e cam_type;
    SensorFuncObj_t *cam_obj;
    uint32_t ext_pwr_pad_id;
    uint32_t ext_pwr_pin_id;
    uint32_t rst_pad_id;
    uint32_t rst_pin_id;
    uint32_t i2c_port_id;
    uint8_t reset_level;
    CspiCfg_t cspi_cfg;
    CamSpiCbEventFunc cb_event;
    void *cb_event_param;
    CamDataIrqFunc cb_data_irq;
    void *cb_data_irq_param;
    CamSpiErrFunc cb_err;
    void *cb_err_param;
    I2cSpeed_e i2c_speed;
    RBuffer_t *req_buf_pool;
    uint32_t frame_size;
    uint16_t img_width;
    uint16_t img_height;
    CamImgFmt_e img_fmt;
} HalCamCtx_t;

static CspiCfg_t s_cspi_default_cfg[2] = {
    {
        .pins =
            {
                .mclk = {.pinNum = 39, .funcNum = PAD_MUX_ALT1},
                .pclk = {.pinNum = 35, .funcNum = PAD_MUX_ALT1},
                .cs = {.pinNum = 36, .funcNum = PAD_MUX_ALT1},
                .sdo0 = {.pinNum = 37, .funcNum = PAD_MUX_ALT1},
                .sdo1 = {.pinNum = 38, .funcNum = PAD_MUX_ALT1},
            },
        .cbEvent = NULL,
    },
    {
        .pins =
            {
                .mclk = {.pinNum = 18, .funcNum = PAD_MUX_ALT1},
                .pclk = {.pinNum = 19, .funcNum = PAD_MUX_ALT1},
                .cs = {.pinNum = 20, .funcNum = PAD_MUX_ALT1},
                .sdo0 = {.pinNum = 21, .funcNum = PAD_MUX_ALT1},
                .sdo1 = {.pinNum = 22, .funcNum = PAD_MUX_ALT1},
            },
        .cbEvent = NULL,
    }};

extern void delay_us(uint32_t us);

static HalCamCtx_t *GetHalCamCtx()
{
    static HalCamCtx_t *ctx = NULL;
    if(ctx == NULL)
    {
        ctx = (HalCamCtx_t *)malloc(sizeof(HalCamCtx_t));
        memset(ctx, 0, sizeof(HalCamCtx_t));
        ctx->cam_type = CAM_TYPE_UNKNWON;
    }

    return ctx;
}

int hal_cam_init_i2c(CamI2cCfg_t *cfg)
{
    HalCamCtx_t *ctx = GetHalCamCtx();
    api_i2c_create(cfg->i2c_port, NULL, &ctx->i2c_port_id);
    if(ctx->i2c_port_id == 0)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_cam_init_i2c_failed0, P_INFO,
                      "camera i2c port %d init failed!", cfg->i2c_port);
        return -1;
    }
    api_i2c_open(ctx->i2c_port_id, NULL, 1000);
    ctx->i2c_speed = cfg->speed;
    api_i2c_ioctl(ctx->i2c_port_id, OPEN_I2C_IOCTL_SPEED, &ctx->i2c_speed);
    return 0;
}

int hal_cam_uninit_i2c()
{
    HalCamCtx_t *ctx = GetHalCamCtx();
    // 暂不释放I2C总线，避免影响其他模块使用
    // api_i2c_close(ctx->i2c_port_id);
    // api_i2c_delete(ctx->i2c_port_id);
    ctx->i2c_port_id = 0;
    return 0;
}

CamType_e hal_cam_get_type()
{
    HalCamCtx_t *ctx = GetHalCamCtx();
    if(ctx->cam_type != CAM_TYPE_UNKNWON)
    {
        return ctx->cam_type;
    }
    int type_num = sizeof(s_cam_list) / sizeof(s_cam_list[0]);
    for(int i = 0; i < type_num; i++)
    {
        uint32_t get_id = 0;
        s_cam_list[i]->pfn_get_sensor_id(&get_id);
        if(get_id == s_cam_list[i]->sensor_id)
        {
            ctx->cam_type = s_cam_list[i]->type;
            ctx->cam_obj = s_cam_list[i];
            return s_cam_list[i]->type;
        }
    }
    return CAM_TYPE_UNKNWON;
}

int hal_cam_set_type(CamType_e type)
{
    HalCamCtx_t *ctx = GetHalCamCtx();
    int type_num = sizeof(s_cam_list) / sizeof(s_cam_list[0]);
    for(int i = 0; i < type_num; i++)
    {
        if(s_cam_list[i]->type == type)
        {
            ctx->cam_type = s_cam_list[i]->type;
            ctx->cam_obj = s_cam_list[i];
            return 0;
        }
    }
    return -1;
}

SensorFuncObj_t *hal_cam_get_func_obj(void)
{
    HalCamCtx_t *ctx = GetHalCamCtx();
    if(ctx->cam_obj == NULL)
    {
        hal_cam_get_type();
    }
    return ctx->cam_obj;
}

static int hal_cam_reset_pin_init(CamRstPinCfg_t *cfg)
{
    HalCamCtx_t *ctx = GetHalCamCtx();
    GpioPinConfig_t pinConfig = {0};
    pinConfig.pinDirection = GPIO_DIRECTION_OUTPUT;
    pinConfig.misc.initOutput = 0;
    api_gpio_create(cfg->io_num, &pinConfig, &ctx->rst_pin_id);
    if(ctx->rst_pin_id == 0)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_cam_reset_pin_init_failed, P_INFO,
                      "create gpio reset pin failed");
        return -1;
    }
    api_gpio_open(ctx->rst_pin_id, &pinConfig, 0);
    uint8_t active = 0;
    api_gpio_ioctl(ctx->rst_pin_id, OPEN_GPIO_IOCTL_OUT_ACT, &active);
    uint8_t level = cfg->reset_level == 1 ? 0 : 1;
    ctx->reset_level = cfg->reset_level;
    api_gpio_write(ctx->rst_pin_id, &level, 1);
    return 0;
}

static int hal_cam_reset(uint32_t reset_period_in_us)
{
    HalCamCtx_t *ctx = GetHalCamCtx();
    if(ctx->rst_pin_id == 0)
    {
        return -1;
    }
    uint16_t level = ctx->reset_level;
    api_gpio_write(ctx->rst_pin_id, &level, 1);
    delay_us(reset_period_in_us);
    level = ctx->reset_level == 1 ? 0 : 1;
    api_gpio_write(ctx->rst_pin_id, &level, 1);
    return 0;
}

int hal_cam_write_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t reg_data)
{
    uint8_t data[2];
    HalCamCtx_t *ctx = GetHalCamCtx();

    if(ctx->i2c_port_id == 0)
    {
        return -1;
    }
    data[0] = reg_addr;
    data[1] = reg_data;
    api_i2c_open(ctx->i2c_port_id, NULL, 1000);
    api_i2c_ioctl(ctx->i2c_port_id, OPEN_I2C_IOCTL_SPEED, &ctx->i2c_speed);
    api_i2c_ioctl(ctx->i2c_port_id, OPEN_I2C_IOCTL_SLAVE_ADDR, &dev_addr);
    api_i2c_write(ctx->i2c_port_id, &data[0], 2 * sizeof(uint8_t), false);
    return 0;
}

int hal_cam_read_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data)
{
    HalCamCtx_t *ctx = GetHalCamCtx();
    if(ctx->i2c_port_id == 0)
    {
        return -1;
    }
    api_i2c_open(ctx->i2c_port_id, NULL, 1000);
    api_i2c_ioctl(ctx->i2c_port_id, OPEN_I2C_IOCTL_SPEED, &ctx->i2c_speed);
    api_i2c_ioctl(ctx->i2c_port_id, OPEN_I2C_IOCTL_SLAVE_ADDR, &dev_addr);
    api_i2c_write(ctx->i2c_port_id, &reg_addr, sizeof(uint8_t), true);
    api_i2c_read(ctx->i2c_port_id, reg_data, sizeof(uint8_t), false);
    return 0;
}

int hal_cam_init_reglist(uint8_t mode)
{
    SensorFuncObj_t *cam_func_obj = hal_cam_get_func_obj();
    uint32_t *reglist = NULL;
    uint32_t reglist_size = 0;
    int ret = 0;
    if(!cam_func_obj)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_cam_init_reglist_failed0, P_INFO,
                      "can not get camera type");
        return -1;
    }

    if(!cam_func_obj->pfn_get_init_reg_list)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_cam_init_reglist_failed1, P_INFO,
                      "pfn_get_init_reg_list not defined");
        return -1;
    }
    cam_func_obj->pfn_get_init_reg_list(mode, &reglist, &reglist_size);
    if(!reglist || reglist_size == 0)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_cam_init_reglist_failed2, P_INFO,
                      "unsupported camera interface mode");

        return -1;
    }
    for(int i = 0; i < reglist_size; i++)
    {
        uint16_t reg_addr = (reglist[i] & 0xFFFF0000) >> 16;
        uint16_t reg_data = (reglist[i] & 0xFFFF);
        if(reg_addr == 0xFFFF)
        {
            hal_cam_reset(reg_data);
            continue;
        }
        if(reg_addr == 0xFFFE)
        {
            osDelay(reg_data);
            continue;
        }
        ret |= hal_cam_write_reg(cam_func_obj->dev_addr, reg_addr, reg_data);
    }
    delay_us(100000);
    return ret;
}

int hal_cam_cspi_config(int drv_id, CamSpiCfg_t *cfg, uint8_t reso,
                        CamSeqCfg_t *seq_cfg)
{
    cspiCtrl_t cspi_ctrl = {0};
    cspiBinaryCtrl_t cspi_binary_ctrl = {0};
    cspiDataFmt_t cspi_data_fmt = {0};
    HalCamCtx_t *ctx = GetHalCamCtx();
    if(ctx->cspi_usrid == 0)
    {
        return -1;
    }

    cspi_ctrl.rxWid = cfg->wireNum;
    cspi_ctrl.rxdSeq = cfg->rxSeq;
    cspi_ctrl.cpol = cfg->cpol;
    cspi_ctrl.cpha = cfg->cpha;
    cspi_ctrl.frameProcEn = 1;
    cspi_ctrl.ddrMode = cfg->ddrMode;
    cspi_ctrl.fillYonly = cfg->yOnly;
    cspi_ctrl.lsCheckEn = 1;
    cspi_ctrl.dpCheckEn = 1;
    cspi_ctrl.hwInitEn = 1;
    cspi_ctrl.rowScaleRatio = cfg->rowScaleRatio;
    cspi_ctrl.colScaleRatio = cfg->colScaleRatio;
    cspi_ctrl.scaleBytes = cfg->scaleBytes;
    cspi_ctrl.otsuCalYAdjEn = 1;
    api_cspi_ioctl(ctx->cspi_usrid, OPEN_CSPI_IOCTL_SET_CSPI_CFG, &cspi_ctrl);

    cspi_data_fmt.endianMode = cfg->endianMode;
    cspi_data_fmt.slaveModeEn = 1;
    cspi_data_fmt.slotSize = 7;
    cspi_data_fmt.wordSize = 7;
    cspi_data_fmt.rxPack = 2;
    api_cspi_ioctl(ctx->cspi_usrid, OPEN_CSPI_IOCTL_SET_DATA_FORMAT,
                   &cspi_data_fmt);

    cspiTimeOutCycle_t cspi_timeout_cycle = {0};
    cspi_timeout_cycle.rxTimeOutCycle = 20;
    cspi_timeout_cycle.dummyCycle = 15;
    api_cspi_ioctl(ctx->cspi_usrid, OPEN_CSPI_IOCTL_SET_RECV_TIMEOUT,
                   &cspi_timeout_cycle);

    cspiFrameInfo0_t cspi_frame_info0 = {0};
    cspi_frame_info0.cspiBusTimeOutCycle = 0x2000;
    api_cspi_ioctl(ctx->cspi_usrid, OPEN_CSPI_IOCTL_SET_FRAME_INFO0,
                   &cspi_frame_info0);

    cspiIntCtrl_t cspi_int_ctrl = {0};
    /*使能帧尾中断*/
    cspi_int_ctrl.frameEndIntEn = 1;
    api_cspi_ioctl(ctx->cspi_usrid, OPEN_CSPI_IOCTL_SET_INTTERUPT,
                   &cspi_int_ctrl);

    api_cspi_ioctl(ctx->cspi_usrid, OPEN_CSPI_IOCTL_SET_CSPI_CFG, &cspi_ctrl);

    cspiDmaCtrl_t cspi_dma_ctrl = {0};
    cspi_dma_ctrl.rxDmaReqEn = 1;
    cspi_dma_ctrl.dmaWorkWaitCycle = 31;
    cspi_dma_ctrl.rxDmaBurstSizeSub1 = 7;
    cspi_dma_ctrl.txDmaBurstSizeSub1 = 7;
    cspi_dma_ctrl.rxDmaThreadHold = 7;
    cspi_dma_ctrl.txDmaThreadHold = 8;
    api_cspi_ioctl(ctx->cspi_usrid, OPEN_CSPI_IOCTL_SET_DMA_CTRL,
                   &cspi_dma_ctrl);

    uint8_t set_reso = reso;
    api_cspi_ioctl(ctx->cspi_usrid, OPEN_CSPI_IOCTL_SET_RESOLUTION, &set_reso);

    cspi_binary_ctrl.wordIdSeq = cfg->wordIdSeq;
    cspi_binary_ctrl.dummyAllowed = cfg->dummyAllowed;
    cspi_binary_ctrl.outSwCtrl = 2;
    api_cspi_ioctl(ctx->cspi_usrid, OPEN_CSPI_IOCTL_SET_BINARY,
                   &cspi_binary_ctrl);

    cspiAutoCgCtrl_t cspi_auto_cg = {0};
    cspi_auto_cg.autoCgEn = 1;
    api_cspi_ioctl(ctx->cspi_usrid, OPEN_CSPI_IOCTL_AUTO_CG_CTRL,
                   &cspi_auto_cg);

    cspiFrameProcLspi_t cspi_frame_proc_lspi = {0};
    cspi_frame_proc_lspi.outEnLspi = 1;
    api_cspi_ioctl(ctx->cspi_usrid, OPEN_CSPI_IOCTL_SET_FRAME_PROC,
                   &cspi_frame_proc_lspi);

    cspiDelayCtrl_t cspi_delay_ctrl = {0};
    cspi_delay_ctrl.clkDelay = seq_cfg->clock_delay;
    api_cspi_ioctl(ctx->cspi_usrid, OPEN_CSPI_IOCTL_SET_DELAY_CTRL,
                   &cspi_delay_ctrl);
    return 0;
}

static int hal_cam_ext_pwr_init(CamExtPwrCfg_t *cfg)
{
    if(!cfg->enable)
    {
        return 0;
    }
    HalCamCtx_t *ctx = GetHalCamCtx();
    GpioPinConfig_t pinConfig = {0};
    pinConfig.pinDirection = GPIO_DIRECTION_OUTPUT;
    pinConfig.misc.initOutput = 0;
    api_gpio_create(cfg->io_num, &pinConfig, &ctx->ext_pwr_pin_id);
    if(ctx->ext_pwr_pin_id == 0)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_cam_ext_pwr_init_failed, P_INFO,
                      "create gpio ext pwr pin failed");
        return -1;
    }
    api_gpio_open(ctx->ext_pwr_pin_id, &pinConfig, 0);
    uint8_t active = 0;
    api_gpio_ioctl(ctx->ext_pwr_pin_id, OPEN_GPIO_IOCTL_OUT_ACT, &active);
    uint16_t level = 1;
    api_gpio_write(ctx->ext_pwr_pin_id, &level, 1);
    return 0;
}

int hal_cam_ext_pwr_set(bool on)
{
    HalCamCtx_t *ctx = GetHalCamCtx();
    if(ctx->ext_pwr_pin_id == 0)
    {
        return -1;
    }
    uint16_t level = on ? 1 : 0;
    api_gpio_write(ctx->ext_pwr_pin_id, &level, 1);
    return 0;
}

int hal_cam_recv(uint8_t *data, uint32_t size)
{
    HalCamCtx_t *ctx = GetHalCamCtx();
    if(ctx->cspi_usrid == 0)
    {
        return -1;
    }
    api_cspi_ioctl(ctx->cspi_usrid, OPEN_CSPI_IOCTL_START_RECV,
                   (uint8_t *)data);
    return 0;
}

int hal_cam_ext_pwr_deinit(CamExtPwrCfg_t *cfg)
{
    HalCamCtx_t *ctx = GetHalCamCtx();
    if(ctx->ext_pwr_pin_id != 0)
    {
        api_gpio_close(ctx->ext_pwr_pin_id);
        api_gpio_delete(ctx->ext_pwr_pin_id);
        ctx->ext_pwr_pin_id = 0;
    }
    return 0;
}

static int hal_cam_check_err()
{
    HalCamCtx_t *ctx = GetHalCamCtx();
    uint8_t cspi_err = 0;
    api_cspi_ioctl(ctx->cspi_usrid, OPEN_CSPI_IOCTL_GET_ERR_STATUS, &cspi_err);

    if(cspi_err != 0)
    {
        if(ctx->cb_err)
        {
            ctx->cb_err(cspi_err, ctx->cb_err_param);
        }
        return -1;
    }
    return 0;
}

static void hal_cam_get_timestamp(uint32_t *sysTime)
{
    // T_TMU_BC_RD* bcRd = HW_TmuBcRd; 4f0700c4
    uint32_t mask;
    uint32_t hfnsfnsbn;

    mask = SaveAndSetIRQMask();
    hfnsfnsbn = *(uint32_t *)0x4f0700c4;
    *sysTime = (hfnsfnsbn >> 4) * 10 + (hfnsfnsbn & 0xf);
    RestoreIRQMask(mask);
}

/*帧中断*/
static void hal_cam_cspi_event_cb()
{
    uint32_t status = 0;
    HalCamCtx_t *ctx = GetHalCamCtx();
    uint8_t frame_valid = 0;
    int ret = 0;
    if(ctx->cspi_usrid == 0)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_cam_cspi_event_cb_failed0, P_INFO,
                      "cspi_usrid is 0");
        return;
    }
    /*检查是否是帧尾中断*/
    api_cspi_ioctl(ctx->cspi_usrid, OPEN_CSPI_IOCTL_CHECK_FRAME_VALID,
                   &frame_valid);
    /* 关闭帧中断*/
    cspiIntCtrl_t cspi_int_ctrl = {0};
    api_cspi_ioctl(ctx->cspi_usrid, OPEN_CSPI_IOCTL_SET_INTTERUPT,
                   &cspi_int_ctrl);
    if(frame_valid != 0)
    {
        /*清除帧尾中断标志位*/
        api_cspi_ioctl(ctx->drv_id, OPEN_CSPI_IOCTL_CLEAR_FRAME_VALID, NULL);
        if(ctx->cb_event)
        {
            ctx->cb_event(status, ctx->cb_event_param);
        }
        uint8_t *frame_addr = NULL;
        ret = rbuffer_lock_back_free_frame(ctx->req_buf_pool, &frame_addr,
                                           ctx->frame_size);
        if(ret != 0)
        {
            ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_cam_cspi_event_cb_failed1,
                          P_INFO, "camera buffer is full");
            api_cspi_ioctl(ctx->cspi_usrid, OPEN_CSPI_IOCTL_SET_CBCTRL, NULL);
            cspi_int_ctrl.frameEndIntEn = 1;
            api_cspi_ioctl(ctx->cspi_usrid, OPEN_CSPI_IOCTL_SET_INTTERUPT,
                           &cspi_int_ctrl);
            return;
        }
        ImgHead_t *img_head = (ImgHead_t *)frame_addr;
        hal_cam_get_timestamp(&img_head->timestamp);
        hal_cam_recv(frame_addr + sizeof(ImgHead_t),
                     ctx->frame_size - sizeof(ImgHead_t));
        api_cspi_ioctl(ctx->cspi_usrid, OPEN_CSPI_IOCTL_SET_CBCTRL, NULL);
    }
    cspi_int_ctrl.frameEndIntEn = 1;
    api_cspi_ioctl(ctx->cspi_usrid, OPEN_CSPI_IOCTL_SET_INTTERUPT,
                   &cspi_int_ctrl);
}

/*DMA传输完成中断*/
static void hal_cam_dma_cb(uint32_t status)
{
    HalCamCtx_t *ctx = GetHalCamCtx();
    int ret = 0;
    if(hal_cam_check_err() != 0)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_cam_dma_cb_error, P_INFO,
                      "camera dma error");
        rbuffer_unlock_back_free_frame(ctx->req_buf_pool);
        return;
    }
    ret = rbuffer_unlock_and_push_back(ctx->req_buf_pool, ctx->frame_size);
    api_cspi_ioctl(ctx->cspi_usrid, OPEN_CSPI_IOCTL_SET_CBCTRL, NULL);
    if(ctx->cb_data_irq)
    {
        ctx->cb_data_irq(status, ctx->cb_data_irq_param);
    }
}

static int hal_cam_cspi_open(int drv_id)
{
    int user_id = 0;
    HalCamCtx_t *ctx = GetHalCamCtx();
    memcpy(&ctx->cspi_cfg, &s_cspi_default_cfg[drv_id],
           sizeof(CspiCfg_t[drv_id]));
    ctx->cspi_cfg.cbEvent = hal_cam_dma_cb;
    user_id = api_cspi_create(drv_id, &ctx->cspi_cfg);
    if(user_id == 0)
    {
        return -1;
    }
    api_cspi_open(user_id, NULL);
    return user_id;
}

int hal_cam_cmos_init(int id, CamExtPwrCfg_t *ext_pwr_cfg,
                      CamRstPinCfg_t *rst_pin_cfg, CamI2cCfg_t *i2c_cfg)
{
    HalCamCtx_t *ctx = GetHalCamCtx();
    hal_cam_init_i2c(i2c_cfg);
    hal_cam_reset_pin_init(rst_pin_cfg);
    hal_cam_ext_pwr_init(ext_pwr_cfg);
    hal_cam_ext_pwr_set(true);
    ctx->cspi_usrid = hal_cam_cspi_open(id);
    if(ctx->cspi_usrid == 0)
    {
        return -1;
    }
    uint8_t mclk_freq = CAM_24_M;
    api_cspi_ioctl(ctx->cspi_usrid, OPEN_CSPI_IOCTL_SET_BUS_SPEED, &mclk_freq);
    delay_us(10);
    hal_cam_reset(100);
    return 0;
}

static int hal_cam_cmos_uninit(int id)
{
    HalCamCtx_t *ctx = GetHalCamCtx();
    if(ctx->cspi_usrid == 0)
    {
        return -1;
    }
    api_cspi_ioctl(ctx->cspi_usrid, OPEN_CSPI_IOCTL_POWER_OFF, NULL);
    api_cspi_close(ctx->cspi_usrid);
    api_cspi_delete(ctx->cspi_usrid);
    ctx->cspi_usrid = 0;
    hal_cam_ext_pwr_set(false);
    hal_cam_uninit_i2c();
    return 0;
}

static int hal_cam_set_img_info(CamCfg_t *cfg)
{
    HalCamCtx_t *ctx = GetHalCamCtx();
    SensorFuncObj_t *cam_obj = hal_cam_get_func_obj();
    uint32_t width = cam_obj->img_width;
    uint32_t height = cam_obj->img_height;
    ctx->img_width = cfg->spi_cfg.rowScaleRatio == 0 ? width : width >> 1;
    ctx->img_height = cfg->spi_cfg.colScaleRatio == 0 ? height : height >> 1;
    if((cfg->reso == CAM_8W_Y) || (cfg->reso == CAM_30W_Y) ||
       (cfg->spi_cfg.yOnly == 1))
    {
        ctx->img_fmt = CAM_IMG_FMT_MONO;
    }
    else
    {
        ctx->img_fmt = CAM_IMG_FMT_YUV422;
    }
    return 0;
}

int hal_cam_init(CamCfg_t *cfg)
{
    HalCamCtx_t *ctx = GetHalCamCtx();
    IRQn_Type irqNum;
    int ret = 0;
    if(!cfg)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_cam_init_failed0, P_INFO,
                      "camera config is NULL");
        return -1;
    }
    if(ctx->init)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_cam_init_already_init, P_INFO,
                      "camera has been initialized");
        return 0;
    }
    hal_cam_cmos_init(cfg->drv_id, &cfg->ext_pwr_cfg, &cfg->rst_pin_cfg,
                      &cfg->i2c_cfg);
    ret = hal_cam_cspi_config(cfg->drv_id, &cfg->spi_cfg, cfg->reso,
                              &cfg->seq_cfg);
    if(ret != 0)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_cam_init_failed1, P_INFO,
                      "hal_cam_cspi_config failed");
        return -1;
    }

    ctx->drv_id = cfg->drv_id;
    irqNum = cfg->drv_id == 0 ? PXIC0_USP0_IRQn : PXIC0_USP1_IRQn;
    hal_cam_set_img_info(cfg);
    XIC_SetVector(irqNum, hal_cam_cspi_event_cb);
    XIC_EnableIRQ(irqNum);
    return 0;
}

void hal_cam_cspi_2_lspi(uint8_t enable)
{
    HalCamCtx_t *ctx = GetHalCamCtx();
    cspiFrameProcLspi_t cspi_frame_proc_lspi = {0};
    cspi_frame_proc_lspi.outEnLspi = enable;
    api_cspi_ioctl(ctx->cspi_usrid, OPEN_CSPI_IOCTL_SET_FRAME_PROC,
                   &cspi_frame_proc_lspi);
    // api_cspi_dump_reg(ctx->cspi_usrid);
}

int hal_cam_req_buf(CamReqBuf_t *req_buf)
{
    HalCamCtx_t *ctx = GetHalCamCtx();

    if(ctx->req_buf_pool)
    {
        rbuffer_close(ctx->req_buf_pool);
    }
    ctx->req_buf_pool = rbuffer_create(
        req_buf->pool_addr,
        req_buf->buff_count * (sizeof(ImgHead_t) + req_buf->buff_size),
        req_buf->buff_count, RQ_LOCK_TYPE_CRITICAL);
    if(!ctx->req_buf_pool)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_cam_req_buf_failed0, P_INFO,
                      "hal_cam_req_buf failed");
        return -1;
    }
    ctx->frame_size = req_buf->buff_size + sizeof(ImgHead_t);
    return 0;
}

int hal_cam_get_buf(CamImg_t *img, uint32_t timeout)
{
    int ret = 0;
    HalCamCtx_t *ctx = GetHalCamCtx();
    if(!ctx->req_buf_pool)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_cam_get_buf_failed0, P_INFO,
                      "hal_cam_get_buf failed");
        return -1;
    }
    uint8_t *frame_addr = NULL;
    uint32_t frame_size = 0;
    ret = rbuffer_lock_front_data_frame(ctx->req_buf_pool, &frame_addr,
                                        &frame_size, timeout);
    if(ret != 0)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_cam_get_buf_failed1, P_INFO,
                      "hal_cam_get_buf failed");
        return 1;
    }
    ImgHead_t *img_head = (ImgHead_t *)frame_addr;
    img->addr = frame_addr + sizeof(ImgHead_t);
    img->size = frame_size - sizeof(ImgHead_t);
    img->timestamp = img_head->timestamp;
    img->width = ctx->img_width;
    img->height = ctx->img_height;
    img->fmt = ctx->img_fmt;
    return 0;
}

int hal_cam_release_buf(CamImg_t *img)
{
    HalCamCtx_t *ctx = GetHalCamCtx();
    rbuffer_unlock_and_pop_front(ctx->req_buf_pool);
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_cam_release_buf, P_INFO,
                  "hal_cam_release_buf");
    return 0;
}

int hal_cam_clear_buff()
{
    HalCamCtx_t *ctx = GetHalCamCtx();
    rbuffer_reset(ctx->req_buf_pool);
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_cam_clear_buff, P_INFO,
                  "hal_cam_clear_buff");
    return 0;
}

int hal_cam_read_data(CamImg_t *img, uint32_t size)
{
    HalCamCtx_t *ctx = GetHalCamCtx();
    if(!img || !img->addr || size == 0)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_cam_read_data_failed0, P_INFO,
                      "camera img is NULL or size is 0");
        return -1;
    }
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_cam_read_data_enter, P_INFO,
                  "hal_cam_read_data");
    uint8_t *frame_addr = NULL;
    uint32_t frame_size = 0;
    int ret = rbuffer_lock_front_data_frame(ctx->req_buf_pool, &frame_addr,
                                            &frame_size, 100);
    if(ret != 0)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_cam_read_data_failed1, P_INFO,
                      "hal_cam_read_data failed");
        return -1;
    }
    ImgHead_t *head = (ImgHead_t *)frame_addr;
    memcpy(img->addr, frame_addr + sizeof(ImgHead_t), size);
    img->size = frame_size;
    img->timestamp = head->timestamp;
    img->fmt = ctx->img_fmt;
    img->width = ctx->img_width;
    img->height = ctx->img_height;
    rbuffer_unlock_and_pop_front(ctx->req_buf_pool);
    return 0;
}

int hal_cam_get_img_info(CamImgInfo_t *img_info)
{
    HalCamCtx_t *ctx = GetHalCamCtx();
    if(!img_info)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_cam_get_img_info_failed0, P_INFO,
                      "camera img info is NULL");
        return -1;
    }
    img_info->max_fps = ctx->cam_obj->max_fps;
    img_info->width = ctx->img_width;
    img_info->height = ctx->img_height;
    img_info->fmt = ctx->img_fmt;
    img_info->sns_height = ctx->cam_obj->img_height;
    img_info->sns_width = ctx->cam_obj->img_width;
    return 0;
}

int hal_cam_deinit()
{
    HalCamCtx_t *ctx = GetHalCamCtx();
    hal_cam_cmos_uninit(ctx->drv_id);
    if(ctx->req_buf_pool)
    {
        rbuffer_close(ctx->req_buf_pool);
        ctx->req_buf_pool = NULL;
    }
    return 0;
}

int hal_cam_register_cb(CamCbCfg_t *cfg)
{
    HalCamCtx_t *ctx = GetHalCamCtx();
    if(!cfg)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_cam_register_cb_failed0, P_INFO,
                      "camera cb config is NULL");
        return -1;
    }
    ctx->cb_data_irq = cfg->cb_data_irq;
    ctx->cb_data_irq_param = cfg->cb_data_irq_param;
    ctx->cb_err = cfg->cb_err;
    ctx->cb_err_param = cfg->cb_err_param;
    ctx->cb_event = cfg->cb_event;
    ctx->cb_event_param = cfg->cb_event_param;
    return 0;
}

int hal_cam_start()
{
    HalCamCtx_t *ctx = GetHalCamCtx();
    if(!ctx->req_buf_pool)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, hal_cam_start_failed1, P_INFO,
                      "buffer pool is NULL, please request buffer first");
        return -1;
    }
    rbuffer_reset(ctx->req_buf_pool);
    api_cspi_ioctl(ctx->cspi_usrid, OPEN_CSPI_IOCTL_START, NULL);
    return 0;
}

int hal_cam_stop()
{
    HalCamCtx_t *ctx = GetHalCamCtx();
    api_cspi_ioctl(ctx->cspi_usrid, OPEN_CSPI_IOCTL_STOP, NULL);
    api_cspi_ioctl(ctx->cspi_usrid, OPEN_CSPI_IOCTL_POWER_OFF, NULL);
    return 0;
}

CamCfg_t *hal_cam_get_default_cfg(CamType_e type)
{
    int type_num = sizeof(s_cam_list) / sizeof(s_cam_list[0]);
    for(int i = 0; i < type_num; i++)
    {
        if(s_cam_list[i]->type == type)
        {
            return s_cam_list[i]->default_cfg;
        }
    }
    return NULL;
}
