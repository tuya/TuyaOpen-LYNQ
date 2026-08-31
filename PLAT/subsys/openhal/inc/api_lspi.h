/****************************************************************************
 *
 * Copy right:   2025-, Copyrigths of EigenComm Ltd.
 * File name:    api_lspi.h
 * Description:  openhal lspi entry header file
 * History:      Rev1.0   2025-09-17
 *
 ****************************************************************************/
#ifndef _API_LSPI_H_
#define _API_LSPI_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "api_def.h"
#include "dma.h"
/*只支持一个LSPI，底层使用USP2 IP*/
#define EC_LSPI_INDEX_START (0)
#define EC_LSPI_INDEX_LIMIT (1)

typedef void (*LspiSleepCb)(void);

typedef struct
{
    DmaInstance_e txInstance;      // Transmit DMA instance number
    int8_t txCh;                   // Transmit channel number
    uint8_t txReq;                 // Transmit DMA request number
    void (*txCb)(uint32_t event);  // Transmit callback
    DmaDescriptor_t *descriptor;   // Tx descriptor
} lspiDma_t;

typedef struct
{
    uint8_t dataListIndex;
    uint8_t trans;
    uint8_t dataLen;
    uint32_t tmp;
    uint32_t dataList[16];
} lspiPrepareSendInfo_t;

// LSPI information (Run-time)
typedef struct
{
    uint32_t busSpeed;                      // LSPI bus speed
    lspiPrepareSendInfo_t prePareSendInfo;  // Prepare Send info
} lspiRteInfo_t;

typedef struct
{
    LSPI_TypeDef *reg;    // SPI register pointer
    lspiDma_t *dma;       // SPI DMA configuration pointer
    lspiRteInfo_t *info;  // Run-Time Information
} lspiRes_t;

/**
\brief General power states
*/
typedef enum
{
    LSPI_POWER_OFF,  // Power off: no operation possible
    LSPI_POWER_FULL  // Power on: full operation at maximum performance
} lspiPowerState_e;

#if((defined CHIP_EC718) && !(defined TYPE_EC718M)) || (defined CHIP_EC716)
typedef struct
{
    uint32_t slaveModeEn : 1;       // 718 Slave Mode Enable
    uint32_t slotSize : 5;          // 718 Slot Size
    uint32_t wordSize : 5;          // 718 Word Size
    uint32_t alignMode : 1;         // 718 Align Mode
    uint32_t endianMode : 1;        // 718 Endian Mode
    uint32_t dataDly : 2;           // 718 Data Delay
    uint32_t txPad : 2;             // 718 Tx padding
    uint32_t rxSignExt : 1;         // 718 Rx Sign Entension
    uint32_t txPack : 2;            // 718 Tx Pack
    uint32_t rxPack : 2;            // 718 Rx Pack                -D21
    uint32_t txFifoEndianMode : 1;  // 718 Tx Fifo Endian Mode    -D22
    uint32_t rxFifoEndianMode : 1;  // 718 Rx Fifo Endian Mode    -D24
    uint32_t eorMode : 1;           // 718 send last byte for DMA
} lspiDataFmt_t;

typedef struct
{
    uint32_t rxDmaReqEn : 1;          // 718 Tx Dma Req Enable
    uint32_t txDmaReqEn : 1;          // 718 Tx Dma Req Enable
    uint32_t rxDmaTimeOutEn : 1;      // 718 Tx Dma Timeout Enable
    uint32_t dmaWorkWaitCycle : 5;    // 718 Dma Work Wait Cycle
    uint32_t rxDmaBurstSizeSub1 : 4;  // 718 Tx Dma Burst Size subtract 1
    uint32_t txDmaBurstSizeSub1 : 4;  // 718 Tx Dma Burst Size subtract 1
    uint32_t rxDmaThreadHold : 4;     // 718 Tx Dma Threadhold
    uint32_t txDmaThreadHold : 4;     // 718 Tx Dma Threadhold
    uint32_t rxFifoFlush : 1;         // 718 Tx Fifo flush
    uint32_t txFifoFlush : 1;         // 718 Tx Fifo flush
} lspiDmaCtrl_t;

typedef struct
{
    uint32_t txUnderRunIntEn : 1;        // 718 Tx Underrun interrupt Enable
    uint32_t txDmaErrIntEn : 1;          // 718 Tx Dma Err Interrupt Enable
    uint32_t txDatIntEn : 1;             // 718 Tx Data Interrupt Enable
    uint32_t rxOverFlowIntEn : 1;        // 718 Tx Overflow Interrupt Enable
    uint32_t rxDmaErrIntEn : 1;          // 718 Tx Dma Err Interrupt Enable
    uint32_t rxDatIntEn : 1;             // 718 Tx Data Interrupt Enable
    uint32_t rxTimeOutIntEn : 1;         // 718 Tx Timeout Interrupt Enable
    uint32_t fsErrIntEn : 1;             // 718 Frame Start Interrupt Enable
    uint32_t frameStartIntEn : 1;        // 718 Frame End Interrupt Enable
    uint32_t frameEndIntEn : 1;          // 718 Frame End Interrupt Enable
    uint32_t cspiBusTimeOutIntEn : 1;    // 718 Not use
    uint32_t lspiRamWrBreakIntEn : 1;    // 718 Lspi ram wr break int enable
    uint32_t lspiRamWrFrameStartEn : 1;  // 718 Lspi ram wr Frame start enable
    uint32_t lspiRamWrFrameEndEn : 1;    // 718 Lspi ram wr Frame end enable
    uint32_t lspiCmdEndEn : 1;           // 718 Lspi sending command end
    uint32_t cspiOtsuEndEn : 1;          // 718 Cspi OTSU one frame end enable
    uint32_t lspiRamWrEndEn : 1;         // 718 Lspi Ram wr end int enable
    uint32_t txIntThreshHold : 4;        // 718 Tx Interrupt Threadhold
    uint32_t rxIntThreshHold : 4;        // 718 Tx Interrupt Threadhold
} lspiIntCtrl_t;

typedef struct
{
    uint32_t rxTimeOutCycle : 24;  // 718 Tx Timeout Cycle
    uint32_t dummyCycle : 4;       // 718 Dummy cycle
} lspiTimeOutCycle_t;

typedef struct
{
    uint32_t txUnderRun : 1;           // 718 Tx Underrun
    uint32_t txDmaErr : 1;             // 718 Tx Dma Err
    uint32_t txDataRdy : 1;            // 718 Tx Data ready, readOnly
    uint32_t rxOverFlow : 1;           // 718 Tx OverFlow
    uint32_t rxDmaErr : 1;             // 718 Tx Dma Err
    uint32_t rxDataRdy : 1;            // 718 Tx Data ready, readOnly
    uint32_t rxFifoTimeOut : 1;        // 718 Tx Fifo timeout
    uint32_t fsErr : 4;                // 718 Frame synchronization Err
    uint32_t frameStart : 1;           // 718 Frame start
    uint32_t frameEnd : 1;             // 718 Frame end
    uint32_t txFifoLevel : 6;          // 718 Tx Fifo Level, readOnly
    uint32_t rxFifoLevel : 6;          // 718 Tx Fifo level, readOnly
    uint32_t cspiBusTimeOut : 1;       // 718 Cspi Bus timeout
    uint32_t lspiRamWrBreak : 1;       // 718 Lspi ram wr break
    uint32_t lspiRamWrFrameStart : 1;  // 718 Lspi ram wr frame start
    uint32_t lspiRamWrFrameEnd : 1;    // 718 Lspi ram wr frame end
    uint32_t cspiOtsuEnd : 1;          // 718 Cspi otsu one frame end
    uint32_t lspiRamWrEnd : 1;         // 718 Lspi ram wr end
} lspiStats_t;

typedef struct
{
    uint32_t enable : 1;       // 718 lspi Enable
    uint32_t data2Lane : 1;    // 718 2 data lane enable
    uint32_t line4 : 1;        // 718 0: not use port as DCX; 1: use port as DCX
    uint32_t datSrc : 1;       // 718 data from camera or memory
    uint32_t colorModeIn : 2;  // 718 Input data color mode
    uint32_t colorModeOut : 3;  // 718 Output data color mode
    uint32_t yAdjEn : 1;        // 718 Y adjust enable
    uint32_t yAdjSel : 1;       // 718 Y adjustment from cspi or sw
    uint32_t yAdjBound : 17;    // 718 Y adjustment bound
    uint32_t dcDummy : 1;       // 718 Send DCX or dummy
    uint32_t busType : 1;  // 718 718-> 0: Interface I, SDA=INOUT; 1: Interface
                           // II, SDA=input, SDO=output
} lspiCtrl_t;

typedef struct
{
    uint32_t wrRdn : 1;           // 718 0:rd; 1:wr
    uint32_t ramWr : 1;           // 718 start to fill frame memory
    uint32_t rdatDummyCycle : 6;  // 718 Dummy cycle before data read
    uint32_t dataLen : 18;        // 718 data len for wr/rd
    uint32_t rsvd : 5;
    uint32_t init : 1;  // 718 0:lspi normal; 1:lspi initial
} lspiCmdCtrl_t;

typedef struct
{
    uint32_t addr : 8;  // 718 command addr
} lspiCmdAddr_t;

typedef struct
{
    uint32_t idle : 1;  // 718 finish formar command or not
} lspiCmdStats_t;

typedef struct
{
    uint32_t ramWrLen : 18;  // 718 Len of ramwr
} lspiRamWrLen_t;

typedef struct
{
    uint32_t frameHeight : 16;  // 718 frame height
    uint32_t frameWidth : 16;   // 718 frame weight
} lspiInfo_t;

typedef struct
{
    uint32_t tailorBottom : 10;  // 718 cut bottom lines
    uint32_t tailorTop : 10;     // 718 cut top lines
} lspiTailorInfo0_t;

typedef struct lspiTailorInfo_
{
    uint32_t tailorLeft : 10;   // 718 cut left lines
    uint32_t tailorRight : 10;  // 718 cut right lines
} lspiTailorInfo_t;

typedef struct
{
    uint32_t rowScaleFrac : 7;  // 718 row scale frac
    uint32_t : 1;
    uint32_t colScaleFrac : 7;  // 718 col scale frac
} lspiScaleInfo_t;

typedef struct
{
    uint32_t grayCtrl : 2;     // 718 gray ctrl
    uint32_t quartileSel : 1;  // 718 quartile from cspi or sw
    uint32_t quartile1 : 8;    // 718 quartile 1
    uint32_t quartile2 : 8;    // 718 quartile 2
    uint32_t quartile3 : 8;    // 718 quartile 3
} lspiQuartileCtrl_t;

typedef struct
{
    uint32_t quartile1InUse : 8;  // 718 quartile 1 in use
    uint32_t quartile2InUse : 8;  // 718 quartile 2 in use
    uint32_t quartile3InUse : 8;  // 718 quartile 3 in use
} lspiQuartileInUse_t;

typedef struct
{
    uint32_t yadjYmin : 8;       // 718 y adj min
    uint32_t yadjYmax : 8;       // 718 y adj max
    uint32_t yadjStrech : 8;     // 718 y adj stretch
    uint32_t yadjStrechFwl : 8;  // 718 y adj stretch fwl
} lspiYAdj_t;

typedef struct
{
    uint32_t yadjYminInUse : 8;       // 718 y adj min
    uint32_t yadjYmaxInUse : 8;       // 718 y adj max
    uint32_t yadjStrechInUse : 8;     // 718 y adj stretch
    uint32_t yadjStrechFwlInUse : 8;  // 718 y adj stretch fwl
} lspiYAdjInUse_t;

typedef struct
{
    uint32_t pageCmd : 8;    // 718 page cmd
    uint32_t pageCmd0 : 16;  // 718 page cmd 0
    uint32_t : 4;
    uint32_t pageCmd01ByteNum : 4;  // 718 page cmd0 + page cmd1 byte num
} lspiGrayPageCmd0_t;

typedef struct
{
    uint32_t pageCmd1 : 32;  // 718 page cmd1
} lspiGrayPageCmd1_t;

typedef struct
{
    uint32_t frameHeightOut : 10;  // 718 frame height out
    uint32_t frameWidthOut : 10;   // 718 frame width out
} lspiFrameInfoOut_t;

typedef struct
{
    uint32_t i2sBusEn : 1;   // 718 I2S bus enable
    uint32_t cspiBusEn : 1;  // 718 Cspi bus enable
    uint32_t lspiBusEn : 1;  // 718 Lspi bus enable
} lspiBusSel_t;
#else  // CHIP 719
typedef struct
{
    uint32_t slaveModeEn : 1;       // 719 Slave Mode Enable
    uint32_t slotSize : 5;          // 719 Slot Size
    uint32_t wordSize : 5;          // 719 Word Size
    uint32_t alignMode : 1;         // 719 Align Mode
    uint32_t endianMode : 1;        // 719 Endian Mode
    uint32_t dataDly : 2;           // 719 Data Delay
    uint32_t txPad : 2;             // 719 Tx padding
    uint32_t rxSignExt : 1;         // 719 Rx Sign Entension
    uint32_t txPack : 2;            // 719 Tx Pack
    uint32_t rxPack : 2;            // 719 Rx Pack                -D21
    uint32_t txFifoEndianMode : 1;  // 719 Tx Fifo Endian Mode    -D22
    uint32_t rxFifoEndianMode : 1;  // 719 Rx Fifo Endian Mode    -D24
    uint32_t eorMode : 1;           // 719 send last byte for DMA
} lspiDataFmt_t;

typedef struct
{
    uint32_t rxDmaReqEn : 1;          // 719 Tx Dma Req Enable
    uint32_t txDmaReqEn : 1;          // 719 Tx Dma Req Enable
    uint32_t rxDmaTimeOutEn : 1;      // 719 Tx Dma Timeout Enable
    uint32_t dmaWorkWaitCycle : 5;    // 719 Dma Work Wait Cycle
    uint32_t rxDmaBurstSizeSub1 : 4;  // 719 Tx Dma Burst Size subtract 1
    uint32_t txDmaBurstSizeSub1 : 4;  // 719 Tx Dma Burst Size subtract 1
    uint32_t rxDmaThreadHold : 4;     // 719 Tx Dma Threadhold
    uint32_t txDmaThreadHold : 4;     // 719 Tx Dma Threadhold
    uint32_t rxFifoFlush : 1;         // 719 Tx Fifo flush
    uint32_t txFifoFlush : 1;         // 719 Tx Fifo flush
} lspiDmaCtrl_t;

typedef struct
{
    uint32_t txUnderRunIntEn : 1;        // 719 Tx Underrun interrupt Enable
    uint32_t txDmaErrIntEn : 1;          // 719 Tx Dma Err Interrupt Enable
    uint32_t txDatIntEn : 1;             // 719 Tx Data Interrupt Enable
    uint32_t rxOverFlowIntEn : 1;        // 719 Tx Overflow Interrupt Enable
    uint32_t rxDmaErrIntEn : 1;          // 719 Tx Dma Err Interrupt Enable
    uint32_t rxDatIntEn : 1;             // 719 Tx Data Interrupt Enable
    uint32_t rxTimeOutIntEn : 1;         // 719 Tx Timeout Interrupt Enable
    uint32_t fsErrIntEn : 1;             // 719 Frame Start Interrupt Enable
    uint32_t frameStartIntEn : 1;        // 719 Frame End Interrupt Enable
    uint32_t frameEndIntEn : 1;          // 719 Frame End Interrupt Enable
    uint32_t cspiBusTimeOutIntEn : 1;    // 719 Not use
    uint32_t lspiRamWrBreakIntEn : 1;    // 719 Lspi ram wr break int enable
    uint32_t lspiRamWrFrameStartEn : 1;  // 719 Lspi ram wr Frame start enable
    uint32_t lspiRamWrFrameEndEn : 1;    // 719 Lspi ram wr Frame end enable
    uint32_t lspiCmdEndEn : 1;           // 719 Lspi sending command end
    uint32_t cspiOtsuEndEn : 1;          // 719 Cspi OTSU one frame end enable
    uint32_t lspiRamWrEndEn : 1;         // 719 Lspi Ram wr end int enable
    uint32_t txIntThreshHold : 4;        // 719 Tx Interrupt Threadhold
    uint32_t rxIntThreshHold : 4;        // 719 Tx Interrupt Threadhold
    uint32_t ramWrIntCtrl : 3;           // 719 Lspi Ram write Int ctrl
    uint32_t ramWrIntCtrlEn : 1;  // 719 Lspi Ram write Int ctrl enable bit
} lspiIntCtrl_t;

typedef struct
{
    uint32_t rxTimeOutCycle : 24;  // 719 Tx Timeout Cycle
    uint32_t dummyCycle : 4;       // 719 Dummy cycle
} lspiTimeOutCycle_t;

typedef struct
{
    uint32_t txUnderRun : 1;           // 719 Tx Underrun
    uint32_t txDmaErr : 1;             // 719 Tx Dma Err
    uint32_t txDataRdy : 1;            // 719 Tx Data ready, readOnly
    uint32_t rxOverFlow : 1;           // 719 Tx OverFlow
    uint32_t rxDmaErr : 1;             // 719 Tx Dma Err
    uint32_t rxDataRdy : 1;            // 719 Tx Data ready, readOnly
    uint32_t rxFifoTimeOut : 1;        // 719 Tx Fifo timeout
    uint32_t fsErr : 4;                // 719 Frame synchronization Err
    uint32_t frameStart : 1;           // 719 Frame start
    uint32_t frameEnd : 1;             // 719 Frame end
    uint32_t txFifoLevel : 6;          // 719 Tx Fifo Level, readOnly
    uint32_t rxFifoLevel : 6;          // 719 Tx Fifo level, readOnly
    uint32_t cspiBusTimeOut : 1;       // 719 Cspi Bus timeout
    uint32_t lspiRamWrBreak : 1;       // 719 Lspi ram wr break
    uint32_t lspiRamWrFrameStart : 1;  // 719 Lspi ram wr frame start
    uint32_t lspiRamWrFrameEnd : 1;    // 719 Lspi ram wr frame end
    uint32_t cspiOtsuEnd : 1;          // 719 Cspi otsu one frame end
    uint32_t lspiRamWrEnd : 1;         // 719 Lspi ram wr end
} lspiStats_t;

typedef struct
{
    uint32_t enable : 1;       // 719 lspi Enable
    uint32_t dspiEn : 1;       // 719 dual spi mode enable
    uint32_t line4 : 1;        // 719 0: not use port as DCX; 1: use port as DCX
    uint32_t datSrc : 1;       // 719 data from camera or memory
    uint32_t colorModeIn : 3;  // 719 Input data color mode
    uint32_t colorModeOut : 3;  // 719 Output data color mode
    uint32_t yAdjEn : 1;        // 719 Y adjust enable
    uint32_t yAdjSel : 1;       // 719 Y adjustment from cspi or sw
    uint32_t yAdjBound : 17;    // 719 Y adjustment bound
    uint32_t dcDummy : 1;       // 719 Send DCX or dummy
    uint32_t dspiCfg : 2;       // 719 0: RGB565/666/888;  1: rsv; 2:
                                // RGB444/565/666/888;  3: RGB565/888
} lspiCtrl_t;

typedef struct
{
    uint32_t wrRdn : 1;           // 719 0:rd; 1:wr
    uint32_t ramWr : 1;           // 719 start to fill frame memory
    uint32_t rdatDummyCycle : 6;  // 719 Dummy cycle before data read
    uint32_t dataLen : 22;        // 719 data len for wr/rd
    uint32_t
        ramWrHaltMode : 1;  // 719 0:pull up lspi_csn to stop trans; 1: lspi_csn
                            // is low, by stopping lspi_clk to stop trans;
    uint32_t init : 1;      // 719 0:lspi normal; 1:lspi initial
} lspiCmdCtrl_t;

typedef struct
{
    uint32_t addr : 8;             // 719 command addr
    uint32_t csnHighCycleMin : 8;  // 719 the num cycles(lspi_clk) during
                                   // lspi_csn high
    uint32_t rsb : 15;
    uint32_t busType : 1;
} lspiCmdAddr_t;

typedef struct
{
    uint32_t idle : 1;  // 719 finish formar command or not
} lspiCmdStats_t;

typedef struct
{
    uint32_t ramWrLen : 18;  // 719 Len of ramwr
} lspiRamWrLen_t;

typedef struct
{
    uint32_t frameHeight : 16;  // 719 frame height
    uint32_t frameWidth : 16;   // 719 frame weight
} lspiInfo_t;

typedef struct
{
    uint32_t tailorBottom : 10;  // 719 cut bottom lines
    uint32_t tailorTop : 10;     // 719 cut top lines
} lspiTailorInfo0_t;

typedef struct
{
    uint32_t tailorLeft : 10;   // 719 cut left lines
    uint32_t tailorRight : 10;  // 719 cut right lines
} lspiTailorInfo_t;

typedef struct
{
    uint32_t rowScaleFrac : 7;  // 719 row scale frac
    uint32_t : 1;
    uint32_t colScaleFrac : 7;  // 719 col scale frac
} lspiScaleInfo_t;

typedef struct
{
    uint32_t grayCtrl : 2;     // 719 gray ctrl
    uint32_t quartileSel : 1;  // 719 quartile from cspi or sw
    uint32_t quartile1 : 8;    // 719 quartile 1
    uint32_t quartile2 : 8;    // 719 quartile 2
    uint32_t quartile3 : 8;    // 719 quartile 3
} lspiQuartileCtrl_t;

typedef struct
{
    uint32_t quartile1InUse : 8;  // 719 quartile 1 in use
    uint32_t quartile2InUse : 8;  // 719 quartile 2 in use
    uint32_t quartile3InUse : 8;  // 719 quartile 3 in use
} lspiQuartileInUse_t;

typedef struct
{
    uint32_t yadjYmin : 8;       // 719 y adj min
    uint32_t yadjYmax : 8;       // 719 y adj max
    uint32_t yadjStrech : 8;     // 719 y adj stretch
    uint32_t yadjStrechFwl : 8;  // 719 y adj stretch fwl
} lspiYAdj_t;

typedef struct
{
    uint32_t yadjYminInUse : 8;       // 719 y adj min
    uint32_t yadjYmaxInUse : 8;       // 719 y adj max
    uint32_t yadjStrechInUse : 8;     // 719 y adj stretch
    uint32_t yadjStrechFwlInUse : 8;  // 719 y adj stretch fwl
} lspiYAdjInUse_t;

typedef struct
{
    uint32_t pageCmd : 8;    // 719 page cmd
    uint32_t pageCmd0 : 16;  // 719 page cmd 0
    uint32_t : 4;
    uint32_t pageCmd01ByteNum : 4;  // 719 page cmd0 + page cmd1 byte num
} lspiGrayPageCmd0_t;

typedef struct
{
    uint32_t pageCmd1 : 32;  // 719 page cmd1
} lspiGrayPageCmd1_t;

typedef struct
{
    uint32_t frameHeightOut : 11;  // 719 frame height out
    uint32_t rsvd : 1;
    uint32_t frameWidthOut : 11;  // 719 frame width out
} lspiFrameInfoOut_t;

typedef struct
{
    uint32_t mspiEn : 1;        // 719 I2S bus enable
    uint32_t mspiAddrLane : 2;  // 719 Cspi bus enable
    uint32_t mspiDataLane : 2;  // 719 Lspi bus enable
    uint32_t rsv : 3;
    uint32_t mspiInst : 8;         // 719 Lspi bus enable
    uint32_t mspiVsyncEn : 1;      // 719 Lspi bus enable
    uint32_t vsyncLineCycle : 15;  // 719 Lspi bus enable
} lspiMspiCtrl_t;

typedef struct
{
    uint32_t hsyncInst : 8;  // 719 I2S bus enable
    uint32_t hsyncAddr : 8;  // 719 Cspi bus enable
    uint32_t vbp : 16;       // 719 Lspi bus enable
} lspiVsyncCtrl_t;

typedef struct
{
    uint32_t lspi8080En : 1;  // 719 I2S bus enable
    uint32_t rsvd : 15;
    uint32_t vfp : 16;
} lspi8080Ctrl_t;

typedef struct
{
    uint32_t lspiCmd0PreEn : 1;           // 719 Lspi cmd0 pre en
    uint32_t lspiCmd1PreEn : 1;           // 719 Lspi cmd1 pre en
    uint32_t lspiCmd0PreParaLen : 3;      // 719 Lspi cmd0 pre param len
    uint32_t lspiCmd1PreParaLen : 3;      // 719 Lspi cmd1 pre param len
    uint32_t lspiCmdPreMspiAddrLane : 2;  // 719 Lspi cmd pre mspi addr lane
    uint32_t lspiCmdPreMspiDataLane : 2;  // 719 Lspi cmd pre mspi data lane
} lspiCmdPreParam0_t;

typedef struct
{
    uint32_t lspiCmd0PreInst : 8;  // 719 Lspi cmd0 pre inst
    uint32_t lspiCmd1PreInst : 8;  // 719 Lspi cmd1 pre inst
    uint32_t lspiCmd0Pre : 8;      // 719 Lspi cmd0 pre
    uint32_t lspiCmd1Pre : 8;      // 719 Lspi cmd1 pre
} lspiCmdPreParam1_t;

typedef struct
{
    uint32_t lspiCmd0PrePara : 32;  // 719 Lspi cmd0 pre para
} lspiCmdPreParam2_t;

typedef struct
{
    uint32_t lspiCmd1PrePara : 32;  // 719 Lspi cmd1 pre para
} lspiCmdPreParam3_t;

typedef struct
{
    uint32_t lspiCmd0PostEn : 1;           // 719 Lspi cmd0 pre en
    uint32_t lspiCmd1PostEn : 1;           // 719 Lspi cmd1 pre en
    uint32_t lspiCmd0PostParaLen : 3;      // 719 Lspi cmd0 pre param len
    uint32_t lspiCmd1PostParaLen : 3;      // 719 Lspi cmd1 pre param len
    uint32_t lspiCmdPostMspiAddrLane : 2;  // 719 Lspi cmd pre mspi addr lane
    uint32_t lspiCmdPostMspiDataLane : 2;  // 719 Lspi cmd pre mspi data lane
} lspiCmdPostParam0_t;

typedef struct
{
    uint32_t lspiTeEn : 1;       // 719 Lspi te en
    uint32_t lspiTeEdgeSel : 1;  // 719 Lspi te edge sel: 0->te rise edge; 1->te
                                 // fall edge
    uint32_t rsvd : 2;
    uint32_t lspiTePos0 : 26;  // 719 Lspi te pos0
} lspiTeParam0_t;

typedef struct
{
    uint32_t lspiTePos1 : 26;  // 719 Lspi te pos1
} lspiTeParam1_t;

typedef struct
{
    uint32_t lspiCmd0PostInst : 8;  // 719 Lspi cmd0 post inst
    uint32_t lspiCmd1PostInst : 8;  // 719 Lspi cmd1 post inst
    uint32_t lspiCmd0Post : 8;      // 719 Lspi cmd0 post
    uint32_t lspiCmd1Post : 8;      // 719 Lspi cmd1 post
} lspiCmdPostParam1_t;

typedef struct
{
    uint32_t lspiCmd0PostPara : 32;  // 719 Lspi cmd0 post para
} lspiCmdPostParam2_t;

typedef struct
{
    uint32_t lspiCmd1PostPara : 32;  // 719 Lspi cmd1 post para
} lspiCmdPostParam3_t;

typedef struct
{
    uint32_t i2sBusEn : 1;   // 719 I2S bus enable
    uint32_t cspiBusEn : 1;  // 719 Cspi bus enable
    uint32_t lspiBusEn : 1;  // 719 Lspi bus enable
} lspiBusSel_t;
#endif

typedef enum LspiIoCtrl_
{
    OPEN_LSPI_IOCTL_ISR = 0,
    OPEN_LSPI_IOCTL_DATA_FORMAT = 1,  // 配置数据格式寄存器，参数为lspiDataFmt_t
    OPEN_LSPI_IOCTL_BUS_SPEED = 2,  // 配置总线速度，参数为uint32_t，单位为Hz
    OPEN_LSPI_IOCTL_DMA_CTRL = 3,  // 配置DMA控制寄存器，参数为lspiDmaCtrl_t
    OPEN_LSPI_IOCTL_INT_CTRL = 4,  // 配置INT控制寄存器，参数为lspiIntCtrl_t
    OPEN_LSPI_IOCTL_CTRL = 5,  // 配置LSPI控制寄存器，参数为lspiCtrl_t
    OPEN_LSPI_IOCTL_CMD_CTRL =
        6,  // 配置LSPI命令控制寄存器，参数为lspiCmdCtrl_t
    OPEN_LSPI_IOCTL_YUV2RGB_INFO0 =
        7,  // 配置YUV2RGB信息寄存器0，参数为lspiYuv2RgbInfo0_t
    OPEN_LSPI_IOCTL_YUV2RGB_INFO1 =
        8,  // 配置YUV2RGB信息寄存器1，参数为lspiYuv2RgbInfo1_t
    OPEN_LSPI_IOCTL_CMD_ADDR =
        9,  // 配置LSPI命令地址寄存器，参数为lspiCmdAddr_t
    OPEN_LSPI_IOCTL_FRAME_INFO = 10,  // 配置LSPI帧信息寄存器，参数为lspiInfo_t
    OPEN_LSPI_IOCTL_TAILOR_INFO0 =
        11,  // 配置LSPI裁剪信息寄存器0，参数为lspitailorInfo0_t
    OPEN_LSPI_IOCTL_TAILOR_INFO1 =
        12,  // 配置LSPI裁剪信息寄存器1，参数为lspitailorInfo1_t
    OPEN_LSPI_IOCTL_SCALE_INFO =
        13,  // 配置LSPI缩放信息寄存器，参数为lspiScaleInfo_t
    OPEN_LSPI_IOCTL_QUARTILE_CTRL =
        14,  // 配置LSPI分块控制寄存器，参数为lspiQuartileCtrl_t
    OPEN_LSPI_IOCTL_YADJ = 15,  // 配置LSPI Y调整寄存器，参数为lspiYadj_t
    OPEN_LSPI_IOCTL_GRAY_PAGE_CMD0 =
        16,  // 配置LSPI灰度页命令寄存器0，参数为lspiGrayPageCmd0_t
    OPEN_LSPI_IOCTL_GRAY_PAGE_CMD1 =
        17,  // 配置LSPI灰度页命令寄存器1，参数为lspiGrayPageCmd1_t
    OPEN_LSPI_IOCTL_FRAME_INFO_OUT =
        18,  // 配置LSPI帧信息输出寄存器，参数为lspiFrameInfoOut_t
    OPEN_LSPI_IOCTL_BUS_SEL = 19,  // 配置LSPI I2C总线选择寄存器，参数为uint8_t
#if(defined TYPE_EC718M)
    OPEN_LSPI_IOCTL_MSPI_CTRL =
        20,  // 配置LSPI MSPI控制寄存器，参数为lspiMspiCtrl_t
    OPEN_LSPI_IOCTL_VSYNC_CTRL =
        21,  // 配置LSPI VSYNC控制寄存器，参数为lspiVsyncCtrl_t
    OPEN_LSPI_IOCTL_8080_CTRL =
        22,  // 配置LSPI 8080控制寄存器，参数为lspi8080Ctrl_t
    OPEN_LSPI_IOCTL_PRE_PARA0_CTRL =
        23,  // 配置LSPI预参数0控制寄存器，参数为lspiPreParam0Ctrl_t
    OPEN_LSPI_IOCTL_PRE_PARA2_CTRL =
        24,  // 配置LSPI预参数2控制寄存器，参数为lspiPreParam2Ctrl_t
    OPEN_LSPI_IOCTL_PRE_PARA3_CTRL =
        25,  // 配置LSPI预参数3控制寄存器，参数为lspiPreParam3Ctrl_t
    OPEN_LSPI_IOCTL_POST_PARA0_CTRL =
        26,  // 配置LSPI后参数0控制寄存器，参数为lspiCmdPostParam0_t
    OPEN_LSPI_IOCTL_TE_CTRL0 =
        27,  // 配置LSPI TE控制寄存器0，参数为lspiTeCtrl0_t
    OPEN_LSPI_IOCTL_TE_CTRL1 =
        28,  // 配置LSPI TE控制寄存器1，参数为lspiTeCtrl1_t
#endif
    OPEN_LSPI_IOCTL_CLEAR_WREND = 29,          // 清除LSPI写结束标志位
    OPEN_LSPI_IOCTL_RST_CLERA_FIFO = 30,       // 重置并清除LSPI FIFO
    OPEN_LSPI_IOCTL_GET_FIFO_ADDR = 31,        // 获取LSPI FIFO地址
    OPEN_LSPI_IOCTL_CLEAN_PREVIEW_STATE = 32,  // 清除preview后的状态寄存器
} LspiIoCtrl_t;

typedef struct LspiConfig_
{
    uint32_t freq;
} LspiConfig_t;
// USP2_MCLK/USP2_CS/USP2_LSPI_TE

typedef enum DataType_
{
    TYPE_CMD = 0,
    TYPE_DATA = 1,
} DataType_t;

typedef struct WriteParam_
{
    DataType_t type;
    uint8_t cmd;
    uint8_t *data;
    uint32_t len;
} WriteParam_t;

int32_t api_lspi_startup(void *para);
uint32_t api_lspi_create(uint32_t index, LspiConfig_t *config);
api_ret_t api_lspi_setup(int8_t index, LspiConfig_t *para);
api_ret_t api_lspi_delete(uint32_t usrId);
api_ret_t api_lspi_open(uint32_t usrId, LspiConfig_t *cfg, size_t timeout);
api_ret_t api_lspi_close(uint32_t usrId);
api_ret_t api_lspi_ioctl(uint32_t usrId, LspiIoCtrl_t type, void *para);
api_ret_t api_lspi_pmctl(uint32_t usrId, open_hal_pm_t *cfg, size_t count);
api_ret_t api_lspi_write(uint32_t usrId, void *buf, size_t count);
api_ret_t api_lspi_read(uint32_t usrId, void *buf, size_t count);
api_ret_t api_lspi_query(uint32_t usrId);

int api_test_lspi(void);

#define CSV_CFG_LSPI_ITEMS (6)
int32_t api_lspi_parse(char *str, LspiConfig_t *cfg);
#ifdef __cplusplus
}
#endif
#endif /* _API_LSPI_H_ */