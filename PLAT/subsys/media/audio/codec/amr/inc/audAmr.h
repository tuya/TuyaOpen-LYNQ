#ifndef __AMR_H__
#define __AMR_H__


#include <stdint.h>


int32_t amrPlay(UINT8 type, char *path);
int32_t amrPlayString(BOOL toneFlag, char *string,uint32_t stringLen);
int32_t amrPlayStream(BOOL toneFlag);
int32_t amrPlayStop(void);


#endif
