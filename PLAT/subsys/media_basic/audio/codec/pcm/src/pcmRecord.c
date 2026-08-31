#ifdef FEATURE_SUBSYS_PCM_RECORD_ENABLE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "sctdef.h"
#include "cmsis_os2.h"
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#include "api_codec.h"
#include "flashex.h"
#include "audio.h"
#include "wav.h"


#define BUFFER_COUNT            2
#define LENGTH_MAX              60000


#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1))
static PLAT_FPSRAM_ZI_CUST uint16_t gRecvBuff[BUFFER_COUNT][AUDIO_RX_TRANSFER_SIZE / 2];
#else
static uint16_t gRecvBuff[BUFFER_COUNT][AUDIO_RX_TRANSFER_SIZE / 2];
#endif


int32_t pcmRecord(char *path)
{
    int32_t   resVal     = -1;
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    FILE     *file       = NULL;
    uint32_t  size       = 0;
#endif
    uint8_t   indexRecv  = 0;
    uint8_t   indexWrite = 0;
    WavHeadT  wavHead    =
    {
        .riffId             = {'R', 'I', 'F', 'F'},
        .riffSize           = sizeof(wavHead) - sizeof(wavHead.riffId) - sizeof(wavHead.riffSize),
        .riffType           = {'W', 'A', 'V', 'E'},
        .fmtId              = {'f', 'm', 't', ' '},
        .fmtSize            = 16,
        .fmtCompressionCode = 1,
        .fmtChannels        = 1,
#if (defined(FEATURE_SUBSYS_G726_ENABLE) || defined(FEATURE_SUBSYS_G711_ENABLE))
        .fmtSampleRate      = 8000,
        .fmtBytesPerSec     = 16000,
#else
        .fmtSampleRate      = 16000,
        .fmtBytesPerSec     = 32000,
#endif
        .fmtBlockAlign      = 2,
        .fmtBitPerSample    = 16,
        .listOrDataId       = {'d', 'a', 't', 'a'},
        .listOrDataLength   = 0
    };

    if (path == NULL)
    {
        SYSLOG_DEBUG("Param error.\r\n");
        goto labelEnd;
    }

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    file = file_fopen(path, "w");
#endif
    if (file == NULL)
    {
        SYSLOG_DEBUG("Failed to open the file \"%s\"\r\n", path);
        goto labelEnd;
    }

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    file_fwrite(&wavHead, sizeof(wavHead), 1, file);
#endif

    apiCodecBoot();
    osDelay(BOOT_TIME_CODEC);
    apiCodecStart(sampleRateConvert(wavHead.fmtSampleRate, true), MONO);
    gCodecRx = CODEC_RX_IDLE;

    while (size < LENGTH_MAX)
    {
        memset(gRecvBuff[indexRecv], 0, sizeof(gRecvBuff[indexRecv]));
        while (gCodecRx == CODEC_RX_START);

        apiCodecRecord((uint8_t *)gRecvBuff[indexRecv], sizeof(gRecvBuff[indexRecv]));
        indexRecv = (indexRecv + 1) % BUFFER_COUNT;

        if (gCodecRx == CODEC_RX_FINISH)
        {
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
            size += file_fwrite(gRecvBuff[indexWrite], 1, sizeof(gRecvBuff[indexWrite]), file);
#endif
            indexWrite  = (indexWrite + 1) % BUFFER_COUNT;
        }

        gCodecRx = CODEC_RX_START;
        osDelay(5);
    }

    wavHead.listOrDataLength = size - sizeof(wavHead);
    wavHead.riffSize        += wavHead.listOrDataLength;
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    file_fseek(file, 0, SEEK_SET);
    file_fwrite(&wavHead, sizeof(wavHead), 1, file);
    file_fclose(file);
#endif

    resVal = 0;

labelEnd:
    return resVal;
}
#endif
