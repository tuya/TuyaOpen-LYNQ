#ifndef __PCM_H__
#define __PCM_H__


#include <stdint.h>
#ifdef FEATURE_SUBSYS_G726_ENABLE
#include "g726.h"
#endif
#ifdef FEATURE_SUBSYS_ADPCM_ENABLE
#include "adpcm.h"
#endif
#ifdef FEATURE_SUBSYS_G711_ENABLE
#include "g711.h"
#endif


#define PCM_PARAM_00H                               "00h"
#define PCM_PARAM_SIN                               "sin"
#define PCM_PARAM_END                               "end"


int32_t  pcmPlay(uint8_t *pcm, uint32_t length, uint32_t rate);
void     pcmEndPlay(void);
uint32_t pcmSinGet(uint32_t freq, uint32_t rate, int16_t **data);


#endif
