/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    subsys.h
 * Description:  EC718 at command demo entry header file
 * History:      Rev1.0   2018-07-12
 *
 ****************************************************************************/
#ifndef  SUBSYS_H
#define  SUBSYS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MBTK_OPENCPU_SUPPORT
#include "FreeRTOS.h"
#include "ostask.h"
#else
#include "mode_config.h"
#endif


typedef enum
{
    DEBUG_CMD  = 0,
    DEBUG_JSON = 1
} DebugT;


#ifdef __cplusplus
}
#endif

#endif /* SUBSYS_H */