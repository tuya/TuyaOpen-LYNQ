/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    test.h
 * Description:  EC618 mqtt demo entry header file
 * History:      Rev1.0   2018-07-12
 *
 ****************************************************************************/
#ifndef __MODE_H__
#define __MODE_H__


#include <stdint.h>


typedef enum
{
    MODE_NORMAL     = 0,
    MODE_TEST       = 1,
    MODE_POWEROFF   = 2 // power off charge / power off alarm
} ModeT;


void    modeSelect(void);
int32_t modeSave(uint8_t mode);
uint8_t modeGet(void);
void    debugSet(uint8_t debug);
uint8_t debugGet(void);
uint8_t debugTypeGet(void);


#endif
