#ifndef __CODEC_H__
#define __CODEC_H__


#include <stdint.h>
#include <stdbool.h>
#include "hal_i2s.h"
#include "codecDrv.h"


typedef enum
{
    CODEC_TX_IDLE    = 0,
    CODEC_TX_START   = 1,
    CODEC_TX_FINISH  = 2,
    CODEC_TX_INVALID = 3
} CodecTxT;


void codecI2sInit(HalCodecType_e codecType);
void codecBoot(HalCodecType_e codecType);
void codecStart(I2sSampleRate_e rate, I2sChannelSel_e channel);
void codecStop(void);
void codecPlay(uint8_t *data, uint32_t length);
#ifdef FEATURE_SUBSYS_PCM_RECORD_ENABLE
void codecRecord(uint8_t *data, uint32_t length);
#endif


#endif
