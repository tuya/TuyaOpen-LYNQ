/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    openime.h
 * Description:  EC7xx header file
 * History:      Rev1.0   
 *
 ****************************************************************************/
#ifndef __OPEN_IME_H__
#define __OPEN_IME_H__


#include "cmsis_os2.h"
#include "subsys.h"

typedef enum _IMELang {
    IMELANG_Unknown           = 0,
    IMELANG_Chinese_Mainland  = 0x1000,
} IMELang;

typedef enum _IMEInputMode {
    IME_KEYBOARD_DISABLE    = 0,
    IME_KEYBOARD_FULL       = 1,
    IME_KEYBOARD_T9         = 2,
} IMEInputMode;

#endif
