#ifndef __MED_PCM_H__
#define __MED_PCM_H__
#include <stdint.h>
#include <stdbool.h>
#include "media.h"

int32_t audPcmStringPlay(uint8_t *src,uint32_t size,audioParamT * audParam);
int32_t audPcmFilePlay(uint8_t *path, audioParamT *audParam);
int32_t audPcmStreamPlay(audioParamT *audParam);
int32_t audPcmPlayStop(void);
#endif/*__MED_PCM_H__*/
