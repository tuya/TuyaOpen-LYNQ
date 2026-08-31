#ifndef __PWM_CODEC_H__
#define __PWM_CODEC_H__


#include <stdint.h>



void pwmdaInit(int sampleRate,int baseFreq,int shift);
void pwmdaSetData(int16_t *data,int len,int shift);
void pwmdaStart(void);
void pwmdaStop(void);


#endif
