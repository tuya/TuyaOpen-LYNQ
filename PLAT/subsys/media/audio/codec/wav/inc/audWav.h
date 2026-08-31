#ifndef __MED_WAV_H__
#define __MED_WAV_H__
#include <stdint.h>
#include <stdbool.h>
#include "media.h"

typedef struct _medWavFmt{
	int16_t audioFmt;
	int16_t numChannels;
	int32_t samplerate;
	int32_t byteRate;
	int16_t	blockAlign;
	int16_t bitsPerSample;	
}medWavFmt;

typedef struct _medWavChunk{
	int32_t chunkId;
	int32_t dataSize;
	int8_t data[0];
}medWavChunkT;

int32_t audWavFilePlay(uint8_t *path, audioParamT *audParam);
int32_t audWavStringPlay(const uint8_t *src, uint32_t size, audioParamT * audParam);
uint32_t audWavPlayStop(void);

#endif/*__MED_WAV_H__*/
