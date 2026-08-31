#ifdef FEATURE_SUBSYS_PCM_ENABLE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "sctdef.h"
#include "cmsis_os2.h"
#include "audio.h"
#include "api_codec.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif


#define PCM_BUFFER_COUNT            2

#if defined (PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST==1)
static PLAT_FPSRAM_ZI_CUST __attribute__((aligned(16))) uint8_t gPcmBuffer[AUDIO_TX_TRANSFER_SIZE * PCM_BUFFER_COUNT] = {0};
#else
static __attribute__((aligned(16))) uint8_t gPcmBuffer[AUDIO_TX_TRANSFER_SIZE * PCM_BUFFER_COUNT] = {0};
#endif
static volatile bool gEnd = false;


void pcmEndPlay(void)
{
    gEnd = true;
}

int32_t pcmPlay(uint8_t *pcm, uint32_t length, uint32_t rate)
{
    int32_t   retVal    = -1;
    uint8_t  *buff      = NULL;
    uint32_t  len       = 0;
    uint32_t  pos       = 0;
    uint8_t   index     = 0;
    uint32_t  pcmTxSize = 0;
    uint8_t   channel   = MONO;
    uint8_t   factor    = 1;

    if ((pcm == NULL) || (length == 0))
    {
        SYSLOG_DEBUG("Param error.\r\n");
        goto labelEnd;
    }

#ifdef FEATURE_SUBSYS_CHANNEL_1TO2_ENABLE
    channel = DUAL_CHANNEL;
    factor  = 2;
#endif

    pcmTxSize = AUDIO_TX_TRANSFER_SIZE / factor;
    rate = sampleRateConvert(rate, true);
    if (rate == -1)
    {
        SYSLOG_DEBUG("rate error: %d\r\n", rate);
        goto labelEnd;
    }

    apiCodecSetPaState(true);
#ifdef PWM_CODEC_ENABLE
    osDelay(BOOT_TIME_PA);
#else
    apiCodecBoot();
    osDelay(70);
#endif

    apiCodecStart(rate, channel);
    gCodecTx = CODEC_TX_IDLE;

    gEnd = false;
    if (length == 1)
    {
        memset(gPcmBuffer, *pcm, AUDIO_TX_TRANSFER_SIZE);
        while (gEnd != true)
        {
            while (gCodecTx == CODEC_TX_START);
            apiCodecPlay((uint8_t *)gPcmBuffer, AUDIO_TX_TRANSFER_SIZE);
            gCodecTx = CODEC_TX_START;
            osDelay(5);
        }
    }
    else if (length <= pcmTxSize)
    {
        len = pcmTxSize / length * length;
        memset(gPcmBuffer, 0, len * factor);
        for (uint32_t i=0; i<len/2; i++)
        {
            for (uint32_t j=0; j<factor; j++)
            {
                ((uint16_t *)gPcmBuffer)[factor * i + j] = ((uint16_t *)pcm)[i % (length / 2)];
            }
        }

        while (gEnd != true)
        {
            while (gCodecTx == CODEC_TX_START);
            apiCodecPlay((uint8_t *)gPcmBuffer, len * factor);
            gCodecTx = CODEC_TX_START;
            osDelay(5);
        }
    }
    else
    {
        while (pos < length)
        {
            len = ((length - pos) >= pcmTxSize) ? pcmTxSize : (length - pos);
            buff = gPcmBuffer + index * pcmTxSize * factor;
            memset(buff, 0, len * factor);
            for (uint32_t i=0; i<len/2; i++)
            {
                for (uint32_t j=0; j<factor; j++)
                {
                    ((uint16_t *)buff)[factor * i + j] = ((uint16_t *)(pcm + pos))[i];
                }
            }
            while (gCodecTx == CODEC_TX_START);
            apiCodecPlay((uint8_t *)buff, len * factor);
            gCodecTx = CODEC_TX_START;
            index = (index + 1) % PCM_BUFFER_COUNT;
            pos   = pos + len;
            osDelay(5);
        }
    }

    apiCodecStop();
    retVal = 0;

labelEnd:
    return retVal;
}

uint32_t pcmSinGet(uint32_t freq, uint32_t rate, int16_t **data)
{
    uint32_t count  = rate / freq;
    uint32_t length = count * 2;

    if (length == 0)
    {
        printf("Param error: freq = %d, rate = %d\r\n", freq, rate);
        goto labelEnd;
    }

    *data  = malloc(length);
    if (*data != NULL)
    {
        for (uint32_t i=0; i<count; i++)
        {
            (*data)[i] = (int16_t)(32768 * sin(2 * 3.14 * i / count));
            // printf("%02X %02X ", ((*data)[i] & 0xFF), (((*data)[i] >> 8) & 0xFF));
        }
    }
    else
    {
        printf("Failed to malloc %d bytes for *data.\r\n", length);
        length = 0;
    }

labelEnd:
    return length;
}
#endif
