/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    audio.h
 * Description:  EC618 mqtt demo entry header file
 * History:      Rev1.0   2018-07-12
 *
 ****************************************************************************/
#ifndef __G711_H__
#define __G711_H__


#include <stdint.h>
#include "media.h"

int32_t g711Encode(char *pathIn, char *pathOut);
int32_t g711Decode(char *pathIn, char *pathOut);
int32_t audG711StreamPlay(audioParamT *audParam);
int32_t audG711PlayStop(void);

#endif
