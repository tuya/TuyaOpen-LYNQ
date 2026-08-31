/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    media.h
 * Description:  EC718 at command demo entry header file
 * History:      Rev1.0   2018-07-12
 *
 ****************************************************************************/
#ifndef  SUBSYS_MEDIA_H
#define  SUBSYS_MEDIA_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef FEATURE_SUBSYS_AUDIO_ENABLE
#include "audio.h"
#endif

void subMediaInit(void);

#ifdef __cplusplus
}
#endif

#endif /* SUBSYS_MEDIA_H */