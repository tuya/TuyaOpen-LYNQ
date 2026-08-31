/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    audio.h
 * Description:  EC618 mqtt demo entry header file
 * History:      Rev1.0   2018-07-12
 *
 ****************************************************************************/
#ifndef __G726_H__
#define __G726_H__


#include <stdint.h>


#define WAV_G726_FILE   "C:/wav.g726"
#define G726_WAV_FILE   "C:/g726.wav"


int32_t g726Encode(char *path);
int32_t g726Decode(char *path);


#endif
