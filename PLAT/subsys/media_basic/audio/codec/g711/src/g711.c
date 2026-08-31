#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#ifdef FEATURE_SUBSYS_WAV_ENABLE
#include "wav.h"
#endif
#include "g711.h"


#define MAX                 32635
#define BUFFER_SIZE_G711    512
#define BUFFER_SIZE_PCM     (BUFFER_SIZE_G711 * 2)


static unsigned char encode(short pcm)
{
    int sign = (pcm & 0x8000) >> 8;
    if (sign != 0)
        pcm = -pcm;
    if (pcm > MAX) pcm = MAX;
    int exponent = 7;
    int expMask;
    for (expMask = 0x4000; (pcm & expMask) == 0
        && exponent>0; exponent--, expMask >>= 1) { }
    int mantissa = (pcm >> ((exponent == 0) ? 4 : (exponent + 3))) & 0x0f;
    unsigned char alaw = (unsigned char)(sign | exponent << 4 | mantissa);
    return (unsigned char)(alaw^0xD5);
}

static short decode(unsigned char alaw)
{
    alaw ^= 0xD5;
    int sign = alaw & 0x80;
    int exponent = (alaw & 0x70) >> 4;
    int data = alaw & 0x0f;
    data <<= 4;
    data += 8;
    if (exponent != 0)
        data += 0x100;
    if (exponent > 1)
        data <<= (exponent - 1);
    return (short)(sign == 0 ? data : -data);
}

static int g711_encode(char* pCodecBits, const char* pBuffer, int nBufferSize)
{
    short* buffer = (short*)pBuffer;
    int i;
    for(i=0; i<nBufferSize/2; i++)
    {
        pCodecBits[i] = encode(buffer[i]);
    }
    return nBufferSize/2;
}

static int g711_decode(char* pRawData, const char* pBuffer, int nBufferSize)
{
    short *out_data = (short*)pRawData;
    int i;
    for(i=0; i<nBufferSize; i++)
    {
        out_data[i] = decode(pBuffer[i]);
    }
    return nBufferSize*2;
}

int32_t g711Encode(char *pathIn, char *pathOut)
{
    int32_t       retVal                       = -1;
    FILE         *fileSrc                      = NULL;
    FILE         *fileDest                     = NULL;
    struct stat   buf                          = {0};
    int32_t       size                         = 0;
    int32_t       count                        = 0;
    uint32_t      length                       = 0;
    char          bufferSrc[BUFFER_SIZE_PCM]   = {0};
    char          bufferDest[BUFFER_SIZE_G711] = {0};

    if ((pathIn == NULL) || (pathOut == NULL))
    {
        SYSLOG_DEBUG("Param error.\r\n");
        goto labelEnd;
    }

    fileSrc = file_fopen(pathIn, "r");
    if (fileSrc == NULL)
    {
        SYSLOG_DEBUG("Failed to open the file \"%s\".\r\n", pathIn);
        goto labelEnd;
    }

    file_fstat((int)fileSrc, &buf);
    size = buf.st_size - sizeof(WavHeadT);
    SYSLOG_DEBUG("The size of the file %s is %d bytes.\r\n", pathIn, buf.st_size);
    file_fseek(fileSrc, sizeof(WavHeadT), SEEK_SET);

    fileDest = file_fopen(pathOut, "w");
    if (fileDest == NULL)
    {
        SYSLOG_DEBUG("Failed to open the file \"%s\".\r\n", pathOut);
        goto labelEnd;
    }

    count = (size / BUFFER_SIZE_PCM) + (((size % BUFFER_SIZE_PCM) == 0) ? 0 : 1);
    for (uint32_t i=0; i<count; i++)
    {
        memset(bufferSrc,  0, sizeof(bufferSrc));
        memset(bufferDest, 0, sizeof(bufferDest));
        length = file_fread(bufferSrc, 1, BUFFER_SIZE_PCM, fileSrc);
        length = g711_encode(bufferDest, bufferSrc, length);
        file_fwrite(bufferDest, 1, length, fileDest);
    }

    file_fstat((int)fileDest, &buf);
    SYSLOG_DEBUG("The size of the file %s is %d bytes.\r\n", pathOut, buf.st_size);

    retVal = 0;

labelEnd:
    if (fileSrc != NULL)
    {
        file_fclose(fileSrc);
    }
    if (fileDest != NULL)
    {
        file_fclose(fileDest);
    }

    return retVal;
}

int32_t g711Decode(char *pathIn, char *pathOut)
{
    int32_t       retVal                       = -1;
    FILE         *fileSrc                      = NULL;
    FILE         *fileDest                     = NULL;
    struct stat   buf                          = {0};
    int32_t       size                         = 0;
    int32_t       count                        = 0;
    uint32_t      length                       = 0;
    char          bufferSrc[BUFFER_SIZE_PCM]   = {0};
    char          bufferDest[BUFFER_SIZE_G711] = {0};
    WavHeadT      wavHead   =
    {
        .riffId             = {'R', 'I', 'F', 'F'},
        .riffSize           = sizeof(wavHead) - sizeof(wavHead.riffId) - sizeof(wavHead.riffSize),
        .riffType           = {'W', 'A', 'V', 'E'},
        .fmtId              = {'f', 'm', 't', ' '},
        .fmtSize            = 16,
        .fmtCompressionCode = 1,
        .fmtChannels        = 1,
        .fmtSampleRate      = 8000,
        .fmtBytesPerSec     = 16000,
        .fmtBlockAlign      = 2,
        .fmtBitPerSample    = 16,
        .listOrDataId       = {'d', 'a', 't', 'a'},
        .listOrDataLength   = 0
    };

    if ((pathIn == NULL) || (pathOut == NULL))
    {
        SYSLOG_DEBUG("Param error.\r\n");
        goto labelEnd;
    }

    fileSrc = file_fopen(pathIn, "r");
    if (fileSrc == NULL)
    {
        SYSLOG_DEBUG("Failed to open the file \"%s\".\r\n", pathIn);
        goto labelEnd;
    }

    file_fstat((int)fileSrc, &buf);
    size = buf.st_size;
    SYSLOG_DEBUG("The size of the file %s is %d bytes.\r\n", pathIn, size);

    fileDest = file_fopen(pathOut, "w");
    if (fileDest == NULL)
    {
        SYSLOG_DEBUG("Failed to open the file \"%s\".\r\n", pathOut);
        goto labelEnd;
    }
    file_fwrite(&wavHead, 1, sizeof(wavHead), fileDest);

    count = (size / BUFFER_SIZE_G711) + (((size % BUFFER_SIZE_G711) == 0) ? 0 : 1);
    for (uint32_t i=0; i<count; i++)
    {
        memset(bufferSrc,  0, sizeof(bufferSrc));
        memset(bufferDest, 0, sizeof(bufferDest));
        length = file_fread(bufferSrc, 1, BUFFER_SIZE_G711, fileSrc);
        length = g711_decode(bufferDest, bufferSrc, length);
        file_fwrite(bufferDest, 1, length, fileDest);
    }

    file_fstat((int)fileDest, &buf);
    wavHead.listOrDataLength  = buf.st_size - sizeof(wavHead);
    wavHead.riffSize         += wavHead.listOrDataLength;
    file_fseek(fileDest, 0, SEEK_SET);
    file_fwrite(&wavHead, 1, sizeof(wavHead), fileDest);
    SYSLOG_DEBUG("The size of the file %s is %d bytes.\r\n", pathOut, buf.st_size);

    retVal = 0;

labelEnd:
    if (fileSrc != NULL)
    {
        file_fclose(fileSrc);
    }
    if (fileDest != NULL)
    {
        file_fclose(fileDest);
    }

    return retVal;
}
