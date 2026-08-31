#include <stdio.h>
#include <stdint.h>
#ifdef FEATURE_SUBSYS_AUDIO_ENABLE
#include "audio.h"
#include "record.h"
#endif


void subMediaInit(void)
{
#ifdef FEATURE_SUBSYS_AUDIO_ENABLE
    audioInit();
	recordInit();
#endif
	
}


