#ifdef FEATURE_HAL_CAM_ENABLE
#include "api_cspi.h"
#include <assert.h>
#include "Driver_Common.h"
#include "slpman.h"

#include DEBUG_LOG_HEADER_FILE

#define EPAT_LOG(subId, debugLevel, format, ...) \
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, subId, debugLevel, format, ##__VA_ARGS__)

typedef struct
{
    bool isInited; /**< Whether spi has been initialized */
    struct
    {
        __IO uint32_t
            DFMT; /**< Data Format Register,                offset: 0x0 */
        __IO uint32_t
            SLOTCTL; /**< Slot Control Register,               offset: 0x4 */
        __IO uint32_t
            CLKCTL; /**< Clock Control Register,              offset: 0x8 */
        __IO uint32_t
            DMACTL; /**< DMA Control Register,                offset: 0xC */
        __IO uint32_t
            INTCTL; /**< Interrupt Control Register,          offset: 0x10 */
        __IO uint32_t TIMEOUTCTL; /**< Timeout Control Register, offset: 0x14 */
        __IO uint32_t
            STAS; /**< Status Register,                     offset: 0x18 */
        __IO uint32_t
            CSPICTL; /**< Camera SPI Control Register,         offset: 0x28 */
        __IO uint32_t
            CCTL; /**< Auto Cg Control Register,            offset: 0x2c */
        __IO uint32_t CSPIINFO0; /**< Cspi Frame info0 Register, offset: 0x30 */
        __IO uint32_t CSPIINFO1; /**< Cspi Frame info1 Register, offset: 0x34 */
        __IO uint32_t
            CSPIDBG; /**< Cspi Debug Register,                 offset: 0x38 */
        __IO uint32_t
            CSPINIT; /**< Cspi Init Register,                  offset: 0x3c */
        __IO uint32_t
            CLSP; /**< Cspi Line Start Register,            offset: 0x40 */
        __IO uint32_t
            CDATP; /**< Cspi Data Packet Register,           offset: 0x44 */
        __IO uint32_t
            CLINFO; /**< Cspi Line Info Register,             offset: 0x48 */
        __IO uint32_t
            CBCTRL; /**< Cspi binary ctrl                     offset: 0x4c */
        __IO uint32_t CSPIPROCLSPI; /**< Cspi frame proc lspi offset: 0x50 */
        __IO uint32_t CSPIQUARTILE; /**< Cspi OTSU quartile offset: 0x54 */
        __IO uint32_t CSPIYADJ;     /**< Cspi y Adjustment     offset: 0x58 */
#if((defined CHIP_EC718) && !(defined TYPE_EC718M)) || (defined CHIP_EC716)
        __IO uint32_t CSPIDLYCTRL;  /**< Cspi delay ctrl  offset: 0xD0 */
        __IO uint32_t I2SBUSSEL;    /**< Cspi bus select    offset: 0xe0 */
        __IO uint32_t HISTOBUFCTRL; /**< Histogram buf ctrl offset: 0xf0 */
#else
        __IO uint32_t CSPIDDRCTRL;  /**< Cspi DDR ctrl  offset: 0x5c */
        __IO uint32_t CSPIDLYCTRL;  /**< Cspi delay ctrl  offset: 0xD8 */
        __IO uint32_t I2SBUSSEL;    /**< Cspi bus select    offset: 0xe4 */
        __IO uint32_t HISTOBUFCTRL; /**< Histogram buf ctrl offset: 0xf0 */
        __IO uint32_t USBVERSION;   /**< Usp version   offset: 0xf4 */
#endif
    } regsBackup;
} cspiDataBase_t;

static uint32_t cspiInitCnt = 0;

static uint32_t sCspiUsrIdList[EC_CSPI_INDEX_LIMIT] = {0};
static uint16_t sCspiUsrIdSeed[EC_CSPI_INDEX_LIMIT] = {0};

void cspi0_dma_rx_event(uint32_t event);
PLAT_FM_ZI static DmaDescriptor_t __ALIGNED(16)
    cspi0_dma_rx_desc[CAM_CHAIN_COUNT];
void cspi1_dma_rx_event(uint32_t event);
PLAT_FM_ZI static DmaDescriptor_t __ALIGNED(16)
    cspi1_dma_rx_desc[CAM_CHAIN_COUNT];
typedef struct CspiCtx_
{
    CSPI_TypeDef *instance;
    cspiDataBase_t database;
    cspiRes_t res;
    cspiDma_t cspi_dma;
    DmaTransferConfig_t dma_cfg;
    cspiInfo_t cspi_info;
} CspiCtx_t;

CspiCtx_t *get_cspi_ctx(int index)
{
    static CspiCtx_t s_ctx[EC_CSPI_INDEX_LIMIT] = {0};
    return &s_ctx[index];
}

void cspi_dma_rx_event(uint32_t event, int index, cspiRes_t *cspi)
{
    CspiCtx_t *ctx = get_cspi_ctx(index);
    switch(event)
    {
        case DMA_EVENT_END:
            if(cspi->info->cbEvent)
            {
                ctx->instance->CBCTRL &= ~3 << 25;
                ctx->instance->STAS |= 0x3 << 3;
                ctx->instance->STAS |= 0xf << 7;
                ctx->instance->STAS |= 0x3 << 11;
                ctx->instance->DMACTL |= 1 << 24;
            }
            cspi->info->cbEvent(ARM_CSPI_EVENT_TRANSFER_COMPLETE);
            break;

        default:
            break;
    }
}

void cspi0_dma_rx_event(uint32_t event)
{
    CspiCtx_t *ctx = get_cspi_ctx(0);
    cspi_dma_rx_event(event, 0, &ctx->res);
}

void cspi1_dma_rx_event(uint32_t event)
{
    CspiCtx_t *ctx = get_cspi_ctx(1);
    cspi_dma_rx_event(event, 1, &ctx->res);
}

void cspi_ctx_reset(int index, CspiCtx_t *ctx)
{
    ctx->instance = index == 0 ? CSPI0 : CSPI1;
    memset(&ctx->database, 0, sizeof(cspiDataBase_t));
    ctx->res.reg = ctx->instance;
    memset(&ctx->res.pins, 0, sizeof(cspiPins_t));
    memset(&ctx->cspi_dma, 0, sizeof(cspiDma_t));
    ctx->cspi_dma.rxInstance = DMA_INSTANCE_MP;
    ctx->cspi_dma.rxCh = -1;
    ctx->cspi_dma.rxReq =
        index == 0 ? DMA_REQUEST_USP0_RX : DMA_REQUEST_USP1_RX;
    ctx->cspi_dma.rxCb = index == 0 ? cspi0_dma_rx_event : cspi1_dma_rx_event;
    ctx->cspi_dma.descriptor =
        index == 0 ? cspi0_dma_rx_desc : cspi1_dma_rx_desc;
    ctx->res.dma = &ctx->cspi_dma;

    memset(&ctx->cspi_info, 0, sizeof(cspiInfo_t));
    ctx->res.info = &ctx->cspi_info;
    ctx->dma_cfg.sourceAddress = NULL;
    ctx->dma_cfg.targetAddress = NULL;
    ctx->dma_cfg.flowControl = DMA_FLOW_CONTROL_SOURCE;
    ctx->dma_cfg.addressIncrement = DMA_ADDRESS_INCREMENT_TARGET;
    ctx->dma_cfg.dataWidth = DMA_DATA_WIDTH_FOUR_BYTES;
    ctx->dma_cfg.burstSize = DMA_BURST_32_BYTES;
    ctx->dma_cfg.totalLength = 0;
}

static int32_t cspi_set_bus_speed(int index, camFrequence_e speed)
{
    CspiCtx_t *ctx = get_cspi_ctx(index);
    cspiRes_t *cspi = &ctx->res;
    uint32_t freqDivInteger = 0;
    uint32_t freqDivRatio = 0;
    switch(speed)
    {
        case CAM_6_5_M:
            freqDivInteger = 0x5E;
            freqDivRatio = 0x85e85f;
            break;

        case CAM_13_M:
            freqDivInteger = 0x2f;
            freqDivRatio = 0x42f42f;
            break;

        case CAM_25_5_M:
            freqDivInteger = 0x18;
            freqDivRatio = 0x181818;
            break;

        case CAM_24_M:
            freqDivInteger = 0x19;
            freqDivRatio = 0x00000a;
            break;

        default:
            break;
    }

    cspi->reg->I2SBUSSEL |= CSPI_BUS_EN_Msk;
    cspi->info->busSpeed = speed;
    if(index == 0)  // i2s0
    {
        CLOCK_fracDivOutCLkEnable(FRACDIV0_OUT0);  // Fracdiv1_en
        CLOCK_setFracDivOutClkDiv(FRACDIV0_OUT0, 1);
        CLOCK_setMclkSrc(MCLK0, MCLK_SRC_FRACDIV0_OUT0);  // Bmclk_sel1
        CLOCK_mclkEnable(MCLK0);                          // Mclk_oe1

        // Fracdiv clk selects 408M and set frac and integer clk
        FracDivConfig_t fracdivCfg;
        memset(&fracdivCfg, 0, sizeof(FracDivConfig_t));
        fracdivCfg.fracdivSel = FRACDIV_0;
        fracdivCfg.source = FRACDIC_ROOT_CLK_612M;
        fracdivCfg.fracDiv0DivRatioInteger = freqDivInteger;
        fracdivCfg.fracDiv0DivRatioFrac = freqDivRatio;
        CLOCK_setFracDivConfig(&fracdivCfg);
    }
    else  // i2s1
    {
        CLOCK_setMclkSrc(MCLK1, MCLK_SRC_FRACDIV1_OUT0);  // Bmclk_sel1
        CLOCK_setFracDivOutClkDiv(FRACDIV1_OUT0, 1);

        CLOCK_fracDivOutCLkEnable(FRACDIV1_OUT0);  // Fracdiv1_en
        CLOCK_mclkEnable(MCLK1);                   // Mclk_oe1

        // Fracdiv clk selects 614M and set frac and integer clk
        FracDivConfig_t fracdivCfg;
        memset(&fracdivCfg, 0, sizeof(FracDivConfig_t));
        fracdivCfg.fracdivSel = FRACDIV_1;
        fracdivCfg.source = FRACDIC_ROOT_CLK_612M;
        fracdivCfg.fracDiv1DivRatioInteger = freqDivInteger;  // 24M
        fracdivCfg.fracDiv1DivRatioFrac = freqDivRatio;
        CLOCK_setFracDivConfig(&fracdivCfg);
    }

    CLOCK_setClockSrc(CLK_FRACDIV, CLK_FRACDIV_SEL_612M);
    CLOCK_clockEnable(CLK_FRACDIV);

    return ARM_DRIVER_OK;
}

static void cspiEnterLpStatePrepare(void *pdata, slpManLpState state)
{
    uint32_t i;
    switch(state)
    {
        case SLPMAN_SLEEP1_STATE:

            for(i = 0; i < EC_CSPI_INDEX_LIMIT; i++)
            {
                CspiCtx_t *ctx = get_cspi_ctx(i);
                cspiDataBase_t *db = &ctx->database;
                if(db->isInited == true)
                {
                    db->regsBackup.DFMT = ctx->instance->DFMT;
                    db->regsBackup.SLOTCTL = ctx->instance->SLOTCTL;
                    db->regsBackup.CLKCTL = ctx->instance->CLKCTL;
                    db->regsBackup.DMACTL = ctx->instance->DMACTL;
                    db->regsBackup.INTCTL = ctx->instance->INTCTL;
                    db->regsBackup.TIMEOUTCTL = ctx->instance->TIMEOUTCTL;
                    db->regsBackup.STAS = ctx->instance->STAS;
                    db->regsBackup.CSPICTL = ctx->instance->CSPICTL;
                    db->regsBackup.CCTL = ctx->instance->CCTL;
                    db->regsBackup.CSPIINFO0 = ctx->instance->CSPIINFO0;
                    db->regsBackup.CSPIINFO1 = ctx->instance->CSPIINFO1;
                    db->regsBackup.CSPIDBG = ctx->instance->CSPIDBG;
                    db->regsBackup.CSPINIT = ctx->instance->CSPINIT;
                    db->regsBackup.CLSP = ctx->instance->CLSP;
                    db->regsBackup.CDATP = ctx->instance->CDATP;
                    db->regsBackup.CLINFO = ctx->instance->CLINFO;
                    db->regsBackup.CBCTRL = ctx->instance->CBCTRL;
                    db->regsBackup.CSPIPROCLSPI = ctx->instance->CSPIPROCLSPI;
                    db->regsBackup.CSPIQUARTILE = ctx->instance->CSPIQUARTILE;
                    db->regsBackup.CSPIYADJ = ctx->instance->CSPIYADJ;
#if((defined CHIP_EC718) && !(defined TYPE_EC718M)) || (defined CHIP_EC716)
                    db->regsBackup.CSPIDLYCTRL = ctx->instance->CSPIDLYCTRL;
                    db->regsBackup.I2SBUSSEL = ctx->instance->I2SBUSSEL;
                    db->regsBackup.HISTOBUFCTRL = ctx->instance->HISTOBUFCTRL;
#else  // 719
                    db->regsBackup.CSPIDDRCTRL = ctx->instance->CSPIDDRCTRL;
                    db->regsBackup.CSPIDLYCTRL = ctx->instance->CSPIDLYCTRL;
                    db->regsBackup.I2SBUSSEL = ctx->instance->I2SBUSSEL;
                    db->regsBackup.HISTOBUFCTRL = ctx->instance->HISTOBUFCTRL;
                    db->regsBackup.USBVERSION = ctx->instance->USBVERSION;
#endif
                }
            }
            break;
        default:
            break;
    }
}

static void cspiExitLpStateRestore(void *pdata, slpManLpState state)
{
    uint32_t i;

    switch(state)
    {
        case SLPMAN_SLEEP1_STATE:

            for(i = 0; i < EC_CSPI_INDEX_LIMIT; i++)
            {
                CspiCtx_t *ctx = get_cspi_ctx(i);
                cspiRes_t *cspi = &ctx->res;
                cspiDataBase_t *db = &ctx->database;
                if(db->isInited == true)
                {
                    GPR_clockEnable((i == 0) ? PCLK_USP0 : PCLK_USP1);
                    GPR_clockEnable((i == 0) ? FCLK_USP0 : FCLK_USP1);
                    cspi_set_bus_speed(i, cspi->info->busSpeed);
                    ctx->instance->DFMT = db->regsBackup.DFMT;
                    ctx->instance->SLOTCTL = db->regsBackup.SLOTCTL;
                    ctx->instance->CLKCTL = db->regsBackup.CLKCTL;
                    ctx->instance->DMACTL = db->regsBackup.DMACTL;
                    ctx->instance->INTCTL = db->regsBackup.INTCTL;
                    ctx->instance->TIMEOUTCTL = db->regsBackup.TIMEOUTCTL;
                    ctx->instance->STAS = db->regsBackup.STAS;
                    ctx->instance->CSPICTL = db->regsBackup.CSPICTL;
                    ctx->instance->CCTL = db->regsBackup.CCTL;
                    ctx->instance->CSPIINFO0 = db->regsBackup.CSPIINFO0;
                    ctx->instance->CSPIINFO1 = db->regsBackup.CSPIINFO1;
                    ctx->instance->CSPIDBG = db->regsBackup.CSPIDBG;
                    ctx->instance->CSPINIT = db->regsBackup.CSPINIT;
                    ctx->instance->CLSP = db->regsBackup.CLSP;
                    ctx->instance->CDATP = db->regsBackup.CDATP;
                    ctx->instance->CLINFO = db->regsBackup.CLINFO;
                    ctx->instance->CBCTRL = db->regsBackup.CBCTRL;
                    ctx->instance->CSPIPROCLSPI = db->regsBackup.CSPIPROCLSPI;
                    ctx->instance->CSPIQUARTILE = db->regsBackup.CSPIQUARTILE;
                    ctx->instance->CSPIYADJ = db->regsBackup.CSPIYADJ;
#if((defined CHIP_EC718) && !(defined TYPE_EC718M)) || (defined CHIP_EC716)
                    ctx->instance->CSPIDLYCTRL = db->regsBackup.CSPIDLYCTRL;
                    ctx->instance->I2SBUSSEL = db->regsBackup.I2SBUSSEL;
                    ctx->instance->HISTOBUFCTRL = db->regsBackup.HISTOBUFCTRL;
#else
                    ctx->instance->CSPIDDRCTRL = db->regsBackup.CSPIDDRCTRL;
                    ctx->instance->CSPIDLYCTRL = db->regsBackup.CSPIDLYCTRL;
                    ctx->instance->I2SBUSSEL = db->regsBackup.I2SBUSSEL;
                    ctx->instance->HISTOBUFCTRL = db->regsBackup.HISTOBUFCTRL;
                    ctx->instance->USBVERSION = db->regsBackup.USBVERSION;
#endif
                }
            }
            break;

        default:
            break;
    }
}

static int32_t cspi_open(int index)
{
    CspiCtx_t *ctx = get_cspi_ctx(index);
    int32_t returnCode;
    // PadConfig_t config;
    cspiRes_t *cspi = &ctx->res;
    cspiDataBase_t *db = &ctx->database;
#ifdef PM_FEATURE_ENABLE
    db->isInited = true;
    apmuVoteToDozeState(PMU_DOZE_USP_MOD, false);
#endif
    // Configure DMA if necessary
    if(cspi->dma)
    {
        DMA_init(cspi->dma->rxInstance);
        returnCode = DMA_openChannel(cspi->dma->rxInstance);

        if(returnCode == ARM_DMA_ERROR_CHANNEL_ALLOC)
            return ARM_DRIVER_ERROR;
        else
            cspi->dma->rxCh = returnCode;

        DMA_setChannelRequestSource(cspi->dma->rxInstance, cspi->dma->rxCh,
                                    (DmaRequestSource_e)cspi->dma->rxReq);
        DMA_rigisterChannelCallback(cspi->dma->rxInstance, cspi->dma->rxCh,
                                    cspi->dma->rxCb);

        // Configure rx DMA and start it
        ctx->dma_cfg.sourceAddress = (void *)&(ctx->instance->RFIFO);
        ctx->dma_cfg.totalLength = CSPI_TRANSFER_TRUNK_SIZE;

        DMA_enableChannelInterrupts(cspi->dma->rxInstance, cspi->dma->rxCh,
                                    DMA_END_INTERRUPT_ENABLE);
#ifndef FEATURE_SUBSYS_CAMERA_ENABLE
        DMA_startChannel(cspi->dma->rxInstance, cspi->dma->rxCh);
#endif
    }

    // Select cspi bus
    cspi->reg->I2SBUSSEL |= CSPI_BUS_EN_Msk;

#ifdef PM_FEATURE_ENABLE
    cspiInitCnt++;

    if(cspiInitCnt == 1U)
    {
        slpManRegisterPredefinedBackupCb(SLP_CALLBACK_I2S_MODULE,
                                         cspiEnterLpStatePrepare, NULL);
        slpManRegisterPredefinedRestoreCb(SLP_CALLBACK_I2S_MODULE,
                                          cspiExitLpStateRestore, NULL);
    }
#endif

    return ARM_DRIVER_OK;
}

int32_t cspi_close(int index)
{
    CspiCtx_t *ctx = get_cspi_ctx(index);
    cspiRes_t *cspi = &ctx->res;
#ifdef PM_FEATURE_ENABLE
    ctx->database.isInited = false;
    cspiInitCnt--;
    if(cspiInitCnt == 0)
    {
        slpManUnregisterPredefinedBackupCb(SLP_CALLBACK_I2S_MODULE);
        slpManUnregisterPredefinedRestoreCb(SLP_CALLBACK_I2S_MODULE);
    }

    apmuVoteToDozeState(PMU_DOZE_USP_MOD, true);
#endif

    DMA_closeChannel(cspi->dma->rxInstance, cspi->dma->rxCh);
    return ARM_DRIVER_OK;
}

static int32_t cspi_power_ctrl(int index, cspiPowerState_e state)
{
    CspiCtx_t *ctx = get_cspi_ctx(index);
    cspiRes_t *cspi = &ctx->res;
    EPAT_LOG(cspi_power_ctrl, P_INFO, "cspi %d: power %s", index,
             state == CSPI_POWER_OFF ? "off" : "on");
    switch(state)
    {
        case CSPI_POWER_OFF: {
            // DMA disable
            if(cspi->dma)
            {
                DMA_stopChannel(cspi->dma->rxInstance, cspi->dma->rxCh, true);
            }
            // Reset register values
            if(index == 0)
            {
                CLOCK_setClockSrc(FCLK_USP0, FCLK_USP0_SEL_26M);
                GPR_swReset(RST_PCLK_USP0);
            }
            else
            {
                CLOCK_setClockSrc(FCLK_USP1, FCLK_USP1_SEL_26M);
                GPR_swReset(RST_PCLK_USP1);
            }
            // disable fracdiv
            CLOCK_clockDisable(CLK_FRACDIV);
            // Disable CSPI fclk and pclk
            CLOCK_clockDisable((index == 0) ? PCLK_USP0 : PCLK_USP1);
            CLOCK_clockDisable((index == 0) ? FCLK_USP0 : FCLK_USP1);
            CLOCK_clockDisable(CLK_HF306M_G);
            break;
        }
        case CSPI_POWER_FULL: {
            // Enable CSPI clock
            CLOCK_clockEnable((index == 0) ? PCLK_USP0 : PCLK_USP1);

#if((defined CHIP_EC718) || (defined CHIP_EC716)) && !(defined TYPE_EC718M)
            if(index == 0)
            {
                CLOCK_setClockSrc(FCLK_USP0,
                                  FCLK_USP0_SEL_102M);  // select USP1 102M
            }
            else
            {
                CLOCK_setClockSrc(FCLK_USP1,
                                  FCLK_USP1_SEL_102M);  // select USP1 102M
            }
#elif(defined TYPE_EC718M)
            if(index == 0)
            {
                CLOCK_setClockSrc(FCLK_USP0,
                                  FCLK_USP0_SEL_612M);  // select USP0 102M
                CLOCK_setClockDiv(FCLK_USP0, 6);
            }
            else
            {
                CLOCK_setClockSrc(FCLK_USP1,
                                  FCLK_USP1_SEL_612M);  // select USP1 102M
                CLOCK_setClockDiv(FCLK_USP1, 6);
            }
#endif
            CLOCK_clockEnable(CLK_HF306M_G);  // open cspi fclk src
            CLOCK_clockEnable((index == 0) ? FCLK_USP0 : FCLK_USP1);

            break;
        }
        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
    return ARM_DRIVER_OK;
}

int32_t cspi_receive(int index)
{
    CspiCtx_t *ctx = get_cspi_ctx(index);
    cspiRes_t *cspi = &ctx->res;
#ifdef FEATURE_SUBSYS_CAMERA_ENABLE
    DMA_startChannel(cspi->dma->rxInstance, cspi->dma->rxCh);
#endif
    DMA_buildDescriptorChain(cspi->dma->descriptor, &ctx->dma_cfg,
                             cspi->info->resolution, true, true, true);
    DMA_loadChannelDescriptorAndRun(cspi->dma->rxInstance, cspi->dma->rxCh,
                                    cspi->dma->descriptor);

    return ARM_DRIVER_OK;
}

static uint32_t usrId_to_cspi(uint32_t usrId)
{
    ASSERT(usrId > 0);
    uint32_t cspi = (uint32_t)(usrId & OPEN_HAL_PORT_MUSK);  // 位提取
    ASSERT(cspi < EC_CSPI_INDEX_LIMIT);
    if(cspi == (sCspiUsrIdList[cspi] & OPEN_HAL_PORT_MUSK))
    {
        return cspi;
    }
    return EC_CSPI_INDEX_LIMIT;
}

api_ret_t api_cspi_query(uint32_t usrId)
{
    uint32_t index = usrId_to_cspi(usrId);
    if(index >= EC_CSPI_INDEX_START && index < EC_CSPI_INDEX_LIMIT)
    {
        if(sCspiUsrIdList[index] == OPEN_HAL_STAT_UNUSED)
        {
            return OPEN_HAL_FREE;
        }
        else if(sCspiUsrIdList[index] & OPEN_HAL_STAT_MUSK)
        {
            return OPEN_HAL_USED;
        }
        else
        {
            return OPEN_HAL_IDLE;
        }
    }
    return OPEN_HAL_NONE;
}

static uint32_t cspi_set_free(uint32_t index)
{
    if(index >= EC_CSPI_INDEX_START && index < EC_CSPI_INDEX_LIMIT)
    {
        sCspiUsrIdList[index] = OPEN_HAL_STAT_UNUSED;
        return 1;
    }
    return 0;
}

static uint32_t cspi_set_idle(uint32_t index)
{
    ASSERT(index < EC_CSPI_INDEX_LIMIT);
    if(sCspiUsrIdList[index] == OPEN_HAL_STAT_UNUSED)
    {
        sCspiUsrIdList[index] = index;
        sCspiUsrIdSeed[index]++;
        sCspiUsrIdList[index] |= (uint32_t)(sCspiUsrIdSeed[index] << 16);
        return sCspiUsrIdList[index];
    }
    else if(sCspiUsrIdList[index] & OPEN_HAL_STAT_MUSK)
    {
        sCspiUsrIdList[index] &= ~(OPEN_HAL_STAT_MUSK);
        return sCspiUsrIdList[index];
    }
    return 0;
}

static uint32_t cspi_set_used(uint32_t usrId)
{
    api_ret_t stat = api_cspi_query(usrId);
    if(stat != OPEN_HAL_IDLE)
    {
        return 0;
    }
    uint32_t index = usrId_to_cspi(usrId);
    sCspiUsrIdList[index] |= OPEN_HAL_STAT_MUSK;
    return 0;
}

int api_cspi_startup(void *para)
{
    for(int i = EC_CSPI_INDEX_START; i < EC_CSPI_INDEX_LIMIT; i++)
    {
        cspi_ctx_reset(i, get_cspi_ctx(i));
        cspi_set_free(i);
    }
    EPAT_LOG(api_cspi_startup, P_INFO, "start up");
    return 0;
}

uint32_t api_cspi_create(CspiIdx_e index, CspiCfg_t *cfg)
{
    uint32_t usrId = cspi_set_idle(index);
    CspiCtx_t *ctx = get_cspi_ctx(index);
    cspiRes_t *cspi = &ctx->res;
    if(usrId)
    {
        if(cfg != NULL)
        {
            memcpy(&cspi->pins, &cfg->pins, sizeof(cspiPins_t));
            cspi->info->cbEvent = cfg->cbEvent;
        }
    }
    EPAT_LOG(api_cspi_create, P_INFO, "cspi(%d): get user id %x, rxReq: %d",
             index, usrId, ctx->cspi_dma.rxReq);
    return usrId;
}

api_ret_t api_cspi_delete(uint32_t usrId)
{
    api_ret_t ret = api_cspi_query(usrId);
    uint32_t index = usrId_to_cspi(usrId);
    CspiCtx_t *ctx = get_cspi_ctx(index);
    if(ret == OPEN_HAL_IDLE)
    {
        if(cspi_set_free(index))
        {
            cspi_ctx_reset(index, ctx);
        }
        ret = OPEN_HAL_DONE;
    }
    EPAT_LOG(api_cspi_delete, P_INFO, "usrid: %x, index: %d, ret: %d", usrId,
             index, ret);
    return ret;
}

api_ret_t api_cspi_open(uint32_t usrId, CspiCfg_t *cfg)
{
    ASSERT(usrId > 0);
    api_ret_t ret = api_cspi_query(usrId);
    uint32_t index = usrId_to_cspi(usrId);
    if(ret == OPEN_HAL_IDLE)
    {
        CspiCtx_t *ctx = get_cspi_ctx(index);
        cspiRes_t *cspi = &ctx->res;
        if(cfg != NULL)
        {
            memcpy(&cspi->pins, &cfg->pins, sizeof(cspiPins_t));
            cspi->info->cbEvent = cfg->cbEvent;
        }
        // cspi power on
        cspi_power_ctrl(index, CSPI_POWER_FULL);
        // todo: add cspi init here
        if(ARM_DRIVER_OK != cspi_open(index))
        {
            return OPEN_HAL_FREE;
        }
        cspi_set_used(usrId);
        ret = OPEN_HAL_DONE;
    }
    EPAT_LOG(api_cspi_open, P_INFO, "usrid: %x, index: %d, ret: %d", usrId,
             index, ret);
    return ret;
}

api_ret_t api_cspi_close(uint32_t usrId)
{
    api_ret_t ret = api_cspi_query(usrId);
    uint32_t index = usrId_to_cspi(usrId);
    if(ret == OPEN_HAL_USED)
    {
        cspi_set_idle(index);
        cspi_close(index);
        ret = OPEN_HAL_DONE;
    }
    EPAT_LOG(api_cspi_close, P_INFO, "usrid: %x, index: %d, ret: %d", usrId,
             index, ret);
    return ret;
}

api_ret_t api_cspi_ioctl(uint32_t usrId, CspiIoCtrl_e type, void *para)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_cspi_query(usrId);
    uint32_t index = usrId_to_cspi(usrId);
    CspiCtx_t *ctx = get_cspi_ctx(index);
    cspiRes_t *cspi = &ctx->res;
    if(ret == OPEN_HAL_USED)
    {
        switch(type)
        {
            case OPEN_CSPI_IOCTL_POWER_ON: {
                cspi_power_ctrl(index, CSPI_POWER_FULL);
                break;
            }
            case OPEN_CSPI_IOCTL_POWER_OFF: {
                cspi_power_ctrl(index, CSPI_POWER_OFF);
                break;
            }
            case OPEN_CSPI_IOCTL_TRANS_ABOUT: {
                // If DMA mode, disable DMA channel
                if(cspi->dma)
                {
                    DMA_stopChannel(cspi->dma->rxInstance, cspi->dma->rxCh,
                                    true);
                }
                // clear SPI run-time resources
                cspi->reg->CSPICTL &= ~CSPI_ENABLE_Msk;
                break;
            }
            case OPEN_CSPI_IOCTL_SET_BUS_SPEED: {
                uint8_t speed = *(uint8_t *)para;
                cspi_set_bus_speed(index, (camFrequence_e)speed);
                break;
            }
            case OPEN_CSPI_IOCTL_SET_DATA_FORMAT: {
                memcpy((void *)&(ctx->instance->DFMT), (cspiDataFmt_t *)para,
                       sizeof(cspiDataFmt_t));
                break;
            }
            case OPEN_CSPI_IOCTL_FLUSH_RX_FIFO: {
                ctx->instance->DMACTL |= 0x1000000;
                break;
            }
            case OPEN_CSPI_IOCTL_SET_INTTERUPT: {
                memcpy((void *)&(ctx->instance->INTCTL), (cspiIntCtrl_t *)para,
                       sizeof(cspiIntCtrl_t));
                break;
            }
            case OPEN_CSPI_IOCTL_SET_RECV_TIMEOUT: {
                memcpy((void *)&(ctx->instance->TIMEOUTCTL),
                       (cspiTimeOutCycle_t *)para, sizeof(cspiTimeOutCycle_t));
                break;
            }
            case OPEN_CSPI_IOCTL_SET_DMA_CTRL: {
                memcpy((void *)&(ctx->instance->DMACTL), (cspiDmaCtrl_t *)para,
                       sizeof(cspiDmaCtrl_t));
                break;
            }
            case OPEN_CSPI_IOCTL_SET_CSPI_CFG: {
                memcpy((void *)&(ctx->instance->CSPICTL), (cspiCtrl_t *)para,
                       sizeof(cspiCtrl_t));
                break;
            }
            case OPEN_CSPI_IOCTL_SET_DELAY_CTRL: {
                memcpy((void *)&(ctx->instance->CSPIDLYCTRL),
                       (cspiDelayCtrl_t *)para, sizeof(cspiDelayCtrl_t));
                break;
            }
            case OPEN_CSPI_IOCTL_START: {
                ctx->instance->CSPICTL |= CSPI_ENABLE_Msk;
                break;
            }
            case OPEN_CSPI_IOCTL_STOP: {
                ctx->instance->CSPICTL &= ~CSPI_ENABLE_Msk;
                break;
            }
            case OPEN_CSPI_IOCTL_SET_FRAME_INFO0: {
                memcpy((void *)&(ctx->instance->CSPIINFO0),
                       (cspiFrameInfo0_t *)para, sizeof(cspiFrameInfo0_t));
                break;
            }
            case OPEN_CSPI_IOCTL_SET_BINARY: {
                memcpy((void *)&(ctx->instance->CBCTRL),
                       (cspiBinaryCtrl_t *)para, sizeof(cspiBinaryCtrl_t));
                break;
            }
            case OPEN_CSPI_IOCTL_AUTO_CG_CTRL: {
                memcpy((void *)&(ctx->instance->CCTL), (cspiAutoCgCtrl_t *)para,
                       sizeof(cspiAutoCgCtrl_t));
                break;
            }
            case OPEN_CSPI_IOCTL_SET_RESOLUTION: {
                uint8_t reso = *(uint8_t *)para;
                cspi->info->resolution = (camResolution_e)reso;
                break;
            }
            case OPEN_CSPI_IOCTL_SET_FRAME_PROC: {
                memcpy((void *)&(ctx->instance->CSPIPROCLSPI),
                       (cspiFrameProcLspi_t *)para,
                       sizeof(cspiFrameProcLspi_t));
                break;
            }
            case OPEN_CSPI_IOCTL_CHECK_FRAME_VALID: {
                *(uint8_t *)para =
                    (ctx->instance->STAS & ICL_STATS_FRAME_END_Msk) != 0 ? 1
                                                                         : 0;
                break;
            }
            case OPEN_CSPI_IOCTL_CLEAR_FRAME_VALID: {
                ctx->instance->STAS |= 0x3 << 3;
                ctx->instance->STAS |= 0xf << 7;
                ctx->instance->STAS |= 0x3 << 11;
                ctx->instance->DMACTL |= 1 << 24;
                break;
            }
            case OPEN_CSPI_IOCTL_GET_ERR_STATUS: {
                uint8_t err = 0;
                uint32_t status = ctx->instance->STAS;
                err |= (status >> ICL_STATS_RX_OVERFLOW_Pos) & 1 ? 1 : 0;
                err |= (status >> ICL_STATS_RX_DMA_ERR_Pos) & 1 ? 2 : 0;
                err |= (status >> ICL_STATS_FS_ERR_Pos) & 1 ? 4 : 0;
                *(uint8_t *)para = err;
                break;
            }
            case OPEN_CSPI_IOCTL_START_RECV: {
                ctx->dma_cfg.targetAddress = (void *)para;
                cspi_receive(index);
                break;
            }
            case OPEN_CSPI_IOCTL_SET_CBCTRL: {
                ctx->instance->CBCTRL |= 2 << 25;
                break;
            }
            default:
                break;
        }
        ret = OPEN_HAL_DONE;
    }
    return ret;
}

/**
  \fn
  \brief    对设备功耗和模式进行配置
  \return
*/
api_ret_t api_cspi_pmctl(uint32_t usrId, open_hal_pm_t *cfg, size_t count)
{
    api_ret_t ret = api_cspi_query(usrId);
    // uint32_t index = usrId_to_cspi(usrId);
    if(ret == OPEN_HAL_USED)
    {
        if(cfg->runtime == RUNTIME_SUSPEND)
        {
            if(cfg->mode == PM_LOWPOW)
            {
            }
        }
        ret = OPEN_HAL_DONE;
    }
    return ret;
}

void cspi_dump_reg(int index)
{
    CspiCtx_t *ctx = get_cspi_ctx(index);
    printf("DFMT: 0x%x\r\n", ctx->instance->DFMT);
    printf("SLOTCTL: 0x%x\r\n", ctx->instance->SLOTCTL);
    printf("CLKCTL: 0x%x\r\n", ctx->instance->CLKCTL);
    printf("DMACTL: 0x%x\r\n", ctx->instance->DMACTL);
    printf("INTCTL: 0x%x\r\n", ctx->instance->INTCTL);
    printf("TIMEOUTCTL: 0x%x\r\n", ctx->instance->TIMEOUTCTL);
    printf("STAS: 0x%x\r\n", ctx->instance->STAS);
    printf("CSPICTL:0x%x\r\n", ctx->instance->CSPICTL);
    printf("CCTL: 0x%x\r\n", ctx->instance->CCTL);
    printf("CSPIINFO0: 0x%x\r\n", ctx->instance->CSPIINFO0);
    printf("CSPIINFO1: 0x%x\r\n", ctx->instance->CSPIINFO1);
    printf("CSPIDBG: 0x%x\r\n", ctx->instance->CSPIDBG);
    printf("CSPINIT: 0x%x\r\n", ctx->instance->CSPINIT);
    printf("CLSP: 0x%x\r\n", ctx->instance->CLSP);
    printf("CDATP:0x%x\r\n", ctx->instance->CDATP);
    printf("CLINFO: 0x%x\r\n", ctx->instance->CLINFO);
    printf("CBCTRL: 0x%x\r\n", ctx->instance->CBCTRL);
    printf("CSPIDLYCTRL:0x%x\r\n", ctx->instance->CSPIDLYCTRL);
    printf("CSPIPROCLSPI: 0x%x\r\n", ctx->instance->CSPIPROCLSPI);
    printf("CSPIQUARTILE: 0x%x\r\n", ctx->instance->CSPIQUARTILE);
    printf("CSPIYADJ:0x%x\r\n", ctx->instance->CSPIYADJ);
    printf("CSPIDLYCTRL: 0x%x\r\n", ctx->instance->CSPIDLYCTRL);
    printf("I2SBUSSEL: 0x%x\r\n", ctx->instance->I2SBUSSEL);
    printf("HISTOBUFCTRL: 0x%x\r\n", ctx->instance->HISTOBUFCTRL);
}

void api_cspi_dump_reg(uint32_t usrId)
{
    uint32_t index = usrId_to_cspi(usrId);
    cspi_dump_reg(index);
}
#endif
