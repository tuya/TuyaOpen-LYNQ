#ifndef __MP3_H__
#define __MP3_H__


#include <stdint.h>
#include <stdbool.h>
#include "media.h"

typedef struct _mp3PlayCtrlInfo{
	uint8_t rate;
	uint8_t toneBufIndex;	
	uint8_t toneFlag;
	uint32_t outSrcIndex;
	uint64_t ringBufFillCnt;
	uint64_t dmaBufFillCnt;
}mp3PlayCtrlInfo;


int32_t mp3Play(char *path, audioParamT *audParam);

int32_t mp3StringPlay(uint8_t *string,UINT32 stringLen,audioParamT *audParam);

int32_t mp3StreamPlay(audioParamT *audParam);

uint32_t mp3PlayStop(bool suspendFlag);

uint32_t getMp3PlaySchedule(void);

int32_t mp3GetInfo(char *filename, AudioInfo_t *audio_param);

#endif
