/****************************************************************************
 *
 * Copy right:   2025-, Copyrigths of EigenComm Ltd.
 * File name:    api_lspi.c
 * Description:  ec7xx openhal lspi entry source file
 * History:      Rev1.0   2025-09-17
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include DEBUG_LOG_HEADER_FILE
#include "bsp.h"
#include "devicemanager.h"
#include "api_comm.h"
#include "api_lspi.h"
#include "slpman.h"

#ifdef EPAT_HAL_DEBUG
#define EPAT_LOG(subId, debugLevel, format, ...) \
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, subId, debugLevel, format, ##__VA_ARGS__)
#else
#define EPAT_LOG(subId, debugLevel, format, ...)
#endif

static uint32_t lspiUsrIdList[EC_LSPI_INDEX_LIMIT] = {0};
static uint16_t lspiUsrIdSeed[EC_LSPI_INDEX_LIMIT] = {0};

typedef struct
{
    bool isInited; /**< Whether spi has been initialized */
    struct
    {
#if((defined CHIP_EC718) && !(defined TYPE_EC718M)) || (defined CHIP_EC716)
        __IO uint32_t DFMT;
        __IO uint32_t RSVD1[2];
        __IO uint32_t DMACTL;
        __IO uint32_t INTCTL; /**< Interrupt Control Register,
                                 offset: 0x10 */
        __IO uint32_t RSVD2;
        __IO uint32_t STAS;  /**< Status Register,
                                offset: 0x18 */
        __IO uint32_t RFIFO; /**< RFIFO,
                                offset: 0x1C */
        __IO uint32_t TFIFO; /**< TFIFO,
                                offset: 0x20 */
        __IO uint32_t RSVD7;
        __IO uint32_t CSPICTL;
        __IO uint32_t RSVD4[13];
        __IO uint32_t LSPI_CTRL;     /**< LSPI control
                                        offset: 0x60 */
        __IO uint32_t LSPI_CCTRL;    /**< LSPI command control
                                        offset: 0x64 */
        __IO uint32_t LSPI_CADDR;    /**< LSPI command addr
                                        offset: 0x68 */
        __IO uint32_t LSPI_STAT;     /**< LSPI status
                                        offset: 0x6c */
        __IO uint32_t LSPI_RAMWLEN;  /**< LSPI ram write len
                                        offset: 0x70 */
        __IO uint32_t LSPFINFO;      /**< LSPI frame info
                                        offset: 0x74 */
        __IO uint32_t LSPTINFO0;     /**< LSPI tailor info0
                                        offset: 0x78 */
        __IO uint32_t LSPTINFO;      /**< LSPI tailor info
                                        offset: 0x7c */
        __IO uint32_t LSPSINFO;      /**< LSPI scale info
                                        offset: 0x80 */
        __IO uint32_t LSPIQUARTCTRL; /**< LSPI quartile ctrl
                                        offset: 0x84 */
        __IO uint32_t LSPIQUARTUSE;  /**< LSPI quartile inuse
                                        offset: 0x88 */
        __IO uint32_t LSPIYADJ;      /**< LSPI Y adj
                                        offset: 0x8c */
        __IO uint32_t LSPIYADJUSE;   /**< LSPI Y adj inuse
                                        offset: 0x90 */
        __IO uint32_t LSPIGPCMD0;    /**< LSPI gray page cmd0
                                        offset: 0x94 */
        __IO uint32_t LSPIGPCMD1;    /**< LSPI gray page cmd1
                                        offset: 0x98 */
        __IO uint32_t LSPFINFO0;     /**< LSPI frame info out
                                        offset: 0x9c */
        __IO uint32_t YUV2RGBINFO0;  /**< YUV to RGB info0
                                        offset: 0xa0 */
        __IO uint32_t YUV2RGBINFO1;  /**< YUV to RGB info1
                                        offset: 0xa4 */
        __IO uint32_t RSVD5[14];     /**< Reserved
                                      */
        __IO uint32_t I2SBUSSEL;     /**< LSPI bus select
                                        offset: 0xe0 */
        __IO uint32_t RSVD6[4];      /**< Reserved
                                      */
#else
        __IO uint32_t DFMT;
        __IO uint32_t RSVD1[2];
        __IO uint32_t DMACTL;
        __IO uint32_t INTCTL; /**< Interrupt Control Register,
                                 offset: 0x10 */
        __IO uint32_t RSVD2;
        __IO uint32_t STAS;  /**< Status Register,
                                offset: 0x18 */
        __IO uint32_t RFIFO; /**< RFIFO,
                                offset: 0x1C */
        __IO uint32_t TFIFO; /**< TFIFO,
                                offset: 0x20 */
        __IO uint32_t RSVD7;
        __IO uint32_t CSPICTL;
        __IO uint32_t RSVD4[13];
        __IO uint32_t LSPI_CTRL;        /**< LSPI control
                                           offset: 0x60 */
        __IO uint32_t LSPI_CCTRL;       /**< LSPI command control
                                           offset: 0x64 */
        __IO uint32_t LSPI_CADDR;       /**< LSPI command addr
                                           offset: 0x68 */
        __IO uint32_t LSPI_STAT;        /**< LSPI status
                                           offset: 0x6c */
        __IO uint32_t LSPI_RAMWLEN;     /**< LSPI ram write len
                                           offset: 0x70 */
        __IO uint32_t LSPFINFO;         /**< LSPI frame info
                                           offset: 0x74 */
        __IO uint32_t LSPTINFO0;        /**< LSPI tailor info0
                                           offset: 0x78 */
        __IO uint32_t LSPTINFO;         /**< LSPI tailor info
                                           offset: 0x7c */
        __IO uint32_t LSPSINFO;         /**< LSPI scale info
                                           offset: 0x80 */
        __IO uint32_t LSPIQUARTCTRL;    /**< LSPI quartile ctrl
                                           offset: 0x84 */
        __IO uint32_t LSPIQUARTUSE;     /**< LSPI quartile inuse
                                           offset: 0x88 */
        __IO uint32_t LSPIYADJ;         /**< LSPI Y adj
                                           offset: 0x8c */
        __IO uint32_t LSPIYADJUSE;      /**< LSPI Y adj inuse
                                           offset: 0x90 */
        __IO uint32_t LSPIGPCMD0;       /**< LSPI gray page cmd0
                                           offset: 0x94 */
        __IO uint32_t LSPIGPCMD1;       /**< LSPI gray page cmd1
                                           offset: 0x98 */
        __IO uint32_t LSPFINFO0;        /**< LSPI frame info out
                                           offset: 0x9c */
        __IO uint32_t YUV2RGBINFO0;     /**< YUV to RGB info0
                                           offset: 0xa0 */
        __IO uint32_t YUV2RGBINFO1;     /**< YUV to RGB info1
                                           offset: 0xa4 */
        __IO uint32_t DEBUG;            /**< lspi debug
                                           offset: 0xa8 */
        __IO uint32_t MSPI_CTRL;        /**< lspi mspi ctrl
                                           offset: 0xac */
        __IO uint32_t VSYNC_CTRL;       /**< lspi vsync ctrl
                                           offset: 0xb0 */
        __IO uint32_t LSPI8080CTRL;     /**< lspi 8080 ctrl
                                           offset: 0xb4 */
        __IO uint32_t LSPICMDPREPARA0;  /**< lspi pre cmd param0
                                           offset: 0xb8 */
        __IO uint32_t LSPICMDPREPARA1;  /**< lspi pre cmd param1
                                           offset: 0xbc */
        __IO uint32_t LSPICMDPREPARA2;  /**< lspi pre cmd param2
                                           offset: 0xc0 */
        __IO uint32_t LSPICMDPREPARA3;  /**< lspi pre cmd param3
                                           offset: 0xc4 */
        __IO uint32_t LSPICMDPOSTPARA0; /**< lspi post cmd param0
                                           offset: 0xc8 */
        __IO uint32_t LSPICMDPOSTPARA1; /**< lspi post cmd param1
                                           offset: 0xcc */
        __IO uint32_t LSPICMDPOSTPARA2; /**< lspi post cmd param2
                                           offset: 0xd0 */
        __IO uint32_t LSPICMDPOSTPARA3; /**< lspi post cmd param3
                                           offset: 0xd4 */
        __IO uint32_t RSVD5;            /**< Reserved
                                         */
        __IO uint32_t LSPITEPARA0;      /**< lspi te param0
                                           offset: 0xdc */
        __IO uint32_t LSPITEPARA1;      /**< lspi te param1
                                           offset: 0xe0 */
        __IO uint32_t I2SBUSSEL;        /**< LSPI bus select
                                           offset: 0xe4 */
        __IO uint32_t RSVD6[3];         /**< Reserved
                                         */
        __IO uint32_t USPVERSION;       /**< USP version
                                           offset: 0xf4 */
#endif
    } regsBackup;
} lspiDataBase_t;

typedef struct LspiCtx_
{
    LSPI_TypeDef *instance;
    uint32_t init_cnt;
    uint32_t work_state;
    lspiDataBase_t db;
    lspiDma_t dma;
    lspiRteInfo_t lspi_info;
    lspiRes_t lspi_res;
    LspiSleepCb sleep_cb;
    uint8_t clk_div;
    uint32_t bpsBak;
} LspiCtx_t;

LspiCtx_t *get_lspi_ctx(int index)
{
    static LspiCtx_t s_ctx[EC_LSPI_INDEX_LIMIT] = {0};
    return &s_ctx[index];
}

void lspi_ctx_reset(int index, LspiCtx_t *ctx)
{
    ctx->instance = LSPI2;
    ctx->init_cnt = 0;
    ctx->work_state = 0;
    ctx->clk_div = 1;
    memset(&ctx->db, 0, sizeof(lspiDataBase_t));
    memset(&ctx->lspi_info, 0, sizeof(lspiRteInfo_t));
    ctx->dma.txInstance = DMA_INSTANCE_MP;
    ctx->dma.txCh = 0;
    ctx->dma.txReq = DMA_REQUEST_USP2_TX;
    ctx->dma.txCb = NULL;
    ctx->dma.descriptor = NULL;
    ctx->lspi_res.info = &ctx->lspi_info;
    ctx->lspi_res.reg = ctx->instance;
}

static int32_t lspiSetBusSpeed(uint32_t bps, LspiCtx_t *ctx)
{
    ctx->clk_div = 1;
    ctx->bpsBak = bps;

    if(bps < 102 * 1024 * 1024)
    {
        ctx->clk_div = (uint8_t)((6 * 102 * 1024 * 1024) / bps);
    }

    if(bps < 1024 * 1024)
    {
        ctx->clk_div = 12;
    }
    return ARM_DRIVER_OK;
}

#ifdef PM_FEATURE_ENABLE
static void lspiEnterLpStatePrepare(void *pdata, slpManLpState state)
{
    uint32_t i;
    switch(state)
    {
        case SLPMAN_SLEEP1_STATE:

            for(i = 0; i < EC_LSPI_INDEX_LIMIT; i++)
            {
                LspiCtx_t *ctx = get_lspi_ctx(i);
                lspiDataBase_t *db = &ctx->db;
                if(db->isInited == true)
                {
#if((defined CHIP_EC718) && !(defined TYPE_EC718M)) || (defined CHIP_EC716)
                    db->regsBackup.DFMT = ctx->instance->DFMT;
                    db->regsBackup.DMACTL = ctx->instance->DMACTL;
                    db->regsBackup.INTCTL = ctx->instance->INTCTL;
                    db->regsBackup.STAS = ctx->instance->STAS;
                    db->regsBackup.CSPICTL = ctx->instance->CSPICTL;
                    db->regsBackup.LSPI_CTRL = ctx->instance->LSPI_CTRL;
                    db->regsBackup.LSPI_CCTRL = ctx->instance->LSPI_CCTRL;
                    db->regsBackup.LSPI_CADDR = ctx->instance->LSPI_CADDR;
                    db->regsBackup.LSPI_STAT = ctx->instance->LSPI_STAT;
                    db->regsBackup.LSPI_RAMWLEN = ctx->instance->LSPI_RAMWLEN;
                    db->regsBackup.LSPFINFO = ctx->instance->LSPFINFO;
                    db->regsBackup.LSPTINFO0 = ctx->instance->LSPTINFO0;
                    db->regsBackup.LSPTINFO = ctx->instance->LSPTINFO;
                    db->regsBackup.LSPSINFO = ctx->instance->LSPSINFO;
                    db->regsBackup.LSPIQUARTCTRL = ctx->instance->LSPIQUARTCTRL;
                    db->regsBackup.LSPIQUARTUSE = ctx->instance->LSPIQUARTUSE;
                    db->regsBackup.LSPIYADJ = ctx->instance->LSPIYADJ;
                    db->regsBackup.LSPIYADJUSE = ctx->instance->LSPIYADJUSE;
                    db->regsBackup.LSPIGPCMD0 = ctx->instance->LSPIGPCMD0;
                    db->regsBackup.LSPIGPCMD1 = ctx->instance->LSPIGPCMD1;
                    db->regsBackup.LSPFINFO0 = ctx->instance->LSPFINFO0;
                    db->regsBackup.YUV2RGBINFO0 = ctx->instance->YUV2RGBINFO0;
                    db->regsBackup.YUV2RGBINFO1 = ctx->instance->YUV2RGBINFO1;
                    db->regsBackup.I2SBUSSEL = ctx->instance->I2SBUSSEL;
#else  // chip 719
                    db->regsBackup.DFMT = ctx->instance->DFMT;
                    db->regsBackup.DMACTL = ctx->instance->DMACTL;
                    db->regsBackup.INTCTL = ctx->instance->INTCTL;
                    db->regsBackup.STAS = ctx->instance->STAS;
                    db->regsBackup.CSPICTL = ctx->instance->CSPICTL;
                    db->regsBackup.LSPI_CTRL = ctx->instance->LSPI_CTRL;
                    db->regsBackup.LSPI_CCTRL = ctx->instance->LSPI_CCTRL;
                    db->regsBackup.LSPI_CADDR = ctx->instance->LSPI_CADDR;
                    db->regsBackup.LSPI_STAT = ctx->instance->LSPI_STAT;
                    db->regsBackup.LSPI_RAMWLEN = ctx->instance->LSPI_RAMWLEN;
                    db->regsBackup.LSPFINFO = ctx->instance->LSPFINFO;
                    db->regsBackup.LSPTINFO0 = ctx->instance->LSPTINFO0;
                    db->regsBackup.LSPTINFO = ctx->instance->LSPTINFO;
                    db->regsBackup.LSPSINFO = ctx->instance->LSPSINFO;
                    db->regsBackup.LSPIQUARTCTRL = ctx->instance->LSPIQUARTCTRL;
                    db->regsBackup.LSPIQUARTUSE = ctx->instance->LSPIQUARTUSE;
                    db->regsBackup.LSPIYADJ = ctx->instance->LSPIYADJ;
                    db->regsBackup.LSPIYADJUSE = ctx->instance->LSPIYADJUSE;
                    db->regsBackup.LSPIGPCMD0 = ctx->instance->LSPIGPCMD0;
                    db->regsBackup.LSPIGPCMD1 = ctx->instance->LSPIGPCMD1;
                    db->regsBackup.LSPFINFO0 = ctx->instance->LSPFINFO0;
                    db->regsBackup.YUV2RGBINFO0 = ctx->instance->YUV2RGBINFO0;
                    db->regsBackup.YUV2RGBINFO1 = ctx->instance->YUV2RGBINFO1;
                    db->regsBackup.DEBUG = ctx->instance->DEBUG;
                    db->regsBackup.MSPI_CTRL = ctx->instance->MSPI_CTRL;
                    db->regsBackup.VSYNC_CTRL = ctx->instance->VSYNC_CTRL;
                    db->regsBackup.LSPI8080CTRL = ctx->instance->LSPI8080CTRL;
                    db->regsBackup.LSPICMDPREPARA0 =
                        ctx->instance->LSPICMDPREPARA0;
                    db->regsBackup.LSPICMDPREPARA1 =
                        ctx->instance->LSPICMDPREPARA1;
                    db->regsBackup.LSPICMDPREPARA2 =
                        ctx->instance->LSPICMDPREPARA2;
                    db->regsBackup.LSPICMDPREPARA3 =
                        ctx->instance->LSPICMDPREPARA3;
                    db->regsBackup.LSPICMDPOSTPARA0 =
                        ctx->instance->LSPICMDPOSTPARA0;
                    db->regsBackup.LSPICMDPOSTPARA1 =
                        ctx->instance->LSPICMDPOSTPARA1;
                    db->regsBackup.LSPICMDPOSTPARA2 =
                        ctx->instance->LSPICMDPOSTPARA2;
                    db->regsBackup.LSPICMDPOSTPARA3 =
                        ctx->instance->LSPICMDPOSTPARA3;
                    db->regsBackup.LSPITEPARA0 = ctx->instance->LSPITEPARA0;
                    db->regsBackup.LSPITEPARA1 = ctx->instance->LSPITEPARA1;
                    db->regsBackup.I2SBUSSEL = ctx->instance->I2SBUSSEL;
                    db->regsBackup.USPVERSION = ctx->instance->USPVERSION;
#endif
                }
            }

            break;
        default:
            break;
    }
}

static void lspiExitLpStateRestore(void *pdata, slpManLpState state)
{
    uint32_t i;
    switch(state)
    {
        case SLPMAN_SLEEP1_STATE:

            for(i = 0; i < EC_LSPI_INDEX_LIMIT; i++)
            {
                LspiCtx_t *ctx = get_lspi_ctx(i);
                lspiDataBase_t *db = &ctx->db;
                if(db->isInited == true)
                {
                    GPR_clockEnable(PCLK_USP2);
                    GPR_clockEnable(FCLK_USP2);
                    lspiSetBusSpeed(ctx->bpsBak, ctx);

#if((defined CHIP_EC718) && !(defined TYPE_EC718M)) || (defined CHIP_EC716)
                    ctx->instance->DFMT = db->regsBackup.DFMT;
                    ctx->instance->DMACTL = db->regsBackup.DMACTL;
                    ctx->instance->INTCTL = db->regsBackup.INTCTL;
                    ctx->instance->STAS = db->regsBackup.STAS;
                    ctx->instance->CSPICTL = db->regsBackup.CSPICTL;
                    ctx->instance->LSPI_CTRL = db->regsBackup.LSPI_CTRL;
                    ctx->instance->LSPI_CCTRL = db->regsBackup.LSPI_CCTRL;
                    ctx->instance->LSPI_CADDR = db->regsBackup.LSPI_CADDR;
                    ctx->instance->LSPI_STAT = db->regsBackup.LSPI_STAT;
                    ctx->instance->LSPI_RAMWLEN = db->regsBackup.LSPI_RAMWLEN;
                    ctx->instance->LSPFINFO = db->regsBackup.LSPFINFO;
                    ctx->instance->LSPTINFO0 = db->regsBackup.LSPTINFO0;
                    ctx->instance->LSPTINFO = db->regsBackup.LSPTINFO;
                    ctx->instance->LSPSINFO = db->regsBackup.LSPSINFO;
                    ctx->instance->LSPIQUARTCTRL = db->regsBackup.LSPIQUARTCTRL;
                    ctx->instance->LSPIQUARTUSE = db->regsBackup.LSPIQUARTUSE;
                    ctx->instance->LSPIYADJ = db->regsBackup.LSPIYADJ;
                    ctx->instance->LSPIYADJUSE = db->regsBackup.LSPIYADJUSE;
                    ctx->instance->LSPIGPCMD0 = db->regsBackup.LSPIGPCMD0;
                    ctx->instance->LSPIGPCMD1 = db->regsBackup.LSPIGPCMD1;
                    ctx->instance->LSPFINFO0 = db->regsBackup.LSPFINFO0;
                    ctx->instance->YUV2RGBINFO0 = db->regsBackup.YUV2RGBINFO0;
                    ctx->instance->YUV2RGBINFO1 = db->regsBackup.YUV2RGBINFO1;
                    ctx->instance->I2SBUSSEL = db->regsBackup.I2SBUSSEL;
#else  // chip 719
                    ctx->instance->DFMT = db->regsBackup.DFMT;
                    ctx->instance->DMACTL = db->regsBackup.DMACTL;
                    ctx->instance->INTCTL = db->regsBackup.INTCTL;
                    ctx->instance->STAS = db->regsBackup.STAS;
                    ctx->instance->CSPICTL = db->regsBackup.CSPICTL;
                    ctx->instance->LSPI_CTRL = db->regsBackup.LSPI_CTRL;
                    ctx->instance->LSPI_CCTRL = db->regsBackup.LSPI_CCTRL;
                    ctx->instance->LSPI_CADDR = db->regsBackup.LSPI_CADDR;
                    ctx->instance->LSPI_STAT = db->regsBackup.LSPI_STAT;
                    ctx->instance->LSPI_RAMWLEN = db->regsBackup.LSPI_RAMWLEN;
                    ctx->instance->LSPFINFO = db->regsBackup.LSPFINFO;
                    ctx->instance->LSPTINFO0 = db->regsBackup.LSPTINFO0;
                    ctx->instance->LSPTINFO = db->regsBackup.LSPTINFO;
                    ctx->instance->LSPSINFO = db->regsBackup.LSPSINFO;
                    ctx->instance->LSPIQUARTCTRL = db->regsBackup.LSPIQUARTCTRL;
                    ctx->instance->LSPIQUARTUSE = db->regsBackup.LSPIQUARTUSE;
                    ctx->instance->LSPIYADJ = db->regsBackup.LSPIYADJ;
                    ctx->instance->LSPIYADJUSE = db->regsBackup.LSPIYADJUSE;
                    ctx->instance->LSPIGPCMD0 = db->regsBackup.LSPIGPCMD0;
                    ctx->instance->LSPIGPCMD1 = db->regsBackup.LSPIGPCMD1;
                    ctx->instance->LSPFINFO0 = db->regsBackup.LSPFINFO0;
                    ctx->instance->YUV2RGBINFO0 = db->regsBackup.YUV2RGBINFO0;
                    ctx->instance->YUV2RGBINFO1 = db->regsBackup.YUV2RGBINFO1;
                    ctx->instance->DEBUG = db->regsBackup.DEBUG;
                    ctx->instance->MSPI_CTRL = db->regsBackup.MSPI_CTRL;
                    ctx->instance->VSYNC_CTRL = db->regsBackup.VSYNC_CTRL;
                    ctx->instance->LSPI8080CTRL = db->regsBackup.LSPI8080CTRL;
                    ctx->instance->LSPICMDPREPARA0 =
                        db->regsBackup.LSPICMDPREPARA0;
                    ctx->instance->LSPICMDPREPARA1 =
                        db->regsBackup.LSPICMDPREPARA1;
                    ctx->instance->LSPICMDPREPARA2 =
                        db->regsBackup.LSPICMDPREPARA2;
                    ctx->instance->LSPICMDPREPARA3 =
                        db->regsBackup.LSPICMDPREPARA3;
                    ctx->instance->LSPICMDPOSTPARA0 =
                        db->regsBackup.LSPICMDPOSTPARA0;
                    ctx->instance->LSPICMDPOSTPARA1 =
                        db->regsBackup.LSPICMDPOSTPARA1;
                    ctx->instance->LSPICMDPOSTPARA2 =
                        db->regsBackup.LSPICMDPOSTPARA2;
                    ctx->instance->LSPICMDPOSTPARA3 =
                        db->regsBackup.LSPICMDPOSTPARA3;
                    ctx->instance->LSPITEPARA0 = db->regsBackup.LSPITEPARA0;
                    ctx->instance->LSPITEPARA1 = db->regsBackup.LSPITEPARA1;
                    ctx->instance->I2SBUSSEL = db->regsBackup.I2SBUSSEL;
                    ctx->instance->USPVERSION = db->regsBackup.USPVERSION;
#endif
                }
                if(ctx->sleep_cb)
                {
                    ctx->sleep_cb();
                }
            }

            break;

        default:
            break;
    }
}

static int32_t lspi_open(int index)
{
    int32_t returnCode;
    LspiCtx_t *ctx = get_lspi_ctx(index);

#ifdef PM_FEATURE_ENABLE
    ctx->db.isInited = true;
    apmuVoteToDozeState(PMU_DOZE_USP_MOD, false);
#endif
    // Configure DMA if necessary
    if(ctx->lspi_res.dma)
    {
        DMA_init(ctx->lspi_res.dma->txInstance);
        returnCode = DMA_openChannel(ctx->lspi_res.dma->txInstance);

        if(returnCode == ARM_DMA_ERROR_CHANNEL_ALLOC)
        {
            EC_ASSERT(0, 0, 0, 0);
        }

        ctx->lspi_res.dma->txCh = returnCode;

        DMA_setChannelRequestSource(
            ctx->lspi_res.dma->txInstance, ctx->lspi_res.dma->txCh,
            (DmaRequestSource_e)ctx->lspi_res.dma->txReq);
        DMA_rigisterChannelCallback(ctx->lspi_res.dma->txInstance,
                                    ctx->lspi_res.dma->txCh,
                                    ctx->lspi_res.dma->txCb);
    }

#ifdef PM_FEATURE_ENABLE
    ctx->init_cnt++;

    if(ctx->init_cnt == 1U)
    {
        ctx->work_state = 0;
        slpManRegisterPredefinedBackupCb(SLP_CALLBACK_I2S_MODULE,
                                         lspiEnterLpStatePrepare, NULL);
        slpManRegisterPredefinedRestoreCb(SLP_CALLBACK_I2S_MODULE,
                                          lspiExitLpStateRestore, NULL);
    }
#endif
    return ARM_DRIVER_OK;
}

int32_t lspi_close(int index)
{
    LspiCtx_t *ctx = get_lspi_ctx(index);
#ifdef PM_FEATURE_ENABLE
    ctx->db.isInited = false;
    ctx->init_cnt--;
    if(ctx->init_cnt == 0)
    {
        ctx->work_state = 0;
    }
    apmuVoteToDozeState(PMU_DOZE_USP_MOD, true);
#endif
    return 0;
}

int32_t lspi_power_ctrl(int index, lspiPowerState_e state)
{
    LspiCtx_t *ctx = get_lspi_ctx(index);
    lspiRes_t *lspi = &ctx->lspi_res;
    switch(state)
    {
        case LSPI_POWER_OFF: {
            if(lspi->dma)
            {
                DMA_stopChannel(lspi->dma->txInstance, lspi->dma->txCh, true);
            }

            CLOCK_clockDisable(PCLK_USP2);
            CLOCK_clockDisable(FCLK_USP2);
            CLOCK_setClockSrc(FCLK_USP2, FCLK_USP2_SEL_26M);
            CLOCK_setClockSrc(CLK_APB_MP, CLK_APB_MP_SEL_26M);

            CLOCK_clockDisable(CLK_HF102M);
            break;
        }
        case LSPI_POWER_FULL: {
            CLOCK_setClockSrc(CLK_APB_MP, CLK_APB_MP_SEL_102M);
            CLOCK_clockEnable(CLK_HF102M);

#if(defined CHIP_EC718) && !(defined TYPE_EC718M) || (defined CHIP_EC716)
            CLOCK_setClockSrc(FCLK_USP2, FCLK_USP2_SEL_102M);
            CLOCK_setClockDiv(FCLK_USP2, ctx->clk_div / 6);
#else
            CLOCK_setClockSrc(FCLK_USP2, FCLK_USP2_SEL_612M);
            CLOCK_setClockDiv(FCLK_USP2, ctx->clk_div);
#endif

            CLOCK_clockEnable(PCLK_USP2);
            CLOCK_clockEnable(FCLK_USP2);
            break;
        }
        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
    return ARM_DRIVER_OK;
}

int32_t lspi_send(int index)
{
    LspiCtx_t *ctx = get_lspi_ctx(index);
    lspiRes_t *lspi = &ctx->lspi_res;
    uint8_t *dataListIndex = &(lspi->info->prePareSendInfo.dataListIndex);
    uint8_t *dataLen = &(lspi->info->prePareSendInfo.dataLen);
    uint32_t *tmp = &(lspi->info->prePareSendInfo.tmp);
    uint32_t *dataList = lspi->info->prePareSendInfo.dataList;
    // Collect the remainder bytes which is less then 4byte
    if(*dataLen % 4 != 0)
    {
        dataList[*dataListIndex] = *tmp;
        *dataListIndex += 1;
    }

    for(int i = 0; i < *dataListIndex; i++)
    {
        lspi->reg->TFIFO = dataList[i];
    }

    lspi->reg->LSPI_CCTRL = 1 | *dataLen << 8;

    // wait until finish
    while(!(lspi->reg->LSPI_STAT & 0x1));

    memset(dataList, 0, *dataListIndex * 4);
    lspi->info->prePareSendInfo.dataListIndex = 0;
    lspi->info->prePareSendInfo.tmp = 0;
    lspi->info->prePareSendInfo.trans = 0;
    lspi->info->prePareSendInfo.dataLen = 0;

    return ARM_DRIVER_OK;
}

void lspi_push_send_data(int index, uint8_t data)
{
    LspiCtx_t *ctx = get_lspi_ctx(index);
    lspiRes_t *lspi = &ctx->lspi_res;
    uint8_t *dataListIndex = &(lspi->info->prePareSendInfo.dataListIndex);
    uint8_t *trans = &(lspi->info->prePareSendInfo.trans);
    uint8_t *dataLen = &(lspi->info->prePareSendInfo.dataLen);
    uint32_t *tmp = &(lspi->info->prePareSendInfo.tmp);
    uint32_t *dataList = lspi->info->prePareSendInfo.dataList;

    *tmp |= data << *trans;
    *dataLen += 1;
    *trans += 8;

    // Round up to 4bytes, then store it into the array of dataList
    if(*trans == 32)
    {
        dataList[*dataListIndex] = *tmp;
        *dataListIndex += 1;
        *trans = 0;
        *tmp = 0;
    }
}

static uint32_t usrId_to_lspi(uint32_t usrId)
{
    uint32_t lspi = (uint32_t)(usrId & OPEN_HAL_PORT_MUSK);
    ASSERT(lspi < EC_LSPI_INDEX_LIMIT);
    if(lspi == (lspiUsrIdList[lspi] & OPEN_HAL_PORT_MUSK))
    {
        return lspi;
    }
    return EC_LSPI_INDEX_LIMIT;
}

api_ret_t api_lspi_query(uint32_t usrId)
{
    uint32_t index = usrId_to_lspi(usrId);
    if(index >= EC_LSPI_INDEX_START && index < EC_LSPI_INDEX_LIMIT)
    {
        if(lspiUsrIdList[index] == OPEN_HAL_STAT_UNUSED)
        {
            return OPEN_HAL_FREE;
        }
        else if(lspiUsrIdList[index] & OPEN_HAL_STAT_MUSK)
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

static uint32_t lspi_set_free(uint32_t index)
{
    if(index >= EC_LSPI_INDEX_START && index < EC_LSPI_INDEX_LIMIT)
    {
        lspiUsrIdList[index] = OPEN_HAL_STAT_UNUSED;
        return 1;
    }
    return 0;
}

static uint32_t lspi_set_idle(uint32_t index)
{
    if(lspiUsrIdList[index] == OPEN_HAL_STAT_UNUSED)
    {
        lspiUsrIdList[index] = index;
        lspiUsrIdSeed[index] = 1;
        lspiUsrIdList[index] |= (uint32_t)(lspiUsrIdSeed[index] << 16);
    }
    else if(lspiUsrIdList[index] & OPEN_HAL_STAT_MUSK)
    {
        lspiUsrIdList[index] &= ~(OPEN_HAL_STAT_MUSK);
    }
    return lspiUsrIdList[index];
}

static uint32_t lspi_set_used(uint32_t usrId)
{
    api_ret_t stat = api_lspi_query(usrId);
    if(stat != OPEN_HAL_IDLE)
    {
        return 0;
    }
    uint32_t index = usrId_to_lspi(usrId);
    lspiUsrIdList[index] |= OPEN_HAL_STAT_MUSK;
    return 0;
}

api_ret_t api_lspi_setup(int8_t index, LspiConfig_t *para)
{
    if(para == NULL)
    {
    }
    else if(index >= EC_LSPI_INDEX_START && index < EC_LSPI_INDEX_LIMIT)
    {
        // TODO: 根据para配置LSPI硬件
        EPAT_LOG(api_lspi_setup, P_INFO, "lspi %d,Config %x", index, *para);
        return OPEN_HAL_DONE;
    }
    return OPEN_HAL_INVALID_PARA;
}

uint32_t api_lspi_create(uint32_t index, LspiConfig_t *cfg)
{
    uint32_t usrId = lspi_set_idle(index);
    LspiCtx_t *ctx = get_lspi_ctx(index);
    if(usrId)
    {
        if(cfg != NULL)
        {
            lspiSetBusSpeed(cfg->freq, ctx);
        }
    }
    EPAT_LOG(api_lspi_create, P_INFO, "lspi(%d): get user id %x, txReq: %d",
             index, usrId, ctx->dma.txReq);
    return usrId;
}

api_ret_t api_lspi_delete(uint32_t usrId)
{
    api_ret_t ret = api_lspi_query(usrId);
    uint32_t index = usrId_to_lspi(usrId);
    LspiCtx_t *ctx = get_lspi_ctx(index);
    if(ret == OPEN_HAL_IDLE)
    {
        if(lspi_set_free(index))
        {
            lspi_ctx_reset(index, ctx);
        }
        ret = OPEN_HAL_DONE;
    }
    EPAT_LOG(api_cspi_delete, P_INFO, "usrid: %x, index: %d, ret: %d", usrId,
             index, ret);
    return ret;
}

api_ret_t api_lspi_open(uint32_t usrId, LspiConfig_t *cfg, size_t timeout)
{
    api_ret_t ret = api_lspi_query(usrId);
    uint32_t index = usrId_to_lspi(usrId);
    if(ret == OPEN_HAL_IDLE)
    {
        LspiCtx_t *ctx = get_lspi_ctx(index);
        if(cfg != NULL)
        {
            lspiSetBusSpeed(cfg->freq, ctx);
        }
        if(ARM_DRIVER_OK != lspi_open(index))
        {
            return OPEN_HAL_FREE;
        }
        // lspi power on
        lspi_power_ctrl(index, LSPI_POWER_FULL);
        lspi_set_used(usrId);
        ret = OPEN_HAL_DONE;
    }
    EPAT_LOG(api_lspi_open, P_INFO, "usrid: %x, index: %d, ret: %d", usrId,
             index, ret);
    return ret;
}

int32_t api_lspi_startup(void *para)
{
    for(int i = EC_LSPI_INDEX_START; i < EC_LSPI_INDEX_LIMIT; i++)
    {
        lspi_ctx_reset(i, get_lspi_ctx(i));
        lspi_set_free(i);
    }
    EPAT_LOG(api_lspi_startup, P_INFO, "start up");
    return 0;
}

api_ret_t api_lspi_close(uint32_t usrId)
{
    api_ret_t ret = api_lspi_query(usrId);
    uint32_t index = usrId_to_lspi(usrId);
    if(ret == OPEN_HAL_USED)
    {
        lspi_set_idle(index);
        lspi_power_ctrl(index, LSPI_POWER_OFF);
        lspi_close(index);
        ret = OPEN_HAL_DONE;
    }
    EPAT_LOG(api_lspi_close, P_INFO, "usrid: %x, index: %d, ret: %d", usrId,
             index, ret);
    return ret;
}

api_ret_t api_lspi_ioctl(uint32_t usrId, LspiIoCtrl_t type, void *para)
{
    api_ret_t ret = api_lspi_query(usrId);
    uint32_t index = usrId_to_lspi(usrId);
    LspiCtx_t *ctx = get_lspi_ctx(index);
    if(ret == OPEN_HAL_USED)
    {
        switch(type)
        {
            case OPEN_LSPI_IOCTL_ISR: {
                break;
            }
            case OPEN_LSPI_IOCTL_DATA_FORMAT: {
                lspiDataFmt_t *data_fmt = (lspiDataFmt_t *)para;
                ctx->instance->DFMT = *(uint32_t *)data_fmt;
                break;
            }
            case OPEN_LSPI_IOCTL_BUS_SPEED: {
                uint32_t speed = *(uint32_t *)para;
                lspiSetBusSpeed(speed, ctx);
                break;
            }
            case OPEN_LSPI_IOCTL_DMA_CTRL: {
                lspiDmaCtrl_t *dma_ctrl = (lspiDmaCtrl_t *)para;
                ctx->instance->DMACTL = *(uint32_t *)dma_ctrl;
                break;
            }
            case OPEN_LSPI_IOCTL_INT_CTRL: {
                lspiIntCtrl_t *int_ctrl = (lspiIntCtrl_t *)para;
                ctx->instance->INTCTL = *(uint32_t *)int_ctrl;
                break;
            }
            case OPEN_LSPI_IOCTL_CTRL: {
                lspiCtrl_t *ctrl = (lspiCtrl_t *)para;
                ctx->instance->LSPI_CTRL = *(uint32_t *)ctrl;
                break;
            }
            case OPEN_LSPI_IOCTL_CMD_CTRL: {
                lspiCmdCtrl_t *cmd_ctrl = (lspiCmdCtrl_t *)para;
                ctx->instance->LSPI_CCTRL = *(uint32_t *)cmd_ctrl;
                break;
            }
            case OPEN_LSPI_IOCTL_YUV2RGB_INFO0: {
                ctx->instance->YUV2RGBINFO0 =
                    0x199 << 18 | 0x12a << 8 | 0x10 << 0;
                break;
            }
            case OPEN_LSPI_IOCTL_YUV2RGB_INFO1: {
                ctx->instance->YUV2RGBINFO1 =
                    0x204 << 20 | 0x64 << 10 | 0xd0 << 0;
                break;
            }
            case OPEN_LSPI_IOCTL_CMD_ADDR: {
                lspiCmdAddr_t *cmd_addr = (lspiCmdAddr_t *)para;
                ctx->instance->LSPI_CADDR = *(uint32_t *)cmd_addr;
                break;
            }
            case OPEN_LSPI_IOCTL_FRAME_INFO: {
                lspiInfo_t *frame_info = (lspiInfo_t *)para;
                ctx->instance->LSPFINFO = *(uint32_t *)frame_info;
                break;
            }
            case OPEN_LSPI_IOCTL_TAILOR_INFO0: {
                lspiTailorInfo0_t *tailor_info = (lspiTailorInfo0_t *)para;
                ctx->instance->LSPTINFO0 = *(uint32_t *)tailor_info;
                break;
            }
            case OPEN_LSPI_IOCTL_TAILOR_INFO1: {
                lspiTailorInfo_t *tailor_info = (lspiTailorInfo_t *)para;
                ctx->instance->LSPTINFO = *(uint32_t *)tailor_info;
                break;
            }
            case OPEN_LSPI_IOCTL_SCALE_INFO: {
                lspiScaleInfo_t *scale_info = (lspiScaleInfo_t *)para;
                ctx->instance->LSPSINFO = *(uint32_t *)scale_info;
                break;
            }
            case OPEN_LSPI_IOCTL_QUARTILE_CTRL: {
                lspiQuartileCtrl_t *quartile_ctrl = (lspiQuartileCtrl_t *)para;
                ctx->instance->LSPIQUARTCTRL = *(uint32_t *)quartile_ctrl;
                break;
            }
            case OPEN_LSPI_IOCTL_YADJ: {
                lspiYAdj_t *y_adj = (lspiYAdj_t *)para;
                ctx->instance->LSPIYADJ = *(uint32_t *)y_adj;
                break;
            }
            case OPEN_LSPI_IOCTL_GRAY_PAGE_CMD0: {
                lspiGrayPageCmd0_t *gray_page_cmd0 = (lspiGrayPageCmd0_t *)para;
                ctx->instance->LSPIGPCMD0 = *(uint32_t *)gray_page_cmd0;
                break;
            }
            case OPEN_LSPI_IOCTL_GRAY_PAGE_CMD1: {
                lspiGrayPageCmd1_t *gray_page_cmd1 = (lspiGrayPageCmd1_t *)para;
                ctx->instance->LSPIGPCMD1 = *(uint32_t *)gray_page_cmd1;
                break;
            }
            case OPEN_LSPI_IOCTL_FRAME_INFO_OUT: {
                lspiFrameInfoOut_t *frame_info_out = (lspiFrameInfoOut_t *)para;
                ctx->instance->LSPFINFO0 = *(uint32_t *)frame_info_out;
                break;
            }
            case OPEN_LSPI_IOCTL_BUS_SEL: {
                uint8_t i2c_bus = *(uint8_t *)para;
                ctx->instance->I2SBUSSEL = i2c_bus;
                break;
            }
#if(defined TYPE_EC718M)
            case OPEN_LSPI_IOCTL_MSPI_CTRL: {
                lspiMspiCtrl_t *mspi_ctrl = (lspiMspiCtrl_t *)para;
                ctx->instance->MSPI_CTRL = *(uint32_t *)mspi_ctrl;
                break;
            }
            case OPEN_LSPI_IOCTL_VSYNC_CTRL: {
                lspiVsyncCtrl_t *vsync_ctrl = (lspiVsyncCtrl_t *)para;
                ctx->instance->VSYNC_CTRL = *(uint32_t *)vsync_ctrl;
                break;
            }
            case OPEN_LSPI_IOCTL_8080_CTRL: {
                lspi8080Ctrl_t *spi_8080 = (lspi8080Ctrl_t *)para;
                ctx->instance->LSPI8080CTRL = *(uint32_t *)spi_8080;
                break;
            }
            case OPEN_LSPI_IOCTL_PRE_PARA0_CTRL: {
                lspiCmdPreParam0_t *pre_para0_ctrl = (lspiCmdPreParam0_t *)para;
                ctx->instance->LSPICMDPREPARA0 = *(uint32_t *)pre_para0_ctrl;
                break;
            }
            case OPEN_LSPI_IOCTL_PRE_PARA2_CTRL: {
                lspiCmdPreParam2_t *pre_para2_ctrl = (lspiCmdPreParam2_t *)para;
                ctx->instance->LSPICMDPREPARA2 = *(uint32_t *)pre_para2_ctrl;
                break;
            }
            case OPEN_LSPI_IOCTL_PRE_PARA3_CTRL: {
                lspiCmdPreParam3_t *pre_para3_ctrl = (lspiCmdPreParam3_t *)para;
                ctx->instance->LSPICMDPREPARA3 = *(uint32_t *)pre_para3_ctrl;
                break;
            }
            case OPEN_LSPI_IOCTL_POST_PARA0_CTRL: {
                lspiCmdPostParam0_t *post_para0 = (lspiCmdPostParam0_t *)para;
                ctx->instance->LSPICMDPOSTPARA0 = *(uint32_t *)post_para0;
                break;
            }
            case OPEN_LSPI_IOCTL_TE_CTRL0: {
                lspiTeParam0_t *te_param0 = (lspiTeParam0_t *)para;
                ctx->instance->LSPITEPARA0 = *(uint32_t *)te_param0;
                break;
            }
            case OPEN_LSPI_IOCTL_TE_CTRL1: {
                lspiTeParam1_t *te_param1 = (lspiTeParam1_t *)para;
                ctx->instance->LSPITEPARA1 = *(uint32_t *)te_param1;
                break;
            }
            case OPEN_LSPI_IOCTL_CLEAR_WREND: {
                ctx->instance->STAS |= (1 << 31);
                break;
            }
            case OPEN_LSPI_IOCTL_RST_CLERA_FIFO: {
                lspiCtrl_t ctrl = {0};
                memcpy(&ctrl, (void *)&ctx->instance->LSPI_CTRL,
                       sizeof(lspiCtrl_t));
                ctrl.enable = 0;
                memcpy((void *)&ctx->instance->LSPI_CTRL, &ctrl,
                       sizeof(uint32_t));
                GPR_swReset(RST_FCLK_USP2);
                ctx->instance->DMACTL |= 1 << 25;
                ctx->instance->STAS |= 1;
                ctrl.enable = 1;
                memcpy((void *)&ctx->instance->LSPI_CTRL, &ctrl,
                       sizeof(uint32_t));
                break;
            }
            case OPEN_LSPI_IOCTL_GET_FIFO_ADDR: {
                uint32_t *fifo_addr = (uint32_t *)para;
                *fifo_addr = (uint32_t) & (ctx->instance->TFIFO);
                break;
            }
#endif
            case OPEN_LSPI_IOCTL_CLEAN_PREVIEW_STATE: {
                if(((ctx->instance->STAS >> 27) & 0x1) > 0)
                {
                    ctx->instance->STAS |= 1 << 27;
                }
                if(((ctx->instance->STAS >> 28) & 0x1) > 0)
                {
                    ctx->instance->STAS |= 1 << 28;
                }
                if(((ctx->instance->STAS >> 13) & 0x3f) > 0)
                {
                    ctx->instance->DMACTL |= 1 << 25;
                }
                ctx->instance->DMACTL |= 1 << 24;
                if(((ctx->instance->STAS >> 13) & 0x3f) > 0)
                {
                    ctx->instance->DMACTL |= 1 << 25;
                }
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
api_ret_t api_lspi_pmctl(uint32_t usrId, open_hal_pm_t *cfg, size_t count)
{
    api_ret_t ret = api_lspi_query(usrId);
    // uint32_t index = usrId_to_lspi(usrId);
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

api_ret_t api_lspi_write(uint32_t usrId, void *buf, size_t count)
{
    api_ret_t ret = api_lspi_query(usrId);
    uint32_t index = usrId_to_lspi(usrId);
    if(ret == OPEN_HAL_USED)
    {
        LspiCtx_t *ctx = get_lspi_ctx(index);
        WriteParam_t *param = (WriteParam_t *)buf;
        lspiCmdAddr_t cmd_addr = {0};

        if(param->type == TYPE_CMD)
        {
            cmd_addr.addr = param->cmd;
            memcpy((void *)&ctx->instance->LSPI_CADDR, &cmd_addr,
                   sizeof(lspiCmdAddr_t));
            if(param->len > 0)
            {
                for(uint8_t i = 0; i < param->len; i++)
                {
                    lspi_push_send_data(index, param->data[i]);
                }
            }
            lspi_send(index);
        }
        else
        {
            lspiDmaCtrl_t dma_ctrl = {0};
            memcpy(&dma_ctrl, (void *)&ctx->instance->DMACTL,
                   sizeof(lspiDmaCtrl_t));
            dma_ctrl.txDmaReqEn = 0;
            memcpy((void *)&ctx->instance->DMACTL, &dma_ctrl,
                   sizeof(lspiDmaCtrl_t));
            lspiCmdCtrl_t cmd_ctrl = {0};
            memcpy(&cmd_ctrl, (void *)&ctx->instance->LSPI_CCTRL,
                   sizeof(lspiCmdCtrl_t));
            cmd_ctrl.wrRdn = 1;
            cmd_ctrl.ramWr = 1;
            cmd_ctrl.dataLen = 2;
            memcpy((void *)&ctx->instance->LSPI_CCTRL, &cmd_ctrl,
                   sizeof(lspiCmdCtrl_t));
            ctx->instance->STAS |= (1 << 31);

            lspiDataFmt_t data_fmt = {0};
            memcpy(&data_fmt, (void *)&ctx->instance->DFMT,
                   sizeof(lspiDataFmt_t));
            uint32_t data = 0;
            memcpy(&data, param->data, sizeof(uint32_t));
            if(data_fmt.txFifoEndianMode)
            {
                ctx->instance->TFIFO = (data >> 16) | (data << 16);
            }
            else
            {
                ctx->instance->TFIFO = data;
            }
        }
        ret = OPEN_HAL_DONE;
    }
    return ret;
}

api_ret_t api_lspi_read(uint32_t usrId, void *buf, size_t count)
{
    api_ret_t ret = api_lspi_query(usrId);
    // uint32_t index = usrId_to_lspi(usrId);
    if(ret == OPEN_HAL_USED)
    {
        if(buf != NULL)
        {
        }
        ret = OPEN_HAL_DONE;
    }
    return ret;
}
#endif

int api_test_lspi(void) { return 0; }
