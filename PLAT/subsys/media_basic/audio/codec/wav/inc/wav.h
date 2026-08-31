#ifndef __WAV_H__
#define __WAV_H__


#include <stdint.h>


typedef struct
{
    char     riffId[4];
    uint32_t riffSize;
    char     riffType[4];
    char     fmtId[4];
    uint32_t fmtSize;
    uint16_t fmtCompressionCode;
    uint16_t fmtChannels;
    uint32_t fmtSampleRate;
    uint32_t fmtBytesPerSec;
    uint16_t fmtBlockAlign;
    uint16_t fmtBitPerSample;
    char     listOrDataId[4];
    uint32_t listOrDataLength;
} WavHeadT;


int32_t wavPlay(char *name);


#endif
