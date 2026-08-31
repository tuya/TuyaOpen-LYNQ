#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "g726codec.h"
#include "g726.h"
#include "storage.h"
#include "wav.h"


#define BIT_RATE    2    // 2, 3, 4, 5.
#define READ_LENGHT 80


int32_t g726Encode(char *path)
{
    int32_t       resVal         = -1;
    FILE         *fileSource     = NULL;
    FILE         *fileDest       = NULL;
    struct stat   buf            = {0};
    bool          OpendSource    = false;
    bool          OpendDest      = false;
    int32_t       size           = 0;
    int32_t       count          = 0;
    uint32_t      length         = 0;
    g726_state_t *g_state        = NULL; //for g726_xx.
    char          ucInBuff[100]  = {0};
    char          ucOutBuff[100] = {0};

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
    size = buf.st_size - sizeof(WavHeadT);
    printf("The size of the file %s is %d bytes.\r\n", path, buf.st_size);
    file_fseek(fileSource, sizeof(WavHeadT), SEEK_SET);

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
    fileDest = file_fopen(WAV_G726_FILE, "w");
#endif
    if (fileDest == NULL)
    {
        printf("Failed to open the file \"%s\".\r\n", WAV_G726_FILE);
        goto labelEnd;
    }
    OpendDest = true;

    g_state = G726_Init(g_state, BIT_RATE);
    if(g_state == NULL)
    {
        printf("Failed to init g726 struct\r\n");
        goto labelEnd;
    }

    count = (size / READ_LENGHT) + (((size % READ_LENGHT) == 0) ? 0 : 1);
    for (uint32_t i=0; i<count; i++)
    {
        memset(ucInBuff, 0, sizeof(ucInBuff));
        memset(ucOutBuff, 0, sizeof(ucOutBuff));
        length = file_fread(ucInBuff, 1, READ_LENGHT, fileSource);
        length = PCM2G726(BIT_RATE, (g726_state_t *)g_state, (char *)ucInBuff, (char *)ucOutBuff, length, 0);
        file_fwrite(ucOutBuff, 1, length, fileDest);
    }
    resVal = 0;
    file_fstat((int)fileDest, &buf);
    printf("The size of the file %s is %d bytes.\r\n", WAV_G726_FILE, buf.st_size);

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
