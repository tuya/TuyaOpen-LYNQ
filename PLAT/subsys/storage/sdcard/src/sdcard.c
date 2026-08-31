#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include DEBUG_LOG_HEADER_FILE
#include "bsp.h"
#include "sdcard.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif


extern ARM_DRIVER_SPI Driver_SPI0;
static ARM_DRIVER_SPI *spiMasterDrv = &CREATE_SYMBOL(Driver_SPI, 0);
#define SD_SPI_SSN_GPIO_INSTANCE    RTE_SPI0_SSN_GPIO_INSTANCE
#define SD_SPI_SSN_GPIO_INDEX       RTE_SPI0_SSN_GPIO_INDEX
#define TRANSFER_DATA_WIDTH         8
#define SPEED_LOW                   0
#define SPEED_HIGH                  1
#define SD_SPI_SPEED_LOW            100000
#define SD_SPI_SPEED_HIGH           61440000
#define SD_POWER_PAD                46
#define SD_POWER_GPIO               21
#define SD_POWER_PORT               ((SD_POWER_GPIO) / 16)
#define SD_POWER_PIN                ((SD_POWER_GPIO) % 16)
#define SPI_POLLING_DMA_SPLIT       14 // DMA: Greater than or equal to 14


#if (RTE_SPI0_IO_MODE == DMA_MODE)
#define SPIDMA_AND_OS_ENABLE
#endif
#ifdef SPIDMA_AND_OS_ENABLE
static osEventFlagsId_t gSpiFlags                           = NULL;
static uint8_t          gSdDMABufferForRed[SD_BLOCK_SIZE]   = {0};
static uint8_t          gSdDMABufferForWrite[SD_BLOCK_SIZE] = {0};
#endif
static uint16_t         gCsPinMask                          = 0;

#ifdef SPI_MUTEX_ENABLE
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
extern osMutexId_t gSpiMutex;
#else
osMutexId_t gSpiMutex = NULL;
#endif
#endif
uint8_t gSdType = 0;

#if 0
static void sdCtrlIoInit(void)
{
    PadConfig_t     padConfig = {0};
    GpioPinConfig_t pinConfig = {0};

    PAD_getDefaultConfig(&padConfig);
    padConfig.mux = PAD_MUX_ALT0;
    PAD_setPinConfig(SD_POWER_PAD,     &padConfig);

    pinConfig.pinDirection = GPIO_DIRECTION_OUTPUT;
    pinConfig.misc.initOutput = 1;
    GPIO_pinConfig(SD_POWER_PORT, SD_POWER_PIN, &pinConfig);
}
#endif

#ifdef SPIDMA_AND_OS_ENABLE
/**
  \fn          void SPI_Callback()
  \brief       SPI callback
  \return
*/
PLAT_FM_RAMCODE void SPI_Callback(uint32_t event)
{
    if (event & ARM_SPI_EVENT_TRANSFER_COMPLETE)
    {
        osEventFlagsSet(gSpiFlags, 0x01);
    }
    else
    {
        ECPLAT_PRINTF(UNILOG_PLA_APP, SPI_Callback, P_ERROR, "event=0x%X", event);
    }
}
#endif

static void sdSpiIoInit(uint8_t speed)
{
    uint32_t freq = (speed == SPEED_LOW) ? SD_SPI_SPEED_LOW : SD_SPI_SPEED_HIGH;

#ifdef SPIDMA_AND_OS_ENABLE
    if (gSpiFlags == NULL)
    {
        gSpiFlags = osEventFlagsNew(NULL);
    }
    spiMasterDrv->Initialize(SPI_Callback);
#else
    spiMasterDrv->Initialize(NULL);
#endif
    spiMasterDrv->PowerControl(ARM_POWER_FULL);

    spiMasterDrv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL0_CPHA0 | ARM_SPI_DATA_BITS(TRANSFER_DATA_WIDTH) |
                          ARM_SPI_MSB_LSB     | ARM_SPI_SS_MASTER_SW, freq);

    gCsPinMask = 1 << SD_SPI_SSN_GPIO_INDEX;
    memset(gSdDMABufferForRed, 0xFF, sizeof(gSdDMABufferForRed));
}


PLAT_FM_RAMCODE void sdSpiCsSetLow(void)
{
    GPIO_pinWrite(SD_SPI_SSN_GPIO_INSTANCE, gCsPinMask, gCsPinMask);
}

PLAT_FM_RAMCODE void sdSpiCsSetHigh(void)
{
    GPIO_pinWrite(SD_SPI_SSN_GPIO_INSTANCE, gCsPinMask, 0);
}

PLAT_FM_RAMCODE uint8_t sdSpiReadWriteByte(uint8_t dataOut)
{
    uint8_t dataIn = 0;

    spiMasterDrv->Transfer(&dataOut, &dataIn, 1);

    return dataIn;
}

#ifdef SPIDMA_AND_OS_ENABLE
PLAT_FM_RAMCODE void sdSpiReadBuffer(uint8_t *bufferIn, uint32_t length)
{
    spiMasterDrv->Transfer(gSdDMABufferForRed, bufferIn, length);
    if ((length >= SPI_POLLING_DMA_SPLIT) && (osEventFlagsWait(gSpiFlags, 0x01, osFlagsWaitAll, 10) == osErrorTimeout))
    {
        SYSLOG_DEBUG("SPI read timeout.\r\n");
        ECPLAT_PRINTF(UNILOG_PLA_APP, sdSpiReadBuffer2, P_DEBUG, "SPI read timeout.");
    }
}

PLAT_FM_RAMCODE void sdSpiWriteBuffer(uint8_t *bufferOut, uint32_t length)
{
    spiMasterDrv->Transfer(bufferOut, gSdDMABufferForWrite, length);
    if ((length >= SPI_POLLING_DMA_SPLIT) && (osEventFlagsWait(gSpiFlags, 0x01, osFlagsWaitAll, 10) == osErrorTimeout))
    {
        SYSLOG_DEBUG("SPI write timeout.\r\n");
        ECPLAT_PRINTF(UNILOG_PLA_APP, sdSpiWriteBuffer2, P_DEBUG, "SPI write timeout.");
    }
}
#endif

PLAT_FM_RAMCODE uint8_t sdWaitReady(void)
{
    uint32_t t = 0;

    do
    {
        if (sdSpiReadWriteByte(0xFF) == 0xFF)
        {
            return SD_OK;
        }

        t++;
    } while (t < 0xFFFF);

    return SD_ERROR;
}

PLAT_FM_RAMCODE void sdDeselect(void)
{
    sdSpiCsSetHigh();
    sdSpiReadWriteByte(0xFF);
}

PLAT_FM_RAMCODE uint8_t sdSelect(void)
{
    sdSpiCsSetLow();

    if (sdWaitReady() == 0)
    {
        return SD_OK;
    }

    sdDeselect();
    return SD_ERROR;
}

PLAT_FM_RAMCODE uint8_t sdGetResponse(uint8_t response)
{
    uint16_t count = 0xFFFF;

    while ((sdSpiReadWriteByte(0xFF) != response) && count)
    {
        count--;
    }

    if (count == 0)
    {
        return SD_ERROR;
    }

    return SD_OK;
}

PLAT_FM_RAMCODE uint8_t sdReceiveData(uint8_t *buf, uint16_t len)
{
    if (sdGetResponse(0xFE))
    {
        return SD_ERROR;
    }

#ifdef SPIDMA_AND_OS_ENABLE
    sdSpiReadBuffer(buf, len);
#else
    while (len--)
    {
        *buf = sdSpiReadWriteByte(0xFF);
        buf++;
    }
#endif

#ifdef SPIDMA_AND_OS_ENABLE
    uint8_t buffer[] = {0xFF, 0xFF};
    sdSpiWriteBuffer(buffer, sizeof(buffer));
#else
    sdSpiReadWriteByte(0xFF);
    sdSpiReadWriteByte(0xFF);
#endif

    return SD_OK;
}

PLAT_FM_RAMCODE uint8_t sdSendBlock(uint8_t *buf, uint8_t cmd)
{
    uint16_t t;

    if (sdWaitReady())
    {
        return SD_ERROR;
    }

    sdSpiReadWriteByte(cmd);

    if (cmd != 0xFD)
    {
#ifdef SPIDMA_AND_OS_ENABLE
        sdSpiWriteBuffer(buf, SD_BLOCK_SIZE);
#else
        for (t = 0; t < SD_BLOCK_SIZE; t++)
        {
            sdSpiReadWriteByte(buf[t]);
        }
#endif

#ifdef SPIDMA_AND_OS_ENABLE
        uint8_t buffer[] = {0xFF, 0xFF, 0xFF};
        sdSpiReadBuffer(buffer, sizeof(buffer));
        t = buffer[2];
#else
        sdSpiReadWriteByte(0xFF);
        sdSpiReadWriteByte(0xFF);

        t = sdSpiReadWriteByte(0xFF);
#endif

        if ((t & 0x1F) != 0x05)
        {
            return SD_ERROR;
        }
    }

    return SD_OK;
}

PLAT_FM_RAMCODE uint8_t sdSendCmd(uint8_t cmd, uint32_t arg)
{
    uint8_t res;
    uint8_t retry = 0;
    uint8_t crc = 0x01;

    if (cmd & 0x80)
    {
        cmd &= 0x7F;
        res = sdSendCmd(CMD55, 0);

        if (res > 1)
        {
            return res;
        }
    }

    if (cmd != CMD12)
    {
        sdDeselect();

        if (sdSelect())
        {
            return 0xFF;
        }
    }

#ifdef SPIDMA_AND_OS_ENABLE
    if (cmd == CMD0)
    {
        crc = 0x95;
    }
    else if (cmd == CMD8)
    {
        crc = 0x87;
    }
    uint8_t sendBuff[] = {cmd | 0x40, arg >> 24, arg >> 16, arg >> 8, arg, crc};
    sdSpiWriteBuffer(sendBuff, sizeof(sendBuff));
#else
    sdSpiReadWriteByte(cmd | 0x40);
    sdSpiReadWriteByte(arg >> 24);
    sdSpiReadWriteByte(arg >> 16);
    sdSpiReadWriteByte(arg >> 8);
    sdSpiReadWriteByte(arg);

    if (cmd == CMD0) crc = 0x95;

    if (cmd == CMD8) crc = 0x87;

    sdSpiReadWriteByte(crc);
#endif

    if (cmd == CMD12)
    {
        sdSpiReadWriteByte(0xFF);
    }


    retry = 10;

    do
    {
        res = sdSpiReadWriteByte(0xFF);
    } while ((res & 0x80) && retry--);

    return res;
}

PLAT_FM_RAMCODE uint8_t sdGetStatus(void)
{
    uint8_t res;
    uint8_t retry = 20;

#ifdef SPI_MUTEX_ENABLE
    osMutexAcquire(gSpiMutex, osWaitForever);
#endif
    do
    {
        res = sdSendCmd(ACMD13, 0);
    }while(res && retry--);

    sdDeselect();
#ifdef SPI_MUTEX_ENABLE
    osMutexRelease(gSpiMutex);
#endif

    return res;
}

PLAT_FM_RAMCODE uint8_t sdGetCid(uint8_t *cid_data)
{
    uint8_t res;

#ifdef SPI_MUTEX_ENABLE
    osMutexAcquire(gSpiMutex, osWaitForever);
#endif
    res = sdSendCmd(CMD10, 0);

    if (res == 0x00)
    {
        res = sdReceiveData(cid_data, 16);
    }

    sdDeselect();
#ifdef SPI_MUTEX_ENABLE
    osMutexRelease(gSpiMutex);
#endif

    return res;
}

PLAT_FM_RAMCODE uint8_t sdGetCsd(uint8_t *csd_data)
{
    uint8_t res;
    res = sdSendCmd(CMD9, 0);

    if (res == 0)
    {
        res = sdReceiveData(csd_data, 16);
    }

    sdDeselect();
    return res;
}

PLAT_FM_RAMCODE uint32_t sdGetBlockCount(void)
{
    uint8_t csd[16];
    uint32_t capacity;
    uint8_t n;
    uint16_t csize;

#ifdef SPI_MUTEX_ENABLE
    osMutexAcquire(gSpiMutex, osWaitForever);
#endif
    if (sdGetCsd(csd) != 0)
    {
        return 0;
    }

    if ((csd[0] & 0xC0) == 0x40)
    {
        csize = csd[9] + ((uint16_t)csd[8] << 8) + ((uint32_t)(csd[7] & 63) << 16) + 1;
        capacity = (uint32_t)csize << 10;
    }
    else
    {
        n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
        csize = (csd[8] >> 6) + ((uint16_t)csd[7] << 2) + ((uint16_t)(csd[6] & 3) << 10) + 1;
        capacity = (uint32_t)csize << (n - 9);
    }
#ifdef SPI_MUTEX_ENABLE
    osMutexRelease(gSpiMutex);
#endif

    return capacity;
}

uint8_t sdInit(void)
{
    uint8_t res;
    uint16_t retry;
    uint8_t ocr[4];
    uint16_t i;
    uint8_t cmd;

#ifdef SPI_MUTEX_ENABLE
    osMutexAcquire(gSpiMutex, osWaitForever);
#endif
    // sdCtrlIoInit();
    // osDelay(100);
    sdSpiIoInit(SPEED_LOW);
    sdSpiCsSetHigh();

    for (i = 0; i < 10; i++)
    {
        sdSpiReadWriteByte(0xFF);
    }

    retry = 20;

    do
    {
        res = sdSendCmd(CMD0, 0);
    } while ((res != 0x01) && retry--);

    gSdType = 0;

    if (res == 0x01)
    {
        if (sdSendCmd(CMD8, 0x1AA) == 1)
        {
            for (i = 0; i < 4; i++)
            {
                ocr[i] = sdSpiReadWriteByte(0xFF);
            }

            if (ocr[2] == 0x01 && ocr[3] == 0xAA)
            {
                retry = 1000;

                do
                {
                    res = sdSendCmd(ACMD41, 1UL << 30);
                } while (res && retry--);

                if (retry && sdSendCmd(CMD58, 0) == 0)
                {
                    for (i = 0; i < 4; i++)
                    {
                        ocr[i] = sdSpiReadWriteByte(0xFF);
                    }

                    if (ocr[0] & 0x40)
                    {
                        gSdType = SD_TYPE_V2HC;
                    }
                    else
                    {
                        gSdType = SD_TYPE_V2;
                    }
                }
            }
        }
        else
        {
            res = sdSendCmd(ACMD41, 0);
            retry = 1000;

            if (res <= 1)
            {
                gSdType = SD_TYPE_V1;
                cmd = ACMD41;
            }
            else
            {
                gSdType = SD_TYPE_MMC;
                cmd = CMD1;
            }

            do
            {
                res = sdSendCmd(cmd, 0);
            } while (res && retry--);

            if (retry == 0 || sdSendCmd(CMD16, SD_BLOCK_SIZE) != 0)
            {
                gSdType = SD_TYPE_ERR;
            }
        }
    }

    sdDeselect();

    if (gSdType)
    {
        res = SD_OK;
        sdSpiIoInit(SPEED_HIGH);
        // SYSLOG_DEBUG("SD card capacity: %d MB\r\n", sdGetBlockCount() >> 11);
    }
    else
    {
        res = SD_ERROR;
        SYSLOG_ERR("No SD card.\r\n");
    }
#ifdef SPI_MUTEX_ENABLE
    osMutexRelease(gSpiMutex);
#endif
    // SYSLOG_DEBUG("SD card gSdType: %d\r\n", gSdType);

    return res;
}

PLAT_FM_RAMCODE uint8_t sdReadDisk(uint8_t *pbuf, uint32_t saddr, uint32_t cnt)
{
    uint8_t res;
    long long lsaddr = saddr;
    // SYSLOG_DEBUG("sdReadDisk saddr: 0x%08x\r\n",saddr);
    // SYSLOG_DEBUG("sdReadDisk cnt: %d \r\n",cnt);

#ifdef SPI_MUTEX_ENABLE
    osMutexAcquire(gSpiMutex, osWaitForever);
#endif
    if (gSdType != SD_TYPE_V2HC)
    {
        lsaddr <<= 9;
    }

    if (cnt == 1)
    {
        res = sdSendCmd(CMD17, lsaddr);

        if (res == 0)
        {
            res = sdReceiveData(pbuf, SD_BLOCK_SIZE);
        }
    }
    else
    {
        res = sdSendCmd(CMD18, lsaddr);

#if (defined(SPIDMA_AND_OS_ENABLE) && defined(SD_FAST))
        sdReceiveDataFast(pbuf, SD_BLOCK_SIZE, cnt);
#else
        do
        {
            res = sdReceiveData(pbuf, SD_BLOCK_SIZE);
            pbuf += SD_BLOCK_SIZE;
        } while (--cnt && res == 0);
#endif

        sdSendCmd(CMD12, 0);
    }

    sdDeselect();
#ifdef SPI_MUTEX_ENABLE
    osMutexRelease(gSpiMutex);
#endif

    return res;
}

PLAT_FM_RAMCODE uint8_t sdWriteDisk(uint8_t *pbuf, uint32_t saddr, uint32_t cnt)
{
    uint8_t retry = 20;
    uint8_t res;
    long long lsaddr = saddr;

#ifdef SPI_MUTEX_ENABLE
    osMutexAcquire(gSpiMutex, osWaitForever);
#endif
    if (gSdType != SD_TYPE_V2HC)
    {
        lsaddr <<= 9;
    }

    if (cnt == 1)
    {
        res = sdSendCmd(CMD24, lsaddr);

        if (res == 0)
        {
            res = sdSendBlock(pbuf, 0xFE);
        }
    }
    else
    {
        if (gSdType != SD_TYPE_MMC)
        {
            do
            {
                res = sdSendCmd(ACMD23, cnt);
            }while(res && retry--);
        }

        res = sdSendCmd(CMD25, lsaddr);

        if (res == 0)
        {
            do
            {
                res = sdSendBlock(pbuf, 0xFC);
                pbuf += SD_BLOCK_SIZE;
            } while (--cnt && res == 0);

            res = sdSendBlock(0, 0xFD);
        }
    }

    sdDeselect();
#ifdef SPI_MUTEX_ENABLE
    osMutexRelease(gSpiMutex);
#endif

    return res;
}
