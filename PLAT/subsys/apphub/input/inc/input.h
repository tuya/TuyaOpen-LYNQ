/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    input.h
 * Description:  EC718 at command demo entry header file
 * History:      Rev1.0   2018-07-12
 *
 ****************************************************************************/
#ifndef  SUBSYS_INPUT_H
#define  SUBSYS_INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "subsys.h"

#define INPUTPROC_MAX_NUM   6

typedef void (*InputProcCallbackT)(void);

void subInputInit(void);
int32_t inputNotify(void);
void mountInputProc(InputProcCallbackT pFun,int index);

#ifdef __cplusplus
}
#endif

#endif /* SUBSYS_INPUT_H */