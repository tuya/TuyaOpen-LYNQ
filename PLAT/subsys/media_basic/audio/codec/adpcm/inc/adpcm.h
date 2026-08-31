/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    audio.h
 * Description:  EC618 mqtt demo entry header file
 * History:      Rev1.0   2018-07-12
 *
 ****************************************************************************/
#ifndef __ADPCM_H__
#define __ADPCM_H__


#include <stdint.h>


#define WAV_ADPCM_FILE      "D:/wav.adpcm"
#define ADPCM_WAV_FILE      "D:/adpcm.wav"


int32_t adpcmEncode(char *path);
int32_t adpcmDecode(char *path);


#endif
