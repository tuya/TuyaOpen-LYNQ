#ifndef __API_CODEC_H__
#define __API_CODEC_H__


#include <stdint.h>
#include "codecDrv.h"
#include "codec.h"
#ifdef PWM_CODEC_ENABLE
#include "pwmCodec.h"
#endif
#ifdef API_CODEC_SPI_PWM
#include "spiCodec.h"
#endif


typedef enum
{
    API_CODEC_ES8311 = ES8311,
    API_CODEC_ES8374 = ES8374,
    API_CODEC_ES8388 = ES8388,
    API_CODEC_ES7148 = ES7148,
    API_CODEC_ES7149 = ES7149,
    API_CODEC_TM8211 = TM8211,
    API_CODEC_ES7111,
    API_CODEC_JY6311,
    API_CODEC_PWM,
    API_CODEC_SPI_PWM,
    API_CODEC_SPI_SIGMADELTA,
} ApiCodecT;

#ifdef FEATURE_SUBSYS_PCM_RECORD_ENABLE
typedef enum
{
    CODEC_RX_IDLE    = 0,
    CODEC_RX_START   = 1,
    CODEC_RX_FINISH  = 2,
    CODEC_RX_INVALID = 3
} CodecRxT;
#endif


extern volatile uint8_t gCodecTx;
#ifdef FEATURE_SUBSYS_PCM_RECORD_ENABLE
extern volatile uint8_t gCodecRx;
#endif


void     apiCodecInit(ApiCodecT apiCodec);
void     apiCodecPaInit(void);
void     apiCodecBoot(void);
void     apiCodecStart(I2sSampleRate_e rate, I2sChannelSel_e channel);
void     apiCodecStop(void);
void     apiCodecPlay(uint8_t *data, uint32_t length);
void     apiCodecSetPaState(bool enable);
#ifdef FEATURE_SUBSYS_PCM_RECORD_ENABLE
void     apiCodecRecord(uint8_t *data, uint32_t length);
#endif
void     apiCodecResume(void);
void     codecVoltageSet(void);
int32_t  sampleRateConvert(uint32_t rate, bool toEnum);


#endif
