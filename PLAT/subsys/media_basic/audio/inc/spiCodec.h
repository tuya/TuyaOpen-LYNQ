#ifndef __SPI_CODEC_H__
#define __SPI_CODEC_H__


#include <stdint.h>


#define SPI_CODEC_FREQ                  25600000
/**************************************** sigmaDelta *********************************/
#define TIME_MS                         20
#define TOTAL_CHAIN_BUFFER_SIZE         (SPI_CODEC_FREQ / 8 / 1000 * TIME_MS * 2)
#define HALF_CHAIN_BUFFER_SIZE          (TOTAL_CHAIN_BUFFER_SIZE / 2)
#define ONE_CHAIN_BUFFER_SIZE           8000 // TOTAL_CHAIN_BUFFER_SIZE / ONE_CHAIN_BUFFER_SIZE = TOTAL_CHAIN_COUNT
#define TOTAL_CHAIN_COUNT               16    // TOTAL_CHAIN_BUFFER_SIZE / ONE_CHAIN_BUFFER_SIZE = TOTAL_CHAIN_COUNT
#define HALF_CHAIN_COUNT                (TOTAL_CHAIN_COUNT / 2)


void spiCodecInit(void);
void spiCodecStart(void);
void spiCodecStop(void);
void spiPwmCodecPlay(int16_t *buffer, uint32_t length);
void spiSigmadeltaCodecPlay(int16_t *buffer, uint32_t length);
void spiCodecCallback(uint32_t event);


#endif
