#ifndef __RECORD_H__
#define __RECORD_H__


#include <stdint.h>


typedef struct
{
    char     *fileName;
    uint32_t  fileSize;
} RecordInfoT;

typedef void (*RecordCallbackT)(RecordInfoT *info);

typedef struct
{
    char            *fileName;
    uint8_t          type;
    uint8_t          quality;
    uint32_t         time;
    RecordCallbackT  callback;
} RecordParamT;


#ifdef FEATURE_SUBSYS_PCM_RECORD_ENABLE
#include "pcmRecord.h"
#endif
#ifdef FEATURE_SUBSYS_AMR_RECORD_ENABLE
#include "amrRecord.h"
#endif


#endif
