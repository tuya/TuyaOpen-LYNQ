#ifndef __MED_PCM_H__
#define __MED_PCM_H__
#include <stdint.h>
#include <stdbool.h>
#include "media.h"

//tuya
#define MED_PCM_FRAME_SIZE					(1920)
#define PCM_DATA_CACHE_PRE_FILL_SIZE		(8*1024)	
#define PCM_DATA_CACHE_SIZE 				(PCM_DATA_CACHE_PRE_FILL_SIZE + MED_PCM_FRAME_SIZE*3)

int32_t audPcmStringPlay(uint8_t *src,uint32_t size,audioParamT * audParam);
int32_t audPcmFilePlay(uint8_t *path, audioParamT *audParam);
int32_t audPcmPlayStop(void);
#endif/*__MED_PCM_H__*/
