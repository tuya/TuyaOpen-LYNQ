#ifdef FEATURE_SUBSYS_MP3_ENABLE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "sctdef.h"
#include "cmsis_os2.h"
#include "mad.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
#include "flashex.h"
#endif
#include "audio.h"

#include "codecDrv.h"
#include "api_codec.h"
#include "hal_i2s.h"


#define INPUT_BUFFER_SIZE                       1500
#ifdef FEATURE_SUBSYS_CHANNEL_1TO2_ENABLE
#define FACTOR_1TO2                             2
#endif
#ifdef FEATURE_SUBSYS_SAMPLE_8TO48_16TO48_ENABLE
#define FACTOR_8TO48                            6
#define FACTOR_16TO48                           3
#endif
#if (defined(FEATURE_SUBSYS_CHANNEL_1TO2_ENABLE) && defined(FEATURE_SUBSYS_SAMPLE_8TO48_16TO48_ENABLE))
#define FACTOR                                  (FACTOR_1TO2 * FACTOR_8TO48)
#elif defined(FEATURE_SUBSYS_CHANNEL_1TO2_ENABLE)
#define FACTOR                                  FACTOR_1TO2
#elif defined(FEATURE_SUBSYS_SAMPLE_8TO48_16TO48_ENABLE)
#define FACTOR                                  FACTOR_8TO48
#else
#define FACTOR                                  1
#endif
#define OUTPUT_BUFFER_SIZE                      (AUDIO_TX_TRANSFER_SIZE / 2 / FACTOR * FACTOR)
#define MP3_FIRST_BUFFER_CONSUME                39
#define MP3_PREDATA_DURATION                    100
#define PLAY_ABORT_CHECK()                      if (audioPlayIsStop() == true) \
                                                { \
                                                    goto labelEnd; \
                                                }

extern osSemaphoreId_t gSpiCodecSemaphore;

static volatile uint16_t *audioBuff  = NULL;
static volatile uint32_t outBufIndex = 0;
static struct mad_stream stream = {0};
static struct mad_frame  frame = {0};
static struct mad_synth  synth = {0};
#if (PSRAM_EXIST == 1)
static PLAT_FPSRAM_ZI_CUST uint8_t  inputBuffer[INPUT_BUFFER_SIZE + MAD_BUFFER_GUARD]; // Huffman decoding needs guard buffer
static PLAT_FPSRAM_ZI_CUST uint16_t outputBuffer[2][OUTPUT_BUFFER_SIZE];
#else
static uint8_t  inputBuffer[INPUT_BUFFER_SIZE + MAD_BUFFER_GUARD]; // Huffman decoding needs guard buffer
static uint16_t outputBuffer[2][OUTPUT_BUFFER_SIZE];
#endif


// Converts a sample from libmad's fixed point number format to a signed short (16 bits)
static inline signed short madFixedToSignedShort(mad_fixed_t sample)
{
    // round
    sample += (1L << (MAD_F_FRACBITS - 16));

    // clip
    if (sample >= MAD_F_ONE)
        sample = MAD_F_ONE - 1;
    else if (sample < -MAD_F_ONE)
        sample = -MAD_F_ONE;

    // quantize 
    return sample >> (MAD_F_FRACBITS + 1 - 16);
}

int32_t mp3Decoder(FILE *file, I2sSampleRate_e rate, bool increase, bool dual)
{
    struct stat buf = {0};
    uint32_t outputBufferSize = OUTPUT_BUFFER_SIZE;
    uint16_t *outputPtr = &outputBuffer[0][0];
    uint16_t *outputBufferEnd = &outputBuffer[0][0] + OUTPUT_BUFFER_SIZE;
    int32_t  status=0, i;
    uint32_t readSize, remaining;
    uint8_t  *readStart = NULL;
    uint8_t  channel    = MONO;
    uint8_t  factor     = 1;

#ifdef FEATURE_SUBSYS_CHANNEL_1TO2_ENABLE
    if (dual == true)
    {
        channel = DUAL_CHANNEL;
        factor *= FACTOR_1TO2;
    }
#endif

#ifdef FEATURE_SUBSYS_SAMPLE_8TO48_16TO48_ENABLE
    if (increase == true)
    {
        if (rate == SAMPLERATE_8K)
        {
            factor *= FACTOR_8TO48;
            rate    = SAMPLERATE_48K;
        }
        else if (rate == SAMPLERATE_16K)
        {
            factor *= FACTOR_16TO48;
            rate    = SAMPLERATE_48K;
        }
    }
#endif

	mad_stream_init(&stream);
	mad_frame_init(&frame);
	mad_synth_init(&synth);

	outBufIndex = 0;
	gCodecTx  = CODEC_TX_IDLE;

	do
	{
		if(stream.buffer == NULL || stream.error == MAD_ERROR_BUFLEN)
		{
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
            file_fstat((int)file, &buf);
            if (file_ftell(file) == buf.st_size)
#endif
            {
                break;
            }

			if(stream.next_frame != NULL)
			{
				remaining = stream.bufend - stream.next_frame;
				memmove(inputBuffer, stream.next_frame, remaining);
				readStart = inputBuffer + remaining;
				readSize = INPUT_BUFFER_SIZE - remaining;
			}
			else // First frame
			{
				readSize  = INPUT_BUFFER_SIZE,
			   	readStart = inputBuffer,
				remaining = 0;
			}

            memset(readStart, 0, readSize);
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
            file_fread(readStart, readSize, 1, file);
#endif

			mad_stream_buffer(&stream, inputBuffer, readSize + remaining);
			stream.error = MAD_ERROR_NONE;
		}

		if (mad_frame_decode(&frame, &stream))
		{
			if (MAD_RECOVERABLE(stream.error))
			{
				continue;
			}
			else
			{
				if(stream.error == MAD_ERROR_BUFLEN)
				{
                    continue;
				}
				else
				{
					status = 1;
					break;
				}
			}
		}

		mad_synth_frame(&synth, &frame);
		 
		for(i = 0; i < synth.pcm.length; i++)
		{
 			signed short sample = madFixedToSignedShort(synth.pcm.samples[0][i]);
			halI2sSrcAdjustVolumn((int16_t *)&sample, sizeof(sample), audioGetVolume());
            for (uint8_t j=0; j<factor; j++)
            {
                *(outputPtr++) = sample;
            }

			if(outputPtr == outputBufferEnd)
			{
                if (gCodecTx == CODEC_TX_IDLE)
                {
                    PLAY_ABORT_CHECK();
                    apiCodecStart(rate, channel);
                }

				while(gCodecTx == CODEC_TX_START)
                {
                    PLAY_ABORT_CHECK();
                }

				if (gCodecTx != CODEC_TX_START)
				{	 
                    audioBuff = &outputBuffer[outBufIndex][0];
                    PLAY_ABORT_CHECK();

#if defined(SPI_CODEC_ENABLE)
                    if(gCodecTx == CODEC_TX_IDLE)
                    {
                        apiCodecPlay(audioBuff, sizeof(outputBuffer[outBufIndex]));
                    }
                    else if (osSemaphoreAcquire(gSpiCodecSemaphore, osWaitForever) == osOK)
                    {
                        apiCodecPlay(audioBuff, sizeof(outputBuffer[outBufIndex]));
                    }
                    gCodecTx = CODEC_TX_FINISH;
#else
                    gCodecTx = CODEC_TX_START;
                    apiCodecPlay((uint8_t *)audioBuff, sizeof(outputBuffer[outBufIndex]));
#endif
				}

				if (++outBufIndex >= 2)
				{
                    outBufIndex = 0;
				}

				outputPtr = &outputBuffer[outBufIndex][0];
				outputBufferEnd = &outputBuffer[outBufIndex][0] + outputBufferSize;
			}	
		}
	}while(1);

	if (outputPtr != &outputBuffer[outBufIndex][0])
	{
		while(gCodecTx == CODEC_TX_START)
        {
            PLAY_ABORT_CHECK();
        }

		if (gCodecTx != CODEC_TX_START)
		{
			gCodecTx = CODEC_TX_START;
			audioBuff = &outputBuffer[outBufIndex][0];

            PLAY_ABORT_CHECK();
#if defined(SPI_CODEC_ENABLE)
            if (osSemaphoreAcquire(gSpiCodecSemaphore, osWaitForever) == osOK)
            {
                apiCodecPlay(audioBuff, outputPtr - &outputBuffer[outBufIndex][0]);
                gCodecTx = CODEC_TX_FINISH;
            }
#else
            apiCodecPlay((uint8_t *)audioBuff, outputPtr - &outputBuffer[outBufIndex][0]);
#endif
		}
	}

    while (gCodecTx == CODEC_TX_START)
    {
        PLAY_ABORT_CHECK();
        osDelay(5);
    }

labelEnd:
	mad_synth_finish(&synth);
	mad_frame_finish(&frame);
	mad_stream_finish(&stream);

	return status;
}

static int32_t mp3SampleRateGet(char *path)
{
    FILE     *file       = NULL;
    uint8_t   buffer[10] = {0};
    uint32_t  length     = 0;
    uint8_t   version    = 0;
    uint8_t   index      = 0;
    uint32_t  sample     = 0;
    uint32_t  table[3]   = {44100, 48000, 32000};

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    file = file_fopen(path, "r");
#endif
    if (file == NULL)
    {
        SYSLOG_ERR("Failed to open the file \"%s\".\r\n", path);
        goto labelEnd;
    }

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    file_fread(buffer, 10, 1, file);
    if (memcmp(buffer, "ID3", strlen("ID3")) == 0)
    {
        length = ((buffer[6] & 0x7F) << 21) + ((buffer[7] & 0x7F) << 14) + ((buffer[8] & 0x7F) << 7) + (buffer[9] & 0x7F);
        file_fseek(file, length, SEEK_CUR);
        file_fread(buffer, 3, 1, file);
    }
    file_fclose(file);
#endif

    version = (buffer[1] >> 3) & 0x03;
    index   = (buffer[2] >> 2) & 0x03;

    if (version == 0x00) // MPEG2.5
    {
        sample = table[index] / 4;
    }
    else if (version == 0x2) // MPEG2
    {
        sample = table[index] / 2;
    }
    else if (version == 0x3) // MPEG1
    {
        sample = table[index];
    }
    else
    {
        sample = 0;
    }

labelEnd:
    return sampleRateConvert(sample, true);
}

void mp3Play(char *path, bool increase, bool dual)
{
    FILE    *file = {0};
    int32_t  rate = 0;

    rate = mp3SampleRateGet(path);
    if (rate != -1)
    {
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
        file = file_fopen(path, "r");
#endif
        if (file != NULL)
        {
#ifndef PWM_CODEC_ENABLE
            apiCodecSetPaState(true);
            apiCodecBoot();
            osDelay(70);
#endif
            mp3Decoder(file, rate, increase, dual);

            apiCodecStop();

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
            file_fclose(file);
#endif
        }
        else
        {
            SYSLOG_ERR("Failed to open the file \"%s\".\r\n", path);
        }
    }
}
#endif
