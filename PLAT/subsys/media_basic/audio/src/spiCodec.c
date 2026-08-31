#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "bsp.h"
#include "slpman.h"
#include "charge.h"
#include "pwrkey.h"
#include "interpolVpu.h"
#include DEBUG_LOG_HEADER_FILE
#ifdef FEATURE_SUBSYS_STATUS_ENABLE
#include "status.h"
#endif
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#include "app.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_SUBSYS_SYSTIME_ENABLE
#include "systime.h"
#endif
#ifdef FEATURE_SUBSYS_AUDIO_ENABLE
#include "audio.h"
#endif
#include "spiCodec.h"
#ifdef FEATURE_SUBSYS_OPENHAL_SPI_ENABLE
#include "api_spi.h"
#endif


/**************************************** PWM ****************************************/
#define SAMPLE_RATE                     16000
#define BYTE_COUNT_IN_ONE_SAMPLE        (SPI_CODEC_FREQ / 8 / SAMPLE_RATE)
#define BYTE_COUNT_IN_ONE_CYCLE         25  // BYTE_COUNT_IN_ONE_SAMPLE / BYTE_COUNT_IN_ONE_CYCLE = CYCLE_REPEAT
#define CYCLE_REPEAT                    4   // BYTE_COUNT_IN_ONE_SAMPLE / BYTE_COUNT_IN_ONE_CYCLE = CYCLE_REPEAT


osSemaphoreId_t gSpiCodecSemaphore  = NULL;
static uint8_t  gPingPongIndex      = 0;
static bool     gPlayStarted        = false;
#if (PSRAM_EXIST == 1)
static PLAT_FPSRAM_ZI_CUST uint8_t gSpiDMABuffer[TOTAL_CHAIN_COUNT][ONE_CHAIN_BUFFER_SIZE] = {0};
#else
static uint8_t gSpiDMABuffer[TOTAL_CHAIN_COUNT][ONE_CHAIN_BUFFER_SIZE] = {0};
#endif
#ifdef FEATURE_SUBSYS_OPENHAL_SPI_ENABLE
static int32_t gOpenHalSpiHandle = -1;
#else
#define TRANSFER_DATA_WIDTH (8)
extern ARM_DRIVER_SPI Driver_SPI1;
static ARM_DRIVER_SPI *spiMasterDrv = &CREATE_SYMBOL(Driver_SPI, 1);
#endif


/**************************************** SPI begin ****************************************/
PLAT_FM_RAMCODE void spiCodecCallback(uint32_t event)
{
    if ((event & ARM_SPI_EVENT_TRANSFER_COMPLETE) != 0)
    {
        osSemaphoreRelease(gSpiCodecSemaphore);
    }
    // ECPLAT_PRINTF(UNILOG_PLA_APP, spiCodecCallback, P_DEBUG, "event1=%X", event);
}

static void spiInit(void)
{
#ifdef FEATURE_SUBSYS_OPENHAL_SPI_ENABLE
    if (gOpenHalSpiHandle == -1)
    {
        api_spi_create(1, NULL, (void *)&gOpenHalSpiHandle);
        api_spi_open(gOpenHalSpiHandle, NULL, 0);
        uint32_t data = SPI_CODEC_FREQ;
        api_spi_ioctl(gOpenHalSpiHandle, OPEN_SPI_IOCTL_CLK, &data);
        data = ARM_SPI_CPOL0_CPHA1;
        api_spi_ioctl(gOpenHalSpiHandle, OPEN_SPI_IOCTL_MODE, &data);
    }
    // api_spi_ioctl(gOpenHalSpiHandle, OPEN_SPI_IOCTL_CONFIG, NULL);  // todo fix
#else
    spiMasterDrv->Initialize(NULL);
    spiMasterDrv->PowerControl(ARM_POWER_FULL);
    spiMasterDrv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL0_CPHA1 | ARM_SPI_DATA_BITS(TRANSFER_DATA_WIDTH) |
                          ARM_SPI_MSB_LSB     | ARM_SPI_SS_MASTER_SW, SPI_CODEC_FREQ);
#endif
}

static void startSpi(void)
{
#ifdef FEATURE_SUBSYS_OPENHAL_SPI_ENABLE
    uint32_t para[] = {(uint32_t)gSpiDMABuffer, ONE_CHAIN_BUFFER_SIZE};
    api_spi_ioctl(gOpenHalSpiHandle, OPEN_SPI_IOCTL_START_SEND, (void *)para);
#else
    spiMasterDrv->Send(gSpiDMABuffer, ONE_CHAIN_BUFFER_SIZE);
#endif
}

void spiCodecStop(void)
{
#ifdef FEATURE_SUBSYS_OPENHAL_SPI_ENABLE
    api_spi_ioctl(gOpenHalSpiHandle, OPEN_SPI_IOCTL_STOP_SEND, NULL);
#else
    spiMasterDrv->StopSend();
#endif
}

void spiCodecStart(void)
{
    gPingPongIndex = 0;
    gPlayStarted   = false;
}

void spiCodecInit(void)
{
    spiInit();
    InterpolVpuInit();
    *(volatile unsigned int *)(0x40000070) = 1;
    if (gSpiCodecSemaphore == NULL)
    {
        gSpiCodecSemaphore = osSemaphoreNew(1, 0, NULL);
        if (gSpiCodecSemaphore == NULL)
        {
            SYSLOG_EMERG("Failed to create semaphore for gSpiCodecSemaphore.\r\n");
        }
    }
}
/**************************************** SPI end ****************************************/

/**************************************** PWM begin ****************************************/
PLAT_FM_RAMCODE void spiPwmCodecPlay(int16_t *buffer, uint32_t length)
{
    uint16_t  pcmDataAfterOffset = 0;
    uint32_t  dutyCycleByteCount = 0;
    uint32_t  sampleStartIndex   = 0;
    uint8_t  *spiDmaBuffer       = NULL;

    spiDmaBuffer   = gSpiDMABuffer[gPingPongIndex * HALF_CHAIN_COUNT];
    gPingPongIndex = (gPingPongIndex + 1) % 2;

    memset(spiDmaBuffer, 0, HALF_CHAIN_BUFFER_SIZE);
    length /= 2;
    for (uint32_t i=0; i<length; i++)
    {
        pcmDataAfterOffset = (uint16_t)((buffer[i] + 32768));
        dutyCycleByteCount = (uint32_t)((uint32_t)(BYTE_COUNT_IN_ONE_CYCLE) * pcmDataAfterOffset) >> 16;
        sampleStartIndex   = i * BYTE_COUNT_IN_ONE_SAMPLE;
        memset(&spiDmaBuffer[sampleStartIndex], 0xFF, dutyCycleByteCount);
        for (uint32_t j=1; j<CYCLE_REPEAT; j++)
        {
            memcpy(&spiDmaBuffer[sampleStartIndex + j * BYTE_COUNT_IN_ONE_CYCLE], &spiDmaBuffer[sampleStartIndex], BYTE_COUNT_IN_ONE_CYCLE);
        }
    }

    if (gPlayStarted == false)
    {
        if (gPingPongIndex == 0)
        {
            gPlayStarted = true;
            startSpi();
        }
        else
        {
            osSemaphoreRelease(gSpiCodecSemaphore);
        }
    }
}
/**************************************** PWM end ****************************************/

/**************************************** sigmaDelta begin ****************************************/
PLAT_FM_RAMCODE void pcm_interpolate_325_times_65(int16_t *input, uint8_t *out_data, int input_size)
{
    int i, j;

    static int32_t reg = {0};
    static int32_t sigma_reg ={0};
    static int32_t out_temp={0};
    static int32_t out_data_reg={0};
    static int32_t dataCnt;
    int32_t dataA,dataB;
    int32_t byteIndex;

    dataCnt = 0;
    byteIndex = 0;

    for (i = 0; i < input_size ; ++i)
    {
        dataA = (reg<<2); dataB = (input[i] - reg)*252;
        for (j = 0; j < 65; ++j)
        {
            sigma_reg = sigma_reg + ((dataA + ((dataB*j)>>12) - out_temp)>>3) ;
            if(sigma_reg >0)
            {
                out_temp = 5*32768;
                out_data_reg = (out_data_reg<<5) | 0x1F;
                
            }
            else
            {
                out_temp = -5*32768;
                out_data_reg = (out_data_reg<<5);
            }
            
            dataCnt += 5;
            if (dataCnt >=8)
            {
                dataCnt -= 8;
                out_data[byteIndex] = (out_data_reg>>dataCnt)&0xFF;
                byteIndex ++ ;
                out_data_reg &= 0xFFFF;
            }

        }
        reg = input[i];
    }
}

PLAT_FM_RAMCODE void pcm_interpolate_320_times_64(int16_t *input, uint8_t *out_data, int input_size)
{
    int i, j;
    static int32_t reg = {0};
    static int32_t sigma_reg ={0};
    static int32_t out_temp={0};
    static int32_t out_data_reg={0};
    static int32_t dataCnt;
    int32_t dataA,dataB;
    int32_t byteIndex;

    dataCnt = 0;
    byteIndex = 0;

    for (i = 0; i < input_size ; ++i)
    {
        dataA = (reg<<2); dataB = (input[i] - reg);
        for (j = 0; j < 64; ++j)
        {
            sigma_reg = sigma_reg + ((dataA + ((dataB*j)>>4) - out_temp)>>3) ;
            if(sigma_reg >0)
            {
                out_temp = 5*32768;
                out_data_reg = (out_data_reg<<5) | 0x1F;
                
            }
            else
            {
                out_temp = -5*32768;
                out_data_reg = (out_data_reg<<5);
            }
            
            dataCnt += 5;
            if (dataCnt >=8)
            {
                dataCnt -= 8;
                out_data[byteIndex] = (out_data_reg>>dataCnt)&0xFF;
                byteIndex ++ ;
                out_data_reg &= 0xFFFF;
            }

        }
        reg = input[i];
    }
}

PLAT_FM_RAMCODE void pcm_interpolate_320_times_40(int16_t *input, uint8_t *out_data, int input_size)
{
    int i, j;
    static int32_t reg = {0};
    static int32_t sigma_reg ={0};
    static int32_t out_temp={0};
    int32_t dataA,dataB;
    int32_t byteIndex = 0;

    for (i = 0; i < input_size ; ++i)
    {
        dataA = (reg<<2); dataB = (input[i] - reg)*410;
        for (j = 0; j < 40; ++j)
        {
            sigma_reg = sigma_reg + ((dataA + ((dataB*j)>>12) - out_temp)>>3);
            if(sigma_reg >0)
            {
                out_temp = 5*32768;
                out_data[byteIndex++] = 0xFF;
                
            }
            else
            {
                out_temp = -5*32768;
                out_data[byteIndex++] = 0x00;
            }
        }
        reg = input[i];
    }
}

PLAT_FM_RAMCODE void pcm_interpolate_320_times_80(int16_t *input, uint8_t *out_data, int input_size)
{
    int i, j;
    static int32_t reg = {0};
    static int32_t sigma_reg ={0};
    static int32_t out_temp={0};
    static int32_t out_data_reg={0};
    static int32_t dataCnt;
    int32_t dataA,dataB;
    int32_t byteIndex;

    dataCnt = 0;
    byteIndex = 0;

    for (i = 0; i < input_size ; ++i)
    {
        dataA = (reg<<2); dataB = (input[i] - reg)*204; // 16384 * (1/80)
        for (j = 0; j < 80; ++j)
        {
            // sigma_reg = sigma_reg + ((dataA + ((dataB*j)>>4) - out_temp)>>3);
            sigma_reg = sigma_reg + ((dataA + ((dataB*j)>>12) - out_temp)>>3);

            if(sigma_reg >0)
            {
                out_temp = 5*32767 << 1;
                out_data_reg = (out_data_reg<<4) | 0xF;  // shift = 4 (320/80),
            }
            else
            {
                out_temp = -5*32767 << 1;
                out_data_reg = (out_data_reg<<4) | 0x0;
            }

            dataCnt += 4;   // (320/80)
            if (dataCnt >=8)
            {
                dataCnt -= 8;
                out_data[byteIndex] = (out_data_reg>>dataCnt)&0xFF;
                byteIndex ++ ;
                out_data_reg &= 0xFFFF;
            }

        }
        reg = input[i];
    }
}

PLAT_FM_RAMCODE void pcm_interpolate_320_times_160(int16_t *input, uint8_t *out_data, int input_size)
{
    int i, j;
    static int32_t reg = {0};
    static int32_t sigma_reg ={0};
    static int32_t out_temp={0};
    static int32_t out_data_reg={0};
    static int32_t dataCnt;
    int32_t dataA,dataB;
    int32_t byteIndex;

    dataCnt = 0;
    byteIndex = 0;

    for (i = 0; i < input_size ; ++i)
    {
        dataA = (reg<<2); dataB = (input[i] - reg)*102; // 16384 * (1/80)
        for (j = 0; j < 160; ++j)
        {
            // sigma_reg = sigma_reg + ((dataA + ((dataB*j)>>4) - out_temp)>>3);
            sigma_reg = sigma_reg + ((dataA + ((dataB*j)>>12) - out_temp)>>3);

            if(sigma_reg >0)
            {
                out_temp = 5*32767 << 1;
                out_data_reg = (out_data_reg<<2) | 0x3;  // shift = 2 (320/160),
            }
            else
            {
                out_temp = -5*32767 << 1;
                out_data_reg = (out_data_reg<<2) | 0x0;
            }

            dataCnt += 2;   // (320/160)
            if (dataCnt >=8)
            {
                dataCnt -= 8;
                out_data[byteIndex] = (out_data_reg>>dataCnt)&0xFF;
                byteIndex ++ ;
                out_data_reg &= 0xFFFF;
            }

        }
        reg = input[i];
    }
}

PLAT_FM_RAMCODE void pcm_interpolate_160_times_20(int16_t *input, uint8_t *out_data, int input_size)
{
    int i, j;
    static int32_t reg = {0};
    static int32_t sigma_reg ={0};
    static int32_t out_temp={0};
    int32_t dataA,dataB;
    int32_t byteIndex = 0;

    for (i = 0; i < input_size ; ++i)
    {
        dataA = (reg<<2); dataB = (input[i] - reg)*820; // 410 = 16384*(1/40)
        for (j = 0; j < 20; ++j)
        {
            sigma_reg = sigma_reg + ((dataA + ((dataB*j)>>12) - out_temp)>>3);
            if(sigma_reg >0)
            {
                out_temp = 5*32768;
                out_data[byteIndex++] = 0xFF;
            }
            else
            {
                out_temp = -5*32768;
                out_data[byteIndex++] = 0x00;
            }
        }
        reg = input[i];
    }
}

PLAT_FM_RAMCODE void pcm_interpolate_160_times_40(int16_t *input, uint8_t *out_data, int input_size)
{
    int i, j;
    static int32_t reg = {0};
    static int32_t sigma_reg ={0};
    static int32_t out_temp={0};
    static int32_t out_data_reg={0};
    static int32_t dataCnt;
    int32_t dataA,dataB;
    int32_t byteIndex;

    dataCnt = 0;
    byteIndex = 0;

    for (i = 0; i < input_size ; ++i)
    {
        dataA = (reg<<2); dataB = (input[i] - reg)*400; // 16384 * (1/40)
        for (j = 0; j < 40; ++j)
        {
            // sigma_reg = sigma_reg + ((dataA + ((dataB*j)>>4) - out_temp)>>3);
            sigma_reg = sigma_reg + ((dataA + ((dataB*j)>>12) - out_temp)>>3);

            if(sigma_reg >0)
            {
                out_temp = 5*32000;
                out_data_reg = (out_data_reg<<4) | 0xF;  // shift = 2 (160/80),
            }
            else
            {
                out_temp = -5*32000;
                out_data_reg = (out_data_reg<<4) | 0x0;
            }

            dataCnt += 4;   // (320/160)
            if (dataCnt >=8)
            {
                dataCnt -= 8;
                out_data[byteIndex] = (out_data_reg>>dataCnt)&0xFF;
                byteIndex ++ ;
                out_data_reg &= 0xFFFF;
            }

        }
        reg = input[i];
    }
}

PLAT_FM_RAMCODE void pcm_interpolate_160_times_80(int16_t *input, uint8_t *out_data, int input_size)
{
    int i, j;
    static int32_t reg = {0};
    static int32_t sigma_reg ={0};
    static int32_t out_temp={0};
    static int32_t out_data_reg={0};
    static int32_t dataCnt;
    int32_t dataA,dataB;
    int32_t byteIndex;

    dataCnt = 0;
    byteIndex = 0;

    for (i = 0; i < input_size ; ++i)
    {
        dataA = (reg<<2); dataB = (input[i] - reg)*204; // 16384 * (1/80)
        for (j = 0; j < 80; ++j)
        {
            // sigma_reg = sigma_reg + ((dataA + ((dataB*j)>>4) - out_temp)>>3);
            sigma_reg = sigma_reg + ((dataA + ((dataB*j)>>12) - out_temp)>>3);

            if(sigma_reg >0)
            {
                out_temp = 5*32767 ;
                out_data_reg = (out_data_reg<<2) | 0x3;  // shift = 2 (160/80),
            }
            else
            {
                out_temp = -5*32767 ;
                out_data_reg = (out_data_reg<<2) | 0x0;
            }

            dataCnt += 2;   // (320/160)
            if (dataCnt >=8)
            {
                dataCnt -= 8;
                out_data[byteIndex] = (out_data_reg>>dataCnt)&0xFF;
                byteIndex ++ ;
                out_data_reg &= 0xFFFF;
            }

        }
        reg = input[i];
    }
}

PLAT_FM_RAMCODE void pcm_interpolate_325_times_25(int16_t *input, uint8_t *out_data, int input_size)
{
     int i, j;

    static int32_t reg = {0};
    static int32_t sigma_reg ={0};
    static int32_t out_temp={0};
    static int32_t out_data_reg={0};
    static int32_t dataCnt;
    int32_t dataA,dataB;
    int32_t byteIndex;

    dataCnt = 0;
    byteIndex = 0;

    for (i = 0; i < input_size ; ++i)
    {
        dataA = (reg<<2); dataB = (input[i] - reg)*655;
        for (j = 0; j < 25; ++j)
        {
            sigma_reg = sigma_reg + ((dataA + ((dataB*j)>>12) - out_temp)>>3) ;
            if(sigma_reg >0)
            {
                out_temp = 5*32768;
                out_data_reg = (out_data_reg<<13) | 0x1FFF;
                
            }
            else
            {
                out_temp = -5*32768;
                out_data_reg = (out_data_reg<<13);
            }
            
            dataCnt += 13;
            while (dataCnt >=8)
            {
                dataCnt -= 8;
                out_data[byteIndex] = (out_data_reg>>dataCnt)&0xFF;
                byteIndex ++ ;
                out_data_reg &= 0xFFFF;
            }

        }
        reg = input[i];
    }
}

PLAT_FM_RAMCODE void pcm_interpolate_325_times_325(int16_t *input, uint8_t *out_data, int input_size) 
{
    int i, j;
    static int32_t reg = {0};
    static int32_t sigma_reg ={0};
    static int32_t out_temp={0};
    int32_t dataA,dataB,dataCnt;
    int32_t byteIndex,bitIndex;

    dataCnt = 0;

    for (i = 0; i < input_size ; ++i)
    {
        dataA = (reg<<2); dataB = (input[i] - reg)*50;
        for (j = 0; j < 325; ++j)
        {
            sigma_reg = sigma_reg + ((dataA + ((dataB*j)>>12) - out_temp)>>3) ;
            byteIndex = dataCnt>>3; bitIndex = dataCnt&0x7;
            if(sigma_reg >0)
            {
                out_temp = 5<<16;
                out_data[byteIndex] = out_data[byteIndex] | (1<<bitIndex);
            }
            else
            {
                out_temp = -5<<16;
                out_data[byteIndex] = out_data[byteIndex] & (~(1<<bitIndex));
            }
            dataCnt++;
        }

        reg = input[i];
    }
}

void pcm_interpolate_325_times_new(int16_t *input, int16_t *output, int input_size)
{
    int i, j;
    static int reg = 0;

    for (i = 0; i< input_size; i++)
    {
        for (j = 0; j < 325; j++)
        {
            output[i * 325 + j] = (reg *  (325 - j) + input[i] * j) / 325;
        }
        reg = input[i];

    }
}

static const int16_t filterCoeffs[41] = {0, -8, -13, -14, -9, 0, 43, 74, 80, 55, 0, -146, -261, -298, -271, 0, 467, 992, 1487, 1863, 2048, 1863, 1487, 992, 467, 0, -217, -298, -261, -146, 0, 55, 80, 74, 43, 0, -9, -14, -13, -8, 0};
PLAT_FM_RAMCODE void fir_filter_old(int16_t *input, int16_t *output, int input_length, int filter_length)
{
    int32_t temp=0;
    int32_t temp_out=0;

    for (int n = 0; n < input_length; n++)
    {
        output[n] = 0;
        temp_out=0;
        for (int i = 0; i < filter_length; i++)
        {
            if (n - i >= 0)
            {
                temp = input[n - i];
                temp_out += (temp* filterCoeffs[i]);
            }
        }

        if ((temp_out>>11) > 32767)
        {
            output[n] = 32767;
        }
        else if ((temp_out>>11) < -32767)
        {
            output[n] = -32767;
        }
        else
        {
            output[n] = (temp_out>>11);
        }
    }
}

void fir_filter(int16_t *input, int16_t *output, int input_length, int filter_length) {
    int32_t temp_out=0;
    static int32_t reg[41] = {0};

    for (int n = 0; n < input_length; n++) {
        temp_out=0;
        for (int i = filter_length-1; i > 0; i--)
        {
            reg[i] = reg[i-1];
        }
        reg[0] = input[n];
        for (int i = 0; i < filter_length; i++)
        {
            temp_out += reg[i]*filterCoeffs[i];
        }

        output[n] = (temp_out>>11);
    }
}

void fir_filter_new(int16_t *input, int16_t *output, int input_length, int filter_length) {
    int32_t temp_out=0;
    static int32_t reg[9] = {0};

    for (int n = 0; n < input_length; n++) 
    {
        for (int i = 8; i > 0; i--)
        {
            reg[i] = reg[i-1];
        }
        reg[0] = input[n];

        temp_out = 0;
        for (int i = 0; i < 9; i++)
        {
            temp_out += reg[i]*filterCoeffs[5*i+0];
        }
        output[5*n+0] = (temp_out>>11);

        temp_out = 0;
        for (int i = 0; i < 8; i++)
        {
            temp_out += reg[i]*filterCoeffs[5*i+1];
        }
        output[5*n+1] = (temp_out>>11);

        temp_out = 0;
        for (int i = 0; i < 8; i++)
        {
            temp_out += reg[i]*filterCoeffs[5*i+2];
        }
        output[5*n+2] = (temp_out>>11);

        temp_out = 0;
        for (int i = 0; i < 8; i++)
        {
            temp_out += reg[i]*filterCoeffs[5*i+3];
        }
        output[5*n+3] = (temp_out>>11);

        temp_out = 0;
        for (int i = 0; i < 8; i++)
        {
            temp_out += reg[i]*filterCoeffs[5*i+4];
        }
        output[5*n+4] = (temp_out>>11);
    }
}

PLAT_FM_RAMCODE void spiSigmadeltaCodecPlay(int16_t *buffer, uint32_t length)
{
    uint32_t  upFactor        = 5;
    uint32_t  upSampledLength = length * upFactor;
    uint32_t  upSampledLen    = upSampledLength / 2;
    int16_t  *filteredData    = NULL;
    uint8_t  *sigmaDeltaData  = NULL;

    if ((buffer == NULL) || (length < 2))
    {
        SYSLOG_ERR("Param error.\r\n");
        goto labelEnd;
    }

    filteredData = (int16_t *)malloc(upSampledLength);
    if (filteredData == NULL)
    {
        SYSLOG_ERR("Failed to malloc %d bytes for filteredData.\r\n", upSampledLength);
        goto labelEnd;
    }

    sigmaDeltaData = gSpiDMABuffer[gPingPongIndex * HALF_CHAIN_COUNT];
    gPingPongIndex = (gPingPongIndex + 1) % 2;

    InterpolVpu(buffer, filteredData, length / 2, filterCoeffs, sizeof(filterCoeffs) / sizeof(filterCoeffs[0]), upFactor);
    pcm_interpolate_320_times_64(filteredData, sigmaDeltaData, upSampledLen);

    if (gPlayStarted == false)
    {
        if (gPingPongIndex == 0)
        {
            gPlayStarted = true;
            startSpi();
        }
        else
        {
            osSemaphoreRelease(gSpiCodecSemaphore);
        }
    }

labelEnd:
    if (filteredData != NULL)
    {
        free(filteredData);
        filteredData = NULL;
    }
}
/**************************************** sigmaDelta end ****************************************/
