#pragma once
#ifndef _AACDEC_VPU_COMMON_H_
#define _AACDEC_VPU_COMMON_H_

#ifndef WIN32
#include "sctdef.h"
#else
//
#endif
#include <stdlib.h>//for calloc
//#include "fixed.h"

#define Vpu_int32       signed long
#define Vpu_int16       short
#define Vpu_int8        signed char
#define Vpu_uint32      unsigned int
#define Vpu_uint16      unsigned short
#define Vpu_uint8       unsigned char
#define Vpu_int64       long long


#ifndef WIN32
void aacDecVpuInit(void);
void aacDecVpuClock_On(void);
void aacDecVpuClock_Off(void);
#endif


#if 1

#define OVERLAPADD_VPU_ENABLE
#ifdef OVERLAPADD_VPU_ENABLE
void overlapAdd_vpu(Vpu_int32* overlap_in, Vpu_int32* transf_in, const Vpu_int32* window_in, Vpu_int32* out, int len);
#endif


#define OVERLAP_VPU_ENABLE
#ifdef OVERLAP_VPU_ENABLE
void overlap_vpu(Vpu_int32* transf_in, const Vpu_int32* window_in, Vpu_int32* out, int len);
#endif


#define memcpyc


#define MEMSET0INT32_DEC_VPU_ENBALE
#ifdef MEMSET0INT32_DEC_VPU_ENBALE
void memset0int32_dec_vpu(Vpu_int32* in, int len);
#endif



#define ADD_VPU_ENABLE
#ifdef ADD_VPU_ENABLE
void add_vpu(Vpu_int32* overlap_in, Vpu_int32* transf_in, Vpu_int32* out, int len);
#endif

#endif





#endif



