#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "slpman.h"
#include "bsp.h"
#include "cmsis_os2.h"
#include DEBUG_LOG_HEADER_FILE
#include "flashex.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_SUBSYS_OPENHAL_SPI_ENABLE
#include "api_spi.h"
#endif


#define SPI_FREQ                61440000
#define TRANSFER_DATA_WIDTH     8

/** \brief driver instance declare */
extern ARM_DRIVER_SPI Driver_SPI0;
#ifndef FEATURE_SUBSYS_OPENHAL_SPI_ENABLE
static ARM_DRIVER_SPI *spiMasterDrv = &CREATE_SYMBOL(Driver_SPI, 0);
#endif
#define SPI_SSN_GPIO_INSTANCE   RTE_SPI0_SSN_GPIO_INSTANCE
#define SPI_SSN_GPIO_INDEX      RTE_SPI0_SSN_GPIO_INDEX
#define SPI_POLLING_DMA_SPLIT   14 // DMA: Greater than or equal to 14

#if (RTE_SPI0_IO_MODE == DMA_MODE)
#define SPIDMA_AND_OS_ENABLE
#endif
#ifndef FEATURE_SUBSYS_OPENHAL_SPI_ENABLE
#ifdef SPIDMA_AND_OS_ENABLE
static osEventFlagsId_t gSpiFlags = NULL;
#endif
#endif

#ifdef SPI_MUTEX_ENABLE
osMutexId_t         gSpiMutex        = NULL;
#endif
static FlashInfoT   gFlashUnverified = {0};
static uint16_t     gFlashIndex      = 0xFFFF;
static FlashInfoT   gFlashList[]     =
{
    {0x8513, 0x100000},   /* P25Q80H      1MB */
    {0x8514, 0x200000},   /* P25Q16SH     2MB */
    {0x8515, 0x400000},   /* P25Q32SH     4MB */
    {0x8516, 0x800000},   /* P25Q64SL     8MB */
    {0x2514, 0x200000},   /* SK25LE016    2MB */
    {0x2515, 0x400000},   /* SK25LE032    4MB */
    {0x2516, 0x800000},   /* SK25LE064    8MB */
    {0x2015, 0x400000},   /* XM25QH32CHIG 4MB */
    {0xC814, 0x200000},   /* MD25Q16CSIG  2MB */
    {0xC414, 0x200000},   /* GT25Q16A     2MB */
    {0xC415, 0x400000},   /* GT25Q32A     4MB */
    {0xC815, 0x800000},   /* GT25Q64A     8MB */
    {0xBA16, 0x800000},   /* Zetta64B     8MB */
};
#ifdef FEATURE_SUBSYS_OPENHAL_SPI_ENABLE
extern uint8_t api_spi_read_write_byte(uint32_t usrId, uint8_t dataOut);
static int32_t gOpenHalSpiHandle = -1;
#endif
static bool gWakeup = false;


static void restoreCallback(void *pdata, slpManLpState state)
{
    gWakeup = true;
}

static void spiFlashCsPinInit(void)
{
    PadConfig_t     padConfig  = {0};
    GpioPinConfig_t gpioConfig = {0};

    PAD_getDefaultConfig(&padConfig);
    padConfig.mux = PAD_MUX_ALT0;
    PAD_setPinConfig(RTE_SPI0_SSN_BIT, &padConfig);

    gpioConfig.pinDirection    = GPIO_DIRECTION_OUTPUT;
    gpioConfig.misc.initOutput = 1U;
    GPIO_pinConfig(RTE_SPI0_SSN_GPIO_INSTANCE, RTE_SPI0_SSN_GPIO_INDEX, &gpioConfig);
}

bool spiFlashExist(void)
{
    bool retVal = false;

#ifdef SPI_MUTEX_ENABLE
    osMutexAcquire(gSpiMutex, osWaitForever);
#endif
    if (spiFlashInfoGet() != NULL)
    {
        if (gWakeup == true)
        {
            gWakeup = false;
            spiFlashCsPinInit();
        }
        retVal = true;
    }
    else
    {

        SYSLOG_WARNING("No external Flash handle.\r\n");
    }
#ifdef SPI_MUTEX_ENABLE
    osMutexRelease(gSpiMutex);
#endif

    return retVal;
}

#ifndef FEATURE_SUBSYS_OPENHAL_SPI_ENABLE
#ifdef SPIDMA_AND_OS_ENABLE
/**
  \fn          void SPI_Callback()
  \brief       SPI callback
  \return
*/
static void SPI_Callback(uint32_t event)
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
#endif

/**
  * @brief Software SPI port initialization
  * @param[in] None
  * @return  None
  */
void spiIoInit(void)
{
#ifdef FEATURE_SUBSYS_OPENHAL_SPI_ENABLE
    if (gOpenHalSpiHandle == -1)
    {
        api_spi_create(0, NULL, &gOpenHalSpiHandle);
        api_spi_open(gOpenHalSpiHandle, NULL, 0);
        uint32_t data = SPI_FREQ;
        api_spi_ioctl(gOpenHalSpiHandle, OPEN_SPI_IOCTL_CLK, &data);
        data = ARM_SPI_CPOL0_CPHA0;
        api_spi_ioctl(gOpenHalSpiHandle, OPEN_SPI_IOCTL_MODE, &data);
    }
    // api_spi_ioctl(gOpenHalSpiHandle, OPEN_SPI_IOCTL_CONFIG, NULL);
#else
    // Initialize master spi
#ifdef SPIDMA_AND_OS_ENABLE
    if (gSpiFlags == NULL)
    {
        gSpiFlags = osEventFlagsNew(NULL);
    }
    spiMasterDrv->Initialize(SPI_Callback);
#else
    spiMasterDrv->Initialize(NULL);
#endif
    // Power on
    spiMasterDrv->PowerControl(ARM_POWER_FULL);

    // Configure master spi bus
    spiMasterDrv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL0_CPHA0 | ARM_SPI_DATA_BITS(TRANSFER_DATA_WIDTH) |
                          ARM_SPI_MSB_LSB     | ARM_SPI_SS_MASTER_SW, SPI_FREQ); // 76.8M Failure at High Temperatures
#endif
}

/**
  * @brief Software SPI_Flash bus driver basic function, send a single byte to MOSI,
  *        and accept MISO data at the same time.
  * @param[in] u8Data:Data sent on the MOSI data line
  * @return    u8Out: Data received on the MISO data line
  */
uint8_t spiFlashWriteByte(uint8_t u8Data)
{
    uint8_t u8Out = 0;

#ifdef FEATURE_SUBSYS_OPENHAL_SPI_ENABLE
    u8Out = api_spi_read_write_byte(gOpenHalSpiHandle, u8Data);
#else
    spiMasterDrv->Transfer(&u8Data, &u8Out, 1);
#endif

    return u8Out;
}

void spiFlashWriteCmdAndAddr(uint8_t cmd, uint32_t addr)
{
    uint8_t dataOut[4] = {cmd, (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, addr & 0xFF};

#ifdef FEATURE_SUBSYS_OPENHAL_SPI_ENABLE
    api_spi_write(gOpenHalSpiHandle, dataOut, sizeof(dataOut));
#else
    uint8_t dataIn[4]  = {0};
    spiMasterDrv->Transfer(dataOut, dataIn, sizeof(dataOut));
#endif
}

void spiCsSetHigh(void)
{
    GPIO_pinWrite(SPI_SSN_GPIO_INSTANCE, 1 << SPI_SSN_GPIO_INDEX, 1 << SPI_SSN_GPIO_INDEX);
}

void spiCsSetLow(void)
{
    GPIO_pinWrite(SPI_SSN_GPIO_INSTANCE, 1 << SPI_SSN_GPIO_INDEX, 0);
}



/**
  * @brief Flash Write Enable
  * @param[in] None
  * @return  None
  */
void spiFlashWriteEnable(void)
{
    spiCsSetLow();
    spiFlashWriteByte(W25X_WriteEnable);
    spiCsSetHigh();
}


/**
  * @brief Flash Write Disable
  * @param[in] None
  * @return  None
  */
void spiFlashWriteDisable(void)
{
    spiCsSetLow();
    spiFlashWriteByte(W25X_WriteDisable);
    spiCsSetHigh();
}


/**
  * @brief Read the BUSY of FLASH and wait if it is busy.
  *        The reason for BUSY is erase, or continuous read and write.
  * @param[in] None
  * @return  None
  */
void spiFlashWaitBusy(void)
{
    uint8_t resVal = 0;

    for (uint32_t i=0; i<1000; i++)
    {
#ifdef SPIDMA_AND_OS_ENABLE
        osDelay(1);
#else
        delay_us(1000);
#endif
        spiCsSetLow();
        spiFlashWriteByte(W25X_ReadStatusReg1);
        resVal = spiFlashWriteByte(Dummy_Byte1);
        spiCsSetHigh();
        if ((resVal & 0x03) == 0x00)
        {
            return;
        }
    }

    SYSLOG_DEBUG("Wait busy timeout.\r\n");
}


/**
  * @brief read FLASH BUSY Status
  * @param[in] None
  * @return  Free return NotBUSY，Busy return BUSY
  */
uint8_t spiFlashReadBusy(void)
{
    uint8_t u8Test;

    spiCsSetLow();
    spiFlashWriteByte(W25X_ReadStatusReg1);
    u8Test = spiFlashWriteByte(Dummy_Byte1);
    spiCsSetHigh();

    if (u8Test & 0x01)return (W25X_BUSY);
    else return (W25X_NotBUSY);
}


/**
  * @brief Erase the entire flash data
  * @param[in] None
  * @return  None
  */
void spiFlashEraseChip(void)
{
#ifdef SPI_MUTEX_ENABLE
    osMutexAcquire(gSpiMutex, osWaitForever);
#endif
    spiFlashWriteEnable();
    spiFlashWaitBusy();
    spiCsSetLow();
    spiFlashWriteByte(W25X_ChipErase);
    spiCsSetHigh();
    spiFlashWaitBusy();
    spiFlashWriteDisable();
#ifdef SPI_MUTEX_ENABLE
    osMutexRelease(gSpiMutex);
#endif
}


/**
  * @brief Erase a 32K or 64K block
  * @param[in] u32DataAddr  :Block first address to start erasing
  * @param[in] u8Mode       :Erase mode 1=32K other=64K
  * @return  None
  */
void spiFlashEraseBlock(uint32_t u32EraseAddr, uint8_t u8Mode)
{
#ifdef SPI_MUTEX_ENABLE
    osMutexAcquire(gSpiMutex, osWaitForever);
#endif
    spiFlashWriteEnable();
    spiFlashWaitBusy();
    spiCsSetLow();

    if (u8Mode == 1)
    {
        spiFlashWriteCmdAndAddr(W25X_BlockErase32K, u32EraseAddr);
    }
    else
    {
        spiFlashWriteCmdAndAddr(W25X_BlockErase64K, u32EraseAddr);
    }

    spiCsSetHigh();
    spiFlashWaitBusy();
    spiFlashWriteDisable();
#ifdef SPI_MUTEX_ENABLE
    osMutexRelease(gSpiMutex);
#endif
}


/**
  * @brief Erase a 4K sector
  * @param[in] u32DataAddr  :sector first address to start erasing
  * @return  None
  */
void spiFlashEraseSector(uint32_t u32EraseAddr)
{
#ifdef SPI_MUTEX_ENABLE
    osMutexAcquire(gSpiMutex, osWaitForever);
#endif
    spiFlashWriteEnable();
    spiCsSetLow();

    spiFlashWriteCmdAndAddr(W25X_SectorErase, u32EraseAddr);

    spiCsSetHigh();
    spiFlashWaitBusy();
    spiFlashWriteDisable();
#ifdef SPI_MUTEX_ENABLE
    osMutexRelease(gSpiMutex);
#endif
}


#if 0
/**
  * @brief Erase a page
  * @param[in] u32DataAddr  :page first address to start erasing
  * @return  None
  */
void spiFlashErasePage(uint32_t u32EraseAddr)
{
#ifdef SPI_MUTEX_ENABLE
    osMutexAcquire(gSpiMutex, osWaitForever);
#endif
    spiFlashWriteEnable();
    spiFlashWaitBusy();
    spiCsSetLow();

    spiFlashWriteCmdAndAddr(W25X_PageErase, u32EraseAddr);

    spiCsSetHigh();
    spiFlashWaitBusy();
    spiFlashWriteDisable();
#ifdef SPI_MUTEX_ENABLE
    osMutexRelease(gSpiMutex);
#endif
}
#endif


/**
  * @brief Start reading data of the specified length at the specified address
  * @param[in] u32ReadAddr       Start reading address(24bit)
  * @param[in] pu8Buffer         Data storage buffer
  * @param[in] u32NumByteToRead  The number of bytes to read
  * @return  None
  */
void spiFlashRead(uint32_t u32ReadAddr, uint8_t *pu8Buffer, uint32_t u32NumByteToRead)
{
#ifdef SPI_MUTEX_ENABLE
    osMutexAcquire(gSpiMutex, osWaitForever);
#endif
    spiCsSetLow();/* Enable chip select */
    spiFlashWriteCmdAndAddr(W25X_ReadData, u32ReadAddr);

#ifdef FEATURE_SUBSYS_OPENHAL_SPI_ENABLE
    api_spi_read(gOpenHalSpiHandle, pu8Buffer, u32NumByteToRead);
#else
#ifdef SPIDMA_AND_OS_ENABLE
    uint32_t index  = 0;
    uint32_t size   = 8191;
    uint32_t count  = u32NumByteToRead / size;
    uint32_t length = u32NumByteToRead % size;

    for (index=0; index<count; index++)
    {
        spiMasterDrv->Transfer(pu8Buffer, &pu8Buffer[index * size], size);
        if (osEventFlagsWait(gSpiFlags, 0x01, osFlagsWaitAll, 100) == osErrorTimeout)
        {
            SYSLOG_DEBUG("Read timeout.\r\n");
        }
    }

    if (length > 0)
    {
        spiMasterDrv->Transfer(pu8Buffer, &pu8Buffer[index * size], length);
        if ((length >= SPI_POLLING_DMA_SPLIT) && (osEventFlagsWait(gSpiFlags, 0x01, osFlagsWaitAll, 100) == osErrorTimeout))
        {
            SYSLOG_DEBUG("Read timeout.\r\n");
        }
    }
#else
    for (uint32_t i = 0; i < u32NumByteToRead; i++)
    {
        pu8Buffer[i] = spiFlashWriteByte(Dummy_Byte1); //Read one byte
    }
#endif
#endif

    spiCsSetHigh();/* Disable chip select */
#ifdef SPI_MUTEX_ENABLE
    osMutexRelease(gSpiMutex);
#endif
}

/**
  * @brief SPI starts writing data of up to 256 bytes at a specified address on one page (0~65535)
  * @param[in] u32WriteAddr:Address to start writing(24bit)
  * @param[in] pu8Buffer:Data storage buffer
  * @param[in] u16NumByteToWrite:The number of bytes to write (maximum 256),
  *            the number should not exceed the number of remaining bytes on the page!!!
  * @return  None
  */
void spiFlashWritePage(uint32_t u32WriteAddr, uint8_t *pu8Buffer, uint16_t u16NumByteToWrite)
{
#ifdef SPI_MUTEX_ENABLE
    osMutexAcquire(gSpiMutex, osWaitForever);
#endif
    spiFlashWriteEnable();
    spiCsSetLow();
    spiFlashWriteCmdAndAddr(W25X_PageProgram, u32WriteAddr);

#ifdef FEATURE_SUBSYS_OPENHAL_SPI_ENABLE
    api_spi_write(gOpenHalSpiHandle, pu8Buffer, u16NumByteToWrite);
#else
#ifdef SPIDMA_AND_OS_ENABLE
    uint8_t pu8RdBuffer[u16NumByteToWrite];
    spiMasterDrv->Transfer(pu8Buffer, pu8RdBuffer, u16NumByteToWrite);
    if ((u16NumByteToWrite >= SPI_POLLING_DMA_SPLIT) && (osEventFlagsWait(gSpiFlags, 0x01, osFlagsWaitAll, 100) == osErrorTimeout))
    {
        SYSLOG_DEBUG("Write page timeout.\r\n");
    }
#else
    for (uint16_t i = 0; i < u16NumByteToWrite; i++)
    {
        spiFlashWriteByte(pu8Buffer[i]);
    }
#endif
#endif

    spiCsSetHigh();
    spiFlashWaitBusy();
    spiFlashWriteDisable();
#ifdef SPI_MUTEX_ENABLE
    osMutexRelease(gSpiMutex);
#endif
}


/**
  * @brief Reads FLASH identification.
  * @param[in] None
  * @return  FLASH identification
  */
uint16_t spiFlashReadMdId(void)
{
    uint16_t u16Temp = 0;

#ifdef SPI_MUTEX_ENABLE
    osMutexAcquire(gSpiMutex, osWaitForever);
#endif
    /* Enable chip select */
    spiCsSetLow();
    /* Send "RDID " instruction */
    spiFlashWriteCmdAndAddr(W25X_ManufactDeviceID, 0);
    /* Read a byte from the FLASH */
    u16Temp |= spiFlashWriteByte(Dummy_Byte1) << 8;
    u16Temp |= spiFlashWriteByte(Dummy_Byte1);
    /* Disable chip select */
    spiCsSetHigh();
#ifdef SPI_MUTEX_ENABLE
    osMutexRelease(gSpiMutex);
#endif

    return u16Temp;
}

FlashInfoT *spiFlashInfoGet(void)
{
    FlashInfoT *info = NULL;

    if ((gFlashIndex >= 0) && (gFlashIndex < sizeof(gFlashList) / sizeof(gFlashList[0])))
    {
        info = &gFlashList[gFlashIndex];
    }
    else if ((gFlashUnverified.id != 0) && (gFlashUnverified.id != 0xFFFF))
    {
        info = &gFlashUnverified;
    }

    return info;
}

int32_t spiFlashInit(void)
{
    int32_t  retVal = -1;
    uint32_t size   = sizeof(gFlashList) / sizeof(gFlashList[0]);
    uint16_t id     = 0;

    slpManRegisterUsrdefinedRestoreCb(restoreCallback, NULL);

#ifdef SPI_MUTEX_ENABLE
    gSpiMutex = osMutexNew(NULL);
    if(gSpiMutex == NULL)
    {
        SYSLOG_EMERG("Failed to create mutex for gSpiMutex\r\n");
        goto labelEnd;
    }
#endif

    spiIoInit();
    id = spiFlashReadMdId();
    for (uint32_t i=0; i<size; i++)
    {
        if (id == gFlashList[i].id)
        {
            retVal      = 0;
            gFlashIndex = i;
            goto labelEnd;
        }
    }
    printf("flash id: 0x%04X\r\n", id);
    if ((id == 0x0000) || (id == 0x00FF) || (id == 0xFF00) || (id == 0xFFFF))
    {
        SYSLOG_WARNING("No external Flash.");
    }
    else
    {
        retVal                = 0;
        gFlashUnverified.id   = id;
        gFlashUnverified.size = 0;
        SYSLOG_WARNING("Unverified external Flash: %04X\r\n", id);
    }

labelEnd:
    return retVal;
}