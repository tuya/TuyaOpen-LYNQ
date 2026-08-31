/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    test.h
 * Description:  EC618 mqtt demo entry header file
 * History:      Rev1.0   2018-07-12
 *
 ****************************************************************************/
#ifndef __INIPARSE_H__
#define __INIPARSE_H__


#include <stdint.h>


#ifdef EXTERNAL_TTS_LFS_ENABLE
#define DEFAULT_INFO                                "D:/default.info"
#else
#define DEFAULT_INFO                                "D:/default.info"
#endif


typedef enum
{
    INI_VALUE_INT = 0,
    INI_VALUE_INVALID_TYPE
} IniValueT;


void    *iniKeyValueRead(char *path, char *key, uint8_t valueType);
int32_t  iniKeyValueWrite(char *path, char *key, uint8_t valueType, void *value);


#endif
