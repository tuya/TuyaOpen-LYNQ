#ifdef FEATURE_SUBSYS_WAV_ENABLE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "sctdef.h"
#include "cmsis_os2.h"
#include DEBUG_LOG_HEADER_FILE
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#include "flashex.h"
#include "audio.h"
#include "api_codec.h"
#include "wav.h"


#define WAV_BUFFER_COUNT            2
#define LIST_ID                     "LIST"
#define DATA_ID                     "data"

#if (PSRAM_EXIST==1)
static PLAT_FPSRAM_ZI_CUST __attribute__((aligned(16))) uint8_t gWavBuffer[AUDIO_TX_TRANSFER_SIZE * WAV_BUFFER_COUNT] = {0};
#else
static __attribute__((aligned(16))) uint8_t gWavBuffer[AUDIO_TX_TRANSFER_SIZE * WAV_BUFFER_COUNT] = {0};
#endif

extern osSemaphoreId_t gSpiCodecSemaphore;

int32_t wavPlay(char *path)
{
    int32_t    retVal    = -1;
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    FILE      *file      = NULL;
#endif
    WavHeadT   wavHead   = {0};
    uint8_t    *buffer   = NULL;
    uint8_t    *buff     = NULL;
    uint32_t   length    = 0;
    uint32_t   len       = 0;
    uint32_t   pos       = 0;
    uint8_t    index     = 0;
    uint32_t   wavTxSize = 0;
    uint8_t    channel   = MONO;
    uint8_t    factor    = 1;
    uint8_t    rate      = 0;
    bool       opened    = false;

    if (path == NULL)
    {
        SYSLOG_DEBUG("path is NULL.\r\n");
        goto labelEnd;
    }

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    file = file_fopen(path, "r");
    if (file == NULL)
#endif
    {
        SYSLOG_DEBUG("Failed to open the file \"%s\"\r\n", path);
        goto labelEnd;
    }

    opened = true;
    memset(&wavHead, 0, sizeof(wavHead));
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    file_fread(&wavHead, sizeof(wavHead), 1, file);
#endif
    if (memcmp(wavHead.listOrDataId, LIST_ID, strlen(LIST_ID)) == 0)
    {
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
        file_fseek(file, wavHead.listOrDataLength, SEEK_CUR);
        file_fread(wavHead.listOrDataId, (sizeof(wavHead.listOrDataId) + sizeof(wavHead.listOrDataLength)), 1, file);
#endif
    }
    else if (memcmp(wavHead.listOrDataId, DATA_ID, strlen(DATA_ID)) != 0)
    {
        SYSLOG_DEBUG("WAV format error.\r\n");
        goto labelEnd;
    }

#ifdef FEATURE_SUBSYS_CHANNEL_1TO2_ENABLE
    channel = DUAL_CHANNEL;
    if (wavHead.fmtChannels == 1)
    {
        factor = 2;
    }
#else
    channel = (wavHead.fmtChannels == 1) ? MONO : DUAL_CHANNEL;
#endif

    wavTxSize = AUDIO_TX_TRANSFER_SIZE / factor;
    buff      = malloc(wavTxSize);
    if (buff == NULL)
    {
        SYSLOG_DEBUG("Failed to malloc %d bytes for buff.\r\n", wavTxSize);
        goto labelEnd;
    }

    rate = sampleRateConvert(wavHead.fmtSampleRate, true);
    if (rate == -1)
    {
        SYSLOG_DEBUG("Failed to malloc %d bytes for buff.\r\n", wavTxSize);
        goto labelEnd;
    }

    apiCodecSetPaState(true);
    apiCodecBoot();
    osDelay(70);
    apiCodecStart(rate, channel);

    gCodecTx = CODEC_TX_IDLE;

    while (pos < wavHead.listOrDataLength)
    {
        length = ((wavHead.listOrDataLength - pos) >= wavTxSize) ? wavTxSize : (wavHead.listOrDataLength - pos);
        buffer = gWavBuffer + index * wavTxSize * factor;
        memset(buffer, 0, length * factor);
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
        file_fread(buff, length, 1, file);
#endif
        halI2sSrcAdjustVolumn((int16_t *)buff, length, audioGetVolume());
        len = length >> 1;
        for (uint32_t i=0; i<len; i++)
        {
            for (uint32_t j=0; j<factor; j++)
            {
                ((uint16_t *)buffer)[factor * i + j] = ((uint16_t *)buff)[i];
            }
        }
        while (gCodecTx == CODEC_TX_START);

#if defined(SPI_CODEC_ENABLE)
        if(gCodecTx == CODEC_TX_IDLE)
        {
            apiCodecPlay(buffer, length * factor);
        }
        else if (osSemaphoreAcquire(gSpiCodecSemaphore, osWaitForever) == osOK)
        {
            apiCodecPlay(buffer, length * factor);
        }
        gCodecTx = CODEC_TX_FINISH;
#else
        gCodecTx = CODEC_TX_START;
        apiCodecPlay(buffer, length * factor);
#endif
        index = (index + 1) % WAV_BUFFER_COUNT;
        pos   = pos + length;
    }

    while (gCodecTx == CODEC_TX_START)
    {
        osDelay(5);
    }

    apiCodecStop();
    retVal = 0;

labelEnd:
    if (opened == true)
    {
        opened = false;
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
        file_fclose(file);
#endif
    }
    if (buff != NULL)
    {
        free(buff);
        buff = NULL;
    }

    return retVal;
}
#endif
