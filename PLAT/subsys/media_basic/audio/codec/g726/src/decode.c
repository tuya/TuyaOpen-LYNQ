#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "g726codec.h"
#include "g726.h"
#include "storage.h"
#include "wav.h"


#define BIT_RATE    2    // 2, 3, 4, 5.
#define READ_LENGHT 80


int32_t g726Decode(char *path)
{
    int32_t       resVal          = -1;
    FILE         *fileSource      = NULL;
    FILE         *fileDest        = NULL;
    struct stat   buf             = {0};
    bool          OpendSource     = false;
    bool          OpendDest       = false;
    int32_t       size            = 0;
    int32_t       count           = 0;
    uint32_t      length          = 0;
    g726_state_t *g_state         = NULL; //for g726_xx.
    char          ucInBuff[330]   = {0};
    char          ucOutBuff[2400] = {0};
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

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    fileSource = file_fopen(path, "r");
#endif
    if (fileSource == NULL)
    {
        printf("Failed to open the file \"%s\".\r\n", path);
        goto labelEnd;
    }
    OpendSource = true;
    file_fstat((int)fileSource, &buf);
    size = buf.st_size;
    printf("The size of the file %s is %d bytes.\r\n", path, size);

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    fileDest = file_fopen(G726_WAV_FILE, "w");
#endif
    if (fileDest == NULL)
    {
        printf("Failed to open the file \"%s\".\r\n", G726_WAV_FILE);
        goto labelEnd;
    }
    OpendDest = true;
    file_fwrite(&wavHead, 1, sizeof(wavHead), fileDest);

    g_state = G726_Init(g_state, BIT_RATE);
    if(g_state == NULL)
    {
        printf("Failed to init g726 struct.\r\n");
        goto labelEnd;
    }

    count = (size / READ_LENGHT) + (((size % READ_LENGHT) == 0) ? 0 : 1);
    for (uint32_t i=0; i<count; i++)
    {
        memset(ucInBuff, 0, sizeof(ucInBuff));
        memset(ucOutBuff, 0, sizeof(ucOutBuff));
        length = file_fread(ucInBuff, 1, READ_LENGHT, fileSource);
        length = G726_2PCM(BIT_RATE, (g726_state_t *)g_state, (char *)ucInBuff, (char *)ucOutBuff, length, 0 );
        file_fwrite(ucOutBuff, 1, length * 2, fileDest);
    }

    file_fstat((int)fileDest, &buf);
    wavHead.listOrDataLength = buf.st_size - sizeof(wavHead);
    wavHead.riffSize        += wavHead.listOrDataLength;
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    file_fseek(fileDest, 0, SEEK_SET);
    file_fwrite(&wavHead, 1, sizeof(wavHead), fileDest);
    printf("The size of the file %s is %d bytes.\r\n", G726_WAV_FILE, buf.st_size);
#endif

    resVal = 0;

labelEnd:
    if (g_state != NULL)
    {
        free(g_state);
        g_state = NULL;
    }

    if (OpendSource == true)
    {
        file_fclose(fileSource);
    }
    if (OpendDest == true)
    {
        file_fclose(fileDest);
    }

    return resVal;    
}
