#ifndef __TTS_H__
#define __TTS_H__


#include <stdint.h>


int32_t ttsInit(void);
int32_t ttsPlay(BOOL toneFlag,char *text,uint32_t textLen);
void ttsStopPlay(BOOL toneStopFlag);


#endif
